/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
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

#include <ags/X/ags_menu_bar_callbacks.h>

#include <ags/object/ags_connectable.h>

#include <ags/file/ags_file.h>

#include <ags/audio/ags_input.h>
#include <ags/audio/ags_output.h>

#include <ags/X/ags_window.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_ffplayer.h>

void ags_menu_bar_open_ok_callback(GtkWidget *widget, AgsMenuBar *menu_bar);
void ags_menu_bar_open_cancel_callback(GtkWidget *widget, AgsMenuBar *menu_bar);

void ags_menu_bar_save_as_ok_callback(GtkWidget *widget, AgsMenuBar *menu_bar);
void ags_menu_bar_save_as_cancel_callback(GtkWidget *widget, AgsMenuBar *menu_bar);

gboolean
ags_menu_bar_destroy_callback(GtkObject *object, AgsMenuBar *menu_bar)
{
  ags_menu_bar_destroy(object);

  return(TRUE);
}

void
ags_menu_bar_show_callback(GtkWidget *widget, AgsMenuBar *menu_bar)
{
  ags_menu_bar_show(widget);
}

void
ags_menu_bar_open_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  GtkFileSelection *file_selection;

  file_selection = (GtkFileSelection *) gtk_file_selection_new(g_strdup("open file\0"));
  gtk_file_selection_set_select_multiple(file_selection, FALSE);

  gtk_widget_show_all((GtkWidget *) file_selection);

  g_signal_connect((GObject *) file_selection->ok_button, "clicked\0",
		   G_CALLBACK(ags_menu_bar_open_ok_callback), menu_bar);
  g_signal_connect((GObject *) file_selection->cancel_button, "clicked\0",
		   G_CALLBACK(ags_menu_bar_open_cancel_callback), menu_bar);
}

void
ags_menu_bar_open_ok_callback(GtkWidget *widget, AgsMenuBar *menu_bar)
{
  GtkFileSelection *file_selection;
  AgsFile *file;

  file_selection = (GtkFileSelection *) gtk_widget_get_ancestor(widget, GTK_TYPE_DIALOG);

  file = ags_file_new();
  file->name = g_strdup(gtk_file_selection_get_filename(file_selection));
  ags_file_read(file);

  g_object_unref(G_OBJECT(file));
  gtk_widget_destroy((GtkWidget *) file_selection);
}

void
ags_menu_bar_open_cancel_callback(GtkWidget *widget, AgsMenuBar *menu_bar)
{
  GtkFileSelection *file_selection;

  file_selection = (GtkFileSelection *) gtk_widget_get_ancestor(widget, GTK_TYPE_DIALOG);
  gtk_widget_destroy((GtkWidget *) file_selection);
}

void
ags_menu_bar_save_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  AgsFile *file;

  file = ags_file_new();
  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) menu_bar);
  file->window = (GtkWidget *) window;
  file->name = g_strdup(window->name);
  ags_file_write(file);
  g_object_unref(G_OBJECT(file));
}

void
ags_menu_bar_save_as_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  GtkFileSelection *file_selection;

  file_selection = (GtkFileSelection *) gtk_file_selection_new(g_strdup("save file as\0"));
  gtk_file_selection_set_select_multiple(file_selection, FALSE);

  gtk_widget_show_all((GtkWidget *) file_selection);

  g_signal_connect((GObject *) file_selection->ok_button, "clicked\0",
		   G_CALLBACK(ags_menu_bar_save_as_ok_callback), (gpointer) menu_bar);
  g_signal_connect((GObject *) file_selection->cancel_button, "clicked\0",
		   G_CALLBACK(ags_menu_bar_save_as_cancel_callback), (gpointer) menu_bar);
}

void
ags_menu_bar_save_as_ok_callback(GtkWidget *widget, AgsMenuBar *menu_bar)
{
  GtkFileSelection *file_selection;
  AgsWindow *window;
  AgsFile *file;

  file_selection = (GtkFileSelection *) gtk_widget_get_ancestor(widget, GTK_TYPE_FILE_SELECTION);

  file = ags_file_new();
  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) menu_bar);
  file->window = (GtkWidget *) window;
  file->name = g_strdup(gtk_file_selection_get_filename((GtkFileSelection *) file_selection));
  ags_file_write(file);

  g_object_unref(G_OBJECT(file));
  gtk_widget_destroy((GtkWidget *) file_selection);
}

void
ags_menu_bar_save_as_cancel_callback(GtkWidget *widget, AgsMenuBar *menu_bar)
{
  GtkFileSelection *file_selection;

  file_selection = (GtkFileSelection *) gtk_widget_get_ancestor(widget, GTK_TYPE_FILE_SELECTION);
  gtk_widget_destroy((GtkWidget *) file_selection);
}

void
ags_menu_bar_quit_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  GtkDialog *dialog;
  GtkWidget *cancel_button;
  gint response;

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) menu_bar);

  /* ask the user if he wants save to a file */
  dialog = (GtkDialog *) gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						"Do you want to save '%s'?\0", window->name);
  cancel_button = gtk_dialog_add_button(dialog,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL);
  gtk_widget_grab_focus(cancel_button);

  response = gtk_dialog_run(dialog);

  if(response == GTK_RESPONSE_YES){
    AgsFile *file;

    file = ags_file_new();
    file->window = (GtkWidget *) window;
    file->name = g_strdup(window->name);
    ags_file_write(file);
    g_object_unref(G_OBJECT(file));
  }

  if(response != GTK_RESPONSE_CANCEL){
    gtk_main_quit();
  }else{
    gtk_widget_destroy(GTK_WIDGET(dialog));
  }
}


void
ags_menu_bar_add_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
}


void
ags_menu_bar_add_panel_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  AgsPanel *panel;

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) menu_bar, AGS_TYPE_WINDOW);

  panel = ags_panel_new(G_OBJECT(window->devout));

  gtk_box_pack_start((GtkBox *) window->machines,
		     GTK_WIDGET(panel),
		     FALSE, FALSE, 0);

  ags_connectable_connect(AGS_CONNECTABLE(panel));

  gtk_widget_show_all(GTK_WIDGET(panel));

  panel->machine.audio->input_pads =
    panel->machine.audio->output_pads = 1;
  ags_audio_set_audio_channels(panel->machine.audio, 2);
}

void
ags_menu_bar_add_mixer_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  AgsMixer *mixer;

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) menu_bar, AGS_TYPE_WINDOW);

  mixer = ags_mixer_new(G_OBJECT(window->devout));

  gtk_box_pack_start((GtkBox *) window->machines,
		     GTK_WIDGET(mixer),
		     FALSE, FALSE, 0);

  ags_connectable_connect(AGS_CONNECTABLE(mixer));

  gtk_widget_show_all(GTK_WIDGET(mixer));

  mixer->machine.audio->audio_channels = 2;
  ags_audio_set_pads(mixer->machine.audio,
		     AGS_TYPE_INPUT, 8);
  ags_audio_set_pads(mixer->machine.audio,
		     AGS_TYPE_OUTPUT, 1);
}

void
ags_menu_bar_add_drum_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  AgsDrum *drum;

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) menu_bar, AGS_TYPE_WINDOW);

  drum = ags_drum_new(G_OBJECT(window->devout));

  gtk_box_pack_start((GtkBox *) window->machines,
		     GTK_WIDGET(drum),
		     FALSE, FALSE, 0);

  /* connect everything */
  ags_connectable_connect(AGS_CONNECTABLE(drum));

  /* */
  gtk_widget_show_all(GTK_WIDGET(drum));

  /* */
  drum->machine.audio->audio_channels = 2;

  /* AgsDrumInputPad */
  ags_audio_set_pads(drum->machine.audio, AGS_TYPE_INPUT, 8);
  ags_audio_set_pads(drum->machine.audio, AGS_TYPE_OUTPUT, 1);
}

void
ags_menu_bar_add_matrix_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  AgsMatrix *matrix;

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) menu_bar, AGS_TYPE_WINDOW);

  matrix = ags_matrix_new(G_OBJECT(window->devout));

  gtk_box_pack_start((GtkBox *) window->machines,
		     (GtkWidget *) matrix,
		     FALSE, FALSE, 0);

  ags_connectable_connect(AGS_CONNECTABLE(matrix));

  gtk_widget_show_all((GtkWidget *) matrix);

  matrix->machine.audio->audio_channels = 1;
  ags_audio_set_pads(matrix->machine.audio, AGS_TYPE_INPUT, 78);
  ags_audio_set_pads(matrix->machine.audio, AGS_TYPE_OUTPUT, 1);
}

void
ags_menu_bar_add_synth_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  AgsSynth *synth;

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) menu_bar, AGS_TYPE_WINDOW);

  synth = ags_synth_new(G_OBJECT(window->devout));

  gtk_box_pack_start((GtkBox *) window->machines,
		     (GtkWidget *) synth,
		     FALSE, FALSE, 0);

  ags_connectable_connect(AGS_CONNECTABLE(synth));

  gtk_widget_show_all((GtkWidget *) synth);

  synth->machine.audio->audio_channels = 1;
  ags_audio_set_pads((AgsAudio*) synth->machine.audio, AGS_TYPE_INPUT, 2);
  ags_audio_set_pads((AgsAudio*) synth->machine.audio, AGS_TYPE_OUTPUT, 78);
}

void
ags_menu_bar_add_ffplayer_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  AgsWindow *window;
  AgsFFPlayer *ffplayer;

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) menu_bar, AGS_TYPE_WINDOW);

  ffplayer = ags_ffplayer_new(G_OBJECT(window->devout));

  gtk_box_pack_start((GtkBox *) window->machines,
		     (GtkWidget *) ffplayer,
		     FALSE, FALSE, 0);

  ags_connectable_connect(AGS_CONNECTABLE(ffplayer));

  gtk_widget_show_all((GtkWidget *) ffplayer);

  //  ffplayer->machine.audio->frequence = ;
  ffplayer->machine.audio->audio_channels = 2;
  ags_audio_set_pads(AGS_MACHINE(ffplayer)->audio, AGS_TYPE_INPUT, 78);
  ags_audio_set_pads(AGS_MACHINE(ffplayer)->audio, AGS_TYPE_OUTPUT, 1);
}


void
ags_menu_bar_remove_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
}

void
ags_menu_bar_about_callback(GtkWidget *menu_item, AgsMenuBar *menu_bar)
{
  static FILE *file = NULL;
  struct stat sb;
  static gchar *license;

  gchar *authors[] = { "joel kraehemann\0", NULL }; 

  if(file == NULL){
    file = fopen("./COPYING\0", "r\0");
    stat("./COPYING\0", &sb);
    license = (gchar *) malloc((sb.st_size + 1) * sizeof(gchar));
    fread(license, sizeof(char), sb.st_size, file);
    license[sb.st_size] = '\0';
    fclose(file);
  }

  gtk_show_about_dialog((GtkWindow *) gtk_widget_get_ancestor((GtkWidget *) menu_bar, GTK_TYPE_WINDOW),
			"program-name\0", "ags\0",
			"authors\0", authors,
			"license\0", license,
			"title\0", "ags\0",
			NULL);
}
