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

#include <ags/X/machine/ags_drum_output_line_callbacks.h>

#include <ags/X/machine/ags_drum.h>

#include <ags/object/ags_config.h>

#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recall_factory.h>

#include <ags/audio/recall/ags_delay_audio.h>

int
ags_drum_output_line_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, gpointer data)
{
  AgsDrumOutputLine *drum_output_line;

  if(old_parent != NULL)
    return(0);

  drum_output_line = AGS_DRUM_OUTPUT_LINE(widget);

  if(AGS_LINE(drum_output_line)->channel != NULL){
    /*
    AgsDrum *drum;
    AgsChannel *channel;
    AgsAudioSignal *audio_signal;
    AgsDelaySharedAudio *delay_shared_audio;
    GList *recall_shared;
    guint stop;

    drum = (AgsDrum *) gtk_widget_get_ancestor(widget, AGS_TYPE_DRUM);

    channel = AGS_LINE(drum_output_line)->channel;
    recall_shared = ags_recall_shared_find_type(AGS_AUDIO(channel->audio)->recall_shared,
						AGS_TYPE_DELAY_SHARED_AUDIO);
      
    if(recall_shared != NULL){
      delay_shared_audio = (AgsDelaySharedAudio *) recall_shared->data;
      stop = ((guint) drum->length_spin->adjustment->value) * (delay_shared_audio->delay + 1);
    }else{
      stop = 1;
    }
    
    audio_signal = ags_audio_signal_get_template(channel->first_recycling->audio_signal);
    ags_audio_signal_stream_resize(audio_signal, stop);
*/
  }

  return(0);
}

void
ags_drum_output_line_set_pads_callback(AgsAudio *audio, GType channel_type,
				       guint pads_new, guint pads_old,
				       AgsDrumOutputLine *output_line)
{
  AgsConfig *config;

  gchar *str;

  gboolean performance_mode;

  config = ags_config_get_instance();
  
  str = ags_config_get_value(config,
			     AGS_CONFIG_GENERIC,
			     "engine-mode\0");
  performance_mode = FALSE;

  if(channel_type == AGS_TYPE_INPUT){
    if(pads_new > pads_old){
      AgsChannel *input;
      
      input = ags_channel_pad_nth(audio->input,
				  pads_old);

      if(str != NULL &&
	 !g_ascii_strncasecmp(str,
			      "performance\0",
			      12)){
	while(input != NULL){
	  /* ags-copy */
	  ags_recall_factory_create(audio,
				    NULL, NULL,
				    "ags-copy\0",
				    0, audio->audio_channels, 
				    input->pad, input->pad + 1,
				    (AGS_RECALL_FACTORY_INPUT |
				     AGS_RECALL_FACTORY_RECALL |
				     AGS_RECALL_FACTORY_ADD),
				    0);

	  input = input->next_pad;
	}
	
	/* set performance mode */
	performance_mode = TRUE;
      }else{
	while(input != NULL){
	  /* ags-buffer */
	  ags_recall_factory_create(audio,
				    NULL, NULL,
				    "ags-buffer\0",
				    0, audio->audio_channels, 
				    input->pad, input->pad + 1,
				    (AGS_RECALL_FACTORY_INPUT |
				     AGS_RECALL_FACTORY_RECALL |
				     AGS_RECALL_FACTORY_ADD),
				    0);

	  input = input->next_pad;
	}
      }
    }
  }
}
