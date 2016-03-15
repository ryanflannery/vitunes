/*
 * Copyright (c) 2016 Ryan Flannery <ryan.flannery@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "mediadb.h"

#include "../schema.c"

#include "../mfile/mfile.h"
#include "../mfile/taglib/mfile_taglib.h"
#include "../plist/plist.h"
#include "../util/dparray.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fts.h>
#include <err.h>


mediadb*
create_new_mdb(const char *dbfile, sqlite3 *dbhandle)
{
   mediadb *mdb = NULL;

   if (NULL == (mdb = (mediadb*) malloc(sizeof(mediadb))))
      err(1, "%s: malloc failed", __FUNCTION__);

   if (NULL == (mdb->dbfile = strdup(dbfile)))
      err(1, "%s: strdup failed", __FUNCTION__);

   mdb->dbhandle = dbhandle;
   return mdb;
}

mediadb*
mdb_init(const char *db)
{
   sqlite3 *sql = NULL;
   char    *err;

   /* assert file doesn't exist - we promise not to blow away anything */
   if (!access(db, F_OK))
   {
      warnx("%s: file %s already exists", __FUNCTION__, db);
      return NULL;
   }

   if (SQLITE_OK != sqlite3_open(db, &sql))
   {
      warnx("%s: failed to create %s: %s", __FUNCTION__,db,sqlite3_errmsg(sql));
      return NULL;
   }

   if (SQLITE_OK != sqlite3_exec(sql, (const char*)schema_sql, NULL, NULL, &err))
   {
      warnx("%s: failed to create db: %s", __FUNCTION__, sqlite3_errmsg(sql));
      sqlite3_free(err);
      sqlite3_close(sql);
      return NULL;
   }

   return create_new_mdb(db, sql);
}

mediadb*
mdb_open(const char *db)
{
   sqlite3 *sql = NULL;

   /* this will fail if the file 'db' doesn't exist (intentional) */
   if (SQLITE_OK != sqlite3_open_v2(db, &sql, SQLITE_OPEN_READWRITE, NULL))
   {
      warnx("%s: failed to open %s: %s", __FUNCTION__, db, sqlite3_errmsg(sql));
      return NULL;
   }

   return create_new_mdb(db, sql);
}

void
mdb_close(mediadb *mdb)
{
   sqlite3_close(mdb->dbhandle);
}

bool
mdb_scan_file(mediadb *mdb, const char *file)
{
   mfile *m;

   if (NULL == (m = mfile_extract_tags(file)))
      return false;

   static const char *SQL =
      "INSERT OR REPLACE INTO mfiles ("
      "  filename, artist, album, title, comment, genre, "
      "  length, bitrate, samplerate, channels, "
      "  last_update"
      ") VALUES ("
      "  ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
      "  strftime('\%Y-\%m-\%d \%H:\%M:\%S', 'now')"
      ")";

   sqlite3_stmt *insert = NULL;
   if (SQLITE_OK != sqlite3_prepare_v2(mdb->dbhandle, SQL, -1, &insert, NULL))
   {
      errx(1, "%s: failed to execute %s: %s", __FUNCTION__, SQL,
            sqlite3_errmsg(mdb->dbhandle));
   }

#  define BIND_TEXT(index, string) \
   if (SQLITE_OK != sqlite3_bind_text(insert, index, string, -1, NULL)) \
   { \
      errx(1, "%s: bind failed for %s: %s", __FUNCTION__, #string, \
         sqlite3_errmsg(mdb->dbhandle)); \
   }

   BIND_TEXT(1, m->filename)
   BIND_TEXT(2, m->artist);
   BIND_TEXT(3, m->album);
   BIND_TEXT(4, m->title);
   BIND_TEXT(5, m->comment);
   BIND_TEXT(6, m->genre);

#  undef BIND_TEXT
#  define BIND_INT(index, i) \
   if (SQLITE_OK != sqlite3_bind_int(insert, index, i)) \
   { \
      errx(1, "%s: bind failed for %s: %s", __FUNCTION__, #i, \
         sqlite3_errmsg(mdb->dbhandle)); \
   }

   BIND_INT(7, m->length);
   BIND_INT(8, m->bitrate);
   BIND_INT(9, m->samplerate);
   BIND_INT(10, m->channels);

#  undef BIND_INT

   if (SQLITE_DONE != sqlite3_step(insert))
   {
      errx(1, "%s: failed to execute: %s", __FUNCTION__,
            sqlite3_errmsg(mdb->dbhandle));
   }

   sqlite3_finalize(insert);

   return true;
}

void
mdb_scan_dirs(mediadb *mdb, char *dirs[])
{
   FTSENT   *ftsent;
   FTS      *fts;

   if (NULL == (fts = fts_open(dirs, FTS_LOGICAL | FTS_NOCHDIR, NULL)))
      err(1, "%s: fts_open() failed", __FUNCTION__);

   while (NULL != (ftsent = fts_read(fts)))
   {
      switch (ftsent->fts_info)
      {
         case FTS_F:    /* regular file */
            if (mdb_scan_file(mdb, ftsent->fts_accpath))
               printf("  ✓ %s\n", ftsent->fts_accpath);

            break;

         case FTS_D:    /* directory (going in) */
            printf("Entering %s\n", ftsent->fts_path);
            break;

         case FTS_DP:   /* directory (coming out) */
         case FTS_DNR:  /* unreadable directory */
         case FTS_NS:   /* file/dir that stat(2) failed on */
         case FTS_ERR:  /* some other error reading/stat'ing file/dir */
         default:       /* there are many other options */
            break;
      }
   }

   if (-1 == fts_close(fts))
      err(1, "%s: fts_close() failed", __FUNCTION__);
}

void
mdb_rescan_files(mediadb *mdb)
{
}

void
extract_files(mediadb *mdb)
{
   static const char *SQL = "SELECT filename FROM mfiles";
   dparray *d = dparray_new(1000);
   size_t count = 0;

   sqlite3_stmt *select = NULL;
   if (SQLITE_OK != sqlite3_prepare_v2(mdb->dbhandle, SQL, -1, &select, NULL))
   {
      errx(1, "%s: failed to execute %s: %s", __FUNCTION__, SQL,
            sqlite3_errmsg(mdb->dbhandle));
   }

   int rc;
   while(SQLITE_ROW == (rc = sqlite3_step(select)))
   {
      const unsigned char *filename = sqlite3_column_text(select, 0);
      dparray_append_record(d, (void*)filename);

      count++;
   }

   if(SQLITE_DONE != rc)
   {
      fprintf(stderr, "select statement didn't finish with DONE (%i): %s\n",
            rc, sqlite3_errmsg(mdb->dbhandle));
   }

   sqlite3_finalize(select);

   /*mdb_scan_file(mdb, (const char*)filename);*/

   printf("extracted %zu filenames\n", count);
}
