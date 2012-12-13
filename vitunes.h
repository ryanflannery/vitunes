/*
 * Copyright (c) 2010, 2011 Ryan Flannery <ryan.flannery@gmail.com>
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

#ifndef VITUNES_H
#define VITUNES_H

#include "compat.h"

#include <sys/time.h>

#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"
#include "debug.h"
#include "ecmd.h"
#include "enums.h"
#include "keybindings.h"
#include "medialib.h"
#include "player.h"
#include "uinterface.h"

/* for unused arguments */
#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED  __attribute__((__unused__))
#else
#  define UNUSED
#endif

/*
 * These are the various things defined in vitunes.c used elsewhere.
 */

/* configurable paths */
extern char *vitunes_dir;
extern char *playlist_dir;
extern char *db_file;

extern char *progname;

/* record keeping  */
extern playlist   *viewing_playlist;
extern playlist   *playing_playlist;
extern int         visual_mode_start;

/* signal flags referenced elsewhere */
extern volatile sig_atomic_t VSIG_QUIT;

/* other */
void load_config();
void process_signals();

#endif
