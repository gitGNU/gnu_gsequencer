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

#ifndef __AGS_MATRIX_H__
#define __AGS_MATRIX_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/X/ags_machine.h>

#include <ags/X/machine/ags_cell_pattern.h>

#define AGS_TYPE_MATRIX                (ags_matrix_get_type())
#define AGS_MATRIX(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_MATRIX, AgsMatrix))
#define AGS_MATRIX_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_MATRIX, AgsMatrixClass))
#define AGS_IS_MATRIX(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_MATRIX))
#define AGS_IS_MATRIX_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_MATRIX))
#define AGS_MATRIX_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_MATRIX, AgsMatrixClass))

typedef struct _AgsMatrix AgsMatrix;
typedef struct _AgsMatrixClass AgsMatrixClass;

struct _AgsMatrix
{
  AgsMachine machine;

  guint flags;

  gchar *name;
  gchar *xml_type;

  guint mapped_output_pad;
  guint mapped_input_pad;

  GtkTable *table;

  GtkToggleButton *run;
  GtkToggleButton *selected;
  GtkToggleButton *index[9];

  AgsCellPattern *cell_pattern;

  GtkSpinButton *length_spin;
  GtkCheckButton *loop_button;
};

struct _AgsMatrixClass
{
  AgsMachineClass machine;
};

GType ags_matrix_get_type(void);

void ags_matrix_input_map_recall(AgsMatrix *matrix, guint input_pad_start);
void ags_matrix_output_map_recall(AgsMatrix *matrix, guint output_pad_start);

AgsMatrix* ags_matrix_new(GObject *soundcard);

#endif /*__AGS_MATRIX_H__*/
