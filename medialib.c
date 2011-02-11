/*
 * Copyright (c) 2010, 2011 Ryan Flannery <ryan.flannery@gmail.com>
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

#include "medialib.h"

/* The global media library struct */
medialib mdb;

/*
 * Load the global media library from disk. The location of the database file
 * and the directory containing all of the playlists must be specified.
 */
void
medialib_load(const char *db_file, const char *playlist_dir)
{
   playlist  *p;
   char     **pfiles;
   int        npfiles;
   int        i;

   /* copy file/directory names */
   mdb.db_file      = strdup(db_file);
   mdb.playlist_dir = strdup(playlist_dir);
   if (mdb.db_file == NULL || mdb.playlist_dir == NULL)
      err(1, "failed to strdup db file and playlist dir in medialib_init");

   /* setup pseudo-playlists */
   mdb.library = playlist_new();
   mdb.library->filename = NULL;
   mdb.library->name = strdup("--LIBRARY--");

   mdb.filter_results = playlist_new();
   mdb.filter_results->filename = NULL;
   mdb.filter_results->name = strdup("--FILTER--");

   if (mdb.library->name == NULL || mdb.filter_results->name == NULL)
      err(1, "failed to strdup psuedo-names in medialib_load");

   /* load the actual database */
   medialib_db_load(db_file);

   /* setup initial record keeping for playlists */
   mdb.nplaylists = 0;
   mdb.playlists_capacity = 2;
   mdb.playlists = calloc(2, sizeof(playlist*));
   if (mdb.playlists == NULL)
      err(1, "medialib_load: failed to allocate initial playlists");

   /* add library/filter pseudo-playlists */
   medialib_playlist_add(mdb.library);
   medialib_playlist_add(mdb.filter_results);

   /* load the rest */
   npfiles = retrieve_playlist_filenames(mdb.playlist_dir, &pfiles);
   for (i = 0; i < npfiles; i++) {
      p = playlist_load(pfiles[i], mdb.library->files, mdb.library->nfiles);
      medialib_playlist_add(p);
      free(pfiles[i]);
   }

   /* set all playlists as saved initially */
   for (i = 0; i < mdb.nplaylists; i++)
      mdb.playlists[i]->needs_saving = false;

   free(pfiles);
}

/* free() all memory associated with global media library */
void
medialib_destroy()
{
   int i;

   /* free the database */
   for (i = 0; i < mdb.library->nfiles; i++)
      mi_free(mdb.library->files[i]);

   /* free all the playlists */
   for (i = 0; i < mdb.nplaylists; i++)
      playlist_free(mdb.playlists[i]);

   /* free all other allocated mdb members */
   free(mdb.playlists);
   free(mdb.db_file);
   free(mdb.playlist_dir);

   /* reset counters */
   mdb.nplaylists = 0;
   mdb.playlists_capacity = 0;
}

/* add a new playlist to the media library */
void
medialib_playlist_add(playlist *p)
{
   playlist **new_playlists;
   int        size;

   /* check to see if we need to resize the array */
   if (mdb.nplaylists == mdb.playlists_capacity) {
      mdb.playlists_capacity += MEDIALIB_PLAYLISTS_CHUNK_SIZE;
      size = mdb.playlists_capacity * sizeof(playlist*);
      if ((new_playlists = realloc(mdb.playlists, size)) == NULL)
         err(1, "medialib_playlist_add: realloc failed");

      mdb.playlists = new_playlists;
   }

   mdb.playlists[mdb.nplaylists++] = p;
}

/*
 * remove a playlist from the media library, and disk, given the playlist's
 * index in the playlists array.
 */
void
medialib_playlist_remove(int pindex)
{
   int i;

   if (pindex < 0 || pindex >= mdb.nplaylists)
      errx(1, "medialib_playlist_remove: index %d out of range", pindex);

   playlist_delete(mdb.playlists[pindex]);

   /* reorder */
   for (i = pindex + 1; i < mdb.nplaylists; i++)
      mdb.playlists[i - 1] = mdb.playlists[i];

   mdb.nplaylists--;
}

/*
 * create the vitunes directory, database file (initially empty, and
 * playlists directory.
 */
void
medialib_setup_files(const char *vitunes_dir, const char *db_file,
   const char *playlist_dir)
{
   struct stat sb;
   FILE *f;
   int   version[3] = {DB_VERSION_MAJOR, DB_VERSION_MINOR, DB_VERSION_OTHER};

   /* create vitunes directory */
   if (mkdir(vitunes_dir, S_IRWXU) == -1) {
      if (errno == EEXIST)
         warnx("vitunes directory '%s' already exists (OK)", vitunes_dir);
      else
         err(1, "unable to create vitunes directory '%s'", vitunes_dir);
   } else
      warnx("vitunes directory '%s' created", vitunes_dir);

   /* create playlists directory */
   if (mkdir(playlist_dir, S_IRWXU) == -1) {
      if (errno == EEXIST)
         warnx("playlists directory '%s' already exists (OK)", playlist_dir);
      else
         err(1, "unable to create playlists directory '%s'", playlist_dir);
   } else
      warnx("playlists directory '%s' created", playlist_dir);

   /* create database file */
   if (stat(db_file, &sb) < 0) {
      if (errno == ENOENT) { 

         /* open for writing */
         if ((f = fopen(db_file, "w")) == NULL)
            err(1, "failed to create database file '%s'", db_file);

         /* save header & version */
         fwrite("vitunes", strlen("vitunes"), 1, f);
         fwrite(version, sizeof(version), 1, f);

         warnx("empty database at '%s' created", db_file);
         fclose(f);
      } else
         err(1, "database file '%s' exists, but cannot access it", db_file);
   } else
      warnx("database file '%s' already exists (OK)", db_file);
}

/* used to sort media db by filenames. */
static int mi_cmp_fn(const void *ai, const void *bi)
{
   const meta_info **a2 = (const meta_info **) ai;
   const meta_info **b2 = (const meta_info **) bi;
   const meta_info *a = (const meta_info *) *a2;
   const meta_info *b = (const meta_info *) *b2;

   return strcmp(a->filename, b->filename);
}

/* load the library database into the global media library */
void
medialib_db_load(const char *db_file)
{
   meta_info *mi;
   FILE      *fin;
   char       header[255] = { 0 };
   int        version[3];

   if ((fin = fopen(db_file, "r")) == NULL)
      err(1, "medialib_db_load: failed to open database file '%s'", db_file);

   /* read and check header & version */
   fread(header, strlen("vitunes"), 1, fin);
   if (strncmp(header, "vitunes", strlen("vitunes")) != 0)
      errx(1, "medialib_db_load: db file '%s' NOT a vitunes database", db_file);

   fread(version, sizeof(version), 1, fin);
   if (version[0] != DB_VERSION_MAJOR || version[1] != DB_VERSION_MINOR
   ||  version[2] != DB_VERSION_OTHER) {
      printf("Loading vitunes database: old database version detected.\n");
      printf("\tExisting database at '%s' is of version %d.%d.%d\n",
         db_file, version[0], version[1], version[2]);
      printf("\tThis version of vitunes only works with version %d.%d.%d\n",
         DB_VERSION_MAJOR, DB_VERSION_MINOR, DB_VERSION_OTHER);
      printf("Remove the existing database and rebuild by doing:\n");
      printf("\t$ rm %s\n", db_file);
      printf("\t$ vitunes -e init\n");
      printf("\t$ vitunes -e add /path/to/music ...\n");
      fflush(stdout);
      exit(1);
   }

   /* read rest of records */
   while (!feof(fin)) {
      mi = mi_new();
      mi_fread(mi, fin);
      if (feof(fin))
         mi_free(mi);
      else if (ferror(fin))
         err(1, "medialib_db_load: error loading database file '%s'", db_file);
      else
         playlist_files_append(mdb.library, &mi, 1, false);
   }

   fclose(fin);

   /* sort library by filenames */
   qsort(mdb.library->files, mdb.library->nfiles, sizeof(meta_info*), mi_cmp_fn);
}

/* save the library database from the global media library to disk */
void
medialib_db_save(const char *db_file)
{
   FILE *fout;
   int   version[3] = {DB_VERSION_MAJOR, DB_VERSION_MINOR, DB_VERSION_OTHER};
   int   i;

   if ((fout = fopen(db_file, "w")) == NULL)
      err(1, "medialib_db_save: failed to open database file '%s'", db_file);

   /* save header & version */
   fwrite("vitunes", strlen("vitunes"), 1, fout);
   fwrite(version, sizeof(version), 1, fout);

   /* save records */
   for (i = 0; i < mdb.library->nfiles; i++) {
      mi_fwrite(mdb.library->files[i], fout);
      if (ferror(fout))
         err(1, "medialib_db_save: error saving database");
   }

   fclose(fout);
}

/* flush the library to stdout in a csv format */
void
medialib_db_flush(FILE *fout, const char *timefmt)
{
   meta_info *mi;
   struct tm *ltime;
   char stime[255];
   int f, i;

   /* header row */
   fprintf(fout, "filename, ");
   for (i = 0; i < MI_NUM_CINFO; i++)
      fprintf(fout, "\"%s\", ", MI_CINFO_NAMES[i]);

   fprintf(fout, "length-seconds, is_url, \"last-updated\"\n");
   fflush(fout);

   /* start output of db */
   for (f = 0; f < mdb.library->nfiles; f++) {

      /* get record */
      mi = mdb.library->files[f];

      /* output record */
      fprintf(fout, "%s, ", mi->filename);
      for (i = 0; i < MI_NUM_CINFO; i++)
         fprintf(fout, "\"%s\", ", mi->cinfo[i]);

      /* convert last-updated time to string */
      ltime = localtime(&(mi->last_updated));
      strftime(stime, sizeof(stime), timefmt, ltime);

      fprintf(fout, "%i, %s, \"%s\"\n",
         mi->length, (mi->is_url ? "true" : "false"), stime);

      fflush(fout);
   }
}

/*
 * AFTER loading the global media library using medialib_load(), this function
 * is used to re-scan all files that exist in the database and re-check their
 * meta_info.  Any files that no longer exist are removed, and any meta
 * information that has changed is updated.
 *
 * The database is then re-saved to disk.
 */
void
medialib_db_update(bool show_skipped)
{
   meta_info *mi;
   struct stat sb;
   char  *filename;
   int    i;

   /* stat counters */
   int    count_removed_file_gone = 0;
   int    count_removed_meta_gone = 0;
   int    count_skipped_not_updated = 0;
   int    count_updated = 0;
   int    count_errors = 0;
   int    count_urls = 0;

   for (i = 0; i < mdb.library->nfiles; i++) {

      filename = mdb.library->files[i]->filename;

      /* skip url's */
      if (mdb.library->files[i]->is_url) {
         printf("s %s\n", filename);
         count_urls++;
         continue;
      }

      if (stat(filename, &sb) == -1) {

         /* file was removed -or- stat() failed */

         if (errno == ENOENT) {
            /* file was removed, remove from library */
            playlist_files_remove(mdb.library, i, 1, false);
            i--;  /* since removed a file, we want to decrement i */
            printf("x %s\n", filename);
            count_removed_file_gone++;
         } else {
            /* stat() failed for some reason - unknown error */
            printf("? %s\n", filename);
            count_errors++;
         }

      } else {

         /*
          * file still exists... check if it has been modified since we
          * last extracted meta-info from it (otherwise we ignore)
          */

         if (sb.st_mtime > mdb.library->files[i]->last_updated) {

            mi = mi_extract(filename);
            if (mi == NULL) {
               /* file now has no meta-info, remove from library */
               playlist_files_remove(mdb.library, i, 1, false);
               i--;  /* since removed a file, we want to decrement i */
               printf("- %s\n", filename);
               count_removed_meta_gone++;
            } else {
               /* file's meta-info has changed, update it */
               mi_sanitize(mi);
               playlist_file_replace(mdb.library, i, mi);
               printf("u %s\n", filename);
               count_updated++;
            }
         } else {
            count_skipped_not_updated++;
            if (show_skipped)
               printf(". %s\n", filename);
         }

      }
   }

   /* save to file */
   medialib_db_save(mdb.db_file);

   /* output some of our stats */
   printf("--------------------------------------------------\n");
   printf("Results of updating database...\n");
   printf("(s) %9d url's skipped\n", count_urls);
   printf("(u) %9d files updated\n", count_updated);
   printf("(x) %9d files removed (file no longer exists)\n",
      count_removed_file_gone);
   printf("(-) %9d files removed (meta-info gone)\n",
      count_removed_meta_gone);
   printf("(.) %9d files skipped (file unchanged since last checked)\n",
      count_skipped_not_updated);
   printf("(?) %9d files with errors (couldn't stat, but kept)\n",
      count_errors);
}

/*
 * AFTER loading the global media library using medialib_load(), this function
 * will scan the list of directories specified in the parameter and
 */
void
medialib_db_scan_dirs(char *dirlist[])
{
   FTS        *fts;
   FTSENT     *ftsent;
   meta_info  *mi;
   char        fullname[PATH_MAX];
   int         i, idx;

   /* stat counters */
   int         count_removed_lost_info = 0;
   int         count_updated = 0;
   int         count_skipped_no_info = 0;
   int         count_skipped_dir = 0;
   int         count_skipped_error = 0;
   int         count_skipped_not_updated = 0;
   int         count_added = 0;



   fts = fts_open(dirlist, FTS_LOGICAL | FTS_NOCHDIR, NULL);
   if (fts == NULL)
      err(1, "medialib_db_scan_dirs: fts_open failed");

   while ((ftsent = fts_read(fts)) != NULL) {

      switch (ftsent->fts_info) {   /* file type */
         case FTS_D:    /* TYPE: directory (going in) */
            printf("Checking Directory: %s\n", ftsent->fts_path);
            break;

         case FTS_DP:   /* TYPE: directory (coming out) */
            break;

         case FTS_DNR:  /* TYPE: unreadable directory */
            printf("Directory '%s' Unreadable\n", ftsent->fts_accpath);
            count_skipped_dir++;
            break;

         case FTS_NS:   /* TYPE: file/dir that couldn't be stat(2) */
         case FTS_ERR:  /* TYPE: other error */
            printf("? %s\n", ftsent->fts_path);
            count_skipped_error++;
            break;

         case FTS_F:    /* TYPE: regular file */

            /* get the full name for the file */
            if (realpath(ftsent->fts_accpath, fullname) == NULL) {
               err(1, "medialib_db_scan_dirs: realpath failed for '%s'",
                  ftsent->fts_accpath);
            }

            /* check if the file already exists in the db */
            idx = -1;
            for (i = 0; i < mdb.library->nfiles; i++) {
               if (strcmp(fullname, mdb.library->files[i]->filename) == 0)
                  idx = i;
            }

            if (idx != -1) {
               /* file already exists in library database - update */

               if (ftsent->fts_statp->st_mtime >
                   mdb.library->files[idx]->last_updated) {

                  /* file has been modified since we last extracted info */

                  mi = mi_extract(ftsent->fts_accpath);

                  if (mi == NULL) {
                     /* file now has no meta-info, remove from library */
                     playlist_files_remove(mdb.library, idx, 1, false);
                     printf("- %s\n", ftsent->fts_accpath);
                     count_removed_lost_info++;
                  } else {
                     /* file's meta-info has changed, update it */
                     mi_sanitize(mi);
                     playlist_file_replace(mdb.library, idx, mi);
                     printf("u %s\n", ftsent->fts_accpath);
                     count_updated++;
                  }
               } else {
                  printf(". %s\n", ftsent->fts_accpath);
                  count_skipped_not_updated++;
               }

            } else {

               /* file does NOT exists in library database - add it */

               mi = mi_extract(ftsent->fts_accpath);

               if (mi == NULL) {
                  /* file has no info */
                  printf("s %s\n", ftsent->fts_accpath);
                  count_skipped_no_info++;
               } else {
                  /* file does have info, add it to library */
                  mi_sanitize(mi);
                  playlist_files_append(mdb.library, &mi, 1, false);
                  printf("+ %s\n", ftsent->fts_accpath);
                  count_added++;
               }
            }
      }
   }

   if (fts_close(fts) == -1)
      err(1, "medialib_db_scan_dirs: failed to close file heirarchy");

   /* save to file */
   medialib_db_save(mdb.db_file);

   /* output some of our stats */
   printf("--------------------------------------------------\n");
   printf("Results of scanning directories...\n");
   printf("(+) %9d files added\n", count_added);
   printf("(u) %9d files updated\n", count_updated);
   printf("(-) %9d files removed (was in DB, but no longer has meta-info)\n",
      count_removed_lost_info);
   printf("(.) %9d files skipped (in DB, file unchanged since last checked)\n",
      count_skipped_not_updated);
   printf("(s) %9d files skipped (no info)\n", count_skipped_no_info);
   printf("(?) %9d files skipped (other error)\n", count_skipped_error);
   printf("    %9d directories skipped (couldn't read)\n", count_skipped_dir);
}
