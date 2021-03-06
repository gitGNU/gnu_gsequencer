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

#include <ags/audio/task/ags_save_file.h>

#include <ags/object/ags_connectable.h>

void ags_save_file_class_init(AgsSaveFileClass *save_file);
void ags_save_file_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_save_file_init(AgsSaveFile *save_file);
void ags_save_file_set_property(GObject *gobject,
				guint prop_id,
				const GValue *value,
				GParamSpec *param_spec);
void ags_save_file_get_property(GObject *gobject,
				guint prop_id,
				GValue *value,
				GParamSpec *param_spec);
void ags_save_file_connect(AgsConnectable *connectable);
void ags_save_file_disconnect(AgsConnectable *connectable);
void ags_save_file_finalize(GObject *gobject);

void ags_save_file_launch(AgsTask *task);

/**
 * SECTION:ags_save_file
 * @short_description: save file task
 * @title: AgsSaveFile
 * @section_id:
 * @include: ags/audio/task/ags_save_file.h
 *
 * The #AgsSaveFile task saves files.
 */

static gpointer ags_save_file_parent_class = NULL;
static AgsConnectableInterface *ags_save_file_parent_connectable_interface;

enum{
  PROP_0,
  PROP_FILE,
};

GType
ags_save_file_get_type()
{
  static GType ags_type_save_file = 0;

  if(!ags_type_save_file){
    static const GTypeInfo ags_save_file_info = {
      sizeof (AgsSaveFileClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_save_file_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSaveFile),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_save_file_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_save_file_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_save_file = g_type_register_static(AGS_TYPE_TASK,
						"AgsSaveFile\0",
						&ags_save_file_info,
						0);

    g_type_add_interface_static(ags_type_save_file,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_save_file);
}

void
ags_save_file_class_init(AgsSaveFileClass *save_file)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_save_file_parent_class = g_type_class_peek_parent(save_file);

  /* gobject */
  gobject = (GObjectClass *) save_file;

  gobject->set_property = ags_save_file_set_property;
  gobject->get_property = ags_save_file_get_property;

  gobject->finalize = ags_save_file_finalize;

  /* properties */
  /**
   * AgsSaveFile:file:
   *
   * The assigned #AgsFile
   * 
   * Since: 0.7.117
   */
  param_spec = g_param_spec_object("file\0",
				   "file of save file\0",
				   "The file of save file task\0",
				   AGS_TYPE_FILE,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FILE,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) save_file;

  task->launch = ags_save_file_launch;
}

void
ags_save_file_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_save_file_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_save_file_connect;
  connectable->disconnect = ags_save_file_disconnect;
}

void
ags_save_file_init(AgsSaveFile *save_file)
{
  save_file->file = NULL;
}

void
ags_save_file_set_property(GObject *gobject,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *param_spec)
{
  AgsSaveFile *save_file;

  save_file = AGS_SAVE_FILE(gobject);

  switch(prop_id){
  case PROP_FILE:
    {
      AgsFile *file;

      file = (AgsFile *) g_value_get_object(value);

      if(save_file->file == (GObject *) file){
	return;
      }

      if(save_file->file != NULL){
	g_object_unref(save_file->file);
      }

      if(file != NULL){
	g_object_ref(file);
      }

      save_file->file = (GObject *) file;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_save_file_get_property(GObject *gobject,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *param_spec)
{
  AgsSaveFile *save_file;

  save_file = AGS_SAVE_FILE(gobject);

  switch(prop_id){
  case PROP_FILE:
    {
      g_value_set_object(value, save_file->file);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_save_file_connect(AgsConnectable *connectable)
{
  ags_save_file_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_save_file_disconnect(AgsConnectable *connectable)
{
  ags_save_file_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_save_file_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_save_file_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_save_file_launch(AgsTask *task)
{
  AgsSaveFile *save_file;
  GError *error;

  save_file = AGS_SAVE_FILE(task);

  g_message("Saving to: %s\0", save_file->file->filename);
  error = NULL;
  ags_file_rw_open(save_file->file,
		   TRUE,
		   &error);
  ags_file_write(save_file->file);
  ags_file_close(save_file->file);
}

/**
 * ags_save_file_new:
 * @file: the #AgsFile
 *
 * Creates an #AgsSaveFile.
 *
 * Returns: an new #AgsSaveFile.
 *
 * Since: 0.4
 */
AgsSaveFile*
ags_save_file_new(AgsFile *file)
{
  AgsSaveFile *save_file;

  save_file = (AgsSaveFile *) g_object_new(AGS_TYPE_SAVE_FILE,
					   NULL);
  g_object_ref(file);
  save_file->file = file;

  return(save_file);
}

