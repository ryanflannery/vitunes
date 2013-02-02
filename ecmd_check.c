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
#include "medialib.h"
#include "vitunes.h"

static void
ecmd_check_exec(int argc, char **argv)
{
   meta_info *mi;
   bool   show_raw, show_sanitized, show_database;
   bool   found;
   char   realfile[PATH_MAX];
   char **files;
   int    nfiles;
   int    ch, f, i;

   if (argc < 3)
      errx(1, "usage: -e %s [-drs] path [...]", argv[0]);

   /* parse options to see which version to show */
   show_raw = false;
   show_sanitized = false;
   show_database = false;
   optreset = 1;
   optind = 0;
   while ((ch = getopt(argc, argv, "rsd")) != -1) {
      switch (ch) {
         case 'r':
            show_raw = true;
            break;
         case 's':
            show_sanitized = true;
            break;
         case 'd':
            show_database = true;
            break;
         case 'h':
         case '?':
         default:
            errx(1, "usage: -e %s [-drs] path [...]", argv[0]);
      }
   }
   files = argv + optind;
   nfiles = argc - optind;

   if (!show_raw && !show_sanitized && !show_database)
      errx(1, "%s: must specify at least one of -r, -s, or -d", argv[0]);

   if (nfiles == 0)
      errx(1, "%s: must provide at least one file to check.", argv[0]);

   /* scan through files... */
   for (f = 0; f < nfiles; f++) {

      printf("Checking: '%s'\n", files[f]);

      /* show raw or sanitized information */
      if (show_raw || show_sanitized) {
         mi = mi_extract(files[f]);
         if (mi == NULL)
            warnx("Failed to extract any meta-information from '%s'", files[f]);
         else {
            /* show raw info */
            if (show_raw) {
               printf("\tThe RAW meta-information from the file is:\n");
               for (i = 0; i < MI_NUM_CINFO; i++)
                  printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
            }

            /* show sanitized info */
            if (show_sanitized) {
               mi_sanitize(mi);
               printf("\tThe SANITIZED meta-information from the file is:\n");
               for (i = 0; i < MI_NUM_CINFO; i++)
                  printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
            }
         }
      }

      /* check if it's in the database */
      if (show_database) {

         /* get absolute filename */
         if (realpath(files[f], realfile) == NULL) {
            warn("%s: realpath failed for %s: skipping", argv[0], files[f]);
            continue;
         }

         /* check if file is in database */
         medialib_load(db_file, playlist_dir);

         found = false;
         mi = NULL;
         for (i = 0; i < mdb.library->nfiles && !found; i++) {
            if (strcmp(realfile, mdb.library->files[i]->filename) == 0) {
               found = true;
               mi = mdb.library->files[i];
            }
         }

         if (!found)
            warnx("File '%s' does NOT exist in the database", files[f]);
         else {
            printf("\tThe meta-information in the DATABASE is:\n");
            for (i = 0; i < MI_NUM_CINFO; i++)
               printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
         }

         medialib_destroy();
      }

   }
}

const struct ecmd ecmd_check = {
   "check",
   ecmd_check_exec
};
