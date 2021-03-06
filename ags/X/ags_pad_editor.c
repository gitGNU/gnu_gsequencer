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

#include <ags/X/ags_pad_editor.h>
#include <ags/X/ags_pad_editor_callbacks.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_applicable.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/X/ags_line_editor.h>

void ags_pad_editor_class_init(AgsPadEditorClass *pad_editor);
void ags_pad_editor_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_pad_editor_applicable_interface_init(AgsApplicableInterface *applicable);
void ags_pad_editor_init(AgsPadEditor *pad_editor);
void ags_pad_editor_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec);
void ags_pad_editor_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec);
void ags_pad_editor_connect(AgsConnectable *connectable);
void ags_pad_editor_disconnect(AgsConnectable *connectable);
void ags_pad_editor_set_update(AgsApplicable *applicable, gboolean update);
void ags_pad_editor_apply(AgsApplicable *applicable);
void ags_pad_editor_reset(AgsApplicable *applicable);

/**
 * SECTION:ags_pad_editor
 * @short_description: A composite widget to edit #AgsChannel
 * @title: AgsPadEditor
 * @section_id:
 * @include: ags/X/ags_pad_editor.h
 *
 * #AgsPadEditor is a composite widget to edit #AgsChannel. It should be
 * packed by an #AgsPadEditor.
 */

enum{
  PROP_0,
  PROP_CHANNEL,
};

GType
ags_pad_editor_get_type(void)
{
  static GType ags_type_pad_editor = 0;

  if(!ags_type_pad_editor){
    static const GTypeInfo ags_pad_editor_info = {
      sizeof (AgsPadEditorClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_pad_editor_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPadEditor),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_pad_editor_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_pad_editor_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_applicable_interface_info = {
      (GInterfaceInitFunc) ags_pad_editor_applicable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_pad_editor = g_type_register_static(GTK_TYPE_VBOX,
						 "AgsPadEditor\0", &ags_pad_editor_info,
						 0);

    g_type_add_interface_static(ags_type_pad_editor,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_pad_editor,
				AGS_TYPE_APPLICABLE,
				&ags_applicable_interface_info);
  }
  
  return(ags_type_pad_editor);
}

void
ags_pad_editor_class_init(AgsPadEditorClass *pad_editor)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  gobject = (GObjectClass *) pad_editor;

  gobject->set_property = ags_pad_editor_set_property;
  gobject->get_property = ags_pad_editor_get_property;

  param_spec = g_param_spec_object("channel\0",
				   "assigned channel\0",
				   "The channel which this pad editor is assigned with\0",
				   AGS_TYPE_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHANNEL,
				  param_spec);
}

void
ags_pad_editor_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_pad_editor_connect;
  connectable->disconnect = ags_pad_editor_disconnect;
}

void
ags_pad_editor_applicable_interface_init(AgsApplicableInterface *applicable)
{
  applicable->set_update = ags_pad_editor_set_update;
  applicable->apply = ags_pad_editor_apply;
  applicable->reset = ags_pad_editor_reset;
}

void
ags_pad_editor_init(AgsPadEditor *pad_editor)
{
  pad_editor->version = AGS_PAD_EDITOR_DEFAULT_VERSION;
  pad_editor->build_id = AGS_PAD_EDITOR_DEFAULT_BUILD_ID;

  pad_editor->line_editor_expander = (GtkExpander *) gtk_expander_new(NULL);
  gtk_box_pack_start(GTK_BOX(pad_editor),
		     GTK_WIDGET(pad_editor->line_editor_expander),
		     FALSE, FALSE,
		     0);

  pad_editor->line_editor = NULL;
}


void
ags_pad_editor_set_property(GObject *gobject,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *param_spec)
{
  AgsPadEditor *pad_editor;

  pad_editor = AGS_PAD_EDITOR(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    {
      AgsChannel *channel;

      channel = (AgsChannel *) g_value_get_object(value);

      ags_pad_editor_set_channel(pad_editor, channel);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_pad_editor_get_property(GObject *gobject,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *param_spec)
{
  AgsPadEditor *pad_editor;

  pad_editor = AGS_PAD_EDITOR(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    g_value_set_object(value, pad_editor->pad);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_pad_editor_connect(AgsConnectable *connectable)
{
  AgsPadEditor *pad_editor;
  AgsAudio *audio;
  GList *line_editor, *line_editor_start;

  pad_editor = AGS_PAD_EDITOR(connectable);

  /* AgsAudio */
  audio = AGS_AUDIO(pad_editor->pad->audio);

  pad_editor->set_audio_channels_handler = g_signal_connect_after(G_OBJECT(audio), "set_audio_channels\0",
								  G_CALLBACK(ags_pad_editor_set_audio_channels_callback), pad_editor);

  /* AgsLineEditor */
  line_editor_start = 
    line_editor = gtk_container_get_children(GTK_CONTAINER(pad_editor->line_editor));

  while(line_editor != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(line_editor->data));

    line_editor = line_editor->next;
  }

  g_list_free(line_editor_start);
}

void
ags_pad_editor_disconnect(AgsConnectable *connectable)
{
  AgsPadEditor *pad_editor;
  AgsAudio *audio;

  pad_editor = AGS_PAD_EDITOR(connectable);

  /* AgsAudio */
  audio = AGS_AUDIO(pad_editor->pad->audio);
  
  g_signal_handler_disconnect(audio,
			      pad_editor->set_audio_channels_handler);
}

void
ags_pad_editor_set_update(AgsApplicable *applicable, gboolean update)
{
  AgsPadEditor *pad_editor;
  GList *line_editor, *line_editor_start;

  pad_editor = AGS_PAD_EDITOR(applicable);

  line_editor_start = 
    line_editor = gtk_container_get_children(GTK_CONTAINER(pad_editor->line_editor));

  while(line_editor != NULL){
    ags_applicable_set_update(AGS_APPLICABLE(line_editor->data), update);

    line_editor = line_editor->next;
  }

  g_list_free(line_editor_start);
}

void
ags_pad_editor_apply(AgsApplicable *applicable)
{
  AgsPadEditor *pad_editor;
  GList *line_editor, *line_editor_start;

  pad_editor = AGS_PAD_EDITOR(applicable);

  line_editor_start = 
    line_editor = gtk_container_get_children(GTK_CONTAINER(pad_editor->line_editor));

  while(line_editor != NULL){
    ags_applicable_apply(AGS_APPLICABLE(line_editor->data));

    line_editor = line_editor->next;
  }

  g_list_free(line_editor_start);
}

void
ags_pad_editor_reset(AgsApplicable *applicable)
{
  AgsPadEditor *pad_editor;
  GList *line_editor, *line_editor_start;

  pad_editor = AGS_PAD_EDITOR(applicable);

  line_editor_start = 
    line_editor = gtk_container_get_children(GTK_CONTAINER(pad_editor->line_editor));

  while(line_editor != NULL){
    ags_applicable_reset(AGS_APPLICABLE(line_editor->data));

    line_editor = line_editor->next;
  }

  g_list_free(line_editor_start);
}

/**
 * ags_pad_editor_set_channel:
 * @pad_editor: an #AgsPadEditor
 * @channel: the new #AgsChannel
 *
 * Is called as channel gets modified.
 *
 * Since: 0.3
 */
void
ags_pad_editor_set_channel(AgsPadEditor *pad_editor, AgsChannel *channel)
{
  GtkVBox *vbox;
  
  if(pad_editor->line_editor != NULL){
    vbox = pad_editor->line_editor;
    pad_editor->line_editor = NULL;
    gtk_widget_destroy(GTK_WIDGET(vbox));
  }

  pad_editor->pad = channel;
  
  if(channel != NULL){
    AgsLineEditor *line_editor;
    
    AgsChannel *next_pad;

    AgsMutexManager *mutex_manager;

    guint pad;
    guint i;

    pthread_mutex_t *application_mutex;
    pthread_mutex_t *channel_mutex;
    
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    /* lookup channel mutex */
    pthread_mutex_lock(application_mutex);

    channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					     channel);
    
    pthread_mutex_unlock(application_mutex);

    /* get some channel fields */
    pthread_mutex_lock(channel_mutex);
    
    next_pad = channel->next_pad;
    pad = channel->pad;

    pthread_mutex_unlock(channel_mutex);

    /* set label */
    gtk_expander_set_label(pad_editor->line_editor_expander,
			   g_strdup_printf("pad: %u\0", pad));

    pad_editor->line_editor = (GtkVBox *) gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(pad_editor->line_editor_expander),
		      GTK_WIDGET(pad_editor->line_editor));

    while(channel != next_pad){
      /* lookup channel mutex */
      pthread_mutex_lock(application_mutex);

      channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					       channel);
    
      pthread_mutex_unlock(application_mutex);
      
      /* instantiate line editor */
      line_editor = ags_line_editor_new(NULL);
      line_editor->editor_type_count = pad_editor->editor_type_count;
      line_editor->editor_type = (GType *) malloc(line_editor->editor_type_count * sizeof(GType));
      
      for(i = 0; i < line_editor->editor_type_count; i++){
	line_editor->editor_type[i] = pad_editor->editor_type[i];
      }

      g_object_set(line_editor,
		   "channel\0", channel,
		   NULL);
      
      gtk_box_pack_start(GTK_BOX(pad_editor->line_editor),
			 GTK_WIDGET(line_editor),
			 FALSE, FALSE,
			 0);

      /* iterate */
      pthread_mutex_lock(channel_mutex);
      
      channel = channel->next;

      pthread_mutex_unlock(channel_mutex);
    }
  }else{
    gtk_expander_set_label(pad_editor->line_editor_expander,
			   NULL);
  }
}

/**
 * ags_pad_editor_new:
 * @channel: the channel to edit
 *
 * Creates an #AgsPadEditor
 *
 * Returns: a new #AgsPadEditor
 *
 * Since: 0.3
 */
AgsPadEditor*
ags_pad_editor_new(AgsChannel *channel)
{
  AgsPadEditor *pad_editor;

  pad_editor = (AgsPadEditor *) g_object_new(AGS_TYPE_PAD_EDITOR,
					     "channel\0", channel,
					     NULL);

  return(pad_editor);
}
