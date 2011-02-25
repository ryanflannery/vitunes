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

#include "commands.h"

bool sorts_need_saving = false;

#define swap(type, x, y) \
   { type temp = x; x = y; y = temp;}

/*
 * List of command-mode commands.  Take note of the following: XXX
 *    1. See 'match_cmd_name()' for the handling of abbreviations.
 *    2. Commands that accept a '!' after their names are handled
 *       in 'match_cmd_name()'.
 */
const cmd CommandPath[] = { 
   {  "bind",     cmd_bind },
   {  "color",    cmd_color },
   {  "display",  cmd_display },
   {  "filter",   cmd_filter },
   {  "mode",     cmd_mode },
   {  "new",      cmd_new },
   {  "playlist", cmd_playlist },
   {  "q",        cmd_quit },
   {  "reload",   cmd_reload },
   {  "set",      cmd_set },
   {  "sort",     cmd_sort },
   {  "unbind",   cmd_unbind },
   {  "w",        cmd_write },
   {  "toggle",   cmd_toggle }
};
const int CommandPathSize = (sizeof(CommandPath) / sizeof(cmd));


/****************************************************************************
 * Toggleset related stuff
 ***************************************************************************/

toggle_list **toggleset;
size_t        toggleset_size;

void
toggleset_init()
{
   const int max_size = 52;  /* since we only have registers a-z and A-Z */
   if ((toggleset = calloc(max_size, sizeof(toggle_list*))) == NULL)
      err(1, "%s: calloc(3) failed", __FUNCTION__);

   toggleset_size = 0;
}

void
toggleset_free()
{
   size_t i;
   for (i = 0; i < toggleset_size; i++)
      toggle_list_free(toggleset[i]);

   free(toggleset);
   toggleset_size = 0;
}

void
toggle_list_add_command(toggle_list *t, char *cmd)
{
   char **new_cmds;
   int    idx, new_size;

   /* resize array */
   if (t->size == 0) {
      if ((t->commands = malloc(sizeof(char*))) == NULL)
         err(1, "%s: malloc(3) failed", __FUNCTION__);

      idx = 0;
      t->size = 1;
   } else {
      new_size = (t->size + 1) * sizeof(char*);
      if ((new_cmds = realloc(t->commands, new_size)) == NULL)
         err(1, "%s: realloc(3) failed", __FUNCTION__);

      idx = t->size;
      t->commands = new_cmds;
      t->size++;
   }

   /* add command */
   if ((t->commands[idx] = strdup(cmd)) == NULL)
      err(1, "%s: strdup(3) failed", __FUNCTION__);
}

toggle_list*
toggle_list_create(int registr, int argc, char *argv[])
{
   toggle_list *t;
   char *cmd = NULL;
   int   i, j;

   if ((t = malloc(sizeof(toggle_list))) == NULL)
      err(1, "%s: malloc(3) failed", __FUNCTION__);

   t->commands = NULL;
   t->registr  = registr;
   t->size     = 0;

   /* parse the argv into the toggle list */
   for (i = 0; i < argc; i++) {
      if (!strcmp("/", argv[i]))
         continue;

      /* count number strings in this command and determine length */
      for (j = i; j < argc && strcmp("/", argv[j]); j++)

      /* now collapse them into a single string */
      cmd = argv2str(j - i + 1, argv + i);
      toggle_list_add_command(t, cmd);
      free(cmd);

      i += (j - i) - 1;
   }

   t->index = t->size - 1;
   return t;
}

void
toggle_list_free(toggle_list *t)
{
   size_t i;

   for (i = 0; i < t->size; i++)
      free(t->commands[i]);

   free(t);
}

void
toggle_add(toggle_list *t)
{
   if (toggle_get(t->registr) != NULL)
      toggle_remove(t->registr);

   toggleset[toggleset_size++] = t;
}

void
toggle_remove(int registr)
{
   size_t i, idx;
   bool   found;

   found = false;
   idx = 0;
   for (i = 0; i < toggleset_size; i++) {
      if (toggleset[i]->registr == registr) {
         idx = i;
         found = true;
      }
   }

   if (!found) return;

   for (i = idx; i < toggleset_size - 1; i++)
      toggleset[i] = toggleset[i + 1];

   toggleset_size--;
}

toggle_list*
toggle_get(int registr)
{
   size_t i;

   for (i = 0; i < toggleset_size; i++) {
      if (toggleset[i]->registr == registr)
         return toggleset[i];
   }

   return NULL;
}


/****************************************************************************
 * Misc handy functions
 ***************************************************************************/

void
setup_viewing_playlist(playlist *p)
{
   viewing_playlist = p;

   ui.playlist->nrows   = p->nfiles;
   ui.playlist->crow    = 0;
   ui.playlist->voffset = 0;
   ui.playlist->hoffset = 0;
}

int
str2bool(const char *s, bool *b)
{
   /* check true/t/yes/y */
   if (strcasecmp(s, "true") == 0 
   ||  strcasecmp(s, "t") == 0 
   ||  strcasecmp(s, "yes") == 0 
   ||  strcasecmp(s, "y") == 0) { 
      *b = true;
      return 0;
   }
   
   /* check false/f/no/n */
   if (strcasecmp(s, "false") == 0 
   ||  strcasecmp(s, "f") == 0 
   ||  strcasecmp(s, "no") == 0 
   ||  strcasecmp(s, "n") == 0) { 
      *b = false;
      return 0;
   }

   return -1;
}


/****************************************************************************
 * Command handlers
 ***************************************************************************/

int
cmd_quit(int argc, char *argv[])
{
   bool forced;
   int  i;

   if (argc != 1) {
      paint_error("usage: q[!]");
      return 1;
   }

   /* is this a forced quit? */
   forced = (strcmp(argv[0], "q!") == 0);

   /* check if there are any unsaved changes if not forced */
   if (!forced) {
      for (i = 0; i < mdb.nplaylists; i++) {
         if (mdb.playlists[i]->needs_saving) {
            paint_error("there are playlists with unsaved changes.  use \"q!\" to force.");
            return 2;
         }
      }
   }

   VSIG_QUIT = 1;
   return 0;
}

int
cmd_write(int argc, char *argv[])
{
   playlist *dup;
   char     *filename;
   bool      forced;
   bool      will_clobber;
   int       i, clobber_index;

   if (argc > 2) {
      paint_error("usage: w[!] [name]");
      return 1;
   }

   forced = (strcmp(argv[0], "w!") == 0);

   if (argc == 1) {  /* "save" */

      /* can't save library or filter results */
      if (viewing_playlist == mdb.library
      ||  viewing_playlist == mdb.filter_results) {
         paint_error("use \"w name\" when saving psuedo-playlists like library/filter");
         return 2;
      }

      /* can't save a new playlist that has no name */
      if (viewing_playlist->filename == NULL) {
         paint_error("use \"w name\" for new playlists");
         return 3;
      }

      /* do the save... */
      playlist_save(viewing_playlist);
      viewing_playlist->needs_saving = false;
      paint_library();
      paint_message("\"%s\" %d songs written",
         viewing_playlist->filename, viewing_playlist->nfiles);

   } else { /* "save as" */

      /* build filename for playlist */
      asprintf(&filename, "%s/%s.playlist", mdb.playlist_dir, argv[1]);
      if (filename == NULL)
         err(1, "cmd_write: asprintf(3) failed");

      /* check to see if playlist with that name already exists */
      will_clobber = false;
      clobber_index = -1;
      for (i = 0; i < mdb.nplaylists; i++) {
         if (mdb.playlists[i]->filename != NULL
         &&  strcmp(mdb.playlists[i]->filename, filename) == 0) {
            will_clobber = true;
            clobber_index = i;
         }
      }

      if (will_clobber && !forced) {
         paint_error("playlist with that name exists (use \"w!\" to overwrite)");
         free(filename);
         return 4;
      }

      /* if reached here, we're going to do the save-as... */

      /* duplicate playlist */
      dup = playlist_dup(viewing_playlist, filename, argv[1]);
      if (will_clobber)
         medialib_playlist_remove(clobber_index);
      else
         ui.library->nrows++;

      /*
       * TODO If the original playlist was a new one, with no name (i.e. if
       * name == NULL) then we should remove that playlist from the medialib
       * and display here.
       */

      /* do the save-as... */
      playlist_save(dup);
      medialib_playlist_add(dup);

      dup->needs_saving = false;
      viewing_playlist->needs_saving = false;

      paint_library();
      paint_message("\"%s\" %d songs written",
         filename, viewing_playlist->nfiles);
   }

   return 0;
}

int
cmd_mode(int argc, char *argv[])
{
   if (argc != 2) {
      paint_error("usage: mode [ linear | loop | random ]");
      return 1;
   }

   if (strcasecmp(argv[1], "linear") == 0)
      player_info.mode = MODE_LINEAR;
   else if (strcasecmp(argv[1], "loop") == 0)
      player_info.mode = MODE_LOOP;
   else if (strcasecmp(argv[1], "random") == 0)
      player_info.mode = MODE_RANDOM;
   else {
      paint_error("invalid mode \"%s\".  must be one of: linear, loop, or random", argv[1]);
      return 2;
   }

   paint_message("mode changed to: %s", argv[1]);
   return 0;
}

int
cmd_new(int argc, char *argv[])
{
   playlist *p;
   char     *name;
   char     *filename;
   int       i;

   if (argc > 2) {
      paint_error("usage: new [name]");
      return 1;
   }

   /* defaults */
   name = "untitled";
   filename = NULL;

   /* was a name specified? */
   if (argc == 2) {
      /* check for existing playlist with name */
      for (i = 0; i < mdb.nplaylists; i++) {
         if (strcmp(mdb.playlists[i]->name, argv[1]) == 0) {
            paint_error("playlist \"%s\" already exists.", argv[1]);
            return 2;
         }
      }

      name = argv[1];
      asprintf(&filename, "%s/%s.playlist", mdb.playlist_dir, name);
      if (filename == NULL)
         err(1, "cmd_new: asprintf(3) failed");
   }

   /* create & setup playlist */
   p = playlist_new();
   p->needs_saving = true;
   p->filename = filename;
   if ((p->name = strdup(name)) == NULL)
      err(1, "cmd_new: strdup(3) failed");

   /* add playlist to media library and update ui */
   medialib_playlist_add(p);
   ui.library->nrows++;
   if (p->filename != NULL)
      playlist_save(p);

   /* redraw */
   paint_library();
   paint_message("playlist \"%s\" added", name);

   return 0;
}

int
cmd_filter(int argc, char *argv[])
{
   playlist *results;
   char     *search_phrase;
   bool      match;
   int       i;

   if (argc == 1) {
      paint_error("usage: filter[!] token [token2 ...]");
      return 1;
   }

   /* determine what kind of filter we're doing */
   match = argv[0][strlen(argv[0]) - 1] != '!';

   /* set the raw query */
   search_phrase = argv2str(argc - 1, argv + 1);
   mi_query_setraw(search_phrase);
   free(search_phrase);

   /* clear existing global query & set new one */
   mi_query_clear();
   for (i = 1; i < argc; i++)
      mi_query_add_token(argv[i]);

   /* do actual filter */
   results = playlist_filter(viewing_playlist, match);

   /* swap necessary bits of results with filter playlist */
   swap(meta_info **, results->files,    mdb.filter_results->files);
   swap(int, results->nfiles,   mdb.filter_results->nfiles);
   swap(int, results->capacity, mdb.filter_results->capacity);
   playlist_free(results);

   /* redraw */
   setup_viewing_playlist(mdb.filter_results);
   paint_library();
   paint_playlist();

   return 0;
}

int
cmd_sort(int argc, char *argv[])
{
   const char *errmsg;

   if (argc != 2) {
      paint_error("usage: sort <sort-description>");
      return 1;
   }

   /* setup global sort description */
   if (mi_sort_set(argv[1], &errmsg) != 0) {
      paint_error("%s: bad sort description: %s", argv[0], errmsg);
      return 2;
   }

   /* do the actual sort */
   qsort(viewing_playlist->files, viewing_playlist->nfiles,
      sizeof(meta_info*), mi_compare);

   if(!ui_is_init())
      return 0;

   /* redraw */
   paint_playlist();

   /* if we sorted a playlist other than library, and user wants to save sorts */
   if (viewing_playlist != mdb.library && sorts_need_saving) {
      viewing_playlist->needs_saving = true;
      paint_library();
   }

   return 0;
}

int
cmd_display(int argc, char *argv[])
{
   const char *errmsg;

   if (argc != 2) {
      paint_error("usage: display [ reset | show | <display-description> ]");
      return 1;
   }

   /* show existng display? */
   if (strcasecmp(argv[1], "show") == 0) {
      paint_message(":display %s", mi_display_tostr());
      return 0;
   }
  
   /* reset display to default? */
   if (strcasecmp(argv[1], "reset") == 0) {
      mi_display_reset();
      paint_playlist();
      return 0;
   }

   /* if reached here, setup global display description */

   if (mi_display_set(argv[1], &errmsg) != 0) {
      paint_error("%s: bad display description: %s", argv[0], errmsg);
      return 1;
   }

   if(ui_is_init())
      paint_playlist();

   return 0;
}

int
cmd_color(int argc, char *argv[])
{
   char  *item;
   char  *fg, *bg;
   int    i_item, i_fg, i_bg, j;

   if (argc != 2) {
      paint_error("usage: %s ITEM=FG,BG", argv[0]);
      return 1;
   }

   /* extract item and foreground/background colors */
   item = argv[1];
   if ((fg = strchr(item, '=')) == NULL) {
      paint_error("usage: %s ITEM=FG,BG", argv[0]);
      return 2;
   }
   *fg = '\0';
   fg++;

   if ((bg = strchr(fg, ',')) == NULL) {
      paint_error("usage: %s ITEM=FG,BG", argv[0]);
      return 3;
   }
   *bg = '\0';
   bg++;

   /* convert all */
   if ((i_item = paint_str2item(item)) < 0) {
      paint_error("invalid item '%s'", item);
      return 4;
   }

   if ((i_fg = paint_str2color(fg)) == -2) {
      paint_error("invalid foreground color '%s'", fg);
      return 5;
   }

   if ((i_bg = paint_str2color(bg)) == -2) {
      paint_error("invalid background color '%s'", bg);
      return 6;
   }

   /* init color */
   init_pair(i_item, i_fg, i_bg);

   /* if this was a cinfo item, indicate that it was set */
   for (j = 0; j < MI_NUM_CINFO; j++) {
      if (i_item == colors.cinfos[j])
         colors.cinfos_set[j] = true;
   }

   /* redraw */
   if (ui_is_init()) {
      ui_clear();
      paint_all();
   }

   return 0;
}

int
cmd_set(int argc, char *argv[])
{
   const char *err;
   char *property;
   char *value;
   bool  tf;
   int   max_w, max_h, new_width;   /* lwidth */

   if (argc != 2) {
      paint_error("usage: %s <property>=<value>", argv[0]);
      return 1;
   }

   /* extract property and value */
   property = argv[1];
   if ((value = strchr(property, '=')) == NULL) {
      paint_error("usage: %s <property>=<value>", argv[0]);
      return 2;
   }
   *value = '\0';
   value++;

   /* handle property */

   if (strcasecmp(property, "lwidth") == 0) {
      /* get max width and height */
      getmaxyx(stdscr, max_h, max_w);

      /* validate and convert width user provided */
      new_width = (int)strtonum(value, 1, max_w, &err);
      if (err != NULL) {
         paint_error("%s %s: bad width: '%s' %s",
            argv[0], property, value, err);
         return 3;
      }

      /* redraw */
      ui.lwidth = new_width;
      ui_resize();
      if(ui_is_init()) {
         ui_clear();
         paint_all();
      }

   } else if (strcasecmp(property, "lhide") == 0) {
      if (str2bool(value, &tf) < 0) {
         paint_error("%s %s: value must be boolean",
            argv[0], property);
         return 4;
      }
      ui.lhide = tf;
      if (ui.lhide) {
         if (ui.active == ui.playlist) ui_hide_library();
         paint_all();
         paint_message("library window hidden");
      } else {
         if (ui.library->cwin == NULL) ui_unhide_library();
         paint_all();
         paint_message("library window un-hidden");
      }

   } else if (strcasecmp(property, "match-fname") == 0) {
      if (str2bool(value, &tf) < 0) {
         paint_error("%s %s: value must be boolean",
            argv[0], property);
         return 5;
      }
      mi_query_match_filename = tf;
      if (mi_query_match_filename)
         paint_message("filenames will be matched against");
      else
         paint_message("filenames will NOT be matched against");

   } else if (strcasecmp(property, "save-sorts") == 0) {
      if (str2bool(value, &tf) < 0) {
         paint_error("%s %s: value must be boolean",
            argv[0], property);
         return 6;
      }
      sorts_need_saving = tf;
      if (sorts_need_saving)
         paint_message("changing sort will be prompted for saving");
      else
         paint_message("changing sort will NOT be prompted for saving");

   } else {
      paint_error("%s: unknown property '%s'", argv[0], property);
      return 7;
   }

   return 0;
}

int
cmd_reload(int argc, char *argv[])
{
   char *db_file;
   char *playlist_dir;

   if (argc != 2) {
      paint_error("usage: %s [ db | conf ]", argv[0]);
      return 1;
   }

   /* reload database or config file */
   if (strcasecmp(argv[1], "db") == 0) {

      db_file = strdup(mdb.db_file);
      playlist_dir = strdup(mdb.playlist_dir);
      if (db_file == NULL || playlist_dir == NULL)
         err(1, "cmd_reload: strdup(3) failed");

      /* stop playback TODO investigate a nice way around this */
      player_stop();

      /* reload db */
      medialib_destroy();
      medialib_load(db_file, playlist_dir);

      free(db_file);
      free(playlist_dir);

      /* re-setup ui basics */
      playing_playlist = NULL;
      setup_viewing_playlist(mdb.library);
      ui.library->voffset = 0;
      ui.library->nrows = mdb.nplaylists;
      ui.library->crow = 0;
      paint_all();

   } else if (strcasecmp(argv[1], "conf") == 0)
      load_config();
   else {
      paint_error("usage: %s [ db | conf ]", argv[0]);
      return 2;
   }

   return 0;
}

int
cmd_bind(int argc, char *argv[])
{
   KeyAction action;
   KeyCode code;

   if (argc < 3 || argc > 4) {
      paint_error("usage: %s <action> <keycode>", argv[0]);
      return 1;
   }

   if (!kb_str2action(argv[1], &action)) {
      paint_error("Unknown action '%s'", argv[1]);
      return 1;
   }

   if (argc == 3) {
      if ((code = kb_str2keycode(argv[2])) < 0) {
         paint_error("Invalid keycode '%s'", argv[2]);
         return 1;
      }
   } else {
      if ((code = kb_str2keycode2(argv[2], argv[3])) < 0) {
         paint_error("Invalid keycode '%s'", argv[2]);
         return 1;
      }
   }

   kb_bind(action, code);
   return 0;
}

int
cmd_unbind(int argc, char *argv[])
{
   KeyAction action;
   KeyCode   key;

   /* unbind all case ("unbind *") */
   if (argc == 2 && strcmp(argv[1], "*") == 0) {
      kb_unbind_all();
      return 0;
   }

   /* unbind action case ("unbind action <ACTION>") */
   if (argc == 3 && strcasecmp(argv[1], "action") == 0) {
      if (kb_str2action(argv[2], &action)) {
         kb_unbind_action(action);
         return 0;
      } else {
         paint_error("Unknown action '%s'", argv[2]);
         return 1;
      }
   }

   /* unbind key case, no control ("unbind key X") */
   if (argc == 3 && strcasecmp(argv[1], "key") == 0) {
      if ((key = kb_str2keycode(argv[2])) < 0) {
         paint_error("Invalid keycode '%s'", argv[2]);
         return 1;
      }

      kb_unbind_key(key);
      return 0;
   }

   /* unbind key case, with control ("unbind key control X") */
   if (argc == 4 && strcasecmp(argv[1], "key") == 0) {
      if ((key = kb_str2keycode2(argv[2], argv[3])) < 0) {
         paint_error("Invalid keycode '%s %s'", argv[2], argv[3]);
         return 1;
      }

      kb_unbind_key(key);
      return 0;
   }

   paint_error("usage: unbind [* | action <ACTION> | key <KEYCODE> ]");
   return 1;
}

int
cmd_toggle(int argc, char *argv[])
{
   toggle_list *t;
   char **cmd_argv;
   int    cmd_argc;
   int    registr;

   if (argc < 3) {
      paint_error("usage: %s <register> <action1> / ...", argv[0]);
      return 1;
   }

   if (strlen(argv[1]) != 1) {
      paint_error("error: register name must be a single letter (in [a-zA-Z])");
      return 1;
   }

   registr = *(argv[1]);

   if (!( ('a' <= registr && registr <= 'z')
   ||     ('A' <= registr && registr <= 'Z'))) {
      paint_error("error: invalid register name.  Must be one of [a-zA-Z]");
      return 1;
   }

   cmd_argc = argc - 2;
   cmd_argv = argv + 2;
   t = toggle_list_create(registr, cmd_argc, cmd_argv);
   toggle_add(t);
   return 0;
}

int
cmd_playlist(int argc, char *argv[])
{
   int x;
   int idx = -1;

   if (argc != 2) {
      paint_error("usage: playlist <list-name>");
      return 1;
   }

   for(x = 0; x < mdb.nplaylists; x++) {
      if(!strncmp(argv[1], mdb.playlists[x]->name, strlen(argv[1]))) {
         if(idx > -1) {
            idx = -2;
            break;
         }

         if(idx == -1)
            idx = x;
      }
   }

   if(idx > -1) {
      setup_viewing_playlist(mdb.playlists[idx]);
      ui.active = ui.playlist;
      paint_all();
      paint_message("jumped to playlist: %s", mdb.playlists[idx]->name);
      return 0;
   }

   if(idx == -1) {
      paint_error("no match for: %s", argv[1]);
      return 0;
   }

   if(idx == -2)
      paint_error("no unique match for: %s", argv[1]);

   return 0;
}

void
cmd_execute(char *cmd)
{
   const char *errmsg = NULL;
   bool   found;
   char **argv;
   int    argc;
   int    found_idx = 0;
   int    num_matches;
   int    i;

   if (str2argv(cmd, &argc, &argv, &errmsg) != 0) {
      paint_error("parse error: %s in '%s'", errmsg, cmd);
      return;
   }

   found = false;
   num_matches = 0;
   for (i = 0; i < CommandPathSize; i++) {
      if (match_command_name(argv[0], CommandPath[i].name)) {
         found = true;
         found_idx = i;
         num_matches++;
      }
   }

   if (found && num_matches == 1)
      (CommandPath[found_idx].func)(argc, argv);
   else if (num_matches > 1)
      paint_error("Ambiguous abbreviation '%s'", argv[0]);
   else
      paint_error("Unknown commands '%s'", argv[0]);

   argv_free(&argc, &argv);
}


/*****************************************************************************
 * command window input methods
 ****************************************************************************/

/*
 * Note: Both of these return 0 if input was successfull, and something else
 * (1) if the user cancelled the input (such as, by hitting ESCAPE)
 */

int
user_getstr(const char *prompt, char **response)
{
   const int MAX_INPUT_SIZE = 1000; /* TODO remove this limit */
   char *input;
   int  pos, ch, ret;

   /* display the prompt */
   werase(ui.command);
   mvwprintw(ui.command, 0, 0, "%s", prompt);

   /* position the cursor */
   curs_set(1);
   wmove(ui.command, 0, strlen(prompt));
   wrefresh(ui.command);

   /* allocate input space and clear */
   if ((input = calloc(MAX_INPUT_SIZE, sizeof(char))) == NULL)
      err(1, "user_getstr: calloc(3) failed for input string");

   bzero(input, MAX_INPUT_SIZE);

   /* start getting input */
   ret = 0;
   pos = 0;
   while ((ch = getch()) && !VSIG_QUIT) {

      /*
       * Handle any signals.  Note that the use of curs_set, wmvoe, and
       * wrefresh here are all necessary to ensure that the cursor does
       * not show anywhere outside of the command window.
       */
      curs_set(0);
      process_signals();
      curs_set(1);
      wmove(ui.command, 0, strlen(prompt) + pos);
      wrefresh(ui.command);

      if (ch == ERR)
         continue;

      if (ch == '\n' || ch == 13)
         break;

      /* handle 'escape' */
      if (ch == 27) {
         ret = 1;
         goto end;
      }

      /* handle 'backspace' / left-arrow, etc. */
      if (ch == 127    || ch == KEY_BACKSPACE || ch == KEY_LEFT
       || ch == KEY_DC || ch == KEY_SDC) {
         if (pos == 0) {
            if (ch == KEY_BACKSPACE) {
               ret = 1;
               goto end;
            }
            beep();
         } else {
            mvwaddch(ui.command, 0, strlen(prompt) + pos - 1, ' ');
            wmove(ui.command, 0, strlen(prompt) + pos - 1);
            wrefresh(ui.command);
            pos--;
         }
         continue;
      }

      /* got regular input.  add to buffer. */
      input[pos] = ch;
      mvwaddch(ui.command, 0, strlen(prompt) + pos, ch);
      wrefresh(ui.command);
      pos++;

      /* see todo above - realloc input buffer here if position reaches max */
      if (pos >= MAX_INPUT_SIZE)
         errx(1, "user_getstr: shamefull limit reached");
   }

   /* For lack of input, bail out */
   if (pos == 0) {
      ret = 1;
      goto end;
   }

   /* NULL-terminate and trim off trailing whitespace */
   input[pos--] = '\0';
   for (; input[pos] == ' ' && pos >= 0; pos--)
      input[pos] = '\0';

   /* trim the fat */
   if ((*response = calloc(strlen(input) + 1, sizeof(char))) == NULL)
      err(1, "user_getstr: calloc(3) failed for result");

   snprintf(*response, strlen(input) + 1, "%s", input);

end:
   free(input);
   curs_set(0);
   return ret;
}

int
user_get_yesno(const char *msg, int *response)
{
   char *answer;

   if (user_getstr(msg, &answer) != 0)
      return 1;

   if (strncasecmp(answer, "yes", 3) == 0
   ||  strncasecmp(answer, "y",   1) == 0)
      *response = 1;
   else if (strncasecmp(answer, "no", 2) == 0
         || strncasecmp(answer, "n",  1) == 0)
      *response = 0;
   else
      *response = -1;

   free(answer);
   return 0;
}

