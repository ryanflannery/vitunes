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

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "enums.h"
#include "paint.h"
#include "str2argv.h"
#include "vitunes.h"
#include "debug.h"


/* for working with number read from user for keybindings */
void gnum_clear();
int  gnum_get();
void gnum_add(int x);


/* for working with active search direction */
venum search_dir_get();
void  search_dir_set(venum dir);


/* for working with yank/cut buffer */
#define YANK_BUFFER_CHUNK_SIZE 100
typedef struct {
   meta_info   **files;
   int           nfiles;
   int           capacity;
} yank_buffer;

void ybuffer_init();
void ybuffer_clear();
void ybuffer_free();
void ybuffer_add(meta_info *f);


/* misc. handy functions used frequently */
void redraw_active();
bool match_command_name(const char *s, const char *cmd);
void execute_external_command(const char *cmd);


/*
 * The arguments passed to each keybinding.  No parsing required.
 */
typedef struct {
   venum    amount;
   venum    direction;
   venum    placement;
   venum    scale;
   int      num;
} Args;

typedef struct {
   int   keycode;
   void  (*func)(Args);
   Args  args;
} keybinding;

extern const keybinding KeyBindings[];
extern const int KeyBindingsSize;


/* keybinding handlers */
void quit_vitunes(Args a);
void load_or_play(Args a);
void show_file_info(Args a);
void pause_playback(Args a);
void stop_playback(Args a);
void seek_playback(Args a);
void switch_focus(Args a);
void redraw(Args a); 
void enter_cmd_mode(Args a);
void external_command(Args a);
void scroll_row(Args a);
void scroll_col(Args a);
void scroll_page(Args a);
void jumpto_file(Args a);
void jumpto_page(Args a);
void yank(Args a);
void paste(Args a);
void cut(Args a);
void undo(Args a);
void redo(Args a);
void search(Args a);
void search_find(Args a);

#endif
