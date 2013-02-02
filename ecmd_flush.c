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
#include <string.h>
#include <unistd.h>

#include "ecmd.h"
#include "medialib.h"
#include "vitunes.h"

static void
ecmd_flush_exec(int argc, char **argv)
{
   int   ch;
   char *time_format = "%Y %m %d %H:%M:%S";

   optreset = 1;
   optind = 0;
   while ((ch = getopt(argc, argv, "t:")) != -1) {
      switch (ch) {
         case 't':
            if ((time_format = strdup(optarg)) == NULL)
               err(1, "%s: strdup of time_format failed", argv[0]);
            break;
         case '?':
         case 'h':
         default:
            errx(1, "usage: %s [-t format]", argv[0]);
      }
   }
   
   medialib_load(db_file, playlist_dir);
   medialib_db_flush(stdout, time_format);
   medialib_destroy();
}

const struct ecmd ecmd_flush = {
   "flush",
   ecmd_flush_exec
};
