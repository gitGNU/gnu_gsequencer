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

#include <ags/X/ags_effect_line_callbacks.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/plugin/ags_base_plugin.h>

#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_audio_run.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_port.h>
#include <ags/audio/ags_recycling_context.h>

#include <ags/audio/thread/ags_audio_loop.h>

#include <ags/audio/recall/ags_peak_channel.h>
#include <ags/audio/recall/ags_volume_channel.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>
#include <ags/audio/recall/ags_copy_pattern_channel_run.h>

#include <ags/widget/ags_led.h>
#include <ags/widget/ags_vindicator.h>
#include <ags/widget/ags_hindicator.h>
#include <ags/widget/ags_dial.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_listing_editor.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line_member.h>
#include <ags/X/ags_machine_editor.h>
#include <ags/X/ags_pad_editor.h>
#include <ags/X/ags_line_editor.h>
#include <ags/X/ags_line_member_editor.h>
#include <ags/X/ags_plugin_browser.h>
#include <ags/X/ags_ladspa_browser.h>
#include <ags/X/ags_dssi_browser.h>
#include <ags/X/ags_lv2_browser.h>

#include <ags/X/thread/ags_gui_thread.h>

void
ags_effect_line_remove_recall_callback(AgsRecall *recall, AgsEffectLine *effect_line)
{
  if(recall->recall_id != NULL && recall->recall_id->recycling_context->parent != NULL){
    if(AGS_IS_RECALL_AUDIO(recall) || AGS_RECALL_AUDIO_RUN(recall)){
      ags_audio_remove_recall(AGS_AUDIO(effect_line->channel->audio), (GObject *) recall, FALSE);
    }else{
      ags_channel_remove_recall(AGS_CHANNEL(effect_line->channel), (GObject *) recall, FALSE);
    }
  }else{
    if(AGS_IS_RECALL_AUDIO(recall) || AGS_RECALL_AUDIO_RUN(recall)){
      ags_audio_remove_recall(AGS_AUDIO(effect_line->channel->audio), (GObject *) recall, TRUE);
    }else{
      ags_channel_remove_recall(AGS_CHANNEL(effect_line->channel), (GObject *) recall, TRUE);
    }
  }
}

void
ags_effect_line_add_effect_callback(AgsChannel *channel,
				    gchar *filename,
				    gchar *effect,
				    AgsEffectLine *effect_line)
{
  AgsWindow *window;
  AgsMachine *machine;
  AgsMachineEditor *machine_editor;
  AgsLineMemberEditor *line_member_editor;
  AgsPluginBrowser *plugin_browser;

  AgsGuiThread *gui_thread;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  AgsApplicationContext *application_context;
  
  GList *pad_editor, *pad_editor_start;
  GList *line_editor, *line_editor_start;
  GList *control_type_name;
  
  pthread_mutex_t *application_mutex;
  
  /* lock gdk threads */
  gdk_threads_enter();

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) effect_line);

  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsAudioLoop *) application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type((AgsThread *) main_loop,
						      AGS_TYPE_GUI_THREAD);

  /*  */
  gdk_threads_enter();

  /* get machine and machine editor */
  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) effect_line,
						   AGS_TYPE_MACHINE);
  machine_editor = (AgsMachineEditor *) machine->properties;

  /* get control type */
  control_type_name = NULL;  

  pad_editor_start = NULL;
  line_editor_start = NULL;

  if(machine_editor != NULL){
    pad_editor_start = 
      pad_editor = gtk_container_get_children((GtkContainer *) machine_editor->input_editor->child);
    pad_editor = g_list_nth(pad_editor,
			    channel->pad);
    
    if(pad_editor != NULL){
      line_editor_start =
	line_editor = gtk_container_get_children((GtkContainer *) AGS_PAD_EDITOR(pad_editor->data)->line_editor);
      line_editor = g_list_nth(line_editor,
			       channel->audio_channel);      
    }else{
      line_editor = NULL;
    }

    if(line_editor != NULL){
      line_member_editor = AGS_LINE_EDITOR(line_editor->data)->member_editor;
      plugin_browser = line_member_editor->plugin_browser;

      if(plugin_browser != NULL &&
	 plugin_browser->active_browser != NULL){
	GList *description, *description_start;
	GList *port_control, *port_control_start;

	gchar *controls;

	/* get plugin browser */
	description_start = NULL;
	port_control_start = NULL;
	
	if(AGS_IS_LADSPA_BROWSER(plugin_browser->active_browser)){
	  description_start = 
	    description = gtk_container_get_children((GtkContainer *) AGS_LADSPA_BROWSER(plugin_browser->active_browser)->description);
	}else if(AGS_IS_DSSI_BROWSER(plugin_browser->active_browser)){
	  description_start = 
	    description = gtk_container_get_children((GtkContainer *) AGS_DSSI_BROWSER(plugin_browser->active_browser)->description);
	}else if(AGS_IS_LV2_BROWSER(plugin_browser->active_browser)){
	  description_start = 
	    description = gtk_container_get_children((GtkContainer *) AGS_LV2_BROWSER(plugin_browser->active_browser)->description);
	}else{
	  g_message("ags_effect_line_callbacks.c unsupported plugin browser\0");
	}

	/* get port description */
	if(description != NULL){
	  description = g_list_last(description);
	  
	  port_control_start =
	    port_control = gtk_container_get_children(GTK_CONTAINER(description->data));
	  
	  if(port_control != NULL){
	    while(port_control != NULL){
	      controls = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(port_control->data));

	      if(!g_ascii_strncasecmp(controls,
				      "led\0",
				      4)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsLed\0");
	      }else if(!g_ascii_strncasecmp(controls,
				      "vertical indicator\0",
				      19)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsVIndicator\0");
	      }else if(!g_ascii_strncasecmp(controls,
				      "horizontal indicator\0",
				      19)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsHIndicator\0");
	      }else if(!g_ascii_strncasecmp(controls,
				      "spin button\0",
				      12)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkSpinButton\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "dial\0",
					    5)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsDial\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "vertical scale\0",
					    15)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkVScale\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "horizontal scale\0",
					    17)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkHScale\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "check-button\0",
					    13)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkCheckButton\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "toggle button\0",
					    14)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkToggleButton\0");
	      }
	      
	      port_control = port_control->next;
	      port_control = port_control->next;
	    }   
	  }

	  /* free lists */
	  g_list_free(description_start);
	  g_list_free(port_control_start);
	}
      }
      
      //      line_member_editor->plugin_browser;
    }
  }else{
    control_type_name = NULL;
  }

  /* add effect */
  ags_effect_line_add_effect(effect_line,
			     control_type_name,
			     filename,
			     effect);

  /* free container children list */
  g_list_free(pad_editor_start);
  g_list_free(line_editor_start);

  gdk_threads_leave();

  /* unlock gdk threads */
  gdk_threads_leave();
}

void
ags_effect_line_remove_effect_callback(AgsChannel *channel,
				       guint nth,
				       AgsEffectLine *effect_line)
{
  AgsWindow *window;

  AgsGuiThread *gui_thread;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  AgsApplicationContext *application_context;

  pthread_mutex_t *application_mutex;

  /* lock gdk threads */
  gdk_threads_enter();

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) effect_line);

  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsAudioLoop *) application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type((AgsThread *) main_loop,
						      AGS_TYPE_GUI_THREAD);

  /*  */
  gdk_threads_enter();

  /* remove effect */
  ags_effect_line_remove_effect(effect_line,
				nth);

  gdk_threads_leave();

  /* unlock gdk threads */
  gdk_threads_leave();
}

void
ags_effect_line_output_port_run_post_callback(AgsRecall *recall,
					      AgsEffectLine *effect_line)
{
  GtkWidget *child;

  GList *list, *list_start;
  
  /* lock gdk threads */
  gdk_threads_enter();
  
  list_start = 
    list = gtk_container_get_children((GtkContainer *) AGS_EFFECT_LINE(effect_line)->table);

  /* check members */
  while(list != NULL){
    if(AGS_IS_LINE_MEMBER(list->data) &&
       (AGS_LINE_MEMBER(list->data)->widget_type == AGS_TYPE_VINDICATOR ||
	AGS_LINE_MEMBER(list->data)->widget_type == AGS_TYPE_HINDICATOR ||
	AGS_LINE_MEMBER(list->data)->widget_type == AGS_TYPE_LED)){
      AgsLineMember *line_member;
      GtkAdjustment *adjustment;

      AgsPort *current;
	
      gdouble average_peak;
      gdouble lower, upper;
      gdouble range;
      gdouble peak;

      GValue value = {0,};

      line_member = AGS_LINE_MEMBER(list->data);
      child = GTK_BIN(line_member)->child;
      
      average_peak = 0.0;
      
      /* play port */
      current = line_member->port;

      if(current == NULL){
	list = list->next;
	
	continue;
      }
      
      /* check if output port and specifier matches */
      pthread_mutex_lock(current->mutex);
      
      if((AGS_PORT_IS_OUTPUT & (current->flags)) == 0 ||
	 current->port_descriptor == NULL ||
	 g_ascii_strcasecmp(current->specifier,
			    line_member->specifier)){
	pthread_mutex_unlock(current->mutex);
	
	list = list->next;
	
	continue;
      }

      /* lower and upper */
      lower = g_value_get_float(AGS_PORT_DESCRIPTOR(current->port_descriptor)->lower_value);
      upper = g_value_get_float(AGS_PORT_DESCRIPTOR(current->port_descriptor)->upper_value);
      
      pthread_mutex_unlock(current->mutex);

      /* get range */
      if(line_member->conversion != NULL){
	lower = ags_conversion_convert(line_member->conversion,
				       lower,
				       TRUE);

	upper = ags_conversion_convert(line_member->conversion,
				       upper,
				       TRUE);
      }

      range = upper - lower;

      /* play port - read value */
      g_value_init(&value, G_TYPE_FLOAT);
      ags_port_safe_read(current,
			 &value);
      
      peak = g_value_get_float(&value);
      g_value_unset(&value);

      if(line_member->conversion != NULL){
	peak = ags_conversion_convert(line_member->conversion,
				      peak,
				      TRUE);
      }

      /* calculate peak */
      if(range == 0.0 ||
	 current->port_value_type == G_TYPE_BOOLEAN){
	if(peak != 0.0){
	  average_peak = 10.0;
	}
      }else{
	average_peak += ((1.0 / (range / peak)) * 10.0);
      }

      /* recall port */
      current = line_member->recall_port;

      /* recall port - read value */
      g_value_init(&value, G_TYPE_FLOAT);
      ags_port_safe_read(current,
			 &value);
      
      peak = g_value_get_float(&value);
      g_value_unset(&value);
      
      if(line_member->conversion != NULL){
	peak = ags_conversion_convert(line_member->conversion,
				      peak,
				      TRUE);
      }

      /* calculate peak */
      if(range == 0.0 ||
	 current->port_value_type == G_TYPE_BOOLEAN){
	if(peak != 0.0){
	  average_peak = 10.0;
	}
      }else{
	average_peak += ((1.0 / (range / peak)) * 10.0);
      }
      
      /* apply */
      if(AGS_IS_LED(child)){
	if(average_peak != 0.0){
	  ags_led_set_active(child);
	}
      }else{
	g_object_get(child,
		     "adjustment\0", &adjustment,
		     NULL);
	
	gtk_adjustment_set_value(adjustment,
				 average_peak);
      }
    }
    
    list = list->next;
  }

  g_list_free(list_start);

  /* unlock gdk threads */
  gdk_threads_leave();
}
