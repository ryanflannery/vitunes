#include "libvlc.h"

libvlc_instance_t      *inst;
libvlc_media_player_t  *mp;
libvlc_media_t         *m;

void
libvlc_start()
{
   inst = libvlc_new(0, NULL);
}

void
libvlc_finish()
{
   libvlc_media_player_release(mp);
   libvlc_release(inst);
}

void
libvlc_sigchld()
{
}

void
libvlc_play(const char *file)
{
   /* create a new item */
   m = libvlc_media_new_path(inst, file);

   /* create a media play playing environment */
   mp = libvlc_media_player_new_from_media(m);

   /* no need to keep the media now */
   libvlc_media_release(m);

   /* play the media_player */
   libvlc_media_player_play(mp);
}

void
libvlc_stop()
{
   libvlc_media_player_stop(mp);
}

void
libvlc_pause()
{
}

void
libvlc_seek(int nsecs)
{
}

void
libvlc_volume_step(float step)
{
}

float
libvlc_get_position()
{
}

float
libvlc_get_volume()
{
}

bool
libvlc_get_playing()
{
}

bool
libvlc_get_paused()
{
}

void
libvlc_set_callback_playnext(void (*f)(void))
{
}

void
libvlc_set_callback_notice(void (*f)(char *, ...))
{
}

void
libvlc_set_callback_error(void (*f)(char *, ...))
{
}

void
libvlc_set_callback_fatal(void (*f)(char *, ...))
{
}

void
libvlc_monitor()
{
}
