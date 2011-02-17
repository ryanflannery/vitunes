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

#include "uinterface.h"

/* the global user interface object */
uinterface ui;

/*****************************************************************************
 * scrollable window stuff
 ****************************************************************************/

swindow*
swindow_new(int h, int w, int y, int x)
{
   swindow *swin = malloc(sizeof(swindow));
   if (swin == NULL)
      err(1, "swindow_new failed to allocate swin");

   swin->w        = w;
   swin->h        = h;
   swin->voffset  = 0;
   swin->hoffset  = 0;
   swin->crow     = 0;
   swin->nrows    = 0;
   swin->cwin     = newwin(h, w, y, x);
   if (swin->cwin == NULL)
      errx(1, "swindow_new: failed to create window");

   return swin;
}

void
swindow_resize(swindow *win, int h, int w, int y, int x)
{
   wresize(win->cwin, h, w);
   mvwin(win->cwin, y, x);

   win->w = w;
   win->h = h;

   if (win->crow >= h)
      win->crow = h - 1;
}

void
swindow_free(swindow *win)
{
   delwin(win->cwin);
   free(win);
}

void
swindow_scroll(swindow *win, Direction d, int n)
{
   switch (d) {
   case UP:
      win->voffset -= n;
      if (win->voffset < 0)
         win->voffset = 0;
      break;

   case DOWN:
      win->voffset += n;
      if (win->voffset >= win->nrows - win->h)
         win->voffset = win->nrows - win->h;
      if (win->voffset < 0)
         win->voffset = 0;
      break;

   case LEFT:
      win->hoffset -= n;
      if (win->hoffset < 0)
         win->hoffset = 0;
      break;

   case RIGHT:
      win->hoffset += n;
      /* NOTE: "overflow" here is handled elsewhere.
       * see input_handlers.c, scroll_col()
       */
      break;

   default:
      err(1, "swindow_scroll: bad direction");
   }
}

/*****************************************************************************
 * UI Create / Resize / Destroy Functions
 ****************************************************************************/

void
ui_init(int library_width)
{
   int lines, cols;

   /* ncurses init */
   initscr();
   raw();
   noecho();
   nonl();
   keypad(stdscr, TRUE);
   start_color();
   curs_set(0);
   ESCDELAY = 0;
   refresh();

   getmaxyx(stdscr, lines, cols);

   /* setup width of library window */
   ui.lwidth = library_width;
   ui.lhide = false;

   /* create the player and command windows */
   ui.player  = newwin(1, cols, 0,         0);
   ui.command = newwin(1, cols, lines - 1, 0);
   if (ui.player == NULL || ui.command == NULL)
      errx(1, "ui_init: failed to create player/command windows");

   /* and the rest */
   ui.library  = swindow_new(lines - 3, ui.lwidth, 2, 0);
   ui.playlist = swindow_new(lines - 3, cols - ui.lwidth - 1, 2, ui.lwidth + 1);

   ui.active = ui.library;
}

bool
ui_is_init()
{
   return isendwin() == TRUE;
}

void
ui_destroy()
{
   /* destroy each window (this also free()'s the mem for each window) */
   delwin(ui.player);
   delwin(ui.command);
   swindow_free(ui.playlist);
   if (ui.library != NULL)
      swindow_free(ui.library);

   /* reset other properties of the ui to sane 'empty' defaults */
   ui.player = NULL;
   ui.command = NULL;
   ui.library = NULL;
   ui.playlist = NULL;
   ui.active = NULL;

   /* end ncurses */
   endwin();
}

void
ui_resize()
{
   struct winsize ws;

   /* get new dimensions and check for changes */
   if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
      err(1, "ui_resize: ioctl failed");

   /* can we even handle the new display size?  if not, just exit */
   if (ws.ws_col < ui.lwidth + 2) {
      endwin();
      errx(1, "ui_resize: not enough columns to render vitunes nicely");
   }
   if (ws.ws_row < 4) {
      endwin();
      errx(1, "ui_resize: not enough rows to render vitunes nicely");
   }

   /* resize ncurses */
   resizeterm(ws.ws_row, ws.ws_col);

   /* resize player & command windows */
   wresize(ui.player,  1, ws.ws_col);
   wresize(ui.command, 1, ws.ws_col);

   /* resize library and playlist windows */
   if (ui.library->cwin == NULL)
      swindow_resize(ui.playlist, ws.ws_row - 3, ws.ws_col, 2, 0);
   else {
      swindow_resize(ui.library,  ws.ws_row - 3, ui.lwidth, 2, 0);
      swindow_resize(ui.playlist, ws.ws_row - 3, ws.ws_col - ui.lwidth - 1, 2, ui.lwidth + 1);
   }

   /* move the command window to the new bottom row */
   mvwin(ui.command, ws.ws_row - 1, 0);

}

void
ui_hide_library()
{
   int w, h;

   /* if already hidden, nothing to do */
   if (ui.library->cwin == NULL) return;

   /* remove library window */
   delwin(ui.library->cwin);
   ui.library->cwin = NULL;

   /* resize & move playlist window */
   getmaxyx(stdscr, h, w);
   swindow_resize(ui.playlist, h - 3, w, 2, 0);
}

void
ui_unhide_library()
{
   int w, h;

   /* if not hidden, nothing to do */
   if (ui.library->cwin != NULL) return;

   getmaxyx(stdscr, h, w);

   /* create library window */
   ui.library->cwin = newwin(h - 3, ui.lwidth, 2, 0);
   if (ui.library->cwin == NULL)
      errx(1, "ui_unhide_library: failed to create newwin");

   /* resize & move playlist window */
   swindow_resize(ui.playlist, h - 3, w - ui.lwidth - 1, 2, ui.lwidth + 1);
}

void
ui_clear()
{
   wclear(ui.player);
   wclear(ui.command);
   wclear(ui.library->cwin);
   wclear(ui.playlist->cwin);
   clear();

   wrefresh(ui.player);
   wrefresh(ui.command);
   wrefresh(ui.library->cwin);
   wrefresh(ui.playlist->cwin);
   refresh();  /* XXX needed? */
}
