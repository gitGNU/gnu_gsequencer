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

#include <ags/audio/recall/ags_play_notation_audio_run.h>
#include <ags/audio/recall/ags_play_notation_audio.h>

#include <ags/util/ags_id_generator.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_config.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_plugin.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>
#include <ags/file/ags_file_lookup.h>

#include <ags/thread/ags_timestamp_thread.h>

#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/thread/ags_audio_loop.h>
#include <ags/audio/thread/ags_soundcard_thread.h>

void ags_play_notation_audio_run_class_init(AgsPlayNotationAudioRunClass *play_notation_audio_run);
void ags_play_notation_audio_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_play_notation_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_play_notation_audio_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_play_notation_audio_run_init(AgsPlayNotationAudioRun *play_notation_audio_run);
void ags_play_notation_audio_run_set_property(GObject *gobject,
					      guint prop_id,
					      const GValue *value,
					      GParamSpec *param_spec);
void ags_play_notation_audio_run_get_property(GObject *gobject,
					      guint prop_id,
					      GValue *value,
					      GParamSpec *param_spec);
void ags_play_notation_audio_run_finalize(GObject *gobject);
void ags_play_notation_audio_run_connect(AgsConnectable *connectable);
void ags_play_notation_audio_run_disconnect(AgsConnectable *connectable);
void ags_play_notation_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_play_notation_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable); 
void ags_play_notation_audio_run_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
xmlNode* ags_play_notation_audio_run_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

void ags_play_notation_audio_run_resolve_dependencies(AgsRecall *recall);
AgsRecall* ags_play_notation_audio_run_duplicate(AgsRecall *recall,
						 AgsRecallID *recall_id,
						 guint *n_params, GParameter *parameter);

void ags_play_notation_audio_run_alloc_input_callback(AgsDelayAudioRun *delay_audio_run,
						      guint nth_run,
						      gdouble delay, guint attack,
						      AgsPlayNotationAudioRun *play_notation_audio_run);

void ags_play_notation_audio_run_write_resolve_dependency(AgsFileLookup *file_lookup,
							GObject *recall);
void ags_play_notation_audio_run_read_resolve_dependency(AgsFileLookup *file_lookup,
						       GObject *recall);

/**
 * SECTION:ags_play_notation_audio_run
 * @short_description: play notation
 * @title: AgsPlayNotationAudioRun
 * @section_id:
 * @include: ags/audio/recall/ags_play_notation_audio_run.h
 *
 * The #AgsPlayNotationAudioRun class play notation.
 */

enum{
  PROP_0,
  PROP_DELAY_AUDIO_RUN,
  PROP_COUNT_BEATS_AUDIO_RUN,
};

static gpointer ags_play_notation_audio_run_parent_class = NULL;
static AgsConnectableInterface* ags_play_notation_audio_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_play_notation_audio_run_parent_dynamic_connectable_interface;
static AgsPluginInterface *ags_play_notation_audio_run_parent_plugin_interface;

GType
ags_play_notation_audio_run_get_type()
{
  static GType ags_type_play_notation_audio_run = 0;

  if(!ags_type_play_notation_audio_run){
    static const GTypeInfo ags_play_notation_audio_run_info = {
      sizeof (AgsPlayNotationAudioRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_play_notation_audio_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPlayNotationAudioRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_play_notation_audio_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_notation_audio_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_notation_audio_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_play_notation_audio_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_play_notation_audio_run = g_type_register_static(AGS_TYPE_RECALL_AUDIO_RUN,
							      "AgsPlayNotationAudioRun\0",
							      &ags_play_notation_audio_run_info,
							      0);

    g_type_add_interface_static(ags_type_play_notation_audio_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_notation_audio_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_notation_audio_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return (ags_type_play_notation_audio_run);
}

void
ags_play_notation_audio_run_class_init(AgsPlayNotationAudioRunClass *play_notation_audio_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_play_notation_audio_run_parent_class = g_type_class_peek_parent(play_notation_audio_run);

  /* GObjectClass */
  gobject = (GObjectClass *) play_notation_audio_run;

  gobject->set_property = ags_play_notation_audio_run_set_property;
  gobject->get_property = ags_play_notation_audio_run_get_property;

  gobject->finalize = ags_play_notation_audio_run_finalize;

  /* properties */
  param_spec = g_param_spec_object("delay-audio-run\0",
				   "assigned AgsDelayAudioRun\0",
				   "the AgsDelayAudioRun which emits notation_alloc_input signal\0",
				   AGS_TYPE_DELAY_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DELAY_AUDIO_RUN,
				  param_spec);

  param_spec = g_param_spec_object("count-beats-audio-run\0",
				   "assigned AgsCountBeatsAudioRun\0",
				   "the AgsCountBeatsAudioRun which just counts\0",
				   AGS_TYPE_COUNT_BEATS_AUDIO_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_COUNT_BEATS_AUDIO_RUN,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) play_notation_audio_run;

  recall->resolve_dependencies = ags_play_notation_audio_run_resolve_dependencies;
  recall->duplicate = ags_play_notation_audio_run_duplicate;
}

void
ags_play_notation_audio_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_play_notation_audio_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_play_notation_audio_run_connect;
  connectable->disconnect = ags_play_notation_audio_run_disconnect;
}

void
ags_play_notation_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_play_notation_audio_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_play_notation_audio_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_play_notation_audio_run_disconnect_dynamic;
}

void
ags_play_notation_audio_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_play_notation_audio_run_parent_plugin_interface = g_type_interface_peek_parent(plugin);
  
  plugin->read = ags_play_notation_audio_run_read;
  plugin->write = ags_play_notation_audio_run_write;
}

void
ags_play_notation_audio_run_init(AgsPlayNotationAudioRun *play_notation_audio_run)
{
  AGS_RECALL(play_notation_audio_run)->name = "ags-play-notation\0";
  AGS_RECALL(play_notation_audio_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(play_notation_audio_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(play_notation_audio_run)->xml_type = "ags-play-notation-audio-run\0";
  AGS_RECALL(play_notation_audio_run)->port = NULL;

  play_notation_audio_run->delay_audio_run = NULL;
  play_notation_audio_run->count_beats_audio_run = NULL;

  play_notation_audio_run->notation = NULL;
}

void
ags_play_notation_audio_run_set_property(GObject *gobject,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *param_spec)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      AgsDelayAudioRun *delay_audio_run;
      gboolean is_template;

      delay_audio_run = g_value_get_object(value);

      if(delay_audio_run == play_notation_audio_run->delay_audio_run){
	return;
      }

      if(delay_audio_run != NULL &&
	 (AGS_RECALL_TEMPLATE & (AGS_RECALL(delay_audio_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(play_notation_audio_run->delay_audio_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(play_notation_audio_run),
				       (AgsRecall *) play_notation_audio_run->delay_audio_run);
	}else{
	  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(play_notation_audio_run)->flags)) != 0){
	    g_signal_handler_disconnect(G_OBJECT(play_notation_audio_run),
					play_notation_audio_run->notation_alloc_input_handler);
	  }
	}

	g_object_unref(G_OBJECT(play_notation_audio_run->delay_audio_run));
      }

      if(delay_audio_run != NULL){
	g_object_ref(delay_audio_run);

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(play_notation_audio_run),
				    ags_recall_dependency_new((GObject *) delay_audio_run));
	}else{
	  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(play_notation_audio_run)->flags)) != 0){
	    play_notation_audio_run->notation_alloc_input_handler =
	      g_signal_connect(G_OBJECT(delay_audio_run), "notation-alloc-input\0",
			       G_CALLBACK(ags_play_notation_audio_run_alloc_input_callback), play_notation_audio_run);
	  }
	}
      }

      play_notation_audio_run->delay_audio_run = delay_audio_run;
    }
    break;
  case PROP_COUNT_BEATS_AUDIO_RUN:
    {
      AgsCountBeatsAudioRun *count_beats_audio_run;
      gboolean is_template;

      count_beats_audio_run = g_value_get_object(value);

      if(count_beats_audio_run == play_notation_audio_run->count_beats_audio_run){
	return;
      }

      if(count_beats_audio_run != NULL &&
	 (AGS_RECALL_TEMPLATE & (AGS_RECALL(count_beats_audio_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(play_notation_audio_run->count_beats_audio_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(play_notation_audio_run),
				       (AgsRecall *) play_notation_audio_run->count_beats_audio_run);
	}

	g_object_unref(G_OBJECT(play_notation_audio_run->count_beats_audio_run));
      }

      if(count_beats_audio_run != NULL){
	g_object_ref(count_beats_audio_run);

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(play_notation_audio_run),
				    ags_recall_dependency_new((GObject *) count_beats_audio_run));
	}
      }

      play_notation_audio_run->count_beats_audio_run = count_beats_audio_run;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_play_notation_audio_run_get_property(GObject *gobject,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *param_spec)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;
  
  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(gobject);

  switch(prop_id){
  case PROP_DELAY_AUDIO_RUN:
    {
      g_value_set_object(value, G_OBJECT(play_notation_audio_run->delay_audio_run));
    }
    break;
  case PROP_COUNT_BEATS_AUDIO_RUN:
    {
      g_value_set_object(value, G_OBJECT(play_notation_audio_run->count_beats_audio_run));
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_play_notation_audio_run_finalize(GObject *gobject)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(gobject);

  if(play_notation_audio_run->delay_audio_run != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->delay_audio_run));
  }

  if(play_notation_audio_run->count_beats_audio_run != NULL){
    g_object_unref(G_OBJECT(play_notation_audio_run->count_beats_audio_run));
  }

  G_OBJECT_CLASS(ags_play_notation_audio_run_parent_class)->finalize(gobject);
}

void
ags_play_notation_audio_run_connect(AgsConnectable *connectable)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_play_notation_audio_run_parent_connectable_interface->connect(connectable);

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(connectable);
}

void
ags_play_notation_audio_run_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_play_notation_audio_run_parent_connectable_interface->disconnect(connectable);
}

void
ags_play_notation_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) != 0){
    return;
  }

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(dynamic_connectable);

  /* call parent */
  ags_play_notation_audio_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* connect */
  play_notation_audio_run->notation_alloc_input_handler =
    g_signal_connect(G_OBJECT(play_notation_audio_run->delay_audio_run), "notation-alloc-input\0",
		     G_CALLBACK(ags_play_notation_audio_run_alloc_input_callback), play_notation_audio_run);
  
}

void
ags_play_notation_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  /* call parent */
  ags_play_notation_audio_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(dynamic_connectable);

  if(play_notation_audio_run->delay_audio_run != NULL){
    g_signal_handler_disconnect(G_OBJECT(play_notation_audio_run->delay_audio_run), play_notation_audio_run->notation_alloc_input_handler);
  }
}

void
ags_play_notation_audio_run_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsFileLookup *file_lookup;
  xmlNode *iter;

  /* read parent */
  ags_play_notation_audio_run_parent_plugin_interface->read(file, node, plugin);

  /* read depenendency */
  iter = node->children;

  while(iter != NULL){
    if(iter->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(iter->name,
		     "ags-dependency-list\0",
		     19)){
	xmlNode *dependency_node;

	dependency_node = iter->children;

	while(dependency_node != NULL){
	  if(dependency_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(dependency_node->name,
			   "ags-dependency\0",
			   15)){
	      file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
							   "file\0", file,
							   "node\0", dependency_node,
							   "reference\0", G_OBJECT(plugin),
							   NULL);
	      ags_file_add_lookup(file, (GObject *) file_lookup);
	      g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
			       G_CALLBACK(ags_play_notation_audio_run_read_resolve_dependency), G_OBJECT(plugin));
	    }
	  }
	  
	  dependency_node = dependency_node->next;
	}
      }
    }

    iter = iter->next;
  }
}

xmlNode*
ags_play_notation_audio_run_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  xmlNode *dependency_node;
  GList *list;
  gchar *id;

  /* write parent */
  node = ags_play_notation_audio_run_parent_plugin_interface->write(file, parent, plugin);

  /* write dependencies */
  child = xmlNewNode(NULL,
		     "ags-dependency-list\0");

  xmlNewProp(child,
	     AGS_FILE_ID_PROP,
	     ags_id_generator_create_uuid());

  xmlAddChild(node,
	      child);

  list = AGS_RECALL(plugin)->dependencies;

  while(list != NULL){
    id = ags_id_generator_create_uuid();

    dependency_node = xmlNewNode(NULL,
				 "ags-dependency\0");

    xmlNewProp(dependency_node,
	       AGS_FILE_ID_PROP,
	       id);

    xmlAddChild(child,
		dependency_node);

    file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
						 "file\0", file,
						 "node\0", dependency_node,
						 "reference\0", list->data,
						 NULL);
    ags_file_add_lookup(file, (GObject *) file_lookup);
    g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		     G_CALLBACK(ags_play_notation_audio_run_write_resolve_dependency), G_OBJECT(plugin));

    list = list->next;
  }

  return(node);
}

void
ags_play_notation_audio_run_resolve_dependencies(AgsRecall *recall)
{
  AgsRecall *template;
  AgsRecallContainer *recall_container;
  AgsPlayNotationAudioRun *play_notation_audio_run;
  AgsRecallDependency *recall_dependency;
  AgsDelayAudioRun *delay_audio_run;
  AgsCountBeatsAudioRun *count_beats_audio_run;
  GList *list;
  AgsRecallID *recall_id;
  guint i, i_stop;

  play_notation_audio_run = AGS_PLAY_NOTATION_AUDIO_RUN(recall);
  
  recall_container = AGS_RECALL_CONTAINER(recall->container);
  
  list = ags_recall_find_template(recall_container->recall_audio_run);

  if(list != NULL){
    template = AGS_RECALL(list->data);
  }else{
    g_warning("AgsRecallClass::resolve - missing dependency");
    return;
  }

  list = template->dependencies;
  delay_audio_run = NULL;
  count_beats_audio_run = NULL;
  i_stop = 2;

  for(i = 0; i < i_stop && list != NULL;){
    recall_dependency = AGS_RECALL_DEPENDENCY(list->data);

    if(AGS_IS_DELAY_AUDIO_RUN(recall_dependency->dependency)){
      if(((AGS_RECALL_INPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_INPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0) ||
	 ((AGS_RECALL_OUTPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_OUTPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0)){
	recall_id = recall->recall_id;
      }else{
	recall_id = (AgsRecallID *) recall->recall_id->recycling_context->parent->recall_id;
      }

      delay_audio_run = (AgsDelayAudioRun *) ags_recall_dependency_resolve(recall_dependency, recall_id);

      i++;
    }else if(AGS_IS_COUNT_BEATS_AUDIO_RUN(recall_dependency->dependency)){
      if(((AGS_RECALL_INPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_INPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0) ||
	 ((AGS_RECALL_OUTPUT_ORIENTATED & (recall->flags)) != 0 &&
	  (AGS_RECALL_OUTPUT_ORIENTATED & (AGS_RECALL(recall_dependency->dependency)->flags)) != 0)){
	recall_id = recall->recall_id;
      }else{
	recall_id = (AgsRecallID *) recall->recall_id->recycling_context->parent->recall_id;
      }

      count_beats_audio_run = (AgsCountBeatsAudioRun *) ags_recall_dependency_resolve(recall_dependency, recall_id);

      i++;
    }

    list = list->next;
  }

  g_object_set(G_OBJECT(recall),
	       "delay-audio-run\0", delay_audio_run,
	       "count-beats-audio-run\0", count_beats_audio_run,
	       NULL);
}

AgsRecall*
ags_play_notation_audio_run_duplicate(AgsRecall *recall,
				      AgsRecallID *recall_id,
				      guint *n_params, GParameter *parameter)
{
  AgsPlayNotationAudioRun *copy, *play_notation_audio_run;

  copy = AGS_PLAY_NOTATION_AUDIO_RUN(AGS_RECALL_CLASS(ags_play_notation_audio_run_parent_class)->duplicate(recall,
													   recall_id,
													   n_params, parameter));

  return((AgsRecall *) copy);
}

void
ags_play_notation_audio_run_alloc_input_callback(AgsDelayAudioRun *delay_audio_run,
						 guint nth_run,
						 gdouble delay, guint attack,
						 AgsPlayNotationAudioRun *play_notation_audio_run)
{
  GObject *soundcard;
  AgsAudio *audio;
  AgsChannel *selected_channel, *channel, *next_pad;
  AgsRecycling *recycling;
  AgsAudioSignal *audio_signal;
  AgsNotation *notation;
  AgsNote *note;

  AgsPlayNotationAudio *play_notation_audio;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;
  AgsTimestampThread *timestamp_thread;

  AgsApplicationContext *application_context;
  AgsConfig *config;
  
  GList *current_position;
  GList *list;

  guint audio_channel;
  guint samplerate;
  guint buffer_length;
  guint i;
  gchar *str;
  
  GValue value = {0,};

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;
  pthread_mutex_t *recycling_mutex;
  
  play_notation_audio = AGS_PLAY_NOTATION_AUDIO(AGS_RECALL_AUDIO_RUN(play_notation_audio_run)->recall_audio);

  audio = AGS_RECALL_AUDIO(play_notation_audio)->audio;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  config = ags_config_get_instance();
  
  /* read config and audio mutex */
  pthread_mutex_lock(application_mutex);
  
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "buffer-size\0");
  buffer_length = g_ascii_strtoull(str,
				   NULL,
				   10);
  free(str);

  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "samplerate\0");
  samplerate = g_ascii_strtoull(str,
				NULL,
				10);
  free(str);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  pthread_mutex_lock(audio_mutex);

  soundcard = (GObject *) audio->soundcard;
  list = audio->notation;//(GList *) g_value_get_pointer(&value);
  
  if(list == NULL){
    pthread_mutex_unlock(audio_mutex);

    return;
  }

  //FIXME:JK: nth_run isn't best joice
  audio_channel = AGS_CHANNEL(AGS_RECYCLING(AGS_RECALL(delay_audio_run)->recall_id->recycling)->channel)->audio_channel;
  
  if((AGS_AUDIO_NOTATION_DEFAULT & (audio->flags)) != 0){
    channel = ags_channel_nth(audio->input,
			      audio_channel);
  }else{
    channel = ags_channel_nth(audio->output,
			      audio_channel);
  }

  /*  */
  pthread_mutex_lock(application_mutex);

  soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) soundcard);
  
  pthread_mutex_unlock(application_mutex);

  pthread_mutex_lock(soundcard_mutex);

  application_context = ags_soundcard_get_application_context(AGS_SOUNDCARD(soundcard));

  pthread_mutex_unlock(soundcard_mutex);
  
  /*  */
  pthread_mutex_lock(application_mutex);

  main_loop = application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  timestamp_thread = (AgsTimestampThread *) ags_thread_find_type(main_loop,
								 AGS_TYPE_TIMESTAMP_THREAD);
  
  //TODO:JK: make it advanced
  notation = AGS_NOTATION(g_list_nth(list, audio_channel)->data);//AGS_NOTATION(ags_notation_find_near_timestamp(list, audio_channel,
    //						   timestamp_thread->timestamp)->data);

  current_position = notation->notes; // start_loop
  
  while(current_position != NULL){
    if(current_position != NULL){
      note = AGS_NOTE(current_position->data);
    
      if(note->x[0] == play_notation_audio_run->count_beats_audio_run->notation_counter){
	if((AGS_AUDIO_REVERSE_MAPPING & (audio->flags)) != 0){
	  selected_channel = ags_channel_pad_nth(channel, audio->input_pads - note->y - 1);
	}else{
	  selected_channel = ags_channel_pad_nth(channel, note->y);
	}

	if(selected_channel == NULL){
	  continue;
	}

	/* lookup channel mutex */
	pthread_mutex_unlock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);
	
	pthread_mutex_unlock(application_mutex);

	/* recycling */
	pthread_mutex_lock(channel_mutex);
	
	recycling = selected_channel->first_recycling;

	pthread_mutex_unlock(channel_mutex);
	
	//#ifdef AGS_DEBUG	
	g_message("playing[%u]: %u | %u\n\0", audio_channel, note->x[0], note->y);
	//#endif

	while(recycling != selected_channel->last_recycling->next){
	  /* lookup recycling mutex */
	  pthread_mutex_unlock(application_mutex);

	  recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
						     (GObject *) recycling);
	
	  pthread_mutex_unlock(application_mutex);

	  /* create audio signal */
	  audio_signal = ags_audio_signal_new((GObject *) soundcard,
					      (GObject *) recycling,
					      (GObject *) AGS_RECALL(play_notation_audio_run)->recall_id);

	  if((AGS_AUDIO_PATTERN_MODE & (audio->flags)) != 0){
	    ags_recycling_create_audio_signal_with_defaults(recycling,
							    audio_signal,
							    delay, attack);
	  }else{
	    ags_recycling_create_audio_signal_with_frame_count(recycling,
							       audio_signal,
							       samplerate /  ((double) samplerate / (double) buffer_length) * (note->x[1] - note->x[0]),
							       delay, attack);
	  }
	  
	  ags_audio_signal_connect(audio_signal);

	  audio_signal->stream_current = audio_signal->stream_beginning;
	  
	  ags_recycling_add_audio_signal(recycling,
					 audio_signal);
	  g_object_unref(audio_signal);

	  /* iterate */
	  pthread_mutex_lock(recycling_mutex);

	  recycling = recycling->next;

	  pthread_mutex_unlock(recycling_mutex);
	}
      }else if(note->x[0] > play_notation_audio_run->count_beats_audio_run->notation_counter){
	break;
      }
    }
    
    current_position = current_position->next;
  }

  pthread_mutex_unlock(audio_mutex);
}

void
ags_play_notation_audio_run_write_resolve_dependency(AgsFileLookup *file_lookup,
						     GObject *recall)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file,
							      AGS_RECALL_DEPENDENCY(file_lookup->ref)->dependency);

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "xpath\0",
  	     g_strdup_printf("xpath=//*[@id='%s']\0", id));
}

void
ags_play_notation_audio_run_read_resolve_dependency(AgsFileLookup *file_lookup,
						    GObject *recall)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;

  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "xpath\0");

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  if(AGS_IS_DELAY_AUDIO_RUN(id_ref->ref)){
    g_object_set(G_OBJECT(recall),
		 "delay-audio-run\0", id_ref->ref,
		 NULL);
  }else if(AGS_IS_COUNT_BEATS_AUDIO_RUN(id_ref->ref)){
    g_object_set(G_OBJECT(recall),
		 "count-beats-audio-run\0", id_ref->ref,
		 NULL);
  }
}

/**
 * ags_play_notation_audio_run_new:
 *
 * Creates an #AgsPlayNotationAudioRun
 *
 * Returns: a new #AgsPlayNotationAudioRun
 *
 * Since: 0.4
 */
AgsPlayNotationAudioRun*
ags_play_notation_audio_run_new()
{
  AgsPlayNotationAudioRun *play_notation_audio_run;

  play_notation_audio_run = (AgsPlayNotationAudioRun *) g_object_new(AGS_TYPE_PLAY_NOTATION_AUDIO_RUN,
								     NULL);

  return(play_notation_audio_run);
}