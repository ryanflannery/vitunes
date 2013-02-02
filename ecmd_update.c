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

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "ecmd.h"
#include "medialib.h"
#include "vitunes.h"

static void
ecmd_update_exec(int argc, char **argv)
{
   int  ch;
   bool force_update = false;
   bool show_skipped = false;

   /* parse options */
   while ((ch = getopt(argc, argv, "fs")) != -1) {
      switch (ch) {
         case 'f':
            force_update = true;
            break;
         case 's':
            show_skipped = true;
            break;
         case 'h':
         case '?':
         default:
            errx(1, "usage: -e %s [-fs]", argv[0]);
      }
   }
   if (optind < argc)
      errx(1, "usage: -e %s [-fs]", argv[0]);

   printf("Loading existing database...\n");
   medialib_load(db_file, playlist_dir);

   printf("Updating existing database...\n");
   medialib_db_update(show_skipped, force_update);

   medialib_destroy();
}

const struct ecmd ecmd_update = {
   "update", NULL,
   "[-fs]",
   ecmd_update_exec
};
