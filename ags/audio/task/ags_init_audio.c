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

#include <ags/audio/task/ags_init_audio.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

void ags_init_audio_class_init(AgsInitAudioClass *init_audio);
void ags_init_audio_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_init_audio_init(AgsInitAudio *init_audio);
void ags_init_audio_connect(AgsConnectable *connectable);
void ags_init_audio_disconnect(AgsConnectable *connectable);
void ags_init_audio_finalize(GObject *gobject);

void ags_init_audio_launch(AgsTask *task);

/**
 * SECTION:ags_init_audio
 * @short_description: init audio object
 * @title: AgsInitAudio
 * @section_id:
 * @include: ags/audio/task/ags_init_audio.h
 *
 * The #AgsInitAudio task inits #AgsAudio.
 */

static gpointer ags_init_audio_parent_class = NULL;
static AgsConnectableInterface *ags_init_audio_parent_connectable_interface;

GType
ags_init_audio_get_type()
{
  static GType ags_type_init_audio = 0;

  if(!ags_type_init_audio){
    static const GTypeInfo ags_init_audio_info = {
      sizeof (AgsInitAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_init_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsInitAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_init_audio_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_init_audio_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_init_audio = g_type_register_static(AGS_TYPE_TASK,
						 "AgsInitAudio\0",
						 &ags_init_audio_info,
						 0);

    g_type_add_interface_static(ags_type_init_audio,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_init_audio);
}

void
ags_init_audio_class_init(AgsInitAudioClass *init_audio)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_init_audio_parent_class = g_type_class_peek_parent(init_audio);

  /* gobject */
  gobject = (GObjectClass *) init_audio;

  gobject->finalize = ags_init_audio_finalize;

  /* task */
  task = (AgsTaskClass *) init_audio;

  task->launch = ags_init_audio_launch;
}

void
ags_init_audio_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_init_audio_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_init_audio_connect;
  connectable->disconnect = ags_init_audio_disconnect;
}

void
ags_init_audio_init(AgsInitAudio *init_audio)
{
  init_audio->audio = NULL;

  init_audio->do_playback = FALSE;
  init_audio->do_sequencer = FALSE;
  init_audio->do_notation = FALSE;
}

void
ags_init_audio_connect(AgsConnectable *connectable)
{
  ags_init_audio_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_init_audio_disconnect(AgsConnectable *connectable)
{
  ags_init_audio_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_init_audio_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_init_audio_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_init_audio_launch(AgsTask *task)
{
  AgsInitAudio *init_audio;

  GObject *soundcard;
  AgsAudio *audio;
  AgsRecallID *recall_id;

  AgsApplicationContext *application_context;
  
  GList *playback;
  GList *list;

  init_audio = AGS_INIT_AUDIO(task);

  audio = init_audio->audio;
  soundcard = audio->soundcard;

  application_context = ags_soundcard_get_application_context(AGS_SOUNDCARD(soundcard));

  g_message("init\0");
  
  /* init audio */
  if(init_audio->do_playback){
    
    g_atomic_int_or(&(AGS_PLAYBACK_DOMAIN(audio->playback_domain)->flags),
		    AGS_PLAYBACK_DOMAIN_PLAYBACK);

    playback = AGS_PLAYBACK_DOMAIN(audio->playback_domain)->playback;
    
    list = ags_audio_recursive_play_init(audio,
					 TRUE, FALSE, FALSE);
    
    while(playback != NULL){
      //      AGS_PLAYBACK(playback->data)->recall_id[0] = list->data;
      g_atomic_int_or(&(AGS_PLAYBACK(playback->data)->flags),
		      AGS_PLAYBACK_PLAYBACK);

      playback = playback->next;
    }
  }

  if(init_audio->do_sequencer){    
    g_atomic_int_or(&(AGS_PLAYBACK_DOMAIN(audio->playback_domain)->flags),
		    AGS_PLAYBACK_DOMAIN_SEQUENCER);

    playback = AGS_PLAYBACK_DOMAIN(audio->playback_domain)->playback;

    list = ags_audio_recursive_play_init(audio,
					 FALSE, TRUE, FALSE);

    while(playback != NULL){
      //      AGS_PLAYBACK(playback->data)->recall_id[1] = list->data;
      g_atomic_int_or(&(AGS_PLAYBACK(playback->data)->flags),
		      AGS_PLAYBACK_SEQUENCER);
      
      playback = playback->next;
    }
  }

  if(init_audio->do_notation){    
    g_atomic_int_or(&(AGS_PLAYBACK_DOMAIN(audio->playback_domain)->flags),
		    AGS_PLAYBACK_DOMAIN_NOTATION);

    playback = AGS_PLAYBACK_DOMAIN(audio->playback_domain)->playback;

    list = ags_audio_recursive_play_init(audio,
					 FALSE, FALSE, TRUE);

    while(playback != NULL){
      //      AGS_PLAYBACK(playback->data)->recall_id[2] = list->data;
      g_atomic_int_or(&(AGS_PLAYBACK(playback->data)->flags),
		      AGS_PLAYBACK_NOTATION);

      playback = playback->next;
    }
  }
}

/**
 * ags_init_audio_new:
 * @audio: the #AgsAudio
 * @do_playback: init playback
 * @do_sequencer: init sequencer
 * @do_notation: init notation
 *
 * Creates an #AgsInitAudio.
 *
 * Returns: an new #AgsInitAudio.
 *
 * Since: 0.4
 */
AgsInitAudio*
ags_init_audio_new(AgsAudio *audio,
		   gboolean do_playback, gboolean do_sequencer, gboolean do_notation)
{
  AgsInitAudio *init_audio;

  init_audio = (AgsInitAudio *) g_object_new(AGS_TYPE_INIT_AUDIO,
					     NULL);

  init_audio->audio = audio;

  init_audio->do_playback = do_playback;
  init_audio->do_sequencer = do_sequencer;
  init_audio->do_notation = do_notation;

  return(init_audio);
}