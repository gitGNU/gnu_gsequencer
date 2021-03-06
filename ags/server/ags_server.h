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
#ifndef __AGS_SERVER_H__
#define __AGS_SERVER_H__

#include <glib.h>
#include <glib-object.h>

#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#ifdef AGS_WITH_XMLRPC_C
#include <xmlrpc-c/util.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/abyss.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#endif

#include <ags/server/ags_registry.h>
#include <ags/server/ags_remote_task.h>

#include <pthread.h>

#define AGS_TYPE_SERVER                (ags_server_get_type())
#define AGS_SERVER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SERVER, AgsServer))
#define AGS_SERVER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_SERVER, AgsServerClass))
#define AGS_IS_SERVER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_SERVER))
#define AGS_IS_SERVER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_SERVER))
#define AGS_SERVER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_SERVER, AgsServerClass))

typedef struct _AgsServer AgsServer;
typedef struct _AgsServerClass AgsServerClass;

typedef enum{
  AGS_SERVER_STARTED        = 1,
  AGS_SERVER_RUNNING        = 1 << 1,
}AgsServerFlags;

struct _AgsServer
{
  GObject object;

  guint flags;

#ifdef AGS_WITH_XMLRPC_C
  TServer abyss_server;
  TSocket *socket;
#endif
  int socket_fd;
  struct sockaddr_in address;

  void *server_info;
  
  GObject *application_context;
  pthread_mutex_t  *application_mutex;
  
  AgsRegistry *registry;
  AgsRemoteTask *remote_task;
};

struct _AgsServerClass
{
  GObjectClass object;
  
  void (*start)(AgsServer *server);
};

GType ags_server_get_type();

void ags_server_start(AgsServer *server);

AgsServer* ags_server_lookup(void *server_info);

#ifdef AGS_WITH_XMLRPC_C
xmlrpc_value* ags_server_create_object(xmlrpc_env *env,
				       xmlrpc_value *param_array,
				       void *server_info);

xmlrpc_value* ags_server_object_set_property(xmlrpc_env *env,
					     xmlrpc_value *param_array,
					     void *server_info);
#endif

AgsServer* ags_server_new(GObject *application_context);

#endif /*__AGS_SERVER_H__*/
