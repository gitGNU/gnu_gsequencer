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

#ifndef __AGS_INIT_AUDIO_H__
#define __AGS_INIT_AUDIO_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task.h>
#include <ags/audio/ags_audio.h>

#define AGS_TYPE_INIT_AUDIO                (ags_init_audio_get_type())
#define AGS_INIT_AUDIO(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_INIT_AUDIO, AgsInitAudio))
#define AGS_INIT_AUDIO_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_INIT_AUDIO, AgsInitAudioClass))
#define AGS_IS_INIT_AUDIO(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_INIT_AUDIO))
#define AGS_IS_INIT_AUDIO_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_INIT_AUDIO))
#define AGS_INIT_AUDIO_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_INIT_AUDIO, AgsInitAudioClass))

typedef struct _AgsInitAudio AgsInitAudio;
typedef struct _AgsInitAudioClass AgsInitAudioClass;

struct _AgsInitAudio
{
  AgsTask task;

  AgsAudio *audio;
  gboolean do_playback;
  gboolean do_sequencer;
  gboolean do_notation;
};

struct _AgsInitAudioClass
{
  AgsTaskClass task;
};

GType ags_init_audio_get_type();

AgsInitAudio* ags_init_audio_new(AgsAudio *audio,
				 gboolean do_playback, gboolean do_sequencer, gboolean do_notation);

#endif /*__AGS_INIT_AUDIO_H__*/
