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

#ifndef E_COMMANDS_H
#define E_COMMANDS_H

#include "compat.h"

#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>

#include "meta_info.h"
#include "medialib.h"
#include "playlist.h"

/* from vitunes.c */
extern char *vitunes_dir;
extern char *playlist_dir;
extern char *db_file;

/* "main" functions for each command */
int ecmd_init(int argc, char *argv[]);
int ecmd_add(int argc, char *argv[]);
int ecmd_addurl(int argc, char *argv[]);
int ecmd_flush(int argc, char *argv[]);
int ecmd_check(int argc, char *argv[]);
int ecmd_rmfile(int argc, char *argv[]);
int ecmd_tag(int argc, char *argv[]);
int ecmd_update(int argc, char *argv[]);
int ecmd_help(int argc, char *argv[]);

/* find and execute the given e-command */
int ecmd_execute(int argc, char *argv[], const char *ecmd);

/* e-command struct and set of commands */
struct ecmd {
   char *name;
   int   (*func)(int argc, char *argv[]);
};

extern const struct ecmd ECMD_PATH[];
extern const int ECMD_PATH_SIZE;

#endif
