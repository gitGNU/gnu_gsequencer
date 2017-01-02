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

#include <ags/audio/recall/ags_feed_channel_run.h>
#include <ags/audio/recall/ags_feed_recycling.h>

#include <ags/lib/ags_parameter.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_plugin.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_recall_id.h>

#include <ags/audio/task/ags_cancel_recall.h>

void ags_feed_channel_run_class_init(AgsFeedChannelRunClass *feed_channel_run);
void ags_feed_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_feed_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_feed_channel_run_init(AgsFeedChannelRun *feed_channel_run);
void ags_feed_channel_run_connect(AgsConnectable *connectable);
void ags_feed_channel_run_disconnect(AgsConnectable *connectable);
void ags_feed_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_feed_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_feed_channel_run_finalize(GObject *gobject);

AgsRecall* ags_feed_channel_run_duplicate(AgsRecall *recall,
					  AgsRecallID *recall_id,
					  guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_feed_channel_run
 * @short_description: feeds channel
 * @title: AgsFeedChannelRun
 * @section_id:
 * @include: ags/audio/recall/ags_feed_channel_run.h
 *
 * The #AgsFeedChannelRun class feeds the channel.
 */

static gpointer ags_feed_channel_run_parent_class = NULL;
static AgsConnectableInterface *ags_feed_channel_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_feed_channel_run_parent_dynamic_connectable_interface;

GType
ags_feed_channel_run_get_type()
{
  static GType ags_type_feed_channel_run = 0;

  if(!ags_type_feed_channel_run){
    static const GTypeInfo ags_feed_channel_run_info = {
      sizeof (AgsFeedChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_feed_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsFeedChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_feed_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_feed_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_feed_channel_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_feed_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
						       "AgsFeedChannelRun\0",
						       &ags_feed_channel_run_info,
						       0);

    g_type_add_interface_static(ags_type_feed_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_feed_channel_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return (ags_type_feed_channel_run);
}

void
ags_feed_channel_run_class_init(AgsFeedChannelRunClass *feed_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_feed_channel_run_parent_class = g_type_class_peek_parent(feed_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) feed_channel_run;

  gobject->finalize = ags_feed_channel_run_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) feed_channel_run;

  recall->duplicate = ags_feed_channel_run_duplicate;
}

void
ags_feed_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_feed_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_feed_channel_run_connect;
  connectable->disconnect = ags_feed_channel_run_disconnect;
}

void
ags_feed_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_feed_channel_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_feed_channel_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_feed_channel_run_disconnect_dynamic;
}

void
ags_feed_channel_run_init(AgsFeedChannelRun *feed_channel_run)
{
  AGS_RECALL(feed_channel_run)->name = "ags-feed\0";
  AGS_RECALL(feed_channel_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(feed_channel_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(feed_channel_run)->xml_type = "ags-feed-channel-run\0";
  AGS_RECALL(feed_channel_run)->port = NULL;

  AGS_RECALL(feed_channel_run)->flags |= AGS_RECALL_INPUT_ORIENTATED;
  AGS_RECALL(feed_channel_run)->child_type = AGS_TYPE_FEED_RECYCLING;
}

void
ags_feed_channel_run_finalize(GObject *gobject)
{
  /* empty */
  
  /* call parent */
  G_OBJECT_CLASS(ags_feed_channel_run_parent_class)->finalize(gobject);
}

void
ags_feed_channel_run_connect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) != 0){
    return;
  }
  
  /* call parent */
  ags_feed_channel_run_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_feed_channel_run_disconnect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) == 0){
    return;
  }

  /* empty */
  
  /* call parent */
  ags_feed_channel_run_parent_connectable_interface->disconnect(connectable);
}

void
ags_feed_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_feed_channel_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_feed_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) == 0){
    return;
  }

  /* empty */
  
  /* call parent */
  ags_feed_channel_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);
}

AgsRecall*
ags_feed_channel_run_duplicate(AgsRecall *recall,
			       AgsRecallID *recall_id,
			       guint *n_params, GParameter *parameter)
{
  AgsFeedChannelRun *copy;
  
  copy = (AgsFeedChannelRun *) AGS_RECALL_CLASS(ags_feed_channel_run_parent_class)->duplicate(recall,
											      recall_id,
											      n_params, parameter);
  
  return((AgsRecall *) copy);
}

/**
 * ags_feed_channel_run_new:
 * @channel: the assigned #AgsChannel
 *
 * Creates an #AgsFeedChannelRun
 *
 * Returns: a new #AgsFeedChannelRun
 *
 * Since: 0.7.111
 */
AgsFeedChannelRun*
ags_feed_channel_run_new(AgsChannel *channel)
{
  AgsFeedChannelRun *feed_channel_run;

  feed_channel_run = (AgsFeedChannelRun *) g_object_new(AGS_TYPE_FEED_CHANNEL_RUN,
							"source\0", channel,
							NULL);

  return(feed_channel_run);
}