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

#include <ags/X/editor/ags_automation_edit_callbacks.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>

#include <ags/X/ags_automation_editor.h>

#include <ags/X/editor/ags_notebook.h>
#include <ags/X/editor/ags_automation_toolbar.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_ffplayer.h>

#include <math.h>
#include <gdk/gdkkeysyms.h>

gboolean
ags_automation_edit_drawing_area_expose_event(GtkWidget *widget, GdkEventExpose *event,
					      AgsAutomationEdit *automation_edit)
{
  cairo_t *cr;
  
  cr = gdk_cairo_create(widget->window);
  cairo_push_group(cr);
    
  ags_automation_edit_paint(automation_edit,
			    cr);

  cairo_pop_group_to_source(cr);
  cairo_paint(cr);

  return(TRUE);
}

gboolean
ags_automation_edit_drawing_area_configure_event(GtkWidget *widget, GdkEventConfigure *event,
						 AgsAutomationEdit *automation_edit)
{
  cairo_t *cr;
  
  automation_edit->flags |= AGS_AUTOMATION_EDIT_RESETING_VERTICALLY;
  ags_automation_edit_reset_vertically(automation_edit,
				       AGS_AUTOMATION_EDIT_RESET_VSCROLLBAR);
  automation_edit->flags &= (~AGS_AUTOMATION_EDIT_RESETING_VERTICALLY);

  automation_edit->flags |= AGS_AUTOMATION_EDIT_RESETING_HORIZONTALLY;
  ags_automation_edit_reset_horizontally(automation_edit,
					 AGS_AUTOMATION_EDIT_RESET_HSCROLLBAR |
					 AGS_AUTOMATION_EDIT_RESET_WIDTH);
  automation_edit->flags &= (~AGS_AUTOMATION_EDIT_RESETING_HORIZONTALLY);  

  cr = gdk_cairo_create(widget->window);
  cairo_push_group(cr);
  
  ags_automation_edit_paint(automation_edit,
  			    cr);
  
  cairo_pop_group_to_source(cr);
  cairo_paint(cr);

  return(FALSE);
}

gboolean
ags_automation_edit_drawing_area_button_press_event(GtkWidget *widget, GdkEventButton *event,
						    AgsAutomationEdit *automation_edit)
{
  AgsAutomationEditor *automation_editor;
  AgsAutomationToolbar *automation_toolbar;

  double tact_factor, zoom_factor;
  double tact;
  
  cairo_t *cr;
  
  guint x, y;

  auto void ags_automation_edit_drawing_area_button_press_event_add_point();

  void ags_automation_edit_drawing_area_button_press_event_add_point(){
    AgsNotebook *notebook;
    
    AgsAcceleration *current_acceleration, *acceleration;

    GList *list, *list_start;
    GList *automation;

    GType channel_type;

    gdouble gui_y, acceleration_y;

    gdouble val;
    gdouble upper, lower, range, step;
    gdouble c_upper, c_lower, c_range;

    guint i;
    guint line;
    gboolean is_audio, is_output, is_input;
  
    if(automation_edit == automation_editor->current_audio_automation_edit){
      notebook = NULL;
      channel_type = G_TYPE_NONE;

      is_audio = TRUE;
    }else if(automation_edit == automation_editor->current_output_automation_edit){
      notebook = automation_editor->current_output_notebook;
      channel_type = AGS_TYPE_OUTPUT;
      
      is_output = TRUE;
    }else if(automation_edit == automation_editor->current_input_automation_edit){
      notebook = automation_editor->current_input_notebook;
      channel_type = AGS_TYPE_INPUT;
      
      is_input = TRUE;
    }

    /* find automation area */
    list =
      list_start = g_list_reverse(g_list_copy(automation_edit->automation_area));
    i = 0;
    
    while(list != NULL){
      if(i + AGS_AUTOMATION_AREA(list->data)->height > y){
	automation_edit->current_area = AGS_AUTOMATION_AREA(list->data);
	break;
      }

      i = i + AGS_AUTOMATION_AREA(list->data)->height + AGS_AUTOMATION_EDIT_DEFAULT_MARGIN;
      list = list->next;
    }

    g_list_free(list_start);

    if(list == NULL){
      return;
    }
    
    /* match specifier */
    if(channel_type == G_TYPE_NONE){
      automation = automation_editor->selected_machine->audio->automation;

      while((automation = ags_automation_find_specifier_with_type_and_line(automation,
									   automation_edit->current_area->control_name,
									   channel_type,
									   0)) != NULL){
	upper = AGS_AUTOMATION(automation->data)->upper;
	lower = AGS_AUTOMATION(automation->data)->lower;
	
	range = upper - lower;

	if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	  c_upper = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					   upper,
					   FALSE);
	  c_lower = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					   lower,
					   FALSE);
	  c_range = c_upper - c_lower;
	}else{
	  c_upper = upper;
	  c_lower = lower;

	  c_range = range;
	}

	if(range == 0.0){
	  automation = automation->next;
	  g_warning("ags_automation_edit_callbacks.c - range = 0.0\0");
	
	  continue;
	}

	/* check steps */
	gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / automation_edit->current_area->height) * (gdouble) (y - i));
	acceleration_y = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	/* conversion */
	val = acceleration_y;

	if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	  val = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
				       val,
				       TRUE);
	}
	
	/* find existing */
	current_acceleration = ags_automation_find_point(AGS_AUTOMATION(automation->data),
							 x, val,
							 FALSE);

	/* add acceleration */
	if(current_acceleration == NULL){
	  acceleration = ags_acceleration_new();
	  acceleration->x = x;
	  acceleration->y = val;
      
	  ags_automation_add_acceleration(automation->data,
					  acceleration,
					  FALSE);
	}

	automation = automation->next;
      }

    }else{
      line = 0;
    
      while((line = ags_notebook_next_active_tab(notebook,
						 line)) != -1){
	automation = automation_editor->selected_machine->audio->automation;

	while((automation = ags_automation_find_specifier_with_type_and_line(automation,
									     automation_edit->current_area->control_name,
									     channel_type,
									     line)) != NULL){
	  upper = AGS_AUTOMATION(automation->data)->upper;
	  lower = AGS_AUTOMATION(automation->data)->lower;
	
	  range = upper - lower;

	  if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	    c_upper = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					     upper,
					     FALSE);
	    c_lower = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					     lower,
					     FALSE);
	    c_range = c_upper - c_lower;
	  }else{
	    c_upper = upper;
	    c_lower = lower;

	    c_range = range;
	  }

	  if(range == 0.0){
	    automation = automation->next;
	    g_warning("ags_automation_edit_callbacks.c - range = 0.0\0");
	
	    continue;
	  }

	  /* check steps */
	  gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / automation_edit->current_area->height) * (gdouble) (y - i));
	  acceleration_y = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	  /* conversion */
	  val = acceleration_y;

	  if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	    val = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					 val,
					 TRUE);
	  }
	
	  /* add acceleration */
	  current_acceleration = ags_automation_find_point(AGS_AUTOMATION(automation->data),
							   x, val,
							   FALSE);

	  /* add acceleration */
	  if(current_acceleration == NULL){
	    acceleration = ags_acceleration_new();
	    acceleration->x = x;
	    acceleration->y = val;
      
	    ags_automation_add_acceleration(automation->data,
					    acceleration,
					    FALSE);
	  }

	  automation = automation->next;
	}

	line++;
      }
    }    
  }
  
  automation_editor = (AgsAutomationEditor *) gtk_widget_get_ancestor(GTK_WIDGET(automation_edit),
								      AGS_TYPE_AUTOMATION_EDITOR);
  gtk_widget_grab_focus(automation_edit->drawing_area);

  if(automation_editor->selected_machine != NULL &&
     event->button == 1){
    automation_toolbar = automation_editor->automation_toolbar;
    
    zoom_factor = 1.0 / 4.0;

    tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active(automation_toolbar->zoom));
    tact = exp2((double) gtk_combo_box_get_active(automation_toolbar->zoom) - 2.0);

    x = (guint) (GTK_RANGE(automation_edit->hscrollbar)->adjustment->value + (guint) event->x) / tact;
    y = (guint) GTK_RANGE(automation_edit->vscrollbar)->adjustment->value + (guint) event->y;

    if(automation_toolbar->selected_edit_mode == automation_toolbar->position){
      automation_edit->flags |= AGS_AUTOMATION_EDIT_POSITION_CURSOR;

      automation_edit->edit_x = x;
      automation_edit->edit_y = y;
      
      gtk_widget_queue_draw(automation_edit->drawing_area);
    }else if(automation_toolbar->selected_edit_mode == automation_toolbar->edit){
      automation_edit->flags |= AGS_AUTOMATION_EDIT_ADDING_ACCELERATION;

      cr = gdk_cairo_create(widget->window);
      cairo_push_group(cr);
    
      ags_automation_edit_drawing_area_button_press_event_add_point();
      ags_automation_edit_paint(automation_edit,
				cr);
      
      cairo_pop_group_to_source(cr);
      cairo_paint(cr);
      
      automation_edit->edit_x = x;
      automation_edit->edit_y = y;
    }else if(automation_toolbar->selected_edit_mode == automation_toolbar->clear){
      automation_edit->flags |= AGS_AUTOMATION_EDIT_DELETING_ACCELERATION;
    }else if(automation_toolbar->selected_edit_mode == automation_toolbar->select){
      automation_edit->flags |= AGS_AUTOMATION_EDIT_SELECTING_ACCELERATIONS;

      automation_edit->select_x0 = x;
      automation_edit->select_y0 = y;
    }
  }

  return(TRUE);
}

gboolean
ags_automation_edit_drawing_area_button_release_event(GtkWidget *widget, GdkEventButton *event,
						      AgsAutomationEdit *automation_edit)
{
  AgsAutomationEditor *automation_editor;
  AgsAutomationToolbar *automation_toolbar;
  AgsNotebook *notebook;

  cairo_t *cr;
  
  double tact_factor, zoom_factor;
  double tact;
  guint x, y;

  auto void ags_automation_edit_drawing_area_button_release_event_delete_point(cairo_t *cr);
  auto void ags_automation_edit_drawing_area_button_release_event_select(cairo_t *cr);

  void ags_automation_edit_drawing_area_button_release_event_delete_point(cairo_t *cr){
    AgsNotebook *notebook;
    
    AgsAcceleration *current_acceleration, *acceleration;

    GList *list, *list_start;
    GList *automation;

    GType channel_type;

    gdouble gui_y, acceleration_y;

    gdouble val;
    gdouble upper, lower, range, step;
    gdouble c_upper, c_lower, c_range;

    guint n_attempts;
    guint i, n, m;
    guint line;
    gboolean is_audio, is_output, is_input;
    
    if(automation_edit == automation_editor->current_audio_automation_edit){
      notebook = NULL;
      channel_type = G_TYPE_NONE;

      is_audio = TRUE;
    }else if(automation_edit == automation_editor->current_output_automation_edit){
      notebook = automation_editor->current_output_notebook;
      channel_type = AGS_TYPE_OUTPUT;
      
      is_output = TRUE;
    }else if(automation_edit == automation_editor->current_input_automation_edit){
      notebook = automation_editor->current_input_notebook;
      channel_type = AGS_TYPE_INPUT;
      
      is_input = TRUE;
    }

    n_attempts = 3;
    
    /* find automation area */
    list =
      list_start = g_list_reverse(g_list_copy(automation_edit->automation_area));
    i = 0;
    line = 0;
    
    while(list != NULL){
      if(i + AGS_AUTOMATION_AREA(list->data)->height > y){
	automation_edit->current_area = AGS_AUTOMATION_AREA(list->data);
	break;
      }

      i = i + AGS_AUTOMATION_AREA(list->data)->height + AGS_AUTOMATION_EDIT_DEFAULT_MARGIN;
      list = list->next;
    }

    g_list_free(list_start);

    if(list == NULL){
      return;
    }
      
    /* match specifier */
    if(channel_type == G_TYPE_NONE){
      automation = automation_editor->selected_machine->audio->automation;

      while((automation = ags_automation_find_specifier_with_type_and_line(automation,
									   automation_edit->current_area->control_name,
									   channel_type,
									   0)) != NULL){
	upper = AGS_AUTOMATION(automation->data)->upper;
	lower = AGS_AUTOMATION(automation->data)->lower;
	
	range = upper - lower;

	if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	  c_upper = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					   upper,
					   FALSE);
	  c_lower = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					   lower,
					   FALSE);
	  c_range = c_upper - c_lower;
	}else{
	  c_upper = upper;
	  c_lower = lower;

	  c_range = range;
	}

	if(range == 0.0){
	  automation = automation->next;
	  g_warning("ags_automation_edit_callbacks.c - range = 0.0\0");
	
	  continue;
	}

	/* check steps */
	gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / automation_edit->current_area->height) * (gdouble) (y - i));
	acceleration_y = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	/* conversion */
	val = acceleration_y;

	if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	  val = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
				       val,
				       TRUE);
	}
	
	/* remove acceleration */
	if(!ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
							   x, val)){
	  
	  for(m = 0; m < tact * n_attempts; m++){
	    for(n = 0; n < n_attempts; n++){
	      if(m % 2 == 1){
		if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								  x - (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		  goto remove_SUCCESS_0;
		}
	      }else{
		if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								  x + (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		  goto remove_SUCCESS_0;
		}
	      }
	    }
	  }
	}

      remove_SUCCESS_0:	
	automation = automation->next;
      }

    }else{
      line = 0;
    
      while((line = ags_notebook_next_active_tab(notebook,
						 line)) != -1){
	automation = automation_editor->selected_machine->audio->automation;

	while((automation = ags_automation_find_specifier_with_type_and_line(automation,
									     automation_edit->current_area->control_name,
									     channel_type,
									     line)) != NULL){
	  upper = AGS_AUTOMATION(automation->data)->upper;
	  lower = AGS_AUTOMATION(automation->data)->lower;
	
	  range = upper - lower;

	  if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	    c_upper = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					     upper,
					     FALSE);
	    c_lower = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					     lower,
					     FALSE);
	    c_range = c_upper - c_lower;
	  }else{
	    c_upper = upper;
	    c_lower = lower;

	    c_range = range;
	  }

	  if(range == 0.0){
	    automation = automation->next;
	    g_warning("ags_automation_edit_callbacks.c - range = 0.0\0");
	
	    continue;
	  }

	  /* check steps */
	  gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / automation_edit->current_area->height) * (gdouble) (y - i));
	  acceleration_y = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	  /* conversion */
	  val = acceleration_y;

	  if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	    val = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					 val,
					 TRUE);
	  }
	
	  /* remove acceleration */
	  if(!ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
							     x, val)){
	  
	    for(m = 0; m < tact * n_attempts; m++){
	      for(n = 0; n < n_attempts; n++){
		if(m % 2 == 1){
		  if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								    x - (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		    goto remove_SUCCESS_1;
		  }
		}else{
		  if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								    x + (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		    goto remove_SUCCESS_1;
		  }
		}
	      }
	    }
	  }

	remove_SUCCESS_1:
	  automation = automation->next;
	}

	line++;
      }
    }    
  }

  void ags_automation_edit_drawing_area_button_release_event_select(cairo_t *cr){
    AgsNotebook *notebook;
    
    GList *list, *list_start;
    GList *automation;

    GType channel_type;

    gdouble gui_y;

    gdouble c_y0, c_y1;
    gdouble upper, lower, range, step;
    gdouble c_upper, c_lower, c_range;

    guint x0, x1;
    guint y0, y1;
    guint line;
    gboolean is_audio, is_output, is_input;

    if(automation_edit == automation_editor->current_audio_automation_edit){
      notebook = NULL;
      channel_type = G_TYPE_NONE;

      is_audio = TRUE;
    }else if(automation_edit == automation_editor->current_output_automation_edit){
      notebook = automation_editor->current_output_notebook;
      channel_type = AGS_TYPE_OUTPUT;
      
      is_output = TRUE;
    }else if(automation_edit == automation_editor->current_input_automation_edit){
      notebook = automation_editor->current_input_notebook;
      channel_type = AGS_TYPE_INPUT;
      
      is_input = TRUE;
    }

    /* get boundaries */
    if(automation_edit->select_x0 > automation_edit->select_x1){
      x0 = automation_edit->select_x1;
      x1 = automation_edit->select_x0;
    }else{
      x0 = automation_edit->select_x0;
      x1 = automation_edit->select_x1;
    }

    if(automation_edit->select_y0 > automation_edit->select_y1){
      y0 = automation_edit->select_y1;
      y1 = automation_edit->select_y0;
    }else{
      y0 = automation_edit->select_y0;
      y1 = automation_edit->select_y1;
    }
    
    /* find automation area */
    list =
      list_start = g_list_reverse(g_list_copy(automation_edit->automation_area));

    while(list != NULL){
      if(AGS_AUTOMATION_AREA(list->data)->y > y1){
	break;
      }

      if(AGS_AUTOMATION_AREA(list->data)->y + AGS_AUTOMATION_AREA(list->data)->height > y0){
	line = 0;
	
	while((line = ags_notebook_next_active_tab(notebook,
						   line)) != -1){
	  automation = automation_editor->selected_machine->audio->automation;

	  while((automation = ags_automation_find_specifier_with_type_and_line(automation,
									       AGS_AUTOMATION_AREA(list->data)->control_name,
									       channel_type,
									       line)) != NULL){

	    upper = AGS_AUTOMATION(automation->data)->upper;
	    lower = AGS_AUTOMATION(automation->data)->lower;
	
	    range = upper - lower;

	    if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	      c_upper = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					       upper,
					       FALSE);
	      c_lower = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					       lower,
					       FALSE);
	      c_range = c_upper - c_lower;
	    }else{
	      c_upper = upper;
	      c_lower = lower;
	      
	      c_range = range;
	    }

	    if(range == 0.0){
	      automation = automation->next;
	      g_warning("ags_automation_edit_callbacks.c - range = 0.0\0");
	      
	      continue;
	    }

	    /* check steps */
	    if(AGS_AUTOMATION_AREA(list->data)->y > y0){
	      gui_y = AGS_AUTOMATION(automation->data)->steps;
	    }else{
	      gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / AGS_AUTOMATION_AREA(list->data)->height) * (gdouble) (y0 - AGS_AUTOMATION_AREA(list->data)->y));
	    }
	    
	    c_y0 = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	    /* conversion */
	    if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	      c_y0 = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					    c_y0,
					    TRUE);
	    }

	    /* check steps */
	    if(AGS_AUTOMATION_AREA(list->data)->y + AGS_AUTOMATION_AREA(list->data)->height < automation_edit->select_y1){
	      gui_y = 0;
	    }else{
	      gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / AGS_AUTOMATION_AREA(list->data)->height) * (gdouble) (y1 - AGS_AUTOMATION_AREA(list->data)->y));
	    }
	    
	    c_y1 = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	    /* conversion */
	    if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	      c_y1 = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					    c_y1,
					    TRUE);
	    }

#ifdef AGS_DEBUG
	    g_message("%s %u %f %u %f\0",
		      AGS_AUTOMATION_AREA(list->data)->control_name,
		      x0, c_y0,
		      x1, c_y1);
#endif
	    
	    /* select */
	    ags_automation_add_region_to_selection(automation->data,
						   x0, c_y0,
						   x1, c_y1,
						   TRUE);

	    automation = automation->next;
	  }

	  line++;
	}
      }

      list = list->next;
    }

    g_list_free(list_start);
  }

  automation_editor = (AgsAutomationEditor *) gtk_widget_get_ancestor(GTK_WIDGET(automation_edit),
								      AGS_TYPE_AUTOMATION_EDITOR);

  if(automation_editor->selected_machine != NULL &&
     event->button == 1){
    automation_toolbar = automation_editor->automation_toolbar;
    
    zoom_factor = 1.0 / 4.0;

    tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active(automation_toolbar->zoom));
    tact = exp2((double) gtk_combo_box_get_active(automation_toolbar->zoom) - 2.0);

    x = (guint) (GTK_RANGE(automation_edit->hscrollbar)->adjustment->value + (guint) event->x) / tact;
    y = (guint) GTK_RANGE(automation_edit->vscrollbar)->adjustment->value + (guint) event->y;

    if((AGS_AUTOMATION_EDIT_POSITION_CURSOR & (automation_edit->flags)) != 0){
      automation_edit->flags &= (~AGS_AUTOMATION_EDIT_POSITION_CURSOR);

      //TODO:JK: implement me
    }else if((AGS_AUTOMATION_EDIT_ADDING_ACCELERATION & (automation_edit->flags)) != 0){
      automation_edit->flags &= (~AGS_AUTOMATION_EDIT_ADDING_ACCELERATION);
    }else if((AGS_AUTOMATION_EDIT_DELETING_ACCELERATION & (automation_edit->flags)) != 0){
      automation_edit->flags &= (~AGS_AUTOMATION_EDIT_DELETING_ACCELERATION);

      ags_automation_edit_drawing_area_button_release_event_delete_point(cr);
      
      gtk_widget_queue_draw(automation_edit->drawing_area);
    }else if((AGS_AUTOMATION_EDIT_SELECTING_ACCELERATIONS & (automation_edit->flags)) != 0){
      automation_edit->flags &= (~AGS_AUTOMATION_EDIT_SELECTING_ACCELERATIONS);

      automation_edit->select_x1 = x;
      automation_edit->select_y1 = y;

      ags_automation_edit_drawing_area_button_release_event_select(cr);
      
      gtk_widget_queue_draw(automation_edit->drawing_area);
    }
  }
  
  return(FALSE);
}

gboolean
ags_automation_edit_drawing_area_motion_notify_event(GtkWidget *widget, GdkEventMotion *event,
						     AgsAutomationEdit *automation_edit)
{
  AgsAutomationEditor *automation_editor;
  AgsAutomationToolbar *automation_toolbar;
  
  cairo_t *cr;

  double tact_factor, zoom_factor;
  double tact;
  guint x, y;
  
  auto void ags_automation_edit_drawing_area_motion_notify_event_draw_selection(cairo_t *cr);

  void ags_automation_edit_drawing_area_motion_notify_event_draw_selection(cairo_t *cr){
    GtkAllocation allocation;
    guint x0_offset, x1_offset, y0_offset, y1_offset;
    guint x0, x1, y0, y1, width, height;
    guint x0_viewport, x1_viewport, y0_viewport, y1_viewport;

    /* get viewport */
    gtk_widget_get_allocation(widget, &allocation);

    x0_viewport = (guint) GTK_RANGE(automation_edit->hscrollbar)->adjustment->value / tact;
    x1_viewport = (guint) (GTK_RANGE(automation_edit->hscrollbar)->adjustment->value + allocation.width) / tact;

    y0_viewport = (guint) GTK_RANGE(automation_edit->vscrollbar)->adjustment->value;
    y1_viewport = (guint) GTK_RANGE(automation_edit->vscrollbar)->adjustment->value + allocation.height;

    /* get real size and offset */
    x0 = automation_edit->select_x0;

    if(event->x >= 0.0){
      x1 = x0_viewport + (event->x / tact);
    }else{
      x1 = x0_viewport;
    }
    
    if(x0 > x1){
      x0_offset = x1;
      x1_offset = x0;

      x1 = x0_offset;
      x0 = x1_offset;
    }else{
      x0_offset = x0;
      x1_offset = x1;
    }

    /* get drawable size and offset */
    if(x0 < x0_viewport){
      //      x0 = 0;
      //      width = x1_offset - x0_viewport;
      x0 -= x0_viewport;
      width = x1 - x0;
    }else{
      x0 -= x0_viewport;
      width = x1 - x0;
    }

    if(x1 > x1_viewport){
      width -= (x1 - x1_viewport);
    }else{
      width -= x0_viewport;
    }

    /* get real size and offset */
    y0 = automation_edit->select_y0;

    if(event->y >= 0.0){
      y1 = y0_viewport + event->y;
    }else{
      y1 = y0_viewport;
    }
    
    if(y0 > y1){
      y0_offset = y1;
      y1_offset = y0;

      y1 = y0_offset;
      y0 = y1_offset;
    }else{
      y0_offset = y0;
      y1_offset = y1;
    }

    /* get drawable size and offset */
    if(y0 < y0_viewport){
      //      y0 = 0;
      //      height = y1_offset - y0_viewport;
      y0 -= y0_viewport;
      height = y1 - y0;
    }else{
      y0 -= y0_viewport;
      height = y1 - y0;
    }

    if(y1 > y1_viewport){
      height -= (y1 - y1_viewport);
    }else{
      height -= y0_viewport;
    }

    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
    cairo_rectangle(cr, (double) x0 * tact, (double) y0, (double) width * tact, (double) height);
    cairo_fill(cr);
  }

  automation_editor = (AgsAutomationEditor *) gtk_widget_get_ancestor(GTK_WIDGET(automation_edit),
								      AGS_TYPE_AUTOMATION_EDITOR);

  if(automation_editor->selected_machine != NULL){
    automation_toolbar = automation_editor->automation_toolbar;

    zoom_factor = 1.0 / 4.0;

    tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active(automation_toolbar->zoom));
    tact = exp2((double) gtk_combo_box_get_active(automation_toolbar->zoom) - 2.0);

    if(event->x >= 0.0){
      x = (guint) event->x;
    }else{
      x = 0;
    }

    if(event->y >= 0.0){
      y = (guint) event->y;
    }else{
      y = 0;
    }
    
    if((AGS_AUTOMATION_EDIT_POSITION_CURSOR & (automation_edit->flags)) != 0){
      //TODO:JK: implement me
    }else if((AGS_AUTOMATION_EDIT_ADDING_ACCELERATION & (automation_edit->flags)) != 0){
      //TODO:JK: implement me
    }else if((AGS_AUTOMATION_EDIT_SELECTING_ACCELERATIONS & (automation_edit->flags)) != 0){
      automation_edit->select_x1 = x / tact;
      automation_edit->select_y1 = GTK_RANGE(automation_edit->vscrollbar)->adjustment->value + y;

      cr = gdk_cairo_create(widget->window);
      cairo_push_group(cr);
    
      ags_automation_edit_paint(automation_edit,
				cr);
      ags_automation_edit_drawing_area_motion_notify_event_draw_selection(cr);

      cairo_pop_group_to_source(cr);
      cairo_paint(cr);
    }
  }
  
  return(FALSE);
}

gboolean
ags_automation_edit_drawing_area_key_press_event(GtkWidget *widget, GdkEventKey *event,
						 AgsAutomationEdit *automation_edit)
{
  AgsAutomationEditor *automation_editor;

  AgsMachine *machine;

  gboolean retval;
  
  if(event->keyval == GDK_KEY_Tab ||
     event->keyval == GDK_ISO_Left_Tab ||
     event->keyval == GDK_KEY_Shift_L ||
     event->keyval == GDK_KEY_Shift_R ||
     event->keyval == GDK_KEY_Alt_L ||
     event->keyval == GDK_KEY_Alt_R ||
     event->keyval == GDK_KEY_Control_L ||
     event->keyval == GDK_KEY_Control_R ){
    retval = FALSE;
  }else{
    retval = TRUE;
  }

  automation_editor = (AgsAutomationEditor *) gtk_widget_get_ancestor(GTK_WIDGET(automation_edit),
								      AGS_TYPE_AUTOMATION_EDITOR);

  machine = automation_editor->selected_machine;
  
  if(machine != NULL){
    switch(event->keyval){
    case GDK_KEY_Control_L:
      {
	automation_edit->key_mask |= AGS_AUTOMATION_EDIT_KEY_L_CONTROL;
      }
      break;
    case GDK_KEY_Control_R:
      {
	automation_edit->key_mask |= AGS_AUTOMATION_EDIT_KEY_R_CONTROL;
      }
      break;
    case GDK_KEY_Shift_L:
      {
	automation_edit->key_mask |= AGS_AUTOMATION_EDIT_KEY_L_SHIFT;
      }
      break;
    case GDK_KEY_Shift_R:
      {
	automation_edit->key_mask |= AGS_AUTOMATION_EDIT_KEY_R_SHIFT;
      }
      break;
    case GDK_KEY_a:
      {
	/* select all notes */
	if((AGS_AUTOMATION_EDIT_KEY_L_CONTROL & (automation_edit->key_mask)) != 0 || (AGS_AUTOMATION_EDIT_KEY_R_CONTROL & (automation_edit->key_mask)) != 0){
	  ags_automation_editor_select_all(automation_editor);
	}
      }
      break;
    case GDK_KEY_c:
      {
	/* copy notes */
	if((AGS_AUTOMATION_EDIT_KEY_L_CONTROL & (automation_edit->key_mask)) != 0 || (AGS_AUTOMATION_EDIT_KEY_R_CONTROL & (automation_edit->key_mask)) != 0){
	  ags_automation_editor_copy(automation_editor);
	}
      }
      break;
    case GDK_KEY_v:
      {
	/* paste notes */
	if((AGS_AUTOMATION_EDIT_KEY_L_CONTROL & (automation_edit->key_mask)) != 0 || (AGS_AUTOMATION_EDIT_KEY_R_CONTROL & (automation_edit->key_mask)) != 0){
	  ags_automation_editor_paste(automation_editor);
	}
      }
      break;
    case GDK_KEY_x:
      {
	/* cut notes */
	if((AGS_AUTOMATION_EDIT_KEY_L_CONTROL & (automation_edit->key_mask)) != 0 || (AGS_AUTOMATION_EDIT_KEY_R_CONTROL & (automation_edit->key_mask)) != 0){
	  ags_automation_editor_cut(automation_editor);
	}
      }
      break;
    }
  }

  return(retval);
}

gboolean
ags_automation_edit_drawing_area_key_release_event(GtkWidget *widget, GdkEventKey *event,
						   AgsAutomationEdit *automation_edit)
{
  AgsAutomationEditor *automation_editor;
  AgsMachine *machine;

  GList *list, *list_start;
  
  double tact_factor, zoom_factor;
  double tact;
  guint x, y;
  gboolean retval;

  auto void ags_automation_edit_drawing_area_key_release_event_iterate(guint x, guint y,
								       gboolean position_cursor,
								       gboolean add_acceleration,
								       gboolean remove_acceleration);

  void ags_automation_edit_drawing_area_key_release_event_iterate(guint x, guint y,
								  gboolean position_cursor,
								  gboolean add_acceleration,
								  gboolean remove_acceleration){
    AgsNotebook *notebook;

    AgsAcceleration *current_acceleration, *acceleration;

    GList *list, *list_start;
    GList *automation;

    GType channel_type;

    gdouble gui_y, acceleration_y;

    gdouble val;
    gdouble upper, lower, range, step;
    gdouble c_upper, c_lower, c_range;

    guint n_attempts;
    guint i, n, m;
    guint line;
    gboolean is_audio, is_output, is_input;

    if(automation_edit == automation_editor->current_audio_automation_edit){
      notebook = NULL;
      channel_type = G_TYPE_NONE;

      is_audio = TRUE;
    }else if(automation_edit == automation_editor->current_output_automation_edit){
      notebook = automation_editor->current_output_notebook;
      channel_type = AGS_TYPE_OUTPUT;
      
      is_output = TRUE;
    }else if(automation_edit == automation_editor->current_input_automation_edit){
      notebook = automation_editor->current_input_notebook;
      channel_type = AGS_TYPE_INPUT;
      
      is_input = TRUE;
    }

    if(position_cursor){
      automation_edit->edit_x = x;
      automation_edit->edit_y = y;
    }
    
    /* find automation area */
    list =
      list_start = g_list_reverse(g_list_copy(automation_edit->automation_area));
    i = 0;
    
    while(list != NULL){
      if(i + AGS_AUTOMATION_AREA(list->data)->height > y){
	automation_edit->current_area = AGS_AUTOMATION_AREA(list->data);
	break;
      }

      i = i + AGS_AUTOMATION_AREA(list->data)->height + AGS_AUTOMATION_EDIT_DEFAULT_MARGIN;
      list = list->next;
    }

    g_list_free(list_start);

    if(list == NULL){
      return;
    }

    n_attempts = 3;

    /* match specifier */
    if(channel_type == G_TYPE_NONE){
      automation = automation_editor->selected_machine->audio->automation;

      while((automation = ags_automation_find_specifier_with_type_and_line(automation,
									   automation_edit->current_area->control_name,
									   channel_type,
									   0)) != NULL){
	upper = AGS_AUTOMATION(automation->data)->upper;
	lower = AGS_AUTOMATION(automation->data)->lower;
	
	range = upper - lower;

	if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	  c_upper = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					   upper,
					   FALSE);
	  c_lower = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					   lower,
					   FALSE);
	  c_range = c_upper - c_lower;
	}else{
	  c_upper = upper;
	  c_lower = lower;

	  c_range = range;
	}

	if(range == 0.0){
	  automation = automation->next;
	  g_warning("ags_automation_edit_callbacks.c - range = 0.0\0");
	
	  continue;
	}

	/* check steps */
	gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / automation_edit->current_area->height) * (gdouble) (y - i));
	acceleration_y = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	/* conversion */
	val = acceleration_y;

	if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	  val = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
				       val,
				       TRUE);
	}	

	/* add acceleration */
	if(add_acceleration){
	  /* find existing */
	  current_acceleration = ags_automation_find_point(AGS_AUTOMATION(automation->data),
							   x, val,
							   FALSE);

	  /* add acceleration */
	  if(current_acceleration == NULL){
	    acceleration = ags_acceleration_new();
	    acceleration->x = x;
	    acceleration->y = val;
      
	    ags_automation_add_acceleration(automation->data,
					    acceleration,
					    FALSE);
	  }
	}

	/* remove acceleration */
	if(remove_acceleration){
	  if(!ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
							     x, val)){
	  
	    for(m = 0; m < tact * n_attempts; m++){
	      for(n = 0; n < n_attempts; n++){
		if(m % 2 == 1){
		  if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								    x - (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		    goto remove_SUCCESS_0;
		  }
		}else{
		  if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								    x + (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		    goto remove_SUCCESS_0;
		  }
		}
	      }
	    }
	  }
	}

      remove_SUCCESS_0:		
	automation = automation->next;
      }
    }else{
      line = 0;
    
      while((line = ags_notebook_next_active_tab(notebook,
						 line)) != -1){
	automation = automation_editor->selected_machine->audio->automation;

	while((automation = ags_automation_find_specifier_with_type_and_line(automation,
									     automation_edit->current_area->control_name,
									     channel_type,
									     line)) != NULL){
	  upper = AGS_AUTOMATION(automation->data)->upper;
	  lower = AGS_AUTOMATION(automation->data)->lower;
	
	  range = upper - lower;

	  if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	    c_upper = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					     upper,
					     FALSE);
	    c_lower = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					     lower,
					     FALSE);
	    c_range = c_upper - c_lower;
	  }else{
	    c_upper = upper;
	    c_lower = lower;

	    c_range = range;
	  }

	  if(range == 0.0){
	    automation = automation->next;
	    g_warning("ags_automation_edit_callbacks.c - range = 0.0\0");
	
	    continue;
	  }

	  /* check steps */
	  gui_y = AGS_AUTOMATION(automation->data)->steps - round(((gdouble) AGS_AUTOMATION(automation->data)->steps / automation_edit->current_area->height) * (gdouble) (y - i));
	  acceleration_y = (gdouble) c_lower + (c_range / AGS_AUTOMATION(automation->data)->steps  * gui_y);

	  /* conversion */
	  val = acceleration_y;

	  if(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion != NULL){
	    val = ags_conversion_convert(AGS_PORT(AGS_AUTOMATION(automation->data)->port)->conversion,
					 val,
					 TRUE);
	  }

	  /* add acceleration */
	  if(add_acceleration){	
	    /* find existing */
	    current_acceleration = ags_automation_find_point(AGS_AUTOMATION(automation->data),
							     x, val,
							     FALSE);

	    /* add acceleration */
	    if(current_acceleration == NULL){
	      acceleration = ags_acceleration_new();
	      acceleration->x = x;
	      acceleration->y = val;
      
	      ags_automation_add_acceleration(automation->data,
					      acceleration,
					      FALSE);
	    }
	  }

	  /* remove acceleration */
	  if(remove_acceleration){
	    if(!ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
							       x, val)){
	  
	      for(m = 0; m < tact * n_attempts; m++){
		for(n = 0; n < n_attempts; n++){
		  if(m % 2 == 1){
		    if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								      x - (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		      goto remove_SUCCESS_1;
		    }
		  }else{
		    if(ags_automation_remove_acceleration_at_position(AGS_AUTOMATION(automation->data),
								      x + (m / 2), val - ((n % 2 == 1) ? -(n / 2): -1 * (n / 2)))){
		      goto remove_SUCCESS_1;
		    }
		  }
		}
	      }
	    }
	  }
	  
	remove_SUCCESS_1:	  
	  automation = automation->next;
	}

	line++;
      }
    }    
  }
    
  if(event->keyval == GDK_KEY_Tab ||
     event->keyval == GDK_ISO_Left_Tab ||
     event->keyval == GDK_KEY_Shift_L ||
     event->keyval == GDK_KEY_Shift_R ||
     event->keyval == GDK_KEY_Alt_L ||
     event->keyval == GDK_KEY_Alt_R ||
     event->keyval == GDK_KEY_Control_L ||
     event->keyval == GDK_KEY_Control_R ){
    retval = FALSE;
  }else{
    retval = TRUE;
  }

  automation_editor = (AgsAutomationEditor *) gtk_widget_get_ancestor(GTK_WIDGET(automation_edit),
								      AGS_TYPE_AUTOMATION_EDITOR);

  machine = automation_editor->selected_machine;
  
  if(machine == NULL){
    return(retval);
  }

  zoom_factor = 1.0 / 4.0;
  
  tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active(automation_editor->automation_toolbar->zoom));
  tact = exp2((double) gtk_combo_box_get_active(automation_editor->automation_toolbar->zoom) - 2.0);
  
  switch(event->keyval){
  case GDK_KEY_Control_L:
    {
      automation_edit->key_mask &= (~AGS_AUTOMATION_EDIT_KEY_L_CONTROL);
    }
    break;
  case GDK_KEY_Control_R:
    {
      automation_edit->key_mask &= (~AGS_AUTOMATION_EDIT_KEY_R_CONTROL);
    }
    break;
  case GDK_KEY_Shift_L:
    {
      automation_edit->key_mask &= (~AGS_AUTOMATION_EDIT_KEY_L_SHIFT);
    }
    break;
  case GDK_KEY_Shift_R:
    {
      automation_edit->key_mask &= (~AGS_AUTOMATION_EDIT_KEY_R_SHIFT);
    }
    break;
  case GDK_KEY_Left:
  case GDK_KEY_leftarrow:
    {
      if((AGS_AUTOMATION_EDIT_KEY_L_SHIFT & automation_edit->key_mask) != 0 ||
	 (AGS_AUTOMATION_EDIT_KEY_R_SHIFT & automation_edit->key_mask) != 0){
	x = automation_edit->edit_x + (1.0 * tact_factor);
	y = automation_edit->edit_y;
      }else{
	x = automation_edit->edit_x + (AGS_AUTOMATION_EDIT_DEFAULT_WIDTH * tact_factor);
	y = automation_edit->edit_y;
      }
      
      ags_automation_edit_drawing_area_key_release_event_iterate(x, y,
								 TRUE,
								 FALSE,
								 FALSE);
    }
    break;
  case GDK_KEY_Right:
  case GDK_KEY_rightarrow:
    {
      if((AGS_AUTOMATION_EDIT_KEY_L_SHIFT & automation_edit->key_mask) != 0 ||
	 (AGS_AUTOMATION_EDIT_KEY_R_SHIFT & automation_edit->key_mask) != 0){
	if(automation_edit->edit_x > (1.0 * tact_factor)){
	  x = automation_edit->edit_x - (1.0 * tact_factor);
	}else{
	  x = 0;
	}
	
	y = automation_edit->edit_y;
      }else{
	if(automation_edit->edit_x > (AGS_AUTOMATION_EDIT_DEFAULT_WIDTH * tact_factor)){
	  x = automation_edit->edit_x - (AGS_AUTOMATION_EDIT_DEFAULT_WIDTH * tact_factor);
	}else{
	  x = 0;
	}

	y = automation_edit->edit_y;
      }

      ags_automation_edit_drawing_area_key_release_event_iterate(x, y,
								 TRUE,
								 FALSE,
								 FALSE);
    }
    break;
  case GDK_KEY_Up:
  case GDK_KEY_uparrow:
    {
      list =
	list_start = g_list_reverse(g_list_copy(automation_edit->automation_area));

      /* current automation area */
      while(list != NULL){
	if(AGS_AUTOMATION_AREA(list->data)->y < automation_edit->edit_y &&
	   AGS_AUTOMATION_AREA(list->data)->y + AGS_AUTOMATION_AREA(list->data)->height > automation_edit->edit_y){
	  break;
	}
	  
	list = list->next;
      }

      if(list != NULL){      
	if((AGS_AUTOMATION_EDIT_KEY_L_CONTROL & automation_edit->key_mask) != 0 ||
	   (AGS_AUTOMATION_EDIT_KEY_R_CONTROL & automation_edit->key_mask) != 0){
	  /* goto prev available automation area */
	  if(list->prev != NULL){
	    y = AGS_AUTOMATION_AREA(list->prev->data)->y + ((AGS_AUTOMATION_AREA(list->data)->height / AGS_AUTOMATION_AREA(list->prev->data)->height) *
							    (automation_edit->edit_y - AGS_AUTOMATION_AREA(list->data)->y));
	  }
	}else{
	  /* move acceleration position y up */
	  if(AGS_AUTOMATION_AREA(list->data)->y + (AGS_AUTOMATION_AREA(list->data)->height / AGS_AUTOMATION_AREA_DEFAULT_PRECISION) > automation_edit->edit_y){
	    y = automation_edit->edit_y - (AGS_AUTOMATION_AREA(list->data)->height / AGS_AUTOMATION_AREA_DEFAULT_PRECISION);
	  }
	}

	ags_automation_edit_drawing_area_key_release_event_iterate(x, y,
								   TRUE,
								   FALSE,
								   FALSE);
      }

      g_list_free(list_start);
    }
    break;
  case GDK_KEY_Down:
  case GDK_KEY_downarrow:
    {
      guint tmp_y;
      
      list =
	list_start = g_list_reverse(g_list_copy(automation_edit->automation_area));

      /* current automation area */
      while(list != NULL){
	if(AGS_AUTOMATION_AREA(list->data)->y < automation_edit->edit_y &&
	   AGS_AUTOMATION_AREA(list->data)->y + AGS_AUTOMATION_AREA(list->data)->height > automation_edit->edit_y){
	  break;
	}
	  
	list = list->next;
      }

      if(list != NULL){
	if((AGS_AUTOMATION_EDIT_KEY_L_CONTROL & automation_edit->key_mask) != 0 ||
	   (AGS_AUTOMATION_EDIT_KEY_R_CONTROL & automation_edit->key_mask) != 0){
	  /* goto next available automation area */
	  if(list->next != NULL){
	    y = AGS_AUTOMATION_AREA(list->next->data)->y + ((AGS_AUTOMATION_AREA(list->data)->height / AGS_AUTOMATION_AREA(list->prev->data)->height) *
							    (automation_edit->edit_y - AGS_AUTOMATION_AREA(list->data)->y));
	  }
	}else{
	  /* move acceleration position y down */
	  if(AGS_AUTOMATION_AREA(list->data)->y + AGS_AUTOMATION_AREA(list->data)->height < automation_edit->edit_y + (AGS_AUTOMATION_AREA(list->data)->height / AGS_AUTOMATION_AREA_DEFAULT_PRECISION)){
	    y = automation_edit->edit_y + (AGS_AUTOMATION_AREA(list->data)->height / AGS_AUTOMATION_AREA_DEFAULT_PRECISION);
	  }
	}

	ags_automation_edit_drawing_area_key_release_event_iterate(x, y,
								   TRUE,
								   FALSE,
								   FALSE);
      }
      
      g_list_free(list_start);
    }
    break;
  case GDK_KEY_space:
    {
      ags_automation_edit_drawing_area_key_release_event_iterate(automation_edit->edit_x, automation_edit->edit_y,
								 TRUE,
								 TRUE,
								 FALSE);
    }
    break;
  case GDK_KEY_Delete:
    {
      ags_automation_edit_drawing_area_key_release_event_iterate(automation_edit->edit_x, automation_edit->edit_y,
								 TRUE,
								 FALSE,
								 TRUE);
    }
    break;
  }

  return(retval);
}

void
ags_automation_edit_vscrollbar_value_changed(GtkRange *range, AgsAutomationEdit *automation_edit)
{
  if((AGS_AUTOMATION_EDIT_RESETING_VERTICALLY & automation_edit->flags) != 0){
    return;
  }

  automation_edit->flags |= AGS_AUTOMATION_EDIT_RESETING_VERTICALLY;
  ags_automation_edit_reset_vertically(automation_edit, 0);
  automation_edit->flags &= (~AGS_AUTOMATION_EDIT_RESETING_VERTICALLY);
}

void
ags_automation_edit_hscrollbar_value_changed(GtkRange *range, AgsAutomationEdit *automation_edit)
{
  AgsAutomationEditor *automation_editor;

  double tact_factor, zoom_factor;
  double tact;

  if((AGS_AUTOMATION_EDIT_RESETING_HORIZONTALLY & automation_edit->flags) != 0){
    return;
  }

  automation_editor = (AgsAutomationEditor *) gtk_widget_get_ancestor(GTK_WIDGET(automation_edit),
								      AGS_TYPE_AUTOMATION_EDITOR);

  zoom_factor = 0.25;

  tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active(automation_editor->automation_toolbar->zoom));
  tact = exp2((double) gtk_combo_box_get_active(automation_editor->automation_toolbar->zoom) - 2.0);

  //  g_message("%f\0", GTK_RANGE(automation_edit->hscrollbar)->adjustment->value);
  
  /* reset ruler */
  gtk_adjustment_set_value(automation_edit->ruler->adjustment,
			   GTK_RANGE(automation_edit->hscrollbar)->adjustment->value / AGS_AUTOMATION_EDIT_DEFAULT_WIDTH);
  gtk_widget_queue_draw(automation_edit->ruler);

  /* update automation edit */
  automation_edit->flags |= AGS_AUTOMATION_EDIT_RESETING_HORIZONTALLY;
  ags_automation_edit_reset_horizontally(automation_edit, 0);
  automation_edit->flags &= (~AGS_AUTOMATION_EDIT_RESETING_HORIZONTALLY);
}
