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

#include <ags/X/ags_preferences.h>
#include <ags/X/ags_preferences_callbacks.h>

#include <ags/object/ags_config.h>
#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_applicable.h>

#include <ags/X/ags_window.h>

#include <ags/X/file/ags_simple_file.h>

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>

void ags_preferences_class_init(AgsPreferencesClass *preferences);
void ags_preferences_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_preferences_applicable_interface_init(AgsApplicableInterface *applicable);
void ags_preferences_init(AgsPreferences *preferences);
void ags_preferences_connect(AgsConnectable *connectable);
void ags_preferences_disconnect(AgsConnectable *connectable);
void ags_preferences_set_update(AgsApplicable *applicable, gboolean update);
void ags_preferences_apply(AgsApplicable *applicable);
void ags_preferences_reset(AgsApplicable *applicable);
static void ags_preferences_finalize(GObject *gobject);
void ags_preferences_show(GtkWidget *widget);
void ags_preferences_show_all(GtkWidget *widget);

/**
 * SECTION:ags_preferences
 * @short_description: A dialog to do preferences
 * @title: AgsPluginPreferences
 * @section_id: 
 * @include: ags/X/ags_plugin_preferences.h
 *
 * #AgsDialogPreferences enables you to make preferences.
 */

static gpointer ags_preferences_parent_class = NULL;

GType
ags_preferences_get_type(void)
{
  static GType ags_type_preferences = 0;

  if(!ags_type_preferences){
    static const GTypeInfo ags_preferences_info = {
      sizeof (AgsPreferencesClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_preferences_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPreferences),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_preferences_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_preferences_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_applicable_interface_info = {
      (GInterfaceInitFunc) ags_preferences_applicable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_preferences = g_type_register_static(GTK_TYPE_DIALOG,
						  "AgsPreferences\0", &ags_preferences_info,
						  0);
    
    g_type_add_interface_static(ags_type_preferences,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_preferences,
				AGS_TYPE_APPLICABLE,
				&ags_applicable_interface_info);
  }

  return(ags_type_preferences);
}

void
ags_preferences_class_init(AgsPreferencesClass *preferences)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;

  ags_preferences_parent_class = g_type_class_peek_parent(preferences);

  /* GtkObjectClass */
  gobject = (GObjectClass *) preferences;

  gobject->finalize = ags_preferences_finalize;

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) preferences;

  widget->show = ags_preferences_show;
  widget->show_all = ags_preferences_show_all;
}

void
ags_preferences_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_preferences_connect;
  connectable->disconnect = ags_preferences_disconnect;
}

void
ags_preferences_applicable_interface_init(AgsApplicableInterface *applicable)
{
  applicable->set_update = ags_preferences_set_update;
  applicable->apply = ags_preferences_apply;
  applicable->reset = ags_preferences_reset;
}

void
ags_preferences_init(AgsPreferences *preferences)
{
  GtkButton *button;

  gchar *str;

  preferences->flags = 0;

  preferences->window = NULL;

  gtk_window_set_title(GTK_WINDOW(preferences),
		       g_strdup("preferences\0"));
  gtk_window_set_deletable(GTK_WINDOW(preferences),
			   TRUE);

  preferences->notebook = (GtkNotebook *) gtk_notebook_new();
  g_object_set(G_OBJECT(preferences->notebook),
	       "tab-pos\0", GTK_POS_LEFT,
	       NULL);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(preferences)->vbox),
		    GTK_WIDGET(preferences->notebook));

  preferences->generic_preferences = ags_generic_preferences_new();
  gtk_notebook_append_page(preferences->notebook,
			   GTK_WIDGET(preferences->generic_preferences),
			   gtk_label_new("generic\0"));

  preferences->audio_preferences = ags_audio_preferences_new();
  gtk_notebook_append_page(preferences->notebook,
			   GTK_WIDGET(preferences->audio_preferences),
			   gtk_label_new("audio\0"));

  preferences->midi_preferences = ags_midi_preferences_new();
  gtk_notebook_append_page(preferences->notebook,
			   GTK_WIDGET(preferences->midi_preferences),
			   gtk_label_new("midi\0"));

  preferences->performance_preferences = ags_performance_preferences_new();
  gtk_notebook_append_page(preferences->notebook,
			   GTK_WIDGET(preferences->performance_preferences),
			   gtk_label_new("performance\0"));

  str = ags_config_get_value(ags_config_get_instance(),
			     AGS_CONFIG_GENERIC,
			     "disable-feature\0");
  
  if(str == NULL ||
     !g_ascii_strncasecmp(str,
			  "experimental\0",
			  13)){
    preferences->server_preferences = NULL;
  }else{
    preferences->server_preferences = ags_server_preferences_new();
    gtk_notebook_append_page(preferences->notebook,
			     GTK_WIDGET(preferences->server_preferences),
			     gtk_label_new("server\0"));
  }
  
  gtk_dialog_add_action_widget(GTK_DIALOG(preferences),
			       gtk_button_new_from_stock(GTK_STOCK_APPLY),
			       GTK_RESPONSE_APPLY);

  gtk_dialog_add_action_widget(GTK_DIALOG(preferences),
			       gtk_button_new_from_stock(GTK_STOCK_CANCEL),
			       GTK_RESPONSE_CANCEL);

  gtk_dialog_add_action_widget(GTK_DIALOG(preferences),
			       gtk_button_new_from_stock(GTK_STOCK_OK),
			       GTK_RESPONSE_OK);
}

void
ags_preferences_connect(AgsConnectable *connectable)
{
  AgsPreferences *preferences;

  preferences = AGS_PREFERENCES(connectable);

  ags_connectable_connect(AGS_CONNECTABLE(preferences->generic_preferences));
  ags_connectable_connect(AGS_CONNECTABLE(preferences->audio_preferences));
  ags_connectable_connect(AGS_CONNECTABLE(preferences->midi_preferences));
  ags_connectable_connect(AGS_CONNECTABLE(preferences->performance_preferences));

  if(preferences->server_preferences != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(preferences->server_preferences));
  }
  
  g_signal_connect_after(G_OBJECT(preferences), "response\0",
			 G_CALLBACK(ags_preferences_response_callback), NULL);

  g_signal_connect_after(G_OBJECT(preferences->notebook), "switch-page\0",
			 G_CALLBACK(ags_preferences_notebook_switch_page_callback), preferences);
}

void
ags_preferences_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_preferences_set_update(AgsApplicable *applicable, gboolean update)
{
  AgsPreferences *preferences;

  preferences = AGS_PREFERENCES(applicable);

  ags_applicable_set_update(AGS_APPLICABLE(preferences->generic_preferences), update);
  ags_applicable_set_update(AGS_APPLICABLE(preferences->audio_preferences), update);
  ags_applicable_set_update(AGS_APPLICABLE(preferences->midi_preferences), update);
  ags_applicable_set_update(AGS_APPLICABLE(preferences->performance_preferences), update);

  if(preferences->server_preferences != NULL){
    ags_applicable_set_update(AGS_APPLICABLE(preferences->server_preferences), update);
  }
}

void
ags_preferences_apply(AgsApplicable *applicable)
{
  AgsPreferences *preferences;

  AgsConfig *config;

  gchar *filename;

  struct passwd *pw;
  uid_t uid;

  gchar **argv;
  GError *error;

  preferences = AGS_PREFERENCES(applicable);
  
  config = ags_config_get_instance();

  ags_config_clear(config);

  ags_applicable_apply(AGS_APPLICABLE(preferences->generic_preferences));
  ags_applicable_apply(AGS_APPLICABLE(preferences->audio_preferences));
  ags_applicable_apply(AGS_APPLICABLE(preferences->midi_preferences));
  ags_applicable_apply(AGS_APPLICABLE(preferences->performance_preferences));

  if(preferences->server_preferences != NULL){
    ags_applicable_apply(AGS_APPLICABLE(preferences->server_preferences));
  }
  
  ags_config_save(config);
}

void
ags_preferences_reset(AgsApplicable *applicable)
{
  AgsPreferences *preferences;

  preferences = AGS_PREFERENCES(applicable);

  ags_applicable_reset(AGS_APPLICABLE(preferences->generic_preferences));
  ags_applicable_reset(AGS_APPLICABLE(preferences->audio_preferences));
  ags_applicable_reset(AGS_APPLICABLE(preferences->midi_preferences));
  ags_applicable_reset(AGS_APPLICABLE(preferences->performance_preferences));

  if(preferences->server_preferences != NULL){
    ags_applicable_reset(AGS_APPLICABLE(preferences->server_preferences));
  }
}

static void
ags_preferences_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_preferences_parent_class)->finalize(gobject);
}

void
ags_preferences_show(GtkWidget *widget)
{
  GList *list, *list_start;

  GTK_WIDGET_CLASS(ags_preferences_parent_class)->show(widget);
  
  list_start = 
    list = gtk_container_get_children((GtkContainer *) GTK_DIALOG(widget)->action_area);
  list = g_list_nth(list,
		    3);
  
  while(list != NULL){
    gtk_widget_hide((GtkWidget *) list->data);

    list = list->next;
  }

  g_list_free(list_start);
}

void
ags_preferences_show_all(GtkWidget *widget)
{
  GList *list, *list_start;

  GTK_WIDGET_CLASS(ags_preferences_parent_class)->show_all(widget);

  list_start = 
    list = gtk_container_get_children((GtkContainer *) GTK_DIALOG(widget)->action_area);
  list = g_list_nth(list,
		    3);
  
  while(list != NULL){
    gtk_widget_hide(list->data);

    list = list->next;
  }

  g_list_free(list_start);
}


/**
 * ags_preferences_new:
 *
 * Creates an #AgsPreferences
 *
 * Returns: a new #AgsPreferences
 *
 * Since: 0.4
 */
AgsPreferences*
ags_preferences_new()
{
  AgsPreferences *preferences;

  preferences = (AgsPreferences *) g_object_new(AGS_TYPE_PREFERENCES,
						NULL);
  
  return(preferences);
}
