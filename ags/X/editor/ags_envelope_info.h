/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#ifndef __AGS_ENVELOPE_INFO_H__
#define __AGS_ENVELOPE_INFO_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <ags/widget/ags_cartesian.h>

#define AGS_TYPE_ENVELOPE_INFO                (ags_envelope_info_get_type())
#define AGS_ENVELOPE_INFO(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_ENVELOPE_INFO, AgsEnvelopeInfo))
#define AGS_ENVELOPE_INFO_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_ENVELOPE_INFO, AgsEnvelopeInfoClass))
#define AGS_IS_ENVELOPE_INFO(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_ENVELOPE_INFO))
#define AGS_IS_ENVELOPE_INFO_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_ENVELOPE_INFO))
#define AGS_ENVELOPE_INFO_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_ENVELOPE_INFO, AgsEnvelopeInfoClass))

typedef struct _AgsEnvelopeInfo AgsEnvelopeInfo;
typedef struct _AgsEnvelopeInfoClass AgsEnvelopeInfoClass;
typedef struct _AgsEnvelopeNoteInfo AgsEnvelopeNoteInfo;

typedef enum{
  AGS_ENVELOPE_INFO_CONNECTED      = 1,
}AgsEnvelopeInfoFlags;

struct _AgsEnvelopeInfo
{
  GtkDialog dialog;

  guint flags;
  
  GtkVBox *note_info;
  GList *children;

  GtkButton *ok;
};

struct _AgsEnvelopeInfoClass
{
  GtkDialogClass dialog;
};

struct _AgsEnvelopeNoteInfo
{
  GtkVBox *vbox;

  AgsCartesian *cartesian;

  GtkLabel *note;

  GtkLabel *attack;
  GtkLabel *decay;
  GtkLabel *sustain;
  GtkLabel *release;
  GtkLabel *ratio;
};

GType ags_envelope_info_get_type(void);

AgsEnvelopeInfo* ags_envelope_info_new(AgsMachine *machine);

#endif /*__AGS_ENVELOPE_INFO_H__*/