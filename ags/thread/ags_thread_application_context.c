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

#include <ags/thread/ags_thread_application_context.h>

#include <ags/util/ags_id_generator.h>

#include <ags/object/ags_config.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_main_loop.h>

#include <ags/file/ags_file.h>
#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>

#include <ags/thread/ags_concurrency_provider.h>
#include <ags/thread/ags_thread-posix.h>
#include <ags/thread/ags_thread_pool.h>
#include <ags/thread/ags_generic_main_loop.h>
#include <ags/thread/ags_autosave_thread.h>
#include <ags/thread/ags_returnable_thread.h>
#include <ags/thread/ags_task_thread.h>
#include <ags/thread/ags_timestamp_thread.h>

#include <ags/thread/file/ags_thread_file_xml.h>

void ags_thread_application_context_class_init(AgsThreadApplicationContextClass *thread_application_context);
void ags_thread_application_context_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_thread_application_context_concurrency_provider_interface_init(AgsConcurrencyProviderInterface *concurrency_provider);
void ags_thread_application_context_init(AgsThreadApplicationContext *thread_application_context);
void ags_thread_application_context_set_property(GObject *gobject,
						 guint prop_id,
						 const GValue *value,
						 GParamSpec *param_spec);
void ags_thread_application_context_get_property(GObject *gobject,
						 guint prop_id,
						 GValue *value,
						 GParamSpec *param_spec);
void ags_thread_application_context_connect(AgsConnectable *connectable);
void ags_thread_application_context_disconnect(AgsConnectable *connectable);
AgsThread* ags_thread_application_context_get_main_loop(AgsConcurrencyProvider *concurrency_provider);
AgsThread* ags_thread_application_context_get_task_thread(AgsConcurrencyProvider *concurrency_provider);
AgsThreadPool* ags_thread_application_context_get_thread_pool(AgsConcurrencyProvider *concurrency_provider);
GList* ags_thread_application_context_get_worker(AgsConcurrencyProvider *concurrency_provider);
void ags_thread_application_context_set_worker(AgsConcurrencyProvider *concurrency_provider,
					       GList *worker);
void ags_thread_application_context_finalize(GObject *gobject);

void ags_thread_application_context_load_config(AgsApplicationContext *application_context);
void ags_thread_application_context_read(AgsFile *file, xmlNode *node, GObject **application_context);
xmlNode* ags_thread_application_context_write(AgsFile *file, xmlNode *parent, GObject *application_context);

void ags_thread_application_context_set_value_callback(AgsConfig *config, gchar *group, gchar *key, gchar *value,
						       AgsThreadApplicationContext *thread_application_context);

/**
 * SECTION:ags_thread_application_context
 * @short_description: threaded application context
 * @title: AgsThreadApplicationContext
 * @section_id:
 * @include: ags/thread/ags_thread_application_context.h
 *
 * The #AgsThreadApplicationContext provides you multi-threaded functionality.
 */

static gpointer ags_thread_application_context_parent_class = NULL;
static AgsConnectableInterface* ags_thread_application_context_parent_connectable_interface;

enum{
  PROP_0,
  PROP_AUTOSAVE_THREAD,
  PROP_THREAD_POOL,
};

GType
ags_thread_application_context_get_type()
{
  static GType ags_type_thread_application_context = 0;

  if(!ags_type_thread_application_context){
    static const GTypeInfo ags_thread_application_context_info = {
      sizeof (AgsThreadApplicationContextClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_thread_application_context_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsThreadApplicationContext),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_thread_application_context_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_thread_application_context_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_concurrency_provider_interface_info = {
      (GInterfaceInitFunc) ags_thread_application_context_concurrency_provider_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_thread_application_context = g_type_register_static(AGS_TYPE_APPLICATION_CONTEXT,
								 "AgsThreadApplicationContext\0",
								 &ags_thread_application_context_info,
								 0);

    g_type_add_interface_static(ags_type_thread_application_context,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_thread_application_context,
				AGS_TYPE_CONCURRENCY_PROVIDER,
				&ags_concurrency_provider_interface_info);
  }

  return (ags_type_thread_application_context);
}

void
ags_thread_application_context_class_init(AgsThreadApplicationContextClass *thread_application_context)
{
  GObjectClass *gobject;
  AgsApplicationContextClass *application_context;
  GParamSpec *param_spec;
  
  ags_thread_application_context_parent_class = g_type_class_peek_parent(thread_application_context);

  /* GObjectClass */
  gobject = (GObjectClass *) thread_application_context;

  gobject->set_property = ags_thread_application_context_set_property;
  gobject->get_property = ags_thread_application_context_get_property;

  gobject->finalize = ags_thread_application_context_finalize;

  /**
   * AgsThreadApplicationContext:autosave-thread:
   *
   * The assigned thread pool.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("autosave-thread\0",
				   "thread pool of thread application context\0",
				   "The thread pool which this thread application context assigned to\0",
				   AGS_TYPE_AUTOSAVE_THREAD,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_THREAD_POOL,
				  param_spec);


  /**
   * AgsThreadApplicationContext:thread-pool:
   *
   * The assigned thread pool.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("thread-pool\0",
				   "thread pool of thread application context\0",
				   "The thread pool which this thread application context assigned to\0",
				   AGS_TYPE_THREAD_POOL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_THREAD_POOL,
				  param_spec);

  /* AgsThreadApplicationContextClass */
  application_context = (AgsApplicationContextClass *) thread_application_context;
  
  application_context->load_config = ags_thread_application_context_load_config;
  application_context->register_types = ags_thread_application_context_register_types;
  application_context->read = ags_thread_application_context_read;
  application_context->write = ags_thread_application_context_write;
}

void
ags_thread_application_context_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_thread_application_context_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_thread_application_context_connect;
  connectable->disconnect = ags_thread_application_context_disconnect;
}

void
ags_thread_application_context_concurrency_provider_interface_init(AgsConcurrencyProviderInterface *concurrency_provider)
{
  concurrency_provider->get_main_loop = ags_thread_application_context_get_main_loop;
  concurrency_provider->get_task_thread = ags_thread_application_context_get_task_thread;
  concurrency_provider->get_thread_pool = ags_thread_application_context_get_thread_pool;
  concurrency_provider->get_worker = ags_thread_application_context_get_worker;
  concurrency_provider->set_worker = ags_thread_application_context_set_worker;
}

void
ags_thread_application_context_init(AgsThreadApplicationContext *thread_application_context)
{
  AgsGenericMainLoop *generic_main_loop;
  
  thread_application_context->flags = 0;

  /* AgsGenericMainLoop */
  generic_main_loop = (AgsGenericMainLoop *) ags_generic_main_loop_new((GObject *) thread_application_context);
  AGS_APPLICATION_CONTEXT(thread_application_context)->main_loop = (GObject *) generic_main_loop;
  g_object_set(thread_application_context,
	       "main-loop\0", generic_main_loop,
	       NULL);

  g_object_ref(generic_main_loop);
  ags_connectable_connect(AGS_CONNECTABLE(generic_main_loop));

  /* AgsTaskThread */
  AGS_APPLICATION_CONTEXT(thread_application_context)->task_thread = (GObject *) ags_task_thread_new();
  ags_thread_add_child_extended(AGS_THREAD(generic_main_loop),
				AGS_THREAD(AGS_APPLICATION_CONTEXT(thread_application_context)->task_thread),
				TRUE, TRUE);

  ags_main_loop_set_async_queue(AGS_MAIN_LOOP(generic_main_loop),
				AGS_APPLICATION_CONTEXT(thread_application_context)->task_thread);

  /* AgsAutosaveThread */
  thread_application_context->autosave_thread = NULL;

  /* AgsWorkerThread */
  thread_application_context->worker = NULL;

  /* AgsThreadPool */
  thread_application_context->thread_pool = AGS_TASK_THREAD(AGS_APPLICATION_CONTEXT(thread_application_context)->task_thread)->thread_pool;
}

void
ags_thread_application_context_set_property(GObject *gobject,
					    guint prop_id,
					    const GValue *value,
					    GParamSpec *param_spec)
{
  AgsThreadApplicationContext *thread_application_context;

  thread_application_context = AGS_THREAD_APPLICATION_CONTEXT(gobject);

  switch(prop_id){
  case PROP_AUTOSAVE_THREAD:
    {
      AgsAutosaveThread *autosave_thread;
      
      autosave_thread = (AgsAutosaveThread *) g_value_get_object(value);

      if(autosave_thread == (AgsAutosaveThread *) thread_application_context->autosave_thread)
	return;

      if(thread_application_context->autosave_thread != NULL){
	g_object_unref(thread_application_context->autosave_thread);
      }
      
      if(autosave_thread != NULL){
	g_object_ref(G_OBJECT(autosave_thread));
      }
      
      thread_application_context->autosave_thread = (AgsThread *) autosave_thread;
    }
    break;
  case PROP_THREAD_POOL:
    {
      AgsThreadPool *thread_pool;
      
      thread_pool = (AgsThreadPool *) g_value_get_object(value);

      if(thread_pool == thread_application_context->thread_pool)
	return;

      if(thread_application_context->thread_pool != NULL)
	g_object_unref(thread_application_context->thread_pool);

      if(thread_pool != NULL)
	g_object_ref(G_OBJECT(thread_pool));

      thread_application_context->thread_pool = thread_pool;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_thread_application_context_get_property(GObject *gobject,
					    guint prop_id,
					    GValue *value,
					    GParamSpec *param_spec)
{
  AgsThreadApplicationContext *thread_application_context;

  thread_application_context = AGS_THREAD_APPLICATION_CONTEXT(gobject);

  switch(prop_id){
  case PROP_AUTOSAVE_THREAD:
    {
      g_value_set_object(value, thread_application_context->autosave_thread);
    }
    break;
  case PROP_THREAD_POOL:
    {
      g_value_set_object(value, thread_application_context->thread_pool);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_thread_application_context_connect(AgsConnectable *connectable)
{
  AgsThreadApplicationContext *thread_application_context;

  thread_application_context = AGS_THREAD_APPLICATION_CONTEXT(connectable);

  if((AGS_APPLICATION_CONTEXT_CONNECTED & (AGS_APPLICATION_CONTEXT(thread_application_context)->flags)) != 0){
    return;
  }

  ags_thread_application_context_parent_connectable_interface->connect(connectable);

  ags_connectable_connect(AGS_CONNECTABLE(thread_application_context->autosave_thread));

  ags_connectable_connect(AGS_CONNECTABLE(AGS_APPLICATION_CONTEXT(thread_application_context)->main_loop));
  ags_connectable_connect(AGS_CONNECTABLE(thread_application_context->thread_pool));
}

void
ags_thread_application_context_disconnect(AgsConnectable *connectable)
{
  AgsThreadApplicationContext *thread_application_context;

  thread_application_context = AGS_THREAD_APPLICATION_CONTEXT(connectable);

  if((AGS_APPLICATION_CONTEXT_CONNECTED & (AGS_APPLICATION_CONTEXT(thread_application_context)->flags)) == 0){
    return;
  }
  
  ags_connectable_disconnect(AGS_CONNECTABLE(thread_application_context->autosave_thread));

  ags_connectable_disconnect(AGS_CONNECTABLE(AGS_APPLICATION_CONTEXT(thread_application_context)->main_loop));
  ags_connectable_disconnect(AGS_CONNECTABLE(thread_application_context->thread_pool));

  ags_thread_application_context_parent_connectable_interface->disconnect(connectable);
}

AgsThread*
ags_thread_application_context_get_main_loop(AgsConcurrencyProvider *concurrency_provider)
{
  return((AgsThread *) AGS_APPLICATION_CONTEXT(concurrency_provider)->main_loop);
}

AgsThread*
ags_thread_application_context_get_task_thread(AgsConcurrencyProvider *concurrency_provider)
{
  return((AgsThread *) AGS_APPLICATION_CONTEXT(concurrency_provider)->task_thread);
}

AgsThreadPool*
ags_thread_application_context_get_thread_pool(AgsConcurrencyProvider *concurrency_provider)
{
  return((AgsThreadPool *) AGS_THREAD_APPLICATION_CONTEXT(concurrency_provider)->thread_pool);
}

GList*
ags_thread_application_context_get_worker(AgsConcurrencyProvider *concurrency_provider)
{
  return(AGS_THREAD_APPLICATION_CONTEXT(concurrency_provider)->worker);
}

void
ags_thread_application_context_set_worker(AgsConcurrencyProvider *concurrency_provider,
					  GList *worker)
{
  AGS_THREAD_APPLICATION_CONTEXT(concurrency_provider)->worker = worker;
}

void
ags_thread_application_context_finalize(GObject *gobject)
{
  AgsThreadApplicationContext *thread_application_context;

  thread_application_context = AGS_THREAD_APPLICATION_CONTEXT(gobject);

  if(thread_application_context->autosave_thread != NULL){
    g_object_unref(thread_application_context->autosave_thread);
  }

  if(thread_application_context->thread_pool != NULL){
    g_object_unref(thread_application_context->thread_pool);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_thread_application_context_parent_class)->finalize(gobject);
}

void
ags_thread_application_context_load_config(AgsApplicationContext *application_context)
{
  AgsConfig *config;
  gchar *model;

  config = application_context->config;
  
  model = ags_config_get_value(config,
			       AGS_CONFIG_THREAD,
			       "model\0");
    
  if(model != NULL){
    if(!g_ascii_strncasecmp(model,
			    "single-threaded\0",
			    16)){
      //TODO:JK: implement me
	
    }else if(!g_ascii_strncasecmp(model,
				  "multi-threaded",
				  15)){
      //TODO:JK: implement me
    }else if(!g_ascii_strncasecmp(model,
				  "super-threaded",
				  15)){
      //TODO:JK: implement me
    }
  }
}

void
ags_thread_application_context_register_types(AgsApplicationContext *application_context)
{
  ags_thread_get_type();

  ags_task_thread_get_type();

  ags_timestamp_thread_get_type();

  ags_thread_pool_get_type();
  ags_returnable_thread_get_type();
}

void
ags_thread_application_context_read(AgsFile *file, xmlNode *node, GObject **application_context)
{
  AgsThreadApplicationContext *gobject;
  GList *list;
  xmlNode *child;

  if(*application_context == NULL){
    gobject = (AgsThreadApplicationContext *) g_object_new(AGS_TYPE_THREAD_APPLICATION_CONTEXT,
							   NULL);

    *application_context = (GObject *) gobject;
  }else{
    gobject = (AgsThreadApplicationContext *) *application_context;
  }

  file->application_context = (GObject *) gobject;

  g_object_set(G_OBJECT(file),
	       "application-context\0", gobject,
	       NULL);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));
  
  /* properties */
  AGS_APPLICATION_CONTEXT(gobject)->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
								     NULL,
								     16);

  AGS_APPLICATION_CONTEXT(gobject)->version = xmlGetProp(node,
							 AGS_FILE_VERSION_PROP);

  AGS_APPLICATION_CONTEXT(gobject)->build_id = xmlGetProp(node,
							  AGS_FILE_BUILD_ID_PROP);

  //TODO:JK: check version compatibelity

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp("ags-thread\0",
		     child->name,
		     11)){
	ags_file_read_thread(file,
			     child,
			     (AgsThread **) &(AGS_APPLICATION_CONTEXT(gobject)->main_loop));
      }else if(!xmlStrncmp("ags-thread-pool\0",
			   child->name,
			   16)){
	ags_file_read_thread_pool(file,
				  child,
				  (AgsThreadPool **) &(gobject->thread_pool));
      }
    }

    child = child->next;
  }

  //TODO:JK: decide about returnable thread
}

xmlNode*
ags_thread_application_context_write(AgsFile *file, xmlNode *parent, GObject *application_context)
{
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-application-context\0");

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", application_context,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_CONTEXT_PROP,
	     "thread\0");

  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", ((~AGS_APPLICATION_CONTEXT_CONNECTED) & (AGS_APPLICATION_CONTEXT(application_context)->flags))));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->build_id);

  /* add to parent */
  xmlAddChild(parent,
	      node);

  /* child elements */
  ags_file_write_thread(file,
			node,
			AGS_THREAD(AGS_APPLICATION_CONTEXT(application_context)->main_loop));

  ags_file_write_thread_pool(file,
			     node,
			     AGS_THREAD_POOL(AGS_THREAD_APPLICATION_CONTEXT(application_context)->thread_pool));

  return(node);
}

void
ags_thread_application_context_set_value_callback(AgsConfig *config, gchar *group, gchar *key, gchar *value,
						  AgsThreadApplicationContext *thread_application_context)
{
  if(!g_ascii_strncasecmp(group,
			  AGS_CONFIG_GENERIC,
			  8)){
    if(!g_ascii_strncasecmp(key,
			    "autosave-thread\0",
			    15)){
      AgsAutosaveThread *autosave_thread;

      if(thread_application_context == NULL ||
	 thread_application_context->autosave_thread == NULL){
	return;
      }
      
      autosave_thread = (AgsAutosaveThread *) thread_application_context->autosave_thread;

      if(!g_ascii_strncasecmp(value,
			      "true\0",
			      5)){
	ags_thread_start((AgsThread *) autosave_thread);
      }else{
	ags_thread_stop((AgsThread *) autosave_thread);
      }
    }
  }else if(!g_ascii_strncasecmp(group,
				AGS_CONFIG_THREAD,
				7)){
    if(!g_ascii_strncasecmp(key,
			    "model\0",
			    6)){
      //TODO:JK: implement me
    }else if(!g_ascii_strncasecmp(key,
				  "lock-global\0",
				  11)){
      //TODO:JK: implement me
    }else if(!g_ascii_strncasecmp(key,
				  "lock-parent\0",
				  11)){
      //TODO:JK: implement me
    }
  }
}

AgsThreadApplicationContext*
ags_thread_application_context_new()
{
  AgsThreadApplicationContext *thread_application_context;

  thread_application_context = (AgsThreadApplicationContext *) g_object_new(AGS_TYPE_THREAD_APPLICATION_CONTEXT,
									    NULL);

  return(thread_application_context);
}
