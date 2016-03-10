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
#include <string.h>
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
   if (m->filename)  free(m->filename);
   if (m->artist)    free(m->artist);
   if (m->album)     free(m->album);
   if (m->title)     free(m->title);
   if (m->comment)   free(m->comment);
   if (m->genre)     free(m->genre);
   free(m);
}

static int
strcmpWithNulls(const char *l, const char *r)
{
   if (NULL == l && NULL == r)
      return 0;
   else if (NULL == l && NULL != r)
      return -1;
   else if (NULL != l && NULL == r)
      return 1;
   else
      return strcmp(l, r);
}

bool
mfile_cmp(const mfile *left, const mfile *right)
{
   /* compare everything EXCEPT filename */
   if (strcmpWithNulls(left->artist,  right->artist)
   ||  strcmpWithNulls(left->album,   right->album)
   ||  strcmpWithNulls(left->title,   right->title)
   ||  strcmpWithNulls(left->comment, right->comment)
   ||  strcmpWithNulls(left->genre,   right->genre)
   ||  left->year       != right->year
   ||  left->track      != right->track
   ||  left->length     != right->length
   ||  left->bitrate    != right->bitrate
   ||  left->samplerate != right->samplerate
   ||  left->channels   != right->channels
   )
      return false;

   return true;
}

void
mfile_fwrite(const mfile *m, FILE *fout)
{
   fprintf(fout, "%50s = '%s'\n", "filename", m->filename);
   fprintf(fout, "%50s = '%s'\n", "artist", m->artist);
   fprintf(fout, "%50s = '%s'\n", "album", m->album);
   fprintf(fout, "%50s = '%s'\n", "title", m->title);
   fprintf(fout, "%50s = '%s'\n", "comment", m->comment);
   fprintf(fout, "%50s = '%s'\n", "genre", m->genre);
   fprintf(fout, "%50s = '%d'\n", "year", m->year);
   fprintf(fout, "%50s = '%d'\n", "track", m->track);
   fprintf(fout, "%50s = '%d'\n", "length", m->length);
   fprintf(fout, "%50s = '%d'\n", "bitrate", m->bitrate);
   fprintf(fout, "%50s = '%d'\n", "samplerate", m->samplerate);
   fprintf(fout, "%50s = '%d'\n", "channels", m->channels);
}
