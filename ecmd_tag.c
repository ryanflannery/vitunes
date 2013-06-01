/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
 * Copyright (c) 2013 Tiago Cunha <tcunha@gmx.com>
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

#include <limits.h>
#include <stdio.h>
#include <tag_c.h>

#include "ecmd.h"
#include "error.h"
#include "meta_info.h"
#include "xmalloc.h"

static int
ecmd_tag_check(struct ecmd_args *args)
{
   return ecmd_args_empty(args) == 0 ? 0 : -1;
}

static void
ecmd_tag_exec(struct ecmd_args *args)
{
   TagLib_File *tag_file;
   TagLib_Tag  *tag;
   const char  *album, *artist, *comment, *genre, *title;
   int          i, track, year;

   /* be verbose, indicate what we're setting... */
   printf("Setting the following tags to all files:\n");
   if ((artist = ecmd_args_get(args, 'a')) != NULL)
      printf("%10.10s => '%s'\n", "artist", artist);
   if ((album = ecmd_args_get(args, 'A')) != NULL)
      printf("%10.10s => '%s'\n", "album", album);
   if ((title = ecmd_args_get(args, 't')) != NULL)
      printf("%10.10s => '%s'\n", "title", title);
   if ((genre = ecmd_args_get(args, 'g')) != NULL)
      printf("%10.10s => '%s'\n", "genre", genre);
   if ((track = ecmd_args_strtonum(args, 'T', 0, INT_MAX)) != -1)
      printf("%10.10s => %u\n", "track", track);
   if ((year = ecmd_args_strtonum(args, 'y', 0, INT_MAX)) != -1)
      printf("%10.10s => %u\n", "year", year);
   if ((comment = ecmd_args_get(args, 'c')) != NULL)
      printf("%10.10s => '%s'\n", "comment", comment);

   /* tag files ... */
   taglib_set_strings_unicode(false);
   for (i = 0; i < args->argc; i++) {
      printf("tagging: '%s'\n", args->argv[i]);

      /* extract taglib stuff */
      if ((tag_file = taglib_file_new(args->argv[i])) == NULL) {
         infox("TagLib: failed to open file '%s': skipping.", args->argv[i]);
         infox("  => Causes: format not supported by TagLib or format doesn't support tags");
         continue;
      }

      tag = taglib_file_tag(tag_file);

      /* apply changes */
      if (artist != NULL) taglib_tag_set_artist(tag, artist);
      if (album != NULL) taglib_tag_set_album(tag, album);
      if (title != NULL) taglib_tag_set_title(tag, title);
      if (genre != NULL) taglib_tag_set_genre(tag, genre);
      if (track != -1) taglib_tag_set_track(tag, track);
      if (year != -1) taglib_tag_set_year(tag, year);
      if (comment != NULL) taglib_tag_set_comment(tag, comment);


      /* save changes and cleanup */
      taglib_file_save(tag_file);
      taglib_tag_free_strings();
      taglib_file_free(tag_file);
   }
}

const struct ecmd ecmd_tag = {
   "tag", NULL,
   "[-A album] [-T track] [-a artist] [-c comment] [-g genre] [-t title]\n\
   \t[-y year] path [...]",
   "A:T:a:c:g:t:y:",
   1, -1,
   ecmd_tag_check,
   ecmd_tag_exec
};
