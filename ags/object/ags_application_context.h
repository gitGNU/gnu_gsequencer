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

#ifndef __AGS_APPLICATION_CONTEXT_H__
#define __AGS_APPLICATION_CONTEXT_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/object/ags_config.h>

#include <ags/file/ags_file.h>

#define AGS_TYPE_APPLICATION_CONTEXT                (ags_application_context_get_type())
#define AGS_APPLICATION_CONTEXT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_APPLICATION_CONTEXT, AgsApplicationContext))
#define AGS_APPLICATION_CONTEXT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_APPLICATION_CONTEXT, AgsApplicationContextClass))
#define AGS_IS_APPLICATION_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_APPLICATION_CONTEXT))
#define AGS_IS_APPLICATION_CONTEXT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_APPLICATION_CONTEXT))
#define AGS_APPLICATION_CONTEXT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_APPLICATION_CONTEXT, AgsApplicationContextClass))

#ifndef PACKAGE_VERSION
#define AGS_VERSION "0.7.0\0"
#else
#define AGS_VERSION PACKAGE_VERSION
#endif

#define AGS_BUILD_ID "CEST 13-10-2015 04:33\0"

#define AGS_DEFAULT_DIRECTORY ".gsequencer\0"
#define AGS_DEFAULT_CONFIG "ags.conf\0"

#define AGS_INIT_CONTEXT_TSD_APPLICATION_CONTEXT "ags-application-context\0"

typedef struct _AgsApplicationContext AgsApplicationContext;
typedef struct _AgsApplicationContextClass AgsApplicationContextClass;

typedef enum{
  AGS_APPLICATION_CONTEXT_DEFAULT            = 1,
  AGS_APPLICATION_CONTEXT_REGISTER_TYPES     = 1 << 1,
  AGS_APPLICATION_CONTEXT_ADD_TO_REGISTRY    = 1 << 2,
  AGS_APPLICATION_CONTEXT_CONNECT            = 1 << 3,
  AGS_APPLICATION_CONTEXT_TYPES_REGISTERED   = 1 << 4,
  AGS_APPLICATION_CONTEXT_READY              = 1 << 5,
  AGS_APPLICATION_CONTEXT_CONNECTED          = 1 << 6,
}AgsApplicationContextFlags;

struct _AgsApplicationContext
{
  GObject object;

  guint flags;

  gchar *version;
  gchar *build_id;

  int argc;
  char **argv;
  
  GObject *log;

  gchar *domain;
  GList *sibling;
  
  AgsConfig *config;

  pthread_mutexattr_t *mutexattr;
  pthread_mutex_t *mutex;
  
  GObject *main_loop;
  GObject *autosave_thread;
  GObject *task_thread;
  
  AgsFile *file;
  GObject *history;
};

struct _AgsApplicationContextClass
{
  GObjectClass object;

  void (*load_config)(AgsApplicationContext *application_context);
  
  void (*register_types)(AgsApplicationContext *application_context);

  void (*read)(AgsFile *file, xmlNode *node, GObject **gobject);
  xmlNode* (*write)(AgsFile *file, xmlNode *parent, GObject *gobject);

  void (*quit)(AgsApplicationContext *application_context);
};

GType ags_application_context_get_type();

void ags_application_context_load_config(AgsApplicationContext *application_context);
void ags_application_context_register_types(AgsApplicationContext *application_context);

void ags_application_read(AgsFile *file, xmlNode *node, GObject **gobject);
xmlNode* ags_application_write(AgsFile *file, xmlNode *parent, GObject *gobject);

void ags_application_context_add_sibling(AgsApplicationContext *application_context,
					 AgsApplicationContext *sibling);
void ags_application_context_remove_sibling(AgsApplicationContext *application_context,
					    AgsApplicationContext *sibling);

AgsApplicationContext* ags_application_context_find_default(GList *application_context);
GList* ags_application_context_find_main_loop(GList *application_context);

void ags_application_context_quit(AgsApplicationContext *application_context);

AgsApplicationContext* ags_application_context_get_instance();
AgsApplicationContext* ags_application_context_new(GObject *main_loop,
						   AgsConfig *config);

#endif /*__AGS_APPLICATION_CONTEXT_H__*/
