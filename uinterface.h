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

#ifndef UINTERFACE_H
#define UINTERFACE_H

#include <sys/ioctl.h>

#include <ctype.h>
#include <err.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "debug.h"
#include "enums.h"

#include "compat.h"

/* struct & methods for a scrollable window */
typedef struct
{
   WINDOW  *cwin;
   int      w, h;    /* it's just handy to track these here */
   int      crow;    /* current row in the window */
   int      nrows;   /* number of rows in the window */
   int      voffset; /* vertical & horizontal scroll offsets */
   int      hoffset;

} swindow;

swindow *swindow_new(int h, int w, int y, int x);
void swindow_free(swindow *win);
void swindow_resize(swindow *win, int h, int w, int y, int x);
void swindow_scroll(swindow *win, Direction d, int n);


/* user interface struct & methods */
typedef struct
{
   WINDOW   *player;
   WINDOW   *command;

   swindow  *library;
   swindow  *playlist;

   /* this is always a pointer to one of the two above swindow's */
   swindow  *active;

   int  lwidth; /* width of the library pane on the left */
   bool lhide;  /* if library window should be hidden or not */

} uinterface;
extern uinterface ui;   /* the global ui struct */

void ui_init(int library_width);
void ui_clear();
void ui_destroy();
void ui_resize();

void ui_hide_library();
void ui_unhide_library();

#endif
