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

#ifndef DPARRAY_H
#define DPARRAY_H

#include <stdlib.h>

/*
 * A simple linear chunk allocator, used exclusively for pointer arrays
 *
 * Assumptions:
 * 1. This interface manages the allocation of the object and list within
 * 2. It does NOT manage the stuff pointed to by elements of the list
 * 3. Thus it should only be used when that stuff is managed elsewhere
 * 4. Any out-o-bounds errors on the part of the caller result in exit(1)
 * 5. Any allocation failures result in exit(1)
 */

typedef struct
{
   void  **records;  /* array of void* records (each managed by caller) */
   size_t  nrecords; /* number of used elements in the array */
   size_t  capacity; /* capacity of array, in # of elements */

   size_t chunk_size;
} dparray;

dparray*
dparray_new(size_t chunksz);

void
dparray_free(dparray *d);

dparray*
dparray_copy(dparray *d);

void
dparray_append_record(dparray *d, void *record);

void
dparray_insert_records(dparray *d, size_t idx, void **data, size_t ndata);

void
dparray_remove_records(dparray *d, size_t idx, size_t n);

void
dparray_replace_record(dparray *d, size_t idx, void *new_record);

#endif
