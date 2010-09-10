#include "ags_drum_input_line.h"
#include "ags_drum_input_line_callbacks.h"

#include "../ags_window.h"

#include "ags_drum.h"

#include "../../audio/recall/ags_delay.h"
#include "../../audio/recall/ags_play_volume.h"
#include "../../audio/recall/ags_play_channel.h"
#include "../../audio/recall/ags_recall_volume.h"
#include "../../audio/recall/ags_copy_pattern.h"

GType ags_drum_input_line_get_type();
void ags_drum_input_line_class_init(AgsDrumInputLineClass *drum_input_line);
void ags_drum_input_line_init(AgsDrumInputLine *drum_input_line);
void ags_drum_input_line_set_property(GObject *gobject,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *param_spec);
void ags_drum_input_line_get_property(GObject *gobject,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *param_spec);
void ags_drum_input_line_destroy(GtkObject *object);
void ags_drum_input_line_connect(AgsDrumInputLine *drum_input_line);

static gpointer ags_drum_input_line_parent_class = NULL;

GType
ags_drum_input_line_get_type()
{
  static GType ags_type_drum_input_line = 0;

  if(!ags_type_drum_input_line){
    static const GTypeInfo ags_drum_input_line_info = {
      sizeof(AgsDrumInputLineClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_drum_input_line_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsDrumInputLine),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_drum_input_line_init,
    };

    ags_type_drum_input_line = g_type_register_static(AGS_TYPE_LINE,
						      "AgsDrumInputLine\0", &ags_drum_input_line_info,
						      0);
  }

  return(ags_type_drum_input_line);
}

void
ags_drum_input_line_class_init(AgsDrumInputLineClass *drum_input_line)
{
  GObjectClass *gobject;
  AgsLineClass *line;
  GParamSpec *param_spec;

  ags_drum_input_line_parent_class = g_type_class_peek_parent(drum_input_line);

  gobject = G_OBJECT_CLASS(line);

  gobject->set_property = ags_drum_input_line_set_property;
  gobject->get_property = ags_drum_input_line_get_property;
}

void
ags_drum_input_line_init(AgsDrumInputLine *drum_input_line)
{
  GtkVScale *scale;

  scale = (GtkVScale *) gtk_vscale_new_with_range(0.0, 1.25, 0.025);
  gtk_range_set_value((GtkRange *) scale, 0.8);
  gtk_range_set_inverted((GtkRange *) scale, TRUE);
  gtk_scale_set_digits((GtkScale *) scale, 3);
  gtk_widget_set_size_request((GtkWidget *) scale, -1, 100);
  gtk_table_attach(AGS_LINE(drum_input_line)->table,
		   (GtkWidget *) scale,
		   0, 1,
		   1, 2,
		   GTK_EXPAND, GTK_EXPAND,
		   0, 0);
}

void
ags_drum_input_line_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec)
{
  G_OBJECT_CLASS(ags_drum_input_line_parent_class)->set_property(gobject,
								 prop_id,
								 value,
								 param_spec);

  fprintf(stdout, "ags_drum_input_line_set_property\n\0");
}

void
ags_drum_input_line_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec)
{
  G_OBJECT_CLASS(ags_drum_input_line_parent_class)->get_property(gobject,
								 prop_id,
								 value,
								 param_spec);

  fprintf(stdout, "ags_drum_input_line_get_property\n\0");
}

void
ags_drum_input_line_destroy(GtkObject *object)
{
}

void
ags_drum_input_line_connect(AgsDrumInputLine *drum_input_line)
{
}

void
ags_drum_input_line_map_recall(AgsDrumInputLine *drum_input_line,
			       guint output_pad_start)
{
  AgsDrum *drum;
  AgsAudio *audio;
  AgsChannel *source, *destination;
  AgsPlayVolume *play_volume;
  AgsRecallVolume *recall_volume;
  AgsPlayChannel *play_channel;
  AgsCopyPattern *copy_pattern;
  AgsCopyPatternSharedAudio *copy_pattern_shared_audio;
  AgsCopyPatternSharedAudioRun *copy_pattern_shared_audio_run;
  AgsCopyPatternSharedChannel *copy_pattern_shared_channel;
  guint i;

  drum = AGS_DRUM(gtk_widget_get_ancestor((GtkWidget *) drum_input_line, AGS_TYPE_DRUM));
  audio = drum->machine.audio;

  if((AGS_DRUM_INPUT_LINE_MAPPED_RECALL & (drum_input_line->flags)) == 0){
    /* AgsPlayChannel */
    play_channel = ags_play_channel_new(source,
					AGS_DEVOUT(audio->devout));
    
    play_channel->recall.flags |= AGS_RECALL_TEMPLATE;
    
    ags_play_channel_connect(play_channel);
    
    g_signal_connect((GObject *) play_channel, "done\0",
		     G_CALLBACK(ags_drum_input_line_play_channel_done), drum_input_line);
    
    g_signal_connect((GObject *) play_channel, "cancel\0",
		     G_CALLBACK(ags_drum_input_line_play_channel_cancel), drum_input_line);
    
    source->play = g_list_append(source->play, (gpointer) play_channel);
  }

  /* create recalls which depend on output */
  destination = ags_channel_nth(audio->output, audio->audio_channels * output_pad_start);

  while(destination != NULL){
    /* AgsCopyPattern */
    copy_pattern_shared_channel = ags_copy_pattern_shared_channel_new(destination,
								      source, (AgsPattern *) source->pattern->data);
    
    copy_pattern = ags_copy_pattern_new(drum->copy_pattern_shared_audio,
					drum->copy_pattern_shared_audio_run,
					copy_pattern_shared_channel);
    
    copy_pattern->recall.flags |= AGS_RECALL_TEMPLATE;
    
    ags_connectable_connect(AGS_CONNECTABLE(copy_pattern));
    
    g_signal_connect((GObject *) copy_pattern, "done\0",
		     G_CALLBACK(ags_drum_input_line_copy_pattern_done), drum_input_line);
    
    g_signal_connect((GObject *) copy_pattern, "cancel\0",
		     G_CALLBACK(ags_drum_input_line_copy_pattern_cancel), drum_input_line);
    
    g_signal_connect((GObject *) copy_pattern, "loop\0",
		     G_CALLBACK(ags_drum_input_line_copy_pattern_loop), drum_input_line);
    
    source->recall = g_list_append(source->recall, (gpointer) copy_pattern);

    
    destination = destination->next_pad;
  }
}

AgsDrumInputLine*
ags_drum_input_line_new(AgsChannel *channel)
{
  AgsDrumInputLine *drum_input_line;

  drum_input_line = (AgsDrumInputLine *) g_object_new(AGS_TYPE_DRUM_INPUT_LINE, NULL);

  drum_input_line->line.channel = channel;

  if(channel != NULL){
    channel->pattern = g_list_alloc();
    channel->pattern->data = (gpointer) ags_pattern_new();
    ags_pattern_set_dim((AgsPattern *) channel->pattern->data, 4, 12, 64);
  }

  return(drum_input_line);
}
