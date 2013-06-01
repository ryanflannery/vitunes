/*
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

#ifndef ECMD_ARGS
#define ECMD_ARGS

#include "compat.h"

/* E-command arguments structure. */
struct ecmd_args_entry {
   int                         flag;   /* Argument flag. */
   char                       *value;  /* Argument value. */
   RB_ENTRY(ecmd_args_entry)   entry;
};
RB_HEAD(ecmd_args_tree, ecmd_args_entry);

struct ecmd_args {
   struct ecmd_args_tree     tree;     /* Tree of arguments. */
   int                       argc;
   char                    **argv;
};

struct ecmd_args;
void               ecmd_args_add(struct ecmd_args *, int, const char *);
int                ecmd_args_empty(struct ecmd_args *);
const char        *ecmd_args_get(struct ecmd_args *, int);
struct ecmd_args  *ecmd_args_parse(const char *, int, char **);

#endif
