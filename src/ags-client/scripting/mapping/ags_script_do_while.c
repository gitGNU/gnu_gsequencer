/* AGS Client - Advanced GTK Sequencer Client
 * Copyright (C) 2013 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags-client/scripting/mapping/ags_script_do_while.h>

#include <ags/object/ags_connectable.h>

#include <ags-client/scripting/mapping/ags_script_array.h>

void ags_script_do_while_class_init(AgsScriptControllerClass *script_do_while);
void ags_script_do_while_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_script_do_while_init(AgsScriptDoWhile *script_do_while);
void ags_script_do_while_connect(AgsConnectable *connectable);
void ags_script_do_while_disconnect(AgsConnectable *connectable);
void ags_script_do_while_finalize(GObject *gobject);

AgsScriptObject* ags_script_do_while_launch(AgsScriptObject *script_object, GError **error);

static gpointer ags_script_do_while_parent_class = NULL;

GType
ags_script_do_while_get_type()
{
  static GType ags_type_script_do_while = 0;

  if(!ags_type_script_do_while){
    static const GTypeInfo ags_script_do_while_info = {
      sizeof (AgsScriptControllerClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_script_do_while_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsScriptController),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_script_do_while_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_script_do_while_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_script_do_while = g_type_register_static(AGS_TYPE_SCRIPT_CONTROLLER,
						      "AgsScriptController\0",
						      &ags_script_do_while_info,
						      0);
    
    g_type_add_interface_static(ags_type_script_do_while,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_script_do_while);
}

void
ags_script_do_while_class_init(AgsScriptControllerClass *script_do_while)
{
  AgsScriptObjectClass *script_object;
  GObjectClass *gobject;

  ags_script_do_while_parent_class = g_type_class_peek_parent(script_do_while);

  /* GObjectClass */
  gobject = (GObjectClass *) script_do_while;

  gobject->finalize = ags_script_do_while_finalize;

  /* AgsScriptObjectClass */
  script_object = (AgsScriptObjectClass *) script_do_while;

  script_object->launch = ags_script_do_while_launch;
}

void
ags_script_do_while_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_script_do_while_connect;
  connectable->disconnect = ags_script_do_while_disconnect;
}

void
ags_script_do_while_init(AgsScriptDoWhile *script_do_while)
{
  script_do_while->loop_control = ags_script_set_new();
}

void
ags_script_do_while_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_do_while_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_do_while_finalize(GObject *gobject)
{
  AgsScriptDoWhile *script_do_while;

  script_do_while = AGS_SCRIPT_DO_WHILE(gobject);

  g_object_unref(G_OBJECT(script_do_while->loop_control));

  G_OBJECT_CLASS(ags_script_do_while_parent_class)->finalize(gobject);
}

AgsScriptObject*
ags_script_do_while_launch(AgsScriptObject *script_object, GError **error)
{
  AgsScriptDoWhile *script_do_while;
  AgsScriptSet *loop_control;

  script_do_while = AGS_SCRIPT_DO_WHILE(script_object);
  loop_control = AGS_SCRIPT_SET(script_do_while->loop_control);

  do{
    AGS_SCRIPT_OBJECT_CLASS(ags_script_do_while_parent_class)->launch(script_object, error);
  }while(ags_script_set_boolean_term(loop_control));
}

AgsScriptDoWhile*
ags_script_do_while_new()
{
  AgsScriptDoWhile *script_do_while;

  script_do_while = (AgsScriptDoWhile *) g_object_new(AGS_TYPE_SCRIPT_DO_WHILE,
						      NULL);
  
  return(script_do_while);
}
