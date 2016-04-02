/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ags/audio/task/ags_append_channel.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_config.h>
#include <ags/object/ags_soundcard.h>

#include <ags/server/ags_server.h>
#include <ags/server/ags_service_provider.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

#include <ags/audio/thread/ags_audio_loop.h>
#include <ags/audio/thread/ags_audio_thread.h>
#include <ags/audio/thread/ags_channel_thread.h>

void ags_append_channel_class_init(AgsAppendChannelClass *append_channel);
void ags_append_channel_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_append_channel_init(AgsAppendChannel *append_channel);
void ags_append_channel_connect(AgsConnectable *connectable);
void ags_append_channel_disconnect(AgsConnectable *connectable);
void ags_append_channel_finalize(GObject *gobject);

void ags_append_channel_launch(AgsTask *task);

/**
 * SECTION:ags_append_channel
 * @short_description: append channel object to audio loop
 * @title: AgsAppendChannel
 * @section_id:
 * @include: ags/channel/task/ags_append_channel.h
 *
 * The #AgsAppendChannel task appends #AgsChannel to #AgsAudioLoop.
 */

static gpointer ags_append_channel_parent_class = NULL;
static AgsConnectableInterface *ags_append_channel_parent_connectable_interface;

GType
ags_append_channel_get_type()
{
  static GType ags_type_append_channel = 0;

  if(!ags_type_append_channel){
    static const GTypeInfo ags_append_channel_info = {
      sizeof (AgsAppendChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_append_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAppendChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_append_channel_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_append_channel_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_append_channel = g_type_register_static(AGS_TYPE_TASK,
						   "AgsAppendChannel\0",
						   &ags_append_channel_info,
						   0);

    g_type_add_interface_static(ags_type_append_channel,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_append_channel);
}

void
ags_append_channel_class_init(AgsAppendChannelClass *append_channel)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_append_channel_parent_class = g_type_class_peek_parent(append_channel);

  /* GObjectClass */
  gobject = (GObjectClass *) append_channel;

  gobject->finalize = ags_append_channel_finalize;

  /* AgsTaskClass */
  task = (AgsTaskClass *) append_channel;

  task->launch = ags_append_channel_launch;
}

void
ags_append_channel_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_append_channel_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_append_channel_connect;
  connectable->disconnect = ags_append_channel_disconnect;
}

void
ags_append_channel_init(AgsAppendChannel *append_channel)
{
  append_channel->audio_loop = NULL;
  append_channel->channel = NULL;
}

void
ags_append_channel_connect(AgsConnectable *connectable)
{
  ags_append_channel_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_append_channel_disconnect(AgsConnectable *connectable)
{
  ags_append_channel_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_append_channel_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_append_channel_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_append_channel_launch(AgsTask *task)
{
  AgsChannel *channel;
  
  AgsAppendChannel *append_channel;

  AgsAudioLoop *audio_loop;

  AgsServer *server;

  AgsConfig *config;

  GList *start_queue;
  
  gchar *str0, *str1;
  
  append_channel = AGS_APPEND_CHANNEL(task);

  audio_loop = AGS_AUDIO_LOOP(append_channel->audio_loop);
  channel = append_channel->channel;

  /* append to AgsDevout */
  ags_audio_loop_add_channel(audio_loop,
			    channel);
  start_queue = NULL;
  
  /**/
  config = ags_config_get_instance();
  
  str0 = ags_config_get_value(config,
			AGS_CONFIG_THREAD,
			"model\0");

  str1 = ags_config_get_value(config,
			AGS_CONFIG_THREAD,
			"super-threaded-scope\0");
  
  if(!g_ascii_strncasecmp(str0,
			  "super-threaded\0",
			  15)){
    /* super threaed setup */
    if(!g_ascii_strncasecmp(str1,
			    "channel\0",
			    8)){
      if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(AGS_PLAYBACK(channel->playback)->flags)))) != 0){
	start_queue = g_list_prepend(start_queue,
				     AGS_PLAYBACK(channel->playback)->channel_thread[0]);
	
	if(AGS_PLAYBACK(channel->playback)->channel_thread[0]->parent == NULL){
	  ags_thread_add_child_extended(audio_loop,
					AGS_PLAYBACK(channel->playback)->channel_thread[0],
					TRUE, TRUE);
	  ags_connectable_connect(AGS_CONNECTABLE(AGS_PLAYBACK(channel->playback)->channel_thread[0]));
	}
      }
    }
  }

  free(str0);
  free(str1);

  /* start queue */
  if(start_queue != NULL){
    if(g_atomic_pointer_get(&(AGS_THREAD(audio_loop)->start_queue)) != NULL){
      g_atomic_pointer_set(&(AGS_THREAD(audio_loop)->start_queue),
			   g_list_concat(start_queue,
					 g_atomic_pointer_get(&(AGS_THREAD(audio_loop)->start_queue))));
    }else{
      g_atomic_pointer_set(&(AGS_THREAD(audio_loop)->start_queue),
			   start_queue);
    }
  }

  /* add to server registry */
  //  server = ags_service_provider_get_server(AGS_SERVICE_PROVIDER(audio_loop->application_context));

  //  if(server != NULL && (AGS_SERVER_RUNNING & (server->flags)) != 0){
  //    ags_connectable_add_to_registry(AGS_CONNECTABLE(append_channel->channel));
  //  }
}

/**
 * ags_append_channel_new:
 * @audio_loop: the #AgsAudioLoop
 * @channel: the #AgsChannel to append
 *
 * Creates an #AgsAppendChannel.
 *
 * Returns: an new #AgsAppendChannel.
 *
 * Since: 0.4
 */
AgsAppendChannel*
ags_append_channel_new(GObject *audio_loop,
		       GObject *channel)
{
  AgsAppendChannel *append_channel;

  append_channel = (AgsAppendChannel *) g_object_new(AGS_TYPE_APPEND_CHANNEL,
						     NULL);

  append_channel->audio_loop = audio_loop;
  append_channel->channel = channel;

  return(append_channel);
}