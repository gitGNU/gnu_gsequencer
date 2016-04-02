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

#include <ags/thread/ags_task_thread.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_main_loop.h>
#include <ags/object/ags_async_queue.h>

#include <ags/thread/ags_concurrency_provider.h>
#include <ags/thread/ags_returnable_thread.h>

#include <sys/types.h>

#include <fontconfig/fontconfig.h>
#include <math.h>

void ags_task_thread_class_init(AgsTaskThreadClass *task_thread);
void ags_task_thread_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_task_thread_async_queue_interface_init(AgsAsyncQueueInterface *async_queue);
void ags_task_thread_init(AgsTaskThread *task_thread);
void ags_task_thread_connect(AgsConnectable *connectable);
void ags_task_thread_disconnect(AgsConnectable *connectable);
void ags_task_thread_finalize(GObject *gobject);
void ags_task_thread_set_run_mutex(AgsAsyncQueue *async_queue, pthread_mutex_t *run_mutex);
pthread_mutex_t* ags_task_thread_get_run_mutex(AgsAsyncQueue *async_queue);
void ags_task_thread_set_run_cond(AgsAsyncQueue *async_queue, pthread_cond_t *run_cond);
pthread_cond_t* ags_task_thread_get_run_cond(AgsAsyncQueue *async_queue);
void ags_task_thread_set_run(AgsAsyncQueue *async_queue, gboolean is_run);
gboolean ags_task_thread_is_run(AgsAsyncQueue *async_queue);

void ags_task_thread_start(AgsThread *thread);
void ags_task_thread_run(AgsThread *thread);

void ags_task_thread_append_task_queue(AgsReturnableThread *returnable_thread, gpointer data);
void ags_task_thread_append_tasks_queue(AgsReturnableThread *returnable_thread, gpointer data);

/**
 * SECTION:ags_task_thread
 * @short_description: task thread
 * @title: AgsTaskThread
 * @section_id:
 * @include: ags/thread/ags_task_thread.h
 *
 * The #AgsTaskThread acts as task queue thread.
 */

static gpointer ags_task_thread_parent_class = NULL;
static AgsConnectableInterface *ags_task_thread_parent_connectable_interface;

GType
ags_task_thread_get_type()
{
  static GType ags_type_task_thread = 0;

  if(!ags_type_task_thread){
    static const GTypeInfo ags_task_thread_info = {
      sizeof (AgsTaskThreadClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_task_thread_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsTaskThread),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_task_thread_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_task_thread_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    static const GInterfaceInfo ags_async_queue_interface_info = {
      (GInterfaceInitFunc) ags_task_thread_async_queue_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_task_thread = g_type_register_static(AGS_TYPE_THREAD,
						  "AgsTaskThread\0",
						  &ags_task_thread_info,
						  0);
    
    g_type_add_interface_static(ags_type_task_thread,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_task_thread,
				AGS_TYPE_ASYNC_QUEUE,
				&ags_async_queue_interface_info);
  }
  
  return (ags_type_task_thread);
}

void
ags_task_thread_class_init(AgsTaskThreadClass *task_thread)
{
  GObjectClass *gobject;
  AgsThreadClass *thread;

  ags_task_thread_parent_class = g_type_class_peek_parent(task_thread);

  /* GObject */
  gobject = (GObjectClass *) task_thread;

  gobject->finalize = ags_task_thread_finalize;

  /* AgsThread */
  thread = (AgsThreadClass *) task_thread;

  thread->start = ags_task_thread_start;
  thread->run = ags_task_thread_run;
}

void
ags_task_thread_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_task_thread_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_task_thread_connect;
  connectable->disconnect = ags_task_thread_disconnect;
}

void
ags_task_thread_async_queue_interface_init(AgsAsyncQueueInterface *async_queue)
{
  async_queue->set_run_mutex = ags_task_thread_set_run_mutex;
  async_queue->get_run_mutex = ags_task_thread_get_run_mutex;

  async_queue->set_run_cond = ags_task_thread_set_run_cond;
  async_queue->get_run_cond = ags_task_thread_get_run_cond;

  async_queue->set_run = ags_task_thread_set_run;
  async_queue->is_run = ags_task_thread_is_run;
}

void
ags_task_thread_init(AgsTaskThread *task_thread)
{
  AgsThread *thread;

  pthread_mutexattr_t mutexattr;

  thread = AGS_THREAD(task_thread);

  //  g_atomic_int_or(&(thread->flags),
  //		  AGS_THREAD_LOCK_GREEDY_RUN_MUTEX);

  thread->freq = AGS_TASK_THREAD_DEFAULT_JIFFIE;

  /* async queue */
  g_atomic_int_set(&(task_thread->is_run),
		   FALSE);

  pthread_mutexattr_init(&(mutexattr));
  pthread_mutexattr_settype(&(mutexattr), PTHREAD_MUTEX_RECURSIVE);

  task_thread->run_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(task_thread->run_mutex, &mutexattr);

  task_thread->run_cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(task_thread->run_cond, NULL);
  
  /*  */
  task_thread->read_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(task_thread->read_mutex, NULL);

  task_thread->launch_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(task_thread->launch_mutex, NULL);

  g_atomic_int_set(&(task_thread->queued),
		   0);
  g_atomic_int_set(&(task_thread->pending),
		   0);

  g_atomic_pointer_set(&(task_thread->exec), NULL);
  g_atomic_pointer_set(&(task_thread->queue),
		       NULL);

  task_thread->thread_pool = ags_thread_pool_new(task_thread);
  task_thread->thread_pool->parent = (AgsThread *) task_thread;
}

void
ags_task_thread_connect(AgsConnectable *connectable)
{
  ags_task_thread_parent_connectable_interface->connect(connectable);
}

void
ags_task_thread_disconnect(AgsConnectable *connectable)
{
  ags_task_thread_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_task_thread_finalize(GObject *gobject)
{
  AgsTaskThread *task_thread;

  task_thread = AGS_TASK_THREAD(gobject);

  /* free AgsTask lists */
  g_list_free_full(g_atomic_pointer_get(&(task_thread->exec)),
		   g_object_unref);
  g_list_free_full(g_atomic_pointer_get(&(task_thread->queue)),
		   g_object_unref);

  /*  */
  G_OBJECT_CLASS(ags_task_thread_parent_class)->finalize(gobject);
}

void
ags_task_thread_set_run_mutex(AgsAsyncQueue *async_queue, pthread_mutex_t *run_mutex)
{
  AGS_TASK_THREAD(async_queue)->run_mutex = run_mutex;
}

pthread_mutex_t*
ags_task_thread_get_run_mutex(AgsAsyncQueue *async_queue)
{
  return(AGS_TASK_THREAD(async_queue)->run_mutex);
}

void
ags_task_thread_set_run_cond(AgsAsyncQueue *async_queue, pthread_cond_t *run_cond)
{
  AGS_TASK_THREAD(async_queue)->run_cond = run_cond;
}

pthread_cond_t*
ags_task_thread_get_run_cond(AgsAsyncQueue *async_queue)
{
  return(AGS_TASK_THREAD(async_queue)->run_cond);
}

void
ags_task_thread_set_run(AgsAsyncQueue *async_queue, gboolean is_run)
{
  g_atomic_int_set(&(AGS_TASK_THREAD(async_queue)->is_run),
		   is_run);
}

gboolean
ags_task_thread_is_run(AgsAsyncQueue *async_queue)
{
  return(g_atomic_int_get(&(AGS_TASK_THREAD(async_queue)->is_run)));
}

void
ags_task_thread_start(AgsThread *thread)
{
  AgsTaskThread *task_thread;

  task_thread = AGS_TASK_THREAD(thread);

  if((AGS_THREAD_SINGLE_LOOP & (g_atomic_int_get(&(thread->flags)))) == 0){
    AGS_THREAD_CLASS(ags_task_thread_parent_class)->start(thread);
  }
}

void
ags_task_thread_run(AgsThread *thread)
{
  AgsTaskThread *task_thread;
  GList *list;
  guint prev_pending;
  static gboolean initialized = FALSE;
  
  task_thread = AGS_TASK_THREAD(thread);
  
  if(!initialized){
    //    play_idle.tv_sec = 0;
    //    play_idle.tv_nsec = 10 * round(sysconf(_SC_CLK_TCK) * (double) buffer_size  / (double) samplerate);
    //    idle = sysconf(_SC_CLK_TCK) * round(sysconf(_SC_CLK_TCK) * (double) buffer_size  / (double) samplerate / 8.0);

    initialized = TRUE;
  }

  /*  */
  pthread_mutex_lock(task_thread->read_mutex);

  g_atomic_pointer_set(&(task_thread->exec),
		       g_atomic_pointer_get(&(task_thread->queue)));
  g_atomic_pointer_set(&(task_thread->queue),
		       NULL);

  list = g_atomic_pointer_get(&(task_thread->exec));
    
  prev_pending = g_atomic_int_get(&(task_thread->pending));
  g_atomic_int_set(&(task_thread->pending),
		   g_list_length(list));
  g_atomic_int_set(&(task_thread->queued),
		   g_atomic_int_get(&(task_thread->queued)) - prev_pending);

  pthread_mutex_unlock(task_thread->read_mutex);

  /* launch tasks */
  if(list != NULL){
    AgsTask *task;

    GMainContext *main_context;
    
    int i;
    
    static GMutex mutex;
    static GCond cond;

    static gboolean initialized = FALSE;

    if(!initialized){
      initialized = TRUE;
      
      g_cond_init(&cond);
      g_mutex_init(&mutex);
    }
    
    main_context = g_main_context_default();

    if(!g_main_context_acquire(main_context)){
      gboolean got_ownership = FALSE;

      g_mutex_lock(&mutex);
      
      while(!got_ownership){
	got_ownership = g_main_context_wait(main_context,
					    &cond,
					    &mutex);
      }

      g_mutex_unlock(&mutex);
    }

    //    gdk_threads_enter();
    
    pthread_mutex_lock(task_thread->launch_mutex);
    //    pthread_mutex_lock(AGS_AUDIO_LOOP(thread->parent)->recall_mutex);

    for(i = 0; i < g_atomic_int_get(&(task_thread->pending)); i++){
      task = AGS_TASK(list->data);

#ifdef AGS_DEBUG
      g_message("ags_task_thread - launching task: %s\n\0", G_OBJECT_TYPE_NAME(task));
#endif

      ags_task_launch(task);
      
      list = list->next;
    }

    //    pthread_mutex_unlock(AGS_AUDIO_LOOP(thread->parent)->recall_mutex);
    pthread_mutex_unlock(task_thread->launch_mutex);
    
    //    gdk_threads_leave();
    
    g_main_context_release(main_context);
    
  }

  pthread_mutex_lock(task_thread->read_mutex);

  g_list_free_full(g_atomic_pointer_get(&(task_thread->exec)),
		   g_object_unref);
  g_atomic_pointer_set(&(task_thread->exec),
		       NULL);

  pthread_mutex_unlock(task_thread->read_mutex);

  /* async queue */
  pthread_mutex_lock(task_thread->run_mutex);
  
  ags_async_queue_set_run(AGS_ASYNC_QUEUE(task_thread),
			  TRUE);
	
  pthread_cond_broadcast(task_thread->run_cond);
  
  pthread_mutex_unlock(task_thread->run_mutex);
  
  /* clean-up */
  //  pango_fc_font_map_cache_clear(pango_cairo_font_map_get_default());
  //  pango_cairo_font_map_set_default(NULL);
  //  cairo_debug_reset_static_data();
  //  FcFini();
}

void
ags_task_thread_append_task_queue(AgsReturnableThread *returnable_thread, gpointer data)
{
  AgsTask *task;
  AgsTaskThread *task_thread;

  AgsTaskThreadAppend *append;
  GList *tmplist;

  gboolean initial_wait;
  int ret;

  g_atomic_int_and(&(AGS_THREAD(returnable_thread)->flags),
  		   (~AGS_THREAD_READY));

  append = (AgsTaskThreadAppend *) g_atomic_pointer_get(&(returnable_thread->safe_data));

  task_thread = g_atomic_pointer_get(&(append->task_thread));
  task = AGS_TASK(g_atomic_pointer_get(&(append->data)));

  //  free(append);

  /* lock */
  pthread_mutex_lock(task_thread->read_mutex);

  /* append to queue */
  g_atomic_int_set(&(task_thread->queued),
		   g_atomic_int_get(&(task_thread->queued)) + 1);

  tmplist = g_atomic_pointer_get(&(task_thread->queue));
  g_atomic_pointer_set(&(task_thread->queue),
		       g_list_append(tmplist, task));

  /* unlock */
  pthread_mutex_unlock(task_thread->read_mutex);
  /*  */
  //  g_message("ags_task_thread_append_task_thread ------------------------- %d\0", devout->append_task_suspend);
}

/**
 * ags_task_thread_append_task:
 * @task_thread: an #AgsTaskThread
 * @task: an #AgsTask
 *
 * Adds the task to @task_thread.
 *
 * Since: 0.4
 */
void
ags_task_thread_append_task(AgsTaskThread *task_thread, AgsTask *task)
{
  AgsTaskThreadAppend *append;
  AgsThread *thread;

  guint flags;
  
#ifdef AGS_DEBUG
  g_message("append task\0");
#endif

  append = (AgsTaskThreadAppend *) malloc(sizeof(AgsTaskThreadAppend));

  g_atomic_pointer_set(&(append->task_thread),
		       task_thread);
  g_atomic_pointer_set(&(append->data),
		       task);

  thread = ags_thread_pool_pull(task_thread->thread_pool);
  
  pthread_mutex_lock(AGS_RETURNABLE_THREAD(thread)->reset_mutex);

  g_atomic_pointer_set(&(AGS_RETURNABLE_THREAD(thread)->safe_data),
		       append);

  ags_returnable_thread_connect_safe_run(AGS_RETURNABLE_THREAD(thread),
					 ags_task_thread_append_task_queue);

  g_atomic_int_or(&(AGS_RETURNABLE_THREAD(thread)->flags),
		  AGS_RETURNABLE_THREAD_IN_USE);
    
  pthread_mutex_unlock(AGS_RETURNABLE_THREAD(thread)->reset_mutex);

  /* pass-through */
  pthread_mutex_lock(thread->mutex);
  
  //  g_atomic_int_or(&(thread->flags),
  //		  AGS_THREAD_READY);

  flags = g_atomic_int_get(&(thread->flags));
  
  if((AGS_THREAD_WAIT_0 & flags) != 0 ||
     (AGS_THREAD_WAIT_1 & flags) != 0 ||
     (AGS_THREAD_WAIT_2 & flags) != 0){
    pthread_cond_signal(thread->cond);
  }
  
  pthread_mutex_unlock(thread->mutex);
}

void
ags_task_thread_append_tasks_queue(AgsReturnableThread *returnable_thread, gpointer data)
{
  AgsTask *task;
  AgsTaskThread *task_thread;
  AgsTaskThreadAppend *append;
  GList *list, *tmplist;
  gboolean initial_wait;
  int ret;

  g_atomic_int_and(&(AGS_THREAD(returnable_thread)->flags),
  		   (~AGS_THREAD_READY));

  append = (AgsTaskThreadAppend *) g_atomic_pointer_get(&(returnable_thread->safe_data));

  task_thread = g_atomic_pointer_get(&(append->task_thread));
  list = (GList *) g_atomic_pointer_get(&(append->data));

  //  free(append);

  /* lock */
  pthread_mutex_lock(task_thread->read_mutex);

  /* append to queue */
  g_atomic_int_set(&(task_thread->queued),
		   g_atomic_int_get(&(task_thread->queued)) + g_list_length(list));

  tmplist = g_atomic_pointer_get(&(task_thread->queue));
  g_atomic_pointer_set(&(task_thread->queue),
		       g_list_concat(tmplist, list));

  /*  */
  pthread_mutex_unlock(task_thread->read_mutex);
}

/**
 * ags_task_thread_append_tasks:
 * @task_thread: an #AgsTaskThread
 * @list: a GList with #AgsTask as data
 *
 * Concats the list with @task_thread's internal task list. Don't
 * free the list you pass. It will be freed for you.
 *
 * Since: 0.4
 */
void
ags_task_thread_append_tasks(AgsTaskThread *task_thread, GList *list)
{
  AgsTaskThreadAppend *append;
  AgsThread *thread;

  guint flags;
  
#ifdef AGS_DEBUG
  g_message("append tasks\0");
#endif

  append = (AgsTaskThreadAppend *) malloc(sizeof(AgsTaskThreadAppend));

  g_atomic_pointer_set(&(append->task_thread),
		       task_thread);
  g_atomic_pointer_set(&(append->data),
		       list);

  thread = ags_thread_pool_pull(task_thread->thread_pool);

  pthread_mutex_lock(AGS_RETURNABLE_THREAD(thread)->reset_mutex);


  g_atomic_pointer_set(&(AGS_RETURNABLE_THREAD(thread)->safe_data),
		       append);
  
  ags_returnable_thread_connect_safe_run(AGS_RETURNABLE_THREAD(thread),
					 ags_task_thread_append_tasks_queue);
  g_atomic_int_or(&(AGS_RETURNABLE_THREAD(thread)->flags),
		  AGS_RETURNABLE_THREAD_IN_USE);
  
  pthread_mutex_unlock(AGS_RETURNABLE_THREAD(thread)->reset_mutex);

  /* pass-through */
  pthread_mutex_lock(thread->mutex);
  
  //  g_atomic_int_or(&(thread->flags),
  //		  AGS_THREAD_READY);

  flags = g_atomic_int_get(&(thread->flags));
  
  if((AGS_THREAD_WAIT_0 & flags) != 0 ||
     (AGS_THREAD_WAIT_1 & flags) != 0 ||
     (AGS_THREAD_WAIT_2 & flags) != 0){
    pthread_cond_signal(thread->cond);
  }
  
  pthread_mutex_unlock(thread->mutex);
}

/**
 * ags_task_thread_new:
 *
 * Create a new #AgsTaskThread.
 *
 * Returns: the new #AgsTaskThread
 *
 * Since: 0.4
 */ 
AgsTaskThread*
ags_task_thread_new()
{
  AgsTaskThread *task_thread;

  task_thread = (AgsTaskThread *) g_object_new(AGS_TYPE_TASK_THREAD,
					       NULL);


  return(task_thread);
}