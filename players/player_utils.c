/*
 * Copyright (c) 2011 Ryan Flannery <ryan.flannery@gmail.com>
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "compat.h"
#include "player_utils.h"

bool
exe_in_path(const char *e) 
{
   char *path, *path_copy, *part, *test;
   bool  found;

   if ((path = strdup(getenv("PATH"))) == NULL)
      err(1, "%s: strdup/getenv failed for $PATH", __FUNCTION__);

   path_copy = path;
   found = false;
   while ((part = strsep(&path, ":")) != NULL && !found) {
      if (strlen(part) == 0) continue;

      if (asprintf(&test, "%s/%s", part, e) == -1)
         err(1, "%s: failed to build path", __FUNCTION__);

      if (access(test, X_OK) == 0)
         found = true;

      free(test);
   }

   free(path_copy);
   return found;
}

