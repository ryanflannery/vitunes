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

#include "ecmd.h"
#include "medialib.h"
#include "meta_info.h"
#include "playlist.h"
#include "vitunes.h"

static void
ecmd_addurl_exec(UNUSED int argc, char **argv)
{
   meta_info   *m;
   bool         found;
   int          found_idx;
   char         input[255];
   int          field, i;

   /* start new record, set filename */
   m = mi_new();
   m->is_url = true;
   if ((m->filename = strdup(argv[0])) == NULL)
      err(1, "%s: strdup failed (filename)", argv[0]);

   /* get fields from user */
   for (field = 0; field < MI_NUM_CINFO; field++) {

      printf("%10.10s: ", MI_CINFO_NAMES[field]);
      if (fgets(input, sizeof(input), stdin) == NULL) {
         warnx("Operation canceled. Database unchanged.");
         mi_free(m);
         return;
      }

      if (input[strlen(input) - 1] == '\n')
         input[strlen(input) - 1] = '\0';

      if ((m->cinfo[field] = strdup(input)) == NULL)
         err(1, "%s: strdup failed (field)", argv[0]);
   }

   /* load existing database and see if file/URL already exists */
   medialib_load(db_file, playlist_dir);

   /* does the URL already exist in the database? */
   found = false;
   found_idx = -1;
   for (i = 0; i < mdb.library->nfiles && !found; i++) {
      if (strcmp(m->filename, mdb.library->files[i]->filename) == 0) {
         found = true;
         found_idx = i;
      }
   }

   if (found) {
      printf("Warning: file/URL '%s' already in the database.\n", argv[0]);
      printf("Do you want to replace the existing record? [y/n] ");

      if (fgets(input, sizeof(input), stdin) == NULL
      || (strcasecmp(input, "yes\n") != 0 && strcasecmp(input, "y\n") != 0)) {
         warnx("Operation Canceled.  Database unchanged.");
         mi_free(m);
         medialib_destroy();
         return;
      }

      mi_sanitize(m);
      playlist_file_replace(mdb.library, found_idx, m);
   } else {
      mi_sanitize(m);
      playlist_files_append(mdb.library, &m, 1, false);
   }

   medialib_db_save(db_file);
   medialib_destroy();
}

const struct ecmd ecmd_addurl = {
   "addurl", NULL,
   "URL|path",
   1, 1,
   NULL,
   ecmd_addurl_exec
};
