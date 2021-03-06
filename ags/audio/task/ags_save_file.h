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

#ifndef __AGS_SAVE_FILE_H__
#define __AGS_SAVE_FILE_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task.h>
#include <ags/file/ags_file.h>

#define AGS_TYPE_SAVE_FILE                (ags_save_file_get_type())
#define AGS_SAVE_FILE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SAVE_FILE, AgsSaveFile))
#define AGS_SAVE_FILE_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SAVE_FILE, AgsSaveFileClass))
#define AGS_IS_SAVE_FILE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_SAVE_FILE))
#define AGS_IS_SAVE_FILE_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_SAVE_FILE))
#define AGS_SAVE_FILE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_SAVE_FILE, AgsSaveFileClass))

typedef struct _AgsSaveFile AgsSaveFile;
typedef struct _AgsSaveFileClass AgsSaveFileClass;

struct _AgsSaveFile
{
  AgsTask task;

  AgsFile *file;
};

struct _AgsSaveFileClass
{
  AgsTaskClass task;
};

GType ags_save_file_get_type();

AgsSaveFile* ags_save_file_new(AgsFile *file);

#endif /*__AGS_SAVE_FILE_H__*/
