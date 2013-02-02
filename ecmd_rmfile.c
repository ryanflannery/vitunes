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
#include <stdbool.h>
#include <string.h>

#include "ecmd.h"
#include "medialib.h"
#include "playlist.h"
#include "vitunes.h"

static void
ecmd_rmfile_func(int argc, char *argv[])
{
   char *filename;
   char  input[255];
   bool  forced;
   bool  found;
   int   found_idx;
   int   i;

   if (argc < 2 || argc > 3)
      errx(1, "usage: -e %s [-f] URL|path", argv[0]);

   /* get filename and if this remove is forced */
   forced = false;
   if (argc == 3) {
      if (strcmp(argv[1], "-f") != 0)
         errx(1, "usage: -e %s [-f] URL|path", argv[0]);
      else
         forced = true;

      filename = argv[2];
   } else
      filename = argv[1];


   /* load database and search for record */
   medialib_load(db_file, playlist_dir);
   found = false;
   found_idx = -1;
   for (i = 0; i < mdb.library->nfiles && !found; i++) {
      if (strcmp(filename, mdb.library->files[i]->filename) == 0) {
         found = true;
         found_idx = i;
      }
   }

   /* if not found then error */
   if (!found) {
      i = (forced ? 0 : 1);
      errx(i, "%s: %s: No such file or URL", argv[0], filename);
   }

   /* if not forced, prompt user if they are sure */
   if (!forced) {
      printf("Are you sure you want to delete '%s'? [y/n] ", filename);
      if (fgets(input, sizeof(input), stdin) == NULL
      || (strcasecmp(input, "yes\n") != 0 && strcasecmp(input, "y\n") != 0))
         errx(1, "%s: operation canceled.  Database unchanged.", argv[0]);
   }

   playlist_files_remove(mdb.library, found_idx, 1, false);
   medialib_db_save(db_file);
   medialib_destroy();
}

const struct ecmd ecmd_rmfile = {
   "rmfile",
   ecmd_rmfile_func
};

const struct ecmd ecmd_rm = {
   "rm",
   ecmd_rmfile_func
};
