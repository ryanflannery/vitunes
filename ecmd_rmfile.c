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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ecmd.h"
#include "error.h"
#include "medialib.h"
#include "playlist.h"
#include "vitunes.h"

static bool forced;

static int
ecmd_rmfile_parse(int argc, char **argv)
{
   int ch;

   while ((ch = getopt(argc, argv, "f")) != -1) {
      switch (ch) {
         case 'f':
            forced = true;
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
ecmd_rmfile_exec(struct ecmd_args *args)
{
   char  input[255];
   bool  found;
   int   found_idx;
   int   i;

   /* load database and search for record */
   medialib_load(db_file, playlist_dir);
   found = false;
   found_idx = -1;
   for (i = 0; i < mdb.library->nfiles && !found; i++) {
      if (strcmp(args->argv[0], mdb.library->files[i]->filename) == 0) {
         found = true;
         found_idx = i;
      }
   }

   /* if not found then error */
   if (!found) {
      i = (forced ? 0 : 1);
      infox("%s: No such file or URL", args->argv[0]);
      exit(i);
   }

   /* if not forced, prompt user if they are sure */
   if (!forced) {
      printf("Are you sure you want to delete '%s'? [y/n] ", args->argv[0]);
      if (fgets(input, sizeof(input), stdin) == NULL
      || (strcasecmp(input, "yes\n") != 0 && strcasecmp(input, "y\n") != 0))
         fatalx("Operation canceled.  Database unchanged.");
   }

   playlist_files_remove(mdb.library, found_idx, 1, false);
   medialib_db_save(db_file);
   medialib_destroy();
}

const struct ecmd ecmd_rmfile = {
   "rmfile", "rm",
   "[-f] URL|path",
   "f",
   1, 1,
   ecmd_rmfile_parse,
   NULL,
   ecmd_rmfile_exec
};
