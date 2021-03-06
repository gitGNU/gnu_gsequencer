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

#include <ags/audio/recall/ags_copy_recycling.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>

#include <ags/audio/recall/ags_copy_audio_signal.h>

#include <stdlib.h>
#include <stdio.h>

void ags_copy_recycling_class_init(AgsCopyRecyclingClass *copy_recycling);
void ags_copy_recycling_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_copy_recycling_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_copy_recycling_init(AgsCopyRecycling *copy_recycling);
void ags_copy_recycling_connect(AgsConnectable *connectable);
void ags_copy_recycling_disconnect(AgsConnectable *connectable);
void ags_copy_recycling_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_copy_recycling_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_copy_recycling_finalize(GObject *gobject);

void ags_copy_recycling_done(AgsRecall *recall);
void ags_copy_recycling_cancel(AgsRecall *recall);
void ags_copy_recycling_remove(AgsRecall *recall);
AgsRecall* ags_copy_recycling_duplicate(AgsRecall *recall,
					AgsRecallID *recall_id,
					guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_copy_recycling
 * @short_description: copys recycling
 * @title: AgsCopyRecycling
 * @section_id:
 * @include: ags/audio/recall/ags_copy_recycling.h
 *
 * The #AgsCopyRecycling class copys the recycling.
 */

static gpointer ags_copy_recycling_parent_class = NULL;
static AgsConnectableInterface *ags_copy_recycling_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_copy_recycling_parent_dynamic_connectable_interface;

GType
ags_copy_recycling_get_type()
{
  static GType ags_type_copy_recycling = 0;

  if(!ags_type_copy_recycling){
    static const GTypeInfo ags_copy_recycling_info = {
      sizeof (AgsCopyRecyclingClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_copy_recycling_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCopyRecycling),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_copy_recycling_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_recycling_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_recycling_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_copy_recycling = g_type_register_static(AGS_TYPE_RECALL_RECYCLING,
						     "AgsCopyRecycling\0",
						     &ags_copy_recycling_info,
						     0);

    g_type_add_interface_static(ags_type_copy_recycling,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_copy_recycling,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return(ags_type_copy_recycling);
}

void
ags_copy_recycling_class_init(AgsCopyRecyclingClass *copy_recycling)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_copy_recycling_parent_class = g_type_class_peek_parent(copy_recycling);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_recycling;

  gobject->finalize = ags_copy_recycling_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) copy_recycling;

  recall->done = ags_copy_recycling_done;
  recall->cancel = ags_copy_recycling_cancel;
  recall->remove = ags_copy_recycling_remove;

  recall->duplicate = ags_copy_recycling_duplicate;
}

void
ags_copy_recycling_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_copy_recycling_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_copy_recycling_connect;
  connectable->disconnect = ags_copy_recycling_disconnect;
}

void
ags_copy_recycling_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_copy_recycling_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_copy_recycling_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_copy_recycling_disconnect_dynamic;
}

void
ags_copy_recycling_init(AgsCopyRecycling *copy_recycling)
{
  AGS_RECALL(copy_recycling)->name = "ags-copy\0";
  AGS_RECALL(copy_recycling)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(copy_recycling)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(copy_recycling)->xml_type = "ags-copy-recycling\0";
  AGS_RECALL(copy_recycling)->port = NULL;

  AGS_RECALL(copy_recycling)->child_type = AGS_TYPE_COPY_AUDIO_SIGNAL;
  AGS_RECALL_RECYCLING(copy_recycling)->flags |= (AGS_RECALL_RECYCLING_MAP_CHILD_DESTINATION);
}

void
ags_copy_recycling_finalize(GObject *gobject)
{
  /* empty */

  /* call parent */
  G_OBJECT_CLASS(ags_copy_recycling_parent_class)->finalize(gobject);
}

void
ags_copy_recycling_connect(AgsConnectable *connectable)
{
  ags_copy_recycling_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_copy_recycling_disconnect(AgsConnectable *connectable)
{
  ags_copy_recycling_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_copy_recycling_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  ags_copy_recycling_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_copy_recycling_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  ags_copy_recycling_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void 
ags_copy_recycling_done(AgsRecall *recall)
{
  AGS_RECALL_CLASS(ags_copy_recycling_parent_class)->done(recall);

  /* empty */
}

void
ags_copy_recycling_cancel(AgsRecall *recall)
{
  AGS_RECALL_CLASS(ags_copy_recycling_parent_class)->cancel(recall);

  /* empty */
}

void 
ags_copy_recycling_remove(AgsRecall *recall)
{
  AGS_RECALL_CLASS(ags_copy_recycling_parent_class)->remove(recall);

  /* empty */
}

AgsRecall*
ags_copy_recycling_duplicate(AgsRecall *recall,
			     AgsRecallID *recall_id,
			     guint *n_params, GParameter *parameter)
{
  AgsCopyRecycling *copy;

  copy = (AgsCopyRecycling *) AGS_RECALL_CLASS(ags_copy_recycling_parent_class)->duplicate(recall,
											   recall_id,
											   n_params, parameter);

  return((AgsRecall *) copy);
}

/**
 * ags_copy_recycling_new:
 * @destination: the destination #AgsRecycling
 * @source: the source #AgsRecycling
 * @soundcard: the #GObject defaulting to
 *
 * Creates an #AgsCopyRecycling
 *
 * Returns: a new #AgsCopyRecycling
 *
 * Since: 0.4
 */
AgsCopyRecycling*
ags_copy_recycling_new(AgsRecycling *destination,
		       AgsRecycling *source,
		       GObject *soundcard)
{
  AgsCopyRecycling *copy_recycling;

  copy_recycling = (AgsCopyRecycling *) g_object_new(AGS_TYPE_COPY_RECYCLING,
						     "soundcard\0", soundcard,
						     "destination\0", destination,
						     "source\0", source,
						     NULL);

  return(copy_recycling);
}
