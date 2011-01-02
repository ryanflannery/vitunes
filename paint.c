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

#include "paint.h"

/* globalx */
_colors colors;
bool showing_file_info = false;

char *player_get_field2show(const meta_info *mi);
char *num2fmt(int n, Direction d);


/*
 * This is used to get which field in the playing file to display on a given
 * paint of the player window.
 * The field displayed rotates between artist, album, and title, changing
 * every 3 seconds.
 */
char *
player_get_field2show(const meta_info *mi)
{
   static time_t last_updated = 0;
   static int    index        = 0;
   static int    offset;
   static int    fields[] = { MI_CINFO_ARTIST, MI_CINFO_ALBUM,
                              MI_CINFO_TITLE };

   /* determine which cinfo item to show */
   if (time(NULL) - last_updated >= 3) {
      last_updated = time(NULL);
      index = (index + 1) % 3;
   }

   if (mi->cinfo[ fields[index] ] != NULL)
      return mi->cinfo[ fields[index] ];
   else {
      /* draw the filename if field info not there, but trim it down */
      offset = 0;
      if (strlen(mi->filename) > 49)
         offset += strlen(mi->filename) - 49;

      return mi->filename + offset;
   }
}

/*
 * Given a number N and an alignment, this builds and returns a printf(3)-
 * style string ("%Ns" or "%-Ns")
 */
char *
num2fmt(int n, Direction d)
{
   static char format[255];

   if (n <= 0) {
      endwin();
      errx(1, "num2sfmt: invalid number %d provided", n);
   }

   if (d == LEFT)
      snprintf(format, sizeof(format), "%%-%d.%ds", n, n);
   else
      snprintf(format, sizeof(format), "%%%d.%ds", n, n);

   return format;
}

/* paint the status bar */
void
paint_status_bar()
{
   static char scratchpad[500];
   char       *focusName;
   int         percent;
   int         w, h;

   getmaxyx(stdscr, h, w);

   /*
    * XXX NOTE: to right-align the text we snprintf to the scratchpad above and
    * then use the paint that to the window.
    */

   /* determine focus'd window name */
   if (ui.active == ui.library)
      focusName = "library";
   else
      focusName = "playlist";

   /* determine how far we've scrolled through the window */
   if (ui.active->nrows == 0)
      percent = 100;
   else
      percent = 100 * (ui.active->voffset + ui.active->crow + 1) / ui.active->nrows;

   /* build the string to print */
   snprintf(scratchpad, sizeof(scratchpad),
      "[%s%s%s] %6d,%-3d %3d%%",
      focusName,
      (ui.active == ui.library ? "" : ":"),
      (ui.active == ui.library ? "" : viewing_playlist->name),
      ui.active->voffset + ui.active->crow + 1,
      ui.active->hoffset,
      percent);

   /* do the printing */
   werase(ui.command);
   wattron(ui.command, COLOR_PAIR(colors.status));
   mvwprintw(ui.player, 0, 0, num2fmt(w, LEFT), " "); /* this fills the bg color */
   mvwprintw(ui.command, 0, 0, num2fmt(w, RIGHT), scratchpad);
   wattroff(ui.command, COLOR_PAIR(colors.status));
   wrefresh(ui.command);
}

/* paint the player */
void
paint_player()
{
   static char *playmode;
   static char *finfo;
   static int   in_hour;
   static int   in_minute;
   static int   in_second;
   static int   percent, whole;
   int w, h;

   getmaxyx(stdscr, h, w);

   /* if nothing's playing, a shameless plug */
   if (!player_status.playing) {
      werase(ui.player);
      wattron(ui.player, COLOR_PAIR(colors.player));
      mvwprintw(ui.player, 0, 0, num2fmt(w, LEFT), "vitunes...");
      wattroff(ui.player, COLOR_PAIR(colors.player));
      wrefresh(ui.player);
      return;
   }

   /* determine time into current selection */
   in_hour   = (int)  roundf(player_status.position / 3600);
   in_minute = ((int) roundf(player_status.position)) % 3600 / 60;
   in_second = ((int) roundf(player_status.position)) % 60;

   /* determine percent time into current selection */
   percent = -1;
   if (playing_playlist->files[player.qidx]->length > 0) {
      whole = playing_playlist->files[player.qidx]->length;
      percent = roundf(100.0 * player_status.position / whole);
   }

   /* get character for playmode */
   switch (player.mode) {
      case PLAYER_MODE_LINEAR:
         playmode = "-";
         break;
      case PLAYER_MODE_LOOP:
         playmode = "O";
         break;
      case PLAYER_MODE_RANDOM:
         playmode = "?";
         break;
   }

   /* determine info about song to show */
   finfo = player_get_field2show(playing_playlist->files[player.qidx]);

   /* draw */
   werase(ui.player);
   wattron(ui.player, COLOR_PAIR(colors.player));
   mvwprintw(ui.player, 0, 0, num2fmt(w, LEFT), " "); /* this fills the bg color */
   mvwprintw(ui.player, 0, 0,
      "[%s] %8.8s +%2.2d:%2.2d:%2.2d (%d%%) %-49.49s",
      playmode,
      (player_status.paused ? "-PAUSED-" : ""),
      in_hour, in_minute, in_second,
      percent,
      finfo);
   wattroff(ui.player, COLOR_PAIR(colors.player));
   wrefresh(ui.player);
}

/* paint the library window */
void
paint_library()
{
   char *str;
   int   row, hoff, index;

   /* if library window is hidden, nothing to do */
   if (ui.library->cwin == NULL) return;

   werase(ui.library->cwin);
   wattron(ui.library->cwin, COLOR_PAIR(colors.library));

   for (row = 0; row < ui.library->h; row++) {

      index = ui.library->voffset + row;

      /* apply attributes */
      if (index < mdb.nplaylists && mdb.playlists[index] == playing_playlist)
         wattron(ui.library->cwin, COLOR_PAIR(colors.playing_library));

      if (index < mdb.nplaylists && mdb.playlists[index]->needs_saving)
         wattron(ui.library->cwin, A_BOLD);

      if (row == ui.library->crow)
         wattron(ui.library->cwin, A_REVERSE);

      if (index >= mdb.nplaylists)
         wattron(ui.library->cwin, COLOR_PAIR(colors.tildas_library));

      /* draw the row */
      if (index >= mdb.nplaylists)
         mvwprintw(ui.library->cwin, row, 0, "~");
      else {
         /* determine horizontal offset */
         str = mdb.playlists[index]->name;
         hoff = ui.library->hoffset;
         if (hoff >= (int)strlen(str))
            hoff = strlen(str);

         /* draw it */
         mvwprintw(ui.library->cwin, row, 0,
            num2fmt(ui.library->w, LEFT),
            str + hoff);
      }

      /* un-apply attributes */
      if (index >= mdb.nplaylists) {
         wattroff(ui.library->cwin, COLOR_PAIR(colors.tildas_library));
         wattron(ui.library->cwin, COLOR_PAIR(colors.library));
      }

      if (row == ui.library->crow) {
         wattroff(ui.library->cwin, A_REVERSE);
         wattron(ui.library->cwin, COLOR_PAIR(colors.library));
      }

      if (index < mdb.nplaylists && mdb.playlists[index]->needs_saving) {
         wattroff(ui.library->cwin, A_BOLD);
         wattron(ui.library->cwin, COLOR_PAIR(colors.library));
      }

      if (index < mdb.nplaylists && mdb.playlists[index] == playing_playlist) {
         wattroff(ui.library->cwin, COLOR_PAIR(colors.playing_library));
         wattron(ui.library->cwin, COLOR_PAIR(colors.library));
      }
   }

   wattroff(ui.library->cwin, COLOR_PAIR(colors.library));
   wrefresh(ui.library->cwin);
}

/* paint the playlist window */
void
paint_playlist()
{
   playlist   *plist;
   bool        hasinfo;
   char       *str;
   int         findex, row, col, colwidth;
   int         xoff, hoff, strhoff;
   int         cattr;


   showing_file_info = false;
   plist = viewing_playlist;

   werase(ui.playlist->cwin);

   for (row = 0; row < ui.playlist->h; row++) {

      /* get index of file to show */
      findex = row + ui.playlist->voffset;

      /* apply row attributes */
       wattron(ui.playlist->cwin, COLOR_PAIR(colors.playlist));

      if (plist == playing_playlist && findex == player.qidx)
         wattron(ui.playlist->cwin, COLOR_PAIR(colors.playing_playlist));

      if (row == ui.playlist->crow)
         wattron(ui.playlist->cwin, A_REVERSE);

      if (findex >= plist->nfiles)
         wattron(ui.playlist->cwin, COLOR_PAIR(colors.tildas_playlist));

      /* draw the row */
      if (findex >= plist->nfiles)
         mvwprintw(ui.playlist->cwin, row, 0, "~");
      else {
         /* this acheives the A_REVERSE attribute spanning the entire row */
         mvwprintw(ui.playlist->cwin, row, 0,
            num2fmt(ui.playlist->w, LEFT), " ");

         /* does the file have any meta-info? */
         hasinfo = false;
         for (col = 0; col < mi_display.nfields; col++) {
            if (plist->files[findex]->cinfo[mi_display.order[col]] != NULL)
               hasinfo = true;
         }

         /* if there's no meta info, just show filename */
         if (!hasinfo) {
            mvwprintw(ui.playlist->cwin, row, 0, num2fmt(ui.playlist->w, LEFT),
               plist->files[findex]->filename);
         } else {

            /* loop through all fields of file and display each ... */
            xoff = 0;
            hoff = ui.playlist->hoffset;
            for (col = 0; col < mi_display.nfields; col++) {

               /* is horizontal offset big enough to skip this field? */
               if (hoff >= mi_display.widths[col]) {
                  hoff -= mi_display.widths[col];
                  continue;
               }

               /* field shown off the screen? */
               if (xoff >= ui.playlist->w)
                  continue;

               /* get string to show (str) */
               str = plist->files[findex]->cinfo[mi_display.order[col]];

               /* determine horizontal offset (strhoff) to apply to str */
               strhoff = 0;
               if (str != NULL) {
                  if (mi_display.align[col] == LEFT) {
                     if (hoff > (int)strlen(str))
                        strhoff = strlen(str);
                     else
                        strhoff = hoff;
                  } else {
                     if ((int)strlen(str) > mi_display.widths[col])
                        strhoff = hoff;
                     else if (hoff < mi_display.widths[col] - (int)strlen(str))
                        strhoff = 0;
                     else
                        strhoff = hoff - (mi_display.widths[col] - strlen(str));

                     if (strhoff > (int)strlen(str))
                        strhoff = strlen(str);
                  }
               }

               /* apply column attribute (only if file is NOT playing) */
               cattr = COLOR_PAIR(colors.cinfos[mi_display.order[col]]);
               if ((plist != playing_playlist || findex != player.qidx)
               && colors.cinfos_set[mi_display.order[col]])
                  wattron(ui.playlist->cwin, cattr);

               /* determine width of this field */
               colwidth = mi_display.widths[col] - hoff;
               if (xoff + colwidth > ui.playlist->w)
                  colwidth = ui.playlist->w - xoff;

               /* print the column */
               mvwprintw(ui.playlist->cwin, row, xoff,
                  num2fmt(colwidth, mi_display.align[col]),
                  (str == NULL ? " " : str + strhoff));

               /* un-apply column attribute */
               if ((plist != playing_playlist || findex != player.qidx)
               && colors.cinfos_set[mi_display.order[col]]) {
                  wattroff(ui.playlist->cwin, cattr);
                  wattron(ui.playlist->cwin, COLOR_PAIR(colors.playlist));
               }

               xoff += 1 + colwidth; /* +1 for space between columns */
               hoff = 0;
            }
         }
      }

      /* un-apply row attributes */
      if (findex >= plist->nfiles)
         wattroff(ui.playlist->cwin, COLOR_PAIR(colors.tildas_playlist));

      if (row == ui.playlist->crow)
         wattroff(ui.playlist->cwin, A_REVERSE);

      if (plist == playing_playlist && findex == player.qidx)
         wattroff(ui.playlist->cwin, COLOR_PAIR(colors.playing_playlist));

      wattroff(ui.playlist->cwin, COLOR_PAIR(colors.playlist));
   }

   wrefresh(ui.playlist->cwin);
}

/* paint borders between windows */
void
paint_borders()
{
   int w, h;
   getmaxyx(stdscr, h, w);

   wattron(stdscr, COLOR_PAIR(colors.bars));
   mvhline(1, 0, ACS_HLINE, w);
   if (ui.library->cwin != NULL) {
      mvvline(1, ui.lwidth, ACS_VLINE, h - 2);
      mvaddch(1, ui.lwidth, ACS_TTEE);
   }
   wattroff(stdscr, COLOR_PAIR(colors.bars));
   refresh();
}

/* paint individual file info in playlist window */
void
paint_playlist_file_info(const meta_info *m)
{
   struct tm *ltime;
   char stime[255];
   int row, nrows, i;
   int h, w;

   getmaxyx(ui.playlist->cwin, h, w);
   werase(ui.playlist->cwin);
   wattron(ui.playlist->cwin, COLOR_PAIR(colors.playlist));

   /* figure out number of rows filename will take */
   nrows = strlen(m->filename) / w;
   if (strlen(m->filename) % w != 0) nrows++;

   /* start painting file info */
   row = 0;
   mvwprintw(ui.playlist->cwin, row++, 0, "What vitunes knows about the file/URL:");
   nl();
   mvwprintw(ui.playlist->cwin, row, 0,  m->filename);
   nonl();

   row += nrows + 1;

   /* paint meta-info */
   mvwprintw(ui.playlist->cwin, row++, 0, "Meta-Information:");
   for (i = 0; i < MI_NUM_CINFO; i++) {
      mvwprintw(ui.playlist->cwin, row++, 0, "%10s: \"%s\"",
         MI_CINFO_NAMES[i], m->cinfo[i]);
   }

   row += 1;

   /* paint other details */
   mvwprintw(ui.playlist->cwin, row++, 0, "Additional Details:");
   mvwprintw(ui.playlist->cwin, row++, 0, "%15s: %i seconds",
      "Length", m->length);
   mvwprintw(ui.playlist->cwin, row++, 0, "%15s: %s",
      "URL?", (m->is_url ? "Yes" : "No"));

   ltime = localtime(&(m->last_updated));
   strftime(stime, sizeof(stime), "%d %B %Y at %H:%M:%S", ltime);
   mvwprintw(ui.playlist->cwin, row, 0, "%15s: %s", "Last Updated", stime);

   wattroff(ui.playlist->cwin, COLOR_PAIR(colors.playlist));
   wrefresh(ui.playlist->cwin);
   showing_file_info = true;
}

/* paint all windows */
void
paint_all()
{
   paint_borders();
   paint_player();
   paint_status_bar();
   paint_library();
   paint_playlist();
}

/*
 * Paints an error message to the command/status window.  The usage is
 * identical to that of printf(3) (vwprintw(3) actually).
 */
void
paint_error(char *fmt, ...)
{
   va_list ap;

   werase(ui.command);
   wmove(ui.command, 0, 0);
   wattron(ui.command, COLOR_PAIR(colors.errors));

   va_start(ap, fmt);
   vwprintw(ui.command, fmt, ap);
   va_end(ap);

   beep();
   wattroff(ui.command, COLOR_PAIR(colors.errors));
   wrefresh(ui.command);
}

/*
 * Paints an informational message to the command/status window.  The usage
 * is identical to that of printf(3) (vwprintw(3) actually).
 */
void
paint_message(char *fmt, ...)
{
   va_list ap;

   werase(ui.command);
   wmove(ui.command, 0, 0);
   wattron(ui.command, COLOR_PAIR(colors.messages));

   va_start(ap, fmt);
   vwprintw(ui.command, fmt, ap);
   va_end(ap);

   wattroff(ui.command, COLOR_PAIR(colors.messages));
   wrefresh(ui.command);
}

/*
 * Each of these members of the global color object will be
 * run through init_pair(3)
 */
void
paint_setup_colors()
{
   int i;

   /* setup the indices to be used for init_pair/COLOR_PAIR */
   colors.bars     = 1;
   colors.player   = 2;
   colors.status   = 3;
   colors.library  = 4;
   colors.playlist = 5;
   colors.errors   = 6;
   colors.messages = 7;
   colors.tildas_library   = 8;
   colors.tildas_playlist  = 9;
   colors.playing_library  = 10;
   colors.playing_playlist = 11;

   /* setup default colors */
   init_pair(colors.bars,     COLOR_WHITE, COLOR_BLACK);
   init_pair(colors.player,   COLOR_GREEN, COLOR_BLACK);
   init_pair(colors.status,   COLOR_WHITE, COLOR_BLACK);
   init_pair(colors.library,  COLOR_WHITE, COLOR_BLACK);
   init_pair(colors.playlist, COLOR_WHITE, COLOR_BLACK);
   init_pair(colors.errors,   COLOR_WHITE, COLOR_RED);
   init_pair(colors.messages, COLOR_RED,   COLOR_BLACK);
   init_pair(colors.tildas_library,   COLOR_BLUE, COLOR_BLACK);
   init_pair(colors.tildas_playlist,  COLOR_BLUE, COLOR_BLACK);
   init_pair(colors.playing_library,  COLOR_GREEN, COLOR_BLACK);
   init_pair(colors.playing_playlist, COLOR_GREEN, COLOR_BLACK);

   /* colors for cinfo fields (columns in playlist window) */
   for (i = 0; i < MI_NUM_CINFO; i++) {
      colors.cinfos[i] = 12 + i;
      colors.cinfos_set[i] = false;
   }
}

int
paint_str2item(const char *str)
{
   int i;

   if (strcasecmp(str, "bars") == 0)
      return colors.bars;
   else if (strcasecmp(str, "player") == 0)
      return colors.player;
   else if (strcasecmp(str, "status") == 0)
      return colors.status;
   else if (strcasecmp(str, "library") == 0)
      return colors.library;
   else if (strcasecmp(str, "playlist") == 0)
      return colors.playlist;
   else if (strcasecmp(str, "errors") == 0)
      return colors.errors;
   else if (strcasecmp(str, "messages") == 0)
      return colors.messages;
   else if (strcasecmp(str, "tildas-library") == 0)
      return colors.tildas_library;
   else if (strcasecmp(str, "tildas-playlist") == 0)
      return colors.tildas_playlist;
   else if (strcasecmp(str, "playing-library") == 0)
      return colors.playing_library;
   else if (strcasecmp(str, "playing-playlist") == 0)
      return colors.playing_playlist;

   /* if reached here, check cinfo's array */
   for (i = 0; i < MI_NUM_CINFO; i++) {
      if (strcasecmp(str, MI_CINFO_NAMES[i]) == 0)
         return colors.cinfos[i];
   }

   return -1;
}

int
paint_str2color(const char *str)
{
   if (strcasecmp(str, "black") == 0)
      return COLOR_BLACK;
   else if (strcasecmp(str, "red") == 0)
      return COLOR_RED;
   else if (strcasecmp(str, "green") == 0)
      return COLOR_GREEN;
   else if (strcasecmp(str, "yellow") == 0)
      return COLOR_YELLOW;
   else if (strcasecmp(str, "blue") == 0)
      return COLOR_BLUE;
   else if (strcasecmp(str, "magenta") == 0)
      return COLOR_MAGENTA;
   else if (strcasecmp(str, "cyan") == 0)
      return COLOR_CYAN;
   else if (strcasecmp(str, "white") == 0)
      return COLOR_WHITE;
   else
      return -1;
}
