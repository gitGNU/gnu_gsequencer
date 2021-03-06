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

#include <ags/audio/task/ags_add_audio.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

void ags_add_audio_class_init(AgsAddAudioClass *add_audio);
void ags_add_audio_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_add_audio_init(AgsAddAudio *add_audio);
void ags_add_audio_set_property(GObject *gobject,
				guint prop_id,
				const GValue *value,
				GParamSpec *param_spec);
void ags_add_audio_get_property(GObject *gobject,
				guint prop_id,
				GValue *value,
				GParamSpec *param_spec);
void ags_add_audio_connect(AgsConnectable *connectable);
void ags_add_audio_disconnect(AgsConnectable *connectable);
void ags_add_audio_finalize(GObject *gobject);

void ags_add_audio_launch(AgsTask *task);

enum{
  PROP_0,
  PROP_SOUNDCARD,
  PROP_AUDIO,
};

/**
 * SECTION:ags_add_audio
 * @short_description: add audio object to soundcard
 * @title: AgsAddAudio
 * @section_id:
 * @include: ags/audio/task/ags_add_audio.h
 *
 * The #AgsAddAudio task adds #AgsAudio to #AgsSoundcard.
 */

static gpointer ags_add_audio_parent_class = NULL;
static AgsConnectableInterface *ags_add_audio_parent_connectable_interface;

GType
ags_add_audio_get_type()
{
  static GType ags_type_add_audio = 0;

  if(!ags_type_add_audio){
    static const GTypeInfo ags_add_audio_info = {
      sizeof (AgsAddAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_add_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAddAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_add_audio_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_add_audio_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_add_audio = g_type_register_static(AGS_TYPE_TASK,
						"AgsAddAudio\0",
						&ags_add_audio_info,
						0);
    
    g_type_add_interface_static(ags_type_add_audio,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_add_audio);
}

void
ags_add_audio_class_init(AgsAddAudioClass *add_audio)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_add_audio_parent_class = g_type_class_peek_parent(add_audio);

  /* gobject */
  gobject = (GObjectClass *) add_audio;

  gobject->set_property = ags_add_audio_set_property;
  gobject->get_property = ags_add_audio_get_property;

  gobject->finalize = ags_add_audio_finalize;

  /* properties */
  /**
   * AgsAddAudio:soundcard:
   *
   * The assigned #AgsSoundcard
   * 
   * Since: 0.7.117
   */
  param_spec = g_param_spec_object("soundcard\0",
				   "soundcard of add audio\0",
				   "The soundcard of add audio task\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);

  /**
   * AgsAddAudio:audio:
   *
   * The assigned #AgsAudio
   * 
   * Since: 0.7.117
   */
  param_spec = g_param_spec_object("audio\0",
				   "audio of add audio\0",
				   "The audio of add audio task\0",
				   AGS_TYPE_AUDIO,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) add_audio;

  task->launch = ags_add_audio_launch;
}

void
ags_add_audio_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_add_audio_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_add_audio_connect;
  connectable->disconnect = ags_add_audio_disconnect;
}

void
ags_add_audio_init(AgsAddAudio *add_audio)
{
  add_audio->soundcard = NULL;
  add_audio->audio = NULL;
}

void
ags_add_audio_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec)
{
  AgsAddAudio *add_audio;

  add_audio = AGS_ADD_AUDIO(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      soundcard = (GObject *) g_value_get_object(value);

      if(add_audio->soundcard == (GObject *) soundcard){
	return;
      }

      if(add_audio->soundcard != NULL){
	g_object_unref(add_audio->soundcard);
      }

      if(soundcard != NULL){
	g_object_ref(soundcard);
      }

      add_audio->soundcard = (GObject *) soundcard;
    }
    break;
  case PROP_AUDIO:
    {
      AgsAudio *audio;

      audio = (AgsAudio *) g_value_get_object(value);

      if(add_audio->audio == (GObject *) audio){
	return;
      }

      if(add_audio->audio != NULL){
	g_object_unref(add_audio->audio);
      }

      if(audio != NULL){
	g_object_ref(audio);
      }

      add_audio->audio = (GObject *) audio;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_add_audio_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec)
{
  AgsAddAudio *add_audio;

  add_audio = AGS_ADD_AUDIO(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      g_value_set_object(value, add_audio->soundcard);
    }
    break;
  case PROP_AUDIO:
    {
      g_value_set_object(value, add_audio->audio);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_add_audio_connect(AgsConnectable *connectable)
{
  ags_add_audio_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_add_audio_disconnect(AgsConnectable *connectable)
{
  ags_add_audio_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_add_audio_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_add_audio_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_add_audio_launch(AgsTask *task)
{
  AgsAddAudio *add_audio;
  GList *list;
  
  add_audio = AGS_ADD_AUDIO(task);

  //TODO:JK: use AgsSoundcard
  
  /* add audio */
  g_object_ref(G_OBJECT(add_audio->audio));
  
  list = ags_soundcard_get_audio(AGS_SOUNDCARD(add_audio->soundcard));
  list = g_list_prepend(list,
			add_audio->audio);
  ags_soundcard_set_audio(AGS_SOUNDCARD(add_audio->soundcard),
			  list);
  
  /* AgsAudio */
  ags_connectable_connect(AGS_CONNECTABLE(add_audio->audio));
}

/**
 * ags_add_audio_new:
 * @soundcard: the #AgsSoundcard
 * @audio: the #AgsAudio to add
 *
 * Creates an #AgsAddAudio.
 *
 * Returns: an new #AgsAddAudio.
 *
 * Since: 0.4
 */
AgsAddAudio*
ags_add_audio_new(GObject *soundcard,
		  AgsAudio *audio)
{
  AgsAddAudio *add_audio;

  add_audio = (AgsAddAudio *) g_object_new(AGS_TYPE_ADD_AUDIO,
					   NULL);

  add_audio->soundcard = soundcard;
  add_audio->audio = audio;

  return(add_audio);
}
