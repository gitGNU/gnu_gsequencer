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

#ifndef __AGS_FUNCTIONAL_MENU_BAR_UTIL_H__
#define __AGS_FUNCTIONAL_MENU_BAR_UTIL_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

gboolean ags_functional_menu_bar_util_activate_path(GtkMenuBar *menu_bar,
						    gchar *path);

gboolean ags_functional_menu_bar_util_activate_menu_path(GtkMenu *menu,
							 gchar *path);

#endif /*__AGS_FUNCTIONAL_MENUBAR_UTIL_H__*/