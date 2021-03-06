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

#ifndef __AGS_STREAM_H__
#define __AGS_STREAM_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recall_id.h>

#define AGS_TYPE_STREAM                (ags_stream_get_type())
#define AGS_STREAM(obj)                (G_TYPE_CHECK_INSTANCE_CAST(obj, AGS_TYPE_STREAM, AgsStream))
#define AGS_STREAM_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_STREAM, AgsStream))

typedef struct _AgsStream AgsStream;
typedef struct _AgsStreamClass AgsStreamClass;

struct _AgsStream{
  AgsRecall recall;

  AgsAudioSignal *audio_signal;
};

struct _AgsStreamClass{
  AgsRecallClass recall;
};

GType ags_stream_get_type();

void ags_stream(AgsRecall *recall, AgsRecallID *recall_id, gpointer data);

AgsStream* ags_stream_new();

#endif /*__AGS_STREAM_H__*/
