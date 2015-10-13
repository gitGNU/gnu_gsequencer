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

#ifndef __AGS_AUTOSAVE_THREAD_H__
#define __AGS_AUTOSAVE_THREAD_H__

#include <glib.h>
#include <glib-object.h>

#ifdef AGS_USE_LINUX_THREADS
#include <ags/thread/ags_thread-kthreads.h>
#else
#include <ags/thread/ags_thread-posix.h>
#endif 

#define AGS_TYPE_AUTOSAVE_THREAD                (ags_autosave_thread_get_type())
#define AGS_AUTOSAVE_THREAD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_AUTOSAVE_THREAD, AgsAutosaveThread))
#define AGS_AUTOSAVE_THREAD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_AUTOSAVE_THREAD, AgsAutosaveThreadClass))
#define AGS_IS_AUTOSAVE_THREAD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_AUTOSAVE_THREAD))
#define AGS_IS_AUTOSAVE_THREAD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_AUTOSAVE_THREAD))
#define AGS_AUTOSAVE_THREAD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_AUTOSAVE_THREAD, AgsAutosaveThreadClass))

#define AGS_AUTOSAVE_THREAD_DEFAULT_JIFFIE (0.2)
#define AGS_AUTOSAVE_THREAD_DEFAULT_FILENAME "ags-autosaved.xml\0"

typedef struct _AgsAutosaveThread AgsAutosaveThread;
typedef struct _AgsAutosaveThreadClass AgsAutosaveThreadClass;

struct _AgsAutosaveThread
{
  AgsThread thread;

  volatile guint tic;
  volatile guint last_sync;

  GObject *application_context;

  guint delay;
  guint counter;
};

struct _AgsAutosaveThreadClass
{
  AgsThreadClass thread;
};

GType ags_autosave_thread_get_type();

AgsAutosaveThread* ags_autosave_thread_new(GObject *application_context);

#endif /*__AGS_AUTOSAVE_THREAD_H__*/
