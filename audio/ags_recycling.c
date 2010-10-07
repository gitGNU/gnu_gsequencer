#include "ags_recycling.h"

#include "../object/ags_marshal.h"

#include "ags_devout.h"
#include "ags_audio.h"
#include "ags_channel.h"

#include <string.h>
#include <math.h>

GType ags_recycling_get_type();
void ags_recycling_class_init(AgsRecyclingClass *recycling_class);
void ags_recycling_init(AgsRecycling *recycling);
void ags_recycling_finalize(GObject *gobject);

void ags_recycling_real_add_audio_signal(AgsRecycling *recycling,
					 AgsAudioSignal *audio_signal);
void ags_recycling_real_add_audio_signal_with_frame_count(AgsRecycling *recycling,
							  AgsAudioSignal *audio_signal,
							  guint frame_count);

void ags_recycling_real_remove_audio_signal(AgsRecycling *recycling,
					    AgsAudioSignal *audio_signal);

enum{
  ADD_AUDIO_SIGNAL,
  ADD_AUDIO_SIGNAL_WITH_FRAME_COUNT,
  REMOVE_AUDIO_SIGNAL,
  LAST_SIGNAL,
};

static gpointer ags_recycling_parent_class = NULL;
static guint recycling_signals[LAST_SIGNAL];

GType
ags_recycling_get_type (void)
{
  static GType ags_type_recycling = 0;

  if(!ags_type_recycling){
    static const GTypeInfo ags_recycling_info = {
      sizeof (AgsRecyclingClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recycling_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecycling),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recycling_init,
    };

    ags_type_recycling = g_type_register_static(G_TYPE_OBJECT,
						"AgsRecycling\0",
						&ags_recycling_info, 0);
  }

  return(ags_type_recycling);
}

void
ags_recycling_class_init(AgsRecyclingClass *recycling)
{
  GObjectClass *gobject;

  ags_recycling_parent_class = g_type_class_peek_parent(recycling);

  gobject = (GObjectClass *) recycling;
  gobject->finalize = ags_recycling_finalize;

  recycling->add_audio_signal = ags_recycling_real_add_audio_signal;
  recycling->add_audio_signal_with_frame_count = ags_recycling_real_add_audio_signal_with_frame_count;
  recycling->remove_audio_signal = ags_recycling_real_remove_audio_signal;

  recycling_signals[ADD_AUDIO_SIGNAL] =
    g_signal_new("add_audio_signal\0",
		 G_TYPE_FROM_CLASS (recycling),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsRecyclingClass, add_audio_signal),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1,
		 G_TYPE_OBJECT);

  recycling_signals[ADD_AUDIO_SIGNAL_WITH_FRAME_COUNT] =
    g_signal_new("add_audio_signal_with_frame_count\0",
		 G_TYPE_FROM_CLASS (recycling),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsRecyclingClass, add_audio_signal_with_frame_count),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__OBJECT_UINT,
		 G_TYPE_NONE, 2,
		 G_TYPE_OBJECT, G_TYPE_UINT);


  recycling_signals[REMOVE_AUDIO_SIGNAL] =
    g_signal_new("remove_audio_signal\0",
		 G_TYPE_FROM_CLASS (recycling),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsRecyclingClass, remove_audio_signal),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1,
		 G_TYPE_OBJECT);
}

void
ags_recycling_init(AgsRecycling *recycling)
{
  //  recycling->flags = 0;

  recycling->channel = NULL;

  recycling->parent = NULL;

  recycling->next = NULL;
  recycling->prev = NULL;

  recycling->audio_signal = NULL;
}

void
ags_recycling_finalize(GObject *gobject)
{
  AgsRecycling *recycling;
  GList *list, *list_next;

  recycling = AGS_RECYCLING(gobject);

  /* AgsAudioSignal */
  list = recycling->audio_signal;

  while(list != NULL){
    list_next = list->next;

    g_object_unref(G_OBJECT(list->data));
    g_list_free1(list);

    list = list_next;
  }

  G_OBJECT_CLASS(ags_recycling_parent_class)->finalize(gobject);
}

void
ags_recycling_connect(AgsRecycling *recycling)
{
}

void
ags_recycling_add_audio_signal(AgsRecycling *recycling,
			       AgsAudioSignal *audio_signal)
{
  g_return_if_fail(AGS_IS_RECYCLING(recycling));
  g_return_if_fail(AGS_IS_AUDIO_SIGNAL(audio_signal));

  g_object_ref(G_OBJECT(recycling));
  g_signal_emit(G_OBJECT(recycling),
		recycling_signals[ADD_AUDIO_SIGNAL], 0,
		audio_signal);
  g_object_unref(G_OBJECT(recycling));
}

void
ags_recycling_real_add_audio_signal(AgsRecycling *recycling,
				    AgsAudioSignal *audio_signal)
{
  AgsAudioSignal *template;

  template = ags_audio_signal_get_template(recycling->audio_signal);

  audio_signal->devout = template->devout;

  audio_signal->recycling = (GObject *) recycling;

  audio_signal->length = template->length;
  audio_signal->last_frame = template->last_frame;

  ags_audio_signal_duplicate_stream(audio_signal, template);

  recycling->audio_signal = g_list_prepend(recycling->audio_signal, (gpointer) audio_signal);
}

void
ags_recycling_add_audio_signal_with_frame_count(AgsRecycling *recycling,
						AgsAudioSignal *audio_signal,
						guint frame_count)
{
  g_return_if_fail(AGS_IS_RECYCLING(recycling));

  g_object_ref((GObject *) recycling);
  g_signal_emit(G_OBJECT(recycling),
		recycling_signals[ADD_AUDIO_SIGNAL], 0,
		audio_signal, frame_count);
  g_object_unref((GObject *) recycling);
}

void
ags_recycling_real_add_audio_signal_with_frame_count(AgsRecycling *recycling,
						     AgsAudioSignal *audio_signal,
						     guint frame_count)
{
  AgsAudioSignal *template;
  GList *stream, *template_stream;
  guint i, template_i, j, k;

  template = ags_audio_signal_get_template(recycling->audio_signal);

  audio_signal->devout = template->devout;

  audio_signal->recycling = (GObject *) recycling;

  ags_audio_signal_stream_resize(audio_signal, (guint) ceil((double) frame_count / (double) AGS_DEVOUT(audio_signal->devout)->buffer_size));
  
  if(template->length == 0)
    return;

  audio_signal->last_frame = frame_count % AGS_DEVOUT(audio_signal->devout)->buffer_size;

  stream = audio_signal->stream_beginning;
  template_stream = template->stream_beginning;

  i = 0;
  template_i = 0;
  j = 0;
  k = 0;

  while(TRUE){
    /* check whether we are done */
    if(j == frame_count)
      goto ags_recycling_add_audio_signal_with_length0;
    
    /* check whether the end of stream's current buffer was reached */
    if(i == AGS_DEVOUT(audio_signal->devout)->buffer_size){
      stream = stream->next;
      i = 0;
    }
    
    /* check whether the end of template's current buffer was reached */
    if(template_i == AGS_DEVOUT(audio_signal->devout)->buffer_size){
      template_stream = template_stream->next;
      template_i = 0;
      k++;
    }
    
    /* check whether the end of the template was reached */
    if(k == template->length - 1 && template_i == template->last_frame){
      template_stream = template->stream_beginning;
      k = 0;
      template_i = 0;
    }
    
    /* copy audio data from template to new AgsAudioSignal */
    ((short*) stream->data)[i] = ((short*) template_stream->data)[template_i];
    
    /* increase indices */
    i++;
    template_i++;
    j++;
  }

 ags_recycling_add_audio_signal_with_length0:

  recycling->audio_signal = g_list_prepend(recycling->audio_signal, (gpointer) audio_signal);
}

void
ags_recycling_remove_audio_signal(AgsRecycling *recycling,
				  AgsAudioSignal *audio_signal)
{
  g_return_if_fail(AGS_IS_RECYCLING(recycling));

  g_object_ref((GObject *) recycling);
  g_signal_emit(G_OBJECT(recycling),
		recycling_signals[ADD_AUDIO_SIGNAL], 0,
		audio_signal);
  g_object_unref((GObject *) recycling);
}

void
ags_recycling_real_remove_audio_signal(AgsRecycling *recycling,
				       AgsAudioSignal *audio_signal)
{
  recycling->audio_signal = g_list_remove(recycling->audio_signal, (gpointer) audio_signal);
  g_object_unref(G_OBJECT(audio_signal));
}

AgsRecycling*
ags_recycling_new(GObject *devout)
{
  AgsRecycling *recycling;
  AgsAudioSignal *audio_signal;

  recycling = (AgsRecycling *) g_object_new(AGS_TYPE_RECYCLING, NULL);

  audio_signal = ags_audio_signal_new((GObject *) recycling, NULL);
  audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
  audio_signal->devout = devout;

  ags_audio_signal_connect(audio_signal);

  recycling->audio_signal = g_list_alloc();
  recycling->audio_signal->data = (gpointer) audio_signal;

  return(recycling);
}
