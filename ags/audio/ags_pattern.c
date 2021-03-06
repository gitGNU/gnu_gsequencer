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

#include <ags/audio/ags_pattern.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_tactable.h>
#include <ags/object/ags_portlet.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_port.h>

#include <stdarg.h>
#include <math.h>
#include <string.h>

void ags_pattern_class_init(AgsPatternClass *pattern_class);
void ags_pattern_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_pattern_tactable_interface_init(AgsTactableInterface *tactable);
void ags_pattern_portlet_interface_init(AgsPortletInterface *portlet);
void ags_pattern_init(AgsPattern *pattern);
void ags_pattern_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec);
void ags_pattern_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec);
void ags_pattern_connect(AgsConnectable *connectable);
void ags_pattern_disconnect(AgsConnectable *connectable);
void ags_pattern_dispose(GObject *gobject);
void ags_pattern_finalize(GObject *gobject);

void ags_pattern_change_bpm(AgsTactable *tactable, gdouble new_bpm, gdouble old_bpm);

void ags_pattern_set_port(AgsPortlet *portlet, GObject *port);
GObject* ags_pattern_get_port(AgsPortlet *portlet);
GList* ags_pattern_list_safe_properties(AgsPortlet *portlet);
void ags_pattern_safe_set_property(AgsPortlet *portlet, gchar *property_name, GValue *value);
void ags_pattern_safe_get_property(AgsPortlet *portlet, gchar *property_name, GValue *value);

/**
 * SECTION:ags_pattern
 * @short_description: Pattern representing tones
 * @title: AgsPattern
 * @section_id:
 * @include: ags/audio/ags_pattern.h
 *
 * #AgsPattern represents an audio pattern of tones.
 */

enum{
  PROP_0,
  PROP_PORT,
  PROP_FIRST_INDEX,
  PROP_SECOND_INDEX,
  PROP_OFFSET,
  PROP_CURRENT_BIT,
  PROP_TIMESTAMP,
};

static gpointer ags_pattern_parent_class = NULL;

GType
ags_pattern_get_type (void)
{
  static GType ags_type_pattern = 0;

  if(!ags_type_pattern){
    static const GTypeInfo ags_pattern_info = {
      sizeof (AgsPatternClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_pattern_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPattern),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_pattern_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_pattern_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_tactable_interface_info = {
      (GInterfaceInitFunc) ags_pattern_tactable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_portlet_interface_info = {
      (GInterfaceInitFunc) ags_pattern_portlet_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_pattern = g_type_register_static(G_TYPE_OBJECT,
					      "AgsPattern\0",
					      &ags_pattern_info,
					      0);

    g_type_add_interface_static(ags_type_pattern,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_pattern,
				AGS_TYPE_TACTABLE,
				&ags_tactable_interface_info);

    g_type_add_interface_static(ags_type_pattern,
				AGS_TYPE_PORTLET,
				&ags_portlet_interface_info);
  }

  return (ags_type_pattern);
}

void
ags_pattern_class_init(AgsPatternClass *pattern)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_pattern_parent_class = g_type_class_peek_parent(pattern);

  gobject = (GObjectClass *) pattern;

  gobject->set_property = ags_pattern_set_property;
  gobject->get_property = ags_pattern_get_property;

  gobject->dispose = ags_pattern_dispose;
  gobject->finalize = ags_pattern_finalize;

  /* properties */
  /**
   * AgsPattern:port:
   *
   * The pattern's port.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("port\0",
				   "port of pattern\0",
				   "The port of pattern\0",
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PORT,
				  param_spec);

  /**
   * AgsPattern:first-index:
   *
   * Selected bank 0.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_uint("first-index\0",
				 "the first index\0",
				 "The first index to select pattern\0",
				 0, 256,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FIRST_INDEX,
				  param_spec);

  /**
   * AgsPattern:second-index:
   *
   * Selected bank 1.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_uint("second-index\0",
				 "the second index\0",
				 "The second index to select pattern\0",
				 0, 256,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SECOND_INDEX,
				  param_spec);

  /**
   * AgsPattern:offset:
   *
   * Position of pattern.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_uint("offset\0",
				 "the offset\0",
				 "The offset within the pattern\0",
				 0, 65535,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_OFFSET,
				  param_spec);

  /**
   * AgsPattern:current-bit:
   *
   * Offset of current position.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_boolean("current-bit\0",
				    "current bit for offset\0",
				    "The current bit for offset\0",
				    FALSE,
				    G_PARAM_READABLE);
  g_object_class_install_property(gobject,
				  PROP_CURRENT_BIT,
				  param_spec);

  /**
   * AgsPattern:timestamp:
   *
   * The pattern's timestamp.
   * 
   * Since: 0.7.12
   */
  param_spec = g_param_spec_object("timestamp\0",
				   "timestamp of pattern\0",
				   "The timestamp of pattern\0",
				   AGS_TYPE_TIMESTAMP,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_TIMESTAMP,
				  param_spec);
}

void
ags_pattern_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_pattern_connect;
  connectable->disconnect = ags_pattern_disconnect;
}

void
ags_pattern_tactable_interface_init(AgsTactableInterface *tactable)
{
  tactable->change_bpm = ags_pattern_change_bpm;
}

void
ags_pattern_portlet_interface_init(AgsPortletInterface *portlet)
{
  portlet->set_port = ags_pattern_set_port;
  portlet->get_port = ags_pattern_get_port;
  portlet->list_safe_properties = ags_pattern_list_safe_properties;
  portlet->safe_set_property = ags_pattern_safe_set_property;
  portlet->safe_get_property = ags_pattern_safe_get_property;
}

void
ags_pattern_init(AgsPattern *pattern)
{
  AgsMutexManager *mutex_manager;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *mutex;
  pthread_mutexattr_t attr;

  /* create mutex */
  //FIXME:JK: memory leak
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr,
			    PTHREAD_MUTEX_RECURSIVE);

  mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex,
		     &attr);

  /* insert mutex */
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  pthread_mutex_lock(application_mutex);

  ags_mutex_manager_insert(mutex_manager,
			   (GObject *) pattern,
			   mutex);
  
  pthread_mutex_unlock(application_mutex);

  /* base initialization */
  pattern->flags = 0;

  /* timestamp */
  pattern->timestamp = NULL;

  /* dimension and pattern */
  pattern->dim[0] = 0;
  pattern->dim[1] = 0;
  pattern->dim[2] = 0;

  pattern->pattern = NULL;

  /* port */
  pattern->port = NULL;

  /* indices */
  pattern->i = 0;
  pattern->j = 0;
  pattern->bit = 0;
}

void
ags_pattern_set_property(GObject *gobject,
			 guint prop_id,
			 const GValue *value,
			 GParamSpec *param_spec)
{
  AgsPattern *pattern;

  pattern = AGS_PATTERN(gobject);

  switch(prop_id){
  case PROP_PORT:
    {
      AgsPort *port;

      port = (AgsPort *) g_value_get_object(value);

      if(port == (AgsPort *) pattern->port){
	return;
      }

      if(pattern->port != NULL){
	g_object_unref(G_OBJECT(pattern->port));
      }

      if(port != NULL){
	g_object_ref(G_OBJECT(port));
      }

      pattern->port = (GObject *) port;
    }
    break;
  case PROP_FIRST_INDEX:
    {
      AgsPort *port;
      guint i;

      i = g_value_get_uint(value);

      port = (AgsPort *) ags_portlet_get_port(AGS_PORTLET(pattern));

      pthread_mutex_lock(port->mutex);

      pattern->i = i;

      pthread_mutex_unlock(port->mutex);
    }
    break;
  case PROP_SECOND_INDEX:
    {
      AgsPort *port;
      guint j;

      j = g_value_get_uint(value);

      port = (AgsPort *) ags_portlet_get_port(AGS_PORTLET(pattern));

      pthread_mutex_lock(port->mutex);

      pattern->j = j;

      pthread_mutex_unlock(port->mutex);
    }
    break;
  case PROP_OFFSET:
    {
      AgsPort *port;
      guint bit;

      bit = g_value_get_uint(value);

      port = (AgsPort *) ags_portlet_get_port(AGS_PORTLET(pattern));

      pthread_mutex_lock(port->mutex);

      pattern->bit = bit;

      pthread_mutex_unlock(port->mutex);
    }
    break;
  case PROP_TIMESTAMP:
    {
      AgsTimestamp *timestamp;

      timestamp = (AgsTimestamp *) g_value_get_object(value);

      if(timestamp == (AgsTimestamp *) pattern->timestamp){
	return;
      }

      if(pattern->timestamp != NULL){
	g_object_unref(G_OBJECT(pattern->timestamp));
      }

      if(timestamp != NULL){
	g_object_ref(G_OBJECT(timestamp));
      }

      pattern->timestamp = (GObject *) timestamp;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_pattern_get_property(GObject *gobject,
			 guint prop_id,
			 GValue *value,
			 GParamSpec *param_spec)
{
  AgsPattern *pattern;

  pattern = AGS_PATTERN(gobject);

  switch(prop_id){
  case PROP_PORT:
    g_value_set_object(value, pattern->port);
    break;
  case PROP_FIRST_INDEX:
    {
      AgsPort *port;

      port = (AgsPort *) ags_portlet_get_port(AGS_PORTLET(pattern));

      pthread_mutex_lock(port->mutex);

      g_value_set_uint(value, pattern->i);

      pthread_mutex_unlock(port->mutex);
    }
    break;
  case PROP_SECOND_INDEX:
    {
      AgsPort *port;

      port = (AgsPort *) ags_portlet_get_port(AGS_PORTLET(pattern));

      pthread_mutex_lock(port->mutex);

      g_value_set_uint(value, pattern->j);

      pthread_mutex_unlock(port->mutex);
    }
    break;
  case PROP_OFFSET:
    {
      AgsPort *port;

      port = (AgsPort *) ags_portlet_get_port(AGS_PORTLET(pattern));

      pthread_mutex_lock(port->mutex);

      g_value_set_uint(value, pattern->bit);

      pthread_mutex_unlock(port->mutex);
    }
    break;
  case PROP_CURRENT_BIT:
    {
      AgsPort *port;

      port = (AgsPort *) ags_portlet_get_port(AGS_PORTLET(pattern));

      pthread_mutex_lock(port->mutex);

      g_value_set_boolean(value, ags_pattern_get_bit(pattern,
						     pattern->i,
						     pattern->j,
						     pattern->bit));

      pthread_mutex_unlock(port->mutex);
    }
    break;
  case PROP_TIMESTAMP:
    g_value_set_object(value, pattern->timestamp);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_pattern_connect(AgsConnectable *connectable)
{
  AgsPattern *pattern;

  pattern = AGS_PATTERN(connectable);

  if((AGS_PATTERN_CONNECTED & (pattern->flags)) != 0){
    return;
  }

  pattern->flags |= AGS_PATTERN_CONNECTED;
}

void
ags_pattern_disconnect(AgsConnectable *connectable)
{
  AgsPattern *pattern;

  pattern = AGS_PATTERN(connectable);

  if((AGS_PATTERN_CONNECTED & (pattern->flags)) == 0){
    return;
  }

  pattern->flags &= (~AGS_PATTERN_CONNECTED);
}

void
ags_pattern_dispose(GObject *gobject)
{
  AgsPattern *pattern;

  pattern = AGS_PATTERN(gobject);

  /* timestamp */
  if(pattern->timestamp != NULL){
    g_object_run_dispose(G_OBJECT(pattern->timestamp));
    
    g_object_unref(G_OBJECT(pattern->timestamp));
  }

  /* port */
  if(pattern->port != NULL){
    g_object_unref(G_OBJECT(pattern->port));

    pattern->port = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_pattern_parent_class)->dispose(gobject);
}

void
ags_pattern_finalize(GObject *gobject)
{
  AgsPattern *pattern;
  
  guint i, j;

  pattern = AGS_PATTERN(gobject);

  /* timestamp */
  if(pattern->timestamp != NULL){
    g_object_unref(G_OBJECT(pattern->timestamp));
  }

  /* pattern */
  if(pattern->pattern != NULL){
    for(i = 0; i < pattern->dim[0]; i++){
      if(pattern->pattern[i] != NULL){
	for(j = 0; j < pattern->dim[1]; j++){
	  if(pattern->pattern[i][j] != NULL){
	    free(pattern->pattern[i][j]);
	  }
	}

	free(pattern->pattern[i]);
      }
    }

    free(pattern->pattern);
  }
 
  /* port */
  if(pattern->port != NULL){
    g_object_unref(G_OBJECT(pattern->port));
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_pattern_parent_class)->finalize(gobject);
}

void
ags_pattern_change_bpm(AgsTactable *tactable, gdouble new_bpm, gdouble old_bpm)
{
  //TODO:JK: implement me
}

void
ags_pattern_set_port(AgsPortlet *portlet, GObject *port)
{
  g_object_set(G_OBJECT(portlet),
	       "port\0", port,
	       NULL);
}

GObject*
ags_pattern_get_port(AgsPortlet *portlet)
{
  AgsPort *port;

  g_object_get(G_OBJECT(portlet),
	       "port\0", &port,
	       NULL);

  return((GObject *) port);
}

GList*
ags_pattern_list_safe_properties(AgsPortlet *portlet)
{
  static GList *list = NULL;

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  
  pthread_mutex_lock(&mutex);

  if(list == NULL){
    list = g_list_prepend(list, "first-index\0");
    list = g_list_prepend(list, "second-index\0");
    list = g_list_prepend(list, "offset\0");
    list = g_list_prepend(list, "current-bit\0");
 }

  pthread_mutex_unlock(&mutex);

  return(list);
}

void
ags_pattern_safe_set_property(AgsPortlet *portlet, gchar *property_name, GValue *value)
{
  //TODO:JK: add check for safe property

  g_object_set_property(G_OBJECT(portlet),
			property_name, value);
}

void
ags_pattern_safe_get_property(AgsPortlet *portlet, gchar *property_name, GValue *value)
{
  //TODO:JK: add check for safe property

  g_object_get_property(G_OBJECT(portlet),
			property_name, value);
}

/**
 * ags_pattern_find_near_timestamp:
 * @pattern: a #GList containing #AgsPattern
 * @timestamp: the matching timestamp
 *
 * Retrieve appropriate pattern for timestamp.
 *
 * Returns: Next match.
 *
 * Since: 0.7.12
 */
GList*
ags_pattern_find_near_timestamp(GList *pattern, GObject *timestamp)
{
  AgsTimestamp *current_timestamp;

  if(timestamp == NULL){
    return(NULL);
  }

  while(pattern != NULL){
    current_timestamp = (AgsTimestamp *) AGS_PATTERN(pattern->data)->timestamp;

    if((AGS_TIMESTAMP_UNIX & (AGS_TIMESTAMP(timestamp)->flags)) != 0){
      if((AGS_TIMESTAMP_UNIX & (current_timestamp->flags)) != 0){
	if(current_timestamp->timer.unix_time.time_val >= AGS_TIMESTAMP(timestamp)->timer.unix_time.time_val &&
	   current_timestamp->timer.unix_time.time_val < AGS_TIMESTAMP(timestamp)->timer.unix_time.time_val + AGS_PATTERN_DEFAULT_DURATION){
	  return(pattern);
	}
      }
    }

    pattern = pattern->next;
  }

  return(NULL);
}

/**
 * ags_pattern_set_dim:
 * @pattern: an #AgsPattern
 * @dim0: bank 0 size
 * @dim1: bank 1 size
 * @length: amount of beats
 *
 * Reallocates the pattern's dimensions.
 *
 * Since: 0.3
 */
void 
ags_pattern_set_dim(AgsPattern *pattern, guint dim0, guint dim1, guint length)
{
  guint ***index0, **index1, *bitmap;
  guint i, j, k, j_set, k_set;
  guint bitmap_size;

  if(dim0 == 0 && pattern->pattern == NULL){
    return;
  }
  
  // shrink
  if(pattern->dim[0] > dim0){
    for(i = dim0; i < pattern->dim[0]; i++){
      for(j = 0; j < pattern->dim[1]; j++){
	free(pattern->pattern[i][j]);
      }

      free(pattern->pattern[i]);
    }

    if(dim0 == 0){
      free(pattern->pattern);
      
      pattern->pattern = NULL;
      pattern->dim[0] = 0;

      return;
    }else{
      pattern->pattern = (guint ***) realloc(pattern->pattern,
					     (int) dim0 * sizeof(guint **));

      pattern->dim[0] = dim0;
    }
  }

  if(pattern->dim[1] > dim1){
    if(dim1 == 0){
      for(i = 0; i < pattern->dim[0]; i++){
	for(j = dim1; j < pattern->dim[1]; j++){
	  free(pattern->pattern[i][j]);
	}

	pattern->pattern[i] = NULL;
      }

      pattern->dim[1] = 0;
      
      return;
    }else{
      for(i = 0; i < pattern->dim[0]; i++){
	for(j = dim1; j < pattern->dim[1]; j++){
	  free(pattern->pattern[i][j]);
	}
      }

      for(i = 0; pattern->dim[0]; i++){
	pattern->pattern[i] = (guint **) realloc(pattern->pattern[i],
						 dim1 * sizeof(guint *));
      }

      pattern->dim[1] = dim1;
    }
  }

  if(pattern->dim[2] > length){
    if(length == 0){
      for(i = 0; i < pattern->dim[0]; i++){
	for(j = 0; j < pattern->dim[1]; j++){
      	  free(pattern->pattern[i][j]);
	  
	  pattern->pattern[i][j] = NULL;
	}
      }

      pattern->dim[2] = 0;
    }else{
      for(i = 0; i < pattern->dim[0]; i++){
	for(j = 0; j < pattern->dim[1]; j++){
	  pattern->pattern[i][j] = (guint *) realloc(pattern->pattern[i][j],
						     (int) ceil((double) length / (double) (sizeof(guint) * 8)) * sizeof(guint));
	}
      }

      pattern->dim[2] = length;
    }
  }

  // grow
  bitmap_size = (int) ceil((double) pattern->dim[2] / (double) (sizeof(guint) * 8)) * sizeof(guint);
    
  if(pattern->dim[0] < dim0){
    if(pattern->pattern == NULL){
      pattern->pattern = (guint ***) malloc(dim0 * sizeof(guint **));
    }else{
      pattern->pattern = (guint ***) realloc(pattern->pattern,
					     dim0 * sizeof(guint **));
    }

    for(i = pattern->dim[0]; i < dim0; i++){
      pattern->pattern[i] = (guint **) malloc(pattern->dim[1] * sizeof(guint *));

      for(j = 0; j < pattern->dim[1]; j++){
	if(bitmap_size == 0){
	  pattern->pattern[i][j] = NULL;
	}else{
	  pattern->pattern[i][j] = (guint *) malloc(bitmap_size);
	  memset(pattern->pattern[i][j], 0, bitmap_size);
	}
      }
    }

    pattern->dim[0] = dim0;
  }

  if(pattern->dim[1] < dim1){  
    for(i = 0; i < pattern->dim[0]; i++){
      if(pattern->pattern[i] == NULL){
	pattern->pattern[i] = (guint **) malloc(dim1 * sizeof(guint *));
      }else{
	pattern->pattern[i] = (guint **) realloc(pattern->pattern[i],
						 dim1 * sizeof(guint *));
      }

      for(j = pattern->dim[1]; j < dim1; j++){
	if(bitmap_size == 0){
	  pattern->pattern[i][j] = NULL;
	}else{
	  pattern->pattern[i][j] = (guint *) malloc(bitmap_size);
	  memset(pattern->pattern[i][j], 0, bitmap_size);
	}
      }
    }

    pattern->dim[1] = dim1;
  }

  if(pattern->dim[2] < length){
    guint new_bitmap_size;
    
    new_bitmap_size = (int) ceil((double) length / (double) (sizeof(guint) * 8)) * sizeof(guint);
    
    for(i = 0; i < pattern->dim[0]; i++){
      for(j = 0; j < pattern->dim[1]; j++){
	if(pattern->pattern[i][j] == NULL){
	  pattern->pattern[i][j] = (guint *) malloc(new_bitmap_size);
	  memset(pattern->pattern[i][j], 0, new_bitmap_size);
	}else{
	  pattern->pattern[i][j] =(guint *) realloc(pattern->pattern[i][j],
						    new_bitmap_size);
	  memset(pattern->pattern[i][j] + bitmap_size, 0, new_bitmap_size - bitmap_size);
	}
      }
    }

    pattern->dim[2] = length;
  }
}

gboolean
ags_pattern_is_empty(AgsPattern *pattern, guint i, guint j)
{
  guint bitmap_length;
  guint n;

  bitmap_length = (guint) ceil((double) pattern->dim[2] / (double) (sizeof(guint) * 8));

  for(n = 0; n < bitmap_length; n++){
    if(pattern->pattern[i][j][n] != 0){
      return(FALSE);
    }
  }

  return(TRUE);
}

/**
 * ags_pattern_get_bit:
 * @pattern: an #AgsPattern
 * @i: bank index 0
 * @j: bank index 1
 * @bit: the tic to check
 *
 * Check for tic to be played.
 *
 * Returns: %TRUE if tone is enabled.
 *
 * Since: 0.3
 */
gboolean
ags_pattern_get_bit(AgsPattern *pattern, guint i, guint j, guint bit)
{
  guint k, value;

  k = (guint) floor((double) bit / (double) (sizeof(guint) * 8));
  value = 1 << (bit % (sizeof(guint) * 8));

  //((1 << (bit % (sizeof(guint) *8))) & (pattern->pattern[i][j][(guint) floor((double) bit / (double) (sizeof(guint) * 8))])) != 0
  if((value & (pattern->pattern[i][j][k])) != 0)
    return(TRUE);
  else
    return(FALSE);
}

/**
 * ags_pattern_toggle_bit:
 * @pattern: an #AgsPattern
 * @i: bank index 0
 * @j: bank index 1
 * @bit: the tic to toggle
 *
 * Toggle tone.
 *
 * Since: 0.3
 */
void
ags_pattern_toggle_bit(AgsPattern *pattern, guint i, guint j, guint bit)
{
  guint k, value;

  k = (guint) floor((double) bit / (double) (sizeof(guint) * 8));
  value = 1 << (bit % (sizeof(guint) * 8));


  if(value & (pattern->pattern[i][j][k]))
    pattern->pattern[i][j][k] &= (~value);
  else
    pattern->pattern[i][j][k] |= value;
}

/**
 * ags_pattern_new:
 *
 * Creates an #AgsPattern
 *
 * Returns: a new #AgsPattern
 *
 * Since: 0.3
 */
AgsPattern*
ags_pattern_new()
{
  AgsPattern *pattern;

  pattern = (AgsPattern *) g_object_new(AGS_TYPE_PATTERN, NULL);

  return(pattern);
}
