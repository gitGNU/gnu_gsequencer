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

#include <ags/audio/thread/ags_recycling_thread.h>

#include <ags/object/ags_marshal.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_concurrent_tree.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_returnable_thread.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_recall_id.h>

#include <ags/audio/thread/ags_iterator_thread.h>

#include <math.h>

void ags_recycling_thread_class_init(AgsRecyclingThreadClass *recycling_thread);
void ags_recycling_thread_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recycling_thread_init(AgsRecyclingThread *recycling_thread);
void ags_recycling_thread_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_recycling_thread_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_recycling_thread_connect(AgsConnectable *connectable);
void ags_recycling_thread_disconnect(AgsConnectable *connectable);
void ags_recycling_thread_finalize(GObject *gobject);
void ags_recycling_thread_queue(void *data);

void ags_recycling_thread_start(AgsThread *thread);
void ags_recycling_thread_run(AgsThread *thread);
void ags_recycling_thread_real_play_channel(AgsRecyclingThread *recycling_thread,
					    GObject *channel,
					    AgsRecallID *recall_id,
					    gint stage);
void ags_recycling_thread_real_play_audio(AgsRecyclingThread *recycling_thread,
					  GObject *output, GObject *audio,
					  AgsRecallID *recall_id,
					  gint stage);

void ags_recycling_thread_play_channel_worker(AgsRecyclingThread *recycling_thread,
					      GObject *channel,
					      AgsRecallID *recall_id,
					      gint stage);
void ags_recycling_thread_play_audio_worker(AgsRecyclingThread *recycling_thread,
					    GObject *output, GObject *audio,
					    AgsRecallID *recall_id,
					    gint stage);

enum{
  PROP_0,
  PROP_ITERATOR_THREAD,
  PROP_FIRST_RECYCLING,
  PROP_LAST_RECYCLING,
};

enum{
  PLAY_AUDIO,
  PLAY_CHANNEL,
  LAST_SIGNAL,
};

static gpointer ags_recycling_thread_parent_class = NULL;
static AgsConnectableInterface *ags_recycling_thread_parent_connectable_interface;

static guint recycling_thread_signals[LAST_SIGNAL];

GType
ags_recycling_thread_get_type()
{
  static GType ags_type_recycling_thread = 0;

  if(!ags_type_recycling_thread){
    static const GTypeInfo ags_recycling_thread_info = {
      sizeof (AgsRecyclingThreadClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recycling_thread_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecyclingThread),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recycling_thread_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recycling_thread_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_recycling_thread = g_type_register_static(AGS_TYPE_THREAD,
						       "AgsRecyclingThread\0",
						       &ags_recycling_thread_info,
						       0);
    
    g_type_add_interface_static(ags_type_recycling_thread,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_recycling_thread);
}

void
ags_recycling_thread_class_init(AgsRecyclingThreadClass *recycling_thread)
{
  GObjectClass *gobject;
  AgsThreadClass *thread;
  GParamSpec *param_spec;

  ags_recycling_thread_parent_class = g_type_class_peek_parent(recycling_thread);

  /* GObject */
  gobject = (GObjectClass *) recycling_thread;

  gobject->set_property = ags_recycling_thread_set_property;
  gobject->get_property = ags_recycling_thread_get_property;

  gobject->finalize = ags_recycling_thread_finalize;

  /* properties */
  param_spec = g_param_spec_object("iterator-thread\0",
				   "assigned iterator thread\0",
				   "The iterator thread object it is assigned to\0",
				    AGS_TYPE_ITERATOR_THREAD,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
   g_object_class_install_property(gobject,
				   PROP_ITERATOR_THREAD,
				   param_spec);

   param_spec = g_param_spec_object("first-recycling\0",
				    "assigned first recycling\0",
				    "The first recycling to acquire lock\0",
				    AGS_TYPE_RECYCLING,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
   g_object_class_install_property(gobject,
				   PROP_FIRST_RECYCLING,
				   param_spec);

   param_spec = g_param_spec_object("last-recycling\0",
				    "assigned last recycling\0",
				    "The last recycling to acquire lock\0",
				    AGS_TYPE_RECYCLING,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
   g_object_class_install_property(gobject,
				   PROP_LAST_RECYCLING,
				   param_spec);

  /* AgsThread */
  thread = (AgsThreadClass *) recycling_thread;

  thread->start = ags_recycling_thread_start;
  thread->run = ags_recycling_thread_run;
  
  /* AgsRecyclingThread */
  recycling_thread->play_channel = ags_recycling_thread_play_channel;
  recycling_thread->play_audio = ags_recycling_thread_play_audio;

  /* signals */
  recycling_thread_signals[PLAY_CHANNEL] = 
    g_signal_new("play_channel\0",
		 G_TYPE_FROM_CLASS(recycling_thread),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsRecyclingThreadClass, play_channel),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__OBJECT_OBJECT_OBJECT_INT_BOOLEAN,
		 G_TYPE_NONE, 5,
		 G_TYPE_OBJECT,
		 G_TYPE_OBJECT,
		 G_TYPE_OBJECT,
		 G_TYPE_INT, G_TYPE_BOOLEAN);

  recycling_thread_signals[PLAY_AUDIO] = 
    g_signal_new("play_audio\0",
		 G_TYPE_FROM_CLASS(recycling_thread),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsRecyclingThreadClass, play_audio),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_OBJECT_UINT_INT_BOOLEAN,
		 G_TYPE_NONE, 9,
		 G_TYPE_OBJECT,
		 G_TYPE_OBJECT, G_TYPE_OBJECT,
		 G_TYPE_OBJECT, G_TYPE_OBJECT,
		 G_TYPE_OBJECT, G_TYPE_UINT,
		 G_TYPE_INT, G_TYPE_BOOLEAN);
}

void
ags_recycling_thread_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_recycling_thread_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recycling_thread_connect;
  connectable->disconnect = ags_recycling_thread_disconnect;
}

void
ags_recycling_thread_init(AgsRecyclingThread *recycling_thread)
{
  recycling_thread->flags = 0;

  recycling_thread->iterator_thread = NULL;

  recycling_thread->iteration_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(recycling_thread->iteration_mutex, NULL);

  recycling_thread->iteration_cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(recycling_thread->iteration_cond, NULL);

  recycling_thread->worker_queue = (pthread_t *) malloc(sizeof(pthread_t));

  recycling_thread->worker_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(recycling_thread->worker_mutex, NULL);

  recycling_thread->worker_cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(recycling_thread->worker_cond, NULL);

  recycling_thread->first_recycling = NULL;
  recycling_thread->last_recycling = NULL;
  recycling_thread->worker = NULL;
}


void
ags_recycling_thread_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
   AgsRecyclingThread *recycling_thread;

   recycling_thread = AGS_RECYCLING_THREAD(gobject);

   switch(prop_id){
   case PROP_ITERATOR_THREAD:
     {
       AgsThread *iterator_thread;

       iterator_thread = (AgsThread *) g_value_get_object(value);

       if(recycling_thread->iterator_thread == iterator_thread){
	 return;
       }

       if(recycling_thread->iterator_thread != NULL){
	 g_object_unref(recycling_thread->iterator_thread);
       }
       
       if(iterator_thread != NULL){
	 g_object_ref(iterator_thread);
       }
       
       recycling_thread->iterator_thread = iterator_thread;
     }
     break;
   case PROP_FIRST_RECYCLING:
     {
       AgsRecycling *first_recycling;

       first_recycling = (AgsRecycling *) g_value_get_object(value);

       if(recycling_thread->first_recycling == first_recycling){
	 return;
       }

       if(recycling_thread->first_recycling != NULL){
	 g_object_unref(recycling_thread->first_recycling);
       }
       
       if(first_recycling != NULL){
	 g_object_ref(first_recycling);
       }
       
       recycling_thread->first_recycling = first_recycling;
     }
     break;
   case PROP_LAST_RECYCLING:
     {
       AgsRecycling *last_recycling;

       last_recycling = (AgsRecycling *) g_value_get_object(value);

       if(recycling_thread->last_recycling == last_recycling){
	 return;
       }

       if(recycling_thread->last_recycling != NULL){
	 g_object_unref(recycling_thread->last_recycling);
       }
       
       if(last_recycling != NULL){
	 g_object_ref(last_recycling);
       }
       
       recycling_thread->last_recycling = last_recycling;
     }
     break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recycling_thread_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec)
{
  AgsRecyclingThread *recycling_thread;

  recycling_thread = AGS_RECYCLING_THREAD(gobject);

  switch(prop_id){
  case PROP_ITERATOR_THREAD:
    g_value_set_object(value, recycling_thread->iterator_thread);
    break;
  case PROP_FIRST_RECYCLING:
    g_value_set_object(value, recycling_thread->first_recycling);
    break;
  case PROP_LAST_RECYCLING:
    g_value_set_object(value, recycling_thread->last_recycling);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recycling_thread_connect(AgsConnectable *connectable)
{
  AgsThread *thread;

  thread = AGS_THREAD(connectable);

  ags_recycling_thread_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_recycling_thread_disconnect(AgsConnectable *connectable)
{
  ags_recycling_thread_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_recycling_thread_finalize(GObject *gobject)
{
  AgsRecyclingThread *recycling_thread;

  recycling_thread = AGS_RECYCLING_THREAD(gobject);

  /*  */
  G_OBJECT_CLASS(ags_recycling_thread_parent_class)->finalize(gobject);
}

void
ags_recycling_thread_queue(void *data)
{
  AgsRecyclingThread *recycling_thread;

  AgsRecyclingThreadWorker *worker;
  
  GList *list;

  guint flags;

  recycling_thread = AGS_RECYCLING_THREAD(data);

  while((AGS_THREAD_RUNNING & (g_atomic_int_get(&(recycling_thread->flags)))) != 0){
    /* wait until fifo is available */
    pthread_mutex_lock(recycling_thread->worker_mutex);

    g_atomic_int_and(&(recycling_thread->flags),
		     ~AGS_RECYCLING_THREAD_WORKER_DONE);
    
    flags = g_atomic_int_get(&(recycling_thread->flags));
    
    if((AGS_RECYCLING_THREAD_WORKER_WAIT & (flags)) != 0 &&
       (AGS_RECYCLING_THREAD_WORKER_DONE & (flags)) == 0){

      while((AGS_RECYCLING_THREAD_WORKER_WAIT & (flags)) != 0 &&
	    (AGS_RECYCLING_THREAD_WORKER_DONE & (flags)) == 0){
	pthread_cond_wait(recycling_thread->worker_cond,
			  recycling_thread->worker_mutex);
	
	flags = g_atomic_int_get(&(recycling_thread->flags));
      }
    }

    g_atomic_int_or(&(recycling_thread->flags),
		    (AGS_RECYCLING_THREAD_WORKER_WAIT |
		     AGS_RECYCLING_THREAD_WORKER_DONE));
    
    pthread_mutex_unlock(recycling_thread->worker_mutex);

    /* lock context */
    ags_concurrent_tree_lock_context(AGS_CONCURRENT_TREE(recycling_thread->first_recycling));
        
    /* process worker */
    list = g_list_reverse(recycling_thread->worker);
  
    while(list != NULL){
      worker = list->data;
      
      if(worker->audio_worker){
	ags_recycling_thread_play_audio_worker(recycling_thread,
					       worker->channel, worker->audio,
					       worker->recall_id,
					       worker->stage);
      }else{
	ags_recycling_thread_play_channel_worker(recycling_thread,
						 worker->channel,
						 worker->recall_id,
						 worker->stage);
      }

      list = list->next;
    }

    /* clear worker */
    g_list_free_full(recycling_thread->worker,
		     free);
  
    recycling_thread->worker = NULL;

    /* unlock context */
    ags_concurrent_tree_unlock_context(AGS_CONCURRENT_TREE(recycling_thread->first_recycling));

    /* notify iterator thread, signal fifo */
    ags_iterator_thread_children_ready(recycling_thread->iterator_thread,
				       recycling_thread);
  }
}

void
ags_recycling_thread_start(AgsThread *thread)
{
  AgsRecyclingThread *recycling_thread;

  recycling_thread = AGS_RECYCLING_THREAD(thread);

  pthread_create(recycling_thread->worker_queue, NULL,
		 &ags_recycling_thread_queue, thread);
  
  AGS_THREAD_CLASS(ags_recycling_thread_parent_class)->start(thread);
}

void
ags_recycling_thread_run(AgsThread *thread)
{
  AgsRecyclingThread *recycling_thread;

  guint flags;
  guint i;
  
  recycling_thread = AGS_RECYCLING_THREAD(thread);

  for(i = 0; i < 3; i++){
    /* wait for next run */
    ags_recycling_thread_fifo(recycling_thread);

    /* signal worker */
    pthread_mutex_lock(recycling_thread->worker_mutex);

    g_atomic_int_and(&(recycling_thread->flags),
		     ~AGS_RECYCLING_THREAD_WORKER_WAIT);
    
    flags = g_atomic_int_get(&(recycling_thread->flags));

    if((AGS_RECYCLING_THREAD_WORKER_DONE & (flags)) == 0){
      pthread_cond_signal(recycling_thread->worker_cond);
    }
    
    pthread_mutex_unlock(recycling_thread->worker_mutex);
  }
}

AgsRecyclingThreadWorker*
ags_recycling_thread_worker_alloc(AgsRecyclingThread *recycling_thread,
				  GObject *audio,
				  GObject *channel,
				  AgsRecallID *recall_id,
				  gint stage,
				  gboolean audio_worker)
{
  AgsRecyclingThreadWorker *worker;

  worker = (AgsRecyclingThreadWorker *) malloc(sizeof(AgsRecyclingThreadWorker));

  worker->audio = audio;
  worker->channel = channel;
  
  worker->recall_id = recall_id;
  worker->stage = stage;

  worker->audio_worker = audio_worker;
  
  return(worker);
}

void
ags_recycling_thread_add_worker(AgsRecyclingThread *recycling_thread,
				AgsRecyclingThreadWorker *worker)
{
  recycling_thread->worker = g_list_prepend(recycling_thread,
					    worker);
}

void
ags_recycling_thread_remove_worker(AgsRecyclingThread *recycling_thread,
				   AgsRecyclingThreadWorker *worker)
{
  recycling_thread->worker = g_list_remove(recycling_thread,
					   worker);
}

void
ags_recycling_thread_real_play_channel(AgsRecyclingThread *recycling_thread,
				       GObject *channel,
				       AgsRecallID *recall_id,
				       gint stage)
{
  ags_recycling_thread_add_worker(recycling_thread,
				  ags_recycling_thread_worker_alloc(recycling_thread,
								    NULL,
								    channel,
								    recall_id,
								    stage,
								    FALSE));
}

void
ags_recycling_thread_play_channel(AgsRecyclingThread *recycling_thread,
				  GObject *channel,
				  AgsRecallID *recall_id,
				  gint stage)
{
  g_return_if_fail(AGS_IS_RECYCLING_THREAD(recycling_thread));
  g_return_if_fail(AGS_IS_CHANNEL(channel));

  g_object_ref((GObject *) recycling_thread);
  g_signal_emit(G_OBJECT(recycling_thread),
		recycling_thread_signals[PLAY_CHANNEL], 0,
		channel,
		recall_id,
		stage);
  g_object_unref((GObject *) recycling_thread);
}

void
ags_recycling_thread_real_play_audio(AgsRecyclingThread *recycling_thread,
				     GObject *output, GObject *audio,
				     AgsRecallID *recall_id,
				     gint stage)
{
  ags_recycling_thread_add_worker(recycling_thread,
				  ags_recycling_thread_worker_alloc(recycling_thread,
								    audio,
								    output,
								    recall_id,
								    stage,
								    TRUE));
}

void
ags_recycling_thread_play_audio(AgsRecyclingThread *recycling_thread,
				GObject *output, GObject *audio,
				AgsRecallID *recall_id,
				gint stage)
{
  g_return_if_fail(AGS_IS_RECYCLING_THREAD(recycling_thread));
  g_return_if_fail(AGS_IS_AUDIO(audio));
  g_return_if_fail(AGS_IS_CHANNEL(output));

  g_object_ref((GObject *) recycling_thread);
  g_signal_emit(G_OBJECT(recycling_thread),
		recycling_thread_signals[PLAY_CHANNEL], 0,
		output, audio,
		recall_id,
		stage);
  g_object_unref((GObject *) recycling_thread);
}

void
ags_recycling_thread_play_channel_worker(AgsRecyclingThread *recycling_thread,
					 GObject *channel,
					 AgsRecallID *recall_id,
					 gint stage)
{
  ags_channel_play(AGS_CHANNEL(channel),
		   recall_id,
		   stage);
}

void
ags_recycling_thread_play_audio_worker(AgsRecyclingThread *recycling_thread,
				       GObject *output, GObject *audio,
				       AgsRecallID *recall_id,
				       gint stage)
{
  AgsChannel *input;
  
  if((AGS_AUDIO_ASYNC & (AGS_AUDIO(audio)->flags)) != 0){
    input = ags_channel_nth(AGS_AUDIO(audio)->input,
			    AGS_CHANNEL(output)->audio_channel);
  }else{
    input = ags_channel_nth(AGS_AUDIO(audio)->input,
			    AGS_CHANNEL(output)->line);
  }

  if((AGS_AUDIO_OUTPUT_HAS_RECYCLING & (AGS_AUDIO(audio)->flags)) != 0){
    AgsRecallID *input_recall_id;
    gint child_position;

    /* input_recall_id - check if there is a new recycling */
    child_position = ags_recycling_context_find_child(recall_id->recycling_context,
						      input->first_recycling);
      
    if(child_position == -1){
      input_recall_id = ags_recall_id_find_recycling_context(input->recall_id,
							     recall_id->recycling_context);
    }else{
      GList *list;

      list = g_list_nth(recall_id->recycling_context->children,
			child_position);

      if(list != NULL){
	input_recall_id = ags_recall_id_find_recycling_context(input->recall_id,
							       AGS_RECYCLING_CONTEXT(list->data));
      }else{
	input_recall_id = NULL;
      }
    }

    ags_audio_play(AGS_AUDIO(audio),
		   input_recall_id,
		   stage);
  }
    
  ags_audio_play(AGS_AUDIO(audio),
		 recall_id,
		 stage);
}

AgsRecyclingThread*
ags_recycling_thread_find_child(AgsRecyclingThread *recycling_thread,
				GObject *first_recycling)
{
  //TODO:JK: implement me
  
  return(NULL);
}

void
ags_recycling_thread_fifo(AgsRecyclingThread *recycling_thread)
{
  guint flags;
  
  pthread_mutex_lock(recycling_thread->iteration_mutex);

  g_atomic_int_or(&(recycling_thread->flags),
		  AGS_RECYCLING_THREAD_WAIT);

  flags = g_atomic_int_get(&(recycling_thread->flags));
  
  while((AGS_RECYCLING_THREAD_WAIT & (flags)) != 0 &&
	(AGS_RECYCLING_THREAD_DONE & (flags)) == 0){
    pthread_cond_wait(recycling_thread->iteration_cond,
		      recycling_thread->iteration_mutex);

    flags = g_atomic_int_get(&(recycling_thread->flags));
  }
  
  pthread_mutex_unlock(recycling_thread->iteration_mutex);
}

AgsRecyclingThread*
ags_recycling_thread_new(GObject *first_recycling,
			 GObject *last_recycling)
{
  AgsRecyclingThread *recycling_thread;
  
  recycling_thread = (AgsRecyclingThread *) g_object_new(AGS_TYPE_RECYCLING_THREAD,
							 "first-recycling\0", first_recycling,
							 "last-recycling\0", last_recycling,
							 NULL);

  return(recycling_thread);
}