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

#ifndef __AGS_MACHINE_H__
#define __AGS_MACHINE_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/audio/ags_audio.h>

#define AGS_TYPE_MACHINE                (ags_machine_get_type())
#define AGS_MACHINE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_MACHINE, AgsMachine))
#define AGS_MACHINE_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_MACHINE, AgsMachineClass))
#define AGS_IS_MACHINE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_MACHINE))
#define AGS_IS_MACHINE_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_MACHINE))
#define AGS_MACHINE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_MACHINE, AgsMachineClass))

#define AGS_MACHINE_DEFAULT_VERSION "0.7.0\0"
#define AGS_MACHINE_DEFAULT_BUILD_ID "CEST 30-10-2015 02:37\0"

typedef struct _AgsMachine AgsMachine;
typedef struct _AgsMachineClass AgsMachineClass;

typedef enum{
  AGS_MACHINE_SOLO              = 1,
  AGS_MACHINE_IS_EFFECT         = 1 <<  1,
  AGS_MACHINE_IS_SEQUENCER      = 1 <<  2,
  AGS_MACHINE_IS_SYNTHESIZER    = 1 <<  3,
  AGS_MACHINE_TAKES_FILE_INPUT  = 1 <<  4,
  AGS_MACHINE_MAPPED_RECALL     = 1 <<  5,
  AGS_MACHINE_PREMAPPED_RECALL  = 1 <<  6,
  AGS_MACHINE_BLOCK_PLAY        = 1 <<  7,
  AGS_MACHINE_BLOCK_STOP        = 1 <<  8,
  AGS_MACHINE_CONNECTED         = 1 <<  9,
  AGS_MACHINE_REVERSE_NOTATION  = 1 << 10,
}AgsMachineFlags;

typedef enum{
  AGS_MACHINE_ACCEPT_WAV          = 1,
  AGS_MACHINE_ACCEPT_OGG          = 1 <<  1,
  AGS_MACHINE_ACCEPT_SOUNDFONT2   = 1 <<  2,
}AgsMachineFileInputFlags;

typedef enum{
  AGS_MACHINE_MONO = 1,
}AgsMachineMappingFlags;

typedef enum{
  AGS_MACHINE_POPUP_COPY_PATTERN          = 1,
  AGS_MACHINE_POPUP_PASTE_PATTERN         = 1 <<  1,
}AgsMachineEditOptions;

typedef enum{
  AGS_MACHINE_POPUP_MIDI_DIALOG           = 1,
}AgsMachineConnectionOptions;

struct _AgsMachine
{
  GtkHandleBox handle_box;

  GObject *application_context;

  char *name;

  gchar *version;
  gchar *build_id;

  guint flags;
  guint file_input_flags;
  guint mapping_flags;
  
  guint bank_0;
  guint bank_1;
  
  AgsAudio *audio;

  GtkToggleButton *play;

  GType output_pad_type;
  GType output_line_type;
  GtkContainer *output;

  GtkWidget *selected_output_pad;
  
  GType input_pad_type;
  GType input_line_type;
  GtkContainer *input;
  
  GtkWidget *selected_input_pad;

  GtkContainer *bridge;

  GList *port;
  gchar **automation_port;

  GtkMenu *popup;
  GtkDialog *properties;
  GtkDialog *rename;
  GtkDialog *connection;
};

struct _AgsMachineClass
{
  GtkHandleBoxClass handle_box;

  void (*map_recall)(AgsMachine *machine);
  GList* (*find_port)(AgsMachine *machine);
};

GType ags_machine_get_type(void);


void ags_machine_set_audio_channels(AgsAudio *audio,
				    guint audio_channels, guint audio_channels_old,
				    AgsMachine *machine);
void ags_machine_set_pads(AgsAudio *audio, GType type,
			  guint pads, guint pads_old,
			  AgsMachine *machine);

void ags_machine_add_default_recalls(AgsMachine *machine) G_DEPRECATED_FOR(ags_machine_map_recall);

void ags_machine_map_recall(AgsMachine *machine);

GtkListStore* ags_machine_get_possible_links(AgsMachine *machine);

AgsMachine* ags_machine_find_by_name(GList *list, char *name);

GList* ags_machine_find_port(AgsMachine *machine);

void ags_machine_set_run(AgsMachine *machine,
			 gboolean run);
void ags_machine_set_run_extended(AgsMachine *machine,
				  gboolean run,
				  gboolean sequencer, gboolean notation);

GtkFileChooserDialog* ags_machine_file_chooser_dialog_new(AgsMachine *machine);

void ags_machine_open_files(AgsMachine *machine,
			    GSList *filenames,
			    gboolean overwrite_channels,
			    gboolean create_channels);

void ags_machine_copy_pattern(AgsMachine *machine);

AgsMachine* ags_machine_new(GObject *soundcard);

#endif /*__AGS_MACHINE_H__*/