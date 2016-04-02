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

#ifndef __AGS_LINE_EDITOR_H__
#define __AGS_LINE_EDITOR_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/audio/ags_channel.h>

#include <ags/X/ags_link_editor.h>
#include <ags/X/ags_line_member_editor.h>

#define AGS_TYPE_LINE_EDITOR                (ags_line_editor_get_type())
#define AGS_LINE_EDITOR(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_LINE_EDITOR, AgsLineEditor))
#define AGS_LINE_EDITOR_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_LINE_EDITOR, AgsLineEditorClass))
#define AGS_IS_LINE_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_LINE_EDITOR))
#define AGS_IS_LINE_EDITOR_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_LINE_EDITOR))
#define AGS_LINE_EDITOR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_LINE_EDITOR, AgsLineEditorClass))

#define AGS_LINE_EDITOR_DEFAULT_VERSION "0.4.2\0"
#define AGS_LINE_EDITOR_DEFAULT_BUILD_ID "CEST 02-10-2014 19:36\0"

typedef struct _AgsLineEditor AgsLineEditor;
typedef struct _AgsLineEditorClass AgsLineEditorClass;

struct _AgsLineEditor
{
  GtkVBox vbox;

  gchar *version;
  gchar *build_id;

  AgsChannel *channel;

  AgsLinkEditor *link_editor;
  AgsLineMemberEditor *member_editor;
};

struct _AgsLineEditorClass
{
  GtkVBoxClass vbox;
};

GType ags_line_editor_get_type(void);

void ags_line_editor_set_channel(AgsLineEditor *line_editor,
				 AgsChannel *channel);

AgsLineEditor* ags_line_editor_new(AgsChannel *channel);

#endif /*__AGS_LINE_EDITOR_H__*/