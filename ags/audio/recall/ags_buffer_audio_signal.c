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

#include <ags/audio/recall/ags_buffer_audio_signal.h>
#include <ags/audio/recall/ags_buffer_recycling.h>
#include <ags/audio/recall/ags_buffer_channel.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_channel_run.h>
#include <ags/audio/ags_recall_recycling.h>
#include <ags/audio/ags_audio_buffer_util.h>

#include <ags/object/ags_config.h>
 
void ags_buffer_audio_signal_class_init(AgsBufferAudioSignalClass *buffer_audio_signal);
void ags_buffer_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_buffer_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_buffer_audio_signal_init(AgsBufferAudioSignal *buffer_audio_signal);
void ags_buffer_audio_signal_connect(AgsConnectable *connectable);
void ags_buffer_audio_signal_disconnect(AgsConnectable *connectable);
void ags_buffer_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_buffer_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_buffer_audio_signal_finalize(GObject *gobject);

void ags_buffer_audio_signal_run_init_pre(AgsRecall *recall);
void ags_buffer_audio_signal_run_pre(AgsRecall *recall);
void ags_buffer_audio_signal_run_inter(AgsRecall *recall);
AgsRecall* ags_buffer_audio_signal_duplicate(AgsRecall *recall,
					     AgsRecallID *recall_id,
					     guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_buffer_audio_signal
 * @short_description: buffers audio signal
 * @title: AgsBufferAudioSignal
 * @section_id:
 * @include: ags/audio/recall/ags_buffer_audio_signal.h
 *
 * The #AgsBufferAudioSignal class buffers the audio signal.
 */

static gpointer ags_buffer_audio_signal_parent_class = NULL;
static AgsConnectableInterface *ags_buffer_audio_signal_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_buffer_audio_signal_parent_dynamic_connectable_interface;

GType
ags_buffer_audio_signal_get_type()
{
  static GType ags_type_buffer_audio_signal = 0;

  if(!ags_type_buffer_audio_signal){
    static const GTypeInfo ags_buffer_audio_signal_info = {
      sizeof (AgsBufferAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_buffer_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsBufferAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_buffer_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_buffer_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_buffer_audio_signal_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_buffer_audio_signal = g_type_register_static(AGS_TYPE_RECALL_AUDIO_SIGNAL,
							  "AgsBufferAudioSignal\0",
							  &ags_buffer_audio_signal_info,
							  0);

    g_type_add_interface_static(ags_type_buffer_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_buffer_audio_signal,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return (ags_type_buffer_audio_signal);
}

void
ags_buffer_audio_signal_class_init(AgsBufferAudioSignalClass *buffer_audio_signal)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_buffer_audio_signal_parent_class = g_type_class_peek_parent(buffer_audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) buffer_audio_signal;

  gobject->finalize = ags_buffer_audio_signal_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) buffer_audio_signal;

  recall->run_init_pre = ags_buffer_audio_signal_run_init_pre;
  recall->run_pre = ags_buffer_audio_signal_run_pre;
  recall->run_inter = ags_buffer_audio_signal_run_inter;
  recall->duplicate = ags_buffer_audio_signal_duplicate;
}

void
ags_buffer_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_buffer_audio_signal_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_buffer_audio_signal_connect;
  connectable->disconnect = ags_buffer_audio_signal_disconnect;
}

void
ags_buffer_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_buffer_audio_signal_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_buffer_audio_signal_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_buffer_audio_signal_disconnect_dynamic;
}

void
ags_buffer_audio_signal_init(AgsBufferAudioSignal *buffer_audio_signal)
{
  AGS_RECALL(buffer_audio_signal)->child_type = G_TYPE_NONE;
}

void
ags_buffer_audio_signal_finalize(GObject *gobject)
{
  /* call parent */
  G_OBJECT_CLASS(ags_buffer_audio_signal_parent_class)->finalize(gobject); 
}

void
ags_buffer_audio_signal_connect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_buffer_audio_signal_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_buffer_audio_signal_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_buffer_audio_signal_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_buffer_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_buffer_audio_signal_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_buffer_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_buffer_audio_signal_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_buffer_audio_signal_run_init_pre(AgsRecall *recall)
{
  GObject *soundcard;
  AgsRecycling *recycling;
  AgsAudioSignal *destination;
  AgsRecallID *parent_recall_id;
  AgsRecyclingContext *recycling_context;
  
  AgsBufferRecycling *buffer_recycling;
  AgsBufferAudioSignal *buffer_audio_signal;

  AgsMutexManager *mutex_manager;
  
  GList *stream;

  gdouble delay;
  guint attack;
  guint length;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *recycling_mutex;
  
  buffer_audio_signal = AGS_BUFFER_AUDIO_SIGNAL(recall);
  buffer_recycling = AGS_BUFFER_RECYCLING(recall->parent);

  soundcard = AGS_RECALL(buffer_audio_signal)->soundcard;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* recycling */
  recall->flags &= (~AGS_RECALL_PERSISTENT);
  recycling = AGS_RECALL_RECYCLING(buffer_recycling)->destination;

  pthread_mutex_lock(application_mutex);

  recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) recycling);
	
  pthread_mutex_unlock(application_mutex);

  /* recycling context */
  recycling_context = recall->recall_id->recycling_context;

  parent_recall_id = ags_recall_id_find_recycling_context(AGS_RECALL_CHANNEL_RUN(recall->parent->parent)->destination->recall_id,
							  recycling_context->parent);

  //TODO:JK: unclear
  attack = 0;
  delay = 0.0;

  /* create new audio signal */
  destination = ags_audio_signal_new((GObject *) soundcard,
				     (GObject *) recycling,
				     (GObject *) parent_recall_id);
  
  g_object_set(buffer_audio_signal,
	       "destination\0", destination,
	       NULL);  
  ags_recycling_create_audio_signal_with_defaults(recycling,
						  destination,
						  delay, attack);
  length = 3; // (guint) (2.0 * soundcard->delay[soundcard->tic_counter]) + 1;
  ags_audio_signal_stream_resize(destination,
				 length);

  ags_connectable_connect(AGS_CONNECTABLE(destination));
  
  destination->stream_current = destination->stream_beginning;

  pthread_mutex_lock(recycling_mutex);

  ags_recycling_add_audio_signal(recycling,
				 destination);

  pthread_mutex_unlock(recycling_mutex);

#ifdef AGS_DEBUG
  g_message("buffer %x to %x\0", destination, parent_recall_id);
  g_message("creating destination\0");
#endif
  
  /* call parent */
  AGS_RECALL_CLASS(ags_buffer_audio_signal_parent_class)->run_init_pre(recall);
}

void
ags_buffer_audio_signal_run_pre(AgsRecall *recall)
{
  /* call parent */
  AGS_RECALL_CLASS(ags_buffer_audio_signal_parent_class)->run_pre(recall);
}

void
ags_buffer_audio_signal_run_inter(AgsRecall *recall)
{
  AgsRecycling *recycling;
  AgsAudioSignal *source, *destination;
  AgsBufferChannel *buffer_channel;
  AgsBufferRecycling *buffer_recycling;
  AgsBufferAudioSignal *buffer_audio_signal;
  
  GList *stream_source, *stream_destination;

  gboolean muted;
  guint buffer_size;
  guint copy_mode;
  guint attack;
  
  GValue value = {0,};

  /* call parent */
  AGS_RECALL_CLASS(ags_buffer_audio_signal_parent_class)->run_inter(recall);

  /* initialize some variables */
  buffer_audio_signal = AGS_BUFFER_AUDIO_SIGNAL(recall);
  buffer_recycling = AGS_BUFFER_RECYCLING(recall->parent);
  buffer_channel = AGS_BUFFER_CHANNEL(AGS_RECALL_CHANNEL_RUN(AGS_RECALL(buffer_recycling)->parent)->recall_channel);

  source = AGS_RECALL_AUDIO_SIGNAL(buffer_audio_signal)->source;
  stream_source = source->stream_current;

  if(stream_source == NULL){
    //    if((AGS_RECALL_PERSISTENT & (recall->flags)) == 0){
      ags_recall_done(recall);
      //    }

    return;
  }

  //FIXME:JK: attack probably needs to be removed
  destination = AGS_RECALL_AUDIO_SIGNAL(buffer_audio_signal)->destination;

  g_value_init(&value, G_TYPE_FLOAT);
  ags_port_safe_read(buffer_channel->muted,
		     &value);

  if(g_value_get_float(&value) == 0.0){
    muted = FALSE;
  }else{
    muted = TRUE;
  }
  
  g_value_unset(&value);

  if(muted){
    return;
  }

  stream_destination = destination->stream_current;
  buffer_size = source->buffer_size;
  
  if(stream_destination != NULL){
    void *buffer_source;

    gboolean resample;
    
    copy_mode = ags_audio_buffer_util_get_copy_mode(ags_audio_buffer_util_format_from_soundcard(destination->format),
						    ags_audio_buffer_util_format_from_soundcard(source->format));
    resample = FALSE;
    
    if(stream_destination->next == NULL){
      ags_audio_signal_add_stream(destination);
    }

    /* check if resample */
    buffer_source = stream_source->data;
    attack = (destination->samplerate / source->samplerate) * source->attack;
    
    if(source->samplerate != destination->samplerate){
      buffer_source = ags_audio_buffer_util_resample(buffer_source, 1,
						     ags_audio_buffer_util_format_from_soundcard(source->format), source->samplerate,
						     buffer_size,
						     destination->samplerate);
      
      resample = TRUE;
    }
    
    /* copy */
    if((AGS_RECALL_INITIAL_RUN & (AGS_RECALL_AUDIO_SIGNAL(recall)->flags)) != 0){
      AGS_RECALL_AUDIO_SIGNAL(recall)->flags &= (~AGS_RECALL_INITIAL_RUN);
      ags_audio_buffer_util_copy_buffer_to_buffer(stream_destination->data, 1, attack,
						  buffer_source, 1, 0,
						  destination->buffer_size - attack, copy_mode);
    }else{
      if(attack != 0 && stream_source->prev != NULL){
	void *buffer_source_prev;
	
	buffer_source_prev = stream_source->prev->data;

	if(resample){
	  buffer_source_prev = ags_audio_buffer_util_resample(buffer_source_prev, 1,
							      ags_audio_buffer_util_format_from_soundcard(source->format), source->samplerate,
							      buffer_size,
							      destination->samplerate);

	}
	
	ags_audio_buffer_util_copy_buffer_to_buffer(stream_destination->data, 1, 0,
						    buffer_source_prev, 1, destination->buffer_size - attack,
						    attack, copy_mode);

	if(resample){
	  free(buffer_source_prev);
	}
      }

      ags_audio_buffer_util_copy_buffer_to_buffer(stream_destination->data, 1, attack,
						  buffer_source, 1, 0,
						  destination->buffer_size - attack, copy_mode);
    }

    if(resample){
      free(buffer_source);
    }
  }
}

AgsRecall*
ags_buffer_audio_signal_duplicate(AgsRecall *recall,
				  AgsRecallID *recall_id,
				  guint *n_params, GParameter *parameter)
{
  AgsBufferAudioSignal *buffer;

  buffer = (AgsBufferAudioSignal *) AGS_RECALL_CLASS(ags_buffer_audio_signal_parent_class)->duplicate(recall,
												      recall_id,
												      n_params, parameter);
  
  return((AgsRecall *) buffer);
}

/**
 * ags_buffer_audio_signal_new:
 * @audio_signal: an #AgsAudioSignal
 *
 * Creates an #AgsBufferAudioSignal
 *
 * Returns: a new #AgsBufferAudioSignal
 *
 * Since: 0.4
 */
AgsBufferAudioSignal*
ags_buffer_audio_signal_new(AgsAudioSignal *audio_signal)
{
  AgsBufferAudioSignal *buffer_audio_signal;

  buffer_audio_signal = (AgsBufferAudioSignal *) g_object_new(AGS_TYPE_BUFFER_AUDIO_SIGNAL,
							      "source\0", audio_signal,
							      NULL);

  return(buffer_audio_signal);
}
