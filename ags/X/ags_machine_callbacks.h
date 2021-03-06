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
#ifndef __AGS_MACHINE_CALLBACKS_H__
#define __AGS_MACHINE_CALLBACKS_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/thread/ags_task_completion.h>

#include <ags/thread/ags_task.h>

#include <ags/X/ags_machine.h>

int ags_machine_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsMachine *machine);
int ags_machine_destroy_callback(GtkObject *object, AgsMachine *machine);

/* controls */
int ags_machine_button_press_callback(GtkWidget *handle_box, GdkEventButton *event, AgsMachine *machine);

int ags_machine_popup_move_up_activate_callback(GtkWidget *widget, AgsMachine *machine);
int ags_machine_popup_move_down_activate_callback(GtkWidget *widget, AgsMachine *machine);
int ags_machine_popup_hide_activate_callback(GtkWidget *widget, AgsMachine *machine);
int ags_machine_popup_show_activate_callback(GtkWidget *widget, AgsMachine *machine);
void ags_machine_popup_destroy_activate_callback(GtkWidget *widget, AgsMachine *machine);
int ags_machine_popup_rename_activate_callback(GtkWidget *widget, AgsMachine *machine);
int ags_machine_popup_properties_activate_callback(GtkWidget *widget, AgsMachine *machine);

int ags_machine_popup_copy_pattern_callback(GtkWidget *widget, AgsMachine *machine);
int ags_machine_popup_paste_pattern_callback(GtkWidget *widget, AgsMachine *machine);

int ags_machine_popup_connection_editor_callback(GtkWidget *widget, AgsMachine *machine);
int ags_machine_popup_midi_dialog_callback(GtkWidget *widget, AgsMachine *machine);

void ags_machine_open_response_callback(GtkDialog *dialog, gint response, AgsMachine *machine);

void ags_machine_play_callback(GtkWidget *toggle_button, AgsMachine *machine);

/* audio */
void ags_machine_set_audio_channels_callback(AgsAudio *audio,
					     guint audio_channels, guint audio_channels_old,
					     AgsMachine *machine);
void ags_machine_set_pads_callback(AgsAudio *audio, GType channel_type,
				   guint pads, guint pads_old,
				   AgsMachine *machine);

void ags_machine_tact_callback(AgsAudio *audio,
			       AgsRecallID *recall_id,
			       AgsMachine *machine);
void ags_machine_done_callback(AgsAudio *audio,
			       AgsRecallID *recall_id,
			       AgsMachine *machine);

/* task */
void ags_machine_start_complete_callback(AgsTaskCompletion *task_completion,
					 AgsMachine *machine);

#endif /*__AGS_MACHINE_CALLBACKS_H__*/
