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

plist*
plist_new()
{
   plist *p;

   if (NULL == (p = (plist*) malloc(sizeof(plist))))
      err(1, "%s: malloc failed", __FUNCTION__);

   p->mfiles = dparray_new(1000);
   p->filename = NULL;
   p->name     = NULL;

   return p;
}

void
plist_free(plist *p)
{
   if (NULL != p->filename) free(p->filename);
   if (NULL != p->name)     free(p->name);
   dparray_free(p->mfiles);
   free(p);
}

plist*
plist_copy(const plist *p, const char *newFilename, const char *newName)
{
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

   copy->mfiles = dparray_copy(p->mfiles);

   return copy;
}

void
plist_add_files(plist *p, size_t idx, mfile **m, size_t n)
{
   dparray_insert_records(p->mfiles, idx, (void**)m, n);
}

void
plist_remove_files(plist *p, size_t idx, size_t n)
{
   dparray_remove_records(p->mfiles, idx, n);
}

void
plist_replace_file(plist *p, size_t idx, mfile *m)
{
   dparray_replace_record(p->mfiles, idx, (void*)m);
}

void
plist_fwrite(const plist *p, FILE *fout, unsigned int d)
{
   size_t i;

   indent(d,fout);   fprintf(fout, "[PLIST] {\n");
   indent(d+1,fout); fprintf(fout, "%s = '%s',\n", "filename", p->filename);
   indent(d+1,fout); fprintf(fout, "%s = '%s',\n", "name", p->filename);
   indent(d+1,fout); fprintf(fout, "%s = '%zu',\n", "capacity", p->mfiles->capacity);
   indent(d+1,fout); fprintf(fout, "%s = '%zu',\n", "nfiles", p->mfiles->nrecords);
   indent(d+1,fout); fprintf(fout, "%s = {\n", "mfiles");

   for (i = 0; i < p->mfiles->nrecords; ++i)
      mfile_fwrite((mfile*)p->mfiles->records[i], fout, d+2);

   indent(d+1,fout); fprintf(fout, "}\n");
   indent(d,fout);   fprintf(fout, "}\n");
}
