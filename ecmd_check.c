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

static bool show_raw;
static bool show_sanitized;
static bool show_database;

static void
ecmd_check_show_db(const char *path)
{
   bool       found;
   char       realfile[PATH_MAX];
   int        i;
   meta_info *mi;

   if (show_database == false)
      return;
   if (realpath(path, realfile) == NULL) {
      warn("realpath failed for %s: skipping", path);
      return;
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
      warnx("File '%s' does NOT exist in the database", path);
   else {
      printf("\tThe meta-information in the DATABASE is:\n");
      for (i = 0; i < MI_NUM_CINFO; i++)
         printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
   }

   medialib_destroy();
}

static void
ecmd_check_show_raw(const char *path)
{
   int        i;
   meta_info *mi;

   if (show_raw == false)
      return;
   if ((mi = mi_extract(path)) == NULL) {
      warnx("Failed to extract any meta-information from '%s'", path);
      return;
   }

   printf("\tThe RAW meta-information from the file is:\n");
   for (i = 0; i < MI_NUM_CINFO; i++)
      printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
}

static void
ecmd_check_show_sanitized(const char *path)
{
   int        i;
   meta_info *mi;

   if (show_sanitized == false)
      return;
   if ((mi = mi_extract(path)) == NULL) {
      warnx("Failed to extract any meta-information from '%s'", path);
      return;
   }

   mi_sanitize(mi);
   printf("\tThe SANITIZED meta-information from the file is:\n");
   for (i = 0; i < MI_NUM_CINFO; i++)
      printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
}

static int
ecmd_check_parse(int argc, char **argv)
{
   int ch;

   while ((ch = getopt(argc, argv, "drs")) != -1) {
      switch (ch) {
         case 'd':
            show_database = true;
            break;
         case 'r':
            show_raw = true;
            break;
         case 's':
            show_sanitized = true;
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
ecmd_check_check(void)
{
   if (!show_raw && !show_sanitized && !show_database)
      return -1;
   return 0;
}

static void
ecmd_check_exec(int argc, char **argv)
{
   int i;

   /* scan through files... */
   for (i = 0; i < argc; i++) {
      printf("Checking: '%s'\n", argv[i]);
      ecmd_check_show_raw(argv[i]);
      ecmd_check_show_sanitized(argv[i]);
      ecmd_check_show_db(argv[i]);
   }
}

const struct ecmd ecmd_check = {
   "check", NULL,
   "-d | -r | -s path [...]",
   1, -1,
   ecmd_check_parse,
   ecmd_check_check,
   ecmd_check_exec
};
