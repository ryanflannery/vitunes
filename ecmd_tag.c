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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tag_c.h>

#include "ecmd.h"
#include "error.h"
#include "meta_info.h"
#include "xmalloc.h"

static char         *artist;
static char         *album;
static char         *title;
static char         *genre;
static char         *comment;
static unsigned int  track;
static unsigned int  year;

static int
ecmd_tag_parse(int argc, char **argv)
{
   const char *errstr;
   int         ch;

   while ((ch = getopt(argc, argv, "A:T:a:c:g:t:y:")) != -1) {
      switch (ch) {
         case 'A':
            free(album);
            album = xstrdup(optarg);
            break;
          case 'T':
            track = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               fatalx("invalid track '%s': %s", optarg, errstr);
            break;
         case 'a':
            free(artist);
            artist = xstrdup(optarg);
            break;
         case 'c':
            free(comment);
            comment = xstrdup(optarg);
            break;
         case 'g':
            free(genre);
            genre = xstrdup(optarg);
            break;
         case 't':
            free(title);
            title = xstrdup(optarg);
            break;
         case 'y':
            year = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               fatalx("invalid year '%s': %s", optarg, errstr);
            break;
         case 'h':
         case '?':
         default:
            return -1;
      }
   }

   return 0;
}

static int
ecmd_tag_check(void)
{
   if (artist == NULL && album == NULL && title == NULL && genre == NULL
   &&  track == 0 && year == 0 && comment == NULL)
      return -1;

   return 0;
}

static void
ecmd_tag_exec(int argc, char **argv)
{
   TagLib_File *tag_file;
   TagLib_Tag  *tag;
   int          i;

   /* be verbose, indicate what we're setting... */
   printf("Setting the following tags to all files:\n");
   if (artist != NULL) printf("%10.10s => '%s'\n", "artist", artist);
   if (album != NULL) printf("%10.10s => '%s'\n", "album", album);
   if (title != NULL) printf("%10.10s => '%s'\n", "title", title);
   if (genre != NULL ) printf("%10.10s => '%s'\n", "genre", genre);
   if (track) printf("%10.10s => %u\n", "track", track);
   if (year) printf("%10.10s => %u\n", "year", year);
   if (comment != NULL) printf("%10.10s => '%s'\n", "comment", comment);

   /* tag files ... */
   taglib_set_strings_unicode(false);
   for (i = 0; i < argc; i++) {
      printf("tagging: '%s'\n", argv[i]);

      /* extract taglib stuff */
      if ((tag_file = taglib_file_new(argv[i])) == NULL) {
         infox("TagLib: failed to open file '%s': skipping.", argv[i]);
         infox("  => Causes: format not supported by TagLib or format doesn't support tags");
         continue;
      }

      tag = taglib_file_tag(tag_file);

      /* apply changes */
      if (artist != NULL) taglib_tag_set_artist(tag, artist);
      if (album != NULL) taglib_tag_set_album(tag, album);
      if (title != NULL) taglib_tag_set_title(tag, title);
      if (genre != NULL) taglib_tag_set_genre(tag, genre);
      if (track) taglib_tag_set_track(tag, track);
      if (year) taglib_tag_set_year(tag, year);
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
   1, -1,
   ecmd_tag_parse,
   ecmd_tag_check,
   ecmd_tag_exec
};
