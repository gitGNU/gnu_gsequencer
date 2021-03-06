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

#include <ags/X/task/ags_scroll_on_play.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_application_context.h>

#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_audio_run.h>

#include <ags/audio/recall/ags_delay_audio.h>
#include <ags/audio/recall/ags_delay_audio_run.h>
#include <ags/audio/recall/ags_count_beats_audio.h>
#include <ags/audio/recall/ags_count_beats_audio_run.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_editor.h>
#include <ags/X/ags_automation_editor.h>
#include <ags/X/editor/ags_pattern_edit.h>
#include <ags/X/editor/ags_note_edit.h>
#include <ags/X/editor/ags_automation_edit.h>

#include <fontconfig/fontconfig.h>

#include <math.h>

void ags_scroll_on_play_class_init(AgsScrollOnPlayClass *scroll_on_play);
void ags_scroll_on_play_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_scroll_on_play_init(AgsScrollOnPlay *scroll_on_play);
void ags_scroll_on_play_connect(AgsConnectable *connectable);
void ags_scroll_on_play_disconnect(AgsConnectable *connectable);
void ags_scroll_on_play_finalize(GObject *gobject);

void ags_scroll_on_play_launch(AgsTask *task);

/**
 * SECTION:ags_scroll_on_play
 * @short_description: scrolls the editor
 * @title: AgsScrollOnPlay
 * @section_id:
 * @include: ags/audio/task/ags_scroll_on_play.h
 *
 * The #AgsScrollOnPlay task scrolls the specified #AgsEditor.
 */

static gpointer ags_scroll_on_play_parent_class = NULL;
static AgsConnectableInterface *ags_scroll_on_play_parent_connectable_interface;

GType
ags_scroll_on_play_get_type()
{
  static GType ags_type_scroll_on_play = 0;

  if(!ags_type_scroll_on_play){
    static const GTypeInfo ags_scroll_on_play_info = {
      sizeof (AgsScrollOnPlayClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_scroll_on_play_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsScrollOnPlay),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_scroll_on_play_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_scroll_on_play_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_scroll_on_play = g_type_register_static(AGS_TYPE_TASK,
						     "AgsScrollOnPlay\0",
						     &ags_scroll_on_play_info,
						     0);

    g_type_add_interface_static(ags_type_scroll_on_play,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_scroll_on_play);
}

void
ags_scroll_on_play_class_init(AgsScrollOnPlayClass *scroll_on_play)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_scroll_on_play_parent_class = g_type_class_peek_parent(scroll_on_play);

  /* gobject */
  gobject = (GObjectClass *) scroll_on_play;

  gobject->finalize = ags_scroll_on_play_finalize;

  /* task */
  task = (AgsTaskClass *) scroll_on_play;

  task->launch = ags_scroll_on_play_launch;
}

void
ags_scroll_on_play_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_scroll_on_play_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_scroll_on_play_connect;
  connectable->disconnect = ags_scroll_on_play_disconnect;
}

void
ags_scroll_on_play_init(AgsScrollOnPlay *scroll_on_play)
{
  scroll_on_play->editor = NULL;
}

void
ags_scroll_on_play_connect(AgsConnectable *connectable)
{
  ags_scroll_on_play_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_scroll_on_play_disconnect(AgsConnectable *connectable)
{
  ags_scroll_on_play_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_scroll_on_play_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_scroll_on_play_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_scroll_on_play_launch(AgsTask *task)
{
  AgsMachine *machine;
  AgsEditor *editor;
  AgsAutomationEditor *automation_editor;
  
  AgsCountBeatsAudioRun *count_beats_audio_run;
  
  AgsScrollOnPlay *scroll_on_play;
  
  cairo_t *cr;
  
  GList *editor_child, *recall;

  gdouble position, value;
  guint control_width;

  scroll_on_play = AGS_SCROLL_ON_PLAY(task);

  gdk_threads_enter();

  if(AGS_IS_EDITOR(scroll_on_play->editor)){
    editor = AGS_EDITOR(scroll_on_play->editor);
    automation_editor = NULL;
  }else if(AGS_IS_AUTOMATION_EDITOR(scroll_on_play->editor)){
    editor = NULL;
    automation_editor = AGS_AUTOMATION_EDITOR(scroll_on_play->editor);

    //TODO:JK: implement me

    return;
  }else{
    return;
  }

  if(editor != NULL){
    editor_child = editor->editor_child;
  }else{
    editor_child = automation_editor->automation_editor_child;
  }
  
  while(editor_child != NULL){    
    if(editor != NULL){
      machine = AGS_EDITOR_CHILD(editor_child->data)->machine;
    }else{
      machine = AGS_AUTOMATION_EDITOR_CHILD(editor_child->data)->machine;
    }
    
    if(machine == NULL){
      editor_child = editor_child->next;
      
      continue;
    }

    recall = machine->audio->play;

    while((recall = ags_recall_find_type(recall,
					 AGS_TYPE_COUNT_BEATS_AUDIO_RUN)) != NULL){
      if(AGS_RECALL(recall->data)->recall_id != NULL && (AGS_RECALL_ID_NOTATION & (AGS_RECALL(recall->data)->recall_id->flags)) != 0){
	break;
      }

      recall = recall->next;
    }

    if(recall == NULL){
      editor_child = editor_child->next;
      
      continue;
    }

    count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(recall->data);

    if(editor != NULL){
      if(AGS_IS_NOTE_EDIT(AGS_EDITOR_CHILD(editor_child->data)->edit_widget)){
	AgsNoteEdit *note_edit;

	note_edit = AGS_NOTE_EDIT(AGS_EDITOR_CHILD(editor_child->data)->edit_widget);

	control_width = note_edit->control_unit.control_width;
      
	position = count_beats_audio_run->notation_counter * note_edit->control_unit.control_width;

	/* scroll */
	note_edit->flags |= AGS_NOTE_EDIT_RESETING_HORIZONTALLY;

	if(position - control_width > 0){
	  value = (position - control_width);
	}else{
	  value = 0.0;
	}

	if(position - control_width > 0){
	  gtk_range_set_value(GTK_RANGE(note_edit->hscrollbar),
			      value);
	  gtk_adjustment_set_value(note_edit->ruler->adjustment,
				   (value) /
				   note_edit->control_current.control_width);
	  gtk_widget_queue_draw((GtkWidget *) note_edit->ruler);
	}
      
	/*  */
	if(position - control_width > 0){
	  note_edit->control_current.x0 = ((guint) round((double) value)) % note_edit->control_current.control_width;

	  if(note_edit->control_current.x0 != 0){
	    note_edit->control_current.x0 = note_edit->control_current.control_width - note_edit->control_current.x0;
	  }
	
	  note_edit->control_current.x1 = (note_edit->width - note_edit->control_current.x0) % note_edit->control_current.control_width;

	  note_edit->control_current.nth_x = (guint) ceil((double)(value) / (double)(note_edit->control_current.control_width));

	  note_edit->control_unit.x0 = ((guint)round((double) value)) % note_edit->control_unit.control_width;

	  if(note_edit->control_unit.x0 != 0){
	    note_edit->control_unit.x0 = note_edit->control_unit.control_width - note_edit->control_unit.x0;
	  }
      
	  note_edit->control_unit.x1 = (note_edit->width - note_edit->control_unit.x0) % note_edit->control_unit.control_width;
      
	  note_edit->control_unit.nth_x = (guint) ceil(round((double) value) / (double) (note_edit->control_unit.control_width));
	  note_edit->control_unit.stop_x = note_edit->control_unit.nth_x + (note_edit->width - note_edit->control_unit.x0 - note_edit->control_unit.x1) / note_edit->control_unit.control_width;
	}
      
	note_edit->flags &= (~AGS_NOTE_EDIT_RESETING_HORIZONTALLY);

	/*  */
	if(GTK_WIDGET_VISIBLE(AGS_EDITOR_CHILD(editor_child->data)->edit_widget)){
	  cr = gdk_cairo_create(GTK_WIDGET(note_edit->drawing_area)->window);

	  cairo_surface_flush(cairo_get_target(cr));
	  cairo_push_group(cr);

	  ags_note_edit_draw_segment(note_edit, cr);
	  ags_note_edit_draw_notation(note_edit, cr);

	  cairo_pop_group_to_source(cr);
	  cairo_paint(cr);

	  /* draw fader */
	  cairo_push_group(cr);

	  cr = gdk_cairo_create(GTK_WIDGET(note_edit->drawing_area)->window);
	  ags_note_edit_draw_scroll(note_edit, cr,
				    position);

	  cairo_pop_group_to_source(cr);
	  cairo_paint(cr);

	  cairo_surface_mark_dirty(cairo_get_target(cr));
	  cairo_destroy(cr);
	}
      }else if(AGS_IS_PATTERN_EDIT(AGS_EDITOR_CHILD(editor_child->data)->edit_widget)){
	AgsPatternEdit *pattern_edit;

	pattern_edit = AGS_PATTERN_EDIT(AGS_EDITOR_CHILD(editor_child->data)->edit_widget);

	position = (count_beats_audio_run->notation_counter) * (pattern_edit->control_unit.control_width);

	control_width = pattern_edit->control_unit.control_width;

	/* scroll */
	pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
	
	if(position - control_width > 0){
	  value = (position - control_width);
	}else{
	  value = 0.0;
	}

	if(position - control_width > 0){
	  gtk_range_set_value(GTK_RANGE(pattern_edit->hscrollbar),
			      value);
	  gtk_adjustment_set_value(pattern_edit->ruler->adjustment,
				   (value) /
				   pattern_edit->control_current.control_width);

	  gtk_widget_queue_draw((GtkWidget *) pattern_edit->ruler);
	}
      
	/*  */
	if(position - control_width > 0){
	  pattern_edit->control_current.x0 = ((guint) round((double) value)) % pattern_edit->control_current.control_width;

	  if(pattern_edit->control_current.x0 != 0){
	    pattern_edit->control_current.x0 = pattern_edit->control_current.control_width - pattern_edit->control_current.x0;
	  }
	
	  pattern_edit->control_current.x1 = (pattern_edit->width - pattern_edit->control_current.x0) % pattern_edit->control_current.control_width;

	  pattern_edit->control_current.nth_x = (guint) ceil((double)(value) / (double)(pattern_edit->control_current.control_width));

	  pattern_edit->control_unit.x0 = ((guint)round((double) value)) % pattern_edit->control_unit.control_width;

	  if(pattern_edit->control_unit.x0 != 0){
	    pattern_edit->control_unit.x0 = pattern_edit->control_unit.control_width - pattern_edit->control_unit.x0;
	  }
      
	  pattern_edit->control_unit.x1 = (pattern_edit->width - pattern_edit->control_unit.x0) % pattern_edit->control_unit.control_width;
      
	  pattern_edit->control_unit.nth_x = (guint) ceil(round((double) value) / (double) (pattern_edit->control_unit.control_width));
	  pattern_edit->control_unit.stop_x = pattern_edit->control_unit.nth_x + (pattern_edit->width - pattern_edit->control_unit.x0 - pattern_edit->control_unit.x1) / pattern_edit->control_unit.control_width;
	}
      
	pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);

	/*  */
	if(GTK_WIDGET_VISIBLE(AGS_EDITOR_CHILD(editor_child->data)->edit_widget)){
	  cr = gdk_cairo_create(GTK_WIDGET(pattern_edit->drawing_area)->window);

	  cairo_surface_flush(cairo_get_target(cr));
	  cairo_push_group(cr);

	  ags_pattern_edit_draw_segment(pattern_edit, cr);
	  ags_pattern_edit_draw_notation(pattern_edit, cr);

	  cairo_pop_group_to_source(cr);
	  cairo_paint(cr);

	  /* draw fader */
	  cairo_push_group(cr);

	  cr = gdk_cairo_create(GTK_WIDGET(pattern_edit->drawing_area)->window);
	  ags_pattern_edit_draw_scroll(pattern_edit, cr,
				       position);

	  cairo_pop_group_to_source(cr);
	  cairo_paint(cr);

	  cairo_surface_mark_dirty(cairo_get_target(cr));
	  cairo_destroy(cr);
	}
      }
    }
  
    editor_child = editor_child->next;
  }
  
  gdk_threads_leave();
}

/**
 * ags_scroll_on_play_new:
 * @editor: the #AgsEditor to scroll
 * @step: the amount to increment in pixel per control width
 *
 * Creates an #AgsScrollOnPlay.
 *
 * Returns: an new #AgsScrollOnPlay.
 *
 * Since: 0.4
 */
AgsScrollOnPlay*
ags_scroll_on_play_new(GtkWidget *editor, gdouble step)
{
  AgsScrollOnPlay *scroll_on_play;

  scroll_on_play = (AgsScrollOnPlay *) g_object_new(AGS_TYPE_SCROLL_ON_PLAY,
						    NULL);

  scroll_on_play->editor = editor;
  scroll_on_play->step = step;
  
  return(scroll_on_play);
}
