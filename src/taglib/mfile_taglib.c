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

#include "mfile_taglib.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <tag_c.h>

mfile*
taglib_extract(const char* file)
{
   char actual_path[PATH_MAX];
   const TagLib_AudioProperties *tlibAP;
   TagLib_File *tlibFile;
   TagLib_Tag  *tlibTag;

   bzero(actual_path, sizeof(actual_path));
   if (NULL == realpath(file, actual_path))
      err(1, "%s: realpath(3) failed to resolve '%s'", __FUNCTION__, file);

   /* use TagLib(3) to extract info */
   if (NULL == (tlibFile = taglib_file_new(actual_path)))
      return NULL;

   if (NULL == (tlibTag = taglib_file_tag(tlibFile)))
      return NULL;

   if (NULL == (tlibAP = taglib_file_audioproperties(tlibFile)))
      return NULL;

   /* reached here? => have a valid media file, extract info to an mfile */
   mfile *m = mfile_new();
   char *str;

   if (str = taglib_tag_artist(tlibTag))  m->artist  = strdup(str);
   if (str = taglib_tag_album(tlibTag))   m->album   = strdup(str);
   if (str = taglib_tag_title(tlibTag))   m->title   = strdup(str);
   if (str = taglib_tag_comment(tlibTag)) m->comment = strdup(str);
   if (str = taglib_tag_genre(tlibTag))   m->genre   = strdup(str);

   m->year     = taglib_tag_year(tlibTag);
   m->track    = taglib_tag_track(tlibTag);
   m->length   = taglib_audioproperties_length(tlibAP);
   m->bitrate  = taglib_audioproperties_bitrate(tlibAP);
   m->channels = taglib_audioproperties_channels(tlibAP);
   m->samplerate = taglib_audioproperties_samplerate(tlibAP);

   //last_update;
   m->is_url = false;

   /* free TagLib(3) stuff */
   taglib_tag_free_strings();
   taglib_file_free(tlibFile);
}

bool
taglib_save_tags(const mfile* mfile)
{
}

