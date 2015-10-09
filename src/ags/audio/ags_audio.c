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

#include <ags/audio/ags_audio.h>

#include <ags/main.h>

#include <ags/lib/ags_list.h>

#include <ags-lib/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_marshal.h>

#include <ags/thread/ags_mutex_manager.h>
#include <ags/thread/ags_audio_loop.h>
#include <ags/thread/ags_audio_thread.h>

#include <ags/audio/ags_config.h>
#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_id.h>

#include <ags/audio/file/ags_audio_file.h>

#include <ags/audio/task/ags_audio_set_recycling.h>

#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>

/**
 * SECTION:ags_audio
 * @short_description: A container of channels organizing them as input or output
 * @title: AgsAudio
 * @section_id:
 * @include: ags/audio/ags_audio.h
 *
 * #AgsAudio organizes #AgsChannel objects either as input or output and
 * is responsible of their alignment. The class can contain #AgsRecall objects
 * in order to perform computation on all channels or in audio context.
 * Therefor exists #AgsRecyclingContainer acting as tree context.
 *
 * At least one #AgsRecallID is assigned to it and has one more if
 * %AGS_AUDIO_OUTPUT_HAS_RECYCLING is set as flag.
 *
 * If %AGS_AUDIO_HAS_NOTATION is set as flag one #AgsNotation is allocated per audio
 * channel.
 */

void ags_audio_class_init(AgsAudioClass *audio_class);
void ags_audio_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_audio_init(AgsAudio *audio);
void ags_audio_set_property(GObject *gobject,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *param_spec);
void ags_audio_get_property(GObject *gobject,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *param_spec);
void ags_audio_add_to_registry(AgsConnectable *connectable);
void ags_audio_remove_from_registry(AgsConnectable *connectable);
void ags_audio_connect(AgsConnectable *connectable);
void ags_audio_disconnect(AgsConnectable *connectable);
void ags_audio_finalize(GObject *gobject);

void ags_audio_real_set_audio_channels(AgsAudio *audio,
				       guint audio_channels, guint audio_channels_old);
void ags_audio_real_set_pads(AgsAudio *audio,
			     GType type,
			     guint channels, guint channels_old);

enum{
  SET_AUDIO_CHANNELS,
  SET_PADS,
  INIT_RUN,
  TACT,
  DONE,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_DEVOUT,
};

static gpointer ags_audio_parent_class = NULL;
static guint audio_signals[LAST_SIGNAL];

extern pthread_mutex_t ags_application_mutex;
extern AgsConfig *config;

GType
ags_audio_get_type (void)
{
  static GType ags_type_audio = 0;

  if(!ags_type_audio){
    static const GTypeInfo ags_audio_info = {
      sizeof(AgsAudioClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsAudio),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_audio_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_audio = g_type_register_static(G_TYPE_OBJECT,
					    "AgsAudio\0", &ags_audio_info,
					    0);

    g_type_add_interface_static(ags_type_audio,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_audio);
}

void
ags_audio_class_init(AgsAudioClass *audio)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_audio_parent_class = g_type_class_peek_parent(audio);

  /* GObjectClass */
  gobject = (GObjectClass *) audio;

  gobject->set_property = ags_audio_set_property;
  gobject->get_property = ags_audio_get_property;

  gobject->finalize = ags_audio_finalize;

  /* properties */
  /**
   * AgsAudio:devout:
   *
   * The assigned #AgsDevout acting as default sink.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("devout\0",
				   "assigned devout\0",
				   "The devout it is assigned with\0",
				   AGS_TYPE_DEVOUT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DEVOUT,
				  param_spec);

  /* AgsAudioClass */
  audio->set_audio_channels = ags_audio_real_set_audio_channels;
  audio->set_pads = ags_audio_real_set_pads;

  audio->init_run = NULL;
  audio->tact = NULL;
  audio->done = NULL;

  /* signals */
  /**
   * AgsAudio::set-audio-channels:
   * @audio: the object to adjust the channels.
   * @audio_channels_new: new audio channel count
   * @audio_channels_old: old audio channel count
   *
   * The ::set-audio-channels signal notifies about changes in channel
   * alignment.
   */
  audio_signals[SET_AUDIO_CHANNELS] = 
    g_signal_new("set-audio-channels\0",
		 G_TYPE_FROM_CLASS(audio),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsAudioClass, set_audio_channels),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__UINT_UINT,
		 G_TYPE_NONE, 2,
		 G_TYPE_UINT, G_TYPE_UINT);

  /**
   * AgsAudio::set-pads:
   * @audio: the object to adjust pads.
   * @type: either #AGS_TYPE_INPUT or #AGS_TYPE_OUTPUT
   * @pads_new: new pad count
   * @pads_old: old pad count
   *
   * The ::set-pads signal notifies about changes in channel
   * alignment.
   */
  audio_signals[SET_PADS] = 
    g_signal_new("set-pads\0",
		 G_TYPE_FROM_CLASS(audio),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsAudioClass, set_pads),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__ULONG_UINT_UINT,
		 G_TYPE_NONE, 3,
		 G_TYPE_ULONG,
		 G_TYPE_UINT, G_TYPE_UINT);

  /**
   * AgsAudio::init-run:
   * @audio: the object to init run.
   *
   * The ::init-run signal is invoked during dynamic initialization of recalls
   * of @audio.
   *
   * Returns: the current #AgsRecallID
   */
  audio_signals[INIT_RUN] = 
    g_signal_new("init-run\0",
		 G_TYPE_FROM_CLASS(audio),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsAudioClass, init_run),
		 NULL, NULL,
		 g_cclosure_user_marshal_OBJECT__VOID,
		 G_TYPE_OBJECT, 0);

  /**
   * AgsAudio::tact:
   * @audio: the object playing.
   * @recall_id: the appropriate #AgsRecallID
   *
   * The ::tact signal is invoked during playback of recalls
   * of @audio.
   */
  audio_signals[TACT] = 
    g_signal_new("tact\0",
		 G_TYPE_FROM_CLASS(audio),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsAudioClass, tact),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1,
		 G_TYPE_OBJECT);

  /**
   * AgsAudio::done:
   * @audio: the object done playing.
   * @recall_id: the appropriate #AgsRecallID
   *
   * The ::done signal is invoked while terminating playback
   * of @audio.
   */
  audio_signals[DONE] = 
    g_signal_new("done\0",
		 G_TYPE_FROM_CLASS(audio),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsAudioClass, done),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1,
		 G_TYPE_OBJECT);
}

void
ags_audio_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->add_to_registry = ags_audio_add_to_registry;
  connectable->remove_from_registry = ags_audio_remove_from_registry;

  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_audio_connect;
  connectable->disconnect = ags_audio_disconnect;
}

void
ags_audio_init(AgsAudio *audio)
{
  AgsMutexManager *mutex_manager;

  gchar *str0, *str1;
  
  pthread_mutex_t *mutex;
  pthread_mutexattr_t attr;

  /* insert audio mutex */
  //FIXME:JK: memory leak
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr,
			    PTHREAD_MUTEX_RECURSIVE);

  mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex,
		     &attr);

  pthread_mutex_lock(&(ags_application_mutex));

  mutex_manager = ags_mutex_manager_get_instance();

  ags_mutex_manager_insert(mutex_manager,
			   (GObject *) audio,
			   mutex);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* base init */
  audio->flags = 0;

  audio->devout = NULL;

  audio->sequence_length = 0;
  audio->audio_channels = 0;
  audio->frequence = 0;

  audio->output_pads = 0;
  audio->output_lines = 0;
  audio->input_pads = 0;
  audio->input_lines = 0;

  audio->output = NULL;
  audio->input = NULL;

  audio->devout_play_domain = ags_devout_play_domain_alloc();
  AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->domain = (GObject *) audio;

  /**/
  str0 = ags_config_get(config,
			AGS_CONFIG_THREAD,
			"model\0");
  str1 = ags_config_get(config,
			AGS_CONFIG_THREAD,
			"super-threaded-scope\0");
  
  if(!g_ascii_strncasecmp(str0,
			  "super-threaded\0",
			  15)){
    if(!g_ascii_strncasecmp(str1,
			    "audio\0",
			    6) ||
       !g_ascii_strncasecmp(ags_config_get(config,
					   AGS_CONFIG_THREAD,
					   "super-threaded-scope\0"),
			    "channel\0",
			    8) ||
       !g_ascii_strncasecmp(ags_config_get(config,
					   AGS_CONFIG_THREAD,
					   "super-threaded-scope\0"),
			    "recycling\0",
			    10)){
      g_atomic_int_or(&(AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->flags),
		      AGS_DEVOUT_PLAY_DOMAIN_SUPER_THREADED_AUDIO);

      AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->audio_thread[0] = ags_audio_thread_new(NULL,
												audio);
      AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->audio_thread[1] = ags_audio_thread_new(NULL,
												audio);
      AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->audio_thread[2] = ags_audio_thread_new(NULL,
												audio);
    }
  }

  free(str0);
  free(str1);
  
  audio->notation = NULL;

  audio->recall_id = NULL;
  audio->recycling_container = NULL;

  audio->container = NULL;
  audio->recall = NULL;
  audio->play = NULL;

  audio->recall_remove= NULL;
  audio->play_remove = NULL;

  audio->machine = NULL;
}

void
ags_audio_set_property(GObject *gobject,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *param_spec)
{
  AgsAudio *audio;

  audio = AGS_AUDIO(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    {
      AgsDevout *devout;

      devout = (AgsDevout *) g_value_get_object(value);

      ags_audio_set_devout(audio, (GObject *) devout);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_get_property(GObject *gobject,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *param_spec)
{
  AgsAudio *audio;

  audio = AGS_AUDIO(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    g_value_set_object(value, audio->devout);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_finalize(GObject *gobject)
{
  AgsAudio *audio;
  AgsChannel *channel;

  AgsMutexManager *mutex_manager;

  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  ags_mutex_manager_remove(mutex_manager,
			   gobject);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  audio = AGS_AUDIO(gobject);

  if(audio->devout != NULL){
    g_object_unref(audio->devout);
  }
  
  /* output */
  channel = audio->output;

  if(channel != NULL){
    while(channel->next != NULL){
      channel = channel->next;
      g_object_unref((GObject *) channel->prev);
    }

    g_object_unref((GObject *) channel);
  }

  /* input */
  channel = audio->input;

  if(channel != NULL){
    while(channel->next != NULL){
      channel = channel->next;
      g_object_unref((GObject *) channel->prev);
    }

    g_object_unref(channel);
  }

  /* free some lists */
  ags_list_free_and_unref_link(audio->notation);

  ags_list_free_and_unref_link(audio->recall_id);

  ags_list_free_and_unref_link(audio->recall);
  ags_list_free_and_unref_link(audio->play);

  ags_list_free_and_unref_link(audio->recall_remove);
  ags_list_free_and_unref_link(audio->play_remove);

  /* call parent */
  G_OBJECT_CLASS(ags_audio_parent_class)->finalize(gobject);
}

void
ags_audio_add_to_registry(AgsConnectable *connectable)
{
  AgsMain *ags_main;
  AgsServer *server;
  AgsAudio *audio;
  AgsChannel *channel;
  AgsRegistryEntry *entry;
  GList *list;
  
  audio = AGS_AUDIO(connectable);

  ags_main = AGS_MAIN(AGS_DEVOUT(audio->devout)->ags_main);

  server = ags_main->server;

  entry = ags_registry_entry_alloc(server->registry);
  g_value_set_object(&(entry->entry),
		     (gpointer) audio);
  ags_registry_add(server->registry,
		   entry);

  /* add play */
  list = audio->play;

  while(list != NULL){
    ags_connectable_add_to_registry(AGS_CONNECTABLE(list->data));

    list = list->next;
  }
  
  /* add recall */
  list = audio->recall;

  while(list != NULL){
    ags_connectable_add_to_registry(AGS_CONNECTABLE(list->data));

    list = list->next;
  }

  /* add output */
  channel = audio->output;

  while(channel != NULL){
    ags_connectable_add_to_registry(AGS_CONNECTABLE(channel));

    channel = channel->next;
  }

  /* add input */
  channel = audio->input;

  while(channel != NULL){
    ags_connectable_add_to_registry(AGS_CONNECTABLE(channel));

    channel = channel->next;
  }
}

void
ags_audio_remove_from_registry(AgsConnectable *connectable)
{
  //TODO:JK: implement me
}

void
ags_audio_connect(AgsConnectable *connectable)
{
  AgsAudio *audio;
  AgsChannel *channel;

  GList *list;

  audio = AGS_AUDIO(connectable);

#ifdef AGS_DEBUG
  g_message("connecting audio\0");
#endif

  /* connect channels */
  channel = audio->output;

  while(channel != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(channel));

    channel = channel->next;
  }

  channel = audio->input;

  while(channel != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(channel));

    channel = channel->next;
  }

  /* connect recall ids */
  list = audio->recall_id;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }

  /* connect recall containers */
  list = audio->container;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }

  /* connect recalls */
  list = audio->recall;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }

  list = audio->play;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }

  /* connect remove recalls */
  list = audio->recall_remove;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }

  list = audio->play_remove;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }

  /* connect notation */
  if(audio->notation != NULL)
    ags_connectable_connect(AGS_CONNECTABLE(audio->notation));
}

void
ags_audio_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

/**
 * ags_audio_set_devout:
 * @audio: the #AgsAudio
 * @devout: an #AgsDevout
 *
 * Sets a devout object on audio.
 *
 * Since: 0.4
 */
void
ags_audio_set_devout(AgsAudio *audio, GObject *devout)
{
  AgsChannel *channel;
  GList *list;
  AgsMutexManager *mutex_manager;
  gchar *str0, *str1;
  
  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  /**/
  str0 = ags_config_get(config,
			AGS_CONFIG_THREAD,
			"model\0");
  str1 = ags_config_get(config,
			AGS_CONFIG_THREAD,
			"super-threaded-scope\0");
  
  if(!g_ascii_strncasecmp(str0,
			  "super-threaded\0",
			  15)){
    /* super threaed setup */
    if(!g_ascii_strncasecmp(str1,
			    "audio\0",
			    6) ||
       !g_ascii_strncasecmp(ags_config_get(config,
					   AGS_CONFIG_THREAD,
					   "super-threaded-scope\0"),
			    "channel\0",
			    8) ||
       !g_ascii_strncasecmp(ags_config_get(config,
					   AGS_CONFIG_THREAD,
					   "super-threaded-scope\0"),
			    "recycling\0",
			    10)){
      
      /* sequencer */
      g_object_set(AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->audio_thread[1],
		   "devout\0", devout,
		   NULL);
  
      /* notation */
      g_object_set(AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->audio_thread[2],
		   "devout\0", devout,
		   NULL);
    }
  }

  free(str0);
  free(str1);
  
  /* audio */
  if(audio->devout == devout){
    pthread_mutex_unlock(mutex);
    return;
  }

  if(audio->devout != NULL)
    g_object_unref(audio->devout);

  if(devout != NULL)
    g_object_ref(devout);

  audio->devout = (GObject *) devout;

  /* recall */
  list = audio->play;
  
  while(list != NULL){
    g_object_set(G_OBJECT(list->data),
		 "devout\0", devout,
		 NULL);
    
    list = list->next;
  }
  
  list = audio->recall;
  
  while(list != NULL){
    g_object_set(G_OBJECT(list->data),
		 "devout\0", devout,
		 NULL);
    
    list = list->next;
  }
  
  /* input */
  channel = audio->input;

  while(channel != NULL){
    g_object_set(G_OBJECT(channel),
		 "devout\0", devout,
		 NULL);
    
    channel = channel->next;
  }

  /* output */
  channel = audio->output;

  while(channel != NULL){
    g_object_set(G_OBJECT(channel),
		 "devout\0", devout,
		 NULL);
    
    channel = channel->next;
  }
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_set_flags:
 * @audio: an AgsAudio
 * @flags: see enum AgsAudioFlags
 *
 * Enable a feature of AgsAudio.
 *
 * Since: 0.4
 */
void
ags_audio_set_flags(AgsAudio *audio, guint flags)
{
  auto GParameter* ags_audio_set_flags_set_recycling_parameter(GType type);
  auto void ags_audio_set_flags_add_recycling_task(GParameter *parameter);

  GParameter* ags_audio_set_flags_set_recycling_parameter(GType type){
    AgsChannel *channel, *start_channel, *end_channel;
    AgsRecycling *recycling, *recycling_next, *start_recycling, *end_recycling;
    GParameter *parameter;
    int i;

    parameter = g_new(GParameter, 4 * audio->audio_channels);

    for(i = 0; i < audio->audio_channels; i++){
      start_channel =
	channel = ags_channel_nth(((g_type_is_a(type, AGS_TYPE_INPUT)) ? audio->input: audio->output), i);
      end_channel = NULL;

      start_recycling =
	recycling = NULL;
      end_recycling = NULL;
	  
      if(channel != NULL){
	start_recycling = 
	  recycling = ags_recycling_new(audio->devout);
	goto ags_audio_set_flags_OUTPUT_RECYCLING;
      }

      while(channel != NULL){
	recycling->next = ags_recycling_new(audio->devout);
      ags_audio_set_flags_OUTPUT_RECYCLING:
	recycling->next->prev = recycling;
	recycling = recycling->next;
	    	    
	channel = channel->next_pad;
      }

      end_channel = ags_channel_pad_last(start_channel);
      end_recycling = recycling;

      /* setting up parameters */
      parameter[i].name = "start_channel";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), start_channel);

      parameter[i].name = "end_channel";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), end_channel);

      parameter[i].name = "start_recycling";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), start_recycling);

      parameter[i].name = "end_recycling";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), end_recycling);
    }

    return(parameter);
  }
  void ags_audio_set_flags_add_recycling_task(GParameter *parameter){
    AgsAudioSetRecycling *audio_set_recycling;

    /* create set recycling task */
    audio_set_recycling = ags_audio_set_recycling_new(audio,
						      parameter);

    /* append AgsAudioSetRecycling */
    //    ags_task_thread_append_task(AGS_TASK_THREAD(AGS_AUDIO_LOOP(AGS_MAIN(AGS_DEVOUT(audio->devout)->ags_main)->main_loop)->task_thread),
    //				AGS_TASK(audio_set_recycling));
  }

  if(audio == NULL || !AGS_IS_AUDIO(audio)){
    return;
  }

  if((AGS_AUDIO_INPUT_HAS_RECYCLING & (audio->flags))){
    GParameter *parameter;
        
    /* check if output has already recyclings */
    if((AGS_AUDIO_OUTPUT_HAS_RECYCLING & (audio->flags)) == 0){
      if(audio->output_pads > 0){
	parameter = ags_audio_set_flags_set_recycling_parameter(AGS_TYPE_OUTPUT);
	ags_audio_set_flags_add_recycling_task(parameter);
      }
      
      audio->flags |= AGS_AUDIO_OUTPUT_HAS_RECYCLING;
    }

    /* check if input has already recyclings */
    if((AGS_AUDIO_INPUT_HAS_RECYCLING & (audio->flags)) != 0){
      if(audio->input_pads > 0){
	parameter = ags_audio_set_flags_set_recycling_parameter(AGS_TYPE_INPUT);
	ags_audio_set_flags_add_recycling_task(parameter);
      }

      audio->flags |= AGS_AUDIO_INPUT_HAS_RECYCLING;
    }
  }

  //TODO:JK: automatization of setting recycling_container root
}
    

/**
 * ags_audio_unset_flags:
 * @audio: an AgsAudio
 * @flags: see enum AgsAudioFlags
 *
 * Disable a feature of AgsAudio.
 *
 * Since: 0.4
 */
void
ags_audio_unset_flags(AgsAudio *audio, guint flags)
{
  auto GParameter* ags_audio_unset_flags_set_recycling_parameter(GType type);
  auto void ags_audio_unset_flags_add_recycling_task(GParameter *parameter);

  GParameter* ags_audio_unset_flags_set_recycling_parameter(GType type){
    AgsChannel *channel, *start_channel, *end_channel;
    AgsRecycling *recycling, *recycling_next, *start_recycling, *end_recycling;
    GParameter *parameter;
    int i;

    parameter = g_new(GParameter, 4 * audio->audio_channels);

    for(i = 0; i < audio->audio_channels; i++){
      start_channel = ags_channel_nth(((g_type_is_a(type, AGS_TYPE_INPUT)) ? audio->input: audio->output), i);
      end_channel = ags_channel_pad_last(start_channel);

      start_recycling = NULL;
      end_recycling = NULL;

      /* setting up parameters */
      parameter[i].name = "start_channel";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), start_channel);

      parameter[i].name = "end_channel";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), end_channel);

      parameter[i].name = "start_recycling";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), start_recycling);

      parameter[i].name = "end_recycling";
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value), end_recycling);
    }

    return(parameter);
  }
  void ags_audio_unset_flags_add_recycling_task(GParameter *parameter){
    AgsAudioSetRecycling *audio_set_recycling;

    /* create set recycling task */
    audio_set_recycling = ags_audio_set_recycling_new(audio,
						      parameter);

    /* append AgsAudioSetRecycling */
    //    ags_task_thread_append_task(AGS_TASK_THREAD(AGS_AUDIO_LOOP(AGS_MAIN(AGS_DEVOUT(audio->devout)->ags_main)->main_loop)->task_thread),
    //				AGS_TASK(audio_set_recycling));
  }

  if(audio == NULL || !AGS_IS_AUDIO(audio)){
    return;
  }
  
  if((AGS_AUDIO_OUTPUT_HAS_RECYCLING & (audio->flags))){
    GParameter *parameter;
    
    /* check if input has already no recyclings */
    if((AGS_AUDIO_INPUT_HAS_RECYCLING & (audio->flags)) != 0){      
      if(audio->input_pads > 0){
	parameter = ags_audio_unset_flags_set_recycling_parameter(AGS_TYPE_INPUT);
	ags_audio_unset_flags_add_recycling_task(parameter);
	
	audio->flags &= (~AGS_AUDIO_INPUT_HAS_RECYCLING);
      }
      
      /* check if output has already recyclings */
      if(audio->output_pads > 0){
	parameter = ags_audio_unset_flags_set_recycling_parameter(AGS_TYPE_OUTPUT);
	ags_audio_unset_flags_add_recycling_task(parameter);
	
	audio->flags &= (~AGS_AUDIO_OUTPUT_HAS_RECYCLING);
      }
    }
  }
}

void
ags_audio_real_set_audio_channels(AgsAudio *audio,
				  guint audio_channels, guint audio_channels_old)
{
  AgsMutexManager *mutex_manager;
  
  gboolean alloc_recycling;
  gboolean link_recycling; // affects AgsInput
  gboolean set_sync_link, set_async_link; // affects AgsOutput

  pthread_mutex_t *prev_mutex, *prev_pad_mutex, *current_mutex;
  
  auto void ags_audio_set_audio_channels_init_parameters(GType type);
  auto void ags_audio_set_audio_channels_grow(GType type);
  auto void ags_audio_set_audio_channels_shrink_zero();
  auto void ags_audio_set_audio_channels_shrink();
  auto void ags_audio_set_audio_channels_grow_notation();
  auto void ags_audio_set_audio_channels_shrink_notation();
  
  void ags_audio_set_audio_channels_init_parameters(GType type){
    if(type == AGS_TYPE_OUTPUT){
      link_recycling = FALSE;

      if((AGS_AUDIO_OUTPUT_HAS_RECYCLING & (audio->flags)) != 0){
	alloc_recycling = TRUE;
      }else{
	alloc_recycling = FALSE;

	if((AGS_AUDIO_INPUT_HAS_RECYCLING & (audio->flags)) != 0){
	  if((AGS_AUDIO_SYNC & (audio->flags)) != 0 && (AGS_AUDIO_ASYNC & (audio->flags)) == 0){
	    set_sync_link = FALSE;
	    set_async_link = TRUE;
	  }else if((AGS_AUDIO_ASYNC & (audio->flags)) != 0){
	    set_async_link = TRUE;
	    set_sync_link = FALSE;
	  }else{
#ifdef AGS_DEBUG
	    g_message("ags_audio_set_audio_channels - warning: AGS_AUDIO_SYNC nor AGS_AUDIO_ASYNC weren't defined\0");
#endif
	    set_sync_link = FALSE;
	    set_async_link = FALSE;
	  }
	}
      }
    }else{
      set_sync_link = FALSE;
      set_async_link = FALSE;
      
      if((AGS_AUDIO_INPUT_HAS_RECYCLING & audio->flags) != 0){
	alloc_recycling = TRUE;
      }else{
	alloc_recycling = FALSE;
      }
      
      if((AGS_AUDIO_ASYNC & audio->flags) != 0 && alloc_recycling){
	link_recycling = TRUE;
      }else{
	link_recycling = FALSE;
      }
    }    
  }
  
  void ags_audio_set_audio_channels_grow(GType type){
    AgsChannel *channel, *start, *current;
    AgsRecycling *first_recycling, *last_recycling;
	  
    guint pads;
    guint i, j;

    if(type == AGS_TYPE_OUTPUT){
      /* AGS_TYPE_OUTPUT */
      pads = audio->output_pads;
      
      start = audio->output;
    }else{
      /* AGS_TYPE_INPUT */
      pads = audio->input_pads;

      start = audio->input;
    }

    /* grow */
    for(j = 0; j < pads; j++){
      for(i = audio_channels_old; i < audio_channels; i++){
	channel = (AgsChannel *) g_object_new(type,
					      "audio\0", (GObject *) audio,
					      "devout\0", audio->devout,
					      NULL);

	if(start == NULL){
	  start = channel;

	  if(g_type_is_a(type, AGS_TYPE_OUTPUT)){
	    audio->output = channel;
	  }else{
	    audio->input = channel;
	  }
	}
	
	if(j * audio_channels + i != 0){
	  /* set prev */
	  channel->prev = ags_channel_nth(start, j * audio_channels + i - 1);

	  pthread_mutex_lock(&(ags_application_mutex));
	  
	  prev_mutex = ags_mutex_manager_lookup(mutex_manager,
						(GObject *) channel->prev);
	  
	  pthread_mutex_unlock(&(ags_application_mutex));
	  
	  pthread_mutex_lock(prev_mutex);
	  
	  channel->prev->next = channel;

	  pthread_mutex_unlock(prev_mutex);
	}
	
	if(j != 0){
	  /* set prev pad */
	  channel->prev_pad = ags_channel_pad_nth(ags_channel_nth(start,
								  i),
						  j - 1);

	  pthread_mutex_lock(&(ags_application_mutex));
	  
	  prev_pad_mutex = ags_mutex_manager_lookup(mutex_manager,
						    (GObject *) channel->prev_pad);
	  
	  pthread_mutex_unlock(&(ags_application_mutex));

	  pthread_mutex_lock(prev_pad_mutex);
	  
	  channel->prev_pad->next_pad = channel;

	  pthread_mutex_unlock(prev_pad_mutex);
	}

	/* set indices */
	channel->pad = j;
	channel->audio_channel = i;
	channel->line = j * audio->audio_channels + i;

	/* reset nested AgsRecycling tree */
	if(alloc_recycling){
	  first_recycling =
	    last_recycling = ags_recycling_new(audio->devout);
	  
	  first_recycling->channel = (GObject *) channel;
	  
	  ags_channel_set_recycling(channel,
				    first_recycling, last_recycling,
				    TRUE, TRUE);
	}else if(set_sync_link){
	  AgsChannel *input;

	  input = ags_channel_nth(audio->input,
				  channel->line);
	  
	  /* set sync link */
	  if(input != NULL){
	    first_recycling = input->first_recycling;
	    last_recycling = input->last_recycling;
	    
	    ags_channel_set_recycling(channel,
				      first_recycling, last_recycling,
				      TRUE, TRUE);
	  }
	}else if(set_async_link){
	  AgsChannel *input, *input_pad_last;

	  input = ags_channel_nth(audio->input,
				  i);

	  /* set async link */
	  if(input != NULL){
	    input_pad_last = ags_channel_pad_last(input);

	    first_recycling = input->first_recycling;
	    last_recycling = input_pad_last->last_recycling;
	    
	    ags_channel_set_recycling(channel,
				      first_recycling, last_recycling,
				      TRUE, TRUE);
	  }
	}
      }
    }
  }
  
  void ags_audio_set_audio_channels_shrink_zero(){
    AgsChannel *channel, *start, *channel_next;
    
    gboolean first_run;
    
    GError *error;

    start =
      channel = audio->output;
    first_run = TRUE;

    error = NULL;

  ags_audio_set_audio_channel_shrink_zero0:

    while(channel != NULL){
      ags_channel_set_link(channel, NULL, &error);
      ags_channel_set_recycling(channel,
				NULL, NULL,
				TRUE, TRUE);

      channel = channel->next;
    }

    channel = start;

    while(channel != NULL){
      channel_next = channel->next;

      g_object_unref((GObject *) channel);

      channel = channel_next;
    }

    if(first_run){
      start =
	channel = audio->input;
      first_run = FALSE;
      goto ags_audio_set_audio_channel_shrink_zero0;
    }

    audio->output = NULL;
    audio->input = NULL;
  }
  
  void ags_audio_set_audio_channels_shrink(){
    AgsChannel *channel, *start;
    AgsChannel *channel0, *channel1;
    AgsRecycling *recycling;

    guint pads, i, j;
    gboolean first_run;
    
    GError *error;

    start =
      channel = audio->output;
    pads = audio->output_pads;
    first_run = TRUE;

  ags_audio_set_audio_channel_shrink0:

    for(i = 0; i < pads; i++){
      channel = ags_channel_nth(channel, audio_channels);

      for(j = audio_channels; j < audio->audio_channels; j++){
	ags_channel_set_link(channel, NULL, &error);
	channel = channel->next;
      }
    }

    channel = start;

    if(i < pads){
      for(i = 0; ; i++){
	for(j = 0; j < audio_channels -1; j++){
	  channel->pad = i;
	  channel->audio_channel = j;
	  channel->line = i * audio_channels + j;

	  channel = channel->next;
	}

	channel->pad = i;
	channel->audio_channel = j;
	channel->line = i * audio_channels + j;

	channel0 = channel->next;
	
	for(; j < audio->audio_channels; j++){
	  channel1 = channel0->next;
      
	  g_object_unref((GObject *) channel0);

	  channel0 = channel1;
	}

	channel->next = channel1;

	if(channel1 != NULL){
	  channel1->prev = channel;
	}else{
	  break;
	}
	
	channel = channel1;
      }
    }

    if(first_run){
      first_run = FALSE;
      start =
	channel = audio->input;
      pads = audio->input_pads;

      goto ags_audio_set_audio_channel_shrink0;
    }
  }
  
  void ags_audio_set_audio_channels_grow_notation(){
    GList *list;
    guint i;

    i = audio->audio_channels;

#ifdef AGS_DEBUG
    g_message("ags_audio_set_audio_channels_grow_notation\n\0");
#endif

    if(audio->audio_channels == 0){
      audio->notation =
	list = g_list_alloc();
      goto ags_audio_set_audio_channels_grow_notation0;
    }else{
      list = g_list_nth(audio->notation, audio->audio_channels - 1);
    }

    for(; i < audio_channels; i++){
      list->next = g_list_alloc();
      list->next->prev = list;
      list = list->next;

    ags_audio_set_audio_channels_grow_notation0:
      list->data = (gpointer) ags_notation_new(i);
    } 
  }
  
  void ags_audio_set_audio_channels_shrink_notation(){
    GList *list, *list_next;

    list = g_list_nth(audio->notation, audio_channels);

    if(audio_channels == 0){
      audio->notation = NULL;
    }else{
      list->prev->next = NULL;
    }

    while(list != NULL){
      list_next = list->next;

      g_object_unref((GObject *) list->data);
      g_list_free1(list);

      list = list_next;
    }
  }
  
  /* entry point */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  pthread_mutex_unlock(&(ags_application_mutex));
  
  /* grow / shrink */
  if(audio_channels > audio_channels_old){
    AgsDevoutPlayDomain *devout_play_domain;
    AgsChannel *current;

    guint i;
    
    /* grow audio channels */
    if((AGS_AUDIO_HAS_NOTATION & (audio->flags)) != 0){
      ags_audio_set_audio_channels_grow_notation();
    }

    if(audio->input_pads > 0){
      ags_audio_set_audio_channels_init_parameters(AGS_TYPE_INPUT);
      ags_audio_set_audio_channels_grow(AGS_TYPE_INPUT);
    }

    audio->input_lines = audio_channels * audio->input_pads;

    if(audio->output_pads > 0){
      ags_audio_set_audio_channels_init_parameters(AGS_TYPE_OUTPUT);
      ags_audio_set_audio_channels_grow(AGS_TYPE_OUTPUT);
    }

    audio->output_lines = audio_channels * audio->output_pads;

    /* grow devout play domain */
    devout_play_domain = AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain);
    current = ags_channel_nth(audio->output,
			      audio->audio_channels);

    if(current != NULL){
      for(i = 0; i < audio_channels - audio_channels_old; i++){
	devout_play_domain->devout_play = g_list_append(devout_play_domain->devout_play,
							current->devout_play);
	
	current = current->next;
      }
    }
  }else if(audio_channels < audio_channels_old){
    AgsDevoutPlayDomain *devout_play_domain;
    AgsDevoutPlay *devout_play;
    AgsChannel *current;

    GList *list;
    
    guint i, j;
    
    /* shrink audio channels */
    if((AGS_AUDIO_HAS_NOTATION & audio->flags) != 0)
      ags_audio_set_audio_channels_shrink_notation();

    if(audio_channels == 0){
      ags_audio_set_audio_channels_shrink_zero();
    }else{
      ags_audio_set_audio_channels_shrink();
    }

    audio->input_lines = audio_channels * audio->input_pads;
    audio->output_lines = audio_channels * audio->output_pads;

    /* shrink devout play domain */
    devout_play_domain = AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain);
    list = devout_play_domain->devout_play;
    
    for(j = 0; j < audio->output_pads; j++){
      for(i = 0; i < audio->audio_channels - audio_channels; i++){
	devout_play = g_list_nth_prev(g_list_last(list),
				      (audio->output_pads - j - 1) * audio_channels)->data;
	devout_play_domain->devout_play = g_list_remove(devout_play_domain->devout_play,
							devout_play);
      }
    }
  }

  /* apply new sizes */
  audio->audio_channels = audio_channels;
  // input_lines must be set earlier because set_sync_link needs it
}

/**
 * ags_audio_set_audio_channels:
 * @audio: the #AgsAudio
 * @audio_channels: new audio channels
 *
 * Resize audio channels AgsInput will be allocated first.
 *
 * Since: 0.3
 */
void
ags_audio_set_audio_channels(AgsAudio *audio, guint audio_channels)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* lock */
  pthread_mutex_lock(mutex);

  if(!AGS_IS_AUDIO(audio)){
    pthread_mutex_unlock(mutex);
    return;
  }

  /* emit */
  g_object_ref((GObject *) audio);
  g_signal_emit(G_OBJECT(audio),
		audio_signals[SET_AUDIO_CHANNELS], 0,
		audio_channels, audio->audio_channels);
  g_object_unref((GObject *) audio);

  /* unlock */
  pthread_mutex_unlock(mutex);
}

/*
 * resize
 * AgsInput has to be allocated first
 */
void
ags_audio_real_set_pads(AgsAudio *audio,
			GType type,
			guint pads, guint pads_old)
{
  AgsChannel *channel;
  
  AgsMutexManager *mutex_manager;
  
  gboolean alloc_recycling, link_recycling, set_sync_link, set_async_link;

  pthread_mutex_t *prev_mutex, *prev_pad_mutex;

  auto void ags_audio_set_pads_init_parameters();
  auto void ags_audio_set_pads_grow();
  auto void ags_audio_set_pads_unlink_all(AgsChannel *channel);
  auto void ags_audio_set_pads_shrink_zero(AgsChannel *channel);
  auto void ags_audio_set_pads_shrink(AgsChannel *channel);
  auto void ags_audio_set_pads_alloc_notation();
  auto void ags_audio_set_pads_free_notation();
  auto void ags_audio_set_pads_add_notes();
  auto void ags_audio_set_pads_remove_notes();

  void ags_audio_set_pads_init_parameters(){
    alloc_recycling = FALSE;
    link_recycling = FALSE;
    set_sync_link = FALSE;
    set_async_link = FALSE;
    
    if(type == AGS_TYPE_OUTPUT){
      if((AGS_AUDIO_OUTPUT_HAS_RECYCLING & (audio->flags)) != 0){
	alloc_recycling = TRUE;
      }else{
	if((AGS_AUDIO_INPUT_HAS_RECYCLING & (audio->flags)) != 0){
	  if((AGS_AUDIO_SYNC & (audio->flags)) != 0 && (AGS_AUDIO_ASYNC & (audio->flags)) == 0){
	    set_async_link = TRUE;
	  }else if((AGS_AUDIO_ASYNC & (audio->flags)) != 0){
	    set_async_link = TRUE;
	  }else{
#ifdef AGS_DEBUG
	    g_message("ags_audio_set_pads - warning: AGS_AUDIO_SYNC nor AGS_AUDIO_ASYNC weren't defined\0");
#endif
	  }
	}
      }
    }else{      
      if((AGS_AUDIO_INPUT_HAS_RECYCLING & audio->flags) != 0){
	alloc_recycling = TRUE;
      }
      
      if((AGS_AUDIO_ASYNC & audio->flags) != 0 && alloc_recycling){
	link_recycling = TRUE;
      }
    }    
  }

  void ags_audio_set_pads_grow(){
    AgsChannel *start, *channel;
    AgsRecycling *first_recycling, *last_recycling;

    guint i, j;
    
    if(g_type_is_a(type, AGS_TYPE_OUTPUT)){
      start = audio->output;
    }else{
      start = audio->input;
    }

    for(j = pads_old; j < pads; j++){
      for(i = 0; i < audio->audio_channels; i++){
	channel = (AgsChannel *) g_object_new(type,
					      "audio\0", (GObject *) audio,
					      "devout\0", audio->devout,
					      NULL);
	if(start == NULL){
	  /* set first channel in AgsAudio */
	  if(g_type_is_a(type, AGS_TYPE_OUTPUT)){
	    start = 
	      audio->output = channel;
	  }else{
	    start = 
	      audio->input = channel;
	  }
	}

	if(j * audio->audio_channels + i != 0){
	  /* set prev */
	  channel->prev = ags_channel_nth(start, j * audio->audio_channels + i - 1);

	  pthread_mutex_lock(&(ags_application_mutex));
	  
	  prev_mutex = ags_mutex_manager_lookup(mutex_manager,
						(GObject *) channel->prev);
	  
	  pthread_mutex_unlock(&(ags_application_mutex));
	  
	  pthread_mutex_lock(prev_mutex);
	  
	  channel->prev->next = channel;

	  pthread_mutex_unlock(prev_mutex);
	}
	
	if(j != 0){
	  /* set prev pad */
	  channel->prev_pad = ags_channel_pad_nth(ags_channel_nth(start,
								  i),
						  j - 1);

	  pthread_mutex_lock(&(ags_application_mutex));
	  
	  prev_pad_mutex = ags_mutex_manager_lookup(mutex_manager,
						    (GObject *) channel->prev_pad);
	  
	  pthread_mutex_unlock(&(ags_application_mutex));

	  pthread_mutex_lock(prev_pad_mutex);
	  
	  channel->prev_pad->next_pad = channel;
	  
	  pthread_mutex_unlock(prev_pad_mutex);
	}

	/* set indices */
	channel->pad = j;
	channel->audio_channel = i;
	channel->line = j * audio->audio_channels + i;

	/* reset nested AgsRecycling tree */
	if(alloc_recycling){
	  first_recycling =
	    last_recycling = ags_recycling_new(audio->devout);
	  
	  first_recycling->channel = (GObject *) channel;
	  
	  ags_channel_set_recycling(channel,
				    first_recycling, last_recycling,
				    TRUE, TRUE);
	}else if(set_sync_link){
	  AgsChannel *input;

	  input = ags_channel_nth(audio->input,
				  channel->line);
	  
	  /* set sync link */
	  if(input != NULL){
	    first_recycling = input->first_recycling;
	    last_recycling = input->last_recycling;
	    
	    ags_channel_set_recycling(channel,
				      first_recycling, last_recycling,
				      TRUE, TRUE);
	  }
	}else if(set_async_link){
	  AgsChannel *input, *input_pad_last;

	  input = ags_channel_nth(audio->input,
				  i);

	  /* set async link */
	  if(input != NULL){
	    input_pad_last = ags_channel_pad_last(input);

	    first_recycling = input->first_recycling;
	    last_recycling = input_pad_last->last_recycling;
	    
	    ags_channel_set_recycling(channel,
				      first_recycling, last_recycling,
				      TRUE, TRUE);
	  }
	}
      }
    }
  }

  void ags_audio_set_pads_unlink_all(AgsChannel *channel){
    GError *error;

    while(channel != NULL){
      error = NULL;
      ags_channel_set_link(channel, NULL, &error);

      if(error != NULL){
	g_error("%s\0", error->message);
      }

      channel = channel->next;
    }
  }
  
  void ags_audio_set_pads_shrink_zero(AgsChannel *channel){
    AgsChannel *channel_next;

    while(channel != NULL){
      channel_next = channel->next;

      g_object_unref((GObject *) channel);

      channel = channel_next;
    }
  }

  void ags_audio_set_pads_shrink(AgsChannel *channel){
    AgsChannel *current;

    guint i;

    if(channel != NULL){
      channel = channel->prev_pad;
    }
    
    current = ags_channel_pad_nth(channel,
				  pads);

    ags_audio_set_pads_shrink_zero(channel);

    /* remove pads */
    channel = current;

    if(current != NULL){
      for(i = 0; i < audio->audio_channels; i++){
	channel->next_pad = NULL;
	
	/* iterate */
	channel = channel->next;
      }

      /* remove channel */
      current = ags_channel_nth(current, audio->audio_channels - 1);
      current->next = NULL;
    }
  }

  void ags_audio_set_pads_alloc_notation(){
    GList *list;
    
    guint i;

#ifdef AGS_DEBUG
    g_message("ags_audio_set_pads_alloc_notation\n\0");
#endif

    if(audio->audio_channels > 0){
      audio->notation =
	list = g_list_alloc();
      i = 0;
      goto ags_audio_set_pads_alloc_notation0;
    }else{
      return;
    }

    for(; i < audio->audio_channels; i++){
      list->next = g_list_alloc();
      list->next->prev = list;
      list = list->next;
    ags_audio_set_pads_alloc_notation0:

      list->data = (gpointer) ags_notation_new(i);
    }
  }

  void ags_audio_set_pads_free_notation(){
    GList *list, *list_next;

    if(audio->audio_channels > 0){
      list = audio->notation;
      audio->notation = NULL;
    }else{
      return;
    }

    while(list != NULL){
      list_next = list->next;

      g_object_unref(G_OBJECT(list->data));
      g_list_free1(list);

      list = list_next;
    }
  }

  void ags_audio_set_pads_add_notes(){
    /* -- useless --
       GList *list;

       list = audio->notation;

       while(list != NULL){
       AGS_NOTATION(list->data)->pads = pads;

       list = list->next;
       }
    */
  }
  
  void ags_audio_set_pads_remove_notes(){
    AgsNotation *notation;
    GList *notation_i, *note, *note_next;

    notation_i = audio->notation;

    while(notation_i != NULL){
      notation = AGS_NOTATION(notation_i->data);
      note = notation->notes;

      while(note != NULL){
	note_next = note->next;

	if(AGS_NOTE(note->data)->y >= pads){
	  if(note->prev != NULL)
	    note->prev->next = note_next;
	  else
	    notation->notes = note_next;

	  if(note_next != NULL)
	    note_next->prev = note->prev;

	  free(note->data);
	  g_list_free1(note);
	}

	note = note_next;
      }

      notation_i = notation_i->next;
    }
  }

  /* entry point */
  ags_audio_set_pads_init_parameters();

  pthread_mutex_lock(&(ags_application_mutex));

  mutex_manager = ags_mutex_manager_get_instance();
	  
  pthread_mutex_unlock(&(ags_application_mutex));

  if(g_type_is_a(type, AGS_TYPE_OUTPUT)){
    /* output */
    pads_old = audio->output_pads;

    if(pads_old == pads){
      return;
    }

    if(audio->audio_channels == 0){
      audio->output_pads = pads;
      return;
    }

    /* grow or shrink */
    if(pads > audio->output_pads){
      AgsDevoutPlayDomain *devout_play_domain;
      AgsChannel *current;

      guint i, j;

      /* instantiate notation */
      if(pads_old == 0){
	if((AGS_AUDIO_NOTATION_DEFAULT & (audio->flags)) == 0){
	  ags_audio_set_pads_alloc_notation();
	}
      }

      /* grow channels */
      ags_audio_set_pads_grow();

      /* alloc devout play domain */
      devout_play_domain = AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain);
      current = audio->output;

      current = ags_channel_pad_nth(current,
				    pads_old);

      for(j = pads_old; j < pads; j++){
	for(i = 0; i < audio->audio_channels; i++){
	  devout_play_domain->devout_play = g_list_append(devout_play_domain->devout_play,
							  current->devout_play);
	
	  current = current->next;
	}
      }
    }else if(pads == 0){      
      if((AGS_AUDIO_HAS_NOTATION & (audio->flags)) != 0 &&
	 audio->notation != NULL){
	ags_audio_set_pads_free_notation();
      }

      channel = audio->output;
      
      /* unlink and remove */
      ags_audio_set_pads_unlink_all(channel);
      ags_audio_set_pads_shrink_zero(channel);
      
      audio->output = NULL;

      /* remove devout play domain */
      g_list_free(AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->devout_play);

      AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain)->devout_play = NULL;
    }else if(pads < audio->output_pads){
      AgsDevoutPlayDomain *devout_play_domain;

      guint i;
      
      ags_audio_set_pads_remove_notes();

      channel = ags_channel_pad_nth(channel,
				    pads);
      
      ags_audio_set_pads_unlink_all(channel);
      ags_audio_set_pads_shrink(channel);

      devout_play_domain = AGS_DEVOUT_PLAY_DOMAIN(audio->devout_play_domain);

      for(i = 0; i < audio->output_pads - pads; i++){
	AgsDevoutPlay *devout_play;

	devout_play = g_list_last(devout_play_domain->devout_play);
	devout_play_domain->devout_play = g_list_remove(devout_play_domain->devout_play,
							devout_play);
      }

    }

    /* apply new size */
    audio->output_pads = pads;
    audio->output_lines = pads * audio->audio_channels;

    if((AGS_AUDIO_SYNC & audio->flags) != 0 && (AGS_AUDIO_ASYNC & audio->flags) == 0){
      //TODO:JK: fix me
      //      audio->input_pads = pads;
      //      audio->input_lines = pads * audio->audio_channels;
    }
  }else if(g_type_is_a(type, AGS_TYPE_INPUT)){
    /* input */
    if(pads_old == pads){
      return;
    }

    if(audio->audio_channels == 0){
      audio->input_pads = pads;
      return;
    }

    /* grow or shrink */
    if(pads > pads_old){	 
      /* instantiate notation */
      if(pads_old == 0){
	if((AGS_AUDIO_NOTATION_DEFAULT & (audio->flags)) != 0){
	  ags_audio_set_pads_alloc_notation();
	}
      }

      /* grow channels */
      ags_audio_set_pads_grow();
    }else if(pads < pads_old){
      if(pads == 0){
	channel = audio->input;
	
	/* shrink channels */
	ags_audio_set_pads_unlink_all(channel);
	ags_audio_set_pads_shrink_zero(channel);
	
	audio->input = NULL;  
      }else{
	channel = ags_channel_pad_nth(channel,
				      pads);
	
	/* shrink channels */
	ags_audio_set_pads_unlink_all(channel);
	ags_audio_set_pads_shrink(channel);
      }
    }

    /* apply new allocation */
    audio->input_pads = pads;
    audio->input_lines = pads * audio->audio_channels;
  }else{
    g_warning("unknown channel type\0");
  }
}

/**
 * ags_audio_set_pads:
 * @audio: an #AgsAudio
 * @type: AGS_TYPE_INPUT or AGS_TYPE_OUTPUT
 * @pads: new pad count
 *
 * Sets pad count for the apropriate @type
 *
 * Since: 0.3
 */
void
ags_audio_set_pads(AgsAudio *audio, GType type, guint pads)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;
  
  guint pads_old;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  if(!AGS_IS_AUDIO(audio)){
    pthread_mutex_unlock(mutex);
    return;
  }

  /* emit */
  g_object_ref((GObject *) audio);
  pads_old = ((g_type_is_a(type, AGS_TYPE_OUTPUT)) ? audio->output_pads: audio->input_pads);
  g_signal_emit(G_OBJECT(audio),
		audio_signals[SET_PADS], 0,
		type, pads, pads_old);
  g_object_unref((GObject *) audio);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_set_sequence_length:
 * @audio: an #AgsAudio
 * @sequence_length: the sequence length
 *
 * Sets sequence length.
 *
 * Since: 0.4
 */
void
ags_audio_set_sequence_length(AgsAudio *audio, guint sequence_length)
{
  audio->sequence_length = sequence_length;
}

/**
 * ags_audio_add_recycling_container:
 * @audio: an #AgsAudio
 * @recycling_container: the #AgsRecyclingContainer
 *
 * Adds a recycling container.
 *
 * Since: 0.4
 */
void
ags_audio_add_recycling_container(AgsAudio *audio, GObject *recycling_container)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* add recycling container */
  pthread_mutex_lock(mutex);

  g_object_ref(recycling_container);
  audio->recycling_container = g_list_prepend(audio->recycling_container, recycling_container);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_remove_recycling_container:
 * @audio: an #AgsAudio
 * @recycling_container: the #AgsRecyclingContainer
 *
 * Removes a recycling container.
 *
 * Since: 0.4
 */
void
ags_audio_remove_recycling_container(AgsAudio *audio, GObject *recycling_container)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */  
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* remove recycling container */
  pthread_mutex_lock(mutex);

  audio->recycling_container = g_list_remove(audio->recycling_container, recycling_container);
  g_object_unref(recycling_container);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_add_recall_id:
 * @audio: an #AgsAudio
 * @recall_id: the #AgsRecallID
 *
 * Adds a recall id.
 *
 * Since: 0.4
 */
void
ags_audio_add_recall_id(AgsAudio *audio, GObject *recall_id)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* add recall id */
  pthread_mutex_lock(mutex);

  g_object_ref(recall_id);
  audio->recall_id = g_list_prepend(audio->recall_id, recall_id);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_remove_recall_id:
 * @audio: an #AgsAudio
 * @recall_id: the #AgsRecallID
 *
 * Removes a recall id.
 *
 * Since: 0.4
 */
void
ags_audio_remove_recall_id(AgsAudio *audio, GObject *recall_id)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* remove recall id */
  pthread_mutex_lock(mutex);

  audio->recall_id = g_list_remove(audio->recall_id, recall_id);
  g_object_unref(recall_id);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_add_recall_container:
 * @audio: an #AgsAudio
 * @recall_container: the #AgsRecallContainer
 *
 * Adds a recall container.
 *
 * Since: 0.4
 */
void
ags_audio_add_recall_container(AgsAudio *audio, GObject *recall_container)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* add recall container */
  pthread_mutex_lock(mutex);
  
  g_object_ref(recall_container);
  audio->container = g_list_prepend(audio->container, recall_container);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_remove_recall_container:
 * @audio: an #AgsAudio
 * @recall_container: the #AgsRecallContainer
 *
 * Removes a recall container.
 *
 * Since: 0.4
 */
void
ags_audio_remove_recall_container(AgsAudio *audio, GObject *recall_container)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* remove recall container */
  pthread_mutex_lock(mutex);

  audio->container = g_list_remove(audio->container, recall_container);
  g_object_unref(recall_container);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_add_recall:
 * @audio: an #AgsAudio
 * @recall_container: the #AgsRecall
 * @play: %TRUE if simple playback.
 *
 * Adds a recall.
 *
 * Since: 0.4
 */
void
ags_audio_add_recall(AgsAudio *audio, GObject *recall, gboolean play)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* add recall */
  pthread_mutex_lock(mutex);
  
  g_object_ref(G_OBJECT(recall));

  if(play){
    audio->play = g_list_prepend(audio->play, recall);
  }else{
    audio->recall = g_list_prepend(audio->recall, recall);
  }
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_remove_recall:
 * @audio: an #AgsAudio
 * @recall_container: the #AgsRecall
 * @play: %TRUE if simple playback.
 *
 * Removes a recall.
 *
 * Since: 0.4
 */
void
ags_audio_remove_recall(AgsAudio *audio, GObject *recall, gboolean play)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* remove recall */
  pthread_mutex_lock(mutex);

  if(play){
    audio->play = g_list_remove(audio->play, recall);
  }else{
    audio->recall = g_list_remove(audio->recall, recall);
  }

  g_object_unref(G_OBJECT(recall));
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_init_run:
 * @audio: an #AgsAudio
 *
 * Is emitted as audio is initialized.
 *
 * Returns: the current #AgsRecallID
 *
 * Since: 0.4
 */
AgsRecallID*
ags_audio_init_run(AgsAudio *audio)
{
  AgsRecallID *recall_id;
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  if(!AGS_IS_AUDIO(audio)){
    pthread_mutex_unlock(mutex);
    return;
  }

  /* emit */
  g_object_ref((GObject *) audio);
  g_signal_emit(G_OBJECT(audio),
		audio_signals[INIT_RUN], 0,
		&recall_id);
  g_object_unref((GObject *) audio);
  
  pthread_mutex_unlock(mutex);

  return(recall_id);
}

/**
 * ags_audio_duplicate_recall:
 * @audio: an #AgsAudio
 * @recall_id: an #AgsRecallID
 * 
 * Duplicate all #AgsRecall templates of this #AgsAudio.
 *
 * Since: 0.4
 */
void
ags_audio_duplicate_recall(AgsAudio *audio,
			   AgsRecallID *recall_id)
{
  AgsRecall *recall, *copy;
  GList *list_recall_start, *list_recall;
  gboolean playback, sequencer, notation;

  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  if(audio == NULL ||
     recall_id == NULL){
    return;
  }

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* initial checks */
  pthread_mutex_lock(mutex);
  
#ifdef AGS_DEBUG
  g_message("ags_audio_duplicate_recall: %s - audio.lines[%u,%u]\n\0", G_OBJECT_TYPE_NAME(audio->machine), audio->output_lines, audio->input_lines);  
#endif

  playback = FALSE;
  sequencer = FALSE;
  notation = FALSE;

  if((AGS_RECALL_ID_PLAYBACK & (recall_id->flags)) != 0){
    playback = TRUE;
  }

  if((AGS_RECALL_ID_SEQUENCER & (recall_id->flags)) != 0){
    sequencer = TRUE;
  }

  if((AGS_RECALL_ID_NOTATION & (recall_id->flags)) != 0){
    notation = TRUE;
  }
  
  /* get the appropriate list */
  if(recall_id->recycling_container->parent == NULL){
    list_recall = g_list_copy(audio->play);
    list_recall_start =
      list_recall = g_list_reverse(list_recall);
  }else{
    list_recall = g_list_copy(audio->recall);
    list_recall_start =
      list_recall = g_list_reverse(list_recall);
  }

  /* notify run */  
  //  ags_recall_notify_dependency(AGS_RECALL(list_recall->data), AGS_RECALL_NOTIFY_RUN, 1);

  /* return if already played */
  if((AGS_RECALL_ID_PRE & (recall_id->flags)) != 0 ||
     (AGS_RECALL_ID_INTER & (recall_id->flags)) != 0 ||
     (AGS_RECALL_ID_POST & (recall_id->flags)) != 0){
    pthread_mutex_unlock(mutex);
    return;
  }

  ags_recall_id_set_run_stage(recall_id, 0);
  ags_recall_id_set_run_stage(recall_id, 1);
  ags_recall_id_set_run_stage(recall_id, 2);

  /* duplicate */
  while(list_recall != NULL){
    recall = AGS_RECALL(list_recall->data);
    
    if((AGS_RECALL_RUN_INITIALIZED & (recall->flags)) != 0 ||
       AGS_IS_RECALL_AUDIO(recall) ||
       !((playback && (AGS_RECALL_PLAYBACK & (recall->flags)) != 0) ||
	 (sequencer && (AGS_RECALL_SEQUENCER & (recall->flags)) != 0) ||
	 (notation && (AGS_RECALL_NOTATION & (recall->flags)) != 0)) ||
       recall->recall_id != NULL){
      list_recall = list_recall->next;
      continue;
    }  

    /* duplicate template only once */
    if((AGS_RECALL_TEMPLATE & (recall->flags)) != 0){
      /* duplicate the recall */
      copy = ags_recall_duplicate(recall, recall_id);

      
      if(copy == NULL){
	/* iterate */    
	list_recall = list_recall->next;

	continue;
      }
    
      /* notify run */
      ags_recall_notify_dependency(copy, AGS_RECALL_NOTIFY_RUN, 1);

#ifdef AGS_DEBUG
      g_message("recall duplicated: %s\n\0", G_OBJECT_TYPE_NAME(copy));
#endif

      /* set appropriate flag */
      if(playback){
	ags_recall_set_flags(copy, AGS_RECALL_PLAYBACK);
      }else if(sequencer){
	ags_recall_set_flags(copy, AGS_RECALL_SEQUENCER);
      }else if(notation){
	ags_recall_set_flags(copy, AGS_RECALL_NOTATION);
      }

      /* append to AgsAudio */
      ags_audio_add_recall(audio,
			   copy,
			   ((recall_id->recycling_container->parent == NULL) ? TRUE: FALSE));

      /* connect */
      ags_connectable_connect(AGS_CONNECTABLE(copy));
    }

    /* iterate */
    list_recall = list_recall->next;
  }

  pthread_mutex_unlock(mutex);

  g_list_free(list_recall_start);
}

/**
 * ags_audio_init_recall:
 * @audio: the #AgsAudio
 * @stage: stage benning at 0 up to 2, or just -1
 * @recall_id: the #AgsRecallID to use or #NULL
 *
 * Initializes the recalls of @audio
 *
 * Since: 0.4
 */
void
ags_audio_init_recall(AgsAudio *audio, gint stage,
		      AgsRecallID *recall_id)
{
  AgsRecall *recall;
  GList *list_recall;
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  if(audio == NULL ||
     recall_id == NULL){
    return;
  }
  
  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  /* check for init */
  
  /* return if already initialized */
  switch(stage){
  case 0:
    if((AGS_RECALL_ID_INIT_PRE & (recall_id->flags)) != 0){
      pthread_mutex_unlock(mutex);
      return;
    }
    break;
  case 1:
    if((AGS_RECALL_ID_INIT_INTER & (recall_id->flags)) != 0){
      pthread_mutex_unlock(mutex);
      return;
    }
    break;
  case 2:
    if((AGS_RECALL_ID_INIT_POST & (recall_id->flags)) != 0){
      pthread_mutex_unlock(mutex);
      return;
    }
    break;
  }

  /* retrieve appropriate recalls */
  if(recall_id->recycling_container->parent == NULL)
    list_recall = audio->play;
  else
    list_recall = audio->recall;

  /* init  */
  while(list_recall != NULL){
    recall = AGS_RECALL(list_recall->data);
    
    if(recall->recall_id == NULL ||
       recall->recall_id->recycling_container != recall_id->recycling_container ||
       AGS_IS_RECALL_AUDIO(recall)){
      list_recall = list_recall->next;
      continue;
    }
    
    if((AGS_RECALL_TEMPLATE & (recall->flags)) == 0){
      if(stage == 0){
	recall_id->flags |= AGS_RECALL_ID_INIT_PRE;
	ags_dynamic_connectable_connect_dynamic(AGS_DYNAMIC_CONNECTABLE(recall));
    
	recall->flags &= (~AGS_RECALL_HIDE);
	ags_recall_run_init_pre(recall);
	recall->flags &= (~AGS_RECALL_REMOVE);
      }else if(stage == 1){
	recall_id->flags |= AGS_RECALL_ID_INIT_INTER;
	ags_recall_run_init_inter(recall);
      }else{
	recall_id->flags |= AGS_RECALL_ID_INIT_POST;
	ags_recall_run_init_post(recall);
      }
    }
    
    list_recall = list_recall->next;
  }
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_resolve_recall:
 * @audio: the #AgsAudio
 * @recall_id: the #AgsRecallID to use
 *
 * Performs resolving of recalls.
 *
 * Since: 0.4
 */
void ags_audio_resolve_recall(AgsAudio *audio,
			      AgsRecallID *recall_id)
{
  AgsRecall *recall;
  GList *list_recall;  
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  if(audio == NULL ||
     recall_id == NULL){
    return;
  }

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  /* return if already duplicated */
  if((AGS_RECALL_ID_RESOLVE & (recall_id->flags)) != 0){
    pthread_mutex_unlock(mutex);
    return;
  }
  
  /* get the appropriate lists */
  if(recall_id->recycling_container->parent == NULL){
    list_recall = audio->play;
  }else{
    list_recall = audio->recall;
  }

  /* resolve */  
  while((list_recall = ags_recall_find_recycling_container(list_recall, (GObject *) recall_id->recycling_container)) != NULL){
    recall = AGS_RECALL(list_recall->data);
    
    ags_recall_resolve_dependencies(recall);

    list_recall = list_recall->next;
  }

  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_is_playing:
 * @audio: the #AgsAudio
 *
 * Determine if #AgsAudio is playing.
 *
 * Returns: TRUE if it's playing otherwise FALSE
 *
 * Since: 0.4
 */
gboolean
ags_audio_is_playing(AgsAudio *audio)
{
  AgsChannel *output;
  AgsRecallID *recall_id;
  AgsDevoutPlay *devout_play;
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* check if playing */
  pthread_mutex_lock(mutex);
  
  output = audio->output;

  while(output != NULL){
    devout_play = AGS_DEVOUT_PLAY(output->devout_play);

    if((AGS_DEVOUT_PLAY_PLAYBACK & (g_atomic_int_get(&(devout_play->flags)))) != 0 ||
       (AGS_DEVOUT_PLAY_SEQUENCER & (g_atomic_int_get(&(devout_play->flags)))) != 0 ||
       (AGS_DEVOUT_PLAY_NOTATION & (g_atomic_int_get(&(devout_play->flags)))) != 0){
  
      pthread_mutex_unlock(mutex);
      
      return(TRUE);
    }

    output = output->next;
  }
  
  pthread_mutex_unlock(mutex);

  return(FALSE);
}

/**
 * ags_audio_play:
 * @audio: the #AgsAudio
 * @recall_id: the #AgsRecallID to apply to
 * @stage: valid range is from 0 up to 2
 *
 * Performs on single play call of appropriate stage.
 *
 * Since: 0.4
 */
void
ags_audio_play(AgsAudio *audio,
	       AgsRecallID *recall_id,
	       gint stage)
{
  AgsRecall *recall;
  GList *list, *list_next;
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  if(audio == NULL ||
     recall_id == NULL){
    return;
  }

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  /* check for status */
  if((AGS_RECALL_ID_PRE & (recall_id->flags)) != 0 &&
     (AGS_RECALL_ID_INTER & (recall_id->flags)) != 0 &&
     (AGS_RECALL_ID_POST & (recall_id->flags)) != 0){
    recall_id->flags &= (~(AGS_RECALL_ID_PRE |
			   AGS_RECALL_ID_INTER |
			   AGS_RECALL_ID_POST));
  }
  
  /* return if already played */
  switch(stage){
  case 0:
    ags_recall_id_unset_run_stage(recall_id, 1);
    
    if((AGS_RECALL_ID_PRE & (recall_id->flags)) != 0){
      pthread_mutex_unlock(mutex);
      return;
    }
    break;
  case 1:
    ags_recall_id_unset_run_stage(recall_id, 2);

    if((AGS_RECALL_ID_INTER & (recall_id->flags)) != 0){
      pthread_mutex_unlock(mutex);
      return;
    }
    break;
  case 2:
    ags_recall_id_unset_run_stage(recall_id, 0);

    if((AGS_RECALL_ID_POST & (recall_id->flags)) != 0){
      pthread_mutex_unlock(mutex);
      return;
    }
    break;
  }

  ags_recall_id_set_run_stage(recall_id, stage);

  /* retrieve appropriate recalls */
  if(recall_id->recycling_container->parent == NULL)
    list = audio->play;
  else
    list = audio->recall;

  /* play */
  while(list != NULL){
    list_next = list->next;

    recall = AGS_RECALL(list->data);

    if(recall == NULL){
      if(recall_id->recycling_container->parent != NULL){
	audio->recall = g_list_remove(audio->recall,
				      recall);
      }else{
	audio->play = g_list_remove(audio->play,
				    recall);
      }

      g_warning("recall == NULL\0");
      list = list_next;
      continue;
    }

    if((AGS_RECALL_TEMPLATE & (recall->flags)) != 0 ||
       recall->recall_id == NULL ||
       (recall->recall_id->recycling_container != recall_id->recycling_container) ||
       AGS_IS_RECALL_AUDIO(recall)){
      list = list_next;

      continue;
    }
    
    if((AGS_RECALL_HIDE & (recall->flags)) == 0){
      if(stage == 0)
	ags_recall_run_pre(recall);
      else if(stage == 1)
	ags_recall_run_inter(recall);
      else
	ags_recall_run_post(recall);
    }

    list = list_next;
  }
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_tact:
 * @audio: an #AgsAudio
 * @recall_id: the current #AgsRecallID
 *
 * Is emitted as audio is played.
 *
 * Since: 0.4
 */
void
ags_audio_tact(AgsAudio *audio, AgsRecallID *recall_id)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  if(!AGS_IS_AUDIO(audio)){
    pthread_mutex_unlock(mutex);
    return;
  }

  /* emit */
  g_object_ref((GObject *) audio);
  g_signal_emit(G_OBJECT(audio),
		audio_signals[TACT], 0,
		recall_id);
  g_object_unref((GObject *) audio);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_done:
 * @audio: an #AgsAudio
 * @recall_id: the current #AgsRecallID
 *
 * Is emitted as playing audio is done.
 *
 * Since: 0.4
 */
void
ags_audio_done(AgsAudio *audio, AgsRecallID *recall_id)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  if(!AGS_IS_AUDIO(audio)){
    pthread_mutex_unlock(mutex);
    return;
  }

  /* emit */
  g_object_ref((GObject *) audio);
  g_signal_emit(G_OBJECT(audio),
		audio_signals[DONE], 0,
		recall_id);
  g_object_unref((GObject *) audio);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_cancel:
 * @audio: the #AgsAudio
 * @recall_id: the #AgsRecallID to apply to
 *
 * Cancel processing audio data.
 *
 * Since: 0.4
 */
void
ags_audio_cancel(AgsAudio *audio,
		 AgsRecallID *recall_id)
{
  AgsRecall *recall;
  GList *list, *list_next;
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;
    
  if(audio == NULL || recall_id == NULL){
    return;
  }

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* cancel recalls */
  pthread_mutex_lock(mutex);

  if(recall_id->recycling_container->parent == NULL){
    list = audio->play;
  }else{
    list = audio->recall;
  }
  
  g_object_ref(recall_id);

  while(list != NULL){
    list_next = list->next;

    recall = AGS_RECALL(list->data);

    if((AGS_RECALL_TEMPLATE & (recall->flags)) ||
       recall->recall_id == NULL ||
       recall->recall_id->recycling_container != recall_id->recycling_container){
      list = list_next;

      continue;
    }

    g_object_ref(recall_id);
    g_object_ref(recall);
    ags_recall_cancel(recall);
    
    list = list_next;
  }
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_remove:
 * @audio: the #AgsAudio
 * @recall_id: the #AgsRecallID to apply to
 *
 * Remove processing audio data.
 *
 * Since: 0.4
 */
void
ags_audio_remove(AgsAudio *audio,
		 AgsRecallID *recall_id)
{
  AgsRecall *recall;
  GList *list, *list_next;
  gboolean play;
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;
  
  if(audio == NULL || recall_id == NULL){
    return;
  }

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
    
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* remove recalls */
  pthread_mutex_lock(mutex);

  if(recall_id->recycling_container->parent == NULL){
    list = audio->play;
    play = TRUE;
  }else{
    list = audio->recall;
    play = FALSE;
  }

  while(list != NULL){
    list_next = list->next;

    recall = AGS_RECALL(list->data);

    if((AGS_RECALL_TEMPLATE & (recall->flags)) ||
       recall->recall_id == NULL ||
       recall->recall_id->recycling_container != recall_id->recycling_container){
      list = list_next;

      continue;
    }

    ags_recall_remove(recall);
    ags_audio_remove_recall(audio,
			    recall,
			    play);
    
    list = list_next;
  }

  audio->recall_id = g_list_remove(audio->recall_id,
				   recall_id);
  g_object_unref(recall_id);
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_find_port:
 * @audio: an #AgsAudio
 *
 * Retrieve all ports of #AgsAudio.
 *
 * Returns: a new #GList containing #AgsPort
 *
 * Since: 0.4
 */
GList*
ags_audio_find_port(AgsAudio *audio)
{
  GList *recall;
  GList *list;
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *mutex;
  
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  list = NULL;
 
  /* collect port of playing recall */
  recall = audio->play;
   
  while(recall != NULL){
    if(AGS_RECALL(recall->data)->port != NULL){
      if(list == NULL){
	list = g_list_copy(AGS_RECALL(recall->data)->port);
      }else{
	if(AGS_RECALL(recall->data)->port != NULL){
	  list = g_list_concat(list,
			       g_list_copy(AGS_RECALL(recall->data)->port));
	}
      }
    }
     
    recall = recall->next;
  }
 
  /* the same for true recall */
  recall = audio->recall;
   
  while(recall != NULL){
    if(AGS_RECALL(recall->data)->port != NULL){
      if(list == NULL){
	list = g_list_copy(AGS_RECALL(recall->data)->port);
      }else{
	if(AGS_RECALL(recall->data)->port != NULL){
	  list = g_list_concat(list,
			       g_list_copy(AGS_RECALL(recall->data)->port));
	}
      }
    }
       
    recall = recall->next;
  }
   
  /*  */
  list = g_list_reverse(list);
    
  pthread_mutex_unlock(mutex);
  
  return(list);
}

/**
 * ags_audio_open_files:
 * @audio: the #AgsAudio
 * @filenames: the files to open
 * @overwrite_channels: if existing channels should be assigned
 * @create_channels: if new channels should be created as not fitting if combined with @overwrite_channels
 *
 * Open some files.
 *
 * Since: 0.4
 */
void
ags_audio_open_files(AgsAudio *audio,
		     GSList *filenames,
		     gboolean overwrite_channels,
		     gboolean create_channels)
{
  AgsChannel *channel;
  AgsAudioFile *audio_file;
  AgsAudioSignal *audio_signal_source_old;

  AgsMutexManager *mutex_manager;

  GList *audio_signal_list;

  guint i, j;
  guint list_length;

  GError *error;

  pthread_mutex_t *mutex, *channel_mutex, *recycling_mutex;

  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  pthread_mutex_lock(mutex);

  channel = audio->input;

  /* overwriting existing channels */
  if(overwrite_channels){
    if(channel != NULL){
      for(i = 0; i < audio->input_pads && filenames != NULL; i++){
	audio_file = ags_audio_file_new((gchar *) filenames->data,
					(AgsDevout *) audio->devout,
					0, audio->audio_channels);
	if(!ags_audio_file_open(audio_file)){
	  filenames = filenames->next;
	  continue;
	}

	ags_audio_file_read_audio_signal(audio_file);
	ags_audio_file_close(audio_file);
	
	audio_signal_list = audio_file->audio_signal;
	
	for(j = 0; j < audio->audio_channels && audio_signal_list != NULL; j++){
	  /* create task */
	  error = NULL;

	  ags_channel_set_link(channel, NULL,
			       &error);

	  if(error != NULL){
	    g_warning("%s\0", error->message);
	  }

	  /* lock channel and recycling */
	  pthread_mutex_lock(&(ags_application_mutex));
  
	  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						   (GObject *) channel);

	  recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
						     (GObject *) channel->first_recycling);
	  
	  pthread_mutex_unlock(&(ags_application_mutex));

	  /* replace template audio signal */
	  pthread_mutex_lock(channel_mutex);
	  pthread_mutex_lock(recycling_mutex);
	  
	  AGS_AUDIO_SIGNAL(audio_signal_list->data)->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
	  AGS_AUDIO_SIGNAL(audio_signal_list->data)->recycling = (GObject *) channel->first_recycling;
	  audio_signal_source_old = ags_audio_signal_get_template(channel->first_recycling->audio_signal);

	  ags_recycling_remove_audio_signal(channel->first_recycling,
					    (gpointer) audio_signal_source_old);
	  ags_recycling_add_audio_signal(channel->first_recycling,
					 audio_signal_list->data);

	  g_object_unref(G_OBJECT(audio_signal_source_old));

	  /* iterate */
	  audio_signal_list = audio_signal_list->next;
	  channel = channel->next;
	  
	  pthread_mutex_unlock(recycling_mutex);
	  pthread_mutex_unlock(channel_mutex);
	}

	if(audio_file->channels < audio->audio_channels)
	  channel = ags_channel_nth(channel,
				    audio->audio_channels - audio_file->channels);
	
	filenames = filenames->next;
      }
    }
  }

  /* appending to channels */
  if(create_channels && filenames != NULL){
    list_length = g_slist_length(filenames);
    
    ags_audio_set_pads((AgsAudio *) audio, AGS_TYPE_INPUT,
		       list_length + AGS_AUDIO(audio)->input_pads);
    channel = ags_channel_nth(AGS_AUDIO(audio)->input,
			      (AGS_AUDIO(audio)->input_pads - list_length) * AGS_AUDIO(audio)->audio_channels);
    
    while(filenames != NULL){
      audio_file = ags_audio_file_new((gchar *) filenames->data,
				      (AgsDevout *) audio->devout,
				      0, audio->audio_channels);
      if(!ags_audio_file_open(audio_file)){
	filenames = filenames->next;
	continue;
      }
      
      ags_audio_file_read_audio_signal(audio_file);
      ags_audio_file_close(audio_file);
	
      audio_signal_list = audio_file->audio_signal;
      
      for(j = 0; j < audio->audio_channels && audio_signal_list != NULL; j++){
	/* lock channel and recycling */
	pthread_mutex_lock(&(ags_application_mutex));
  
	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);

	recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
						   (GObject *) channel->first_recycling);
	  
	pthread_mutex_unlock(&(ags_application_mutex));

	/* replace template audio signal */
	pthread_mutex_lock(channel_mutex);
	pthread_mutex_lock(recycling_mutex);
	  
	AGS_AUDIO_SIGNAL(audio_signal_list->data)->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
	AGS_AUDIO_SIGNAL(audio_signal_list->data)->recycling = (GObject *) channel->first_recycling;
	audio_signal_source_old = ags_audio_signal_get_template(channel->first_recycling->audio_signal);
	
	ags_recycling_remove_audio_signal(channel->first_recycling,
					  (gpointer) audio_signal_source_old);
	ags_recycling_add_audio_signal(channel->first_recycling,
				       audio_signal_list->data);
	
	g_object_unref(G_OBJECT(audio_signal_source_old));

	/* iterate */
	audio_signal_list = audio_signal_list->next;
	channel = channel->next;

	pthread_mutex_unlock(recycling_mutex);
	pthread_mutex_unlock(channel_mutex);
      }
      
      if(audio->audio_channels > audio_file->channels)
	channel = ags_channel_nth(channel,
				  audio->audio_channels - audio_file->channels);
      
      filenames = filenames->next;
    }
  }
  
  pthread_mutex_unlock(mutex);
}

/**
 * ags_audio_recursive_play_init:
 * @audio: the #AgsAudio object
 * @playback: if doing playback
 * @sequencer: if doing sequencer
 * @notation: if doing notation
 *
 * Initializes #AgsAudio in order to do playback, sequencer or notation.
 *
 * Returns: a list containing all #AgsRecallID
 *
 * Since: 0.4
 */
GList*
ags_audio_recursive_play_init(AgsAudio *audio,
			      gboolean playback, gboolean sequencer, gboolean notation)
{
  AgsChannel *channel;
  AgsRecallID *recall_id;

  AgsMutexManager *mutex_manager;
  
  GList *list, *list_start;
  gint stage;
  gboolean arrange_recall_id, duplicate_templates, resolve_dependencies;

  pthread_mutex_t *mutex, *channel_mutex;

  if(audio == NULL){
    return;
  }
  
  /* lookup mutex */
  pthread_mutex_lock(&(ags_application_mutex));
  
  mutex_manager = ags_mutex_manager_get_instance();

  mutex = ags_mutex_manager_lookup(mutex_manager,
				   (GObject *) audio);
  
  pthread_mutex_unlock(&(ags_application_mutex));

  /* recurisive init playback */
  pthread_mutex_lock(mutex);

  list = NULL;
  list_start = NULL;

  for(stage = 0; stage < 3; stage++){
    channel = audio->output;
    list = list_start;

    if(stage == 0){
      arrange_recall_id = TRUE;
      duplicate_templates = TRUE;
      resolve_dependencies = TRUE;
    }else{
      arrange_recall_id = FALSE;
      duplicate_templates = FALSE;
      resolve_dependencies = FALSE;
    }

    while(channel != NULL){
      if(stage == 0){
	recall_id = ags_channel_recursive_play_init(channel, stage,
						    arrange_recall_id, duplicate_templates,
						    playback, sequencer, notation,
						    resolve_dependencies,
						    NULL);
	
	list_start = g_list_append(list_start,
				   recall_id);
      }else{
	ags_channel_recursive_play_init(channel, stage,
					arrange_recall_id, duplicate_templates,
					playback, sequencer, notation,
					resolve_dependencies,
					AGS_RECALL_ID(list->data));

	list = list->next;
      }

      /* iterate */
      pthread_mutex_lock(&(ags_application_mutex));
      channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) channel);
  
      pthread_mutex_unlock(&(ags_application_mutex));
      
      pthread_mutex_lock(channel_mutex);

      channel = channel->next;

      pthread_mutex_unlock(channel_mutex);
    }
  }
  
  pthread_mutex_unlock(mutex);

  return(list_start);
}

/**
 * ags_audio_new:
 * @devout: an #AgsDevout
 *
 * Creates an #AgsAudio, with defaults of @devout.
 *
 * Returns: a new #AgsAudio
 *
 * Since: 0.3
 */
AgsAudio*
ags_audio_new(AgsDevout *devout)
{
  AgsAudio *audio;

  audio = (AgsAudio *) g_object_new(AGS_TYPE_AUDIO,
				    "devout\0", devout,
				    NULL);

  return(audio);
}
