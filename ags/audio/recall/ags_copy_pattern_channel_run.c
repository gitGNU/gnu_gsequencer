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

#include <ags/audio/recall/ags_copy_pattern_channel_run.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_plugin.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_pattern.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_copy_pattern_audio.h>
#include <ags/audio/recall/ags_copy_pattern_audio_run.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>
#include <ags/audio/recall/ags_delay_audio_run.h>

#include <stdlib.h>

void ags_copy_pattern_channel_run_class_init(AgsCopyPatternChannelRunClass *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_copy_pattern_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_copy_pattern_channel_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_copy_pattern_channel_run_init(AgsCopyPatternChannelRun *copy_pattern_channel_run);
void ags_copy_pattern_channel_run_connect(AgsConnectable *connectable);
void ags_copy_pattern_channel_run_disconnect(AgsConnectable *connectable);
void ags_copy_pattern_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_copy_pattern_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_copy_pattern_channel_run_finalize(GObject *gobject);

void ags_copy_pattern_channel_run_resolve_dependencies(AgsRecall *recall);
void ags_copy_pattern_channel_run_run_init_pre(AgsRecall *recall);
void ags_copy_pattern_channel_run_done(AgsRecall *recall);
void ags_copy_pattern_channel_run_cancel(AgsRecall *recall);
void ags_copy_pattern_channel_run_remove(AgsRecall *recall);
AgsRecall* ags_copy_pattern_channel_run_duplicate(AgsRecall *recall,
						  AgsRecallID *recall_id,
						  guint *n_params, GParameter *parameter);

void ags_copy_pattern_channel_run_sequencer_alloc_callback(AgsDelayAudioRun *delay_audio_run,
							   guint run_order,
							   gdouble delay, guint attack,
							   AgsCopyPatternChannelRun *copy_pattern_channel_run);

/**
 * SECTION:ags_copy_pattern_channel_run
 * @short_description: copys pattern
 * @title: AgsCopyPatternChannelRun
 * @section_id:
 * @include: ags/audio/recall/ags_copy_pattern_channel_run.h
 *
 * The #AgsCopyPatternChannelRun class copys pattern.
 */

static gpointer ags_copy_pattern_channel_run_parent_class = NULL;
static AgsConnectableInterface* ags_copy_pattern_channel_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_copy_pattern_channel_run_parent_dynamic_connectable_interface;
static AgsPluginInterface *ags_copy_pattern_channel_run_parent_plugin_interface;

GType
ags_copy_pattern_channel_run_get_type()
{
  static GType ags_type_copy_pattern_channel_run = 0;

  if(!ags_type_copy_pattern_channel_run){
    static const GTypeInfo ags_copy_pattern_channel_run_info = {
      sizeof (AgsCopyPatternChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_copy_pattern_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCopyPatternChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_copy_pattern_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_pattern_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_pattern_channel_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_copy_pattern_channel_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };    

    ags_type_copy_pattern_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
							       "AgsCopyPatternChannelRun\0",
							       &ags_copy_pattern_channel_run_info,
							       0);
    
    g_type_add_interface_static(ags_type_copy_pattern_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
    
    g_type_add_interface_static(ags_type_copy_pattern_channel_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);

    g_type_add_interface_static(ags_type_copy_pattern_channel_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_copy_pattern_channel_run);
}

void
ags_copy_pattern_channel_run_class_init(AgsCopyPatternChannelRunClass *copy_pattern_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  AgsRecallChannelRunClass *recall_channel_run;
  GParamSpec *param_spec;

  ags_copy_pattern_channel_run_parent_class = g_type_class_peek_parent(copy_pattern_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_pattern_channel_run;

  gobject->finalize = ags_copy_pattern_channel_run_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) copy_pattern_channel_run;

  recall->resolve_dependencies = ags_copy_pattern_channel_run_resolve_dependencies;
  recall->run_init_pre = ags_copy_pattern_channel_run_run_init_pre;
  recall->done = ags_copy_pattern_channel_run_done;
  recall->cancel = ags_copy_pattern_channel_run_cancel;
  recall->remove = ags_copy_pattern_channel_run_remove;
  recall->duplicate = ags_copy_pattern_channel_run_duplicate;
}

void
ags_copy_pattern_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_copy_pattern_channel_run_connectable_parent_interface;

  ags_copy_pattern_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_copy_pattern_channel_run_connect;
  connectable->disconnect = ags_copy_pattern_channel_run_disconnect;
}

void
ags_copy_pattern_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_copy_pattern_channel_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_copy_pattern_channel_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_copy_pattern_channel_run_disconnect_dynamic;
}

void
ags_copy_pattern_channel_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_copy_pattern_channel_run_parent_plugin_interface = g_type_interface_peek_parent(plugin);
}

void
ags_copy_pattern_channel_run_init(AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  AGS_RECALL(copy_pattern_channel_run)->name = "ags-copy-pattern\0";
  AGS_RECALL(copy_pattern_channel_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(copy_pattern_channel_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(copy_pattern_channel_run)->xml_type = "ags-copy-pattern-channel-run\0";
  AGS_RECALL(copy_pattern_channel_run)->port = NULL;

  AGS_RECALL(copy_pattern_channel_run)->child_type = G_TYPE_NONE;
}

void
ags_copy_pattern_channel_run_connect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) != 0){
    return;
  }

  ags_copy_pattern_channel_run_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_copy_pattern_channel_run_disconnect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) == 0){
    return;
  }

  ags_copy_pattern_channel_run_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_copy_pattern_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  AgsDelayAudioRun *delay_audio_run;

  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_copy_pattern_channel_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* AgsCopyPatternChannelRun */
  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(dynamic_connectable);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run);

  /* connect sequencer_alloc in AgsDelayAudioRun */
  delay_audio_run = copy_pattern_audio_run->delay_audio_run;

  copy_pattern_channel_run->sequencer_alloc_handler =
    g_signal_connect(G_OBJECT(delay_audio_run), "sequencer-alloc-input\0",
		     G_CALLBACK(ags_copy_pattern_channel_run_sequencer_alloc_callback), copy_pattern_channel_run);
}

void
ags_copy_pattern_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  AgsDelayAudioRun *delay_audio_run;

  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) == 0){
    return;
  }

  /* AgsCopyPatternChannelRun */
  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(dynamic_connectable);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run);

  /* disconnect sequencer_alloc in AgsDelayAudioRun */
  delay_audio_run = copy_pattern_audio_run->delay_audio_run;

  g_signal_handler_disconnect(G_OBJECT(delay_audio_run),
			      copy_pattern_channel_run->sequencer_alloc_handler);

  /* call parent */
  ags_copy_pattern_channel_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);
}

void
ags_copy_pattern_channel_run_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_copy_pattern_channel_run_parent_class)->finalize(gobject);
}

void
ags_copy_pattern_channel_run_resolve_dependencies(AgsRecall *recall)
{
  //TODO:JK: implement me
}

void
ags_copy_pattern_channel_run_run_init_pre(AgsRecall *recall)
{
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->run_init_pre(recall);

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(recall);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run);

  /* notify dependency */
  ags_recall_notify_dependency(AGS_RECALL(copy_pattern_audio_run->count_beats_audio_run),
 			       AGS_RECALL_NOTIFY_CHANNEL_RUN, 1);
}

void
ags_copy_pattern_channel_run_done(AgsRecall *recall)
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;

  copy_pattern_channel_run = AGS_COPY_PATTERN_CHANNEL_RUN(recall);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run);

  /* denotify dependency */
  ags_recall_notify_dependency(AGS_RECALL(copy_pattern_audio_run->count_beats_audio_run),
 			       AGS_RECALL_NOTIFY_CHANNEL_RUN, -1);

  /* call parent */
  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->done(recall);
}

void
ags_copy_pattern_channel_run_cancel(AgsRecall *recall)
{
  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->cancel(recall);
}

void
ags_copy_pattern_channel_run_remove(AgsRecall *recall)
{

  AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->remove(recall);

  /* empty */
}

AgsRecall*
ags_copy_pattern_channel_run_duplicate(AgsRecall *recall,
				       AgsRecallID *recall_id,
				       guint *n_params, GParameter *parameter)
{
  AgsCopyPatternChannelRun *copy;

  copy = AGS_COPY_PATTERN_CHANNEL_RUN(AGS_RECALL_CLASS(ags_copy_pattern_channel_run_parent_class)->duplicate(recall,
													     recall_id,
													     n_params, parameter));

  /* empty */

  return((AgsRecall *) copy);
}

void
ags_copy_pattern_channel_run_sequencer_alloc_callback(AgsDelayAudioRun *delay_audio_run,
						      guint run_order,
						      gdouble delay, guint attack,
						      AgsCopyPatternChannelRun *copy_pattern_channel_run)
{
  AgsChannel *source;
  AgsPattern *pattern;
  AgsCopyPatternAudio *copy_pattern_audio;
  AgsCopyPatternAudioRun *copy_pattern_audio_run;
  AgsCopyPatternChannel *copy_pattern_channel;

  AgsMutexManager *mutex_manager;

  gboolean current_bit;

  GValue pattern_value = { 0, };  
  GValue i_value = { 0, };
  GValue j_value = { 0, };

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *pattern_mutex;
  pthread_mutex_t *source_mutex;
  
  if(delay != 0.0){
    return;
  }

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get AgsCopyPatternAudio */
  copy_pattern_audio = AGS_COPY_PATTERN_AUDIO(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run->recall_audio);

  /* get AgsCopyPatternAudioRun */
  copy_pattern_audio_run = AGS_COPY_PATTERN_AUDIO_RUN(AGS_RECALL_CHANNEL_RUN(copy_pattern_channel_run)->recall_audio_run);

  /* get AgsCopyPatternChannel */
  copy_pattern_channel = AGS_COPY_PATTERN_CHANNEL(copy_pattern_channel_run->recall_channel_run.recall_channel);

  g_value_init(&i_value, G_TYPE_UINT64);
  ags_port_safe_read(copy_pattern_audio->bank_index_0, &i_value);

  g_value_init(&j_value, G_TYPE_UINT64);
  ags_port_safe_read(copy_pattern_audio->bank_index_1, &j_value);

  /* get AgsPattern */
  g_value_init(&pattern_value, G_TYPE_POINTER);
  ags_port_safe_read(copy_pattern_channel->pattern,
		     &pattern_value);

  pattern = g_value_get_pointer(&pattern_value);

  pthread_mutex_lock(application_mutex);
  
  pattern_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) pattern);
  
  pthread_mutex_unlock(application_mutex);

  /* write pattern port - current offset */
  ags_port_safe_set_property(copy_pattern_channel->pattern,
			     "first-index\0", &i_value);
  g_value_unset(&i_value);
  
  ags_port_safe_set_property(copy_pattern_channel->pattern,
			     "second-index\0", &j_value);
  g_value_unset(&j_value);
  
  /* read pattern port - current bit */
  pthread_mutex_lock(pattern_mutex);
  
  current_bit = ags_pattern_get_bit(pattern,
				    pattern->i,
				    pattern->j,
				    copy_pattern_audio_run->count_beats_audio_run->sequencer_counter);
  
  pthread_mutex_unlock(pattern_mutex);

  g_value_unset(&pattern_value);

  /*  */
  if(current_bit){
    AgsChannel *link;
    AgsRecycling *recycling;
    AgsRecycling *end_recycling;
    AgsAudioSignal *audio_signal;

    gdouble delay;
    guint attack;
  
    pthread_mutex_t *link_mutex;
    
    //    g_message("ags_copy_pattern_channel_run_sequencer_alloc_callback - playing channel: %u; playing pattern: %u\0",
    //	      AGS_RECALL_CHANNEL(copy_pattern_channel)->source->line,
    //	      copy_pattern_audio_run->count_beats_audio_run->sequencer_counter);

    /* get source */
    source = AGS_RECALL_CHANNEL(copy_pattern_channel)->source;

    pthread_mutex_lock(application_mutex);
  
    source_mutex = ags_mutex_manager_lookup(mutex_manager,
					    (GObject *) source);
    
    pthread_mutex_unlock(application_mutex);
    
    /* source fields */
    pthread_mutex_lock(source_mutex);

    link = source->link;
    
    recycling = source->first_recycling;

    if(recycling != NULL){
      end_recycling = source->last_recycling->next;
    }
    
    pthread_mutex_unlock(source_mutex);

    /* link */
    if(link != NULL){
      pthread_mutex_lock(application_mutex);
      
      link_mutex = ags_mutex_manager_lookup(mutex_manager,
					    (GObject *) link);
      
      pthread_mutex_unlock(application_mutex);
    }

    /* create audio signals */
    if(recycling != NULL){
      AgsRecallID *child_recall_id;

      while(recycling != end_recycling){
	if(link == NULL){
	  child_recall_id = AGS_RECALL(copy_pattern_channel_run)->recall_id;
	}else{
	  GList *list;

	  pthread_mutex_lock(link_mutex);
	  
	  list = link->recall_id;

	  while(list != NULL){
	    if(AGS_RECALL_ID(list->data)->recycling_context->parent == AGS_RECALL(copy_pattern_channel_run)->recall_id->recycling_context){
	      child_recall_id = (AgsRecallID *) list->data;
	      break;
	    }
	  
	    list = list->next;
	  }

	  if(list == NULL){
	    child_recall_id = NULL;
	  }

	  pthread_mutex_unlock(link_mutex);
	}

	audio_signal = ags_audio_signal_new(AGS_RECALL(copy_pattern_audio)->soundcard,
					    (GObject *) recycling,
					    (GObject *) child_recall_id);
	ags_recycling_create_audio_signal_with_defaults(recycling,
							audio_signal,
							0.0, attack);
	audio_signal->flags &= (~AGS_AUDIO_SIGNAL_TEMPLATE);
	audio_signal->stream_current = audio_signal->stream_beginning;
	ags_connectable_connect(AGS_CONNECTABLE(audio_signal));
	
	/*
	 * emit add_audio_signal on AgsRecycling
	 */
#ifdef AGS_DEBUG
	g_message("play %x\0", AGS_RECALL(copy_pattern_channel_run)->recall_id);
#endif

	audio_signal->recall_id = (GObject *) child_recall_id;
	ags_recycling_add_audio_signal(recycling,
				       audio_signal);

	/*
	 * unref AgsAudioSignal because AgsCopyPatternChannelRun has no need for it
	 * if you need a valid reference to audio_signal you have to g_object_ref(audio_signal)
	 */
	//	g_object_unref(audio_signal);
		
	recycling = recycling->next;
      }
    }
  }
  
      //      g_message("%u\n\0", copy_pattern->shared_audio_run->bit);
      //      copy_pattern->shared_audio_run->bit++;
  //  }
}

/**
 * ags_copy_pattern_channel_run_new:
 *
 * Creates an #AgsCopyPatternChannelRun
 *
 * Returns: a new #AgsCopyPatternChannelRun
 *
 * Since: 0.4
 */
AgsCopyPatternChannelRun*
ags_copy_pattern_channel_run_new()
{
  AgsCopyPatternChannelRun *copy_pattern_channel_run;

  copy_pattern_channel_run = (AgsCopyPatternChannelRun *) g_object_new(AGS_TYPE_COPY_PATTERN_CHANNEL_RUN,
								       NULL);

  return(copy_pattern_channel_run);
}
