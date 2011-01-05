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

#include "player.h"

/* global structs defined in player.h */
player_status_t player_status;
player_t player;


/*
 * Initialize the player and player_status global structs.  Everything is set
 * to non-functional values.  These are used elsewhere to indicate that the
 * child process is not running.
 */
void
player_init(char *prog, char *pargs[], playmode mode)
{
   /* player status init */
   player_status.playing = false;
   player_status.paused = false;
   player_status.position = -1;

   /* player init */
   player.program = prog;
   player.pargs = pargs;
   player.pid = -1;
   player.pipe_read  = -1;
   player.pipe_write = -1;

   player.mode = mode;
   player.queue = NULL;
   player.qidx = -1;

   player.rseed = time(0);
   srand(player.rseed);
}

/*****************************************************************************
 * stuff for starting, stoping, and re-starting child process
 ****************************************************************************/

void
player_child_launch()
{
   int pwrite[2];
   int pread[2];
   int flags;

   if (pipe(pwrite) == -1 || pipe(pread) == -1)
      err(1, "player_child_launch: pipe() failed");

   switch (player.pid = fork()) {
   case -1:
      err(1, "player_child_launch: fork() failed");
      break;

   case 0:  /* child process */
      if (close(0) == -1 || close(1) == -1 || close(2) == -1)
         err(1, "player_child_launch: child close()'s failed(1)");

      if (close(pwrite[1]) == -1 || close(pread[0]) == -1)
         err(1, "player_child_launch: child close()'s failed(2)");

      if (dup(pwrite[0]) == -1 || dup(pread[1]) == -1)
         err(1, "player_child_launch: child dup()'s failed");

      if (execv(player.program, player.pargs) == -1)
         kill(getppid(), SIGCHLD);  /* send signal NOW! */

      /*
       * If reached here, mplayer failed.  Exit now.
       * This is caught in vitunes.c signal handling.
       */
      exit(1);
      break;
   }

   /* Back to the parent... */

   /* close unnecessary pipe ends */
   if (close(pwrite[0]) == -1 || close(pread[1]) == -1)
      err(1, "player_child_launch: parent close()'s failed");

   /* setup player pipes */
   player.pipe_read  = pread[0];
   player.pipe_write = pwrite[1];

   /* setup read pipe to media player as non-blocking */
   if ((flags = fcntl(player.pipe_read, F_GETFL, 0)) == -1)
      err(1, "player_child_launch: fcntl() failed to get current flags");

   if (fcntl(player.pipe_read, F_SETFL, flags | O_NONBLOCK) == -1)
      err(1, "player_child_launch: fcntl() failed to set pipe non-blocking");
}

void
player_child_relaunch()
{
   int previous_position;
   int status;

   wait(&status);
   player_child_launch();

   /* restart playback where left-off */
   if (player_status.playing && !player_status.paused) {
      previous_position = player_status.position;
      player_play();
      player_seek(previous_position);
   }
}

void
player_child_kill()
{
   static const char *cmd = "\nquit\n";

   player_send_cmd(cmd);

   close(player.pipe_read);
   close(player.pipe_write);
   waitpid(player.pid, NULL, 0);

   player_init(player.program, player.pargs, player.mode);
}


/*****************************************************************************
 * player control functions (setting queue, playing, pausing, etc.)
 ****************************************************************************/

void
player_set_queue(playlist *queue, int pos)
{
   player.queue = queue;
   player.qidx  = pos;
}

void
player_send_cmd(const char *cmd)
{
   write(player.pipe_write, cmd, strlen(cmd));
}

void
player_play()
{
   static const char *cmd_fmt = "\nloadfile \"%s\" 0\nget_property time_pos\n";
   char *cmd;

   /* assert valid queue/qidx */
   if (player.queue == NULL)
      errx(1, "player_play: bad queue/qidx");

   if (player.qidx < 0 || player.qidx > player.queue->nfiles)
      errx(1, "player_play: qidx %i out-of-range", player.qidx);

   /* build and send the command to the player */
   asprintf(&cmd, cmd_fmt, player.queue->files[player.qidx]->filename);
   if (cmd == NULL)
      err(1, "player_play: asprintf failed");

   player_send_cmd(cmd);
   free(cmd);

   /* update player status info */
   player_status.playing = true;
   player_status.paused = false;
   player_status.position = 0;
}

/*
 * Play the next song in the queue.  This will also stop playback if the mode
 * is linear and the end of the playlist is reached.
 */
void
player_play_next_song()
{
   switch (player.mode) {
   case PLAYER_MODE_LINEAR:
      if (++player.qidx == player.queue->nfiles) {
         player.qidx = 0;
         player_stop();
      } else
         player_play();

      break;

   case PLAYER_MODE_LOOP:
      if (++player.qidx == player.queue->nfiles)
         player.qidx = 0;

      player_play();
      break;

   case PLAYER_MODE_RANDOM:
      player.qidx = rand() % player.queue->nfiles;
      player_play();
      break;
   }
}

/* cease playback */
void
player_stop()
{
   static const char *cmd = "\nstop\n";

   player_send_cmd(cmd);

   player_status.playing = false;
   player_status.paused = false;
}

/* pause/unpause playback */
void
player_pause()
{
   static const char *cmd = "\npause\n";

   if (!player_status.playing)
      return;

   player_send_cmd(cmd);
   player_status.paused = !player_status.paused;
}

/* seek forward/backward in the currently playing file */
void
player_seek(int seconds)
{
   static const char *cmd_fmt = "\nseek %i 0\nget_property time_pos\n";
   char *cmd;

   if (!player_status.playing)
      return;

   asprintf(&cmd, cmd_fmt, seconds);
   if (cmd == NULL)
      err(1, "player_seek: asprintf failed");

   player_send_cmd(cmd);
   free(cmd);

   if (player_status.paused)
      player_status.paused = false;
}

/*****************************************************************************
 * Player monitor function, called repeatedly via the signal handler in the
 * vitunes main loop.
 *
 * This communicates with the child process periodically to accomplish the
 * following:
 *    1. If the player is currently playing a song, determine the position
 *       (in seconds) into the playback
 *    2. When the player finishes playing a song, it starts playing the next
 *       song, according to the current playmode.
 ****************************************************************************/
void
player_monitor()
{
   static const char *query_cmd   = "\nget_property time_pos\n";
   static const char *answer_fail = "ANS_ERROR=PROPERTY_UNAVAILABLE";
   static const char *answer_good = "ANS_time_pos";
   static char response[1000];  /* mplayer can be noisy */
   char *s;
   int   nbytes;

   /* in this case, nothing to monitor */
   if (!player_status.playing || player_status.paused)
      return;

   /* read any output from the player */
   bzero(response, sizeof(response));
   nbytes = read(player.pipe_read, &response, sizeof(response));

   if (nbytes == -1 && errno == EAGAIN)
      return;

   response[nbytes + 1] = '\0';

   /* case: reached end of playback for a given file */
   if (strstr(response, answer_fail) != NULL) {
      player_play_next_song();
      return;
   }

   /* case: continue in playing current file.  update position */
   if ((s = strstr(response, answer_good)) != NULL) {
      while (strstr(s + 1, answer_good) != NULL)
         s = strstr(s + 1, answer_good);

      if (sscanf(s, "ANS_time_pos=%f", &player_status.position) != 1)
         errx(1, "player_monitor: player child is misbehaving.");

   }

   player_send_cmd(query_cmd);
}
