#ifndef __AGS_PROPERTY_COLLECTION_EDITOR_H__
#define __AGS_PROPERTY_COLLECTION_EDITOR_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/X/ags_property_editor.h>

#include <stdarg.h>

#define AGS_TYPE_PROPERTY_COLLECTION_EDITOR                (ags_property_collection_editor_get_type())
#define AGS_PROPERTY_COLLECTION_EDITOR(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_PROPERTY_COLLECTION_EDITOR, AgsPropertyCollectionEditor))
#define AGS_PROPERTY_COLLECTION_EDITOR_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_PROPERTY_COLLECTION_EDITOR, AgsPropertyCollectionEditorClass))
#define AGS_IS_PROPERTY_COLLECTION_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_PROPERTY_COLLECTION_EDITOR))
#define AGS_IS_PROPERTY_COLLECTION_EDITOR_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_PROPERTY_COLLECTION_EDITOR))
#define AGS_PROPERTY_COLLECTION_EDITOR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_PROPERTY_COLLECTION_EDITOR, AgsPropertyCollectionEditorClass))

typedef struct _AgsPropertyCollectionEditor AgsPropertyCollectionEditor;
typedef struct _AgsPropertyCollectionEditorClass AgsPropertyCollectionEditorClass;

struct _AgsPropertyCollectionEditor
{
  AgsPropertyEditor property_editor;

  GType child_type;
  guint child_parameter_count;
  GParameter *child_parameter;

  GtkVBox *child;

  GtkButton *add_collection;
};

struct _AgsPropertyCollectionEditorClass
{
  AgsPropertyEditorClass property_editor;
};

GType ags_property_collection_editor_get_type();

AgsPropertyCollectionEditor* ags_property_collection_editor_new(GType child_type,
								guint child_parameter_count,
								GParameter *child_parameter);

#endif /*__AGS_PROPERTY_COLLECTION_EDITOR_H__*/

