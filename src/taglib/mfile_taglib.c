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

#include <string.h>
#include <stdio.h>
#include <err.h>

#include <tag_c.h>

mfile*
taglib_extract(const char* file)
{
   const TagLib_AudioProperties *tlibAP;
   TagLib_File *tlibFile;
   TagLib_Tag  *tlibTag;

   /* use TagLib(3) to extract info */
   if (NULL == (tlibFile = taglib_file_new(file)))
      return NULL;

   if (NULL == (tlibTag = taglib_file_tag(tlibFile)))
      return NULL;

   if (NULL == (tlibAP = taglib_file_audioproperties(tlibFile)))
      return NULL;

   /* reached here? => have a valid media file, extract info to an mfile */
   mfile *m = mfile_new();
   char  *str;

   /* copy strings */
   m->filename = strdup(file);
   if ((str = taglib_tag_artist(tlibTag)))  m->artist  = strdup(str);
   if ((str = taglib_tag_album(tlibTag)))   m->album   = strdup(str);
   if ((str = taglib_tag_title(tlibTag)))   m->title   = strdup(str);
   if ((str = taglib_tag_comment(tlibTag))) m->comment = strdup(str);
   if ((str = taglib_tag_genre(tlibTag)))   m->genre   = strdup(str);

   /* if any allocation failed, free what succeeded and fail */
   if (NULL == m->filename || NULL == m->artist  || NULL == m->album
   ||  NULL == m->title    || NULL == m->comment || NULL == m->genre)
   {
      taglib_tag_free_strings();
      taglib_file_free(tlibFile);
      mfile_free(m);
      return NULL;
   }

   /* copy non strings */
   m->year        = taglib_tag_year(tlibTag);
   m->track       = taglib_tag_track(tlibTag);
   m->length      = taglib_audioproperties_length(tlibAP);
   m->bitrate     = taglib_audioproperties_bitrate(tlibAP);
   m->channels    = taglib_audioproperties_channels(tlibAP);
   m->samplerate  = taglib_audioproperties_samplerate(tlibAP);

   /* TODO last_update; */
   m->is_url = false;

   /* free TagLib(3) stuff */
   taglib_tag_free_strings();
   taglib_file_free(tlibFile);

   return m;
}

int
taglib_save_tags(const mfile* mfile)
{
   TagLib_File *tlibFile;
   TagLib_Tag  *tlibTag;

   if (NULL == (tlibFile = taglib_file_new(mfile->filename)))
      return -1; /* TODO magic num / document in .h */

   if (NULL == (tlibTag = taglib_file_tag(tlibFile)))
      return -2; /* TODO same as above */

   /* string properties */
   if (NULL != mfile->artist) taglib_tag_set_artist(tlibTag,   mfile->artist);
   if (NULL != mfile->album)  taglib_tag_set_album(tlibTag,    mfile->album);
   if (NULL != mfile->title)  taglib_tag_set_title(tlibTag,    mfile->title);
   if (NULL != mfile->comment)taglib_tag_set_comment(tlibTag,  mfile->comment);
   if (NULL != mfile->genre)  taglib_tag_set_genre(tlibTag,    mfile->genre);

   /* non strings */
   taglib_tag_set_track(tlibTag,  mfile->track);
   taglib_tag_set_year(tlibTag,   mfile->year);

   /* save and free */
   taglib_file_save(tlibFile);
   taglib_tag_free_strings();
   taglib_file_free(tlibFile);

   return 0;
}

