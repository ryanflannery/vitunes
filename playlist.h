/*
 * Copyright (c) 2010 Ryan Flannery <ryan.flannery@gmail.com>
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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <glob.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "meta_info.h"

#define PLAYLIST_CHUNK_SIZE   100

/* the core playlist structure */
typedef struct
{
   char  *filename;     /* filename containing the playlist */
   char  *name;         /* name of the playlist used in display */
   bool   needs_saving; /* does this playlist have unsaved changes? */

   /* the array of files (their meta information) in the playlist */
   meta_info **files;
   int         nfiles;     /* number of files in the playlist */
   int         capacity;   /* current size malloc()'d for the files */

} playlist;

/*
 * IMPORTANT NOTES ABOUT THE "playlist" STRUCTURE:
 * 1. The elements of the "files" array are simply pointers to the
 *    already existing meta-info elements in the media database.
 *
 * 2. When loading a playlist from a file, each element of the playlist
 *    is compared against the media database to find a corresponding entry.
 *    If no such file exists in the media database, a new record is added
 *    to the DB but it contains *only* the filename read from the playlist
 *    file (no meta info).
 *
 * 3. The media database mentioned above is simply an array of meta_info
 *    structs, passed to the functions when necessary below.
 */

/* create/destroy/duplicate playlist structs */
playlist *playlist_new(void);
void playlist_free(playlist *p);
playlist *playlist_dup(const playlist *original, const char *filename,
                       const char* name);

/* remove a file from a playlist */
void playlist_file_add(playlist *p, meta_info *f, int index);
void playlist_file_append(playlist *p, meta_info *f);
void playlist_file_remove(playlist *p, int index);
void playlist_file_replace(playlist *p, int index, meta_info *newEntry);

/* load/save/delete playlists from/to/from filesystem */
playlist *playlist_load(const char *filename, meta_info **db, int ndb);
void playlist_save(const playlist *p);
void playlist_delete(playlist *p);

/* filter a playlist to all records matching/not-matching a given string */
playlist *playlist_filter(const playlist *p, bool m);

/* retrieve all playlist files in a given directory and return number found */
int retrieve_playlist_filenames(const char *dirname, char ***files);

#endif
