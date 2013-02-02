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

#include "ecmd.h"
#include "medialib.h"
#include "vitunes.h"

static void
ecmd_add_exec(int argc, char **argv)
{
   if (argc == 1)
      errx(1, "usage: -e %s path [...]", argv[0]);

   printf("Loading existing database...\n");
   medialib_load(db_file, playlist_dir);

   printf("Scanning directories for files to add to database...\n");
   medialib_db_scan_dirs(argv + 1);

   medialib_destroy();
}

const struct ecmd ecmd_add = {
   "add", NULL,
   "path [...]",
   ecmd_add_exec
};
