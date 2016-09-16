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

#include <ags/audio/task/ags_remove_soundcard.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_sound_provider.h>

void ags_remove_soundcard_class_init(AgsRemoveSoundcardClass *remove_soundcard);
void ags_remove_soundcard_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_remove_soundcard_init(AgsRemoveSoundcard *remove_soundcard);
void ags_remove_soundcard_connect(AgsConnectable *connectable);
void ags_remove_soundcard_disconnect(AgsConnectable *connectable);
void ags_remove_soundcard_finalize(GObject *gobject);

void ags_remove_soundcard_launch(AgsTask *task);

/**
 * SECTION:ags_remove_soundcard
 * @short_description: remove soundcard object to context
 * @title: AgsRemoveSoundcard
 * @section_id:
 * @include: ags/audio/task/ags_remove_soundcard.h
 *
 * The #AgsRemoveSoundcard task removes #AgsSoundcard to context.
 */

static gpointer ags_remove_soundcard_parent_class = NULL;
static AgsConnectableInterface *ags_remove_soundcard_parent_connectable_interface;

GType
ags_remove_soundcard_get_type()
{
  static GType ags_type_remove_soundcard = 0;

  if(!ags_type_remove_soundcard){
    static const GTypeInfo ags_remove_soundcard_info = {
      sizeof (AgsRemoveSoundcardClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_remove_soundcard_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRemoveSoundcard),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_remove_soundcard_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_remove_soundcard_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_remove_soundcard = g_type_register_static(AGS_TYPE_TASK,
						       "AgsRemoveSoundcard\0",
						       &ags_remove_soundcard_info,
						       0);

    g_type_add_interface_static(ags_type_remove_soundcard,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_remove_soundcard);
}

void
ags_remove_soundcard_class_init(AgsRemoveSoundcardClass *remove_soundcard)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_remove_soundcard_parent_class = g_type_class_peek_parent(remove_soundcard);

  /* gobject */
  gobject = (GObjectClass *) remove_soundcard;

  gobject->finalize = ags_remove_soundcard_finalize;

  /* task */
  task = (AgsTaskClass *) remove_soundcard;

  task->launch = ags_remove_soundcard_launch;
}

void
ags_remove_soundcard_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_remove_soundcard_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_remove_soundcard_connect;
  connectable->disconnect = ags_remove_soundcard_disconnect;
}

void
ags_remove_soundcard_init(AgsRemoveSoundcard *remove_soundcard)
{
  remove_soundcard->application_context = NULL;
  
  remove_soundcard->soundcard = NULL;
}

void
ags_remove_soundcard_connect(AgsConnectable *connectable)
{
  ags_remove_soundcard_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_remove_soundcard_disconnect(AgsConnectable *connectable)
{
  ags_remove_soundcard_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_remove_soundcard_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_remove_soundcard_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_remove_soundcard_launch(AgsTask *task)
{
  AgsRemoveSoundcard *remove_soundcard;
  
  remove_soundcard = AGS_REMOVE_SOUNDCARD(task);

  ags_sound_provider_set_soundcard(AGS_SOUND_PROVIDER(remove_soundcard->application_context),
				   g_list_remove(ags_sound_provider_get_soundcard(AGS_SOUND_PROVIDER(remove_soundcard->application_context)),
						 remove_soundcard->soundcard));
}

/**
 * ags_remove_soundcard_new:
 * @application_context: the #AgsApplicationContext
 * @soundcard: the #AgsSoundcard to remove
 *
 * Creates an #AgsRemoveSoundcard.
 *
 * Returns: an new #AgsRemoveSoundcard.
 *
 * Since: 0.7.65
 */
AgsRemoveSoundcard*
ags_remove_soundcard_new(AgsApplicationContext *application_context,
			 GObject *soundcard)
{
  AgsRemoveSoundcard *remove_soundcard;

  remove_soundcard = (AgsRemoveSoundcard *) g_object_new(AGS_TYPE_REMOVE_SOUNDCARD,
							 NULL);

  remove_soundcard->application_context = application_context;

  remove_soundcard->soundcard = soundcard;

  return(remove_soundcard);
}