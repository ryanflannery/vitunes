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

#include "plist.h"

#include "../util/indent.h"
#include <stdlib.h>
#include <string.h>
#include <err.h>

void
plist_increase_capacity(plist *p)
{
   static const size_t PLIST_CHUNK_SIZE = 100; /* subject to tweaking! */
   mfile **new_mfiles;
   size_t  nbytes;

   p->capacity += PLIST_CHUNK_SIZE;
   nbytes = p->capacity * sizeof(mfile*);
   if (NULL == (new_mfiles = (mfile**) realloc(p->mfiles, nbytes)))
      err(1, "%s: failed to realloc mfiles", __FUNCTION__);

   p->mfiles = new_mfiles;
}

plist*
plist_new()
{
   plist *p;

   if (NULL == (p = (plist*) malloc(sizeof(plist))))
      err(1, "%s: malloc failed", __FUNCTION__);

   p->filename = NULL;
   p->name     = NULL;
   p->mfiles   = NULL;
   p->nfiles   = 0;
   p->capacity = 0;

   return p;
}

void
plist_free(plist *p)
{
   if (NULL != p->filename) free(p->filename);
   if (NULL != p->name)     free(p->name);
   if (NULL != p->mfiles)   free(p->mfiles);
   free(p);
}

plist*
plist_copy(const plist *p, const char *newFilename, const char *newName)
{
   size_t i;
   plist *copy = plist_new();

   if (NULL != newFilename)
   {
      if (NULL == (copy->filename = strdup(newFilename)))
         err(1, "%s: strdup filename failed", __FUNCTION__);
   }

   if (NULL != newName)
   {
      if (NULL == (copy->name = strdup(newName)))
         err(1, "%s: strdup name failed", __FUNCTION__);
   }

   if (NULL == (copy->mfiles = (mfile**) calloc(p->capacity, sizeof(mfile*))))
      err(1, "%s: failed to allocate files", __FUNCTION__);

   for (i = 0; i < p->nfiles; ++i)
      copy->mfiles[i] = p->mfiles[i];

   copy->nfiles   = p->nfiles;
   copy->capacity = p->capacity;

   return copy;
}

void
plist_add_files(plist *p, size_t idx, mfile **m, size_t n)
{
   size_t i;

   if (idx > p->nfiles)
      errx(1, "%s: index %zu too big (max %zu)", __FUNCTION__, idx, p->nfiles);

   while (p->capacity <= p->nfiles + n)
      plist_increase_capacity(p);

   /* push everything after idx back n places */
   for (i = p->nfiles + n; i > idx && i >= n; i--)
      p->mfiles[i] = p->mfiles[i - n];

   /* insert the files */
   for (i = 0; i < n; i++)
      p->mfiles[idx + i] = m[i];

   p->nfiles += n;
}

void
plist_remove_files(plist *p, size_t idx, size_t n)
{
   size_t i;

   if (idx > p->nfiles)
      errx(1, "%s: index %zu too big (max %zu)", __FUNCTION__, idx, p->nfiles);

   for (i = idx; i < p->nfiles; ++i)
      p->mfiles[i] = p->mfiles[i + n];

   p->nfiles -= n;
}

void
plist_replace_file(plist *p, size_t idx, mfile *m)
{
   if (idx > p->nfiles)
      errx(1, "%s: index %zu too big (max %zu)", __FUNCTION__, idx, p->nfiles);

   p->mfiles[idx] = m;
}

void
plist_fwrite(const plist *p, FILE *fout, unsigned int d)
{
   size_t i;

   indent(d);   fprintf(fout, "[PLIST] {\n");
   indent(d+1); fprintf(fout, "%s = '%s',\n", "filename", p->filename);
   indent(d+1); fprintf(fout, "%s = '%s',\n", "name", p->filename);
   indent(d+1); fprintf(fout, "%s = '%zu',\n", "capacity", p->capacity);
   indent(d+1); fprintf(fout, "%s = '%zu',\n", "nfiles", p->nfiles);
   indent(d+1); fprintf(fout, "%s = {\n", "mfiles");

   for (i = 0; i < p->nfiles; ++i)
      mfile_fwrite(p->mfiles[i], fout, d+2);

   indent(d+1); fprintf(fout, "}\n");
   indent(d);   fprintf(fout, "}\n");
}
