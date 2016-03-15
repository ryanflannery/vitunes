#ifndef VITUNES_LIBVLC_H
#define VITUNES_LIBVLC_H

#include <sys/types.h>
#include <vlc/vlc.h>
#include <stdbool.h>

void libvlc_start();
void libvlc_finish();
void libvlc_sigchld();

void libvlc_play(const char *file);
void libvlc_stop();
void libvlc_pause();
void libvlc_seek(int nsecs);
void libvlc_volume_step(float step);

float libvlc_get_position();
float libvlc_get_volume();
bool  libvlc_get_playing();
bool  libvlc_get_paused();

void libvlc_set_callback_playnext(void (*f)(void));
void libvlc_set_callback_notice(void (*f)(char *, ...));
void libvlc_set_callback_error(void (*f)(char *, ...));
void libvlc_set_callback_fatal(void (*f)(char *, ...));

void libvlc_monitor();

#endif
