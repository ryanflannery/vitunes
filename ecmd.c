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

#include "ecmd.h"

const struct ecmd ECMD_PATH[] = { 
   { "init",      ecmd_init },
   { "add",       ecmd_add },
   { "addurl",    ecmd_addurl },
   { "check",     ecmd_check },
   { "rmfile",    ecmd_rmfile },
   { "rm",        ecmd_rmfile },
   { "update",    ecmd_update },
   { "flush",     ecmd_flush },
   { "tag",       ecmd_tag },
   { "help",      ecmd_help }
};
const int ECMD_PATH_SIZE = (sizeof(ECMD_PATH) / sizeof(struct ecmd));

int
ecmd_exec(const char *ecmd, int argc, char **argv)
{
   int i;

   for (i = 0; i < ECMD_PATH_SIZE; i++) {
      if (strcmp(ecmd, ECMD_PATH[i]) == 0) {
         ECMD_PATH[i].func(argc, argv);
         return 0;
      }
   }

   return -1;
}