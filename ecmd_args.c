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

#include <stdlib.h>
#include <unistd.h>

#include "ecmd_args.h"
#include "str2argv.h"
#include "xmalloc.h"

/* E-command arguments compare function. */
static int
ecmd_args_cmp(struct ecmd_args_entry *entry1, struct ecmd_args_entry *entry2)
{
   return entry1->flag - entry2->flag;
}

/* Tree prototypes and functions. */
RB_PROTOTYPE_STATIC(ecmd_args_tree, ecmd_args_entry, entry, ecmd_args_cmp);
RB_GENERATE_STATIC(ecmd_args_tree, ecmd_args_entry, entry, ecmd_args_cmp);

/* Find flag in the arguments tree. */
static struct ecmd_args_entry *
ecmd_args_find(struct ecmd_args *args, int flag)
{
   struct ecmd_args_entry  entry;

   entry.flag = flag;
   return RB_FIND(ecmd_args_tree, &args->tree, &entry);
}

/* Add a flag and value to the arguments tree. */
void
ecmd_args_add(struct ecmd_args *args, int flag, const char *value)
{
   struct ecmd_args_entry  *entry;

   /* Remove existing value. */
   if ((entry = ecmd_args_find(args, flag)) != NULL) {
      free(entry->value);
      if (value != NULL)
         entry->value = xstrdup(value);
      else
         entry->value = NULL;
      return;
   }

   /* New value. */
   entry = xcalloc(1, sizeof *entry);
   entry->flag = flag;
   if (value != NULL)
      entry->value = xstrdup(value);
   RB_INSERT(ecmd_args_tree, &args->tree, entry);
}

/*
 * Checks if the arguments tree is empty. Useful for those e-commands that need
 * at least one flag.
 */
int
ecmd_args_empty(struct ecmd_args *args)
{
   return RB_EMPTY(&args->tree);
}

/* Get e-command argument value. */
const char *
ecmd_args_get(struct ecmd_args *args, int flag)
{
   struct ecmd_args_entry  *entry;

   if ((entry = ecmd_args_find(args, flag)) == NULL)
      return NULL;
   return entry->value;
}

/*
 * Generically parse function that adds the flags and, if applicable, its
 * values to the arguments tree. It initialises the arguments structure, which
 * isn't freed on purpose, because the program always quits on error.
 */
struct ecmd_args *
ecmd_args_parse(const char *optstring, int argc, char **argv)
{
   int                ch;
   struct ecmd_args  *args;

   /* Initialise e-command arguments structure. */
   args = xcalloc(1, sizeof *args);

   /* Reset getopt(3) variables (skip e-command name). */
   optind = 1;
   optreset = 1;
   while ((ch = getopt(argc, argv, optstring)) != -1) {
      if (ch == '?')
         return NULL;
      ecmd_args_add(args, ch, optarg);
   }
   argc -= optind;
   argv += optind;

   args->argc = argc;
   args->argv = argv_copy(argc, argv);

   return args;
}
