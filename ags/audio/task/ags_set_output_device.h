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

#ifndef __AGS_SET_OUTPUT_DEVICE_H__
#define __AGS_SET_OUTPUT_DEVICE_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task.h>

#define AGS_TYPE_SET_OUTPUT_DEVICE                (ags_set_output_device_get_type())
#define AGS_SET_OUTPUT_DEVICE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SET_OUTPUT_DEVICE, AgsSetOutputDevice))
#define AGS_SET_OUTPUT_DEVICE_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SET_OUTPUT_DEVICE, AgsSetOutputDeviceClass))
#define AGS_IS_SET_OUTPUT_DEVICE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_SET_OUTPUT_DEVICE))
#define AGS_IS_SET_OUTPUT_DEVICE_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_SET_OUTPUT_DEVICE))
#define AGS_SET_OUTPUT_DEVICE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_SET_OUTPUT_DEVICE, AgsSetOutputDeviceClass))

typedef struct _AgsSetOutputDevice AgsSetOutputDevice;
typedef struct _AgsSetOutputDeviceClass AgsSetOutputDeviceClass;

struct _AgsSetOutputDevice
{
  AgsTask task;

  GObject *soundcard;
  char *device;
};

struct _AgsSetOutputDeviceClass
{
  AgsTaskClass task;
};

GType ags_set_output_device_get_type();

AgsSetOutputDevice* ags_set_output_device_new(GObject *soundcard,
					      char *device);

#endif /*__AGS_SET_OUTPUT_DEVICE_H__*/
