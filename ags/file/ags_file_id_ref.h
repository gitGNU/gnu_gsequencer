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

#ifndef __AGS_FILE_ID_REF_H__
#define __AGS_FILE_ID_REF_H__

#include <glib.h>
#include <glib-object.h>

#include <libxml/tree.h>

#define AGS_TYPE_FILE_ID_REF                (ags_file_id_ref_get_type())
#define AGS_FILE_ID_REF(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_FILE_ID_REF, AgsFileIdRef))
#define AGS_FILE_ID_REF_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_FILE_ID_REF, AgsFileIdRef))
#define AGS_IS_FILE_ID_REF(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_FILE_ID_REF))
#define AGS_IS_FILE_ID_REF_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_FILE_ID_REF))
#define AGS_FILE_ID_REF_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_FILE_ID_REF, AgsFileIdRefClass))

#define AGS_FILE_RESOLVE(f)          ((AgsFileResolve)(f))

#define AGS_FILE_ID_REF_SERIALIZE_DATA "ags-file-id-ref-serizalize-data\0"
#define AGS_FILE_ID_REF_RESOLVE_DATA "ags-file-id-ref-resolve-data\0"

typedef struct _AgsFileIdRef AgsFileIdRef;
typedef struct _AgsFileIdRefClass AgsFileIdRefClass;

typedef void (*AgsFileResolve)(void);

struct _AgsFileIdRef
{
  GObject object;

  GObject *application_context;
  GObject *file;

  xmlNode *node;
  gchar *xpath;
  gpointer ref;
};

struct _AgsFileIdRefClass
{
  GObjectClass object;

  void (*resolved)(AgsFileIdRef *file_id_ref);
};

GType ags_file_id_ref_get_type();

void ags_file_id_ref_resolved(AgsFileIdRef *file_id_ref);

AgsFileIdRef* ags_file_id_ref_new();

#endif /*__AGS_FILE_ID_REF_H__*/
