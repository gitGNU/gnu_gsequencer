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

#include <ags/audio/task/ags_cancel_channel.h>

#include <ags/object/ags_connectable.h>

#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

void ags_cancel_channel_class_init(AgsCancelChannelClass *cancel_channel);
void ags_cancel_channel_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_cancel_channel_init(AgsCancelChannel *cancel_channel);
void ags_cancel_channel_connect(AgsConnectable *connectable);
void ags_cancel_channel_disconnect(AgsConnectable *connectable);
void ags_cancel_channel_finalize(GObject *gobject);

void ags_cancel_channel_launch(AgsTask *task);

/**
 * SECTION:ags_cancel_channel
 * @short_description: cancel channel object
 * @title: AgsCancelChannel
 * @section_id:
 * @include: ags/audio/task/ags_cancel_channel.h
 *
 * The #AgsCancelChannel task cancels #AgsChannel playback.
 */

static gpointer ags_cancel_channel_parent_class = NULL;
static AgsConnectableInterface *ags_cancel_channel_parent_connectable_interface;

GType
ags_cancel_channel_get_type()
{
  static GType ags_type_cancel_channel = 0;

  if(!ags_type_cancel_channel){
    static const GTypeInfo ags_cancel_channel_info = {
      sizeof (AgsCancelChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_cancel_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCancelChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_cancel_channel_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_cancel_channel_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_cancel_channel = g_type_register_static(AGS_TYPE_TASK,
						     "AgsCancelChannel\0",
						     &ags_cancel_channel_info,
						     0);
    
    g_type_add_interface_static(ags_type_cancel_channel,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_cancel_channel);
}

void
ags_cancel_channel_class_init(AgsCancelChannelClass *cancel_channel)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_cancel_channel_parent_class = g_type_class_peek_parent(cancel_channel);

  /* gobject */
  gobject = (GObjectClass *) cancel_channel;

  gobject->finalize = ags_cancel_channel_finalize;

  /* task */
  task = (AgsTaskClass *) cancel_channel;

  task->launch = ags_cancel_channel_launch;
}

void
ags_cancel_channel_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_cancel_channel_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_cancel_channel_connect;
  connectable->disconnect = ags_cancel_channel_disconnect;
}

void
ags_cancel_channel_init(AgsCancelChannel *cancel_channel)
{
  cancel_channel->channel = NULL;
  cancel_channel->recall_id = NULL;

  cancel_channel->playback = NULL;
}

void
ags_cancel_channel_connect(AgsConnectable *connectable)
{
  ags_cancel_channel_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_cancel_channel_disconnect(AgsConnectable *connectable)
{
  ags_cancel_channel_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_cancel_channel_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_cancel_channel_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_cancel_channel_launch(AgsTask *task)
{
  AgsPlayback *playback;
  AgsChannel *channel;

  AgsCancelChannel *cancel_channel;

  cancel_channel = AGS_CANCEL_CHANNEL(task);

  channel = cancel_channel->channel;
  playback = AGS_PLAYBACK(channel->playback);
  
  /* cancel playback */
  if(cancel_channel->recall_id == AGS_PLAYBACK(channel->playback)->recall_id[0]){
    g_atomic_int_and(&(playback->flags),
		     (~AGS_PLAYBACK_PLAYBACK));

    g_object_ref(AGS_PLAYBACK(channel->playback)->recall_id[0]);
    ags_channel_tillrecycling_cancel(channel,
				     AGS_PLAYBACK(channel->playback)->recall_id[0]);
    AGS_PLAYBACK(channel->playback)->recall_id[0] = NULL;

    if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
      //      ags_thread_stop(playback->channel_thread[0]);
    }
  }

  /* cancel sequencer */
  if(cancel_channel->recall_id == AGS_PLAYBACK(channel->playback)->recall_id[1]){
    g_atomic_int_and(&(playback->flags),
		     (~AGS_PLAYBACK_SEQUENCER));

    g_object_ref(AGS_PLAYBACK(channel->playback)->recall_id[1]);
    ags_channel_tillrecycling_cancel(channel,
				     AGS_PLAYBACK(channel->playback)->recall_id[1]);
    AGS_PLAYBACK(channel->playback)->recall_id[1] = NULL;

    if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
      //      ags_thread_stop(playback->channel_thread[1]);
    }
  }

  /* cancel notation */
  if(cancel_channel->recall_id == AGS_PLAYBACK(channel->playback)->recall_id[2]){
    g_atomic_int_and(&(playback->flags),
		     (~AGS_PLAYBACK_NOTATION));

    g_object_ref(AGS_PLAYBACK(channel->playback)->recall_id[2]);
    ags_channel_tillrecycling_cancel(channel,
				     AGS_PLAYBACK(channel->playback)->recall_id[2]);
    AGS_PLAYBACK(channel->playback)->recall_id[2] = NULL;

    if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
      //      ags_thread_stop(playback->channel_thread[2]);
    }
  }

  ags_channel_done(channel,
		   cancel_channel->recall_id);		   
}

/**
 * ags_cancel_channel_new:
 * @channel: the #AgsChannel to cancel
 * @recall_id: the #AgsRecallID to cancel
 * @play: the #AgsDevoutPlay-struct
 *
 * Creates an #AgsCancelChannel.
 *
 * Returns: an new #AgsCancelChannel.
 *
 * Since: 0.4
 */
AgsCancelChannel*
ags_cancel_channel_new(AgsChannel *channel, AgsRecallID *recall_id,
		       GObject *playback)
{
  AgsCancelChannel *cancel_channel;

  cancel_channel = (AgsCancelChannel *) g_object_new(AGS_TYPE_CANCEL_CHANNEL,
						     NULL);

  cancel_channel->channel = channel;
  cancel_channel->recall_id = recall_id;

  cancel_channel->playback = playback;

  return(cancel_channel);
}