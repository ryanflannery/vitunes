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

#include "mfile.h"

#include <stdlib.h>
#include <err.h>


mfile*
mfile_new()
{

   mfile *m;

   if (NULL == (m = (mfile*) malloc(sizeof(mfile))))
      err(1, "%s: malloc failed", __FUNCTION__);

   m->filename    = NULL;
   m->artist      = NULL;
   m->album       = NULL;
   m->title       = NULL;
   m->comment     = NULL;
   m->genre       = NULL;
   m->year        = 0;
   m->track       = 0;
   m->length      = 0;
   m->bitrate     = 0;
   m->samplerate  = 0;
   m->channels    = 0;
   m->last_update = 0;
   m->is_url      = false;

   return m;
}

void
mfile_free(mfile* m)
{
   if (m->filename)  free (m->filename);
   if (m->artist)    free(m->artist);
   if (m->album)     free(m->album);
   if (m->title)     free(m->title);
   if (m->comment)   free(m->comment);
   if (m->genre)     free(m->genre);
   free(m);
}

