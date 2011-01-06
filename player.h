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

#include <sys/wait.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "meta_info.h"
#include "playlist.h"

#include "compat.h"


/*
 * Available play-modes.
 *    Linear:  Songs in the queue play in the order they appear
 *    Loop:    Like linear, but when the end is reached, the queue restarts
 *    Random:  Songs are chosen at random and play never ends
 */
typedef enum {
   PLAYER_MODE_LINEAR,
   PLAYER_MODE_LOOP,
   PLAYER_MODE_RANDOM
} playmode;


/*
 * The following global structure used to track status of the child process
 * used to play the media.  It is updated by the player_monitor function and
 * used elsewhere (paint_player) to determine if the child is playing, and
 * if so, where it's at in the current song.
 */
typedef struct {
   bool  playing;    /* playing or not (still true if paused) */
   bool  paused;     /* if paused or not */
   float position;   /* position, in seconds, into currently playing file */
} player_status_t;
extern player_status_t player_status;


/*
 * This global structure encapsulates info about the child process used to
 * play the media.  It includes the current play queue and location within
 * the queue.
 */
typedef struct {
   /* child process and pipe info */
   char    *program;    /* program to execute */
   char   **pargs;      /* arguments to program */
   pid_t    pid;        /* process id */
   int      pipe_read;  /* read-end of pipe */
   int      pipe_write; /* write-end of pipe */

   /* currently playing queue information */
   playmode  mode;      /* playback mode */
   playlist *queue;     /* pointer to playlist */
   int       qidx;      /* index of currently playing file in the queue */

   /* seed used by rand(3) */
   int       rseed;

} player_t;
extern player_t player;


/* initialize all player & player_status info. */
void player_init(char *prog, char *pargs[], playmode mode);

/* for starting, stoping, and restarting the child process player */
void player_child_launch();
void player_child_relaunch();
void player_child_kill();

/* setup player queue */
void player_set_queue(playlist *queue, int position);

void player_send_cmd(const char *cmd);

/* player control functions */
void player_play();
void player_play_next_song();
void player_stop();
void player_pause();
void player_seek(int seconds);

/*
 * This should be called periodically to monitor the child media player
 * process and update the global player_status object
 */
void player_monitor();

#endif
