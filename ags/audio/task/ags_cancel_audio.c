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

#include <ags/audio/task/ags_cancel_audio.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

void ags_cancel_audio_class_init(AgsCancelAudioClass *cancel_audio);
void ags_cancel_audio_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_cancel_audio_init(AgsCancelAudio *cancel_audio);
void ags_cancel_audio_connect(AgsConnectable *connectable);
void ags_cancel_audio_disconnect(AgsConnectable *connectable);
void ags_cancel_audio_finalize(GObject *gobject);

void ags_cancel_audio_launch(AgsTask *task);

/**
 * SECTION:ags_cancel_audio
 * @short_description: cancel audio object in audio loop
 * @title: AgsCancelAudio
 * @section_id:
 * @include: ags/audio/task/ags_cancel_audio.h
 *
 * The #AgsCancelAudio task cancels #AgsAudio playback.
 */

static gpointer ags_cancel_audio_parent_class = NULL;
static AgsConnectableInterface *ags_cancel_audio_parent_connectable_interface;

GType
ags_cancel_audio_get_type()
{
  static GType ags_type_cancel_audio = 0;

  if(!ags_type_cancel_audio){
    static const GTypeInfo ags_cancel_audio_info = {
      sizeof (AgsCancelAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_cancel_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCancelAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_cancel_audio_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_cancel_audio_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_cancel_audio = g_type_register_static(AGS_TYPE_TASK,
						   "AgsCancelAudio\0",
						   &ags_cancel_audio_info,
						   0);
    
    g_type_add_interface_static(ags_type_cancel_audio,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_cancel_audio);
}

void
ags_cancel_audio_class_init(AgsCancelAudioClass *cancel_audio)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_cancel_audio_parent_class = g_type_class_peek_parent(cancel_audio);

  /* gobject */
  gobject = (GObjectClass *) cancel_audio;

  gobject->finalize = ags_cancel_audio_finalize;

  /* task */
  task = (AgsTaskClass *) cancel_audio;

  task->launch = ags_cancel_audio_launch;
}

void
ags_cancel_audio_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_cancel_audio_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_cancel_audio_connect;
  connectable->disconnect = ags_cancel_audio_disconnect;
}

void
ags_cancel_audio_init(AgsCancelAudio *cancel_audio)
{
  cancel_audio->audio = NULL;

  cancel_audio->do_playback = FALSE;
  cancel_audio->do_sequencer = FALSE;
  cancel_audio->do_notation = FALSE;
}

void
ags_cancel_audio_connect(AgsConnectable *connectable)
{
  ags_cancel_audio_parent_connectable_interface->connect(connectable);


  /* empty */
}

void
ags_cancel_audio_disconnect(AgsConnectable *connectable)
{
  ags_cancel_audio_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_cancel_audio_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_cancel_audio_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_cancel_audio_launch(AgsTask *task)
{
  AgsPlaybackDomain *playback_domain;
  AgsAudio *audio;
  AgsChannel *channel;

  AgsCancelAudio *cancel_audio;

  cancel_audio = AGS_CANCEL_AUDIO(task);

  audio = cancel_audio->audio;
  playback_domain = AGS_PLAYBACK_DOMAIN(audio->playback_domain);
  
  /* cancel playback */
  if(cancel_audio->do_playback){
    g_atomic_int_and(&(playback_domain->flags),
		     (~AGS_PLAYBACK_DOMAIN_PLAYBACK));

    channel = audio->output;

    while(channel != NULL){
      if(AGS_PLAYBACK(channel->playback)->recall_id[0] == NULL){
	channel = channel->next;
	
	continue;
      }

      g_object_ref(AGS_PLAYBACK(channel->playback)->recall_id[0]);
      ags_channel_tillrecycling_cancel(channel,
				       AGS_PLAYBACK(channel->playback)->recall_id[0]);
      AGS_PLAYBACK(channel->playback)->recall_id[0] = NULL;

      channel = channel->next;
    }

    if((AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
      ags_thread_stop(playback_domain->audio_thread[0]);
    }
  }

  /* cancel sequencer */
  if(cancel_audio->do_sequencer){
    g_atomic_int_and(&(playback_domain->flags),
		     (~AGS_PLAYBACK_DOMAIN_SEQUENCER));

    channel = audio->output;

    while(channel != NULL){
      if(AGS_PLAYBACK(channel->playback)->recall_id[1] == NULL){
	channel = channel->next;
	
	continue;
      }

      g_object_ref(AGS_PLAYBACK(channel->playback)->recall_id[1]);
      ags_channel_tillrecycling_cancel(channel,
				       AGS_PLAYBACK(channel->playback)->recall_id[1]);
      AGS_PLAYBACK(channel->playback)->recall_id[1] = NULL;
      
      channel = channel->next;
    }

    if((AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
      ags_thread_stop(playback_domain->audio_thread[1]);
    }
  }

  /* cancel notation */
  if(cancel_audio->do_notation){
    g_atomic_int_and(&(playback_domain->flags),
		     (~AGS_PLAYBACK_DOMAIN_NOTATION));

    channel = audio->output;

    while(channel != NULL){
      if(AGS_PLAYBACK(channel->playback)->recall_id[2] == NULL){
	channel = channel->next;
	
	continue;
      }

      g_object_ref(AGS_PLAYBACK(channel->playback)->recall_id[2]);
      ags_channel_tillrecycling_cancel(channel,
				       AGS_PLAYBACK(channel->playback)->recall_id[2]);
      AGS_PLAYBACK(channel->playback)->recall_id[2] = NULL;
      
      channel = channel->next;
    }

    if((AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
      ags_thread_stop(playback_domain->audio_thread[2]);
    }
  }
}

/**
 * ags_cancel_audio_new:
 * @audio: the #AgsAudio to cancel
 * @do_playback: if %TRUE playback is canceld
 * @do_sequencer: if %TRUE sequencer is canceld
 * @do_notation: if %TRUE notation is canceld
 *
 * Creates an #AgsCancelAudio.
 *
 * Returns: an new #AgsCancelAudio.
 *
 * Since: 0.4
 */
AgsCancelAudio*
ags_cancel_audio_new(AgsAudio *audio,
		     gboolean do_playback, gboolean do_sequencer, gboolean do_notation)
{
  AgsCancelAudio *cancel_audio;

  cancel_audio = (AgsCancelAudio *) g_object_new(AGS_TYPE_CANCEL_AUDIO,
						 NULL);

  cancel_audio->audio = audio;

  cancel_audio->do_playback = do_playback;
  cancel_audio->do_sequencer = do_sequencer;
  cancel_audio->do_notation = do_notation;

  return(cancel_audio);
}