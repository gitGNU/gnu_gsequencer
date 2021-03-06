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

#ifndef __AGS_RECYCLING_THREAD_H__
#define __AGS_RECYCLING_THREAD_H__

#include <glib.h>
#include <glib-object.h>

#ifdef AGS_USE_LINUX_THREADS
#include <ags/thread/ags_thread-kthreads.h>
#else
#include <ags/thread/ags_thread-posix.h>
#endif 

#include <ags/audio/ags_recall_id.h>

#define AGS_TYPE_RECYCLING_THREAD                (ags_recycling_thread_get_type())
#define AGS_RECYCLING_THREAD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECYCLING_THREAD, AgsRecyclingThread))
#define AGS_RECYCLING_THREAD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_RECYCLING_THREAD, AgsRecyclingThread))
#define AGS_IS_RECYCLING_THREAD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_RECYCLING_THREAD))
#define AGS_IS_RECYCLING_THREAD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_RECYCLING_THREAD))
#define AGS_RECYCLING_THREAD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_RECYCLING_THREAD, AgsRecyclingThreadClass))

#define AGS_RECYCLING_THREAD_DEFAULT_JIFFIE (100)

typedef struct _AgsRecyclingThread AgsRecyclingThread;
typedef struct _AgsRecyclingThreadClass AgsRecyclingThreadClass;
typedef struct _AgsRecyclingThreadWorker AgsRecyclingThreadWorker;

typedef enum{
  AGS_RECYCLING_THREAD_WAIT           = 1,
  AGS_RECYCLING_THREAD_DONE           = 1 <<  1,
  AGS_RECYCLING_THREAD_LOCKED         = 1 <<  2,
  AGS_RECYCLING_THREAD_LOCKED_PARENT  = 1 <<  3,
  AGS_RECYCLING_THREAD_WORKER_WAIT    = 1 <<  4,
  AGS_RECYCLING_THREAD_WORKER_DONE    = 1 <<  5,
}AgsRecyclingThreadFlags;

struct _AgsRecyclingThread
{
  AgsThread thread;

  volatile guint flags;

  AgsThread *iterator_thread;

  pthread_mutex_t *iteration_mutex;
  pthread_cond_t *iteration_cond;

  pthread_t *worker_queue;
  
  pthread_mutex_t *worker_mutex;
  pthread_cond_t *worker_cond;

  GObject *first_recycling;
  GObject *last_recycling;
  GList *worker;
};

struct _AgsRecyclingThreadClass
{
  AgsThreadClass thread;

  void (*play_channel)(AgsRecyclingThread *recycling_thread,
		       GObject *channel,
		       AgsRecallID *recall_id,
		       gint stage);

  void (*play_audio)(AgsRecyclingThread *recycling_thread,
		     GObject *output, GObject *audio,
		     AgsRecallID *recall_id,
		     gint stage);
};

struct _AgsRecyclingThreadWorker
{
  AgsRecyclingThread *recycling_thread;

  GObject *audio;
  GObject *channel;

  AgsRecallID *recall_id;
  gint stage;

  gboolean audio_worker;
};

GType ags_recycling_thread_get_type();

AgsRecyclingThreadWorker* ags_recycling_thread_worker_alloc(AgsRecyclingThread *recycling_thread,
							    GObject *audio,
							    GObject *channel,
							    AgsRecallID *recall_id,
							    gint stage,
							    gboolean audio_worker);

void ags_recycling_thread_add_worker(AgsRecyclingThread *recycling_thread,
				     AgsRecyclingThreadWorker *worker);
void ags_recycling_thread_remove_worker(AgsRecyclingThread *recycling_thread,
					AgsRecyclingThreadWorker *worker);

void ags_recycling_thread_play_channel(AgsRecyclingThread *recycling_thread,
				       GObject *channel,
				       AgsRecallID *recall_id,
				       gint stage);
void ags_recycling_thread_play_audio(AgsRecyclingThread *recycling_thread,
				     GObject *output, GObject *audio,
				     AgsRecallID *recall_id,
				     gint stage);

AgsRecyclingThread* ags_recycling_thread_find_child(AgsRecyclingThread *recycling_thread,
						    GObject *first_recycling);

void ags_recycling_thread_fifo(AgsRecyclingThread *recycling_thread);

AgsRecyclingThread* ags_recycling_thread_new(GObject *first_recycling,
					     GObject *last_recycling);

#endif /*__AGS_RECYCLING_THREAD_H__*/
