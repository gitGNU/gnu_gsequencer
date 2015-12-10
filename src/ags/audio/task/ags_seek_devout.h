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

#ifndef __AGS_SEEK_DEVOUT_H__
#define __AGS_SEEK_DEVOUT_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_task.h>
#include <ags/audio/ags_devout.h>

#define AGS_TYPE_SEEK_DEVOUT                (ags_seek_devout_get_type())
#define AGS_SEEK_DEVOUT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SEEK_DEVOUT, AgsSeekDevout))
#define AGS_SEEK_DEVOUT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SEEK_DEVOUT, AgsSeekDevoutClass))
#define AGS_IS_SEEK_DEVOUT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_SEEK_DEVOUT))
#define AGS_IS_SEEK_DEVOUT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_SEEK_DEVOUT))
#define AGS_SEEK_DEVOUT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_SEEK_DEVOUT, AgsSeekDevoutClass))

typedef struct _AgsSeekDevout AgsSeekDevout;
typedef struct _AgsSeekDevoutClass AgsSeekDevoutClass;

struct _AgsSeekDevout
{
  AgsTask task;

  GObject *devout;
  guint steps;
  gboolean move_forward;
};

struct _AgsSeekDevoutClass
{
  AgsTaskClass task;
};

GType ags_seek_devout_get_type();

AgsSeekDevout* ags_seek_devout_new(GObject *devout,
				   guint steps,
				   gboolean move_forward);

#endif /*__AGS_SEEK_DEVOUT_H__*/