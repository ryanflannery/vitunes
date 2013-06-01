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

#include <string.h>
#include <unistd.h>

#include "ecmd.h"
#include "error.h"
#include "medialib.h"
#include "vitunes.h"
#include "xmalloc.h"

static char *time_format = "%Y %m %d %H:%M:%S";

static int
ecmd_flush_parse(int argc, char **argv)
{
   int ch;

   while ((ch = getopt(argc, argv, "t:")) != -1) {
      switch (ch) {
         case 't':
            time_format = xstrdup(optarg);
            break;
         case '?':
         case 'h':
         default:
            return -1;
      }
   }

   return 0;
}

static void
ecmd_flush_exec(UNUSED struct ecmd_args *args)
{
   medialib_load(db_file, playlist_dir);
   medialib_db_flush(stdout, time_format);
   medialib_destroy();
}

const struct ecmd ecmd_flush = {
   "flush", NULL,
   "[-t format]",
   "t:",
   0, 0,
   ecmd_flush_parse,
   NULL,
   ecmd_flush_exec
};
