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

#ifndef COMMANDS_H
#define COMMANDS_H

#include "enums.h"
#include "paint.h"
#include "str2argv.h"
#include "vitunes.h"
#include "debug.h"
#include "keybindings.h"

#include "compat.h"

/****************************************************************************
 * Toggle-list handling stuff
 ***************************************************************************/

/* command-mode command struct */
typedef struct {
   char  *name;
   int   (*func)(int argc, char **argv);
} cmd;

extern const cmd CommandPath[];
extern const int CommandPathSize;

/* command-mode command handlers */
int cmd_quit(int argc, char *argv[]);
int cmd_write(int argc, char *argv[]);
int cmd_mode(int argc, char *argv[]);
int cmd_new(int argc, char *argv[]);
int cmd_filter(int argc, char *argv[]);
int cmd_sort(int argc, char *argv[]);
int cmd_display(int argc, char *argv[]);
int cmd_color(int argc, char *argv[]);
int cmd_set(int argc, char *argv[]);
int cmd_reload(int argc, char *argv[]);
int cmd_bind(int argc, char *argv[]);
int cmd_unbind(int argc, char *argv[]);
int cmd_toggle(int argc, char *argv[]);
int cmd_playlist(int argc, char *argv[]);

/* parse a string and execute it as a command */
void cmd_execute(char *cmd);


/****************************************************************************
 * Toggle-list handling stuff
 ***************************************************************************/

/* struct for a single toggle list */
typedef struct {
   int      registr;
   char   **commands;
   size_t   size;
   size_t   index;
} toggle_list;

/* the array (and size) of all toggle lists currently loaded */
extern toggle_list **toggleset;
extern size_t        toggleset_size;

/* initialize and free the toggleset */
void toggleset_init();
void toggleset_free();

toggle_list *toggle_list_create(int registr, int argc, char *argv[]);
void toggle_list_add_command(toggle_list *t, char *cmd);
void toggle_list_free(toggle_list *t);

void toggle_add(toggle_list *t);
void toggle_remove(int registr);
toggle_list* toggle_get(int registr);


/****************************************************************************
 * Misc.
 ***************************************************************************/

int user_getstr(const char *prompt, char **response);
int user_get_yesno(const char *prompt, int *response);

void setup_viewing_playlist(playlist *p);


#endif
