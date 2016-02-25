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

#include <ags/plugin/ags_lv2_manager.h>

#include <ags/object/ags_marshal.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void ags_lv2_manager_class_init(AgsLv2ManagerClass *lv2_manager);
void ags_lv2_manager_init (AgsLv2Manager *lv2_manager);
void ags_lv2_manager_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_lv2_manager_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_lv2_manager_finalize(GObject *gobject);

/**
 * SECTION:ags_lv2_manager
 * @short_description: Singleton pattern to organize LV2
 * @title: AgsLv2Manager
 * @section_id:
 * @include: ags/object/ags_lv2_manager.h
 *
 * The #AgsLv2Manager loads/unloads LV2 plugins.
 */

enum{
  PROP_0,
  PROP_LOCALE,
};

static gpointer ags_lv2_manager_parent_class = NULL;

AgsLv2Manager *ags_lv2_manager = NULL;
static const gchar *ags_lv2_default_path = "/usr/lib/lv2\0";

GType
ags_lv2_manager_get_type (void)
{
  static GType ags_type_lv2_manager = 0;

  if(!ags_type_lv2_manager){
    static const GTypeInfo ags_lv2_manager_info = {
      sizeof (AgsLv2ManagerClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_lv2_manager_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsLv2Manager),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_lv2_manager_init,
    };

    ags_type_lv2_manager = g_type_register_static(G_TYPE_OBJECT,
						  "AgsLv2Manager\0",
						  &ags_lv2_manager_info,
						  0);
  }

  return (ags_type_lv2_manager);
}

void
ags_lv2_manager_class_init(AgsLv2ManagerClass *lv2_manager)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;
  
  ags_lv2_manager_parent_class = g_type_class_peek_parent(lv2_manager);

  /* GObjectClass */
  gobject = (GObjectClass *) lv2_manager;

  gobject->set_property = ags_lv2_manager_set_property;
  gobject->get_property = ags_lv2_manager_get_property;

  gobject->finalize = ags_lv2_manager_finalize;

  /* properties */
  /**
   * AgsLv2Manager:locale:
   *
   * The assigned locale.
   * 
   * Since: 0.4.3
   */
  param_spec = g_param_spec_string("locale\0",
				   "locale of lv2 manager\0",
				   "The locale this lv2 manager is assigned to\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LOCALE,
				  param_spec);
}

void
ags_lv2_manager_init(AgsLv2Manager *lv2_manager)
{
  lv2_manager->lv2_plugin = NULL;
}

void
ags_lv2_manager_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsLv2Manager *lv2_manager;

  lv2_manager = AGS_LV2_MANAGER(gobject);

  switch(prop_id){
  case PROP_LOCALE:
    {
      gchar *locale;

      locale = (gchar *) g_value_get_string(value);

      if(lv2_manager->locale == locale){
	return;
      }
      
      if(lv2_manager->locale != NULL){
	g_free(lv2_manager->locale);
      }

      lv2_manager->locale = g_strdup(locale);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_lv2_manager_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  AgsLv2Manager *lv2_manager;

  lv2_manager = AGS_LV2_MANAGER(gobject);

  switch(prop_id){
  case PROP_LOCALE:
    g_value_set_string(value, lv2_manager->locale);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_lv2_manager_finalize(GObject *gobject)
{
  AgsLv2Manager *lv2_manager;
  GList *lv2_plugin;

  lv2_manager = AGS_LV2_MANAGER(gobject);

  lv2_plugin = lv2_manager->lv2_plugin;

  g_list_free_full(lv2_plugin,
		   g_object_unref);
}

/**
 * ags_lv2_manager_get_filenames:
 * 
 * Retrieve all filenames
 *
 * Returns: a %NULL-terminated array of filenames
 *
 * Since: 0.4.3
 */
gchar**
ags_lv2_manager_get_filenames()
{
  AgsLv2Manager *lv2_manager;
  GList *lv2_plugin;
  gchar **filenames;
  guint i;

  lv2_manager = ags_lv2_manager_get_instance();

  lv2_plugin = lv2_manager->lv2_plugin;
  filenames = NULL;
  
  for(i = 0; lv2_plugin != NULL; i++){
    if(filenames == NULL){
      filenames = (gchar **) malloc(2 * sizeof(gchar *));
      filenames[i] = AGS_BASE_PLUGIN(lv2_plugin->data)->filename;
      filenames[i + 1] = NULL;
    }else{
      if(!g_strv_contains(filenames,
			  AGS_BASE_PLUGIN(lv2_plugin->data)->filename)){
	filenames = (gchar **) realloc(filenames,
				       (i + 2) * sizeof(gchar *));
	filenames[i] = AGS_BASE_PLUGIN(lv2_plugin->data)->filename;
	filenames[i + 1] = NULL;
      }
    }
    
    lv2_plugin = lv2_plugin->next;
  }

  return(filenames);
}

/**
 * ags_lv2_manager_find_lv2_plugin:
 * @filename: the filename of the plugin
 * @effect: the effect's name
 *
 * Lookup filename in loaded plugins.
 *
 * Returns: the #AgsLv2Plugin-struct
 *
 * Since: 0.4.3
 */
AgsLv2Plugin*
ags_lv2_manager_find_lv2_plugin(gchar *filename, gchar *effect)
{
  AgsLv2Manager *lv2_manager;
  AgsLv2Plugin *lv2_plugin;
  GList *list;

  lv2_manager = ags_lv2_manager_get_instance();

  list = lv2_manager->lv2_plugin;

  while(list != NULL){
    lv2_plugin = AGS_LV2_PLUGIN(list->data);
    if(!g_strcmp0(AGS_BASE_PLUGIN(lv2_plugin)->filename,
		  filename) &&
       !g_strcmp0(AGS_BASE_PLUGIN(lv2_plugin)->effect,
		  effect)){
      return(lv2_plugin);
    }

    list = list->next;
  }

  return(NULL);
}

/**
 * ags_lv2_manager_load_file:
 * @filename: the filename of the plugin
 *
 * Load @filename specified plugin.
 *
 * Since: 0.4.3
 */
void
ags_lv2_manager_load_file(AgsTurtle *turtle,
			  gchar *filename)
{
  AgsLv2Manager *lv2_manager;
  AgsLv2Plugin *lv2_plugin;

  xmlNode *node;
  
  GList *effect_list;
  GList *uri_list;
  
  gchar *str;
  gchar *path;
  gchar *xpath;
  gchar *effect;
  gchar *uri;
  
  guint effect_index;
  
  GError *error;

  void *plugin_so;
  LV2_Descriptor_Function lv2_descriptor;
  LV2_Descriptor *plugin_descriptor;

  uint32_t i;

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  if(turtle == NULL ||
     filename == NULL){
    return;
  }

  lv2_manager = ags_lv2_manager_get_instance();
  
  /* load plugin */
  pthread_mutex_lock(&(mutex));

  path = g_strdup_printf("%s/%s\0",
			 ags_lv2_default_path,
			 filename);
  g_message("ags_lv2_manager.c loading - %s\0", path);

  xpath = "//rdf-triple//rdf-verb//rdf-pname-ln[substring(text(), string-length(text()) - string-length('doap:name') + 1) = 'doap:name']/ancestor::*[self::rdf-verb][1]/following-sibling::rdf-object-list[1]//rdf-string[text()]\0";
  effect_list = ags_turtle_find_xpath(turtle,
				      xpath);

  while(effect_list != NULL){
    /* read effect name */
    node = effect_list->data;
    
    if(node == NULL){
      effect_list = effect_list->next;
	  
      continue;
    }
	  
    str = xmlNodeGetContent(node);

    if(strlen(str) < 2){
      effect_list = effect_list->next;
	  
      continue;
    }

    effect = g_strndup(str + 1,
		       strlen(str) - 2);

    /* find URI */
    xpath = "//rdf-triple//rdf-subject[ancestor::*[self::rdf-triple][1]//rdf-verb[@verb='a']/following-sibling::*//rdf-pname-ln[substring(text(), string-length(text()) - string-length(':Plugin') + 1) = ':Plugin'] and ancestor::*[self::rdf-triple][1]//rdf-verb[//rdf-pname-ln[substring(text(), string-length(text()) - string-length(':name') + 1) = ':name']]/following-sibling::*//rdf-string[text()='\"%s\"']]/rdf-iri";
    xpath = g_strdup_printf(xpath,
			    effect);
    uri_list = ags_turtle_find_xpath(turtle,
				     xpath);
    free(xpath);
    
    uri = NULL;
  
    if(uri_list != NULL){
      xmlNode *child;

      child = ((xmlNode *) uri_list->data)->children;

      while(child != NULL){
	if(child->type == XML_ELEMENT_NODE){
	  if(!g_ascii_strncasecmp(child->name,
				  "rdf-iriref\0",
				  11)){
	    uri = xmlNodeGetContent(child);

	    if(strlen(uri) > 2){
	      uri = g_strndup(uri + 1,
			      strlen(uri) - 2);
	    }

	    break;
	  }else if(!g_ascii_strncasecmp(child->name,
					"rdf-prefixed-name\0",
					18)){
	    xmlNode *pname_node;

	    gchar *pname;
	  
	    pname_node = child->children;
	    pname = NULL;
	  
	    while(pname_node != NULL){
	      if(pname_node->type == XML_ELEMENT_NODE){
		if(!g_ascii_strncasecmp(pname_node->name,
					"rdf-pname-ln\0",
					11)){
		  pname = xmlNodeGetContent(pname_node);
		
		  break;
		}
	      }

	      pname_node = pname_node->next;
	    }

	    if(pname != NULL){
	      gchar *suffix, *prefix;
	      gchar *offset;

	      offset = index(pname, ':');

	      if(offset != NULL){
		GList *prefix_node;
	      
		offset++;
		suffix = g_strndup(offset,
				   strlen(pname) - (offset - pname));
		prefix = g_strndup(pname,
				   offset - pname);

		str = g_strdup_printf("//rdf-pname-ns[text()='%s']/following-sibling::rdf-iriref",
				      prefix);
		prefix_node = ags_turtle_find_xpath(lv2_plugin->turtle,
						    str);
		free(str);

		if(prefix_node != NULL){
		  gchar *iriref;

		  iriref = xmlNodeGetContent(prefix_node->data);

		  if(iriref != NULL){
		    if(strlen(iriref) > 2){
		      gchar *tmp;
		    
		      tmp = g_strndup(iriref + 1,
				      strlen(iriref) - 2);
		      uri = g_strdup_printf("%s/%s\0",
					    tmp,
					    suffix);

		      free(tmp);
		    }
		  
		    free(iriref);
		  }
		}
	      }
	    }
	  
	    break;
	  }
	}

	child = child->next;
      }
    }

    /* get uri index and append plugin */
    plugin_so = dlopen(path,
		       RTLD_NOW);
  
    if(plugin_so){
      g_warning("ags_lv2_manager.c - failed to load static object file\0");
    
      dlerror();
    }

    lv2_descriptor = (LV2_Descriptor_Function) dlsym(plugin_so,
						     "lv2_descriptor\0");
  
    if(dlerror() == NULL && lv2_descriptor){
      for(i = 0; (plugin_descriptor = lv2_descriptor(i)) != NULL; i++){
	if(!strncmp(plugin_descriptor->URI,
		    uri,
		    strlen(uri))){
	  lv2_plugin = g_object_new(AGS_TYPE_LV2_PLUGIN,
				    "turtle\0", turtle,
				    "filename\0", path,
				    "effect\0", effect,
				    "uri\0", uri,
				    "effect-index\0", i,
				    NULL);
	  lv2_manager->lv2_plugin = g_list_prepend(lv2_manager->lv2_plugin,
						   lv2_plugin);

	  break;
	}
      }  
    }
    
    effect_list = effect_list->next;
  }

  pthread_mutex_unlock(&(mutex));
}

/**
 * ags_lv2_manager_load_default_directory:
 * 
 * Loads all available plugins.
 *
 * Since: 0.4.3
 */
void
ags_lv2_manager_load_default_directory()
{
  AgsLv2Manager *lv2_manager;

  GDir *dir;

  gchar *path, *plugin_path;
  gchar *str;

  GError *error;

  lv2_manager = ags_lv2_manager_get_instance();

  error = NULL;
  dir = g_dir_open(ags_lv2_default_path,
		   0,
		   &error);

  if(error != NULL){
    g_warning(error->message);
  }

  while((path = g_dir_read_name(dir)) != NULL){
    if(!g_ascii_strncasecmp(path,
			    "..\0",
			    3) ||
       !g_ascii_strncasecmp(path,
			    ".\0",
			    2)){
      continue;
    }
    
    plugin_path = g_strdup_printf("%s/%s\0",
				  ags_lv2_default_path,
				  path);

    if(g_file_test(plugin_path,
		   G_FILE_TEST_IS_DIR)){
      AgsTurtle *manifest, *turtle;

      xmlDoc *doc;

      //      FILE *out;

      //      xmlChar *buffer;
      //      int size;
      
      GList *ttl_list, *binary_list;

      gchar *turtle_path, *filename;
      
      manifest = ags_turtle_new(g_strdup_printf("%s/manifest.ttl\0",
						plugin_path));
      ags_turtle_load(manifest,
		      NULL);

      /* read binary from turtle */
      binary_list = ags_turtle_find_xpath(manifest,
					  "//rdf-triple//rdf-verb[//rdf-pname-ln[substring(text(), string-length(text()) - string-length(':binary') + 1) = ':binary']]/following-sibling::rdf-object-list//rdf-iriref[substring(text(), string-length(text()) - string-length('.so>') + 1) = '.so>']\0");

      
      /* read turtle from manifest */
      ttl_list = ags_turtle_find_xpath(manifest,
				       "//rdf-triple//rdf-verb[//rdf-pname-ln[substring(text(), string-length(text()) - string-length(':binary') + 1) = ':binary']]/following-sibling::rdf-object-list//rdf-iriref[substring(text(), string-length(text()) - string-length('.ttl>') + 1) = '.ttl>']\0");

      /* persist XML */
      //NOTE:JK: no need for it
      /*
      xmlDocDumpFormatMemoryEnc(manifest->doc, &buffer, &size, "UTF-8\0", TRUE);

      out = fopen(g_strdup_printf("%s/manifest.xml\0", plugin_path), "w+\0");

      fwrite(buffer, size, sizeof(xmlChar), out);
      fflush(out);
      */
      
      /* load */
      while(ttl_list != NULL &&
	    binary_list != NULL){
	/* read filename */
	turtle_path = xmlNodeGetContent((xmlNode *) ttl_list->data);

	if(turtle_path == NULL){
	  ttl_list = ttl_list->next;
	  binary_list = binary_list->next;
	  continue;
	}
	
	turtle_path = g_strndup(&(turtle_path[1]),
				strlen(turtle_path) - 2);
	
	if(!g_ascii_strncasecmp(turtle_path,
				"http://\0",
				7)){
	  ttl_list = ttl_list->next;
	  binary_list = binary_list->next;
	  continue;
	}

	/* load turtle doc */
	g_message(turtle_path);

	turtle = ags_turtle_new(g_strdup_printf("%s/%s\0",
						plugin_path,
						turtle_path));
	ags_turtle_load(turtle,
			NULL);

	/* read filename of binary */
	str = xmlNodeGetContent((xmlNode *) binary_list->data);

	if(str == NULL){
	  ttl_list = ttl_list->next;
	  binary_list = binary_list->next;
	  continue;
	}
	
	str = g_strndup(&(str[1]),
			strlen(str) - 2);
	filename = g_strdup_printf("%s/%s\0",
				   path,
				   str);
	free(str);
	
	/* load specified plugin */
	ags_lv2_manager_load_file(turtle,
				  filename);

	/* persist XML */
	//NOTE:JK: no need for it
	//	xmlDocDumpFormatMemoryEnc(turtle->doc, &buffer, &size, "UTF-8\0", TRUE);

	//	out = fopen(g_strdup_printf("%s/%s.xml\0", plugin_path, turtle_path), "w+\0");
	
	//	fwrite(buffer, size, sizeof(xmlChar), out);
	//	fflush(out);
	//	xmlSaveFormatFileEnc("-\0", turtle->doc, "UTF-8\0", 1);

	
	ttl_list = ttl_list->next;
	binary_list = binary_list->next;
      }

      g_object_unref(manifest);
    }
  }
}
  
/**
 * ags_lv2_manager_uri_index:
 * @filename: the plugin.so filename
 * @uri: the uri's name within plugin
 *
 * Retrieve the uri's index within @filename
 *
 * Returns: the index, G_MAXULONG if not found
 *
 * Since: 0.4.3
 */
uint32_t
ags_lv2_manager_uri_index(gchar *filename,
			  gchar *uri)
{
  AgsLv2Plugin *lv2_plugin;

  gchar *effect; //FIXME:JK: 
  
  void *plugin_so;
  LV2_Descriptor_Function lv2_descriptor;
  LV2_Descriptor *plugin_descriptor;

  uint32_t uri_index;
  uint32_t i;

  if(filename == NULL ||
     uri == NULL){
    return(G_MAXULONG);
  }
  
  /* load plugin */
  effect = NULL;
  lv2_plugin = ags_lv2_manager_find_lv2_plugin(filename, effect);

  plugin_so = AGS_BASE_PLUGIN(lv2_plugin)->plugin_so;
  
  uri_index = G_MAXULONG;

  if(plugin_so){
    lv2_descriptor = (LV2_Descriptor_Function) dlsym(plugin_so,
						     "lv2_descriptor\0");
    
    if(dlerror() == NULL && lv2_descriptor){
      for(i = 0; (plugin_descriptor = lv2_descriptor(i)) != NULL; i++){
	if(!strncmp(plugin_descriptor->URI,
		    uri,
		    strlen(uri))){
	  uri_index = i;
	  break;
	}
      }
    }
  }
  
  return(uri_index);
}

/**
 * ags_lv2_manager_find_uri:
 * @filename: the plugin.so filename
 * @effect: the uri's effect
 *
 * Retrieve the uri of @filename and @effect
 *
 * Returns: the appropriate uri
 *
 * Since: 0.4.3
 */
gchar*
ags_lv2_manager_find_uri(gchar *filename,
			 gchar *effect)
{
  AgsLv2Plugin *lv2_plugin;
  
  GList *uri_node;

  gchar *uri;
  gchar *str;
  
  /* load plugin */
  lv2_plugin = ags_lv2_manager_find_lv2_plugin(filename, effect);

  if(lv2_plugin == NULL){
    return(NULL);
  }
  
  str = g_strdup_printf("//rdf-triple//rdf-subject[ancestor::*[self::rdf-triple][1]//rdf-verb[@verb='a']/following-sibling::*//rdf-pname-ln[substring(text(), string-length(text()) - string-length(':Plugin') + 1) = ':Plugin'] and ancestor::*[self::rdf-triple][1]//rdf-verb[//rdf-pname-ln[substring(text(), string-length(text()) - string-length(':name') + 1) = ':name']]/following-sibling::*//rdf-string[text()='%s']]/rdf-iri",
			effect);
  uri_node = ags_turtle_find_xpath(lv2_plugin->turtle,
				   str);
  free(str);
  
  uri = NULL;
  
  if(uri_node != NULL){
    xmlNode *child;

    child = ((xmlNode *) uri_node->data)->children;

    while(child != NULL){
      if(child->type == XML_ELEMENT_NODE){
	if(!g_ascii_strncasecmp(child->name,
				"rdf-iriref\0",
				11)){
	  uri = xmlNodeGetContent(child);

	  if(strlen(uri) > 2){
	    uri = g_strndup(uri + 1,
			    strlen(uri) - 2);
	  }

	  break;
	}else if(!g_ascii_strncasecmp(child->name,
				      "rdf-prefixed-name\0",
				      18)){
	  xmlNode *pname_node;

	  gchar *pname;
	  
	  pname_node = child->children;
	  pname = NULL;
	  
	  while(pname_node != NULL){
	    if(pname_node->type == XML_ELEMENT_NODE){
	      if(!g_ascii_strncasecmp(pname_node->name,
				      "rdf-pname-ln\0",
				      11)){
		pname = xmlNodeGetContent(pname_node);
		
		break;
	      }
	    }

	    pname_node = pname_node->next;
	  }

	  if(pname != NULL){
	    gchar *suffix, *prefix;
	    gchar *offset;

	    offset = index(pname, ':');

	    if(offset != NULL){
	      GList *prefix_node;
	      
	      offset++;
	      suffix = g_strndup(offset,
				 strlen(pname) - (offset - pname));
	      prefix = g_strndup(pname,
				 offset - pname);

	      str = g_strdup_printf("//rdf-pname-ns[text()='%s']/following-sibling::rdf-iriref",
				    prefix);
	      prefix_node = ags_turtle_find_xpath(lv2_plugin->turtle,
						  str);
	      free(str);

	      if(prefix_node != NULL){
		gchar *iriref;

		iriref = xmlNodeGetContent(prefix_node->data);

		if(iriref != NULL){
		  if(strlen(iriref) > 2){
		    gchar *tmp;
		    
		    tmp = g_strndup(iriref + 1,
				    strlen(iriref) - 2);
		    uri = g_strdup_printf("%s/%s\0",
					  tmp,
					  suffix);

		    free(tmp);
		  }
		  
		  free(iriref);
		}
	      }
	    }
	  }
	  
	  break;
	}
      }

      child = child->next;
    }
  }

  return(uri);
}

/**
 * ags_lv2_manager_get_instance:
 *
 * Get instance.
 *
 * Returns: the #AgsLv2Manager
 *
 * Since: 0.4.3
 */
AgsLv2Manager*
ags_lv2_manager_get_instance()
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&(mutex));

  if(ags_lv2_manager == NULL){
    ags_lv2_manager = ags_lv2_manager_new(AGS_LV2_MANAGER_DEFAULT_LOCALE);

    pthread_mutex_unlock(&(mutex));

    ags_lv2_manager_load_default_directory();
  }else{
    pthread_mutex_unlock(&(mutex));
  }

  return(ags_lv2_manager);
}

/**
 * ags_lv2_manager_new:
 * @locale: the default locale
 *
 * Creates an #AgsLv2Manager
 *
 * Returns: a new #AgsLv2Manager
 *
 * Since: 0.4.3
 */
AgsLv2Manager*
ags_lv2_manager_new(gchar *locale)
{
  AgsLv2Manager *lv2_manager;

  lv2_manager = (AgsLv2Manager *) g_object_new(AGS_TYPE_LV2_MANAGER,
					       "locale\0", locale,
					       NULL);

  return(lv2_manager);
}
