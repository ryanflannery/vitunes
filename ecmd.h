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

#ifndef ECMD_H
#define ECMD_H

#include "ecmd_args.h"

struct ecmd {
   const char  *name;
   const char  *alias;        /* may be NULL */
   const char  *usage;        /* may be an empty string */
   const char  *optstring;    /* may be NULL */
   int          args_lower;   /* minimum number of arguments */
   int          args_upper;   /* negative number means no limit */
   int        (*check)(struct ecmd_args *args); /* may be NULL */
   void       (*exec)(struct ecmd_args *args);
};

extern const struct ecmd ecmd_add;
extern const struct ecmd ecmd_addurl;
extern const struct ecmd ecmd_check;
extern const struct ecmd ecmd_flush;
extern const struct ecmd ecmd_help;
extern const struct ecmd ecmd_init;
extern const struct ecmd ecmd_rmfile;
extern const struct ecmd ecmd_tag;
extern const struct ecmd ecmd_update;

int ecmd_exec(const char *ecmd, int argc, char **argv);

#endif
