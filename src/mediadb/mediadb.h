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

#ifndef MEDIADB_H
#define MEDIADB_H

#include <sqlite3.h>
#include <stdbool.h>

typedef struct
{
   char     *dbfile;
   sqlite3  *dbhandle;

} mediadb;

/*
 * Load a new mediadb object from the database file specified
 */
mediadb*
mdb_init(const char *db);

mediadb*
mdb_open(const char *dbfile);

void
mdb_close(mediadb *mdb);

bool
mdb_scan_file(mediadb *mdb, const char *file);

void
mdb_scan_dirs(mediadb *mdb, char *dirs[]);

void
mdb_rescan_files(mediadb *mdb);

/*
void
mdb_get_playlists(mediadb *mdb, plist **playlists, size_t *nplaylists);

plist*
mdb_get_playlist(mediadb *mdb, const char *name);

void
mdb_add_empty_playlist(mediadb *mdb, const char *name);

void
mdb_remove_playlist(mediadb *mdb, const char *name);
*/

#endif
