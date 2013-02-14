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
 * gstplayer.c
 *
 * This gstreamer based player is part of vitunes, a great player written
 * by Ryan Flannery.
 */

#include "gstplayer.h"
#include "../player.h"

/* player data */
static gst_player gplayer;


/* callback handler for gapless playback */
void
gstplayer_handle_about_to_finish(GstElement *obj UNUSED, gpointer userdata UNUSED)
{
   gplayer.about_to_finish = true;
   if (gplayer.playnext_cb)
      gplayer.playnext_cb();
}

void
gstplayer_init()
{
   /* 
    * - init gstreamer
    * - create playbin2
    * - 
    */
   GstElement *player;
   GstBus     *bus;
   GstElement *video_sink;

   /* init gstreamer */
   gst_init(NULL, NULL);
   /* create player */
   player = gst_element_factory_make("playbin2", "vitunes");
   if (!player)
      gplayer.fatal_cb("gstplayer_init: could not create player");
   /* create fake video-sink, since we're an audio player */
   video_sink = gst_element_factory_make("fakesink", "video-sink");
   if (!video_sink) {
      gst_object_unref(GST_OBJECT(player));
      gplayer.fatal_cb("gstplayer_init: could not create fakesink");
   }
   /* add fake video sink to pipeline */
   g_object_set(G_OBJECT(player), "video-sink", video_sink, NULL);
   /* get bus */
   bus = gst_pipeline_get_bus(GST_PIPELINE(player));
   if (!bus){
      gst_object_unref(GST_OBJECT(video_sink));
      gst_object_unref(GST_OBJECT(player));
      gplayer.fatal_cb("gstplayer_init: could not create gstreamer bus");
   }
   /* gapless playback */
   g_signal_connect(G_OBJECT(player), "about-to-finish",
                    G_CALLBACK(gstplayer_handle_about_to_finish), NULL);
   /* update gplayer struct */
   gplayer.player = player;
   gplayer.bus = bus;
   gplayer.about_to_finish = false;
}


/* play via gstreamer */
void
gstplayer_play(const char *filename)
{
   gchar *uri;
   if (!filename)
      return;
   if (!gplayer.player)
      return;

   uri = (gchar *)filename;
   if (! gst_uri_is_valid(uri))
      uri = g_filename_to_uri(filename, NULL, NULL);
   /* set actual state to pause*/
   if (!gplayer.about_to_finish)
      gst_element_set_state(GST_ELEMENT(gplayer.player), GST_STATE_READY);
   /* load song */
   g_object_set(G_OBJECT(gplayer.player), "uri", uri, NULL);
   /* start playback */
   if (!gplayer.about_to_finish)
      gst_element_set_state(GST_ELEMENT(gplayer.player), GST_STATE_PLAYING);
   gplayer.playing = true;
   gplayer.paused = false;
   gplayer.about_to_finish = false;
   g_free(uri);
}

/* pause / unpause playback */
void
gstplayer_pause()
{
   /* assertions */
   if (!gplayer.player)
      gplayer.fatal_cb("gstplayer_pause: player not initialized\n");

   if (! gplayer.paused) {
           /* set paused state in player */
           gst_element_set_state(GST_ELEMENT(gplayer.player),
                                 GST_STATE_PAUSED);
   } else { 
           /* set play state in player */
           gst_element_set_state(GST_ELEMENT(gplayer.player),
                                 GST_STATE_PLAYING);
   }

   /* update state */
   gplayer.paused = !gplayer.paused;
}

/* stop playback */
void
gstplayer_stop()
{
   if (!gplayer.player)
      gplayer.fatal_cb("gstplayer_pause: player not initialized\n");

   /* set pipeline into ready state (== STOP)*/
   gst_element_set_state(GST_ELEMENT(gplayer.player), GST_STATE_READY);
   gplayer.playing = false;
   gplayer.paused = false;
   gplayer.about_to_finish = false;
}

void
gstplayer_cleanup()
{
   if (! gplayer.player)
      return;
   /* stop playbin before destroying */
   gst_element_set_state(GST_ELEMENT(gplayer.player), GST_STATE_NULL);
   /* clean up */
   gst_object_unref(G_OBJECT(gplayer.player));
}

void
gstplayer_seek(int seconds)
{
   GstFormat seek_format = GST_FORMAT_TIME;
   GstSeekFlags seek_flags = GST_SEEK_FLAG_FLUSH;
   gint64 seek;
   if (seconds == 0)
      return;
   gst_element_query_position(GST_ELEMENT(gplayer.player), &seek_format, &seek);
   seek += (seconds * GST_SECOND);
   gst_element_seek_simple(GST_ELEMENT(gplayer.player), seek_format,
                           seek_flags, seek);
   return;
}


void
gstplayer_monitor()
{
   GstMessage *msg;
   if (!gplayer.bus)
      gplayer.fatal_cb("gstplayer_pause: player not initialized\n");

   msg = gst_bus_pop(gplayer.bus);
   if (!msg)
      return;

   /* handle messages */
   switch(GST_MESSAGE_TYPE(msg)) {
   case GST_MESSAGE_EOS: {
      /* end of stream, start next */
      gst_element_set_state(GST_ELEMENT(gplayer.player), GST_STATE_READY);
      if (gplayer.playnext_cb != NULL)
         gplayer.playnext_cb();
      break;
   }
   case GST_MESSAGE_ERROR: {
      GError *error;
      gst_message_parse_error(msg, &error, NULL);
      gplayer.error_cb("player_monitor: %s", error->message);
      g_error_free(error);
      break;
   }
   default:
      break;
   }

   return;
}

bool
gstplayer_is_playing()
{
   if (!gplayer.player)
      return false;
   return gplayer.playing;
}

bool
gstplayer_is_paused()
{
   if (!gplayer.player)
      return false;
   return gplayer.paused;
}

float
gstplayer_get_volume(void)
{
   gdouble vol;
   g_object_get(G_OBJECT(gplayer.player), "volume", &vol, NULL);
   vol *= 100;
   return (float)vol;
}

void
gstplayer_volume_step(float vol)
{
   gdouble current;
   g_object_get(G_OBJECT(gplayer.player), "volume", &current, NULL);
   current += (vol / 100);
   if (current > 1.0)
      current = 1.0;
   if (current < 0.0)
      current = 0;
   g_object_set(G_OBJECT(gplayer.player), "volume", current, NULL);
   return;
}

float
gstplayer_get_position(void)
{
   GstMessage *msg;
   gint64 pos;
   GstFormat pos_format = GST_FORMAT_TIME;
   float actual_pos;
   if (!gplayer.bus)
      gplayer.fatal_cb("gstplayer_monitor: player not initialized");

   /* update time */
   gst_element_query_position(GST_ELEMENT(gplayer.player), &pos_format, &pos);
   /* position is in nanoseconds, lets convert */
   actual_pos = (float) (pos / GST_SECOND);
   msg = gst_bus_pop(gplayer.bus);
   if (!msg)
      return actual_pos;
   return actual_pos;
}

void
gstplayer_set_callback_playnext(void (*f)(void))
{
   gplayer.playnext_cb = f;
}
void
gstplayer_set_callback_notice(void (*f)(const char *, ...))
{
   gplayer.notice_cb = f;
}
void
gstplayer_set_callback_error(void (*f)(const char *, ...))
{
   gplayer.error_cb = f;
}
void 
gstplayer_set_callback_fatal(void (*f)(const char *, ...))
{
   gplayer.fatal_cb = f;
}

/* vim: set ts=3:expandtab */
