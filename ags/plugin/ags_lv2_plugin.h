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

#ifndef __AGS_LV2_PLUGIN_H__
#define __AGS_LV2_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/plugin/ags_base_plugin.h>
#include <ags/lib/ags_turtle.h>

#include <lv2.h>

#define AGS_TYPE_LV2_PLUGIN                (ags_lv2_plugin_get_type())
#define AGS_LV2_PLUGIN(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_LV2_PLUGIN, AgsLv2Plugin))
#define AGS_LV2_PLUGIN_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_LV2_PLUGIN, AgsLv2PluginClass))
#define AGS_IS_LV2_PLUGIN(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_LV2_PLUGIN))
#define AGS_IS_LV2_PLUGIN_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_LV2_PLUGIN))
#define AGS_LV2_PLUGIN_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_LV2_PLUGIN, AgsLv2PluginClass))

#define AGS_LV2_PLUGIN_DESCRIPTOR(ptr) ((LV2_Descriptor *)(ptr))

typedef struct _AgsLv2Plugin AgsLv2Plugin;
typedef struct _AgsLv2PluginClass AgsLv2PluginClass;

typedef enum{
  AGS_LV2_PLUGIN_IS_SYNTHESIZER  = 1,
};

struct _AgsLv2Plugin
{
  AgsBasePlugin base_plugin;

  guint flags;
  
  gchar *uri;

  AgsTurtle *turtle;
};

struct _AgsLv2PluginClass
{
  AgsBasePluginClass base_plugin;
};

GType ags_lv2_plugin_get_type(void);

gchar* ags_lv2_plugin_find_uri(AgsLv2Plugin *lv2_plugin,
			       gchar *effect);

AgsLv2Plugin* ags_lv2_plugin_new(AgsTurtle *turtle, gchar *filename, gchar *effect, gchar *uri, guint effect_index);

#endif /*__AGS_LV2_PLUGIN_H__*/
