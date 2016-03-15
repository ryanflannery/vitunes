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

#ifndef PLIST_H
#define PLIST_H

#include "../mfile/mfile.h"
#include "../util/dparray.h"

#include <stdbool.h>

typedef struct
{
   char  *filename;  /* realpath(3) of the file containing the plist */
   char  *name;      /* name of the playlist */

   dparray *mfiles;

} plist;

/* allocate and return a new, empty plist */
plist*
plist_new();

/* free a plist and all of it's contents */
void
plist_free(plist *p);

/* copy a plist, overriding it's name & filename with the provided values */
plist*
plist_copy(const plist *p, const char *newFilename, const char *newName);

/* add n mfiles m into plist p starting at index idx */
void
plist_add_files(plist *p, size_t idx, mfile **m, size_t n);

/* remove n mfiles from plist p starting at index idx */
void
plist_remove_files(plist *p, size_t idx, size_t n);

/* replace the file at index idx in playlist p with m */
void
plist_replace_file(plist *p, size_t idx, mfile* m);

/* dump a plist to a file stream */
void
plist_fwrite(const plist *p, FILE *fout, unsigned int indent_level);

#endif
