/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
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

/*
 * TODO Maintain an index of the filenames of each meta_info in the db
 * and provide ability to bin-search against that when loading
 * playlists from file.  As-is, really large playlists can take a
 * while.  Also could be used in medialib_scan_dirs when checking if file
 * already exists in database.  Possibly elsewhere too.
 */

#ifndef MEDIALIB_H
#define MEDIALIB_H

#include "playlist.h"

#define MEDIALIB_PLAYLISTS_CHUNK_SIZE  100

/* current database file-format version */
#define DB_VERSION_MAJOR   2
#define DB_VERSION_MINOR   1
#define DB_VERSION_OTHER   0

typedef struct {
   /* some locations of where things are loaded/saved */
   char     *db_file;      /* file containing the database */
   char     *playlist_dir; /* directory where playlists are stored */

   /* pseudo-playlists */
   playlist *library;         /* playlist representing the database */
   playlist *filter_results;  /* playlist representing results of a filter */
      /*
       * NOTE: these also exist as the first two members of the playlists
       * array below (this makes searching playlists, displaying, etc.
       * easier)
       */

   /* the playlists */
   playlist **playlists;            /* array of all playlists */
   int        nplaylists;           /* num playlists in array */
   int        playlists_capacity;   /* total size of playlists array */

} medialib;


/* the global medialib object used throughout vitunes */
extern medialib mdb;


/* load/free the global media library */
void medialib_load(const char *db_file, const char *playlist_dir);
void medialib_destroy();

/* add/remove playlists to/from the global media library */
void medialib_playlist_add(playlist *p);
void medialib_playlist_remove(int pindex);

/* create all the necessary files/directories for vitunes medialib */
void medialib_setup_files(const char *vitunes_dir, const char *db_file,
   const char *playlist_dir);

/* load/save the core database from/to disk */
void medialib_db_load(const char *db_file);
void medialib_db_save(const char *db_file);

/* update/add files to the database */
void medialib_db_update(bool show_skipped, bool force_update);
void medialib_db_scan_dirs(char *dirlist[]);

/* debug routine for dumping db contents to stdout */
void medialib_db_flush(FILE *f, const char *time_fmt);

#endif
