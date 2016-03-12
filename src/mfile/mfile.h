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

#ifndef MFILE_H
#define MFILE_H

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct
{
   char  *filename;     /* realpath(3) of a file -or- url of stream */
   bool   is_url;       /* true IFF filename is a url */

   /* core meta info (can read and write this) */
   char  *artist;
   char  *album;
   char  *title;
   char  *comment;
   char  *genre;
   int    year;
   int    track;

   /* audio properties (read only) */
   int    length;       /* seconds */
   int    bitrate;      /* in kb/s */
   int    samplerate;   /* in Hz */
   int    channels;     /* #channels in the stream */

   time_t last_update;  /* time of last file modification TODO unused */
} mfile;


/*
 * Allocate a new mfile object. All parameters are NULL/0.
 */
mfile*
mfile_new();

/*
 * free() an mfile object, include all non-NULL strings within it.
 */
void
mfile_free(mfile* m);

/*
 * Compare two mfile objects on all properties EXCEPT the filename, is_url,
 * and last_update.
 * Returns true if both are identical, false otherwise.
 * This is not a suitable comparator for any ordering. Useful only for debug.
 */
bool
mfile_cmp(const mfile *left, const mfile *right);

/*
 * Write an mfile object to the provided FILE stream. Useful for debugging.
 */
void
mfile_fwrite(const mfile *m, FILE *fout, unsigned int indent_level);

/*
 * Arbitrary constructor - used for debugging
 */
mfile*
mfile_construct(const char *artist, const char *album,
                const char *title,  const char *comment,
                const char *genre,
                int year,
                int track);

#endif
