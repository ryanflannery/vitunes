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

#ifndef PAINT_H
#define PAINT_H

#include <curses.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "enums.h"
#include "meta_info.h"
#include "player.h"
#include "playlist.h"
#include "uinterface.h"
#include "vitunes.h"

#include "compat.h"

/* colors used by paint - each of these will be a number for a COLOR_PAIR */
typedef struct {
   /* visual dividers of windows */
   int   bars;

   /* individual windows */
   int   player;
   int   status;
   int   library;
   int   playlist;

   /* error/info messages */
   int   errors;
   int   messages;

   /* empty rows in library and playlist window */
   int   tildas_library;
   int   tildas_playlist;

   /* currently playing rows in the library & playlist windows */
   int   playing_library;
   int   playing_playlist;

   /* current row in inactive window */
   int   current_inactive;

   /* individual fields in the playlist window */
   int   cinfos[MI_NUM_CINFO];
   bool  cinfos_set[MI_NUM_CINFO];

} _colors;
extern _colors colors;

/* routines for painting each window */
void paint_status_bar();
void paint_player();
void paint_library();
void paint_playlist();
void paint_borders();
void paint_all();

extern bool showing_file_info;
void paint_playlist_file_info(const meta_info *m);

/* routines for painting errors/messages in the command/status window */
void paint_error(char *fmt, ...);
void paint_message(char *fmt, ...);

/* for setting up and working with the colors */
void paint_setup_colors();
int  paint_str2item(const char *str);
int  paint_str2color(const char *str);

#endif
