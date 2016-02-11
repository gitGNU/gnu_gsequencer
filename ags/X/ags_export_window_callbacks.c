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

#include <ags/X/ags_export_window_callbacks.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>
#include <ags/thread/ags_task_thread.h>

#include <ags/audio/thread/ags_audio_loop.h>
#include <ags/audio/thread/ags_export_thread.h>

#include <ags/audio/task/ags_export_output.h>

#include <ags/X/ags_xorg_application_context.h>
#include <ags/X/ags_window.h>
#include <ags/X/ags_navigation.h>

void ags_export_window_stop_callback(AgsThread *thread,
				     AgsExportWindow *export_window);

void
ags_export_window_file_chooser_button_callback(GtkWidget *file_chooser_button,
					       AgsExportWindow *export_window)
{
  GtkFileChooserDialog *file_chooser;

  file_chooser = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new("Export to file ...\0",
								      GTK_WINDOW(export_window),
								      GTK_FILE_CHOOSER_ACTION_SAVE,
								      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
								      NULL);
  if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT){
    char *filename;

    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
    gtk_entry_set_text(export_window->filename,
		       filename);
  }
  
  gtk_widget_destroy((GtkWidget *) file_chooser);
}

void
ags_export_window_tact_callback(GtkWidget *spin_button,
				AgsExportWindow *export_window)
{
  AgsWindow *window;

  window = AGS_XORG_APPLICATION_CONTEXT(export_window->application_context)->window;

  gtk_label_set_text(export_window->duration,
		     ags_navigation_absolute_tact_to_time_string(gtk_spin_button_get_value(export_window->tact) * 16.0,
								 window->navigation->bpm->adjustment->value,
								 ags_soundcard_get_delay_factor(AGS_SOUNDCARD(window->soundcard))));
}

void
ags_export_window_export_callback(GtkWidget *toggle_button,
				  AgsExportWindow *export_window)
{
  AgsWindow *window;
  AgsMachine *machine;
  
  AgsMutexManager *mutex_manager;
  AgsAudioLoop *audio_loop;
  AgsTaskThread *task_thread;

  AgsApplicationContext *application_context;
  
  GList *machines_start;
  gboolean success;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;

  window = AGS_XORG_APPLICATION_CONTEXT(export_window->application_context)->window;

  application_context = window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
      
  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  audio_loop = application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* get task and soundcard thread */
  task_thread = (AgsTaskThread *) ags_thread_find_type(audio_loop,
						       AGS_TYPE_TASK_THREAD);
  
  machines_start = NULL;

  if(gtk_toggle_button_get_active((GtkToggleButton *) toggle_button)){
    AgsExportOutput *export_output;
    AgsExportThread *export_thread;
    GList *machines;
    gchar *filename;
    gboolean live_performance;

    export_thread = (AgsExportThread *) ags_thread_find_type(audio_loop,
							     AGS_TYPE_EXPORT_THREAD);

    filename = gtk_entry_get_text(export_window->filename);

    /* test filename */
    if(filename == NULL ||
       strlen(filename) == 0){
      return;
    }

    if(g_file_test(filename,
		   G_FILE_TEST_EXISTS)){
      GtkDialog *dialog;
      gint response;
      
      if(g_file_test(filename,
		     (G_FILE_TEST_IS_DIR | G_FILE_TEST_IS_SYMLINK))){
	return;
      }

      dialog = gtk_message_dialog_new((GtkWindow *) export_window,
				      GTK_DIALOG_MODAL,
				      GTK_MESSAGE_QUESTION,
				      GTK_BUTTONS_OK_CANCEL,
				      "Replace existing file?\0");
      response = gtk_dialog_run(dialog);
      gtk_widget_destroy((GtkWidget *) dialog);

      if(response == GTK_RESPONSE_REJECT){
	return;
      }

      g_remove(filename);
    }

    /* get some preferences */
    live_performance = gtk_toggle_button_get_active((GtkToggleButton *) export_window->live_export);

    machines_start = 
      machines = gtk_container_get_children(GTK_CONTAINER(window->machines));

    /* start machines */
    success = FALSE;

    while(machines != NULL){
      machine = AGS_MACHINE(machines->data);

      if((AGS_MACHINE_IS_SEQUENCER & (machine->flags)) != 0 ||
	 (AGS_MACHINE_IS_SYNTHESIZER & (machine->flags)) != 0){
	printf("found machine to play!\n\0");

	ags_machine_set_run_extended(machine,
				     TRUE,
				     !gtk_toggle_button_get_active(export_window->exclude_sequencer), TRUE);
	success = TRUE;
      }

      machines = machines->next;
    }

    /* start export thread */
    if(success){
      guint tic;
      gdouble delay;
      
      pthread_mutex_lock(application_mutex);

      soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					      (GObject *) window->soundcard);
  
      pthread_mutex_unlock(application_mutex);

      pthread_mutex_lock(soundcard_mutex);

      /* create task */
      delay = ags_soundcard_get_delay(AGS_SOUNDCARD(window->soundcard));

      tic = (gtk_spin_button_get_value(export_window->tact) + 1) * delay * 16.0;

      export_output = ags_export_output_new(export_thread,
					    window->soundcard,
					    filename,
					    tic,
					    live_performance);
      g_signal_connect(export_thread, "stop\0",
		       G_CALLBACK(ags_export_window_stop_callback), export_window);

      pthread_mutex_unlock(soundcard_mutex);

      /* append AgsStartSoundcard */
      ags_task_thread_append_task(task_thread,
				  (AgsTask *) export_output);

      ags_navigation_set_seeking_sensitive(window->navigation,
					   FALSE);
    }
  }else{
    GList *machines;

    machines_start = 
      machines = gtk_container_get_children(GTK_CONTAINER(window->machines));

    /* stop machines */
    while(machines != NULL){
      machine = AGS_MACHINE(machines->data);

      if((AGS_MACHINE_IS_SEQUENCER & (machine->flags)) !=0 ||
	 (AGS_MACHINE_IS_SYNTHESIZER & (machine->flags)) != 0){
	printf("found machine to stop!\n\0");
    
	ags_machine_set_run(machine,
			    FALSE);
	
	success = TRUE;
      }

      machines = machines->next;
    }

    /* disable auto-seeking */
    if(success){
      ags_navigation_set_seeking_sensitive(window->navigation,
					   TRUE);
    }
  }

  /* free machine list */
  g_list_free(machines_start);
}

void
ags_export_window_stop_callback(AgsThread *thread,
				AgsExportWindow *export_window)
{
  gtk_toggle_button_set_active(export_window->export,
			       FALSE);
}