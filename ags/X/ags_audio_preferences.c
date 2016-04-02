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

#include <ags/X/ags_audio_preferences.h>
#include <ags/X/ags_audio_preferences_callbacks.h>

#include <ags/object/ags_config.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_applicable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_preferences.h>

void ags_audio_preferences_class_init(AgsAudioPreferencesClass *audio_preferences);
void ags_audio_preferences_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_audio_preferences_applicable_interface_init(AgsApplicableInterface *applicable);
void ags_audio_preferences_init(AgsAudioPreferences *audio_preferences);
void ags_audio_preferences_connect(AgsConnectable *connectable);
void ags_audio_preferences_disconnect(AgsConnectable *connectable);
void ags_audio_preferences_set_update(AgsApplicable *applicable, gboolean update);
void ags_audio_preferences_apply(AgsApplicable *applicable);
void ags_audio_preferences_reset(AgsApplicable *applicable);
static void ags_audio_preferences_finalize(GObject *gobject);
void ags_audio_preferences_show(GtkWidget *widget);

/**
 * SECTION:ags_audio_preferences
 * @short_description: A composite widget to do audio related preferences
 * @title: AgsAudioPreferences
 * @section_id: 
 * @include: ags/X/ags_audio_preferences.h
 *
 * #AgsAudioPreferences enables you to make preferences of soundcard, audio channels,
 * samplerate and buffer size.
 */

static gpointer ags_audio_preferences_parent_class = NULL;

GType
ags_audio_preferences_get_type(void)
{
  static GType ags_type_audio_preferences = 0;

  if(!ags_type_audio_preferences){
    static const GTypeInfo ags_audio_preferences_info = {
      sizeof (AgsAudioPreferencesClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_preferences_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAudioPreferences),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_preferences_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_audio_preferences_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_applicable_interface_info = {
      (GInterfaceInitFunc) ags_audio_preferences_applicable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_audio_preferences = g_type_register_static(GTK_TYPE_VBOX,
							"AgsAudioPreferences\0", &ags_audio_preferences_info,
							0);
    
    g_type_add_interface_static(ags_type_audio_preferences,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_audio_preferences,
				AGS_TYPE_APPLICABLE,
				&ags_applicable_interface_info);
  }

  return(ags_type_audio_preferences);
}

void
ags_audio_preferences_class_init(AgsAudioPreferencesClass *audio_preferences)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;

  ags_audio_preferences_parent_class = g_type_class_peek_parent(audio_preferences);

  /* GtkObjectClass */
  gobject = (GObjectClass *) audio_preferences;

  gobject->finalize = ags_audio_preferences_finalize;

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) audio_preferences;

  widget->show = ags_audio_preferences_show;
}

void
ags_audio_preferences_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_audio_preferences_connect;
  connectable->disconnect = ags_audio_preferences_disconnect;
}

void
ags_audio_preferences_applicable_interface_init(AgsApplicableInterface *applicable)
{
  applicable->set_update = ags_audio_preferences_set_update;
  applicable->apply = ags_audio_preferences_apply;
  applicable->reset = ags_audio_preferences_reset;
}

void
ags_audio_preferences_init(AgsAudioPreferences *audio_preferences)
{
  GtkTable *table;
  GtkHBox *hbox;
  GtkLabel *label;
  GtkCellRenderer *cell_renderer;

  g_signal_connect_after((GObject *) audio_preferences, "parent_set\0",
			 G_CALLBACK(ags_audio_preferences_parent_set_callback), (gpointer) audio_preferences);

  table = (GtkTable *) gtk_table_new(2, 7, FALSE);
  gtk_box_pack_start(GTK_BOX(audio_preferences),
		     GTK_WIDGET(table),
		     FALSE, FALSE,
		     2);

  /* sound card */
  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label\0", "sound card\0",
				    "xalign\0", 0.0,
				    NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  audio_preferences->card = (GtkComboBox *) gtk_combo_box_new();
  gtk_table_attach(table,
		   GTK_WIDGET(audio_preferences->card),
		   1, 2,
		   0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  cell_renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(audio_preferences->card),
			     cell_renderer,
			     FALSE); 
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(audio_preferences->card),
				 cell_renderer,
				 "text\0", 1,
				 NULL);
  gtk_combo_box_set_active(audio_preferences->card, 0);
  
  /* audio channels */
  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label\0", "audio channels\0",
				    "xalign\0", 0.0,
				    NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  audio_preferences->audio_channels = (GtkSpinButton *) gtk_spin_button_new_with_range(1.0, 24.0, 1.0);
  gtk_spin_button_set_value(audio_preferences->audio_channels, 2);
  gtk_table_attach(table,
		   GTK_WIDGET(audio_preferences->audio_channels),
		   1, 2,
		   1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* samplerate */
  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label\0", "samplerate\0",
				    "xalign\0", 0.0,
				    NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   2, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  audio_preferences->samplerate = (GtkSpinButton *) gtk_spin_button_new_with_range(1.0, 192000.0, 1.0);
  gtk_spin_button_set_value(audio_preferences->samplerate, 44100);
  gtk_table_attach(table,
		   GTK_WIDGET(audio_preferences->samplerate),
		   1, 2,
		   2, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* buffer size */
  label = (GtkLabel *) g_object_new(GTK_TYPE_LABEL,
				    "label\0", "buffer size\0",
				    "xalign\0", 0.0,
				    NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   3, 4,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  audio_preferences->buffer_size = (GtkSpinButton *) gtk_spin_button_new_with_range(1.0, 65535.0, 1.0);
  gtk_spin_button_set_value(audio_preferences->buffer_size, 512);
  gtk_table_attach(table,
		   GTK_WIDGET(audio_preferences->buffer_size),
		   1, 2,
		   3, 4,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  /* JACK */
  audio_preferences->enable_jack = (GtkCheckButton *) gtk_check_button_new_with_label("Enable JACK\0");
  gtk_table_attach(table,
		   GTK_WIDGET(audio_preferences->enable_jack),
		   0, 2,
		   4, 5,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  label = gtk_label_new("JACK driver\0");
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   5, 6,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  audio_preferences->jack_driver = (GtkComboBoxText *) gtk_combo_box_text_new();
  gtk_table_attach(table,
		   GTK_WIDGET(audio_preferences->jack_driver),
		   1, 2,
		   5, 6,
		   GTK_FILL, GTK_FILL,
		   0, 0);
  
  label = gtk_label_new("JACK server\0");
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   6, 7,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  hbox = (GtkHBox *) gtk_hbox_new(TRUE, 0);
  gtk_table_attach(table,
		   GTK_WIDGET(hbox),
		   1, 2,
		   6, 7,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  audio_preferences->start_jack = (GtkButton *) gtk_button_new_with_label("start\0");
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(audio_preferences->start_jack),
		     FALSE, FALSE,
		     0);

  audio_preferences->stop_jack = (GtkButton *) gtk_button_new_with_label("stop\0");
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(audio_preferences->stop_jack),
		     FALSE, FALSE,
		     0);
  
  /* set default insensitive */
  gtk_widget_set_sensitive(audio_preferences->jack_driver,
			   FALSE);
  
  gtk_widget_set_sensitive(audio_preferences->start_jack,
			   FALSE);
  gtk_widget_set_sensitive(audio_preferences->stop_jack,
			   FALSE);
}

void
ags_audio_preferences_connect(AgsConnectable *connectable)
{
  AgsAudioPreferences *audio_preferences;

  audio_preferences = AGS_AUDIO_PREFERENCES(connectable);

  g_signal_connect(G_OBJECT(audio_preferences->card), "changed\0",
		   G_CALLBACK(ags_audio_preferences_card_changed_callback), audio_preferences);

  g_signal_connect(G_OBJECT(audio_preferences->enable_jack), "clicked\0",
		   G_CALLBACK(ags_audio_preferences_enable_jack_callback), audio_preferences);

  g_signal_connect(G_OBJECT(audio_preferences->start_jack), "clicked\0",
		   G_CALLBACK(ags_audio_preferences_start_jack_callback), audio_preferences);

  g_signal_connect(G_OBJECT(audio_preferences->stop_jack), "clicked\0",
		   G_CALLBACK(ags_audio_preferences_stop_jack_callback), audio_preferences);
}

void
ags_audio_preferences_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

static void
ags_audio_preferences_finalize(GObject *gobject)
{
  //TODO:JK: implement me
}

void
ags_audio_preferences_set_update(AgsApplicable *applicable, gboolean update)
{
  //TODO:JK: implement me
}

void
ags_audio_preferences_apply(AgsApplicable *applicable)
{
  AgsPreferences *preferences;
  AgsAudioPreferences *audio_preferences; 
  AgsConfig *config;
  GList *card_id, *card_name;
  GtkListStore *model;
  GtkTreeIter current;
  GValue value =  {0,};
  char *device, *str;
  int card_num;
  guint channels, channels_min, channels_max;
  guint rate, rate_min, rate_max;
  guint buffer_size, buffer_size_min, buffer_size_max;

  audio_preferences = AGS_AUDIO_PREFERENCES(applicable);

  preferences = (AgsPreferences *) gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
							   AGS_TYPE_PREFERENCES);
  config = ags_config_get_instance();

  /* device */
  model = gtk_combo_box_get_model(audio_preferences->card);

  if(!(gtk_combo_box_get_active_iter(audio_preferences->card,
				     &current))){
    return;
  }

  gtk_tree_model_get_value(model,
			   &current,
			   0,
			   &value);
  
  //FIXME:JK: work-around for alsa-handle
  str = g_strdup_printf("%s,0", g_value_get_string(&value));
  g_message("%s\0", str);
  ags_config_set_value(config,
		 AGS_CONFIG_SOUNDCARD,
		 "alsa-handle\0",
		 str);
  g_free(str);

  /* samplerate */
  str = g_strdup_printf("%u\0",
			(guint) gtk_spin_button_get_value(audio_preferences->samplerate));
  ags_config_set_value(config,
		 AGS_CONFIG_SOUNDCARD,
		 "samplerate\0",
		 str);
  g_free(str);

  /* buffer size */
  str = g_strdup_printf("%u\0",
			(guint) gtk_spin_button_get_value(audio_preferences->buffer_size));
  ags_config_set_value(config,
		 AGS_CONFIG_SOUNDCARD,
		 "buffer-size\0",
		 str);
  g_free(str);

  /* dsp channels */
  str = g_strdup_printf("%u\0",
			(guint) gtk_spin_button_get_value(audio_preferences->audio_channels));
  ags_config_set_value(config,
		 AGS_CONFIG_SOUNDCARD,
		 "dsp-channels\0",
		 str);
  g_free(str);
}

void
ags_audio_preferences_reset(AgsApplicable *applicable)
{
  AgsWindow *window;
  AgsPreferences *preferences;
  AgsAudioPreferences *audio_preferences;
  
  AgsConfig *config;
  AgsSoundcard *soundcard;
  GtkListStore *model;
  GtkTreeIter current;
  GList *card_id, *card_name;

  char *device, *str, *tmp;
  guint nth;
  gboolean found_card;
  int card_num;
  guint channels, channels_min, channels_max;
  guint rate, rate_min, rate_max;
  guint buffer_size, buffer_size_min, buffer_size_max;

  GValue value =  {0,};
  
  GError *error;

  audio_preferences = AGS_AUDIO_PREFERENCES(applicable);

  /*  */
  preferences = (AgsPreferences *) gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
							   AGS_TYPE_PREFERENCES);
  window = AGS_WINDOW(preferences->window);
  config = ags_config_get_instance();

  soundcard = AGS_SOUNDCARD(window->soundcard);

  /* refresh */
  ags_soundcard_list_cards(soundcard,
			   &card_id, &card_name);    
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "alsa-handle\0");
#ifdef AGS_DEBUG
  g_message("configured soundcard: %s\0", str);
#endif
  
  nth = 0;
  found_card = FALSE;

  while(card_id != NULL){
    //FIXME:JK: work-around for alsa-handle
    tmp = g_strdup_printf("%s,0\0",
			  card_id->data);
    if(!g_ascii_strcasecmp(tmp,
			   str)){
      found_card = TRUE;
    }

    g_free(tmp);
    
    if(!found_card){
      nth++;
    }
    
    card_id = card_id->next;
    card_name = card_name->next;
  }
  
  if(!found_card){
    nth = 0;
  }
  
  gtk_combo_box_set_active(audio_preferences->card,
			   nth);
  
  g_list_free(card_id);
  g_list_free(card_name);

  /*  */
  device = ags_soundcard_get_device(soundcard);
  ags_soundcard_get_presets(soundcard,
			    &channels,
			    &rate,
			    &buffer_size,
			    NULL);

  /*  */
  gtk_spin_button_set_value(audio_preferences->audio_channels,
			    (gdouble) channels);
  gtk_spin_button_set_value(audio_preferences->samplerate,
			    (gdouble) rate);
  gtk_spin_button_set_value(audio_preferences->buffer_size,
			    (gdouble) buffer_size);

  /*  */
  model = gtk_combo_box_get_model(audio_preferences->card);

  if(!(gtk_combo_box_get_active_iter(audio_preferences->card,
				     &current))){
    return;
  }

  gtk_tree_model_get_value(model,
			   &current,
			   0,
			   &value);
  str = g_strdup(g_value_get_string(&value));

  error = NULL;
  ags_soundcard_pcm_info(soundcard,
			 str,
			 &channels_min, &channels_max,
			 &rate_min, &rate_max,
			 &buffer_size_min, &buffer_size_max,
			 &error);

  if(error != NULL){
    GtkMessageDialog *dialog;

    dialog = (GtkMessageDialog *) gtk_message_dialog_new((GtkWindow *) gtk_widget_get_ancestor(GTK_WIDGET(audio_preferences),
											       AGS_TYPE_PREFERENCES),
							 GTK_DIALOG_MODAL,
							 GTK_MESSAGE_ERROR,
							 GTK_BUTTONS_CLOSE,
							 "%s\0", error->message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));

    gtk_spin_button_set_range(audio_preferences->audio_channels, 0.0, 24.0);
    gtk_spin_button_set_range(audio_preferences->samplerate, 1.0, 192000.0);
    gtk_spin_button_set_range(audio_preferences->buffer_size, 1.0, 65535.0);

    return;
  }

  gtk_spin_button_set_range(audio_preferences->audio_channels,
			    channels_min, channels_max);
  gtk_spin_button_set_range(audio_preferences->samplerate,
			    rate_min, rate_max);
  gtk_spin_button_set_range(audio_preferences->buffer_size,
			    buffer_size_min, buffer_size_max);
}

void
ags_audio_preferences_show(GtkWidget *widget)
{
  AgsAudioPreferences *audio_preferences;
  pthread_t thread;

  audio_preferences = AGS_AUDIO_PREFERENCES(widget);
  
  GTK_WIDGET_CLASS(ags_audio_preferences_parent_class)->show(widget);
}

/**
 * ags_audio_preferences_new:
 *
 * Creates an #AgsAudioPreferences
 *
 * Returns: a new #AgsAudioPreferences
 *
 * Since: 0.4
 */
AgsAudioPreferences*
ags_audio_preferences_new()
{
  AgsAudioPreferences *audio_preferences;

  audio_preferences = (AgsAudioPreferences *) g_object_new(AGS_TYPE_AUDIO_PREFERENCES,
							   NULL);
  
  return(audio_preferences);
}