/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
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

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ecmd.h"
#include "meta_info.h"

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

   while ((ch = getopt(argc, argv, "a:A:t:g:T:y:c:")) != -1) {
      switch (ch) {
         case 'a':
            if ((artist = strdup(optarg)) == NULL)
               err(1, "%s: strdup ARTIST failed", argv[0]);
            break;
         case 'A':
            if ((album = strdup(optarg)) == NULL)
               err(1, "%s: strdup ALBUM failed", argv[0]);
            break;
         case 't':
            if ((title = strdup(optarg)) == NULL)
               err(1, "%s: strdup TITLE failed", argv[0]);
            break;
         case 'g':
            if ((genre = strdup(optarg)) == NULL)
               err(1, "%s: strdup GENRE failed", argv[0]);
            break;
         case 'T':
            track = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               errx(1, "%s: invalid track '%s': %s", argv[0], optarg, errstr);
            break;
         case 'y':
            year = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               errx(1, "%s: invalid year '%s': %s", argv[0], optarg, errstr);
            break;
         case 'c':
            if ((comment = strdup(optarg)) == NULL)
               err(1, "%s: strdup COMMENT failed", argv[0]);
            break;
         case 'h':
         case '?':
         default:
            return -1;
      }
   }

   return 0;
}

static void
ecmd_tag_exec(int argc, char **argv)
{
   TagLib_File *tag_file;
   TagLib_Tag  *tag;
   char **files;
   int nfiles, f;

   files  = argv + optind;
   nfiles = argc - optind;

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
   for (f = 0; f < nfiles; f++) {
      printf("tagging: '%s'\n", files[f]);

      /* extract taglib stuff */
      if ((tag_file = taglib_file_new(files[f])) == NULL) {
         warnx("TagLib: failed to open file '%s': skipping.", files[f]);
         printf("  => Causes: format not supported by TagLib or format doesn't support tags\n");
         fflush(stdout);
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
   ecmd_tag_exec
};
