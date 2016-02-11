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

#include <ags/audio/task/ags_init_channel.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

void ags_init_channel_class_init(AgsInitChannelClass *init_channel);
void ags_init_channel_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_init_channel_init(AgsInitChannel *init_channel);
void ags_init_channel_connect(AgsConnectable *connectable);
void ags_init_channel_disconnect(AgsConnectable *connectable);
void ags_init_channel_finalize(GObject *gobject);

void ags_init_channel_launch(AgsTask *task);

/**
 * SECTION:ags_init_channel
 * @short_description: init channel object
 * @title: AgsInitChannel
 * @section_id:
 * @include: ags/audio/task/ags_init_channel.h
 *
 * The #AgsInitChannel task inits #AgsChannel.
 */

static gpointer ags_init_channel_parent_class = NULL;
static AgsConnectableInterface *ags_init_channel_parent_connectable_interface;

GType
ags_init_channel_get_type()
{
  static GType ags_type_init_channel = 0;

  if(!ags_type_init_channel){
    static const GTypeInfo ags_init_channel_info = {
      sizeof (AgsInitChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_init_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsInitChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_init_channel_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_init_channel_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_init_channel = g_type_register_static(AGS_TYPE_TASK,
						   "AgsInitChannel\0",
						   &ags_init_channel_info,
						   0);

    g_type_add_interface_static(ags_type_init_channel,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_init_channel);
}

void
ags_init_channel_class_init(AgsInitChannelClass *init_channel)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_init_channel_parent_class = g_type_class_peek_parent(init_channel);

  /* GObjectClass */
  gobject = (GObjectClass *) init_channel;

  gobject->finalize = ags_init_channel_finalize;

  /* AgsTaskClass */
  task = (AgsTaskClass *) init_channel;

  task->launch = ags_init_channel_launch;
}

void
ags_init_channel_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_init_channel_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_init_channel_connect;
  connectable->disconnect = ags_init_channel_disconnect;
}

void
ags_init_channel_init(AgsInitChannel *init_channel)
{
  init_channel->channel = NULL;
  init_channel->play_pad = FALSE;

  init_channel->do_playback = FALSE;
  init_channel->do_sequencer = FALSE;
  init_channel->do_notation = FALSE;
}

void
ags_init_channel_connect(AgsConnectable *connectable)
{
  ags_init_channel_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_init_channel_disconnect(AgsConnectable *connectable)
{
  ags_init_channel_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_init_channel_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_init_channel_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_init_channel_launch(AgsTask *task)
{
  AgsChannel *channel;
  AgsRecallID *recall_id;
  
  AgsInitChannel *init_channel;

  GList *list, *list_start;

  gint stage;
  gboolean arrange_recall_id, duplicate_templates, resolve_dependencies;

  init_channel = AGS_INIT_CHANNEL(task);

  list = NULL;
  list_start = NULL;

  /* init channel */
  if(init_channel->play_pad){
    AgsChannel *next_pad;

    next_pad = init_channel->channel->next_pad;

    for(stage = 0; stage < 3; stage++){

      channel = init_channel->channel;
      list = list_start;
      
      if(stage == 0){
	arrange_recall_id = TRUE;
	duplicate_templates = TRUE;
	resolve_dependencies = TRUE;
      }else{
	arrange_recall_id = FALSE;
	duplicate_templates = FALSE;
	resolve_dependencies = FALSE;
      }
      
      while(channel != next_pad){
	if(stage == 0){
	  if(init_channel->do_playback){
	    g_atomic_int_or(&(AGS_PLAYBACK(channel->playback)->flags),
			    AGS_PLAYBACK_PLAYBACK);

	    recall_id = ags_channel_recursive_play_init(channel, stage,
							arrange_recall_id, duplicate_templates,
							TRUE, FALSE, FALSE,
							resolve_dependencies,
							NULL);
	    AGS_PLAYBACK(channel->playback)->recall_id[0] = recall_id;
	  }
	  
	  if(init_channel->do_sequencer){
	    g_atomic_int_or(&(AGS_PLAYBACK(channel->playback)->flags),
			      AGS_PLAYBACK_SEQUENCER);

	    recall_id = ags_channel_recursive_play_init(channel, stage,
							arrange_recall_id, duplicate_templates,
							FALSE, TRUE, FALSE,
							resolve_dependencies,
							NULL);
	    AGS_PLAYBACK(channel->playback)->recall_id[1] = recall_id;
	  }
	  
	  if(init_channel->do_notation){
	    g_atomic_int_or(&(AGS_PLAYBACK(channel->playback)->flags),
			    AGS_PLAYBACK_NOTATION);

	    recall_id = ags_channel_recursive_play_init(channel, stage,
							arrange_recall_id, duplicate_templates,
							FALSE, FALSE, TRUE,
							resolve_dependencies,
							NULL);
	    AGS_PLAYBACK(channel->playback)->recall_id[2] = recall_id;
	  }

	  list_start = g_list_append(list_start,
				     recall_id);
	}else{
	  if(init_channel->do_playback){
	    ags_channel_recursive_play_init(channel, stage,
					    arrange_recall_id, duplicate_templates,
					    TRUE, FALSE, FALSE,
					    resolve_dependencies,
					    AGS_RECALL_ID(list->data));
	  }

	  if(init_channel->do_sequencer){
	    ags_channel_recursive_play_init(channel, stage,
					    arrange_recall_id, duplicate_templates,
					    FALSE, TRUE, FALSE,
					    resolve_dependencies,
					    AGS_RECALL_ID(list->data));
	  }

	  if(init_channel->do_notation){
	    ags_channel_recursive_play_init(channel, stage,
					    arrange_recall_id, duplicate_templates,
					    FALSE, FALSE, TRUE,
					    resolve_dependencies,
					    AGS_RECALL_ID(list->data));
	
	  }

	  list = list->next;
	}

	channel = channel->next;
      }
    }
  }else{
    AgsRecallID *recall_id;

    channel = init_channel->channel;

    for(stage = 0; stage < 3; stage++){

      channel = init_channel->channel;
      list = list_start;
      
      if(stage == 0){
	arrange_recall_id = TRUE;
	duplicate_templates = TRUE;
	resolve_dependencies = TRUE;
      }else{
	arrange_recall_id = FALSE;
	duplicate_templates = FALSE;
	resolve_dependencies = FALSE;
      }
      
      if(stage == 0){
	if(init_channel->do_playback){
	  g_atomic_int_or(&(AGS_PLAYBACK(channel->playback)->flags),
			  AGS_PLAYBACK_PLAYBACK);

	  recall_id = ags_channel_recursive_play_init(channel, stage,
						      arrange_recall_id, duplicate_templates,
						      TRUE, FALSE, FALSE,
						      resolve_dependencies,
						      NULL);
	  AGS_PLAYBACK(channel->playback)->recall_id[0] = recall_id;
	}
	  
	if(init_channel->do_sequencer){
	  g_atomic_int_or(&(AGS_PLAYBACK(channel->playback)->flags),
			  AGS_PLAYBACK_SEQUENCER);

	  recall_id = ags_channel_recursive_play_init(channel, stage,
						      arrange_recall_id, duplicate_templates,
						      FALSE, TRUE, FALSE,
						      resolve_dependencies,
						      NULL);
	  AGS_PLAYBACK(channel->playback)->recall_id[1] = recall_id;
	}
	  
	if(init_channel->do_notation){
	  g_atomic_int_or(&(AGS_PLAYBACK(channel->playback)->flags),
			  AGS_PLAYBACK_NOTATION);

	  recall_id = ags_channel_recursive_play_init(channel, stage,
						      arrange_recall_id, duplicate_templates,
						      FALSE, FALSE, TRUE,
						      resolve_dependencies,
						      NULL);
	  AGS_PLAYBACK(channel->playback)->recall_id[2] = recall_id;
	}

	list_start = g_list_append(list_start,
				   recall_id);
      }else{
	if(init_channel->do_playback){
	  ags_channel_recursive_play_init(channel, stage,
					  arrange_recall_id, duplicate_templates,
					  TRUE, FALSE, FALSE,
					  resolve_dependencies,
					  AGS_RECALL_ID(list->data));
	}

	if(init_channel->do_sequencer){
	  ags_channel_recursive_play_init(channel, stage,
					  arrange_recall_id, duplicate_templates,
					  FALSE, TRUE, FALSE,
					  resolve_dependencies,
					  AGS_RECALL_ID(list->data));
	}

	if(init_channel->do_notation){
	  ags_channel_recursive_play_init(channel, stage,
					  arrange_recall_id, duplicate_templates,
					  FALSE, FALSE, TRUE,
					  resolve_dependencies,
					  AGS_RECALL_ID(list->data));
	
	}
      }
    }
  }
}

/**
 * ags_init_channel_new:
 * @channel: the #AgsChannel
 * @play_pad: %TRUE all channels of accordig pad are initialized
 * @do_playback: init playback
 * @do_sequencer: init sequencer
 * @do_notation: init notation
 *
 * Creates an #AgsInitChannel.
 *
 * Returns: an new #AgsInitChannel.
 *
 * Since: 0.4
 */
AgsInitChannel*
ags_init_channel_new(AgsChannel *channel, gboolean play_pad,
		     gboolean do_playback, gboolean do_sequencer, gboolean do_notation)
{
  AgsInitChannel *init_channel;

  init_channel = (AgsInitChannel *) g_object_new(AGS_TYPE_INIT_CHANNEL,
						 NULL);

  init_channel->channel = channel;
  init_channel->play_pad = play_pad;

  init_channel->do_playback = do_playback;
  init_channel->do_sequencer = do_sequencer;
  init_channel->do_notation = do_notation;

  return(init_channel);
}