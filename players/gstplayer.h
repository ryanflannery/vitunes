/*
 * Copyright (c) 2011 Daniel Walter <sahne@0x90.at>
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

/* 
 * gstplayer.h
 *
 * This gstreamer based player is part of vitunes, a great player written
 * by Ryan Flannery.
 */

#ifndef VITUNES_GSTPLAYER_H
#define VITUNES_GSTPLAYER_H

#include <gst/gst.h>
#include <stdbool.h>

typedef struct {
   float        position;
   float        volume;
   bool         playing;
   bool         paused; 
   bool         about_to_finish;
   /* callback functions */
   void	(*playnext_cb)(void);
   void (*notice_cb)(char *, ...);
   void (*error_cb)(char *, ...);
   void (*fatal_cb)(char *, ...);

   /* backend data */
   GstElement  *player;
   GstBus      *bus;
} gst_player;

void  gstplayer_init(void);
void  gstplayer_cleanup(void);

void  gstplayer_stop(void);
void  gstplayer_play(const char *);
void  gstplayer_pause(void);

void  gstplayer_seek(int);
void  gstplayer_volume_step(float);

float gstplayer_get_position(void);
float gstplayer_get_volume(void);
bool  gstplayer_is_playing(void);
bool  gstplayer_is_paused(void);

void  gstplayer_set_callback_playnext(void (*f)(void));
void  gstplayer_set_callback_notice(void (*f)(char *, ...));
void  gstplayer_set_callback_error(void (*f)(char *, ...));
void  gstplayer_set_callback_fatal(void (*f)(char *, ...));

void  gstplayer_monitor(void);

#endif /* VITUNES_GSTPLAYER_H */
