/*
 * Copyright (c) 2010 Ryan Flannery <ryan.flannery@gmail.com>
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

#include "keybindings.h"

/*****************************************************************************
 * for working with number read from user for keybindings
 ****************************************************************************/
int _global_input_num = 0;

void gnum_clear()
{ _global_input_num = 0; }

int  gnum_get()
{ return _global_input_num; }

void gnum_set(int x)
{ _global_input_num = x; }

void gnum_add(int x)
{
   _global_input_num = _global_input_num * 10 + x;
}


/*****************************************************************************
 * for working with active search direction
 ****************************************************************************/
venum _global_search_dir = FORWARDS;

venum search_dir_get()
{ return _global_search_dir; }

void search_dir_set(venum dir)
{ _global_search_dir = dir; }


/*****************************************************************************
 * functions for working with yank/cut buffer
 ****************************************************************************/

 /* the global yank/copy bufer */
yank_buffer _yank_buffer;

void
ybuffer_init()
{
   _yank_buffer.files = calloc(YANK_BUFFER_CHUNK_SIZE, sizeof(meta_info*));
   if (_yank_buffer.files == NULL)
      err(1, "ybuffer_init: calloc(3) failed");

   _yank_buffer.capacity = YANK_BUFFER_CHUNK_SIZE;
   _yank_buffer.nfiles = 0;
}

void
ybuffer_clear()
{
   _yank_buffer.nfiles = 0;
}

void
ybuffer_free()
{
   free(_yank_buffer.files);
   _yank_buffer.capacity = 0;
   _yank_buffer.nfiles = 0;
}

void
ybuffer_add(meta_info *f)
{
   meta_info **new_buff;
   int   new_capacity;

   /* do we need to realloc()? */
   if (_yank_buffer.nfiles == _yank_buffer.capacity) {
      _yank_buffer.capacity += YANK_BUFFER_CHUNK_SIZE;
      new_capacity = _yank_buffer.capacity * sizeof(meta_info*);
      if ((new_buff = realloc(_yank_buffer.files, new_capacity)) == NULL)
         err(1, "ybuffer_add: realloc(3) failed [%i]", new_capacity);

      _yank_buffer.files = new_buff;
   }

   /* add the file */
   _yank_buffer.files[ _yank_buffer.nfiles++ ] = f;
}


/*****************************************************************************
 * misc. handy stuff used frequently
 ****************************************************************************/

void
redraw_active()
{
   if (ui.active == ui.library)
      paint_library();
   else
      paint_playlist();
}

/*
 * Given string input from user (argv[0]) and a command name, check if the
 * input matches the command name, taking into acount '!' weirdness and
 * abbreviations.
 */
bool
match_command_name(const char *input, const char *cmd)
{
   bool  found;
   char *icopy;

   if (input == NULL || strlen(input) == 0)
      return false;

   if (strcmp(input, cmd) == 0)
      return true;

   /* check for '!' weirdness and abbreviations */

   if ((icopy = strdup(input)) == NULL)
      err(1, "match_command_name: strdup(3) failed");

   /* remove '!' from input, if present */
   if (strstr(icopy, "!") != NULL)
      *strstr(icopy, "!") = '\0';

   /* now check against command & abbreviation */
   if (strstr(cmd, icopy) == cmd)
      found = true;
   else
      found = false;

   free(icopy);
   return found;
}

void
execute_external_command(const char *cmd)
{
   def_prog_mode();
   endwin();

   system(cmd);
   printf("\nPress ENTER to continue");
   fflush(stdout);
   while (getchar() != '\n');

   reset_prog_mode();
   paint_all();
}

/*****************************************************************************
 * keybinding handlers
 ****************************************************************************/

void
quit_vitunes(Args a UNUSED)
{
   VSIG_QUIT = 1;
}

void
load_or_play(Args a UNUSED)
{
   Args dummy;
   int  idx;

   if (ui.active == ui.library) {
      /* load playlist & switch focus */
      idx = ui.library->voffset + ui.library->crow;
      viewing_playlist = mdb.playlists[idx];
      ui.playlist->nrows = mdb.playlists[idx]->nfiles;
      ui.playlist->crow  = 0;
      ui.playlist->voffset = 0;
      ui.playlist->hoffset = 0;

      paint_playlist();
      switch_focus(dummy);
   } else {
      /* play song */
      if (ui.active->crow >= ui.active->nrows) {
         paint_message("no file here");
         return;
      }
      player_set_queue(viewing_playlist, ui.active->voffset + ui.active->crow);
      playing_playlist = viewing_playlist;
      player_play();
   }
}

void
show_file_info(Args a UNUSED)
{
   int idx;

   if (ui.active == ui.library)
      return;

   if (ui.active->crow >= ui.active->nrows) {
      paint_message("no file here");
      return;
   }

   if (showing_file_info)
      paint_playlist();
   else {
      /* get file index and show */
      idx = ui.active->voffset + ui.active->crow;
      paint_playlist_file_info(viewing_playlist->files[idx]);
   }
}

void
pause_playback(Args a UNUSED)
{
   player_pause();
}

void
stop_playback(Args a UNUSED)
{
   player_stop();
   playing_playlist = NULL;
}

void
seek_playback(Args a)
{
   int n, secs;

   /* determine number of seconds to seek */
   switch (a.scale) {
      case SECONDS:
         secs = a.num;
         break;
      case MINUTES:
         secs = a.num * 60;
         break;
      default:
         errx(1, "seek_playback: invalid scale");
   }

   /* adjust for direction */
   switch (a.direction) {
      case FORWARDS:
         /* no change */
         break;
      case BACKWARDS:
         secs *= -1;
         break;
      default:
         errx(1, "seek_playback: invalid direction");
   }

   /* is there a multiplier? */
   n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* apply n & seek */
   player_seek(secs * n);
}

void
switch_focus(Args a UNUSED)
{
   if (ui.active == ui.library) {
      ui.active = ui.playlist;
      if (ui.lhide) {
         ui_hide_library();
         paint_all();
      }
   } else {
      ui.active = ui.library;
      if (ui.lhide) {
         ui_unhide_library();
         paint_all();
      }
   }

   paint_status_bar();
}

void
redraw(Args a UNUSED)
{
   ui_clear();
   paint_all();
}

void
enter_cmd_mode(Args a UNUSED)
{
   const char *errmsg = NULL;
   char  *cmd;
   char **argv;
   int    argc;
   int    num_matches;
   bool   found;
   int    found_idx = 0;
   int    i;


   /* get command from user */
   if (user_getstr(":", &cmd) != 0 || strlen(cmd) == 0) {
      werase(ui.command);
      wrefresh(ui.command);
      return;
   }

   /* check for '!' used for executing external commands */
   if (cmd[0] == '!') {
      execute_external_command(cmd + 1);
      free(cmd);
      return;
   }

   /* convert to argc/argv structure */
   if (str2argv(cmd, &argc, &argv, &errmsg) != 0) {
      paint_error("parse error: %s in '%s'", errmsg, cmd);
      free(cmd);
      return;
   }

   /* search path for appropriate command to execute */
   found = false;
   num_matches = 0;
   for (i = 0; i < CommandPathSize; i++) {
      if (match_command_name(argv[0], CommandPath[i].name)) {
         found = true;
         found_idx = i;
         num_matches++;
      }
   }

   /* execute command or indicate failure */
   if (found && num_matches == 1)
      (CommandPath[found_idx].func)(argc, argv);
   else if (num_matches > 1)
      paint_error("Ambiguous abbreviation '%s'", argv[0]);
   else
      paint_error("Unknown command '%s'", argv[0]);

   argv_free(&argc, &argv);
   free(cmd);
}

void
external_command(Args a UNUSED)
{
   char  *cmd;

   /* get command from user */
   if (user_getstr("!", &cmd) != 0) {
      werase(ui.command);
      wrefresh(ui.command);
      return;
   }

   execute_external_command(cmd);
   free(cmd);
}

void
scroll_row(Args a)
{
   int n;

   /* determine how many rows to scroll */
   n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* update current row */
   switch (a.direction) {
      case DOWN:
         ui.active->crow += n;
         break;
      case UP:
         ui.active->crow -= n;
         break;
      default:
         errx(1, "scroll_row: invalid direction");
   }

   /* handle off-the-edge cases */
   if (ui.active->crow < 0) {
      swindow_scroll(ui.active, UP, -1 * ui.active->crow);
      ui.active->crow = 0;
   }

   if (ui.active->voffset + ui.active->crow >= ui.active->nrows)
      ui.active->crow = ui.active->nrows - ui.active->voffset - 1;

   if (ui.active->crow >= ui.active->h) {
      swindow_scroll(ui.active, DOWN, ui.active->crow - ui.active->h + 1);
      ui.active->crow = ui.active->h - 1;
   }

   if (ui.active->crow < 0)
      ui.active->crow = 0;

   /* redraw */
   redraw_active();
   paint_status_bar();
}

void
scroll_col(Args a)
{
   int   maxlen;
   int   maxhoff;
   int   n;
   int   i;

   /* determine how many cols to scroll */
   n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* determine maximum horizontal offset */
   maxhoff = 0;
   if (ui.active == ui.playlist) {
      if (mi_display_getwidth() > ui.active->w)
         maxhoff = mi_display_getwidth() - ui.active->w;
   } else {
      maxlen = 0;
      for (i = 0; i < mdb.nplaylists; i++) {
         if ((int) strlen(mdb.playlists[i]->name) > maxlen)
            maxlen = strlen(mdb.playlists[i]->name);
      }
      if (maxlen > ui.active->w)
         maxhoff = maxlen - ui.active->w;
   }

   /* scroll */
   switch (a.amount) {
      case SINGLE:
         swindow_scroll(ui.active, a.direction, n);
         if (ui.active->hoffset > maxhoff)
            ui.active->hoffset = maxhoff;
         break;
      case WHOLE:
         switch (a.direction) {
            case LEFT:
               ui.active->hoffset = 0;
               break;
            case RIGHT:
               ui.active->hoffset = maxhoff;
               break;
            default:
               errx(1, "scroll_col: invalid direction");
         }
         break;
      default:
         errx(1, "scroll_col: invalid amount");
   }

   /* redraw */
   redraw_active();
   paint_status_bar();
}

void
scroll_page(Args a)
{
   bool maintain_row_idx;
   int  voffset_original;
   int  max_row;
   int  diff;
   int  n;

   voffset_original = ui.active->voffset;

   /* determine max row number */
   if (ui.active->voffset + ui.active->h >= ui.active->nrows)
      max_row = ui.active->nrows - ui.active->voffset - 1;
   else
      max_row = ui.active->h - 1;

   /* determine how many pages to scroll */
   n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* determine how much crow should change */
   switch (a.amount) {
      case SINGLE:
         diff = 1 * n;
         maintain_row_idx = true;
         break;
      case HALF:
         diff = (ui.active->h / 2) * n;
         maintain_row_idx = false;
         break;
      case WHOLE:
         diff = ui.active->h * n;
         maintain_row_idx = false;
         break;
      default:
         errx(1, "scroll_page: invalid amount");
   }
   swindow_scroll(ui.active, a.direction, diff);

   /* handle updating the current row */
   if (maintain_row_idx) {

      if (a.direction == DOWN)
         ui.active->crow -= diff;
      else
         ui.active->crow += diff;

      /* handle off-the-edge cases */
      if (ui.active->crow < 0)
         ui.active->crow = 0;

      if (ui.active->voffset + ui.active->crow >= ui.active->nrows)
         ui.active->crow = ui.active->nrows - ui.active->voffset - 1;

      if (ui.active->crow >= ui.active->h)
         ui.active->crow = ui.active->h - 1;

      if (ui.active->crow < 0)
         ui.active->crow = 0;

   } else {
      /*
       * We only move current row here if there was a change this time
       * in the vertical offset
       */

      if (a.direction == DOWN
      &&  ui.active->voffset == voffset_original) {
         ui.active->crow += diff;
         if (ui.active->crow > max_row)
            ui.active->crow = max_row;
      }

      if (a.direction == UP
      &&  ui.active->voffset == 0 && voffset_original == 0) {
         ui.active->crow -= diff;
         if (ui.active->crow < 0)
            ui.active->crow = 0;
      }
   }

   redraw_active();
   paint_status_bar();
}

void
jumpto_page(Args a)
{
   int n;
   int max_row;

   /* get offset */
   n = 0;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* determine max row number */
   if (ui.active->voffset + ui.active->h >= ui.active->nrows)
      max_row = ui.active->nrows - ui.active->voffset - 1;
   else
      max_row = ui.active->h - 1;

   /* update current row */
   switch (a.placement) {
      case TOP:
         ui.active->crow = n - 1;
         break;
      case MIDDLE:
         ui.active->crow = max_row / 2;
         break;
      case BOTTOM:
         ui.active->crow = max_row - n + 1;
         break;
      default:
         errx(1, "jumpto_page: invalid location");
   }

   /* sanitize current row */
   if (ui.active->crow < 0)
      ui.active->crow = 0;

   if (ui.active->crow >= ui.active->h)
      ui.active->crow = ui.active->h - 1;

   if (ui.active->crow > max_row)
      ui.active->crow = max_row;

   /* redraw */
   redraw_active();
   paint_status_bar();
}

void
jumpto_file(Args a)
{
   float pct;
   int   n, line, input;

   /* determine line/percent to jump to */
   n = -1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* get line number to jump to */
   switch (a.scale) {
      case NUMBER:
         switch (a.num) {
            case 'g':
               /* retrieve second 'g' (or cancel) and determine jump-point */
               while ((input = getch()) && input != 'g') {
                  if (input != ERR) {
                     ungetch(input);
                     return;
                  }
               }

               if (n < 0)
                  line = 1;
               else if (n >= ui.active->nrows)
                  line = ui.active->nrows;
               else
                  line = n;

               break;

            case 'G':
               /* determine jump-point */
               if (n < 0 || n >= ui.active->nrows)
                  line = ui.active->nrows;
               else
                  line = n;

               break;

            default:
               errx(1, "jumpto_file: NUMBER type with no num!");
         }

         break;

      case PERCENT:
         if (n <= 0 || n > 100)
            return;

         pct = (float) n / 100;
         line = pct * (float) ui.active->nrows;
         break;

      default:
         errx(1, "jumpto_file: invalid scale");
   }

   /* jump */
   if (line <= ui.active->voffset) {
      swindow_scroll(ui.active, UP, ui.active->voffset - line + 1);
      ui.active->crow = 0;
   } else if (line > ui.active->voffset + ui.active->h) {
      swindow_scroll(ui.active, DOWN, line - (ui.active->voffset + ui.active->h));
      ui.active->crow = ui.active->h - 1;
   } else {
      ui.active->crow = line - ui.active->voffset - 1;
   }

   /* redraw */
   redraw_active();

   if (a.num != -1)   /* XXX a damn dirty hack for now. see search_find */
      paint_status_bar();
}

void
search(Args a)
{
   const char *errmsg = NULL;
   Args   find_args;
   char  *search_phrase;
   char  *prompt = NULL;
   char **argv  = NULL;
   int    argc = 0;
   int    i;

   /* determine prompt to use */
   switch (a.direction) {
      case FORWARDS:
         prompt = "/";
         break;
      case BACKWARDS:
         prompt = "?";
         break;
      default:
         errx(1, "search: invalid direction");
   }

   /* get search phrase from user */
   if (user_getstr(prompt, &search_phrase) != 0) {
      wclear(ui.command);
      wrefresh(ui.command);
      return;
   }

   /* set the global query description and the search direction */
   if (str2argv(search_phrase, &argc, &argv, &errmsg) != 0) {
      free(search_phrase);
      paint_error("parse error: %s in '%s'", errmsg, search_phrase);
      return;
   }

   /* setup query */
   mi_query_clear();
   mi_query_setraw(search_phrase);
   for (i = 0; i < argc; i++)
      mi_query_add_token(argv[i]);

   search_dir_set(a.direction);
   argv_free(&argc, &argv);
   free(search_phrase);

   /* do the search */
   find_args.direction = SAME;
   search_find(find_args);
}

void
search_find(Args a)
{
   Args  foo;
   bool  matches;
   char *msg;
   int   dir;
   int   start_idx;
   int   idx;
   int   c;

   /* determine direction to do the search */
   switch (a.direction) {
      case SAME:
         dir = search_dir_get();
         break;

      case OPPOSITE:
         if (search_dir_get() == FORWARDS)
            dir = BACKWARDS;
         else
            dir = FORWARDS;
         break;

      default:
         errx(1, "search_find: invalid direction");
   }

   /* start looking from current row */
   start_idx = ui.active->voffset + ui.active->crow;
   msg = NULL;
   for (c = 1; c < ui.active->nrows + 1; c++) {

      /* get idx of record */
      if (dir == FORWARDS)
         idx = start_idx + c;
      else
         idx = start_idx - c;

      /* normalize idx */
      if (idx < 0) {
         idx = ui.active->nrows + idx;
         msg = "search hit TOP, continuing at BOTTOM";

      } else if (idx >= ui.active->nrows) {
         idx %= ui.active->nrows;
         msg = "search hit BOTTOM, continuing at TOP";
      }

      /* check if record at idx matches */
      if (ui.active == ui.library)
         matches = str_match_query(mdb.playlists[idx]->name);
      else
         matches = mi_match(viewing_playlist->files[idx]);

      /* found one, jump to it */
      if (matches) {
         if (msg != NULL)
            paint_message(msg);

         gnum_set(idx + 1);
         foo.scale = NUMBER;
         foo.num = 'G';
         jumpto_file(foo);
         return;
      }
   }

   paint_error("Pattern not found: %s", mi_query_getraw());
}


/*****************************************************************************
 * yank, delete, and paste keybindings.
 ****************************************************************************/

void
yank(Args a UNUSED)
{
   bool got_target;
   int  start, end;
   int  input;
   int  n;

   if (ui.active == ui.library) {
      paint_error("cannot yank in library window");
      return;
   }

   if (viewing_playlist->nfiles == 0) {
      paint_error("nothing to yank!");
      return;
   }

   /* determine range */
   n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* get next input from user */
   got_target = false;
   start = 0;
   end = 0;
   while ((input = getch()) && !got_target) {
      if (input == ERR)
         continue;

      switch (input) {
         case 'y':   /* yank next n lines */
            start = ui.active->voffset + ui.active->crow;
            end = start + n;
            got_target = true;
            break;

         case 'G':   /* yank to end of current playlist */
            start = ui.active->voffset + ui.active->crow;
            end = ui.active->nrows;
            got_target = true;
            break;

         /*
          * TODO handle other directions ( j/k/H/L/M/^u/^d/ etc. )
          * here.  this will be ... tricky.
          * might want to re-organize other stuff?
          */

         default:
            ungetch(input);
            return;
      }
   }

   /* sanitize start and end */
   if (end > ui.active->nrows)
      end = ui.active->nrows;

   /* clear existing yank buffer and add new stuff */
   ybuffer_clear();
   for (n = start; n < end; n++)
      ybuffer_add(viewing_playlist->files[n]);

   /* notify user # of rows yanked */
   paint_message("Yanked %d files.", end - start);
}

/*
 * TODO so much of the logic in cut() is similar with that of yank() above.
 * combine the two, use an Args parameter to determine if the operation is
 * a yank or a delete
 */
void
cut(Args a UNUSED)
{
   playlist *p;
   char *warning;
   bool  got_target;
   int   start, end;
   int   response;
   int   input;
   int   n;

   /* determine range */
   n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* get range */
   got_target = false;
   start = 0;
   end = 0;
   while ((input = getch()) && !got_target) {
      if (input == ERR)
         continue;

      switch (input) {
         case 'd':   /* delete next n lines */
            start = ui.active->voffset + ui.active->crow;
            end = start + n;
            got_target = true;
            break;

         case 'G':   /* delete to end of current playlist */
            start = ui.active->voffset + ui.active->crow;
            end = ui.active->nrows;
            got_target = true;
            break;

         default:
            ungetch(input);
            return;
      }
   }

   if (start >= ui.active->nrows) {
      paint_message("nothing to delete here!");
      return;
   }

   /* handle deleting of a whole playlist */
   if (ui.active == ui.library) {
      /* get playlist to delete */
      n = ui.active->voffset + ui.active->crow;
      p = mdb.playlists[n];

      /*
       * XXX i simply do NOT want to be able to delete multiple playlists
       * while drunk.
       */
      if (end != start + 1) {
         paint_error("cannot delete multiple playlists");
         return;
      }
      if (p == mdb.library || p == mdb.filter_results) {
         paint_error("cannot delete pseudo-playlists like LIBRARY or FILTER");
         return;
      }

      asprintf(&warning, "Are you sure you want to delete '%s'?", p->name);
      if (warning == NULL)
         err(1, "cut: asprintf(3) failed");

      /* make sure user wants this */
      if (user_get_yesno(warning, &response) != 0) {
         paint_message("delete of '%s' cancelled", p->name);
         return;
      }
      if (response != 1) {
         paint_message("playlist '%s' not deleted", p->name);
         return;
      }

      /* delete playlist and redraw library window */
      if (viewing_playlist == p) {
         viewing_playlist = mdb.library;
         ui.playlist->nrows = mdb.library->nfiles;
         ui.playlist->crow = 0;
         ui.playlist->voffset = 0;
         ui.playlist->hoffset = 0;
         paint_playlist();
      }
      ui.library->nrows--;
      if (ui.library->voffset + ui.library->crow >= ui.library->nrows)
         ui.library->crow = ui.library->nrows - ui.library->voffset - 1;

      medialib_playlist_remove(n);
      paint_library();
      free(warning);
      return;
   }

   /* return to handling of deleting files in a playlist... */

   /* can't delete from library */
   if (viewing_playlist == mdb.library) {
      paint_error("cannot delete from library");
      return;
   }

   /* sanitize start and end */
   if (end > ui.active->nrows)
      end = ui.active->nrows;

   /* clear existing yank buffer and add new stuff */
   ybuffer_clear();
   for (n = start; n < end; n++)
      ybuffer_add(viewing_playlist->files[n]);

   /* delete files */
   playlist_files_remove(viewing_playlist, start, end - start, true);

   /* update ui appropriately */
   viewing_playlist->needs_saving = true;
   ui.active->nrows = viewing_playlist->nfiles;
   if (ui.active->voffset + ui.active->crow >= ui.active->nrows)
      ui.active->crow = ui.active->nrows - ui.active->voffset - 1;


   /* redraw */
   paint_playlist();
   paint_message("%d fewer files.", end - start);
}

void
paste(Args a)
{
   playlist *p;
   int start = 0;
   int i;

   if (_yank_buffer.nfiles == 0) {
      paint_error("nothing to paste");
      return;
   }

   /* determine the playlist we're pasting into */
   if (ui.active == ui.library) {
      i = ui.active->voffset + ui.active->crow;
      p = mdb.playlists[i];
   } else {
      p = viewing_playlist;
   }

   /* can't alter library */
   if (p == mdb.library) {
      paint_error("Cannot alter %s psuedo-playlist", mdb.library->name);
      return;
   }

   if (ui.active == ui.library) {
      /* figure out where to paste into playlist */
      switch (a.placement) {
         case BEFORE:
            start = 0;
            break;
         case AFTER:
            start = p->nfiles;
            break;
         default:
            errx(1, "paste: invalid placement [if]");
      }

   } else {
      /* determine index in playlist to insert new files after */
      switch (a.placement) {
         case BEFORE:
            start = ui.active->voffset + ui.active->crow;
            break;
         case AFTER:
            start = ui.active->voffset + ui.active->crow + 1;
            if (start > p->nfiles) start = p->nfiles;
            break;
         default:
            errx(1, "paste: invalid placement [else]");
      }
   }

   /* add files */
   playlist_files_add(p, _yank_buffer.files, start, _yank_buffer.nfiles, true);

   if (p == viewing_playlist)
      ui.playlist->nrows = p->nfiles;

   p->needs_saving = true;

   /* redraw */
   paint_library();
   paint_playlist();
   if (ui.active == ui.library)
      paint_message("Pasted %d files to '%s'", _yank_buffer.nfiles, p->name);
   else
      paint_message("Pasted %d files.", _yank_buffer.nfiles);
}


void
undo(Args a UNUSED)
{
   if (ui.active == ui.library) {
      paint_message("Cannot undo in library window.");
      return;
   }

   if (playlist_undo(viewing_playlist) != 0)
      paint_message("Nothing to undo.");
   else
      paint_message("Undo successfull.");

   /* TODO more informative message like in vim */

   ui.playlist->nrows = viewing_playlist->nfiles;
   if (ui.playlist->voffset + ui.playlist->crow >= ui.playlist->nrows)
      ui.playlist->crow = ui.playlist->nrows - ui.playlist->voffset - 1;

   paint_playlist();
}

void
redo(Args a UNUSED)
{
   if (ui.active == ui.library) {
      paint_message("Cannot redo in library window.");
      return;
   }

   if (playlist_redo(viewing_playlist) != 0)
      paint_message("Nothing to redo.");
   else
      paint_message("Redo successfull.");

   /* TODO */

   ui.playlist->nrows = viewing_playlist->nfiles;
   if (ui.playlist->voffset + ui.playlist->crow >= ui.playlist->nrows)
      ui.playlist->crow = ui.playlist->nrows - ui.playlist->voffset - 1;

   paint_playlist();
}

