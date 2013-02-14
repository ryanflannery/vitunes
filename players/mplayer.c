/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
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

#include "mplayer.h"
#include "mplayer_conf.h"

/* callback functions */
void (*mplayer_callback_playnext)(void) = NULL;
void (*mplayer_callback_notice)(const char *, ...) = NULL;
void (*mplayer_callback_error)(const char *, ...) = NULL;
void (*mplayer_callback_fatal)(const char *, ...) = NULL;


/* record keeping */
static struct {
   /* exported to player interface */
   float       position;
   float       volume;
   bool        playing;
   bool        paused;

   /* specific to this backend */
   pid_t    pid;
   int      pipe_read;
   int      pipe_write;
   const char *current_song;
} mplayer_state;

bool restarting = false;


void mplayer_volume_set(float);
void mplayer_volume_query();

static void
mplayer_send_cmd(const char *cmd)
{
   write(mplayer_state.pipe_write, cmd, strlen(cmd));
}

void
mplayer_start()
{
   int pwrite[2];
   int pread[2];
   int flags;

   if (!exe_in_path(MPLAYER_PATH))
      errx(1, "it appears '%s' does not exist in your $PATH", MPLAYER_PATH);

   if (pipe(pwrite) == -1 || pipe(pread) == -1)
      err(1, "%s: pipe() failed", __FUNCTION__);

   switch (mplayer_state.pid = fork()) {
   case -1:
      err(1, "%s: fork() failed", __FUNCTION__);
      break;

   case 0:  /* child process */
      if (close(0) == -1 || close(1) == -1 || close(2) == -1)
         err(1, "%s: child close()'s failed(1)", __FUNCTION__);

      if (close(pwrite[1]) == -1 || close(pread[0]) == -1)
         err(1, "%s: child close()'s failed(2)", __FUNCTION__);

      if (dup(pwrite[0]) == -1 || dup(pread[1]) == -1)
         err(1, "%s: child dup()'s failed", __FUNCTION__);

      if (execvp(MPLAYER_PATH, MPLAYER_ARGS) == -1)
         kill(getppid(), SIGCHLD);  /* send signal NOW! */

      exit(1);
      break;
   }

   /* Back to the parent... */

   /* close unnecessary pipe ends */
   if (close(pwrite[0]) == -1 || close(pread[1]) == -1)
      err(1, "%s: parent close()'s failed", __FUNCTION__);

   /* setup player pipes */
   mplayer_state.pipe_read  = pread[0];
   mplayer_state.pipe_write = pwrite[1];

   /* setup read pipe to media player as non-blocking */
   if ((flags = fcntl(mplayer_state.pipe_read, F_GETFL, 0)) == -1)
      err(1, "%s: fcntl() failed to get current flags", __FUNCTION__);

   if (fcntl(mplayer_state.pipe_read, F_SETFL, flags | O_NONBLOCK) == -1)
      err(1, "%s: fcntl() failed to set pipe non-blocking", __FUNCTION__);

   if (!restarting) {
      mplayer_state.playing  = false;
      mplayer_state.paused   = false;
      mplayer_state.volume   = -1;
      mplayer_state.position = 0;
      mplayer_state.current_song = NULL;
   }
   restarting = true;
}

void
mplayer_finish()
{
   mplayer_send_cmd("\nquit\n");

   close(mplayer_state.pipe_read);
   close(mplayer_state.pipe_write);

   waitpid(mplayer_state.pid, NULL, 0);
}

void
mplayer_restart()
{
   int status;

   close(mplayer_state.pipe_read);
   close(mplayer_state.pipe_write);
   wait(&status);

   restarting = true;
   mplayer_start();

   if (mplayer_state.playing && !mplayer_state.paused) {
      int previous_position = mplayer_state.position;
      mplayer_play(mplayer_state.current_song);
      mplayer_seek(previous_position);
   }
}

void
mplayer_sigchld_message()
{
   /* TODO find a way to check if mplayer is in $PATH in start() */
   mplayer_callback_fatal("%s is crashing too often.  Possible causes are:\n\
   1. %s is not in your $PATH\n\
   2. The installed %s is older, and not supported by vitunes\n",
   MPLAYER_PATH, MPLAYER_PATH, MPLAYER_PATH);
}

void
mplayer_sigchld()
{
   static time_t  last_sigchld = -1;

   if (kill(mplayer_state.pid, 0) != 0) {
      if (time(0) - last_sigchld <= 1) {
         if (mplayer_callback_fatal != NULL)
            mplayer_sigchld_message();
      } else {
         mplayer_restart();
         if (mplayer_callback_error != NULL)
            mplayer_callback_error("%s died.  Restarting it.", MPLAYER_PATH);
      }   
   }

   last_sigchld = time(0);
}

void
mplayer_play(const char *file)
{
   static const char *cmd_fmt = "\nloadfile \"%s\" 0\nget_property time_pos\n";
   char *cmd;

   if (asprintf(&cmd, cmd_fmt, file) == -1)
      err(1, "%s: asprintf failed", __FUNCTION__);

   mplayer_send_cmd(cmd);
   free(cmd);

   mplayer_state.position = 0;
   mplayer_state.playing  = true;
   mplayer_state.paused   = false;
   mplayer_state.current_song = file;

   /* if we have a volume, reset it */
   if (mplayer_state.volume > -1)
      mplayer_volume_set(mplayer_state.volume);
}

void
mplayer_stop()
{
   mplayer_send_cmd("\nstop\n");

   mplayer_state.playing = false;
   mplayer_state.paused  = false;
}

void
mplayer_pause()
{
   if (!mplayer_state.playing)
      return;

   mplayer_send_cmd("\npause\n");
   mplayer_state.paused = !mplayer_state.paused;
}

void
mplayer_seek(int seconds)
{
   static const char *cmd_fmt = "\nseek %i 0\nget_property time_pos\n";
   char *cmd;

   if (!mplayer_state.playing)
      return;

   if (asprintf(&cmd, cmd_fmt, seconds) == -1)
      err(1, "%s: asprintf failed", __FUNCTION__);

   mplayer_send_cmd(cmd);
   free(cmd);

   if (mplayer_state.paused)
      mplayer_state.paused = false;
}

void
mplayer_volume_step(float percent)
{
   static const char *cmd_fmt = "\npausing_keep volume %f\n";
   char *cmd;

   if (!mplayer_state.playing)
      return;

   /* this is a hack, since mplayer's volume command doesn't seem to work
    * as documented.
    */
   if (mplayer_state.volume > -1) {
      percent += mplayer_state.volume;
      mplayer_volume_set(percent);
      return;
   }

   if (asprintf(&cmd, cmd_fmt, percent) == -1)
      err(1, "%s: asprintf failed", __FUNCTION__);

   mplayer_send_cmd(cmd);
   free(cmd);

   mplayer_volume_query();
}

void
mplayer_volume_set(float percent)
{
   static const char *cmd_fmt = "\npausing_keep set_property volume %f\n";
   char *cmd;

   if (!mplayer_state.playing)
      return;

   if (percent > 100) percent = 100;
   if (percent < 0)   percent = 0;

   if (asprintf(&cmd, cmd_fmt, percent) == -1)
      err(1, "%s: asprintf failed", __FUNCTION__);


   mplayer_send_cmd(cmd);
   free(cmd);

   mplayer_volume_query();
}

void
mplayer_volume_query()
{
   static const char *cmd = "\npausing_keep get_property volume\n";

   if (!mplayer_state.playing)
      return;

   mplayer_send_cmd(cmd);
}

/* query functions */
float mplayer_get_position() { return mplayer_state.position; }
float mplayer_get_volume()   { return mplayer_state.volume; }
bool  mplayer_is_playing()   { return mplayer_state.playing; }
bool  mplayer_is_paused()    { return mplayer_state.paused; }

/* set-callback functions */
void
mplayer_set_callback_playnext(void (*f)(void))
{
   mplayer_callback_playnext = f;
}

void
mplayer_set_callback_notice(void (*f)(const char *, ...))
{
   mplayer_callback_notice = f;
}

void
mplayer_set_callback_error(void (*f)(const char *, ...))
{
   mplayer_callback_error = f;
}

void
mplayer_set_callback_fatal(void (*f)(const char *, ...))
{
   mplayer_callback_fatal = f;
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
mplayer_monitor()
{
   static const char *query_cmd   = "\nget_property time_pos\n";
   static const char *answer_fail = "ANS_ERROR=PROPERTY_UNAVAILABLE";
   static const char *answer_good = "ANS_time_pos";
   static char response[1000];  /* mplayer can be noisy */
   char *s;
   int   nbytes;

   /* in this case, nothing to monitor */
   if (!mplayer_state.playing || mplayer_state.paused)
      return;

   /* read any output from the player */
   bzero(response, sizeof(response));
   nbytes = read(mplayer_state.pipe_read, &response, sizeof(response));

   if (nbytes == -1 && errno == EAGAIN)
      return;

   response[nbytes + 1] = '\0';

   /* case: reached end of playback for a given file */
   if (strstr(response, answer_fail) != NULL) {
      if (mplayer_callback_playnext != NULL) mplayer_callback_playnext();
      return;
   }

   /* case: continue in playing current file.  update position */
   if ((s = strstr(response, answer_good)) != NULL) {
      while (strstr(s + 1, answer_good) != NULL)
         s = strstr(s + 1, answer_good);

      if (sscanf(s, "ANS_time_pos=%20f", &mplayer_state.position) != 1)
         errx(1, "player_monitor: player child is misbehaving.");
   }

   mplayer_send_cmd(query_cmd);

   /* check for recent volume */
   static const char *volume_good  = "ANS_volume";
   if ((s = strstr(response, volume_good)) != NULL) {
      while (strstr(s + 1, volume_good) != NULL)
         s = strstr(s + 1, volume_good);

      if (sscanf(s, "ANS_volume=%20f", &mplayer_state.volume) != 1)
         errx(1, "player_monitor: player child is misbehaving.");
   }
}

