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

#ifndef CONFIG_H
#define CONFIG_H

/* hard-coded defaults for various things.  self explanatory. */
const int DEFAULT_LIBRARY_WINDOW_WIDTH = 18;

playmode DEFAULT_PLAYER_MODE = PLAYER_MODE_LOOP;
char *DEFAULT_PLAYER_PATH = "/usr/local/bin/mplayer";
char *DEFAULT_PLAYER_ARGS[] = { "mplayer", "-slave", "-idle", "-quiet", "-msglevel", "cplayer=0:ao=0:vo=0:decaudio=0:decvideo=0:demuxer=0", NULL };

/*
 * XXX IMPORTANT NOTE:
 * If you're having problems with the keybindings on your platform, most
 * likely it's because the key-mappings below between "CONTROL+x" are
 * different on your system.  I'm trying to find a better way of mapping
 * these, but ncurses(3) does not appear to be able to handle these (via
 * keypad(3) or any other mechanism) in some reasonable fashion.
 * Thoughts?  Investigate further... TODO
 */

#define K_TAB      9
#define K_ENTER    13
#define K_CNTRL_B  2
#define K_CNTRL_C  3
#define K_CNTRL_D  4
#define K_CNTRL_E  5
#define K_CNTRL_F  6
#define K_CNTRL_L  12
#define K_CNTRL_P  16
#define K_CNTRL_U  21
#define K_CNTRL_Y  25
#define K_CNTRL_SLASH 28


/*
 * List of keybindings.  See input_handlers.h for relevant structs.
 * Each function listed here is in input_handlers.*
 */
const keybinding KeyBindings[] = {
   { K_CNTRL_C,      quit_vitunes,     { 0 } },
   { K_CNTRL_SLASH,  quit_vitunes,     { 0 } },

   { K_ENTER,        load_or_play,     { 0 } },
   { 'm',            show_file_info,   { 0 } },
   { K_CNTRL_P,      pause_playback,   { 0 } },
   { 'z',            pause_playback,   { 0 } },
   { 's',            stop_playback,    { 0 } },
   { '[',            seek_playback,    { .direction = BACKWARDS, .scale = SECONDS, .num = 10 }},
   { ']',            seek_playback,    { .direction = FORWARDS,  .scale = SECONDS, .num = 10 }},
   { 'b',            seek_playback,    { .direction = BACKWARDS, .scale = SECONDS, .num = 10 }},
   { 'f',            seek_playback,    { .direction = FORWARDS,  .scale = SECONDS, .num = 10 }},
   { '{',            seek_playback,    { .direction = BACKWARDS, .scale = MINUTES, .num = 1 }},
   { '}',            seek_playback,    { .direction = FORWARDS,  .scale = MINUTES, .num = 1 }},
   { 'B',            seek_playback,    { .direction = BACKWARDS, .scale = MINUTES, .num = 1 }},
   { 'F',            seek_playback,    { .direction = FORWARDS,  .scale = MINUTES, .num = 1 }},

   { K_TAB,          switch_focus,     { 0 } },
   { K_CNTRL_L,      redraw,           { 0 } },

   { 'j',            scroll_row,       { .direction = DOWN }},
   { KEY_DOWN,       scroll_row,       { .direction = DOWN }},
   { 'k',            scroll_row,       { .direction = UP }},
   { KEY_UP,         scroll_row,       { .direction = UP }},
   { '-',            scroll_row,       { .direction = UP }},

   { 'h',            scroll_col,       { .direction = LEFT,  .amount = SINGLE }},
   { KEY_LEFT,       scroll_col,       { .direction = LEFT,  .amount = SINGLE }},
   { KEY_BACKSPACE,  scroll_col,       { .direction = LEFT,  .amount = SINGLE }},
   { 'l',            scroll_col,       { .direction = RIGHT, .amount = SINGLE }},
   { KEY_RIGHT,      scroll_col,       { .direction = RIGHT, .amount = SINGLE }},
   { ' ',            scroll_col,       { .direction = RIGHT, .amount = SINGLE }},
   { '$',            scroll_col,       { .direction = RIGHT, .amount = WHOLE }},
   { '^',            scroll_col,       { .direction = LEFT,  .amount = WHOLE }},
   { '0',            scroll_col,       { .direction = LEFT,  .amount = WHOLE }},
   { '|',            scroll_col,       { .direction = LEFT,  .amount = WHOLE }},

   { K_CNTRL_E,      scroll_page,      { .direction = DOWN, .amount = SINGLE }},
   { K_CNTRL_Y,      scroll_page,      { .direction = UP,   .amount = SINGLE }},
   { K_CNTRL_U,      scroll_page,      { .direction = UP,   .amount = HALF }},
   { K_CNTRL_D,      scroll_page,      { .direction = DOWN, .amount = HALF }},
   { K_CNTRL_B,      scroll_page,      { .direction = UP,   .amount = WHOLE }},
   { KEY_PPAGE,      scroll_page,      { .direction = UP,   .amount = WHOLE }},
   { K_CNTRL_F,      scroll_page,      { .direction = DOWN, .amount = WHOLE }},
   { KEY_NPAGE,      scroll_page,      { .direction = DOWN, .amount = WHOLE }},

   { 'G',            jumpto_file,      { .scale = NUMBER }},
   { '%',            jumpto_file,      { .scale = PERCENT }},

   { 'H',            jumpto_page,      { .placement = TOP }},
   { 'M',            jumpto_page,      { .placement = MIDDLE }},
   { 'L',            jumpto_page,      { .placement = BOTTOM }},

   { 'y',            yank,             { 0 } },
   { 'd',            cut,              { 0 } },
   { 'p',            paste,            { .placement = AFTER }},
   { 'P',            paste,            { .placement = BEFORE }},

   { '/',            search,           { .direction = FORWARDS }},
   { '?',            search,           { .direction = BACKWARDS }},
   { 'n',            search_find,      { .direction = SAME }},
   { 'N',            search_find,      { .direction = OPPOSITE }},

   { ':',            enter_cmd_mode,   { 0 }},
   { '!',            external_command, { 0 }}
};
const int KeyBindingsSize = (sizeof(KeyBindings) / sizeof(keybinding));

#define ExecuteKeyBinding(i)  \
   ((KeyBindings[(i)].func)(KeyBindings[(i)].args))


/*
 * List of command-mode commands.  See input_handlers.h for cmd struct.
 * All of these functions are defined in input_handlers.*
 * XXX Note that commands that accept a '!' after their names are handled
 * in 'match_cmd_name' in input_handlers.c.  Also see that for handling of
 * abbreviations.
 * XXX Note that the '!' used to execute an external command is handled in
 * the enter_cmd_mode() function.
 */
const cmd CommandPath[] = {
   {  "color",    cmd_color },
   {  "display",  cmd_display },
   {  "filter",   cmd_filter },
   {  "mode",     cmd_mode },
   {  "new",      cmd_new },
   {  "q",        cmd_quit },
   {  "reload",   cmd_reload },
   {  "set",      cmd_set },
   {  "sort",     cmd_sort },
   {  "w",        cmd_write }
};
const int CommandPathSize = (sizeof(CommandPath) / sizeof(cmd));

#endif
