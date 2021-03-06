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

#include <ags/X/ags_machine.h>
#include <ags/X/ags_machine_callbacks.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>
#include <ags/object/ags_marshal.h>
#include <ags/object/ags_plugin.h>

#include <ags/thread/ags_mutex_manager.h>
#include <ags/thread/ags_task_thread.h>
#include <ags/thread/ags_task_completion.h>

#include <ags/file/ags_file.h>
#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>

#include <ags/audio/ags_sound_provider.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_pattern.h>

#include <ags/audio/thread/ags_audio_loop.h>

#include <ags/audio/file/ags_audio_file.h>

#include <ags/audio/task/ags_init_audio.h>
#include <ags/audio/task/ags_append_audio.h>
#include <ags/audio/task/ags_start_soundcard.h>
#include <ags/audio/task/ags_start_sequencer.h>
#include <ags/audio/task/ags_cancel_audio.h>
#include <ags/audio/task/ags_open_file.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_effect_bridge.h>

#include <ags/X/thread/ags_gui_thread.h>

void ags_machine_class_init(AgsMachineClass *machine);
void ags_machine_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_machine_plugin_interface_init(AgsPluginInterface *plugin);
void ags_machine_init(AgsMachine *machine);
void ags_machine_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec);
void ags_machine_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec);
void ags_machine_connect(AgsConnectable *connectable);
void ags_machine_disconnect(AgsConnectable *connectable);
gchar* ags_machine_get_version(AgsPlugin *plugin);
void ags_machine_set_version(AgsPlugin *plugin, gchar *version);
gchar* ags_machine_get_build_id(AgsPlugin *plugin);
void ags_machine_set_build_id(AgsPlugin *plugin, gchar *build_id);
static void ags_machine_finalize(GObject *gobject);
void ags_machine_show(GtkWidget *widget);

void ags_machine_real_resize_audio_channels(AgsMachine *machine,
						  guint new_size, guint old_size);
void ags_machine_real_resize_pads(AgsMachine *machine,
					GType channel_type,
					guint new_size, guint old_size);
void ags_machine_real_map_recall(AgsMachine *machine);
GList* ags_machine_real_find_port(AgsMachine *machine);

GtkMenu* ags_machine_popup_new(AgsMachine *machine);

/**
 * SECTION:ags_machine
 * @short_description: visualize audio object.
 * @title: AgsMachine
 * @section_id:
 * @include: ags/X/ags_machine.h
 *
 * #AgsMachine is a composite widget to act as base class to visualize #AgsAudio.
 */

#define AGS_DEFAULT_MACHINE "ags-default-machine\0"

enum{
  RESIZE_AUDIO_CHANNELS,
  RESIZE_PADS,
  MAP_RECALL,
  FIND_PORT,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_AUDIO,
};

static gpointer ags_machine_parent_class = NULL;
static guint machine_signals[LAST_SIGNAL];

GType
ags_machine_get_type(void)
{
  static GType ags_type_machine = 0;

  if(!ags_type_machine){
    static const GTypeInfo ags_machine_info = {
      sizeof (AgsMachineClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_machine_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMachine),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_machine_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_machine_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_machine_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_machine = g_type_register_static(GTK_TYPE_HANDLE_BOX,
					      "AgsMachine\0", &ags_machine_info,
					      0);
    
    g_type_add_interface_static(ags_type_machine,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_machine,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_machine);
}

void
ags_machine_class_init(AgsMachineClass *machine)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;
  GParamSpec *param_spec;

  ags_machine_parent_class = g_type_class_peek_parent(machine);

  /* GObjectClass */
  gobject = (GObjectClass *) machine;
  
  gobject->set_property = ags_machine_set_property;
  gobject->get_property = ags_machine_get_property;

  gobject->finalize = ags_machine_finalize;

  /* properties */
  /**
   * AgsMachine:audio:
   *
   * The assigned #AgsAudio to visualize.
   * 
   * Since: 0.3
   */
  param_spec = g_param_spec_object("audio\0",
				   "assigned audio\0",
				   "The audio it is assigned to\0",
				   AGS_TYPE_AUDIO,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO,
				  param_spec);

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) machine;

  widget->show = ags_machine_show;

  /* AgsMachineClass */
  machine->resize_pads = ags_machine_real_resize_pads;
  machine->resize_audio_channels = ags_machine_real_resize_audio_channels;
  machine->map_recall = ags_machine_real_map_recall;
  machine->find_port = ags_machine_real_find_port;

  /* signals */
  /**
   * AgsMachine::resize-audio-channels:
   * @machine: the #AgsMachine to modify
   * @channel: the #AgsChannel to set
   * @new_size: the new size
   * @old_size: the old size
   *
   * The ::resize-audio-channels signal notifies about changed channel allocation within
   * audio.
   */
  machine_signals[RESIZE_AUDIO_CHANNELS] =
    g_signal_new("resize-audio-channels\0",
		 G_TYPE_FROM_CLASS(machine),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsMachineClass, resize_audio_channels),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__UINT_UINT,
		 G_TYPE_NONE, 2,
		 G_TYPE_UINT,
		 G_TYPE_UINT);

  /**
   * AgsMachine::resize-pads:
   * @machine: the #AgsMachine to modify
   * @channel: the #AgsChannel to set
   * @channel_type: either %AGS_TYPE_INPUT or %AGS_TYPE_OUTPUT
   * @new_size: the new size
   * @old_size: the old size
   *
   * The ::resize-pads signal notifies about changed channel allocation within
   * audio.
   */
  machine_signals[RESIZE_PADS] =
    g_signal_new("resize-pads\0",
		 G_TYPE_FROM_CLASS(machine),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsMachineClass, resize_pads),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__ULONG_UINT_UINT,
		 G_TYPE_NONE, 3,
		 G_TYPE_ULONG,
		 G_TYPE_UINT,
		 G_TYPE_UINT);

  /**
   * AgsMachine::map-recall:
   * @machine: the #AgsMachine
   *
   * The ::map-recall should be used to add the machine's default recall.
   */
  machine_signals[MAP_RECALL] =
    g_signal_new("map-recall\0",
                 G_TYPE_FROM_CLASS (machine),
                 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsMachineClass, map_recall),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE, 0);

  /**
   * AgsMachine::find-port:
   * @machine: the #AgsMachine to resize
   * Returns: a #GList with associated ports
   *
   * The ::find-port as recall should be mapped
   */
  machine_signals[FIND_PORT] =
    g_signal_new("find-port\0",
		 G_TYPE_FROM_CLASS(machine),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsMachineClass, find_port),
		 NULL, NULL,
		 g_cclosure_user_marshal_POINTER__VOID,
		 G_TYPE_POINTER, 0);
}

void
ags_machine_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_machine_connect;
  connectable->disconnect = ags_machine_disconnect;
}

void
ags_machine_plugin_interface_init(AgsPluginInterface *plugin)
{
  plugin->get_name = NULL;
  plugin->set_name = NULL;
  plugin->get_version = ags_machine_get_version;
  plugin->set_version = ags_machine_set_version;
  plugin->get_build_id = ags_machine_get_build_id;
  plugin->set_build_id = ags_machine_set_build_id;
  plugin->get_xml_type = NULL;
  plugin->set_xml_type = NULL;
  plugin->read = NULL;
  plugin->write = NULL;
  plugin->get_ports = NULL;
}

void
ags_machine_init(AgsMachine *machine)
{
  GtkVBox *vbox;
  GtkFrame *frame;

  machine->name = NULL;

  machine->version = AGS_MACHINE_DEFAULT_VERSION;
  machine->build_id = AGS_MACHINE_DEFAULT_BUILD_ID;

  machine->flags = 0;
  machine->file_input_flags = 0;
  machine->mapping_flags = 0;
  machine->connection_flags = 0;
  
  machine->output_pad_type = G_TYPE_NONE;
  machine->input_pad_type = G_TYPE_NONE;

  machine->bank_0 = 0;
  machine->bank_1 = 0;

  vbox = (GtkVBox *) gtk_vbox_new(FALSE,
				  0);
  
  frame = (GtkFrame *) gtk_frame_new(NULL);
  gtk_container_add((GtkContainer *) machine,
		    (GtkWidget *) frame);

  machine->audio = ags_audio_new(NULL);
  g_object_ref(G_OBJECT(machine->audio));

  machine->audio->flags |= AGS_AUDIO_CAN_NEXT_ACTIVE;
  machine->audio->machine = (GObject *) machine;

  /* AgsAudio */
  g_signal_connect_after(G_OBJECT(machine->audio), "set_audio_channels\0",
			 G_CALLBACK(ags_machine_set_audio_channels_callback), machine);

  g_signal_connect_after(G_OBJECT(machine->audio), "set_pads\0",
			 G_CALLBACK(ags_machine_set_pads_callback), machine);
  
  machine->play = NULL;

  machine->output = NULL;
  machine->selected_output_pad = NULL;

  machine->input = NULL;
  machine->selected_input_pad = NULL;
  
  machine->bridge = NULL;

  machine->port = NULL;
  machine->automation_port = NULL;

  machine->popup = ags_machine_popup_new(machine);
  g_object_ref(machine->popup);
  
  machine->menu_tool_button = (GtkMenuToolButton *) g_object_new(GTK_TYPE_MENU_TOOL_BUTTON,
								 "label\0", "machine\0",
								 "menu\0", machine->popup,
								 NULL);
  gtk_frame_set_label_widget(frame,
			     (GtkWidget *) machine->menu_tool_button);
  machine->properties = NULL;
  machine->rename = NULL;
  machine->connection_editor = NULL;
  machine->midi_dialog = NULL;

  machine->application_context = NULL;
}

void
ags_machine_set_property(GObject *gobject,
			 guint prop_id,
			 const GValue *value,
			 GParamSpec *param_spec)
{
  AgsWindow *window;
  AgsMachine *machine;

  machine = AGS_MACHINE(gobject);
  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) machine,
						 AGS_TYPE_WINDOW);

  switch(prop_id){
  case PROP_AUDIO:
    {
      AgsAudio *audio;
      gboolean reset;

      audio = (AgsAudio *) g_value_get_object(value);
      
      reset = TRUE;

      if(machine->audio != NULL){
	AgsSoundcard *soundcard;
	GList *pad;
	GList *list;

	/* remove from soundcard */
	soundcard = AGS_SOUNDCARD(audio->soundcard);
	
	list = ags_soundcard_get_audio(soundcard);
	list = g_list_remove(list,
			     G_OBJECT(audio));
	ags_soundcard_set_audio(soundcard,
				list);
	audio->soundcard = NULL;
	
	g_object_unref(G_OBJECT(machine->audio));

	if(audio == NULL){
	  /* destroy pad */
	  pad = gtk_container_get_children(machine->output);
	  //	  pad = g_list_nth(pad, audio->output_pads);
	  
	  while(pad != NULL){
	    gtk_widget_destroy(pad->data);

	    pad = pad->next;
	  }

	  pad = gtk_container_get_children(machine->input);
	  //	  pad = g_list_nth(pad, audio->input_pads);
	  
	  while(pad != NULL){
	    gtk_widget_destroy(pad->data);
	    
	    pad = pad->next;
	  }
	  
	  reset = FALSE;
	}
      }
      
      if(audio != NULL){
	g_object_ref(G_OBJECT(audio));
	machine->audio = audio;

	if(reset){
	  AgsChannel *input, *output;
	  GList *pad;
	  GList *line;
	  guint i;

	  /* set channel and resize for AgsOutput */
	  if(machine->output_pad_type != G_TYPE_NONE){
	    output = audio->output;
	    pad = gtk_container_get_children(machine->output);

	    i = 0;

	    while(pad != NULL && output != NULL){
	      line = gtk_container_get_children(GTK_CONTAINER(AGS_PAD(pad->data)->expander_set));

	      ags_pad_resize_lines(AGS_PAD(pad->data), machine->output_line_type,
				   audio->audio_channels, g_list_length(line));
	      g_object_set(G_OBJECT(pad->data),
			   "channel\0", output,
			   NULL);

	      g_list_free(line);
	      
	      output = output->next_pad;
	      pad = pad->next;
	      i++;
	    }

	    if(output != NULL){
	      AgsPad *pad;

	      /* add pad */
	      for(; i < audio->output_pads; i++){
		pad = g_object_new(machine->output_pad_type,
				   "channel\0", output,
				   NULL);
		gtk_container_add(machine->output,
				  GTK_WIDGET(pad));

		ags_pad_resize_lines(pad, machine->output_line_type,
				     audio->audio_channels, 0);
	      }
	    }else{
	      /* destroy pad */
	      pad = gtk_container_get_children(machine->output);
	      pad = g_list_nth(pad, audio->output_pads);

	      while(pad != NULL){
		gtk_widget_destroy(pad->data);

		pad = pad->next;
	      }	      
	    }
	  }

	  /* set channel and resize for AgsOutput */
	  if(machine->input_pad_type != G_TYPE_NONE){
	    input = audio->input;
	    pad = gtk_container_get_children(machine->input);

	    i = 0;

	    while(pad != NULL && input != NULL){
	      line = gtk_container_get_children(GTK_CONTAINER(AGS_PAD(pad->data)->expander_set));

	      ags_pad_resize_lines(AGS_PAD(pad->data), machine->input_line_type,
				   audio->audio_channels, g_list_length(line));
	      g_object_set(G_OBJECT(pad->data),
			   "channel\0", input,
			   NULL);

	      g_list_free(line);

	      input = input->next_pad;
	      pad = pad->next;
	      i++;
	    }

	    if(input != NULL){
	      AgsPad *pad;

	      /* add pad */
	      for(; i < audio->input_pads; i++){
		pad = g_object_new(machine->input_pad_type,
				   "channel\0", input,
				   NULL);
		gtk_container_add(machine->input,
				  GTK_WIDGET(pad));

		ags_pad_resize_lines(pad, machine->input_line_type,
				     audio->audio_channels, 0);
	      }
	    }else{
	      /* destroy pad */
	      pad = gtk_container_get_children(machine->input);
	      pad = g_list_nth(pad, audio->input_pads);

	      while(pad != NULL){
		gtk_widget_destroy(pad->data);

		pad = pad->next;
	      }	      
	    }
	  }
	}else{
	  AgsPad *pad;
	  AgsChannel *channel;
	  guint i;

	  /* add pad */
	  if(machine->output_pad_type != G_TYPE_NONE){
	    channel = audio->output;

	    for(i = 0; i < audio->output_pads; i++){
	      pad = g_object_new(machine->output_pad_type,
				 "channel\0", channel,
				 NULL);
	      gtk_container_add(machine->output,
				GTK_WIDGET(pad));	  
	      ags_pad_resize_lines(pad, machine->output_line_type,
				   audio->audio_channels, 0);

	      channel = channel->next_pad;
	    }
	  }

	  if(machine->input_pad_type != G_TYPE_NONE){
	    channel = audio->input;

	    for(i = 0; i < audio->input_pads; i++){
	      pad = g_object_new(machine->input_pad_type,
				 "channel\0", channel,
				 NULL);
	      gtk_container_add(machine->output,
				GTK_WIDGET(pad));
	      ags_pad_resize_lines(pad, machine->input_line_type,
				   audio->audio_channels, 0);

	      channel = channel->next_pad;
	    }
	  }
	}
      }else{
	machine->audio = NULL;
      }
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_machine_get_property(GObject *gobject,
			 guint prop_id,
			 GValue *value,
			 GParamSpec *param_spec)
{
  AgsMachine *machine;

  machine = AGS_MACHINE(gobject);

  switch(prop_id){
  case PROP_AUDIO:
    g_value_set_object(value, machine->audio);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_machine_connect(AgsConnectable *connectable)
{
  AgsMachine *machine;

  GList *list_start, *list;

  /* AgsMachine */
  machine = AGS_MACHINE(connectable);

  if((AGS_MACHINE_CONNECTED & (machine->flags)) != 0){
    return;
  }

  machine->flags |= AGS_MACHINE_CONNECTED;

  if((AGS_MACHINE_PREMAPPED_RECALL & (machine->flags)) == 0){
    if((AGS_MACHINE_MAPPED_RECALL & (machine->flags)) == 0){
      ags_machine_map_recall(machine);
    }
  }else{
    //    machine->flags &= ~AGS_MACHINE_PREMAPPED_RECALL;
#ifdef AGS_DEBUG
    g_message("find port\0");
#endif
    
    ags_machine_find_port(machine);
  }

  if(machine->bridge != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(machine->bridge));
  }
  
  if(machine->play != NULL){
    g_signal_connect(G_OBJECT(machine->play), "clicked\0",
		     G_CALLBACK(ags_machine_play_callback), (gpointer) machine);
  }

  /* GtkWidget */
  //  g_signal_connect(G_OBJECT (machine), "button_press_event\0",
  //		   G_CALLBACK(ags_machine_button_press_callback), (gpointer) machine);

  /* AgsPad - input */
  if(machine->input != NULL){
    list_start =
      list = gtk_container_get_children(GTK_CONTAINER(machine->input));

    while(list != NULL){
      ags_connectable_connect(AGS_CONNECTABLE(list->data));
      
      list = list->next;
    }

    g_list_free(list_start);
  }

  /* AgsPad - output */
  if(machine->output != NULL){
    list_start =
      list = gtk_container_get_children(GTK_CONTAINER(machine->output));
    
    while(list != NULL){
      ags_connectable_connect(AGS_CONNECTABLE(list->data));
      
      list = list->next;
    }

    g_list_free(list_start);
  }

  /* audio */
  g_signal_connect_after(machine->audio, "tact\0",
			 G_CALLBACK(ags_machine_tact_callback), machine);

  g_signal_connect_after(machine->audio, "done\0",
			 G_CALLBACK(ags_machine_done_callback), machine);
}

void
ags_machine_disconnect(AgsConnectable *connectable)
{
  AgsMachine *machine;

  GList *list_start, *list;

  /* AgsMachine */
  machine = AGS_MACHINE(connectable);

  if((AGS_MACHINE_CONNECTED & (machine->flags)) == 0){
    return;
  }

  machine->flags &= (~AGS_MACHINE_CONNECTED);

  if(machine->bridge != NULL){
    ags_connectable_disconnect(AGS_CONNECTABLE(machine->bridge));
  }

  /* AgsPad - input */
  if(machine->input != NULL){
    list_start =
      list = gtk_container_get_children(GTK_CONTAINER(machine->input));

    while(list != NULL){
      ags_connectable_disconnect(AGS_CONNECTABLE(list->data));
      
      list = list->next;
    }

    g_list_free(list_start);
  }

  /* AgsPad - output */
  if(machine->output != NULL){
    list_start =
      list = gtk_container_get_children(GTK_CONTAINER(machine->output));
    
    while(list != NULL){
      ags_connectable_disconnect(AGS_CONNECTABLE(list->data));
      
      list = list->next;
    }

    g_list_free(list_start);
  }

  //TODO:JK: implement me
  g_signal_handlers_disconnect_by_data(machine->audio,
				       machine);
}


gchar*
ags_machine_get_version(AgsPlugin *plugin)
{
  return(AGS_MACHINE(plugin)->version);
}

void
ags_machine_set_version(AgsPlugin *plugin, gchar *version)
{
  AGS_MACHINE(plugin)->version = version;

  //TODO:JK: implement me
}

gchar*
ags_machine_get_build_id(AgsPlugin *plugin)
{
  return(AGS_MACHINE(plugin)->build_id);
}

void
ags_machine_set_build_id(AgsPlugin *plugin, gchar *build_id)
{
  AGS_MACHINE(plugin)->build_id = build_id;

  //TODO:JK: implement me
}

static void
ags_machine_finalize(GObject *gobject)
{
  AgsMachine *machine;

  GObject *soundcard;
  
  AgsMutexManager *mutex_manager;

  GList *list, *list_start;

  char *str;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;

  machine = (AgsMachine *) gobject;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* lookup audio mutex */
  pthread_mutex_lock(application_mutex);

  soundcard = machine->audio->soundcard;  
  soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) soundcard);
  
  pthread_mutex_unlock(application_mutex);

  /* remove from soundcard */
  if(soundcard_mutex != NULL){
    pthread_mutex_lock(soundcard_mutex);
  }
  
  list = ags_soundcard_get_audio(AGS_SOUNDCARD(soundcard));
  ags_soundcard_set_audio(AGS_SOUNDCARD(soundcard),
			  g_list_remove(list,
					machine->audio));

  if(soundcard_mutex != NULL){
    pthread_mutex_unlock(soundcard_mutex);
  }
  
  //TODO:JK: better clean-up of audio
  
  if(machine->properties != NULL){
    gtk_widget_destroy((GtkWidget *) machine->properties);
  }

  if(machine->rename != NULL){
    gtk_widget_destroy((GtkWidget *) machine->rename);
  }
  
  if(machine->name != NULL){
    g_free(machine->name);
  }
  
  if(machine->audio != NULL){
    g_object_unref(G_OBJECT(machine->audio));
  }

  G_OBJECT_CLASS(ags_machine_parent_class)->finalize(gobject);
}

void
ags_machine_show(GtkWidget *widget)
{
  AgsMachine *machine;
  AgsWindow *window;
  GtkFrame *frame;

  GTK_WIDGET_CLASS(ags_machine_parent_class)->show(widget);

  machine = (AgsMachine *) widget;

  GTK_WIDGET_CLASS(ags_machine_parent_class)->show(widget);

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) widget);

  frame = (GtkFrame *) gtk_container_get_children((GtkContainer *) machine)->data;
  gtk_widget_show_all((GtkWidget *) frame);
}

void
ags_machine_real_resize_audio_channels(AgsMachine *machine,
				       guint audio_channels, guint audio_channels_old)
{
  AgsAudio *audio;
  AgsChannel *channel;

  AgsMutexManager *mutex_manager;

  GList *list_output_pad, *list_output_pad_start;
  GList *list_input_pad, *list_input_pad_start;
  GList *list_output_pad_next, *list_output_pad_next_start;
  GList *list_input_pad_next, *list_input_pad_next_start;

  guint i, j;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;

  audio = machine->audio;
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* lookup audio mutex */
  pthread_mutex_lock(application_mutex);
    
  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  gdk_threads_enter();
  
  if(audio_channels > audio_channels_old){
    /* grow lines */
    AgsPad *pad;

    GList *list, *list_start;

    guint input_pads, output_pads;

    pthread_mutex_lock(audio_mutex);

    input_pads = audio->input_pads;
    output_pads = audio->output_pads;
    
    pthread_mutex_unlock(audio_mutex);

    list_input_pad_start = 
      list_input_pad = g_list_reverse(gtk_container_get_children((GtkContainer *) machine->input));

    list_output_pad_start = 
	list_output_pad = g_list_reverse(gtk_container_get_children((GtkContainer *) machine->output));

    /* AgsInput */
    if(machine->input != NULL){
      /* get input */
      pthread_mutex_lock(audio_mutex);

      channel = audio->input;

      pthread_mutex_unlock(audio_mutex);

      for(i = 0; i < input_pads; i++){
	/* lookup channel mutex */
	pthread_mutex_lock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);
  
	pthread_mutex_unlock(application_mutex);

	/* create AgsPad's if necessary or resize */
	if(audio_channels_old == 0){
	  pad = g_object_new(machine->input_pad_type,
			     "channel\0", channel,
			     NULL);
	  gtk_box_pack_start((GtkBox *) machine->input,
			     (GtkWidget *) pad,
			     FALSE, FALSE,
			     0);

	  ags_pad_resize_lines((AgsPad *) pad, machine->input_line_type,
			       audio_channels, 0);
	}else{
	  pad = AGS_PAD(list_input_pad->data);

	  ags_pad_resize_lines((AgsPad *) pad, machine->input_line_type,
			       audio_channels, audio_channels_old);
	}

	/* iterate */
	pthread_mutex_lock(channel_mutex);
	
	channel = channel->next_pad;
	
	pthread_mutex_unlock(channel_mutex);
	
	if(audio_channels_old != 0){
	  list_input_pad = list_input_pad->next;
	}
      }
    }

    /* AgsOutput */
    if(machine->output != NULL){
      /* get output */
      pthread_mutex_lock(audio_mutex);

      channel = audio->output;

      pthread_mutex_unlock(audio_mutex);

      for(i = 0; i < output_pads; i++){
	/* lookup channel mutex */
	pthread_mutex_lock(application_mutex);

	channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						 (GObject *) channel);
  
	pthread_mutex_unlock(application_mutex);

	/* create AgsPad's if necessary or resize */
	if(audio_channels_old == 0){
	  pad = g_object_new(machine->output_pad_type,
			     "channel\0", channel,
			     NULL);
	  gtk_box_pack_start((GtkBox *) machine->output,
			     (GtkWidget *) pad,
			     FALSE, FALSE,
			     0);
	  ags_pad_resize_lines((AgsPad *) pad, machine->output_line_type,
			       audio_channels, 0);
	}else{
	  pad = AGS_PAD(list_output_pad->data);

	  ags_pad_resize_lines((AgsPad *) pad, machine->output_line_type,
			       audio_channels, audio_channels_old);
	}

	/* iterate */
	pthread_mutex_lock(channel_mutex);
	
	channel = channel->next_pad;
	
	pthread_mutex_unlock(channel_mutex);

	if(audio_channels_old != 0){
	  list_output_pad = list_output_pad->next;
	}
      }
    }

    /* show all */
    if(gtk_widget_get_visible((GtkWidget *) machine)){
      if(audio_channels_old == 0){
	/* AgsInput */
	if(machine->input != NULL){
	  list_input_pad = list_input_pad_start;

	  while(list_input_pad != NULL){
	    gtk_widget_show_all(GTK_WIDGET(list_input_pad->data));

	    list_input_pad = list_input_pad->next;
	  }
	}
	
	/* AgsOutput */
	if(machine->output != NULL){
	  list_output_pad = list_output_pad_start;
	  
	  while(list_input_pad != NULL){
	    gtk_widget_show_all(GTK_WIDGET(list_input_pad->data));
	    
	    list_input_pad = list_input_pad->next;
	  }
	}
      }else{
	if(machine->input != NULL){
	  GList *list_input_line, *list_input_line_start;

	  list_input_pad = list_input_pad_start;

	  while(list_input_pad != NULL){
	    list_input_line_start =
	      list_input_line = g_list_reverse(gtk_container_get_children(GTK_CONTAINER(AGS_PAD(list_input_pad->data)->expander_set)));
	    list_input_line = g_list_nth(list_input_line,
					 audio_channels_old);
	    
	    while(list_input_line != NULL){
	      gtk_widget_show_all(GTK_WIDGET(list_input_line->data));

	      list_input_line = list_input_line->next;
	    }

	    g_list_free(list_input_line_start);
	    
	    list_input_pad = list_input_pad->next;
	  }
	}
	
	/* AgsOutput */
	if(machine->output != NULL){
	  GList *list_output_line, *list_output_line_start;

	  
	  list_output_pad = list_output_pad_start;
	  
	  while(list_output_pad != NULL){
	    list_output_line_start = 
	      list_output_line = g_list_reverse(gtk_container_get_children(GTK_CONTAINER(AGS_PAD(list_output_pad->data)->expander_set)));
	    list_output_line = g_list_nth(list_output_line,
					 audio_channels_old);
	    
	    while(list_output_line != NULL){
	      gtk_widget_show_all(GTK_WIDGET(list_output_line->data));

	      list_output_line = list_output_line->next;
	    }	    
	    
	    list_output_pad = list_output_pad->next;
	  }
	}	
      }
    }
    
    g_list_free(list_input_pad_start);
    g_list_free(list_output_pad_start);
  }else if(audio_channels < audio_channels_old){
    /* shrink lines */
    list_output_pad_start = 
      list_output_pad = gtk_container_get_children((GtkContainer *) machine->output);

    list_input_pad_start = 
      list_input_pad = gtk_container_get_children((GtkContainer *) machine->input);

    if(audio_channels == 0){
      /* AgsInput */
      while(list_input_pad != NULL){
	list_input_pad_next = list_input_pad->next;

	gtk_widget_destroy(GTK_WIDGET(list_input_pad->data));

	list_input_pad = list_input_pad_next;
      }

      /* AgsOutput */
      while(list_output_pad != NULL){
	list_output_pad_next = list_output_pad->next;

	gtk_widget_destroy(GTK_WIDGET(list_output_pad->data));

	list_output_pad = list_output_pad_next;
      }
    }else{
      /* AgsInput */
      for(i = 0; list_input_pad != NULL; i++){
	ags_pad_resize_lines(AGS_PAD(list_input_pad->data), machine->input_pad_type,
			     audio_channels, audio_channels_old);

	list_input_pad = list_input_pad->next;
      }

      /* AgsOutput */
      for(i = 0; list_output_pad != NULL; i++){
	ags_pad_resize_lines(AGS_PAD(list_output_pad->data), machine->output_pad_type,
			     audio_channels, audio_channels_old);

	list_output_pad = list_output_pad->next;
      }
    }

    if(list_output_pad_start){
      g_list_free(list_output_pad_start);
    }
    
    if(list_input_pad_start){
      g_list_free(list_input_pad_start);
    }
  }

  gdk_threads_leave();
}

/**
 * ags_machine_resize_audio_channels:
 * @machine: the #AgsMachine
 * @new_size: new allocation
 * @old_size: old allocation
 *
 * Resize audio channel allocation.
 *
 * Since: 0.7.8
 */
void
ags_machine_resize_audio_channels(AgsMachine *machine,
				  guint new_size,
				  guint old_size)
{
  g_return_if_fail(AGS_IS_MACHINE(machine));

  g_object_ref((GObject *) machine);
  g_signal_emit(G_OBJECT(machine),
		machine_signals[RESIZE_AUDIO_CHANNELS], 0,
		new_size,
		old_size);
  g_object_unref((GObject *) machine);
}

void
ags_machine_real_resize_pads(AgsMachine *machine, GType type,
			     guint pads, guint pads_old)
{
  AgsPad *pad;

  AgsAudio *audio;
  AgsChannel *channel;
  AgsChannel *input, *output;
  
  AgsMutexManager *mutex_manager;

  GList *list_pad;
  
  guint i, j;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *channel_mutex;

  audio = machine->audio;
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* lookup audio mutex */
  pthread_mutex_lock(application_mutex);
    
  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  gdk_threads_enter();
  
  if(pads_old < pads){
    pthread_mutex_lock(audio_mutex);

    input = audio->input;
    output = audio->output;
    
    pthread_mutex_unlock(audio_mutex);

    /* grow input */
    if(machine->input != NULL){
      if(type == AGS_TYPE_INPUT){
	channel = ags_channel_nth(input,
				  pads_old * audio->audio_channels);
      
	for(i = pads_old; i < pads; i++){
	  /* lookup channel mutex */
	  pthread_mutex_lock(application_mutex);

	  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						   (GObject *) channel);
  
	  pthread_mutex_unlock(application_mutex);

	  /* instantiate pad */
	  pad = g_object_new(machine->input_pad_type,
			     "channel\0", channel,
			     NULL);
	  gtk_box_pack_start((GtkBox *) machine->input,
			     (GtkWidget *) pad, FALSE, FALSE, 0);

	  /* resize lines */
	  ags_pad_resize_lines((AgsPad *) pad, machine->input_line_type,
			       audio->audio_channels, 0);
	  
	  /* iterate */
	  pthread_mutex_lock(channel_mutex);
	
	  channel = channel->next_pad;
	
	  pthread_mutex_unlock(channel_mutex);
	}

	/* show all */
	list_pad = gtk_container_get_children(GTK_CONTAINER(machine->input));
	list_pad = g_list_nth(list_pad,
			      pads_old);

	while(list_pad != NULL){
	  gtk_widget_show_all(GTK_WIDGET(list_pad->data));

	  list_pad = list_pad->next;
	}
      }
    }
    
    /* grow output */
    if(machine->output != NULL){
      if(type == AGS_TYPE_OUTPUT){
	channel = ags_channel_nth(output,
				  pads_old * audio->audio_channels);
    
	for(i = pads_old; i < pads; i++){
	  /* lookup channel mutex */
	  pthread_mutex_lock(application_mutex);

	  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
						   (GObject *) channel);
  
	  pthread_mutex_unlock(application_mutex);

	  /* instantiate pad */
	  pad = g_object_new(machine->output_pad_type,
			     "channel\0", channel,
			     NULL);
	  gtk_box_pack_start((GtkBox *) machine->output, (GtkWidget *) pad, FALSE, FALSE, 0);

	  /* resize lines */
	  ags_pad_resize_lines((AgsPad *) pad, machine->output_line_type,
			       audio->audio_channels, 0);

	  /* iterate */
	  pthread_mutex_lock(channel_mutex);
	
	  channel = channel->next_pad;
	
	  pthread_mutex_unlock(channel_mutex);
	}

	/* show all */
	list_pad = gtk_container_get_children(GTK_CONTAINER(machine->output));
	list_pad = g_list_nth(list_pad,
			      pads_old);

	while(list_pad != NULL){
	  gtk_widget_show_all(GTK_WIDGET(list_pad->data));

	  list_pad = list_pad->next;
	}
      }
    }
  }else if(pads_old > pads){
    GList *list, *list_next;

    /* input - destroy AgsPad's */
    if(type == AGS_TYPE_INPUT){
      list = gtk_container_get_children(GTK_CONTAINER(machine->input));
      list = g_list_nth(list, pads);

      while(list != NULL){
	list_next = list->next;

	gtk_widget_destroy(GTK_WIDGET(list->data));

	list = list_next;
      }
    }
    
    /* output - destroy AgsPad's */
    if(type == AGS_TYPE_OUTPUT){
      list = gtk_container_get_children(GTK_CONTAINER(machine->output));
      list = g_list_nth(list, pads);

      while(list != NULL){
	list_next = list->next;

	gtk_widget_destroy(GTK_WIDGET(list->data));

	list = list_next;
      }
    }
  }

  gdk_threads_leave();
}

/**
 * ags_machine_resize_pads:
 * @machine: the #AgsMachine
 * @channel_type: the channel #GType
 * @new_size: new allocation
 * @old_size: old allocation
 *
 * Resize pad allocation.
 *
 * Since: 0.7.8
 */
void
ags_machine_resize_pads(AgsMachine *machine,
			GType channel_type,
			guint new_size,
			guint old_size)
{
  g_return_if_fail(AGS_IS_MACHINE(machine));

  g_object_ref((GObject *) machine);
  g_signal_emit(G_OBJECT(machine),
		machine_signals[RESIZE_PADS], 0,
		channel_type,
		new_size,
		old_size);
  g_object_unref((GObject *) machine);
}

void
ags_machine_real_map_recall(AgsMachine *machine)
{
  if((AGS_MACHINE_MAPPED_RECALL & (machine->flags)) != 0){
    return;
  }

  machine->flags |= AGS_MACHINE_MAPPED_RECALL;

  ags_machine_find_port(machine);
}

/**
 * ags_machine_map_recall:
 * @machine: the #AgsMachine to add its default recall.
 *
 * You may want the @machine to add its default recall.
 */
void
ags_machine_map_recall(AgsMachine *machine)
{
  g_return_if_fail(AGS_IS_MACHINE(machine));

  g_object_ref((GObject *) machine);
  g_signal_emit((GObject *) machine,
		machine_signals[MAP_RECALL], 0);
  g_object_unref((GObject *) machine);
}

GList*
ags_machine_real_find_port(AgsMachine *machine)
{
  GList *pad, *pad_start;
  
  GList *port, *tmp_port;

  port = NULL;

  /* find output ports */
  if(machine->output != NULL){
    pad_start = 
      pad = gtk_container_get_children(machine->output);

    while(pad != NULL){
      tmp_port = ags_pad_find_port(AGS_PAD(pad->data));
      
      if(port != NULL){
	port = g_list_concat(port,
			     tmp_port);
      }else{
	port = tmp_port;
      }

      pad = pad->next;
    }

    g_list_free(pad_start);
  }

  /* find input ports */
  if(machine->input != NULL){
    pad_start = 
      pad = gtk_container_get_children(machine->input);

    while(pad != NULL){
      tmp_port = ags_pad_find_port(AGS_PAD(pad->data));
      
      if(port != NULL){
	port = g_list_concat(port,
			     tmp_port);
      }else{
	port = tmp_port;
      }

      pad = pad->next;
    }

    g_list_free(pad_start);
  }

  /* find bridge ports */
  if(machine->bridge != NULL){
    tmp_port = ags_effect_bridge_find_port((AgsEffectBridge *) machine->bridge);

    if(port != NULL){
      port = g_list_concat(port,
			   tmp_port);
    }else{
      port = tmp_port;
    }
  }
    
  return(port);
}

/**
 * ags_machine_find_port:
 * @machine: the #AgsMachine
 * Returns: an #GList containing all related #AgsPort
 *
 * Lookup ports of associated recalls.
 *
 * Since: 0.4
 */
GList*
ags_machine_find_port(AgsMachine *machine)
{
  GList *list;

  list = NULL;
  g_return_val_if_fail(AGS_IS_MACHINE(machine),
		       NULL);

  g_object_ref((GObject *) machine);
  g_signal_emit((GObject *) machine,
		machine_signals[FIND_PORT], 0,
		&list);
  g_object_unref((GObject *) machine);

  return(list);
}

/**
 * ags_machine_find_by_name:
 * @list: a #GList of #AgsMachine
 * @name: the name of machine
 *
 * Find the specified by @name machine.
 *
 * Returns: the matching #AgsMachine, or %NULL
 *
 * Since: 0.3
 */
AgsMachine*
ags_machine_find_by_name(GList *list, char *name)
{
  while(list != NULL){
    if(!g_strcmp0(AGS_MACHINE(list->data)->name, name))
      return((AgsMachine *) list->data);

    list = list->next;
  }

  return(NULL);
}

/**
 * ags_machine_set_run:
 * @machine: the #AgsMachine
 * @run: if %TRUE playback is started, otherwise stopped
 *
 * Start/stop playback of @machine.
 *
 * Since: 0.4
 */
void
ags_machine_set_run(AgsMachine *machine,
		    gboolean run)
{
  ags_machine_set_run_extended(machine,
			       run,
			       TRUE, TRUE);
}

/**
 * ags_machine_set_run_extended:
 * @machine: the #AgsMachine
 * @run: if %TRUE playback is started, otherwise stopped
 * @sequencer: if doing sequencer
 * @notation: if doing notation
 *
 * Start/stop playback of @machine.
 *
 * Since: 0.4.2
 */
void
ags_machine_set_run_extended(AgsMachine *machine,
			     gboolean run,
			     gboolean sequencer, gboolean notation)
{
  AgsWindow *window;

  AgsMutexManager *mutex_manager;
  AgsAudioLoop *audio_loop;
  AgsTaskThread *task_thread;

  AgsApplicationContext *application_context;

  gboolean no_soundcard;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_loop_mutex;

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) machine);

  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  no_soundcard = FALSE;
  
  pthread_mutex_lock(application_mutex);

  if(ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(application_context)) == NULL){
    no_soundcard = TRUE;
  }

  pthread_mutex_unlock(application_mutex);

  if(no_soundcard){
    g_message("No soundcard available\0");
    
    return;
  }

  /* get threads */
  pthread_mutex_lock(application_mutex);

  audio_loop = (AgsAudioLoop *) application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  /* lookup audio loop mutex */
  pthread_mutex_lock(application_mutex);

  audio_loop_mutex = ags_mutex_manager_lookup(mutex_manager,
					      (GObject *) audio_loop);
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  task_thread = (AgsTaskThread *) ags_thread_find_type((AgsThread *) audio_loop,
						       AGS_TYPE_TASK_THREAD);

  if(run){
    AgsInitAudio *init_audio;
    AgsAppendAudio *append_audio;
    AgsStartSoundcard *start_soundcard;
    AgsStartSequencer *start_sequencer;
    GList *list;

    list = NULL;

    if(sequencer){
      /* create init task */
      init_audio = ags_init_audio_new(machine->audio,
				      FALSE, TRUE, FALSE);
      list = g_list_prepend(list, init_audio);
    
      /* create append task */
      append_audio = ags_append_audio_new((GObject *) audio_loop,
					  (GObject *) machine->audio,
					  FALSE, TRUE, FALSE);

      list = g_list_prepend(list, append_audio);
    }

    if(notation){
      /* create init task */
      init_audio = ags_init_audio_new(machine->audio,
				      FALSE, FALSE, TRUE);
      list = g_list_prepend(list, init_audio);

      /* create append task */
      append_audio = ags_append_audio_new((GObject *) audio_loop,
					  (GObject *) machine->audio,
					  FALSE, FALSE, TRUE);

      list = g_list_prepend(list, append_audio);
    }
    
    /* create start task */
    if(list != NULL){
      AgsGuiThread *gui_thread;
      AgsTaskCompletion *task_completion;

      gui_thread = (AgsGuiThread *) ags_thread_find_type((AgsThread *) audio_loop,
							 AGS_TYPE_GUI_THREAD);

      /* start soundcard */
      start_soundcard = ags_start_soundcard_new(window->application_context);
      list = g_list_prepend(list, start_soundcard);

      /* task completion */
      task_completion = ags_task_completion_new((GObject *) start_soundcard,
						NULL);
      g_signal_connect_after(G_OBJECT(task_completion), "complete\0",
			     G_CALLBACK(ags_machine_start_complete_callback), machine);
      ags_connectable_connect(AGS_CONNECTABLE(task_completion));

      pthread_mutex_lock(gui_thread->task_completion_mutex);
      
      g_atomic_pointer_set(&(gui_thread->task_completion),
			   g_list_prepend(g_atomic_pointer_get(&(gui_thread->task_completion)),
					  task_completion));

      pthread_mutex_unlock(gui_thread->task_completion_mutex);

      /* start sequencer */
      start_sequencer = ags_start_sequencer_new(window->application_context);
      list = g_list_prepend(list, start_sequencer);
      
      /* append AgsStartSoundcard and AgsStartSequencer */
      list = g_list_reverse(list);
      
      ags_task_thread_append_tasks((AgsTaskThread *) task_thread,
				   list);
    }
  }else{
    AgsCancelAudio *cancel_audio;

    /* create cancel task */
    cancel_audio = ags_cancel_audio_new(machine->audio,
					FALSE, sequencer, notation);
    
    /* append AgsCancelAudio */
    ags_task_thread_append_task((AgsTaskThread *) task_thread,
				(AgsTask *) cancel_audio);
  }
}

/**
 * ags_machine_get_possible_audio_output_connections:
 * @machine: the #AgsMachine
 *
 * Find audio output connections suitable for @machine.
 *
 * Returns: a #GtkListStore containing one column with a string representing
 * machines by its type and name.
 *
 * Since: 0.4
 */
GtkListStore*
ags_machine_get_possible_audio_output_connections(AgsMachine *machine)
{
  AgsWindow *window;
  
  AgsApplicationContext *application_context;

  GtkListStore *model;

  GList *list;
  GtkTreeIter iter;

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) machine,
						 AGS_TYPE_WINDOW);

  if(window != NULL){
    application_context = (AgsApplicationContext *) window->application_context;
  }else{
    application_context = NULL;
  }
  
  model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

  gtk_list_store_append(model, &iter);
  gtk_list_store_set(model, &iter,
		     0, "NULL\0",
		     1, NULL,
		     -1);

  if(application_context != NULL){
    list = ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(application_context));

    while(list != NULL){
      if(list->data != machine){
	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter,
			   0, g_strdup_printf("%s: %s\0", 
					      G_OBJECT_TYPE_NAME(G_OBJECT(list->data)),
					      ags_soundcard_get_device(AGS_SOUNDCARD(list->data))),
			   1, list->data,
			   -1);
      }

      list = list->next;
    }
  }
  
  return(model);
}

/**
 * ags_machine_get_possible_links:
 * @machine: the #AgsMachine
 *
 * Find links suitable for @machine.
 *
 * Returns: a #GtkListStore containing one column with a string representing
 * machines by its type and name.
 *
 * Since: 0.4
 */
GtkListStore*
ags_machine_get_possible_links(AgsMachine *machine)
{
  GtkListStore *model;

  GList *list;
  GtkTreeIter iter;

  model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

  gtk_list_store_append(model, &iter);
  gtk_list_store_set(model, &iter,
		     0, "NULL\0",
		     1, NULL,
		     -1);

  if(GTK_WIDGET(machine)->parent != NULL){
    list = gtk_container_get_children(GTK_CONTAINER(GTK_WIDGET(machine)->parent));

    while(list != NULL){
      if(list->data != machine){
	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter,
			   0, g_strdup_printf("%s: %s\0", 
					      G_OBJECT_TYPE_NAME(G_OBJECT(list->data)),
					      AGS_MACHINE(list->data)->name),
			   1, list->data,
			   -1);
      }

      list = list->next;
    }
  }
  
  return(model);
}

/**
 * ags_machine_file_chooser_dialog_new:
 * @machine: the #AgsMachine
 *
 * Creates a new machine file chooser dialog in order to
 * open audio files.
 *
 * Returns: a new #GtkFileChooserDialog
 *
 * Since: 0.4
 */
GtkFileChooserDialog*
ags_machine_file_chooser_dialog_new(AgsMachine *machine)
{
  GtkFileChooserDialog *file_chooser;
  GtkCheckButton *check_button;

  file_chooser = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new(g_strdup("open audio files\0"),
								      (GtkWindow *) gtk_widget_get_toplevel((GtkWidget *) machine),
								      GTK_FILE_CHOOSER_ACTION_OPEN,
								      GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
								      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								      NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser), TRUE);

  check_button = (GtkCheckButton *) gtk_check_button_new_with_label(g_strdup("open in new channel\0"));
  gtk_toggle_button_set_active((GtkToggleButton *) check_button, TRUE);
  gtk_box_pack_start((GtkBox *) GTK_DIALOG(file_chooser)->vbox, (GtkWidget *) check_button, FALSE, FALSE, 0);
  g_object_set_data((GObject *) file_chooser, "create\0", (gpointer) check_button);

  check_button = (GtkCheckButton *) gtk_check_button_new_with_label(g_strdup("overwrite existing links\0"));
  gtk_toggle_button_set_active((GtkToggleButton *) check_button, TRUE);
  gtk_box_pack_start((GtkBox *) GTK_DIALOG(file_chooser)->vbox, (GtkWidget *) check_button, FALSE, FALSE, 0);
  g_object_set_data((GObject *) file_chooser, "overwrite\0", (gpointer) check_button);

  return(file_chooser);
}

/**
 * ags_machine_open_files:
 * @machine: the #AgsMachine
 * @filenames: the filenames
 * @overwrite_channels: reset channels
 * @create_channels: instantiate new channels
 *
 * Opens audio files and modifies or creates new channels if wished.
 *
 * Since: 0.4
 */
void
ags_machine_open_files(AgsMachine *machine,
		       GSList *filenames,
		       gboolean overwrite_channels,
		       gboolean create_channels)
{
  AgsWindow *window;

  AgsOpenFile *open_file;

  AgsMutexManager *mutex_manager;
  AgsAudioLoop *audio_loop;
  AgsTaskThread *task_thread;

  AgsApplicationContext *application_context;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_loop_mutex;

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) machine);
  
  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  audio_loop = (AgsAudioLoop *) application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* lookup audio loop mutex */
  pthread_mutex_lock(application_mutex);
    
  audio_loop_mutex = ags_mutex_manager_lookup(mutex_manager,
					      (GObject *) audio_loop);
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  task_thread = (AgsTaskThread *) ags_thread_find_type((AgsThread *) audio_loop,
						       AGS_TYPE_TASK_THREAD);

  /* instantiate open file task */
  open_file = ags_open_file_new(machine->audio,
				filenames,
				overwrite_channels,
				create_channels);

  ags_task_thread_append_task(task_thread,
			      AGS_TASK(open_file));

}

void
ags_machine_copy_pattern(AgsMachine *machine)
{
  AgsAudio *audio;
  AgsChannel *channel;
  
  AgsMutexManager *mutex_manager;

  xmlDoc *clipboard;
  xmlNode *audio_node, *notation_node;

  xmlChar *buffer;
  int size;
  gint i;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *current_mutex;

  auto xmlNode* ags_machine_copy_pattern_to_notation(AgsChannel *current);

  xmlNode* ags_machine_copy_pattern_to_notation(AgsChannel *current){
    AgsPattern *pattern;
    xmlNode *notation_node, *current_note;
    guint x_boundary, y_boundary;
    guint bank_0, bank_1, k;
    
    /* create root node */
    notation_node = xmlNewNode(NULL, BAD_CAST "notation\0");

    xmlNewProp(notation_node, BAD_CAST "program\0", BAD_CAST "ags\0");
    xmlNewProp(notation_node, BAD_CAST "type\0", BAD_CAST AGS_NOTATION_CLIPBOARD_TYPE);
    xmlNewProp(notation_node, BAD_CAST "version\0", BAD_CAST AGS_NOTATION_CLIPBOARD_VERSION);
    xmlNewProp(notation_node, BAD_CAST "format\0", BAD_CAST AGS_NOTATION_CLIPBOARD_FORMAT);
    xmlNewProp(notation_node, BAD_CAST "base_frequency\0", BAD_CAST g_strdup("0\0"));
    xmlNewProp(notation_node, BAD_CAST "audio-channel\0", BAD_CAST g_strdup_printf("%u\0", current->audio_channel));

    bank_0 = machine->bank_0;
    bank_1 = machine->bank_1;
    
    x_boundary = G_MAXUINT;
    y_boundary = G_MAXUINT;

    while(current != NULL){
      pattern = current->pattern->data;

      for(k = 0; k < pattern->dim[2]; k++){
	if(ags_pattern_get_bit(pattern, bank_0, bank_1, k)){
	  current_note = xmlNewChild(notation_node, NULL, BAD_CAST "note\0", NULL);
	  
	  xmlNewProp(current_note, BAD_CAST "x\0", BAD_CAST g_strdup_printf("%u\0", k));
	  xmlNewProp(current_note, BAD_CAST "x1\0", BAD_CAST g_strdup_printf("%u\0", k + 1));

	  if((AGS_MACHINE_REVERSE_NOTATION & (machine->flags)) != 0){
	    xmlNewProp(current_note, BAD_CAST "y\0", BAD_CAST g_strdup_printf("%u\0", machine->audio->input_pads - current->pad - 1));
	  }else{
	    xmlNewProp(current_note, BAD_CAST "y\0", BAD_CAST g_strdup_printf("%u\0", current->pad));
	  }
	  
	  if(x_boundary > k){
	    x_boundary = k;
	  }
      
	  if((AGS_MACHINE_REVERSE_NOTATION & (machine->flags)) != 0){
	    guint tmp;

	    tmp = machine->audio->input_pads - current->pad - 1;
	    
	    if(y_boundary > tmp){
	      y_boundary = tmp;
	    }
	  }else{
	    if(y_boundary > current->pad){
	      y_boundary = current->pad;
	    }
	  }
	}
      }
      
      current = current->next;
    }

    xmlNewProp(notation_node, BAD_CAST "x_boundary\0", BAD_CAST g_strdup_printf("%u\0", x_boundary));
    xmlNewProp(notation_node, BAD_CAST "y_boundary\0", BAD_CAST g_strdup_printf("%u\0", y_boundary));

    return(notation_node);
  }
  
  /* create document */
  clipboard = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);

  /* create root node */
  audio_node = xmlNewNode(NULL, BAD_CAST "audio\0");
  xmlDocSetRootElement(clipboard, audio_node);

  audio = machine->audio;

  mutex_manager = ags_mutex_manager_get_instance();  
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* lookup audio mutex */
  pthread_mutex_lock(application_mutex);
  
  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) audio);
  
  pthread_mutex_unlock(application_mutex);

  /* copy to clipboard */
  pthread_mutex_lock(audio_mutex);

  channel = audio->input;

  pthread_mutex_unlock(audio_mutex);

  for(i = 0; i < machine->audio->audio_channels; i++){
    /* lookup channel mutex */
    pthread_mutex_lock(application_mutex);

    current_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) channel);
  
    pthread_mutex_unlock(application_mutex);

    /* do it so */
    pthread_mutex_lock(current_mutex);
    
    notation_node = ags_machine_copy_pattern_to_notation(channel);
    xmlAddChild(audio_node, notation_node);

    channel = channel->next;

    pthread_mutex_unlock(current_mutex);
  }
  
  /* write to clipboard */
  xmlDocDumpFormatMemoryEnc(clipboard, &buffer, &size, "UTF-8\0", TRUE);
  gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
			 buffer, size);
  gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
  
  xmlFreeDoc(clipboard);
}

/**
 * ags_machine_new:
 * @soundcard: the assigned soundcard.
 *
 * Creates an #AgsMachine
 *
 * Returns: a new #AgsMachine
 *
 * Since: 0.3
 */
AgsMachine*
ags_machine_new(GObject *soundcard)
{
  AgsMachine *machine;
  GValue value;

  machine = (AgsMachine *) g_object_new(AGS_TYPE_MACHINE,
					NULL);
  
  g_value_init(&value, G_TYPE_OBJECT);
  g_value_set_object(&value, soundcard);
  g_object_set_property(G_OBJECT(machine->audio),
			"soundcard\0", &value);
  g_value_unset(&value);

  return(machine);
}

/**
 * ags_machine_popup_new:
 * @machine: the assigned machine.
 *
 * Creates #GtkMenu to use as @machine's popup context menu.
 *
 * Returns: a new #GtkMenu containing basic actions.
 *
 * Since: 0.3
 */
GtkMenu*
ags_machine_popup_new(AgsMachine *machine)
{
  GtkMenu *popup;
  GtkMenuItem *item;
  GList *list, *list_start;

  popup = (GtkMenu *) gtk_menu_new();

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("move up\0"));
  gtk_menu_shell_append((GtkMenuShell *) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("move down\0"));
  gtk_menu_shell_append((GtkMenuShell *) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("hide\0"));
  gtk_menu_shell_append((GtkMenuShell *) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("show\0"));
  gtk_menu_shell_append((GtkMenuShell *) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("destroy\0"));
  gtk_menu_shell_append((GtkMenuShell *) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("rename\0"));
  gtk_menu_shell_append((GtkMenuShell *) popup, (GtkWidget*) item);
  
  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("properties\0"));
  gtk_menu_shell_append((GtkMenuShell *) popup, (GtkWidget*) item);

  list_start = 
    list = gtk_container_get_children((GtkContainer *) popup);

  g_signal_connect((GObject*) list->data, "activate\0",
		   G_CALLBACK(ags_machine_popup_move_up_activate_callback), (gpointer) machine);
  list = list->next;

  g_signal_connect((GObject*) list->data, "activate\0",
		   G_CALLBACK(ags_machine_popup_move_down_activate_callback), (gpointer) machine);
  list = list->next;

  g_signal_connect((GObject*) list->data, "activate\0",
		   G_CALLBACK(ags_machine_popup_hide_activate_callback), (gpointer) machine);
  list = list->next;

  g_signal_connect((GObject*) list->data, "activate\0",
		   G_CALLBACK(ags_machine_popup_show_activate_callback), (gpointer) machine);
  list = list->next;

  g_signal_connect((GObject*) list->data, "activate\0",
		   G_CALLBACK(ags_machine_popup_destroy_activate_callback), (gpointer) machine);
  list = list->next;

  g_signal_connect((GObject*) list->data, "activate\0",
		   G_CALLBACK(ags_machine_popup_rename_activate_callback), (gpointer) machine);
  list = list->next;

  g_signal_connect((GObject*) list->data, "activate\0",
		   G_CALLBACK(ags_machine_popup_properties_activate_callback), (gpointer) machine);

  g_list_free(list_start);
  gtk_widget_show_all((GtkWidget*) popup);

  return(popup);
}

/**
 * ags_machine_popup_add_edit_options:
 * @machine: the assigned machine.
 * @edit_options: the options to set
 *
 * Add options to edit submenu
 *
 * Since: 0.4.2
 */
void
ags_machine_popup_add_edit_options(AgsMachine *machine, guint edit_options)
{
  GtkMenu *edit;
  GtkMenuItem *item;
  
  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("edit\0"));
  gtk_menu_shell_append((GtkMenuShell *) machine->popup, (GtkWidget*) item);
  gtk_widget_show((GtkWidget *) item);

  edit = (GtkMenu *) gtk_menu_new();
  gtk_menu_item_set_submenu(item,
			    (GtkWidget *) edit);

  gtk_widget_show((GtkWidget *) edit);

  if((AGS_MACHINE_POPUP_COPY_PATTERN & edit_options) != 0){
    item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("copy pattern\0"));
    gtk_menu_shell_append((GtkMenuShell *) edit, (GtkWidget*) item);
    
    g_signal_connect((GObject*) item, "activate\0",
		     G_CALLBACK(ags_machine_popup_copy_pattern_callback), (gpointer) machine);

    gtk_widget_show((GtkWidget *) item);
  }
  
  if((AGS_MACHINE_POPUP_PASTE_PATTERN & edit_options) != 0){
    item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("paste pattern\0"));
    gtk_menu_shell_append((GtkMenuShell *) edit, (GtkWidget*) item);

    g_signal_connect((GObject*) item, "activate\0",
		     G_CALLBACK(ags_machine_popup_paste_pattern_callback), (gpointer) machine);

    gtk_widget_show((GtkWidget *) item);
  }

  gtk_widget_show_all(machine->popup);
}

/**
 * ags_machine_popup_add_connection_options:
 * @machine: the assigned machine.
 * @connection_options: the options to set
 *
 * Add options to connection submenu
 *
 * Since: 0.4.3
 */
void
ags_machine_popup_add_connection_options(AgsMachine *machine, guint connection_options)
{
  GtkMenu *connection;
  GtkMenuItem *item;

  gchar *str;

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("connection\0"));
  gtk_menu_shell_append((GtkMenuShell *) machine->popup, (GtkWidget*) item);
  gtk_widget_show((GtkWidget *) item);

  connection = (GtkMenu *) gtk_menu_new();
  gtk_menu_item_set_submenu(item,
			    (GtkWidget *) connection);

  gtk_widget_show((GtkWidget *) connection);

  if((AGS_MACHINE_POPUP_CONNECTION_EDITOR & connection_options) != 0){
    item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("audio connection\0"));
    gtk_menu_shell_append((GtkMenuShell *) connection, (GtkWidget*) item);
    
    g_signal_connect((GObject*) item, "activate\0",
		     G_CALLBACK(ags_machine_popup_connection_editor_callback), (gpointer) machine);

    gtk_widget_show((GtkWidget *) item);
  }

  if((AGS_MACHINE_POPUP_MIDI_DIALOG & connection_options) != 0){
    item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("MIDI dialog\0"));
    gtk_menu_shell_append((GtkMenuShell *) connection, (GtkWidget*) item);
    
    g_signal_connect((GObject*) item, "activate\0",
		     G_CALLBACK(ags_machine_popup_midi_dialog_callback), (gpointer) machine);

    gtk_widget_show((GtkWidget *) item);
  }

  gtk_widget_show_all(machine->popup);
}
