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

#ifndef PLAYER_H
#define PLAYER_H

#include <err.h>

#include "playlist.h"
#include "compat.h"
#include "paint.h"
#include "debug.h"

/* "static" backends (those that aren't dynamically loaded) */
#include "players/mplayer.h"

/*
 * Available play-modes.
 *    Linear:  Songs in the queue play in the order they appear
 *    Loop:    Like linear, but when the end is reached, the queue restarts
 *    Random:  Songs are chosen at random and play never ends
 */
typedef enum {
   MODE_LINEAR,
   MODE_LOOP,
   MODE_RANDOM
} playmode;


/* player setup/destroy functions */
void player_init(const char *backend);
void player_destroy();

void player_set_queue(playlist *queue, int position);

/* player control functions */
void player_play();
void player_stop();
void player_pause();
void player_seek(int seconds);
void player_skip_song(int num);
void player_volume_step(float percent);

/* This is called periodically to monitor the backend player */
void player_monitor();


/* Available back-end players */
typedef enum {
   BACKEND_MPLAYER,
   BACKEND_GSTREAMER
} backend_id;


/* player backends */
typedef struct {
   backend_id  type;
   char       *name;

   /* for dynamically loaded backends */
   bool  dynamic;    /* true if dlopen(3) required */
   char *lib_name;   /* name of dynamic lib */

   /* setup/destroy functions */
   void (*start)(void);
   void (*finish)(void);
   void (*sigchld)(void);

   /* playback control */
   void (*play)(const char*);
   void (*stop)(void);
   void (*pause)(void);
   void (*seek)(int);
   void (*volume_step)(float);

   /* query functions */
   float (*position)(void);
   float (*volume)(void);
   bool  (*playing)(void);
   bool  (*paused)(void);

   /* callback functions */
   void (*set_callback_playnext)(void (*f)(void));
   void (*set_callback_notice)(void (*f)(char *, ...));
   void (*set_callback_error)(void (*f)(char *, ...));
   void (*set_callback_fatal)(void (*f)(char *, ...));

   /* monitor function */
   void (*monitor)(void);
} player_backend_t;
extern player_backend_t player;


/* vitunes-specific record keeping about the player */
typedef struct {
   playmode  mode;   /* playback mode */
   playlist *queue;  /* pointer to playlist */
   int       qidx;   /* index into currently playing playlist */

   int       rseed;  /* seed used by rand(3) */
} player_info_t;
extern player_info_t player_info;

#endif
