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

#include <ags/X/ags_xorg_application_context.h>

#include <ags/object/ags_distributed_manager.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_config.h>
#include <ags/object/ags_main_loop.h>

#include <ags/file/ags_file.h>
#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>

#include <ags/thread/ags_concurrency_provider.h>
#include <ags/thread/ags_thread-posix.h>
#include <ags/thread/ags_thread_pool.h>

#include <ags/audio/ags_sound_provider.h>
#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_midiin.h>

#include <ags/audio/jack/ags_jack_midiin.h>
#include <ags/audio/jack/ags_jack_server.h>

#include <ags/X/file/ags_gui_file_xml.h>

#include <ags/X/thread/ags_gui_thread.h>

#include <sys/types.h>
#include <pwd.h>

#include <sys/mman.h>

#include <jack/jslist.h>
#include <jack/jack.h>
#include <jack/control.h>
#include <stdbool.h>

void ags_xorg_application_context_class_init(AgsXorgApplicationContextClass *xorg_application_context);
void ags_xorg_application_context_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_xorg_application_context_concurrency_provider_interface_init(AgsConcurrencyProviderInterface *concurrency_provider);
void ags_xorg_application_context_sound_provider_interface_init(AgsSoundProviderInterface *sound_provider);
void ags_xorg_application_context_init(AgsXorgApplicationContext *xorg_application_context);
void ags_xorg_application_context_set_property(GObject *gobject,
					       guint prop_id,
					       const GValue *value,
					       GParamSpec *param_spec);
void ags_xorg_application_context_get_property(GObject *gobject,
					       guint prop_id,
					       GValue *value,
					       GParamSpec *param_spec);
void ags_xorg_application_context_connect(AgsConnectable *connectable);
void ags_xorg_application_context_disconnect(AgsConnectable *connectable);
AgsThread* ags_xorg_application_context_get_main_loop(AgsConcurrencyProvider *concurrency_provider);
AgsThread* ags_xorg_application_context_get_task_thread(AgsConcurrencyProvider *concurrency_provider);
AgsThreadPool* ags_xorg_application_context_get_thread_pool(AgsConcurrencyProvider *concurrency_provider);
GList* ags_xorg_application_context_get_soundcard(AgsSoundProvider *sound_provider);
void ags_xorg_application_context_set_soundcard(AgsSoundProvider *sound_provider,
						GList *soundcar);
GList* ags_xorg_application_context_get_sequencer(AgsSoundProvider *sound_provider);
void ags_xorg_application_context_set_sequencer(AgsSoundProvider *sound_provider,
						GList *sequencer);
GList* ags_xorg_application_context_get_distributed_manager(AgsSoundProvider *sound_provider);
void ags_xorg_application_context_finalize(GObject *gobject);

void ags_xorg_application_context_load_config(AgsApplicationContext *application_context);
void ags_xorg_application_context_register_types(AgsApplicationContext *application_context);

void ags_xorg_application_context_read(AgsFile *file, xmlNode *node, GObject **application_context);
xmlNode* ags_xorg_application_context_write(AgsFile *file, xmlNode *parent, GObject *application_context);

static gpointer ags_xorg_application_context_parent_class = NULL;
static AgsConnectableInterface* ags_xorg_application_context_parent_connectable_interface;

enum{
  PROP_0,
  PROP_WINDOW,
};

AgsXorgApplicationContext *ags_xorg_application_context;

GType
ags_xorg_application_context_get_type()
{
  static GType ags_type_xorg_application_context = 0;

  if(!ags_type_xorg_application_context){
    static const GTypeInfo ags_xorg_application_context_info = {
      sizeof (AgsXorgApplicationContextClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_xorg_application_context_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsXorgApplicationContext),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_xorg_application_context_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_xorg_application_context_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_concurrency_provider_interface_info = {
      (GInterfaceInitFunc) ags_xorg_application_context_concurrency_provider_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_sound_provider_interface_info = {
      (GInterfaceInitFunc) ags_xorg_application_context_sound_provider_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_xorg_application_context = g_type_register_static(AGS_TYPE_APPLICATION_CONTEXT,
							       "AgsXorgApplicationContext\0",
							       &ags_xorg_application_context_info,
							       0);

    g_type_add_interface_static(ags_type_xorg_application_context,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_xorg_application_context,
				AGS_TYPE_CONCURRENCY_PROVIDER,
				&ags_concurrency_provider_interface_info);

    g_type_add_interface_static(ags_type_xorg_application_context,
				AGS_TYPE_SOUND_PROVIDER,
				&ags_sound_provider_interface_info);
  }

  return (ags_type_xorg_application_context);
}

void
ags_xorg_application_context_class_init(AgsXorgApplicationContextClass *xorg_application_context)
{
  GObjectClass *gobject;
  AgsApplicationContextClass *application_context;
  GParamSpec *param_spec;

  ags_xorg_application_context_parent_class = g_type_class_peek_parent(xorg_application_context);

  /* GObjectClass */
  gobject = (GObjectClass *) xorg_application_context;

  gobject->set_property = ags_xorg_application_context_set_property;
  gobject->get_property = ags_xorg_application_context_get_property;

  gobject->finalize = ags_xorg_application_context_finalize;
  
  /**
   * AgsXorgApplicationContext:window:
   *
   * The assigned window.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("window\0",
				   "window of xorg application context\0",
				   "The window which this xorg application context assigned to\0",
				   AGS_TYPE_WINDOW,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_WINDOW,
				  param_spec);

  /* AgsXorgApplicationContextClass */
  application_context = (AgsApplicationContextClass *) xorg_application_context;
  
  application_context->load_config = ags_xorg_application_context_load_config;
  application_context->register_types = ags_xorg_application_context_register_types;
}

void
ags_xorg_application_context_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_xorg_application_context_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_xorg_application_context_connect;
  connectable->disconnect = ags_xorg_application_context_disconnect;
}

void
ags_xorg_application_context_concurrency_provider_interface_init(AgsConcurrencyProviderInterface *concurrency_provider)
{
  concurrency_provider->get_main_loop = ags_xorg_application_context_get_main_loop;
  concurrency_provider->get_task_thread = ags_xorg_application_context_get_task_thread;
  concurrency_provider->get_thread_pool = ags_xorg_application_context_get_thread_pool;
}

void
ags_xorg_application_context_sound_provider_interface_init(AgsSoundProviderInterface *sound_provider)
{
  sound_provider->get_soundcard = ags_xorg_application_context_get_soundcard;
  sound_provider->set_soundcard = ags_xorg_application_context_set_soundcard;
  sound_provider->get_sequencer = ags_xorg_application_context_get_sequencer;
  sound_provider->set_sequencer = ags_xorg_application_context_set_sequencer;
  sound_provider->get_distributed_manager = ags_xorg_application_context_get_distributed_manager;
}

void
ags_xorg_application_context_init(AgsXorgApplicationContext *xorg_application_context)
{
  AgsWindow *window;

  AgsServer *server;

  AgsAudioLoop *audio_loop;
  GObject *soundcard;
  GObject *sequencer;
  AgsJackServer *jack_server;
  
  AgsConfig *config;

  JSList *jslist;
  struct passwd *pw;
  uid_t uid;
  gchar *wdir, *config_file;

  AGS_APPLICATION_CONTEXT(xorg_application_context)->log = NULL;

  /**/
  config = ags_config_get_instance();
  AGS_APPLICATION_CONTEXT(xorg_application_context)->config = config;
  g_object_set(config,
	       "application-context\0", xorg_application_context,
	       NULL);

  uid = getuid();
  pw = getpwuid(uid);

  wdir = g_strdup_printf("%s/%s\0",
			 pw->pw_dir,
			 AGS_DEFAULT_DIRECTORY);

  config_file = g_strdup_printf("%s/%s\0",
				wdir,
				AGS_DEFAULT_CONFIG);

  ags_config_load_from_file(config,
			    config_file);

  g_free(wdir);
  g_free(config_file);

  /* distributed manager */
  xorg_application_context->distributed_manager = NULL;

  jack_server = ags_jack_server_new(xorg_application_context,
				    NULL);
  xorg_application_context->distributed_manager = g_list_prepend(xorg_application_context->distributed_manager,
								 jack_server);
  g_object_ref(G_OBJECT(jack_server));

  /* AgsSoundcard */
  xorg_application_context->soundcard = NULL;

  soundcard = ags_devout_new(xorg_application_context);
  xorg_application_context->soundcard = g_list_prepend(xorg_application_context->soundcard,
						       soundcard);
  g_object_ref(G_OBJECT(soundcard));

  jslist = jackctl_server_get_drivers_list(jack_server->jackctl);
  //  jackctl_server_start(jack_server->jackctl);
  jackctl_server_open(jack_server->jackctl,
  		      jslist->data);
    
  soundcard = ags_distributed_manager_register_soundcard(AGS_DISTRIBUTED_MANAGER(jack_server),
							 TRUE);
  xorg_application_context->soundcard = g_list_prepend(xorg_application_context->soundcard,
						       soundcard);
  g_object_ref(G_OBJECT(soundcard));

  if(soundcard != NULL){
    xorg_application_context->soundcard = g_list_prepend(xorg_application_context->soundcard,
							 soundcard);
    g_object_ref(G_OBJECT(soundcard));
  }

  /* AgsSequencer */
  xorg_application_context->sequencer = NULL;

  sequencer = ags_midiin_new(xorg_application_context);
  xorg_application_context->sequencer = g_list_prepend(xorg_application_context->sequencer,
						       sequencer);
  g_object_ref(G_OBJECT(sequencer));

  sequencer = ags_distributed_manager_register_sequencer(AGS_DISTRIBUTED_MANAGER(jack_server),
							 FALSE);

  if(sequencer != NULL){
    xorg_application_context->sequencer = g_list_prepend(xorg_application_context->sequencer,
							 sequencer);
    g_object_ref(G_OBJECT(sequencer));
  }
  
  /* AgsWindow */
  window = ags_window_new(xorg_application_context);
  g_object_set(window,
	       "soundcard\0", soundcard,
	       NULL);
  AGS_XORG_APPLICATION_CONTEXT(xorg_application_context)->window = window;
  g_object_ref(G_OBJECT(window));

  gtk_window_set_default_size((GtkWindow *) window, 500, 500);
  gtk_paned_set_position((GtkPaned *) window->paned, 300);

  ags_connectable_connect(AGS_CONNECTABLE(window));
  gtk_widget_show_all((GtkWidget *) window);

  /* AgsServer */
  xorg_application_context->server = ags_server_new(xorg_application_context);

  /* AgsAudioLoop */
  AGS_APPLICATION_CONTEXT(xorg_application_context)->main_loop = 
    audio_loop = (AgsThread *) ags_audio_loop_new((GObject *) soundcard,
						  xorg_application_context);
  g_object_set(xorg_application_context,
	       "main-loop\0", audio_loop,
	       NULL);

  g_object_ref(audio_loop);
  ags_connectable_connect(AGS_CONNECTABLE(audio_loop));

  /* AgsTaskThread */
  AGS_APPLICATION_CONTEXT(xorg_application_context)->task_thread = (AgsThread *) ags_task_thread_new();
  g_atomic_pointer_set(&(AGS_THREAD(audio_loop)->start_queue),
		       g_list_append(g_atomic_pointer_get(&(AGS_THREAD(audio_loop)->start_queue)),
				     AGS_APPLICATION_CONTEXT(xorg_application_context)->task_thread));
  ags_thread_add_child_extended(AGS_THREAD(audio_loop),
				AGS_APPLICATION_CONTEXT(xorg_application_context)->task_thread,
				TRUE, TRUE);

  ags_main_loop_set_async_queue(AGS_MAIN_LOOP(audio_loop),
				AGS_APPLICATION_CONTEXT(xorg_application_context)->task_thread);
  
  /* AgsSoundcardThread */
  xorg_application_context->soundcard_thread = (AgsThread *) ags_soundcard_thread_new(soundcard);
  ags_thread_add_child_extended(AGS_THREAD(audio_loop),
				xorg_application_context->soundcard_thread,
				TRUE, TRUE);

  /* AgsExportThread */
  xorg_application_context->export_thread = (AgsThread *) ags_export_thread_new(soundcard,
										NULL);
  ags_thread_add_child_extended(AGS_THREAD(audio_loop),
				xorg_application_context->export_thread,
				TRUE, TRUE);

  /* AgsGuiThread */
  xorg_application_context->gui_thread = (AgsThread *) ags_gui_thread_new();
  g_atomic_pointer_set(&(AGS_THREAD(audio_loop)->start_queue),
		       g_list_append(g_atomic_pointer_get(&(AGS_THREAD(audio_loop)->start_queue)),
				     xorg_application_context->gui_thread));
  ags_thread_add_child_extended(AGS_THREAD(audio_loop),
				xorg_application_context->gui_thread,
				TRUE, TRUE);

  /* AgsThreadPool */
  AGS_XORG_APPLICATION_CONTEXT(xorg_application_context)->thread_pool = AGS_TASK_THREAD(AGS_APPLICATION_CONTEXT(xorg_application_context)->task_thread)->thread_pool;
}

void
ags_xorg_application_context_set_property(GObject *gobject,
					  guint prop_id,
					  const GValue *value,
					  GParamSpec *param_spec)
{
  AgsXorgApplicationContext *xorg_application_context;

  xorg_application_context = AGS_XORG_APPLICATION_CONTEXT(gobject);

  switch(prop_id){
  case PROP_WINDOW:
    {
      AgsWindow *window;
      
      window = (AgsWindow *) g_value_get_object(value);

      if(window == xorg_application_context->window)
	return;

      if(xorg_application_context->window != NULL)
	g_object_unref(xorg_application_context->window);

      if(window != NULL)
	g_object_ref(G_OBJECT(window));

      xorg_application_context->window = window;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_xorg_application_context_get_property(GObject *gobject,
					  guint prop_id,
					  GValue *value,
					  GParamSpec *param_spec)
{
  AgsXorgApplicationContext *xorg_application_context;

  xorg_application_context = AGS_XORG_APPLICATION_CONTEXT(gobject);

  switch(prop_id){
  case PROP_WINDOW:
    {
      g_value_set_object(value, xorg_application_context->window);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_xorg_application_context_connect(AgsConnectable *connectable)
{
  AgsXorgApplicationContext *xorg_application_context;

  xorg_application_context = AGS_XORG_APPLICATION_CONTEXT(connectable);

  if((AGS_APPLICATION_CONTEXT_CONNECTED & (AGS_APPLICATION_CONTEXT(xorg_application_context)->flags)) != 0){
    return;
  }

  ags_xorg_application_context_parent_connectable_interface->connect(connectable);

  g_message("connecting gui\0");

  ags_connectable_connect(AGS_CONNECTABLE(xorg_application_context->window));
}

void
ags_xorg_application_context_disconnect(AgsConnectable *connectable)
{
  AgsXorgApplicationContext *xorg_application_context;

  xorg_application_context = AGS_XORG_APPLICATION_CONTEXT(connectable);

  if((AGS_APPLICATION_CONTEXT_CONNECTED & (AGS_APPLICATION_CONTEXT(xorg_application_context)->flags)) == 0){
    return;
  }

  ags_xorg_application_context_parent_connectable_interface->disconnect(connectable);
}

AgsThread*
ags_xorg_application_context_get_main_loop(AgsConcurrencyProvider *concurrency_provider)
{
  return(AGS_APPLICATION_CONTEXT(concurrency_provider)->main_loop);
}

AgsThread*
ags_xorg_application_context_get_task_thread(AgsConcurrencyProvider *concurrency_provider)
{
  return(AGS_APPLICATION_CONTEXT(concurrency_provider)->task_thread);
}

AgsThreadPool*
ags_xorg_application_context_get_thread_pool(AgsConcurrencyProvider *concurrency_provider)
{
  return(AGS_XORG_APPLICATION_CONTEXT(concurrency_provider)->thread_pool);
}

GList*
ags_xorg_application_context_get_soundcard(AgsSoundProvider *sound_provider)
{
  return(AGS_XORG_APPLICATION_CONTEXT(sound_provider)->soundcard);
}

void
ags_xorg_application_context_set_soundcard(AgsSoundProvider *sound_provider,
					   GList *soundcard)
{
  AGS_XORG_APPLICATION_CONTEXT(sound_provider)->soundcard = soundcard;
}

GList*
ags_xorg_application_context_get_sequencer(AgsSoundProvider *sound_provider)
{
  return(AGS_XORG_APPLICATION_CONTEXT(sound_provider)->sequencer);
}

void
ags_xorg_application_context_set_sequencer(AgsSoundProvider *sound_provider,
					   GList *sequencer)
{
  AGS_XORG_APPLICATION_CONTEXT(sound_provider)->sequencer = sequencer;
}

GList*
ags_xorg_application_context_get_distributed_manager(AgsSoundProvider *sound_provider)
{
  return(AGS_XORG_APPLICATION_CONTEXT(sound_provider)->distributed_manager);
}

void
ags_xorg_application_context_finalize(GObject *gobject)
{
  AgsXorgApplicationContext *xorg_application_context;

  G_OBJECT_CLASS(ags_xorg_application_context_parent_class)->finalize(gobject);

  xorg_application_context = AGS_XORG_APPLICATION_CONTEXT(gobject);
}

void
ags_xorg_application_context_load_config(AgsApplicationContext *application_context)
{
  //TODO:JK: implement me
}

void
ags_xorg_application_context_register_types(AgsApplicationContext *application_context)
{
  ags_gui_thread_get_type();

  /* gui */
  //TODO:JK: move me
  ags_dial_get_type();

  /* register machine */
  ags_panel_get_type();
  ags_panel_input_pad_get_type();
  ags_panel_input_line_get_type();

  ags_mixer_get_type();
  ags_mixer_input_pad_get_type();
  ags_mixer_input_line_get_type();

  ags_drum_get_type();
  ags_drum_output_pad_get_type();
  ags_drum_output_line_get_type();
  ags_drum_input_pad_get_type();
  ags_drum_input_line_get_type();

  ags_matrix_get_type();

  ags_synth_get_type();
  ags_synth_input_pad_get_type();
  ags_synth_input_line_get_type();

  ags_ffplayer_get_type();
}

void
ags_xorg_application_context_read(AgsFile *file, xmlNode *node, GObject **application_context)
{
  AgsXorgApplicationContext *gobject;
  GList *list;
  xmlNode *child;

  if(*application_context == NULL){
    gobject = g_object_new(AGS_TYPE_XORG_APPLICATION_CONTEXT,
			   NULL);

    *application_context = (GObject *) gobject;
  }else{
    gobject = (AgsApplicationContext *) *application_context;
  }

  file->application_context = gobject;

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
      if(!xmlStrncmp("ags-window\0",
			   child->name,
			   11)){
	ags_file_read_window(file,
			     child,
			     &(gobject->window));
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_xorg_application_context_write(AgsFile *file, xmlNode *parent, GObject *application_context)
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
	     "xorg\0");

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

  ags_file_write_window(file,
			node,
			AGS_XORG_APPLICATION_CONTEXT(application_context)->window);

  return(node);
}

AgsXorgApplicationContext*
ags_xorg_application_context_new()
{
  AgsXorgApplicationContext *xorg_application_context;

  xorg_application_context = (AgsXorgApplicationContext *) g_object_new(AGS_TYPE_XORG_APPLICATION_CONTEXT,
									NULL);

  return(xorg_application_context);
}


