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

#ifndef __AGS_INPUT_H__
#define __AGS_INPUT_H__

#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>

#include <ags/audio/ags_channel.h>

#define AGS_TYPE_INPUT                (ags_input_get_type())
#define AGS_INPUT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_INPUT, AgsInput))
#define AGS_INPUT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_INPUT, AgsInputClass))
#define AGS_IS_INPUT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_INPUT))
#define AGS_IS_INPUT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_INPUT))
#define AGS_INPUT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_INPUT, AgsInputClass))

typedef struct _AgsInput AgsInput;
typedef struct _AgsInputClass AgsInputClass;

struct _AgsInput
{
  AgsChannel channel;

  GObject *file_link;
  GObject *synth_generator;
};

struct _AgsInputClass
{
  AgsChannelClass channel;
};

GType ags_input_get_type();

gboolean ags_input_open_file(AgsInput *input,
			     gchar *filename,
			     gchar *preset,
			     gchar *instrument,
			     gchar *sample,
			     guint audio_channel);
gboolean ags_input_apply_synth(AgsInput *input,
			       guint oscillator,
			       gdouble frequency,
			       gdouble phase,
			       gdouble volume,
			       guint n_frames);

gboolean ags_input_is_active(AgsInput *input,
			     GObject *parent_recycling_context);

AgsInput* ags_input_next_active(AgsInput *input,
				GObject *parent_recycling_context);

AgsInput* ags_input_new(GObject *audio);

#endif /*__AGS_INPUT_H__*/
