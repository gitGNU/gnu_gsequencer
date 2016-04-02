/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2015 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags/X/ags_effect_line.h>
#include <ags/X/ags_effect_line_callbacks.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_marshal.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_plugin.h>

#include <ags/plugin/ags_ladspa_manager.h>
#include <ags/plugin/ags_lv2_manager.h>

#ifdef AGS_USE_LINUX_THREADS
#include <ags/thread/ags_thread-kthreads.h>
#else
#include <ags/thread/ags_thread-posix.h>
#endif 
#include <ags/thread/ags_task_thread.h>

#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_ladspa.h>

#include <ags/widget/ags_dial.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_line_member.h>

#include <ags/X/task/ags_add_line_member.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <ladspa.h>

void ags_effect_line_class_init(AgsEffectLineClass *effect_line);
void ags_effect_line_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_effect_line_plugin_interface_init(AgsPluginInterface *plugin);
void ags_effect_line_init(AgsEffectLine *effect_line);
void ags_effect_line_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_effect_line_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_effect_line_connect(AgsConnectable *connectable);
void ags_effect_line_disconnect(AgsConnectable *connectable);
gchar* ags_effect_line_get_name(AgsPlugin *plugin);
void ags_effect_line_set_name(AgsPlugin *plugin, gchar *name);
gchar* ags_effect_line_get_version(AgsPlugin *plugin);
void ags_effect_line_set_version(AgsPlugin *plugin, gchar *version);
gchar* ags_effect_line_get_build_id(AgsPlugin *plugin);
void ags_effect_line_set_build_id(AgsPlugin *plugin, gchar *build_id);

GList* ags_effect_line_add_ladspa_effect(AgsEffectLine *effect_line,
					 gchar *filename,
					 gchar *effect);
GList* ags_effect_line_add_lv2_effect(AgsEffectLine *effect_line,
				      gchar *filename,
				      gchar *effect);
GList* ags_effect_line_real_add_effect(AgsEffectLine *effect_line,
				       gchar *filename,
				       gchar *effect);
void ags_effect_line_real_remove_effect(AgsEffectLine *effect_line,
					guint nth);
void ags_effect_line_real_map_recall(AgsEffectLine *effect_line);
GList* ags_effect_line_real_find_port(AgsEffectLine *effect_line);

/**
 * SECTION:ags_effect_line
 * @short_description: A composite widget to visualize a bunch of #AgsChannel
 * @title: AgsEffectLine
 * @section_id:
 * @include: ags/X/ags_effect_line.h
 *
 * #AgsEffectLine is a composite widget to visualize one #AgsChannel. It should be
 * packed by an #AgsEffectLine.
 */

enum{
  ADD_EFFECT,
  REMOVE_EFFECT,
  MAP_RECALL,
  FIND_PORT,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_CHANNEL,
};

static gpointer ags_effect_line_parent_class = NULL;
static guint effect_line_signals[LAST_SIGNAL];

GType
ags_effect_line_get_type(void)
{
  static GType ags_type_effect_line = 0;

  if(!ags_type_effect_line){
    static const GTypeInfo ags_effect_line_info = {
      sizeof(AgsEffectLineClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_effect_line_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsEffectLine),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_effect_line_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_effect_line_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_effect_line_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_effect_line = g_type_register_static(GTK_TYPE_VBOX,
						  "AgsEffectLine\0", &ags_effect_line_info,
						  0);

    g_type_add_interface_static(ags_type_effect_line,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_effect_line,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_effect_line);
}

void
ags_effect_line_class_init(AgsEffectLineClass *effect_line)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_effect_line_parent_class = g_type_class_peek_parent(effect_line);

  /* GObjectClass */
  gobject = G_OBJECT_CLASS(effect_line);

  gobject->set_property = ags_effect_line_set_property;
  gobject->get_property = ags_effect_line_get_property;

  /* properties */
  /**
   * AgsEffectLine:channel:
   *
   * The start of a bunch of #AgsChannel to visualize.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("channel\0",
				   "assigned channel\0",
				   "The channel it is assigned with\0",
				   AGS_TYPE_CHANNEL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHANNEL,
				  param_spec);

  /* AgsEffectLineClass */
  effect_line->add_effect = ags_effect_line_real_add_effect;
  effect_line->remove_effect = ags_effect_line_real_remove_effect;
  effect_line->map_recall = ags_effect_line_real_map_recall;
  effect_line->find_port = ags_effect_line_real_find_port;

  /* signals */
  /**
   * AgsEffectLine::add-effect:
   * @effect_line: the #AgsEffectLine to modify
   * @effect: the effect's name
   *
   * The ::add-effect signal notifies about added effect.
   */
  effect_line_signals[ADD_EFFECT] =
    g_signal_new("add-effect\0",
		 G_TYPE_FROM_CLASS(effect_line),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsEffectLineClass, add_effect),
		 NULL, NULL,
		 g_cclosure_user_marshal_POINTER__STRING_STRING,
		 G_TYPE_POINTER, 2,
		 G_TYPE_STRING,
		 G_TYPE_STRING);

  /**
   * AgsEffectLine::remove-effect:
   * @effect_line: the #AgsEffectLine to modify
   * @nth: the nth effect
   *
   * The ::remove-effect signal notifies about removed effect.
   */
  effect_line_signals[REMOVE_EFFECT] =
    g_signal_new("remove-effect\0",
		 G_TYPE_FROM_CLASS(effect_line),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsEffectLineClass, remove_effect),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);

  /**
   * AgsEffectLine::map-recall:
   * @effect_line: the #AgsEffectLine
   *
   * The ::map-recall should be used to add the effect_line's default recall.
   */
  effect_line_signals[MAP_RECALL] =
    g_signal_new("map-recall\0",
                 G_TYPE_FROM_CLASS (effect_line),
                 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsEffectLineClass, map_recall),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__UINT,
                 G_TYPE_NONE, 0);

  /**
   * AgsEffectLine::find-port:
   * @effect_line: the #AgsEffectLine to resize
   * Returns: a #GList with associated ports
   *
   * The ::find-port as recall should be mapped
   */
  effect_line_signals[FIND_PORT] =
    g_signal_new("find-port\0",
		 G_TYPE_FROM_CLASS(effect_line),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsEffectLineClass, find_port),
		 NULL, NULL,
		 g_cclosure_user_marshal_POINTER__VOID,
		 G_TYPE_POINTER, 0);
}

void
ags_effect_line_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_effect_line_connect;
  connectable->disconnect = ags_effect_line_disconnect;
}

void
ags_effect_line_plugin_interface_init(AgsPluginInterface *plugin)
{
  plugin->get_name = NULL;
  plugin->set_name = NULL;
  plugin->get_version = ags_effect_line_get_version;
  plugin->set_version = ags_effect_line_set_version;
  plugin->get_build_id = ags_effect_line_get_build_id;
  plugin->set_build_id = ags_effect_line_set_build_id;
  plugin->get_xml_type = NULL;
  plugin->set_xml_type = NULL;
  plugin->get_ports = NULL;
  plugin->read = NULL;
  plugin->write = NULL;
  plugin->set_ports = NULL;
}

void
ags_effect_line_init(AgsEffectLine *effect_line)
{
  effect_line->flags = 0;

  effect_line->name = NULL;
  
  effect_line->version = AGS_EFFECT_LINE_DEFAULT_VERSION;
  effect_line->build_id = AGS_EFFECT_LINE_DEFAULT_BUILD_ID;

  effect_line->channel = NULL;

  effect_line->label = g_object_new(GTK_TYPE_LABEL,
				    NULL);
  gtk_box_pack_start(GTK_BOX(effect_line),
		     GTK_WIDGET(effect_line->label),
		     FALSE, FALSE,
		     0);

  effect_line->table = gtk_table_new(1, AGS_EFFECT_LINE_COLUMNS_COUNT,
				    TRUE);
  gtk_box_pack_start(GTK_BOX(effect_line),
		     GTK_WIDGET(effect_line->table),
		     FALSE, FALSE,
		     0);
}

void
ags_effect_line_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsEffectLine *effect_line;

  effect_line = AGS_EFFECT_LINE(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    {
      AgsChannel *channel;

      channel = (AgsChannel *) g_value_get_object(value);

      if(effect_line->channel == channel){
	return;
      }

      if(effect_line->channel != NULL){
	g_object_unref(effect_line->channel);
      }

      if(channel != NULL){
	g_object_ref(channel);
      }

      effect_line->channel = channel;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_effect_line_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  AgsEffectLine *effect_line;

  effect_line = AGS_EFFECT_LINE(gobject);

  switch(prop_id){
  case PROP_CHANNEL:
    {
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_effect_line_connect(AgsConnectable *connectable)
{
  AgsEffectLine *effect_line;
  GList *list, *list_start;

  effect_line = AGS_EFFECT_LINE(connectable);

  if((AGS_EFFECT_LINE_CONNECTED & (effect_line->flags)) != 0){
    return;
  }
  
  /* channel */
  g_signal_connect_after((GObject *) effect_line->channel, "add-effect\0",
			 G_CALLBACK(ags_effect_line_add_effect_callback), effect_line);

  /* connect line members */
  list_start = 
    list = gtk_container_get_children(GTK_CONTAINER(effect_line->table));
  
  while(list != NULL){
    if(AGS_IS_CONNECTABLE(list->data)){
      ags_connectable_connect(AGS_CONNECTABLE(list->data));
    }

    list = list->next;
  }

  if(list_start != NULL){
    g_list_free(list_start);
  }
}

void
ags_effect_line_disconnect(AgsConnectable *connectable)
{
  AgsEffectLine *effect_line;

  effect_line = AGS_EFFECT_LINE(connectable);

  if((AGS_EFFECT_LINE_CONNECTED & (effect_line->flags)) == 0){
    return;
  }

  //TODO:JK: implement me
}

gchar*
ags_effect_line_get_name(AgsPlugin *plugin)
{
  return(AGS_EFFECT_LINE(plugin)->name);
}

void
ags_effect_line_set_name(AgsPlugin *plugin, gchar *name)
{
  AgsEffectLine *effect_line;

  effect_line = AGS_EFFECT_LINE(plugin);

  effect_line->name = name;
}

gchar*
ags_effect_line_get_version(AgsPlugin *plugin)
{
  return(AGS_EFFECT_LINE(plugin)->version);
}

void
ags_effect_line_set_version(AgsPlugin *plugin, gchar *version)
{
  AgsEffectLine *effect_line;

  effect_line = AGS_EFFECT_LINE(plugin);

  effect_line->version = version;
}

gchar*
ags_effect_line_get_build_id(AgsPlugin *plugin)
{
  return(AGS_EFFECT_LINE(plugin)->build_id);
}

void
ags_effect_line_set_build_id(AgsPlugin *plugin, gchar *build_id)
{
  AgsEffectLine *effect_line;

  effect_line = AGS_EFFECT_LINE(plugin);

  effect_line->build_id = build_id;
}

GList*
ags_effect_line_add_ladspa_effect(AgsEffectLine *effect_line,
				  gchar *filename,
				  gchar *effect)
{
  AgsLineMember *line_member;
  AgsAddLineMember *add_line_member;
  GtkAdjustment *adjustment;

  AgsLadspaPlugin *ladspa_plugin;
  
  GList *list, *list_start;
  GList *recall, *recall_start;
  GList *port, *recall_port;
  GList *port_descriptor;
  
  gdouble step;
  guint port_count;
  guint x, y;
  guint k;
  
  /* load plugin */
  ladspa_plugin = ags_ladspa_manager_find_ladspa_plugin(filename, effect);

  /* retrieve position within table  */
  x = 0;
  y = 0;
  
  list_start = 
    list = effect_line->table->children;

  while(list != NULL){
    if(y <= ((GtkTableChild *) list->data)->top_attach){
      y = ((GtkTableChild *) list->data)->top_attach + 1;
    }

    list = list->next;
  }

  /* find ports */
  recall_start =
    recall = ags_recall_get_by_effect(effect_line->channel->play,
				      filename,
				      effect);
  recall = g_list_last(recall);
  port = AGS_RECALL(recall->data)->port;

  g_list_free(recall_start);

  recall_start = 
    recall = ags_recall_get_by_effect(effect_line->channel->recall,
				      filename,
				      effect);
  recall = g_list_last(recall);

  recall_port = AGS_RECALL(recall->data)->port;
  g_list_free(recall_start);

  /* load ports */
  port_descriptor = AGS_BASE_PLUGIN(ladspa_plugin)->port;

  port_count = g_list_length(port_descriptor);
  k = 0;

  while(port_descriptor != NULL){
    if((AGS_PORT_DESCRIPTOR_CONTROL & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
      GtkWidget *child_widget;
      
      GType widget_type;
      
      if(x == AGS_EFFECT_LINE_COLUMNS_COUNT){
	x = 0;
	y++;
	gtk_table_resize(effect_line->table,
			 y + 1, AGS_EFFECT_LINE_COLUMNS_COUNT);
      }
      
      if((AGS_PORT_DESCRIPTOR_TOGGLED & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	widget_type = GTK_TYPE_TOGGLE_BUTTON;
      }else{
	widget_type = AGS_TYPE_DIAL;
      }

      /* add line member */
      line_member = (AgsLineMember *) g_object_new(AGS_TYPE_LINE_MEMBER,
						   "widget-type\0", widget_type,
						   "widget-label\0", AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_name,
						   "plugin-name\0", g_strdup_printf("ladspa-%lu\0", ladspa_plugin->unique_id),
						   "filename\0", filename,
						   "effect\0", effect,
						   "specifier\0", g_strdup(AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_name),
						   "control-port\0", g_strdup_printf("%d/%d\0",
										     k,
										     port_count),
						   NULL);
      child_widget = ags_line_member_get_widget(line_member);
      
      if(AGS_IS_DIAL(child_widget)){
	AgsDial *dial;
	GtkAdjustment *adjustment;
	float lower_bound, upper_bound;
	
	dial = child_widget;

	/* add controls of ports and apply range  */
	lower_bound = g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->lower_value);
	upper_bound = g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->upper_value);

	adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 0.1, 0.1, 0.0);
	g_object_set(dial,
		     "adjustment", adjustment,
		     NULL);

	if(upper_bound >= 0.0 && lower_bound >= 0.0){
	  step = (upper_bound - lower_bound) / AGS_DIAL_DEFAULT_PRECISION;
	}else if(upper_bound < 0.0 && lower_bound < 0.0){
	  step = -1.0 * (lower_bound - upper_bound) / AGS_DIAL_DEFAULT_PRECISION;
	}else{
	  step = (upper_bound - lower_bound) / AGS_DIAL_DEFAULT_PRECISION;
	}

	gtk_adjustment_set_step_increment(adjustment,
					  step);
	gtk_adjustment_set_lower(adjustment,
				 lower_bound);
	gtk_adjustment_set_upper(adjustment,
				 upper_bound);
	gtk_adjustment_set_value(adjustment,
				 g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->default_value));
      }

#ifdef AGS_DEBUG
      g_message("ladspa bounds: %f %f\0", lower_bound, upper_bound);
#endif
	  
      gtk_table_attach(effect_line->table,
		       line_member,
		       x, x + 1,
		       y, y + 1,
		       GTK_FILL, GTK_FILL,
		       0, 0);
      
      port = port->next;
      x++;
    }

    ags_connectable_connect(AGS_CONNECTABLE(line_member));
    gtk_widget_show_all(line_member);
    
    port_descriptor = port_descriptor->next;
    k++;
  }
  
  return(port);
}

GList*
ags_effect_line_add_lv2_effect(AgsEffectLine *effect_line,
			       gchar *filename,
			       gchar *effect)
{
  AgsLineMember *line_member;
  AgsAddLineMember *add_line_member;
  GtkAdjustment *adjustment;

  AgsLv2Plugin *lv2_plugin;

  GList *list, *list_start;
  GList *recall, *recall_start;
  GList *port, *recall_port;
  GList *port_descriptor;
  
  gdouble step;
  guint port_count;
  guint x, y;
  guint k;
  
  /* load plugin */
  lv2_plugin = ags_lv2_manager_find_lv2_plugin(filename, effect);

  /* retrieve position within table  */
  x = 0;
  y = 0;

  list_start = 
    list = effect_line->table->children;

  while(list != NULL){
    if(y <= ((GtkTableChild *) list->data)->top_attach){
      y = ((GtkTableChild *) list->data)->top_attach + 1;
    }

    list = list->next;
  }

  /* find ports */
  recall_start =
    recall = ags_recall_get_by_effect(effect_line->channel->play,
				      filename,
				      effect);
  recall = g_list_last(recall);
  port = AGS_RECALL(recall->data)->port;

  g_list_free(recall_start);

  recall_start = 
    recall = ags_recall_get_by_effect(effect_line->channel->recall,
				      filename,
				      effect);
  recall = g_list_last(recall);

  recall_port = AGS_RECALL(recall->data)->port;
  g_list_free(recall_start);


  /* load ports */
  port_descriptor = AGS_BASE_PLUGIN(lv2_plugin)->port;

  port_count = g_list_length(port_descriptor);
  k = 0;
  
  while(port_descriptor != NULL &&
	port != NULL){
    if((AGS_PORT_DESCRIPTOR_CONTROL & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
      GtkWidget *child_widget;
	  
      GType widget_type;

      if(x == AGS_EFFECT_LINE_COLUMNS_COUNT){
	x = 0;
	y++;
	gtk_table_resize(effect_line->table,
			 y + 1, AGS_EFFECT_LINE_COLUMNS_COUNT);
      }

      if((AGS_PORT_DESCRIPTOR_TOGGLED & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	widget_type = GTK_TYPE_TOGGLE_BUTTON;
      }else{
	widget_type = AGS_TYPE_DIAL;
      }

      /* add line member */
      line_member = (AgsLineMember *) g_object_new(AGS_TYPE_LINE_MEMBER,
						   "widget-type\0", widget_type,
						   "widget-label\0", AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_name,
						   "plugin-name\0", g_strdup_printf("lv2-<%s>\0", lv2_plugin->uri),
						   "filename\0", filename,
						   "effect\0", effect,
						   "specifier\0", g_strdup(AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_name),
						   "control-port\0", g_strdup_printf("%d/%d\0",
										     k,
										     port_count),
						   NULL);
      child_widget = ags_line_member_get_widget(line_member);

      if(AGS_IS_DIAL(child_widget)){
	AgsDial *dial;
	GtkAdjustment *adjustment;

	float lower_bound, upper_bound;
	
	dial = child_widget;

	/* add controls of ports and apply range  */
	lower_bound = g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->lower_value);
	upper_bound = g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->upper_value);

	adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 0.1, 0.1, 0.0);
	g_object_set(dial,
		     "adjustment", adjustment,
		     NULL);

	if(upper_bound >= 0.0 && lower_bound >= 0.0){
	  step = (upper_bound - lower_bound) / AGS_DIAL_DEFAULT_PRECISION;
	}else if(upper_bound < 0.0 && lower_bound < 0.0){
	  step = -1.0 * (lower_bound - upper_bound) / AGS_DIAL_DEFAULT_PRECISION;
	}else{
	  step = (upper_bound - lower_bound) / AGS_DIAL_DEFAULT_PRECISION;
	}

	gtk_adjustment_set_step_increment(adjustment,
					  step);
	gtk_adjustment_set_lower(adjustment,
				 lower_bound);
	gtk_adjustment_set_upper(adjustment,
				 upper_bound);
	gtk_adjustment_set_value(adjustment,
				 g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->default_value));
      }

#ifdef AGS_DEBUG
      g_message("lv2 bounds: %f %f\0", lower_bound, upper_bound);
#endif
	  
      gtk_table_attach(effect_line->table,
		       line_member,
		       x, x + 1,
		       y, y + 1,
		       GTK_FILL, GTK_FILL,
		       0, 0);
      
      ags_connectable_connect(AGS_CONNECTABLE(line_member));
      gtk_widget_show_all(line_member);

      port = port->next;
      x++;
    }

    port_descriptor = port_descriptor->next;
    k++;
  }
  
  return(port);
}

GList*
ags_effect_line_real_add_effect(AgsEffectLine *effect_line,
				gchar *filename,
				gchar *effect)
{
  AgsLadspaPlugin *ladspa_plugin;
  AgsLv2Plugin *lv2_plugin;
  
  GList *port;

  /* load plugin */
  ladspa_plugin = ags_ladspa_manager_find_ladspa_plugin(filename, effect);
  port = NULL;
  
  if(ladspa_plugin != NULL){
    port = ags_effect_line_add_ladspa_effect(effect_line,
					     filename,
					     effect);
  }else{
    lv2_plugin = ags_lv2_manager_find_lv2_plugin(filename, effect);
    
    if(lv2_plugin != NULL){
      port = ags_effect_line_add_lv2_effect(effect_line,
					    filename,
					    effect);
    }
  }
  
  return(port);
}

GList*
ags_effect_line_add_effect(AgsEffectLine *effect_line,
			   gchar *filename,
			   gchar *effect)
{
  GList *port;
  
  g_return_val_if_fail(AGS_IS_EFFECT_LINE(effect_line), NULL);

  g_object_ref((GObject *) effect_line);
  g_signal_emit(G_OBJECT(effect_line),
		effect_line_signals[ADD_EFFECT], 0,
		filename,
		effect,
		&port);
  g_object_unref((GObject *) effect_line);

  return(port);
}

void
ags_effect_line_real_remove_effect(AgsEffectLine *effect_line,
				   guint nth)
{
  GList *control;
  GList *play_ladspa;
  GList *port;

  play_ladspa = ags_recall_template_find_type(effect_line->channel->play,
					      AGS_TYPE_RECALL_LADSPA);

  /* destroy controls */
  port = AGS_RECALL(g_list_nth(play_ladspa,
			       nth)->data)->port;
    
  while(port != NULL){
    control = gtk_container_get_children(effect_line->table);
      
    while(control != NULL){
      if(AGS_IS_LINE_MEMBER(control->data) &&
	 AGS_LINE_MEMBER(control->data)->port == port->data){
	gtk_widget_destroy(control->data);
	break;
      }
	
      control = control->next;
    }
      
    port = port->next;
  }

  /* remove recalls */
  ags_channel_remove_effect(effect_line->channel,
			    nth);
}

void
ags_effect_line_remove_effect(AgsEffectLine *effect_line,
			      guint nth)
{
  g_return_if_fail(AGS_IS_EFFECT_LINE(effect_line));

  g_object_ref((GObject *) effect_line);
  g_signal_emit(G_OBJECT(effect_line),
		effect_line_signals[REMOVE_EFFECT], 0,
		nth);
  g_object_unref((GObject *) effect_line);
}

void
ags_effect_line_real_map_recall(AgsEffectLine *effect_line)
{
  if((AGS_EFFECT_LINE_MAPPED_RECALL & (effect_line->flags)) != 0){
    return;
  }

  effect_line->flags |= AGS_EFFECT_LINE_MAPPED_RECALL;

  ags_effect_line_find_port(effect_line);
}

/**
 * ags_effect_line_map_recall:
 * @effect_line: the #AgsEffectLine to add its default recall.
 *
 * You may want the @effect_line to add its default recall.
 */
void
ags_effect_line_map_recall(AgsEffectLine *effect_line)
{
  g_return_if_fail(AGS_IS_EFFECT_LINE(effect_line));

  g_object_ref((GObject *) effect_line);
  g_signal_emit((GObject *) effect_line,
		effect_line_signals[MAP_RECALL], 0);
  g_object_unref((GObject *) effect_line);
}

GList*
ags_effect_line_real_find_port(AgsEffectLine *effect_line)
{
  GList *port, *tmp_port;
  GList *line_member, *line_member_start;

  if(effect_line == NULL || effect_line->table == NULL){
    return(NULL);
  }

  line_member_start = 
    line_member = gtk_container_get_children(GTK_CONTAINER(effect_line->table));
  
  port = NULL;

  if(line_member != NULL){
    while(line_member != NULL){
      if(AGS_IS_LINE_MEMBER(line_member->data)){
	tmp_port = ags_line_member_find_port(AGS_LINE_MEMBER(line_member->data));

	if(port != NULL){
	  port = g_list_concat(port,
			       tmp_port);
	}else{
	  port = tmp_port;
	}
      }

      line_member = line_member->next;
    }

    g_list_free(line_member_start);
  }  
  
  return(port);
}

/**
 * ags_effect_line_find_port:
 * @effect_line: the #AgsEffectLine
 * Returns: an #GList containing all related #AgsPort
 *
 * Lookup ports of associated recalls.
 *
 * Since: 0.7.8
 */
GList*
ags_effect_line_find_port(AgsEffectLine *effect_line)
{
  GList *list;

  list = NULL;
  g_return_val_if_fail(AGS_IS_EFFECT_LINE(effect_line),
		       NULL);

  g_object_ref((GObject *) effect_line);
  g_signal_emit((GObject *) effect_line,
		effect_line_signals[FIND_PORT], 0,
		&list);
  g_object_unref((GObject *) effect_line);

  return(list);
}

/**
 * ags_effect_line_new:
 * @effect_line: the parent effect_line
 * @channel: the #AgsChannel to visualize
 *
 * Creates an #AgsEffectLine
 *
 * Returns: a new #AgsEffectLine
 *
 * Since: 0.4
 */
AgsEffectLine*
ags_effect_line_new(AgsChannel *channel)
{
  AgsEffectLine *effect_line;

  effect_line = (AgsEffectLine *) g_object_new(AGS_TYPE_EFFECT_LINE,
					       "channel\0", channel,
					       NULL);

  return(effect_line);
}