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

#include <ags/X/ags_line_callbacks.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>
#include <ags/thread/ags_task_thread.h>

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

#include <ags/widget/ags_indicator.h>
#include <ags/widget/ags_vindicator.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line_member.h>

#include <ags/X/task/ags_change_indicator.h>

int
ags_line_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsLine *line)
{
  if(old_parent == NULL){
    //    gtk_widget_show_all(GTK_WIDGET(line));
  }
}

void
ags_line_remove_recall_callback(AgsRecall *recall, AgsLine *line)
{
  if(recall->recall_id != NULL && recall->recall_id->recycling_context->parent != NULL){
    if(AGS_IS_RECALL_AUDIO(recall) || AGS_RECALL_AUDIO_RUN(recall)){
      ags_audio_remove_recall(AGS_AUDIO(line->channel->audio), (GObject *) recall, FALSE);
    }else{
      ags_channel_remove_recall(AGS_CHANNEL(line->channel), (GObject *) recall, FALSE);
    }
  }else{
    if(AGS_IS_RECALL_AUDIO(recall) || AGS_RECALL_AUDIO_RUN(recall)){
      ags_audio_remove_recall(AGS_AUDIO(line->channel->audio), (GObject *) recall, TRUE);
    }else{
      ags_channel_remove_recall(AGS_CHANNEL(line->channel), (GObject *) recall, TRUE);
    }
  }
}

int
ags_line_group_clicked_callback(GtkWidget *widget, AgsLine *line)
{
  AgsPad *pad;
  AgsLine *current;
  GtkContainer *container;
  GList *list, *list_start;

  pad = (AgsPad *) gtk_widget_get_ancestor(GTK_WIDGET(line), AGS_TYPE_PAD);

  container = (GtkContainer *) pad->expander_set;

  list_start =
    list = gtk_container_get_children(container);

  if(gtk_toggle_button_get_active(line->group)){
    ags_line_group_changed(line);

    while(list != NULL){
      current = AGS_LINE(list->data);

      if(!gtk_toggle_button_get_active(current->group)){
	g_list_free(list_start);
	return(0);
      }

      list = list->next;
    }

    gtk_toggle_button_set_active(pad->group, TRUE);
  }else{
    if(g_list_length(list) > 1){
      if(gtk_toggle_button_get_active(pad->group)){
	gtk_toggle_button_set_active(pad->group, FALSE);
      }

      while(list != NULL){
	current = AGS_LINE(list->data);

	if(gtk_toggle_button_get_active(current->group)){
	  ags_line_group_changed(line);
	  g_list_free(list_start);
	  return(0);
	}

	list = list->next;
      } 
    }

    gtk_toggle_button_set_active(line->group, TRUE);
  }

  g_list_free(list_start);

  return(0);
}

void
ags_line_add_effect_callback(AgsChannel *channel,
			     gchar *filename,
			     gchar *effect,
			     AgsLine *line)
{
  gdk_threads_enter();

  ags_line_add_effect(line,
		      filename,
		      effect);

  gdk_threads_leave();
}

void
ags_line_remove_effect_callback(AgsChannel *channel,
				guint nth,
				AgsLine *line)
{
  gdk_threads_enter();

  ags_line_remove_effect(line,
			 nth);

  gdk_threads_leave();
}

void
ags_line_volume_callback(GtkRange *range,
			 AgsLine *line)
{
  AgsVolumeChannel *volume_channel;
  GList *list;
  GValue value = {0,};

  g_value_init(&value, G_TYPE_DOUBLE);
  g_value_set_double(&value, gtk_range_get_value(range));

  list = line->channel->play;

  while((list = ags_recall_find_type(list, AGS_TYPE_VOLUME_CHANNEL)) != NULL){
    volume_channel = AGS_VOLUME_CHANNEL(list->data);
    ags_port_safe_write(volume_channel->volume,
			&value);

    list = list->next;
  }

  list = line->channel->recall;

  while((list = ags_recall_find_type(list, AGS_TYPE_VOLUME_CHANNEL)) != NULL){
    volume_channel = AGS_VOLUME_CHANNEL(list->data);
    ags_port_safe_write(volume_channel->volume,
			&value);

    list = list->next;
  }
}

void
ags_line_peak_run_post_callback(AgsRecall *peak_channel_run,
				AgsLine *line)
{
  AgsWindow *window;
  AgsMachine *machine;
  GtkWidget *child;

  AgsPort *port;

  AgsChangeIndicator *change_indicator;

  AgsMutexManager *mutex_manager;
  AgsThread *audio_loop;
  AgsTaskThread *task_thread;

  AgsApplicationContext *application_context;
  
  GList *list, *list_start;

  gdouble peak;

  GValue value = {0,};

  pthread_mutex_t *application_mutex;

  gdk_threads_enter();
  
  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) line,
						   AGS_TYPE_MACHINE);
  
  window = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) machine,
						  AGS_TYPE_WINDOW);
  
  application_context = window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  audio_loop = application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  task_thread = (AgsTaskThread *) ags_thread_find_type(audio_loop,
						       AGS_TYPE_TASK_THREAD);

  list_start = 
    list = gtk_container_get_children((GtkContainer *) AGS_LINE(line)->expander->table);

  while(list != NULL){
    if(AGS_IS_LINE_MEMBER(list->data) &&
       AGS_LINE_MEMBER(list->data)->widget_type == AGS_TYPE_VINDICATOR){

      child = GTK_BIN(list->data)->child;

      port = AGS_PEAK_CHANNEL(AGS_RECALL_CHANNEL_RUN(peak_channel_run)->recall_channel)->peak;
	
      g_value_init(&value, G_TYPE_DOUBLE);
      ags_port_safe_read(port,
			 &value);

      peak = g_value_get_double(&value);
      g_value_unset(&value);
      
      //      if(peak_channel_run->recall_id->recycling_context->parent == NULL)
	//	g_message("%f\0", peak);
      
      change_indicator = ags_change_indicator_new((AgsIndicator *) child,
						  peak);

      ags_task_thread_append_task(task_thread,
				  (AgsTask *) change_indicator);

      break;
    }
    
    list = list->next;
  }

  g_list_free(list_start);

  gdk_threads_leave();
}

void
ags_line_channel_done_callback(AgsChannel *source, AgsRecallID *recall_id,
			       AgsLine *line)
{
  AgsChannel *channel;
  AgsPlayback *playback;
  AgsChannel *next_pad;
  GList *current_recall;
  gboolean all_done;

  channel = AGS_PAD(AGS_LINE(line)->pad)->channel;
  next_pad = channel->next_pad;

  all_done = TRUE;

  while(channel != next_pad){
    current_recall = channel->play;
    playback = AGS_PLAYBACK(channel->playback);
    
    if(playback->recall_id[0] != NULL){
      all_done = FALSE;
      break;
    }
    
    channel = channel->next;
  }

  if(all_done){
    AgsPad *pad;

    pad = AGS_PAD(AGS_LINE(line)->pad);

    if(pad->play != NULL){
      gtk_toggle_button_set_active(pad->play, FALSE);
    }
  }
}
