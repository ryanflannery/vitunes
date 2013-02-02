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
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "meta_info.h"

void
ecmd_tag(int argc, char *argv[])
{
   TagLib_File *tag_file;
   TagLib_Tag  *tag;
   bool  set_artist  = false;
   bool  set_album   = false;
   bool  set_title   = false;
   bool  set_genre   = false;
   bool  set_track   = false;
   bool  set_year    = false;
   bool  set_comment = false;
   char *artist = NULL, *album = NULL, *title = NULL, *genre = NULL,
        *comment = NULL;
   const char *errstr = NULL;
   unsigned int track = 0, year = 0;
   char **files;
   int ch, nfiles, f;

   static struct option longopts[] = {
      { "artist",  required_argument, NULL, 'a' },
      { "album",   required_argument, NULL, 'A' },
      { "title",   required_argument, NULL, 't' },
      { "genre",   required_argument, NULL, 'g' },
      { "track",   required_argument, NULL, 'T' },
      { "year",    required_argument, NULL, 'y' },
      { "comment", required_argument, NULL, 'c' },
      { NULL,     0,                 NULL,  0  }
   };

   /* parse options and get list of files */
   optreset = 1;
   optind = 0;
   while ((ch = getopt_long_only(argc, argv, "a:A:t:g:T:y:c:", longopts, NULL)) != -1) {
      switch (ch) {
         case 'a':
            set_artist = true;
            if ((artist = strdup(optarg)) == NULL)
               err(1, "%s: strdup ARTIST failed", argv[0]);
            break;
         case 'A':
            set_album = true;
            if ((album = strdup(optarg)) == NULL)
               err(1, "%s: strdup ALBUM failed", argv[0]);
            break;
         case 't':
            set_title = true;
            if ((title = strdup(optarg)) == NULL)
               err(1, "%s: strdup TITLE failed", argv[0]);
            break;
         case 'g':
            set_genre = true;
            if ((genre = strdup(optarg)) == NULL)
               err(1, "%s: strdup GENRE failed", argv[0]);
            break;
         case 'T':
            set_track = true;
            track = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               errx(1, "%s: invalid track '%s': %s", argv[0], optarg, errstr);
            break;
         case 'y':
            set_year = true;
            year = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               errx(1, "%s: invalid year '%s': %s", argv[0], optarg, errstr);
            break;
         case 'c':
            set_comment = true;
            if ((comment = strdup(optarg)) == NULL)
               err(1, "%s: strdup COMMENT failed", argv[0]);
            break;
         case 'h':
         case '?':
         default:
            errx(1, "usage: see 'vitunes -e help tag'");
      }
   }
   files  = argv + optind;
   nfiles = argc - optind;

   /* be verbose, indicate what we're setting... */
   printf("Setting the following tags to all files:\n");
   if (set_artist) printf("%10.10s => '%s'\n", "artist", artist);
   if (set_album) printf("%10.10s => '%s'\n", "album", album);
   if (set_title) printf("%10.10s => '%s'\n", "title", title);
   if (set_genre) printf("%10.10s => '%s'\n", "genre", genre);
   if (set_track) printf("%10.10s => %u\n", "track", track);
   if (set_year) printf("%10.10s => %u\n", "year", year);
   if (set_comment) printf("%10.10s => '%s'\n", "comment", comment);

   if (!set_artist && !set_album && !set_title && !set_genre
   &&  !set_track && !set_year && !set_comment)
      errx(1, "%s: nothing to set!  See 'vitunes -e help tag'", argv[0]);

   if (nfiles == 0)
      errx(1, "%s: must provide at least one file to tag.", argv[0]);

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
      if (set_artist) taglib_tag_set_artist(tag, artist);
      if (set_album) taglib_tag_set_album(tag, album);
      if (set_title) taglib_tag_set_title(tag, title);
      if (set_genre) taglib_tag_set_genre(tag, genre);
      if (set_track) taglib_tag_set_track(tag, track);
      if (set_year) taglib_tag_set_year(tag, year);
      if (set_comment) taglib_tag_set_comment(tag, comment);


      /* save changes and cleanup */
      taglib_file_save(tag_file);
      taglib_tag_free_strings();
      taglib_file_free(tag_file);
   }
}
