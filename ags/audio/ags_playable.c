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

#include <ags/audio/ags_playable.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_audio_buffer_util.h>

#include <math.h>

void ags_playable_base_init(AgsPlayableInterface *interface);

/**
 * SECTION:ags_playable
 * @short_description: read/write audio
 * @title: AgsPlayable
 * @section_id:
 * @include: ags/object/ags_playable.h
 *
 * The #AgsPlayable interface gives you a unique access to file related
 * IO operations.
 */

GType
ags_playable_get_type()
{
  static GType ags_type_playable = 0;

  if(!ags_type_playable){
    static const GTypeInfo ags_playable_info = {
      sizeof(AgsPlayableInterface),
      (GBaseInitFunc) ags_playable_base_init,
      NULL, /* base_finalize */
    };

    ags_type_playable = g_type_register_static(G_TYPE_INTERFACE,
					       "AgsPlayable\0", &ags_playable_info,
					       0);
  }

  return(ags_type_playable);
}


GQuark
ags_playable_error_quark()
{
  return(g_quark_from_static_string("ags-playable-error-quark\0"));
}

void
ags_playable_base_init(AgsPlayableInterface *interface)
{
  /* empty */
}

/**
 * ags_playable_open:
 * @playable: the #AgsPlayable
 * @name: the filename 
 * 
 * Opens a file in read-only mode.
 *
 * Returns: %TRUE on success
 *
 * Since: 0.4.2
 */
gboolean
ags_playable_open(AgsPlayable *playable, gchar *name)
{
  AgsPlayableInterface *playable_interface;
  gboolean ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), FALSE);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->open, FALSE);
  ret_val = playable_interface->open(playable, name);

  return(ret_val);
}

/**
 * ags_playable_rw_open:
 * @playable: the #AgsPlayable
 * @name: the filename 
 * @create: if %TRUE file is created
 * @samplerate: the samplerate of the file
 * @channels: the count of audio channels
 * @frames: the count of frames
 * @format: the audio file's format
 * 
 * Opens a file in read/write mode.
 *
 * Returns: %TRUE on success.
 *
 * Since: 0.4.2
 */
gboolean
ags_playable_rw_open(AgsPlayable *playable, gchar *name,
		     gboolean create,
		     guint samplerate, guint channels,
		     guint frames,
		     guint format)
{
  AgsPlayableInterface *playable_interface;
  gboolean ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), FALSE);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->open, FALSE);
  ret_val = playable_interface->rw_open(playable, name,
					create,
					samplerate, channels,
					frames,
					format);

  return(ret_val);
}

/**
 * ags_playable_level_count:
 * @playable: the #AgsPlayable
 * 
 * Retrieve the count of levels.
 *
 * Returns: level count
 *
 * Since: 0.4.2
 */
guint
ags_playable_level_count(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  guint ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), 0);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->level_count, 0);
  ret_val = playable_interface->level_count(playable);

  return(ret_val);
}

/**
 * ags_playable_nth_level:
 * @playable: the #AgsPlayable
 * 
 * Retrieve the selected level.
 *
 * Returns: nth level
 *
 * Since: 0.4.2
 */
guint
ags_playable_nth_level(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  guint ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), 0);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->nth_level, 0);
  ret_val = playable_interface->nth_level(playable);

  return(ret_val);
}

/**
 * ags_playable_selected_level:
 * @playable: the #AgsPlayable
 * 
 * Retrieve the selected level's name.
 *
 * Returns: nth level name
 *
 * Since: 0.4.2
 */
gchar*
ags_playable_selected_level(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  gchar *ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), 0);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->selected_level, 0);
  ret_val = playable_interface->selected_level(playable);

  return(ret_val);

}

/**
 * ags_playable_sublevel_names:
 * @playable: the #AgsPlayable
 * 
 * Retrieve the all sub-level's name.
 *
 * Returns: sub-level names
 *
 * Since: 0.4.2
 */
gchar**
ags_playable_sublevel_names(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  gchar **ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), NULL);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->sublevel_names, NULL);
  ret_val = playable_interface->sublevel_names(playable);

  return(ret_val);
}

/**
 * ags_playable_level_select:
 * @playable: an #AgsPlayable
 * @nth_level: of type guint
 * @sublevel_name: a gchar pointer
 * @error: an error that may occure
 *
 * Select a level in an monolythic file where @nth_level and @sublevel_name are equivalent.
 * If @sublevel_name is NULL @nth_level will be chosen.
 *
 * Since: 0.4.2
 */
void
ags_playable_level_select(AgsPlayable *playable,
			  guint nth_level, gchar *sublevel_name,
			  GError **error)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->level_select);
  playable_interface->level_select(playable, nth_level, sublevel_name, error);
}

/**
 * ags_playable_level_up:
 * @playable: an #AgsPlayable
 * @levels: n-levels up
 * @error: returned error
 *
 * Move up in hierarchy.
 *
 * Since: 0.4.2
 */
void
ags_playable_level_up(AgsPlayable *playable,
		      guint levels,
		      GError **error)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->level_up);
  playable_interface->level_up(playable, levels, error);
}

/**
 * ags_playable_iter_start:
 * @playable: an #AgsPlayable
 *
 * Start iterating current level.
 *
 * Since: 0.4.2
 */
void
ags_playable_iter_start(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->iter_start);
  playable_interface->iter_start(playable);
}

/**
 * ags_playable_iter_next:
 * @playable: an #AgsPlayable
 *
 * Iterating next on current level.
 *
 * Returns: %TRUE if has more, otherwise %FALSE
 *
 * Since: 0.4.2
 */
gboolean
ags_playable_iter_next(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;
  gboolean ret_val;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable), FALSE);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->iter_next, FALSE);
  ret_val = playable_interface->iter_next(playable);

  return(ret_val);
}

/**
 * ags_playable_info:
 * @playable: an #AgsPlayable
 * @channels: channels
 * @frames: frames
 * @loop_start: loop start
 * @loop_end: loop end
 * @error: returned error
 *
 * Retrieve information about selected audio data.
 *
 * Since: 0.4.2
 */
void
ags_playable_info(AgsPlayable *playable,
		  guint *channels, guint *frames,
		  guint *loop_start, guint *loop_end,
		  GError **error)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->info);
  playable_interface->info(playable, channels, frames, loop_start, loop_end, error);
}

/**
 * ags_playable_get_samplerate:
 * @playable: the #AgsPlayable
 *
 * Get samplerate.
 *
 * Returns: the samplerate
 * 
 * Since: 0.7.65
 */
guint
ags_playable_get_samplerate(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable),
		       0);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->get_samplerate,
		       0);
  playable_interface->get_samplerate(playable);
}

/**
 * ags_playable_get_format:
 * @playable: the #AgsPlayable
 *
 * Get format.
 *
 * Returns: the format
 * 
 * Since: 0.7.65
 */
guint
ags_playable_get_format(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable),
		       0);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->get_format,
		       0);
  playable_interface->get_format(playable);
}

/**
 * ags_playable_read:
 * @playable: an #AgsPlayable
 * @channel: nth channel
 * @error: returned error
 *
 * Read audio buffer of playable audio data.
 * 
 * Returns: audio buffer
 *
 * Since: 0.4.2
 */
double*
ags_playable_read(AgsPlayable *playable,
		  guint channel,
		  GError **error)
{
  AgsPlayableInterface *playable_interface;

  g_return_val_if_fail(AGS_IS_PLAYABLE(playable),
		       NULL);
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_val_if_fail(playable_interface->read,
		       NULL);
  playable_interface->read(playable, channel, error);
}

/**
 * ags_playable_write:
 * @playable: an #AgsPlayable
 * @buffer: audio data
 * @buffer_length: frame count
 *
 * Write @buffer_length of @buffer audio data.
 *
 * Since: 0.4.2
 */
void
ags_playable_write(AgsPlayable *playable,
		   double *buffer, guint buffer_length)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->write);
  playable_interface->write(playable, buffer, buffer_length);
}

/**
 * ags_playable_flush:
 * @playable: an #AgsPlayable
 *
 * Flush internal audio buffer.
 *
 * Since: 0.4.2
 */
void
ags_playable_flush(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->flush);
  playable_interface->flush(playable);
}

/**
 * ags_playable_seek:
 * @playable: an #AgsPlayable
 * @frames: n-frames to seek
 * @whence: SEEK_SET, SEEK_CUR, or SEEK_END
 *
 * Seek @playable to address.
 *
 * Since: 0.4.2
 */
void
ags_playable_seek(AgsPlayable *playable,
		  guint frames, gint whence)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->seek);
  playable_interface->seek(playable, frames, whence);
}

/**
 * ags_playable_close:
 * @playable: an #AgsPlayable
 *
 * Close audio file.
 *
 * Since: 0.4.2
 */
void
ags_playable_close(AgsPlayable *playable)
{
  AgsPlayableInterface *playable_interface;

  g_return_if_fail(AGS_IS_PLAYABLE(playable));
  playable_interface = AGS_PLAYABLE_GET_INTERFACE(playable);
  g_return_if_fail(playable_interface->close);
  playable_interface->close(playable);
}

/**
 * ags_playable_read_audio_signal:
 * @playable: an #AgsPlayable
 * @soundcard: the #AgsSoundcar defaulting to
 * @start_channel: read from channel
 * @channels_to_read: n-times
 *
 * Read the audio signal of @AgsPlayable.
 *
 * Returns: a #GList of #AgsAudioSignal
 *
 * Since: 0.4.2
 */
GList*
ags_playable_read_audio_signal(AgsPlayable *playable,
			       GObject *soundcard,
			       guint start_channel, guint channels_to_read)
{
  AgsAudioSignal *audio_signal;

  AgsMutexManager *mutex_manager;
  
  GList *stream, *list, *list_beginning;
  
  gchar *str;
  
  double *buffer;

  guint copy_mode;
  guint channels;
  guint frames;
  guint resampled_frames;
  guint loop_start;
  guint loop_end;
  guint length;
  guint samplerate;
  guint buffer_size;
  guint format;
  guint target_samplerate;
  guint i, j, k, i_stop, j_stop;
  gboolean resample;

  GError *error;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;
  
  g_return_val_if_fail(AGS_IS_PLAYABLE(playable),
		       NULL);

  error = NULL;
  ags_playable_info(playable,
		    &channels, &frames,
		    &loop_start, &loop_end,
		    &error);

  if(soundcard == NULL){
    AgsConfig *config;

    gchar *str;
    
    config = ags_config_get_instance();

    /* samplerate */
    str = ags_config_get_value(config,
			       AGS_CONFIG_SOUNDCARD,
			       "samplerate\0");

    if(str == NULL){
      str = ags_config_get_value(config,
				 AGS_CONFIG_SOUNDCARD_0,
				 "samplerate\0");
    }
    
    if(str != NULL){
      target_samplerate = g_ascii_strtoull(str,
				    NULL,
				    10);

      g_free(str);
    }else{
      target_samplerate = AGS_SOUNDCARD_DEFAULT_SAMPLERATE;
    }

    /* buffer-size */
    str = ags_config_get_value(config,
			       AGS_CONFIG_SOUNDCARD,
			       "buffer-size\0");

    if(str == NULL){
      str = ags_config_get_value(config,
				 AGS_CONFIG_SOUNDCARD_0,
				 "buffer-size\0");
    }
    
    if(str != NULL){
      buffer_size = g_ascii_strtoull(str,
				     NULL,
				     10);
      
      g_free(str);
    }else{
      buffer_size = AGS_SOUNDCARD_DEFAULT_BUFFER_SIZE;
    }

    /* format */
    str = ags_config_get_value(config,
			       AGS_CONFIG_SOUNDCARD,
			       "format\0");

    if(str == NULL){
      str = ags_config_get_value(config,
				 AGS_CONFIG_SOUNDCARD_0,
				 "format\0");
    }
    
    if(str != NULL){
      format = g_ascii_strtoull(str,
				     NULL,
				     10);
      
      g_free(str);
    }else{
      format = AGS_SOUNDCARD_DEFAULT_FORMAT;
    }
  }else{
    mutex_manager = ags_mutex_manager_get_instance();
    application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

    /* lookup soundcard mutex */
    pthread_mutex_lock(application_mutex);
    
    soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					       soundcard);
    
    pthread_mutex_unlock(application_mutex);

    /* get presets */
    pthread_mutex_lock(soundcard_mutex);
    
    ags_soundcard_get_presets(AGS_SOUNDCARD(soundcard),
			      NULL,
			      &target_samplerate,
			      &buffer_size,
			      &format);

    pthread_mutex_unlock(soundcard_mutex);
  }

  copy_mode = ags_audio_buffer_util_get_copy_mode(ags_audio_buffer_util_format_from_soundcard(format),
						  AGS_AUDIO_BUFFER_UTIL_DOUBLE);
  samplerate = ags_playable_get_samplerate(playable);
  
  if(target_samplerate != samplerate){
    resampled_frames = (target_samplerate / samplerate) * frames;
    resample = TRUE;
  }else{
    resampled_frames = frames;
  }
  
  length = (guint) ceil((double)(resampled_frames) / (double)(buffer_size));

#ifdef AGS_DEBUG
  g_message("%d-%d %d %d\0", samplerate, target_samplerate, buffer_size, format);
  g_message("ags_playable_read_audio_signal:\n  frames = %u\n  buffer_size = %u\n  length = %u\n\0", frames, buffer_size, length);
#endif
  
  list = NULL;
  i = start_channel;
  i_stop = start_channel + channels_to_read;

  for(; i < i_stop; i++){
    audio_signal = ags_audio_signal_new(soundcard,
					NULL,
					NULL);
    audio_signal->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
    g_object_set(audio_signal,
		 "samplerate\0", target_samplerate,
		 "buffer-size\0", buffer_size,
		 "format\0", format,
		 NULL);
    
    list = g_list_prepend(list, audio_signal);

    ags_connectable_connect(AGS_CONNECTABLE(audio_signal));
  }
  
  list_beginning = list;

  j_stop = (guint) floor((double)(resampled_frames) / (double)(buffer_size));
  
  for(i = start_channel; list != NULL; i++){
    audio_signal = AGS_AUDIO_SIGNAL(list->data);
    ags_audio_signal_stream_resize(audio_signal, length);
    audio_signal->loop_start = loop_start;
    audio_signal->loop_end = loop_end;
    //TODO:JK: read resolution of file
    //    audio_signal->format = AGS_SOUNDCARD_SIGNED_16_BIT;

    error = NULL;
    buffer = ags_playable_read(playable,
			       i,
			       &error);

    if(error != NULL){
      g_error("%s\0", error->message);
    }

    if(buffer != NULL){
      if(resample){
	double *tmp;

	tmp = buffer;
	buffer = ags_audio_buffer_util_resample(buffer, 1,
						AGS_AUDIO_BUFFER_UTIL_DOUBLE, samplerate,
						frames,
						target_samplerate);
	free(tmp);
      }
      
      stream = audio_signal->stream_beginning;

      for(j = 0; j < j_stop && stream != NULL; j++){
	ags_audio_buffer_util_copy_buffer_to_buffer(stream->data, 1, 0,
						    buffer, 1, j * buffer_size,
						    buffer_size, copy_mode);
	
	stream = stream->next;
      }
    
      if(resampled_frames % buffer_size != 0){
	ags_audio_buffer_util_copy_buffer_to_buffer(stream->data, 1, 0,
						    buffer, 1, j * buffer_size,
						    resampled_frames % buffer_size, copy_mode);
      }

      free(buffer);
    }
    
    list = list->next;
  }

  return(list_beginning);
}
