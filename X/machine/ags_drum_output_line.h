#ifndef __AGS_DRUM_OUTPUT_LINE_H__
#define __AGS_DRUM_OUTPUT_LINE_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/X/ags_line.h>

#define AGS_TYPE_DRUM_OUTPUT_LINE                (ags_drum_output_line_get_type())
#define AGS_DRUM_OUTPUT_LINE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_DRUM_OUTPUT_LINE, AgsDrumOutputLine))
#define AGS_DRUM_OUTPUT_LINE_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_DRUM_OUTPUT_LINE, AgsDrumOutputLineClass))
#define AGS_IS_DRUM_OUTPUT_LINE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_DRUM_OUTPUT_LINE))
#define AGS_IS_DRUM_OUTPUT_LINE_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_DRUM_OUTPUT_LINE))
#define AGS_DRUM_OUTPUT_LINE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_DRUM_OUTPUT_LINE, AgsDrumOutputLineClass))

typedef struct _AgsDrumOutputLine AgsDrumOutputLine;
typedef struct _AgsDrumOutputLineClass AgsDrumOutputLineClass;

typedef enum{
  AGS_DRUM_OUTPUT_LINE_MAPPED_RECALL  =  1,
}AgsDrumOutputLineFlags;

struct _AgsDrumOutputLine
{
  AgsLine line;

  guint flags;
};

struct _AgsDrumOutputLineClass
{
  AgsLineClass line;
};

GType ags_drum_output_line_get_type();

void ags_drum_output_line_map_recall(AgsDrumOutputLine *drum_output_line);

AgsDrumOutputLine* ags_drum_output_line_new(AgsChannel *channel);

#endif /*__AGS_DRUM_OUTPUT_LINE_H__*/
