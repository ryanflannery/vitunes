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

/* gloabls */
player_backend_t player;
player_info_t player_info;


/* callbacks */
static void callback_playnext() { player_skip_song(1); }

static void
callback_fatal(char *fmt, ...)
{
   va_list ap;

   ui_destroy();

   fprintf(stderr,"The player-backend '%s' has experienced a fatal error:\n",
         player.name);

   va_start(ap, fmt);
   vfprintf(stderr, fmt, ap);
   va_end(ap);

   fflush(stderr);

   VSIG_QUIT = 1;
   exit(1);
}


/* definition of backends */
const player_backend_t PlayerBackends[] = { 
   {   
      BACKEND_MPLAYER, "mplayer", false, NULL,
      mplayer_start,
      mplayer_finish,
      mplayer_sigchld,
      mplayer_play,
      mplayer_stop,
      mplayer_pause,
      mplayer_seek,
      mplayer_volume_step,
      mplayer_get_position,
      mplayer_get_volume,
      mplayer_is_playing,
      mplayer_is_paused,
      mplayer_set_callback_playnext,
      mplayer_set_callback_notice,
      mplayer_set_callback_error,
      mplayer_set_callback_fatal,
      mplayer_monitor
   },  
   { 0, "", false, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
const size_t PlayerBackendsSize = sizeof(PlayerBackends) / sizeof(player_backend_t);


/* setup/destroy functions */
void
player_init(const char *backend)
{
   bool   found;
   size_t i;

   player_info.mode  = MODE_LINEAR;
   player_info.queue = NULL;
   player_info.qidx  = -1;

   player_info.rseed = time(0);
   srand(player_info.rseed);

   /* find the player backend */
   found = false;
   for (i = 0; i < PlayerBackendsSize && !found; i++) {
      if (!strcasecmp(backend, PlayerBackends[i].name)) {
         player = PlayerBackends[i];
         found = true;
      }
   }

   if (!found) {
      ui_destroy();
      errx(1, "backend '%s' not supported", backend);
   }

   if (player.dynamic) {
      ui_destroy();
      errx(1, "dynamically loaded backends not yet supported");
   }

   player.set_callback_playnext(callback_playnext);
   player.set_callback_notice(paint_message);
   player.set_callback_error(paint_error);
   player.set_callback_fatal(callback_fatal);
   player.start();
}

void
player_destroy()
{
   player.finish();
}

void
player_set_queue(playlist *queue, int pos)
{
   player_info.queue = queue;
   player_info.qidx  = pos;
}

void
player_play()
{
   if (player_info.queue == NULL)
      errx(1, "player_play: bad queue/qidx");

   if (player_info.qidx < 0 || player_info.qidx > player_info.queue->nfiles)
      errx(1, "player_play: qidx %i out-of-range", player_info.qidx);

   player.play(player_info.queue->files[player_info.qidx]->filename);

}

void
player_stop()
{
   player.stop();
}

void
player_pause()
{
   if (!player.playing())
      return;

   player.pause();
}

void
player_seek(int seconds)
{
   if (!player.playing())
      return;

   player.seek(seconds);
}

/* TODO merge this with the player_play_prev_song into player_skip_song */
void
player_play_next_song(int skip)
{
   if (!player.playing())
      return;

   switch (player_info.mode) {
   case MODE_LINEAR:
      player_info.qidx += skip;
      if (player_info.qidx >= player_info.queue->nfiles) {
         player_info.qidx = 0;
         player_stop();
      } else
         player_play();

      break;

   case MODE_LOOP:
      player_info.qidx += skip;
      if (player_info.qidx >= player_info.queue->nfiles)
         player_info.qidx %= player_info.queue->nfiles;

      player_play();
      break;

   case MODE_RANDOM:
      player_info.qidx = rand() % player_info.queue->nfiles;
      player_play();
      break;
   }
}

/* TODO (see comment for player_play_next_song) */
void
player_play_prev_song(int skip)
{
   if (!player.playing())
      return;

   switch (player_info.mode) {
   case MODE_LINEAR:
      player_info.qidx -= skip;
      if (player_info.qidx < 0) {
         player_info.qidx = 0;
         player_stop();
      } else
         player_play();

      break;

   case MODE_LOOP:
      skip %= player_info.queue->nfiles;
      if (skip <= player_info.qidx)
         player_info.qidx -= skip;
      else
         player_info.qidx = player_info.queue->nfiles - (skip + player_info.qidx);

      player_play();
      break;

   case MODE_RANDOM:
      player_info.qidx = rand() % player_info.queue->nfiles;
      player_play();
      break;
   }
}

/* TODO merge with above... wtf didn't i do it that way!? */
void
player_skip_song(int num)
{
   if (num >= 0)
      player_play_next_song(num);
   else
      player_play_prev_song(num * -1);
}

void
player_volume_step(float percent)
{
   if (!player.playing())
      return;

   player.volume_step(percent);
}

void
player_monitor(void)
{
   player.monitor();
}

