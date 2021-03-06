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

#include <ags/audio/ags_recall_audio.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>
#include <ags/object/ags_packable.h>

#include <ags/audio/ags_automation.h>
#include <ags/audio/ags_recall_container.h>

#include <math.h>

void ags_recall_audio_class_init(AgsRecallAudioClass *recall_audio);
void ags_recall_audio_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recall_audio_packable_interface_init(AgsPackableInterface *packable);
void ags_recall_audio_init(AgsRecallAudio *recall_audio);
void ags_recall_audio_set_property(GObject *gobject,
				   guint prop_id,
				   const GValue *value,
				   GParamSpec *param_spec);
void ags_recall_audio_get_property(GObject *gobject,
				   guint prop_id,
				   GValue *value,
				   GParamSpec *param_spec);
void ags_recall_audio_connect(AgsConnectable *connectable);
void ags_recall_audio_disconnect(AgsConnectable *connectable);
gboolean ags_recall_audio_pack(AgsPackable *packable, GObject *container);
gboolean ags_recall_audio_unpack(AgsPackable *packable);
void ags_recall_audio_dispose(GObject *gobject);
void ags_recall_audio_finalize(GObject *gobject);

void ags_recall_audio_load_automation(AgsRecall *recall,
				      GList *automation_port);
void ags_recall_audio_unload_automation(AgsRecall *recall);
void ags_recall_audio_automate(AgsRecall *recall);
AgsRecall* ags_recall_audio_duplicate(AgsRecall *recall,
				      AgsRecallID *recall_id,
				      guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_recall_audio
 * @short_description: audio context of recall
 * @title: AgsRecallAudio
 * @section_id:
 * @include: ags/audio/ags_recall_audio.h
 *
 * #AgsRecallAudio acts as audio recall.
 */

enum{
  PROP_0,
  PROP_AUDIO,
};

static gpointer ags_recall_audio_parent_class = NULL;
static AgsConnectableInterface* ags_recall_audio_parent_connectable_interface;
static AgsPackableInterface* ags_recall_audio_parent_packable_interface;

GType
ags_recall_audio_get_type()
{
  static GType ags_type_recall_audio = 0;

  if(!ags_type_recall_audio){
    static const GTypeInfo ags_recall_audio_info = {
      sizeof (AgsRecallAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_audio_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_audio_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_packable_interface_info = {
      (GInterfaceInitFunc) ags_recall_audio_packable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_recall_audio = g_type_register_static(AGS_TYPE_RECALL,
						   "AgsRecallAudio\0",
						   &ags_recall_audio_info,
						   0);

    g_type_add_interface_static(ags_type_recall_audio,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_recall_audio,
				AGS_TYPE_PACKABLE,
				&ags_packable_interface_info);
  }

  return(ags_type_recall_audio);
}

void
ags_recall_audio_class_init(AgsRecallAudioClass *recall_audio)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_recall_audio_parent_class = g_type_class_peek_parent(recall_audio);

  /* GObjectClass */
  gobject = (GObjectClass *) recall_audio;

  gobject->dispose = ags_recall_audio_dispose;
  gobject->finalize = ags_recall_audio_finalize;

  gobject->set_property = ags_recall_audio_set_property;
  gobject->get_property = ags_recall_audio_get_property;

  /* properties */
  /**
   * AgsRecallAudio:audio:
   *
   * The assigned audio.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("audio\0",
				   "assigned audio\0",
				   "The audio object it is assigned to\0",
				   AGS_TYPE_AUDIO,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) recall_audio;

  recall->load_automation = ags_recall_audio_load_automation;
  recall->unload_automation = ags_recall_audio_unload_automation;
  recall->automate = ags_recall_audio_automate;
  recall->duplicate = ags_recall_audio_duplicate;
}

void
ags_recall_audio_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_recall_audio_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recall_audio_connect;
  connectable->disconnect = ags_recall_audio_disconnect;
}

void
ags_recall_audio_packable_interface_init(AgsPackableInterface *packable)
{
  ags_recall_audio_parent_packable_interface = g_type_interface_peek_parent(packable);

  packable->pack = ags_recall_audio_pack;
  packable->unpack = ags_recall_audio_unpack;
}

void
ags_recall_audio_init(AgsRecallAudio *recall_audio)
{
  recall_audio->flags = 0;

  recall_audio->samplerate = 0;
  recall_audio->buffer_size = 0;
  recall_audio->audio_buffer_util_format = 0;
  
  recall_audio->n_channels = 0;
  recall_audio->mapping = NULL;
  
  recall_audio->audio = NULL;
}

void
ags_recall_audio_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec)
{
  AgsRecallAudio *recall_audio;

  recall_audio = AGS_RECALL_AUDIO(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    {
      AgsAudio *audio;

      guint i;
      
      audio = (AgsAudio *) g_value_get_object(value);

      if(recall_audio->audio == audio){
	return;
      }

      if(recall_audio->audio != NULL){
	g_object_unref(recall_audio->audio);
      }
      
      if(audio != NULL){
	g_object_ref(audio);

	/* get audio channels */
	recall_audio->n_channels = audio->audio_channels;
	
	/* allocate mapping */
	recall_audio->mapping = (guint *) malloc(recall_audio->n_channels * sizeof(guint));

	for(i = 0; i < recall_audio->n_channels; i++){
	  recall_audio->mapping[i] = i;
	}
      }
      
      recall_audio->audio = audio;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recall_audio_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec)
{
  AgsRecallAudio *recall_audio;

  recall_audio = AGS_RECALL_AUDIO(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    g_value_set_object(value, recall_audio->audio);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recall_audio_connect(AgsConnectable *connectable)
{
  ags_recall_audio_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_recall_audio_disconnect(AgsConnectable *connectable)
{
  ags_recall_audio_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

gboolean
ags_recall_audio_pack(AgsPackable *packable, GObject *container)
{
  AgsRecallContainer *recall_container;
  GList *list;

  if(ags_recall_audio_parent_packable_interface->pack(packable, container))
    return(TRUE);

  recall_container = AGS_RECALL_CONTAINER(container);

  g_object_set(G_OBJECT(recall_container),
	       "recall_audio\0", AGS_RECALL(packable),
	       NULL);

  /* set in AgsRecallAudioRun */
  list = recall_container->recall_audio_run;

  while(list != NULL){
    g_object_set(G_OBJECT(list->data),
		 "recall_audio\0", AGS_RECALL_AUDIO(packable),
		 NULL);

    list = list->next;
  }

  return(FALSE);
}

gboolean
ags_recall_audio_unpack(AgsPackable *packable)
{
  AgsRecall *recall;
  AgsRecallContainer *recall_container;
  GList *list;

  recall = AGS_RECALL(packable);

  if(recall == NULL){
    return(TRUE);
  }

  recall_container = AGS_RECALL_CONTAINER(recall->container);

  if(recall_container == NULL){
    return(TRUE);
  }

  /* ref */
  g_object_ref(recall);
  g_object_ref(recall_container);

  /* unset in AgsRecallAudioRun */
  list = recall_container->recall_audio_run;

  while(list != NULL){
    g_object_set(G_OBJECT(list->data),
		 "recall_audio\0", NULL,
		 NULL);

    list = list->next;
  }

  /* call parent */
  if(ags_recall_audio_parent_packable_interface->unpack(packable)){
    g_object_unref(recall);
    g_object_unref(recall_container);

    return(TRUE);
  }

  recall_container->recall_audio = NULL;

  /* unref */
  g_object_unref(recall);
  g_object_unref(recall_container);

  return(FALSE);
}

void
ags_recall_audio_dispose(GObject *gobject)
{
  AgsRecallAudio *recall_audio;

  recall_audio = AGS_RECALL_AUDIO(gobject);

  /* unpack */
  ags_packable_unpack(AGS_PACKABLE(recall_audio));
  
  /* audio */
  if(recall_audio->audio != NULL){
    g_object_unref(G_OBJECT(recall_audio->audio));

    recall_audio->audio = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_recall_audio_parent_class)->dispose(gobject);
}

void
ags_recall_audio_finalize(GObject *gobject)
{
  AgsRecallAudio *recall_audio;

  recall_audio = AGS_RECALL_AUDIO(gobject);

  /* audio */
  if(recall_audio->audio != NULL){
    g_object_unref(G_OBJECT(recall_audio->audio));
  }

  if(recall_audio->mapping != NULL){
    free(recall_audio->mapping);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_recall_audio_parent_class)->finalize(gobject);
}

void
ags_recall_audio_load_automation(AgsRecall *recall,
				 GList *automation_port)
{
  AgsAudio *audio;

  AgsAutomation *current;

  GList *automation;
  
  audio = AGS_RECALL_AUDIO(recall)->audio;

  while(automation_port != NULL){
    if((AGS_PORT_IS_OUTPUT & (AGS_PORT(automation_port->data)->flags)) != 0){
      automation_port = automation_port->next;

      continue;
    }

    if(ags_automation_find_port(audio->automation,
				automation_port->data) == NULL){
      current = ags_automation_new((GObject *) audio,
				   0,
				   G_TYPE_NONE,
				   AGS_PORT(automation_port->data)->specifier);
      g_object_set(current,
		   "port\0", automation_port->data,
		   NULL);
      ags_audio_add_automation(audio,
			       (GObject *) current);

      //TODO:JK: property
      AGS_PORT(automation_port->data)->automation = (GObject *) current;
    }
    
    automation_port = automation_port->next;
  }
}

void
ags_recall_audio_unload_automation(AgsRecall *recall)
{
  AgsAudio *audio;

  AgsAutomation *current;
  
  GList *automation;
  GList *automation_port;
  
  audio = AGS_RECALL_AUDIO(recall)->audio;

  automation_port = recall->automation_port;
  
  while(automation_port != NULL){
    if((automation = ags_automation_find_port(audio->automation,
					      (GObject *) automation_port->data)) != NULL){
      current = automation->data;
      ags_audio_remove_automation(audio,
				  (GObject *) current);
    }
    
    automation_port = automation_port->next;
  }

  g_list_free(recall->automation_port);
  recall->automation_port = NULL;
}

void
ags_recall_audio_automate(AgsRecall *recall)
{
  GObject *soundcard;
  AgsAudio *audio;

  AgsAutomation *automation;
  GList *port;

  gdouble delay;
  guint note_offset, delay_counter;
  
  guint loop_left, loop_right;
  gboolean do_loop;

  double x, step;
  guint ret_x;
  gboolean return_prev_on_failure;

  audio = AGS_RECALL_AUDIO(recall)->audio;
  soundcard = audio->soundcard;

  /* retrieve position */
  port = recall->port;

  note_offset = ags_soundcard_get_note_offset(AGS_SOUNDCARD(soundcard));
  
  delay = ags_soundcard_get_delay(AGS_SOUNDCARD(soundcard));
  delay_counter = ags_soundcard_get_delay_counter(AGS_SOUNDCARD(soundcard));

  /* retrieve loop information */
  ags_soundcard_get_loop(AGS_SOUNDCARD(soundcard),
			 &loop_left, &loop_right,
			 &do_loop);

  return_prev_on_failure = TRUE;

  if(do_loop &&
     loop_left <= note_offset){
    if(note_offset == loop_left){
      return_prev_on_failure = TRUE;
    }
  }

  /*  */
  x = ((double) note_offset + (delay_counter / delay)) * ((1.0 / AGS_AUTOMATION_MINIMUM_ACCELERATION_LENGTH) * AGS_NOTATION_MINIMUM_NOTE_LENGTH);
  step = ((1.0 / AGS_AUTOMATION_MINIMUM_ACCELERATION_LENGTH) * AGS_NOTATION_MINIMUM_NOTE_LENGTH);

  while(port != NULL){
    automation = (AgsAutomation *) AGS_PORT(port->data)->automation;

    if(automation != NULL &&
       (AGS_AUTOMATION_BYPASS & (automation->flags)) == 0){
      GValue value = {0,};

      ret_x = ags_automation_get_value(automation,
				       floor(x), ceil(x + step),
				       return_prev_on_failure,
				       &value);

      if(ret_x != G_MAXUINT){
	ags_port_safe_write(port->data,
			    &value);
      }
    }

    port = port->next;
  }
}

AgsRecall*
ags_recall_audio_duplicate(AgsRecall *recall,
			   AgsRecallID *recall_id,
			   guint *n_params, GParameter *parameter)
{
  AgsRecallAudio *copy;

  /* duplicate */
  copy = AGS_RECALL_AUDIO(AGS_RECALL_CLASS(ags_recall_audio_parent_class)->duplicate(recall,
										     recall_id,
										     n_params, parameter));

  g_message("ags warning - ags_recall_audio_duplicate: you shouldn't do this %s\n\0", G_OBJECT_TYPE_NAME(recall));
  
  return((AgsRecall *) copy);
}

/**
 * ags_recall_audio_new:
 * @audio: the assigned #AgsAudio
 *
 * Creates an #AgsRecallAudio.
 *
 * Returns: a new #AgsRecallAudio.
 *
 * Since: 0.4
 */
AgsRecallAudio*
ags_recall_audio_new(AgsAudio *audio)
{
  AgsRecallAudio *recall_audio;

  recall_audio = (AgsRecallAudio *) g_object_new(AGS_TYPE_RECALL_AUDIO,
						 "audio\0", audio,
						 NULL);

  return(recall_audio);
}
