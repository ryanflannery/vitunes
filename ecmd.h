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

#ifndef ECMD_H
#define ECMD_H

#include <stdio.h>
#include <string.h>

/* "main" functions for each command */
void ecmd_init(int argc, char *argv[]);
void ecmd_add(int argc, char *argv[]);
void ecmd_addurl(int argc, char *argv[]);
void ecmd_flush(int argc, char *argv[]);
void ecmd_check(int argc, char *argv[]);
void ecmd_rmfile(int argc, char *argv[]);
void ecmd_tag(int argc, char *argv[]);
void ecmd_update(int argc, char *argv[]);
void ecmd_help(int argc, char *argv[]);

int ecmd_exec(const char *ecmd, int argc, char **argv);

/* e-command struct and set of commands */
struct ecmd {
   const char *name;
   void (*func)(int argc, char *argv[]);
};

extern const struct ecmd ECMD_PATH[];
extern const int ECMD_PATH_SIZE;

#endif