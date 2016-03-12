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

#include "util/indent.h"
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
mfile_fwrite(const mfile *m, FILE *fout, unsigned int d)
{
   indent(d);   fprintf(fout, "[MFILE] {\n");
   indent(d+1); fprintf(fout, "%s = '%s',\n", "filename", m->filename);
   indent(d+1); fprintf(fout, "%s = '%s',\n", "artist", m->artist);
   indent(d+1); fprintf(fout, "%s = '%s',\n", "album", m->album);
   indent(d+1); fprintf(fout, "%s = '%s',\n", "title", m->title);
   indent(d+1); fprintf(fout, "%s = '%s',\n", "comment", m->comment);
   indent(d+1); fprintf(fout, "%s = '%s',\n", "genre", m->genre);
   indent(d+1); fprintf(fout, "%s = '%d',\n", "year", m->year);
   indent(d+1); fprintf(fout, "%s = '%d',\n", "track", m->track);
   indent(d+1); fprintf(fout, "%s = '%d',\n", "length", m->length);
   indent(d+1); fprintf(fout, "%s = '%d',\n", "bitrate", m->bitrate);
   indent(d+1); fprintf(fout, "%s = '%d',\n", "samplerate", m->samplerate);
   indent(d+1); fprintf(fout, "%s = '%d'\n", "channels", m->channels);
   indent(d);   fprintf(fout, "}\n");
}

mfile*
mfile_construct(const char *artist, const char *album,
                const char *title,  const char *comment,
                const char *genre,
                int year,
                int track)
{
   mfile *m = mfile_new();

   m->artist   = strdup(artist);
   m->album    = strdup(album);
   m->title    = strdup(title);
   m->comment  = strdup(comment);
   m->genre    = strdup(genre);
   m->track    = track;
   m->year     = year;

   return m;
}
