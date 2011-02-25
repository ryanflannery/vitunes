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

#include "keybindings.h"


/* This table maps KeyActions to their string representations */
typedef struct {
   KeyAction   action;
   char       *name;
} KeyActionName;

const KeyActionName KeyActionNames[] = {
   { scroll_up,               "scroll_up" },
   { scroll_down,             "scroll_down" },
   { scroll_up_page,          "scroll_up_page" },
   { scroll_down_page,        "scroll_down_page" },
   { scroll_up_halfpage,      "scroll_up_halfpage" },
   { scroll_down_halfpage,    "scroll_down_halfpage" },
   { scroll_up_wholepage,     "scroll_up_wholepage" },
   { scroll_down_wholepage,   "scroll_down_wholepage" },
   { scroll_left,             "scroll_left" },
   { scroll_right,            "scroll_right" },
   { scroll_leftmost,         "scroll_leftmost" },
   { scroll_rightmost,        "scroll_rightmost" },
   { jumpto_screen_top,       "jumpto_screen_top" },
   { jumpto_screen_middle,    "jumpto_screen_middle" },
   { jumpto_screen_bottom,    "jumpto_screen_bottom" },
   { jumpto_line,             "jumpto_line" },
   { jumpto_percent,          "jumpto_percent" },
   { search_forward,          "search_forward" },
   { search_backward,         "search_backward" },
   { find_next_forward,       "find_next_forward" },
   { find_next_backward,      "find_next_backward" },
   { visual,                  "visual" },
   { cut,                     "cut" },
   { yank,                    "yank" },
   { paste_after,             "paste_after" },
   { paste_before,            "paste_before" },
   { undo,                    "undo" },
   { redo,                    "redo" },
   { go,                      "go" },
   { quit,                    "quit" },
   { redraw,                  "redraw" },
   { command_mode,            "command_mode" },
   { shell,                   "shell" },
   { switch_windows,          "switch_window" },
   { show_file_info,          "show_file_info" },
   { load_playlist,           "load_playlist" },
   { media_play,              "media_play" },
   { media_pause,             "media_pause" },
   { media_stop,              "media_stop" },
   { media_next,              "media_next" },
   { media_prev,              "media_prev" },
   { volume_increase,         "volume_increase" },
   { volume_decrease,         "volume_decrease" },
   { seek_forward_seconds,    "seek_forward_seconds" },
   { seek_backward_seconds,   "seek_backward_seconds" },
   { seek_forward_minutes,    "seek_forward_minutes" },
   { seek_backward_minutes,   "seek_backward_minutes" },
   { toggle_forward,          "toggle_forward" },
   { toggle_backward,         "toggle_backward" }
};
const size_t KeyActionNamesSize = sizeof(KeyActionNames) / sizeof(KeyActionName);


/* This table maps KeyActions to their handler-functions (with arguments) */
typedef struct {
   KeyAction      action;
   ActionHandler  handler;
   bool           visual;
   KbaArgs        args;
} KeyActionHandler;

#define ARG_NOT_USED { .num = 0 }
const KeyActionHandler KeyActionHandlers[] = {
{ scroll_up,             kba_scroll_row,     true,  { .direction = UP }},
{ scroll_down,           kba_scroll_row,     true,  { .direction = DOWN }},
{ scroll_up_page,        kba_scroll_page,    true,  { .direction = UP,   .amount = SINGLE }},
{ scroll_down_page,      kba_scroll_page,    true,  { .direction = DOWN, .amount = SINGLE }},
{ scroll_up_halfpage,    kba_scroll_page,    true,  { .direction = UP,   .amount = HALF }},
{ scroll_down_halfpage,  kba_scroll_page,    true,  { .direction = DOWN, .amount = HALF }},
{ scroll_up_wholepage,   kba_scroll_page,    true,  { .direction = UP,   .amount = WHOLE }},
{ scroll_down_wholepage, kba_scroll_page,    true,  { .direction = DOWN, .amount = WHOLE }},
{ scroll_left,           kba_scroll_col,     true,  { .direction = LEFT,  .amount = SINGLE }},
{ scroll_right,          kba_scroll_col,     true,  { .direction = RIGHT, .amount = SINGLE }},
{ scroll_leftmost,       kba_scroll_col,     true,  { .direction = LEFT,  .amount = WHOLE }},
{ scroll_rightmost,      kba_scroll_col,     true,  { .direction = RIGHT, .amount = WHOLE }},
{ jumpto_screen_top,     kba_jumpto_screen,  true,  { .placement = TOP }},
{ jumpto_screen_middle,  kba_jumpto_screen,  true,  { .placement = MIDDLE }},
{ jumpto_screen_bottom,  kba_jumpto_screen,  true,  { .placement = BOTTOM }},
{ jumpto_line,           kba_jumpto_file,    true,  { .scale = NUMBER, .num = 'G' }},
{ jumpto_percent,        kba_jumpto_file,    true,  { .scale = PERCENT }},
{ search_forward,        kba_search,         true,  { .direction = FORWARDS }},
{ search_backward,       kba_search,         true,  { .direction = BACKWARDS }},
{ find_next_forward,     kba_search_find,    true,  { .direction = SAME }},
{ find_next_backward,    kba_search_find,    true,  { .direction = OPPOSITE }},
{ visual,                kba_visual,         true,  ARG_NOT_USED },
{ cut,                   kba_cut,            true,  ARG_NOT_USED },
{ yank,                  kba_yank,           true,  ARG_NOT_USED },
{ paste_after,           kba_paste,          false, { .placement = AFTER }},
{ paste_before,          kba_paste,          false, { .placement = BEFORE }},
{ undo,                  kba_undo,           false, ARG_NOT_USED },
{ redo,                  kba_redo,           false, ARG_NOT_USED },
{ go,                    kba_go,             true,  ARG_NOT_USED },
{ quit,                  kba_quit,           false, ARG_NOT_USED },
{ redraw,                kba_redraw,         false, ARG_NOT_USED },
{ command_mode,          kba_command_mode,   false, ARG_NOT_USED },
{ shell,                 kba_shell,          false, ARG_NOT_USED },
{ switch_windows,        kba_switch_windows, false, ARG_NOT_USED },
{ show_file_info,        kba_show_file_info, false, ARG_NOT_USED },
{ load_playlist,         kba_load_playlist,  false, ARG_NOT_USED },
{ media_play,            kba_play,           false, ARG_NOT_USED },
{ media_pause,           kba_pause,          false, ARG_NOT_USED },
{ media_stop,            kba_stop,           false, ARG_NOT_USED },
{ media_next,            kba_play_next,      false, ARG_NOT_USED },
{ media_prev,            kba_play_prev,      false, ARG_NOT_USED },
{ volume_increase,       kba_volume,         false, { .direction = FORWARDS }},
{ volume_decrease,       kba_volume,         false, { .direction = BACKWARDS }},
{ seek_forward_seconds,  kba_seek,           false, { .direction = FORWARDS,  .scale = SECONDS, .num = 10 }},
{ seek_backward_seconds, kba_seek,           false, { .direction = BACKWARDS, .scale = SECONDS, .num = 10 }},
{ seek_forward_minutes,  kba_seek,           false, { .direction = FORWARDS,  .scale = MINUTES, .num = 1 }},
{ seek_backward_minutes, kba_seek,           false, { .direction = BACKWARDS, .scale = MINUTES, .num = 1 }},
{ toggle_forward,        kba_toggle,         false, { .direction = FORWARDS }},
{ toggle_backward,       kba_toggle,         false, { .direction = BACKWARDS }}
};
const size_t KeyActionHandlersSize = sizeof(KeyActionHandlers) / sizeof(KeyActionHandler);


/* This table contains the default keybindings */
typedef struct {
   KeyCode     key;
   KeyAction   action;
} KeyBinding;

#define MY_K_TAB     9
#define MY_K_ENTER  13
#define MY_K_ESCAPE 27
#define kb_CONTROL(x)   ((x) - 'a' + 1)

const KeyBinding DefaultKeyBindings[] = {
   { 'k',               scroll_up },
   { '-',               scroll_up },
   { KEY_UP,            scroll_up },
   { 'j',               scroll_down },
   { KEY_DOWN,          scroll_down },
   { kb_CONTROL('y'),   scroll_up_page },
   { kb_CONTROL('e'),   scroll_down_page },
   { kb_CONTROL('u'),   scroll_up_halfpage },
   { kb_CONTROL('d'),   scroll_down_halfpage },
   { kb_CONTROL('b'),   scroll_up_wholepage },
   { KEY_PPAGE,         scroll_up_wholepage },
   { kb_CONTROL('f'),   scroll_down_wholepage },
   { KEY_NPAGE,         scroll_down_wholepage },
   { 'h',               scroll_left },
   { KEY_LEFT,          scroll_left },
   { KEY_BACKSPACE,     scroll_left },
   { 'l',               scroll_right },
   { KEY_RIGHT,         scroll_right },
   { ' ',               scroll_right },
   { '^',               scroll_leftmost },
   { '0',               scroll_leftmost },
   { '|',               scroll_leftmost },
   { '$',               scroll_rightmost },
   { 'H',               jumpto_screen_top },
   { 'M',               jumpto_screen_middle },
   { 'L',               jumpto_screen_bottom },
   { 'G',               jumpto_line },
   { '%',               jumpto_percent },
   { '/',               search_forward },
   { '?',               search_backward },
   { 'n',               find_next_forward },
   { 'N',               find_next_backward },
   { 'v',               visual },
   { 'V',               visual },
   { 'd',               cut },
   { 'y',               yank },
   { 'p',               paste_after },
   { 'P',               paste_before },
   { 'u',               undo },
   { kb_CONTROL('r'),   redo },
   { 'g',               go },
   { kb_CONTROL('c'),   quit },
   { kb_CONTROL('/'),   quit },
   { kb_CONTROL('l'),   redraw },
   { ':',               command_mode },
   { '!',               shell },
   { MY_K_TAB,          switch_windows },
   { 'm',               show_file_info },
   { MY_K_ENTER,        load_playlist },
   { MY_K_ENTER,        media_play },
   { 'z',               media_pause },
   { 's',               media_stop },
   { 'f',               seek_forward_seconds },
   { ']',               seek_forward_seconds },
   { 'b',               seek_backward_seconds },
   { '[',               seek_backward_seconds },
   { 'F',               seek_forward_minutes },
   { '}',               seek_forward_minutes },
   { 'B',               seek_backward_minutes },
   { '{',               seek_backward_minutes },
   { '(',               media_prev },
   { ')',               media_next },
   { '<',               volume_decrease },
   { '>',               volume_increase },
   { 't',               toggle_forward },
   { 'T',               toggle_backward }
};
const size_t DefaultKeyBindingsSize = sizeof(DefaultKeyBindings) / sizeof(KeyBinding);


/* Mapping of special keys to their values */
typedef struct {
   char    *str;
   KeyCode  code;
} SpecialKeyCode;
SpecialKeyCode SpecialKeys[] = {
   { "BACKSPACE", KEY_BACKSPACE },
   { "ENTER",     MY_K_ENTER },
   { "SPACE",     ' ' },
   { "TAB",       MY_K_TAB },
   { "UP",        KEY_UP },
   { "DOWN",      KEY_DOWN },
   { "LEFT",      KEY_LEFT },
   { "RIGHT",     KEY_RIGHT },
   { "PAGEUP",    KEY_PPAGE },
   { "PAGEDOWN",  KEY_NPAGE }
};
const size_t SpecialKeysSize = sizeof(SpecialKeys) / sizeof(SpecialKeyCode);


/* 
 * This is the actual keybinding table mapping KeyCodes to actions.  It is
 * loaded at initial runtime (kb_init()), and modified thereafter, either via
 * "bind" commands in the config file or issued during runtime.
 */
#define KEYBINDINGS_CHUNK_SIZE 100
KeyBinding *KeyBindings;
size_t KeyBindingsSize;
size_t KeyBindingsCapacity;

void
kb_increase_capacity()
{
   KeyBinding  *new_buffer;
   size_t       nbytes;

   KeyBindingsCapacity += KEYBINDINGS_CHUNK_SIZE;
   nbytes = KeyBindingsCapacity * sizeof(KeyBinding);
   if ((new_buffer = realloc(KeyBindings, nbytes)) == NULL)
      err(1, "%s: failed to realloc(3) keybindings", __FUNCTION__);

   KeyBindings = new_buffer;
}


/****************************************************************************
 *
 *        Routines for initializing, free'ing, manipulating,
 *                    and retrieving keybindings.
 *
 ****************************************************************************/

void
kb_init()
{
   size_t i;

   /* setup empty buffer */
   KeyBindingsSize = 0;
   KeyBindingsCapacity = 0;
   kb_increase_capacity();

   /* install default bindings */
   for (i = 0; i < DefaultKeyBindingsSize; i++)
      kb_bind(DefaultKeyBindings[i].action, DefaultKeyBindings[i].key);
}

void
kb_free()
{
   free(KeyBindings);
   KeyBindingsSize = 0;
}

void
kb_bind(KeyAction action, KeyCode key)
{
   kb_unbind_key(key);

   if (KeyBindingsSize == KeyBindingsCapacity)
      kb_increase_capacity();

   KeyBindings[KeyBindingsSize].action = action;
   KeyBindings[KeyBindingsSize].key = key;
   KeyBindingsSize++;
}

void
kb_unbind_action(KeyAction action)
{
   size_t i, j;

   for (i = 0; i < KeyBindingsSize; i++) {
      if (KeyBindings[i].action == action) {
         for (j = i; j < KeyBindingsSize - 1; j++)
            KeyBindings[j] = KeyBindings[j + 1];

         KeyBindingsSize--;
      }
   }
}

void
kb_unbind_key(KeyCode key)
{
   size_t i, j;

   for (i = 0; i < KeyBindingsSize; i++) {
      if (KeyBindings[i].key == key) {
         for (j = i; j < KeyBindingsSize - 1; j++)
            KeyBindings[j] = KeyBindings[j + 1];

         KeyBindingsSize--;
      }
   }
}

void
kb_unbind_all()
{
   KeyBindingsSize = 0;
}

bool
kb_str2action(const char *s, KeyAction *action)
{
   size_t i;

   for (i = 0; i < KeyActionNamesSize; i++) {
      if (strcasecmp(KeyActionNames[i].name, s) == 0) {
         *action = KeyActionNames[i].action;
         return true;
      }
   }

   return false;
}

KeyCode
kb_str2specialKey(char *s)
{
   size_t i;

   for (i = 0; i < SpecialKeysSize; i++) {
      if (strcasecmp(SpecialKeys[i].str, s) == 0)
         return SpecialKeys[i].code;
   }

   return -1;
}

KeyCode
kb_str2keycode(char *s)
{
   KeyCode val;

   if (strlen(s) == 1)
      val = s[0];
   else
      val = kb_str2specialKey(s);

   return val;
}

KeyCode
kb_str2keycode2(char *control, char *key)
{
   KeyCode val;

   if (strcasecmp(control, "control") != 0)
      return -1;

   if ((val = kb_str2keycode(key)) > 0)
      return kb_CONTROL(val);
   else
      return -1;
}

bool
kb_execute(KeyCode k)
{
   KeyAction action;
   size_t i;
   bool   found;

   /* visual mode and ESC pressed ?*/
   if (visual_mode_start != -1 && k == MY_K_ESCAPE) {
	   visual_mode_start = -1;
	   return true;
   }

   /* Is the key bound? */
   found  = false;
   action = -1;
   for (i = 0; i < KeyBindingsSize; i++) {
      if (KeyBindings[i].key == k) {
         action = KeyBindings[i].action;
         found = true;
      }
   }

   if (!found)
      return false;

   /* Execute theaction handler. */
   for (i = 0; i < KeyActionHandlersSize; i++) {
      if (KeyActionHandlers[i].action == action) {
         if (visual_mode_start == -1 || KeyActionHandlers[i].visual) {
            ((KeyActionHandlers[i].handler)(KeyActionHandlers[i].args));
            return true;
         }
      }
   }

   return false;
}

bool
kb_execute_by_name(const char *name)
{
   KeyAction   a;
   size_t      x;

   if (!kb_str2action(name, &a))
      return false;

   for(x = 0; x < KeyActionHandlersSize; x++) {
      if(a == KeyActionHandlers[x].action) {
         KeyActionHandlers[x].handler(KeyActionHandlers[x].args);
         return true;
      }
   }
   return false;
}


/*****************************************************************************
 *
 *                     Individual Keybinding Handlers
 *
 ****************************************************************************/

KbaArgs
get_dummy_args()
{
   KbaArgs dummy = { .direction = -1, .scale = -1, .amount = -1,
      .placement = -1, .num = -1 };

   return dummy;
}

void
kba_scroll_row(KbaArgs a)
{
   int n = gnum_retrieve();

   /* update current row */
   switch (a.direction) {
   case DOWN:
      ui.active->crow += n;
      break;
   case UP:
      ui.active->crow -= n;
      break;
   default:
      errx(1, "%s: invalid direction", __FUNCTION__);
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
kba_scroll_page(KbaArgs a)
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
kba_scroll_col(KbaArgs a)
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
kba_jumpto_screen(KbaArgs a)
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
kba_jumpto_file(KbaArgs a)
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
kba_go(KbaArgs a UNUSED)
{
   /* NOTE: While I intend to support most of vim's g* commands, currently
    *       this only supports 'gg' ... simply because I use it.
    */
   KbaArgs args = get_dummy_args();

   args.scale = NUMBER;
   args.num   = 'g';
   kba_jumpto_file(args);
}

void
kba_search(KbaArgs a)
{
   const char *errmsg = NULL;
   KbaArgs   find_args;
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
   find_args = get_dummy_args();
   find_args.direction = SAME;
   kba_search_find(find_args);
}

void
kba_search_find(KbaArgs a)
{
   KbaArgs  foo;
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
         foo = get_dummy_args();
         foo.scale = NUMBER;
         foo.num = 'G';
         kba_jumpto_file(foo);
         return;
      }
   }

   paint_error("Pattern not found: %s", mi_query_getraw());
}


void
kba_visual(KbaArgs a UNUSED)
{
   if (ui.active == ui.library) {
      paint_message("No visual mode in library window.  Sorry.");
      return;
   }

   if (visual_mode_start == -1)
      visual_mode_start = ui.active->voffset + ui.active->crow;
   else
      visual_mode_start = -1;

   paint_playlist();
}

/*
 * TODO so much of the logic in cut() is similar with that of yank() above.
 * combine the two, use an Args parameter to determine if the operation is
 * a yank or a delete
 */
void
kba_cut(KbaArgs a UNUSED)
{
   playlist *p;
   char *warning;
   bool  got_target;
   int   start, end;
   int   response;
   int   input;
   int   n;

   if (visual_mode_start != -1) {
      start = visual_mode_start;
      end = ui.active->voffset + ui.active->crow;
      visual_mode_start = -1;
      if (start > end) {
         int tmp = end;
         end = start;
         start = tmp;
      }
      end++;
   } else {
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
   paint_library();
   paint_message("%d fewer files.", end - start);
}

void
kba_yank(KbaArgs a UNUSED)
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

   if (visual_mode_start != -1) {
      start = visual_mode_start;
      end = ui.active->voffset + ui.active->crow;
      visual_mode_start = -1;
      if (start > end) {
         int tmp = end;
         end = start;
         start = tmp;
      }
      end++;
   } else {
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
   }

   /* sanitize start and end */
   if (end > ui.active->nrows)
      end = ui.active->nrows;

   /* clear existing yank buffer and add new stuff */
   ybuffer_clear();
   for (n = start; n < end; n++)
      ybuffer_add(viewing_playlist->files[n]);

   paint_playlist();
   /* notify user # of rows yanked */
   paint_message("Yanked %d files.", end - start);
}

void
kba_paste(KbaArgs a)
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
kba_undo(KbaArgs a UNUSED)
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
kba_redo(KbaArgs a UNUSED)
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

void
kba_command_mode(KbaArgs a UNUSED)
{
   char  *cmd;

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

   cmd_execute(cmd);
   free(cmd);
   return;
}

void
kba_shell(KbaArgs a UNUSED)
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
kba_quit(KbaArgs a UNUSED)
{
   VSIG_QUIT = 1;
}

void
kba_redraw(KbaArgs a UNUSED)
{
   ui_clear();
   paint_all();
}

void
kba_switch_windows(KbaArgs a UNUSED)
{
   if (ui.active == ui.library) {
      ui.active = ui.playlist;
      if (ui.lhide)
         ui_hide_library();
   } else {
      ui.active = ui.library;
      if (ui.lhide)
         ui_unhide_library();
   }

   paint_all();
   paint_status_bar();
}

void
kba_show_file_info(KbaArgs a UNUSED)
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
kba_load_playlist(KbaArgs a UNUSED)
{
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
      kba_switch_windows(get_dummy_args());
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
kba_play(KbaArgs a UNUSED)
{
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
      kba_switch_windows(get_dummy_args());
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
kba_pause(KbaArgs a UNUSED)
{
   player_pause();
}

void
kba_stop(KbaArgs a UNUSED)
{
   player_stop();
   playing_playlist = NULL;
}

void
kba_play_next(KbaArgs a UNUSED)
{
   int n = 1;

   /* is there a multiplier? */
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   player_skip_song(n);
}

void
kba_play_prev(KbaArgs a UNUSED)
{
   int n = 1;

   /* is there a multiplier? */
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   player_skip_song(n * -1);
}

void
kba_volume(KbaArgs a)
{
   float pcnt;

   if (gnum_get() > 0)
      pcnt = gnum_retrieve();
   else
      pcnt = 1;

   switch (a.direction) {
   case FORWARDS:
      break;
   case BACKWARDS:
      pcnt *= -1;
      break;
   default:
      errx(1, "kba_volume: invalid direction");
   }

   player_volume_step(pcnt);
}

void
kba_seek(KbaArgs a)
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
kba_toggle(KbaArgs a)
{
   toggle_list *t;
   char  *cmd;
   bool   got_register;
   int    n, input, registr;

   /* is there a multiplier? */
   n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }

   /* get the register */
   got_register = false;
   registr = -1;
   while ((input = getch()) && !got_register) {
      if (input == ERR)
         continue;

      if (('a' <= input && input <= 'z')
      ||  ('A' <= input && input <= 'Z')) {
         got_register = true;
         registr = input;
      }
   }

   /* get the command to execute */
   if ((t = toggle_get(registr)) == NULL) {
      paint_error("No toggle list in register %c (%i).", registr, registr);
      return;
   }

   /* update index */
   n %= t->size;
   switch (a.direction) {
      case FORWARDS:
         t->index += n;
         t->index %= t->size;
         break;
      case BACKWARDS:
         if (n <= (int)t->index) {
            t->index -= n;
            t->index %= t->size;
         } else {
            t->index = t->size - (n - t->index);
         }
         break;
      default:
         errx(1, "%s: invalid direction", __FUNCTION__);
   }

   /* execute */
   cmd = t->commands[t->index];
   cmd_execute(cmd);
}


/*****************************************************************************
 *
 *                       Routines for Working with 'gnum'
 *
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

int
gnum_retrieve()
{
   int n = 1;
   if (gnum_get() > 0) {
      n = gnum_get();
      gnum_clear();
   }
   return n;
}


/*****************************************************************************
 *
 *                Routines for Working with Search Direction
 *
 ****************************************************************************/

Direction _global_search_dir = FORWARDS;

Direction search_dir_get()
{ return _global_search_dir; }

void search_dir_set(Direction dir)
{ _global_search_dir = dir; }


/*****************************************************************************
 *
 *               Routines for Working with Copy/Cut/Paste Buffer
 *
 ****************************************************************************/

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
 *
 *                           Misc. Handy Routines
 *
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
   int   input;

   def_prog_mode();
   endwin();

   system(cmd);
   printf("\nPress ENTER or type command to continue");
   fflush(stdout);
   raw();
   while(!VSIG_QUIT) {
      if ((input = getch()) && input != ERR) {
         if (input != '\r')
            ungetch(input);
         break;
      }
   }
   reset_prog_mode();
   paint_all();
}

