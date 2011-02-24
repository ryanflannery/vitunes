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

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "debug.h"
#include "enums.h"
#include "paint.h"
#include "vitunes.h"

#include "compat.h"

/*
 * List of all actions that can be bound by keybindings.
 * NOTE: Using "count" trick (see last element), so no enum should be defined.
 */
typedef enum {
   /* scrolling vertically */
   scroll_up,
   scroll_down,
   scroll_up_page,
   scroll_down_page,
   scroll_up_halfpage,
   scroll_down_halfpage,
   scroll_up_wholepage,
   scroll_down_wholepage,
   /* scrolling horizontally */
   scroll_left,
   scroll_right,
   scroll_leftmost,
   scroll_rightmost,
   /* jumping within screen */
   jumpto_screen_top,
   jumpto_screen_middle,
   jumpto_screen_bottom,
   /* jumping within whole playlist/library */
   jumpto_line,
   jumpto_percent,
   /* searching */
   search_forward,
   search_backward,
   find_next_forward,
   find_next_backward,
   /* copy/cut/paste & undo/redo */
   visual,
   cut,
   yank,
   paste_after,
   paste_before,
   undo,
   redo,
   /* misc. */
   go,
   quit,
   redraw,
   command_mode,
   shell,
   /* vitunes specific stuff */
   switch_windows,
   show_file_info,
   load_playlist,
   media_play,
   media_pause,
   media_stop,
   media_next,
   media_prev,
   volume_increase,
   volume_decrease,
   seek_forward_seconds,
   seek_backward_seconds,
   seek_forward_minutes,
   seek_backward_minutes,
   toggle_forward,
   toggle_backward
} KeyAction;

typedef int KeyCode;


/* Keybinding initializing and binding routines */
void kb_init();
void kb_free();
void kb_bind(KeyAction, KeyCode);
void kb_unbind_action(KeyAction);
void kb_unbind_key(KeyCode);
void kb_unbind_all();
bool kb_execute(KeyCode);
bool kb_execute_by_name(const char *);

bool    kb_str2action(const char*, KeyAction*);
KeyCode kb_str2keycode(char*);
KeyCode kb_str2keycode2(char*, char*);


/* 
 * This is the generic argument structure for all keybinding action handlers.
 * This is used liberally to prevent any intricate parsing and make heavy use
 * of code-reuse.
 */
typedef struct {
   Direction   direction;
   Scale       scale;
   Amount      amount;
   Placement   placement;
   int         num;
} KbaArgs;
typedef void(*ActionHandler)(KbaArgs a);

/* Individual keybinding action handlers */
void kba_scroll_row(KbaArgs a);
void kba_scroll_page(KbaArgs a);
void kba_scroll_col(KbaArgs a);

void kba_jumpto_screen(KbaArgs a);
void kba_jumpto_file(KbaArgs a);

void kba_search(KbaArgs a);
void kba_search_find(KbaArgs a);

void kba_visual(KbaArgs a);
void kba_cut(KbaArgs a);
void kba_yank(KbaArgs a);
void kba_paste(KbaArgs a);
void kba_undo(KbaArgs a);
void kba_redo(KbaArgs a);

void kba_command_mode(KbaArgs a);
void kba_go(KbaArgs a);
void kba_shell(KbaArgs a);
void kba_quit(KbaArgs a);
void kba_redraw(KbaArgs a); 

void kba_switch_windows(KbaArgs a);
void kba_show_file_info(KbaArgs a);
void kba_load_playlist(KbaArgs a);
void kba_play(KbaArgs a);
void kba_pause(KbaArgs a);
void kba_stop(KbaArgs a);
void kba_play_next(KbaArgs a);
void kba_play_prev(KbaArgs a);
void kba_volume(KbaArgs a);
void kba_seek(KbaArgs a);
void kba_toggle(KbaArgs a);


/*
 * The 'gnum' is the number n that can be applied to many keybindings.  E.g.
 * how the sequence "15j" will move down 15 lines.  "15" is the gnum here.
 * These are the routines used to init/set/clear the gnum.
 */
void gnum_clear();
void gnum_set(int x);
void gnum_add(int x);
int  gnum_get();        /* Return current gnum. */
int  gnum_retrieve();   /* Return current gnum and then clear it. */


/* These are used to set/use the search direction */
Direction search_dir_get();
void  search_dir_set(Direction d);


/* This is the copy/cut buffer and the routines used to manipulate it. */
#define YANK_BUFFER_CHUNK_SIZE 100
typedef struct {
   meta_info   **files;
   int           nfiles;
   int           capacity;
} yank_buffer;
extern yank_buffer _yank_buffer;

void ybuffer_init();
void ybuffer_clear();
void ybuffer_free();
void ybuffer_add(meta_info *f);


/* Misc. handy functions used frequently */
void redraw_active();
bool match_command_name(const char *s, const char *cmd);
void execute_external_command(const char *cmd);

#endif
