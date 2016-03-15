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

#include "dparray.h"

#include <err.h>

void
_dparray_increase_capacity(dparray *d)
{
   void **new_records;
   size_t nbytes;

   d->capacity += d->chunk_size;
   nbytes = d->capacity * sizeof(void*);
   if (NULL == (new_records = (void**) realloc(d->records, nbytes)))
      err(1, "%s: realloc failed", __FUNCTION__);

   d->records = new_records;
}

dparray*
dparray_new(size_t chunksz)
{
   dparray *d;
   if (NULL == (d = (dparray*) malloc(sizeof(dparray))))
      err(1, "%s: allocation failed", __FUNCTION__);

   d->records  = NULL;
   d->nrecords = 0;
   d->capacity = 0;
   d->chunk_size = chunksz;

   return d;
}

void
dparray_free(dparray *d)
{
   if (NULL != d->records) free(d->records);
   free(d);
}

dparray*
dparray_copy(dparray *d)
{
   dparray *copy = dparray_new(d->chunk_size);
   size_t   i;

   if (NULL == copy)
      return NULL;

   if (NULL == (copy->records = (void**) calloc(d->capacity, sizeof(void*))))
      err(1, "%s: calloc failed (%zu)", __FUNCTION__, d->capacity);

   for (i = 0; i < d->nrecords; ++i)
      copy->records[i] = d->records[i];

   copy->nrecords = d->nrecords;
   copy->capacity = d->capacity;

   return copy;
}

void
dparray_append_record(dparray *d, void *record)
{
   dparray_insert_records(d, d->nrecords, &record, 1);
}

void
dparray_insert_records(dparray *d, size_t idx, void **data, size_t ndata)
{
   size_t i;

   if (idx > d->nrecords)
      errx(1, "%s: idx %zu too big (max %zu)", __FUNCTION__, idx, d->nrecords);

   while (d->capacity <= d->nrecords + ndata)
      _dparray_increase_capacity(d);

   /* make space from idx to (idx + ndata) - push everything back */
   for (i = d->nrecords + ndata; i > idx && i >= ndata; i--)
      d->records[i] = d->records[i - ndata];

   /* insert the data in that new space */
   for (i = 0; i < ndata; i++)
      d->records[idx + i] = data[i];

   d->nrecords += ndata;
}

void
dparray_remove_records(dparray *d, size_t idx, size_t n)
{
   size_t i;

   if (idx > d->nrecords)
      errx(1, "%s: idx %zu too big (max %zu)", __FUNCTION__, idx, d->nrecords);

   for (i = idx; i < d->nrecords; ++i)
      d->records[i] = d->records[i + n];

   d->nrecords -= n;
}

void
dparray_replace_record(dparray *d, size_t idx, void *new_record)
{
   if (idx > d->nrecords)
      errx(1, "%s: idx %zu too big (max %zu)", __FUNCTION__, idx, d->nrecords);

   d->records[idx] = new_record;
}
