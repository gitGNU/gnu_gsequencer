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

#include <ags/X/editor/ags_pattern_edit.h>
#include <ags/X/editor/ags_pattern_edit_callbacks.h>

#include <ags/object/ags_connectable.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/recall/ags_count_beats_audio_run.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_editor.h>

#include <ags/X/editor/ags_pattern_edit.h>

#include <gdk/gdkkeysyms.h>

#include <atk/atk.h>

#include <math.h>

static GType ags_accessible_pattern_edit_get_type(void);
void ags_pattern_edit_class_init(AgsPatternEditClass *pattern_edit);
void ags_accessible_pattern_edit_class_init(AtkObject *object);
void ags_accessible_pattern_edit_action_interface_init(AtkActionIface *action);
void ags_pattern_edit_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_pattern_edit_init(AgsPatternEdit *pattern_edit);
void ags_pattern_edit_connect(AgsConnectable *connectable);
void ags_pattern_edit_disconnect(AgsConnectable *connectable);
AtkObject* ags_pattern_edit_get_accessible(GtkWidget *widget);

gboolean ags_accessible_pattern_edit_do_action(AtkAction *action,
					       gint i);
gint ags_accessible_pattern_edit_get_n_actions(AtkAction *action);
const gchar* ags_accessible_pattern_edit_get_description(AtkAction *action,
							 gint i);
const gchar* ags_accessible_pattern_edit_get_name(AtkAction *action,
						  gint i);
const gchar* ags_accessible_pattern_edit_get_keybinding(AtkAction *action,
							gint i);
gboolean ags_accessible_pattern_edit_set_description(AtkAction *action,
						     gint i);
gchar* ags_accessible_pattern_edit_get_localized_name(AtkAction *action,
						      gint i);

void ags_pattern_edit_paint(AgsPatternEdit *pattern_edit);

/**
 * SECTION:ags_pattern_edit
 * @short_description: edit notes
 * @title: AgsPatternEdit
 * @section_id:
 * @include: ags/X/editor/ags_pattern_edit.h
 *
 * The #AgsPatternEdit lets you edit notes.
 */

GtkStyle *pattern_edit_style;

static GQuark quark_accessible_object = 0;

GType
ags_pattern_edit_get_type(void)
{
  static GType ags_type_pattern_edit = 0;

  if(!ags_type_pattern_edit){
    static const GTypeInfo ags_pattern_edit_info = {
      sizeof (AgsPatternEditClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_pattern_edit_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPatternEdit),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_pattern_edit_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_pattern_edit_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_pattern_edit = g_type_register_static(GTK_TYPE_TABLE,
						"AgsPatternEdit\0", &ags_pattern_edit_info,
						0);
    
    g_type_add_interface_static(ags_type_pattern_edit,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_pattern_edit);
}

static GType
ags_accessible_pattern_edit_get_type(void)
{
  static GType ags_type_accessible_pattern_edit = 0;

  if(!ags_type_accessible_pattern_edit){
    const GTypeInfo ags_accesssible_pattern_edit_info = {
      sizeof(GtkAccessibleClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) ags_accessible_pattern_edit_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(GtkAccessible),
      0,             /* n_preallocs */
      NULL, NULL
    };

    static const GInterfaceInfo atk_action_interface_info = {
      (GInterfaceInitFunc) ags_accessible_pattern_edit_action_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_accessible_pattern_edit = g_type_register_static(GTK_TYPE_ACCESSIBLE,
							      "AgsAccessiblePatternEdit\0", &ags_accesssible_pattern_edit_info,
							      0);

    g_type_add_interface_static(ags_type_accessible_pattern_edit,
				ATK_TYPE_ACTION,
				&atk_action_interface_info);
  }
  
  return(ags_type_accessible_pattern_edit);
}

void
ags_pattern_edit_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_pattern_edit_connect;
  connectable->disconnect = ags_pattern_edit_disconnect;
}

void
ags_pattern_edit_class_init(AgsPatternEditClass *pattern_edit)
{
  quark_accessible_object = g_quark_from_static_string("ags-accessible-object\0");  
}

void
ags_accessible_pattern_edit_class_init(AtkObject *object)
{
  /* empty */
}

void
ags_accessible_pattern_edit_action_interface_init(AtkActionIface *action)
{
  action->do_action = ags_accessible_pattern_edit_do_action;
  action->get_n_actions = ags_accessible_pattern_edit_get_n_actions;
  action->get_description = ags_accessible_pattern_edit_get_description;
  action->get_name = ags_accessible_pattern_edit_get_name;
  action->get_keybinding = ags_accessible_pattern_edit_get_keybinding;
  action->set_description = ags_accessible_pattern_edit_set_description;
  action->get_localized_name = ags_accessible_pattern_edit_get_localized_name;
}

void
ags_pattern_edit_init(AgsPatternEdit *pattern_edit)
{
  GtkAdjustment *adjustment;

  pattern_edit->key_mask = 0;

  pattern_edit->ruler = ags_ruler_new();
  gtk_table_attach(GTK_TABLE(pattern_edit), (GtkWidget *) pattern_edit->ruler,
		   0, 1, 0, 1,
		   GTK_FILL|GTK_EXPAND, GTK_FILL,
		   0, 0);

  pattern_edit->drawing_area = (GtkDrawingArea *) gtk_drawing_area_new();
  gtk_widget_set_style((GtkWidget *) pattern_edit->drawing_area, pattern_edit_style);
  gtk_widget_set_events(GTK_WIDGET (pattern_edit->drawing_area), GDK_EXPOSURE_MASK
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK
			| GDK_CONTROL_MASK
			| GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK
			);
  gtk_widget_set_can_focus((GtkWidget *) pattern_edit->drawing_area,
			   TRUE);

  gtk_table_attach(GTK_TABLE(pattern_edit),
		   (GtkWidget *) pattern_edit->drawing_area,
		   0, 1,
		   1, 2,
		   GTK_FILL|GTK_EXPAND,
		   GTK_FILL|GTK_EXPAND,
		   0, 0);

  pattern_edit->control.note = ags_note_new();

  pattern_edit->width = 0;
  pattern_edit->height = 0;
  pattern_edit->map_width = AGS_PATTERN_EDIT_MAX_CONTROLS * 16 * 64;
  pattern_edit->map_height = 78;

  pattern_edit->control_height = 14;
  pattern_edit->control_margin_y = 2;

  pattern_edit->control_width = 16;

  pattern_edit->y0 = 0;
  pattern_edit->y1 = 0;

  pattern_edit->nth_y = 0;
  pattern_edit->stop_y = 0;

  /* AgsPatternEditControlCurrent is used by ags_pattern_edit_draw_segment */
  pattern_edit->control_current.control_count = AGS_PATTERN_EDIT_MAX_CONTROLS;
  pattern_edit->control_current.control_width = 64;

  pattern_edit->control_current.x0 = 0;
  pattern_edit->control_current.x1 = 0;

  pattern_edit->control_current.nth_x = 0;

  /* AgsPatternEditControlUnit is used by ags_pattern_edit_draw_notation */
  pattern_edit->control_unit.control_count = 16 * AGS_PATTERN_EDIT_MAX_CONTROLS;
  pattern_edit->control_unit.control_width = 1 * 4;

  pattern_edit->control_unit.x0 = 0;
  pattern_edit->control_unit.x1 = 0;

  pattern_edit->control_unit.nth_x = 0;
  pattern_edit->control_unit.stop_x = 0;

  /* offset for pasting from clipboard */
  pattern_edit->selected_x = 0;
  pattern_edit->selected_y = 0;

  /* GtkScrollbars */
  adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 1.0, 16.0, 1.0);
  pattern_edit->vscrollbar = (GtkVScrollbar *) gtk_vscrollbar_new(adjustment);
  gtk_table_attach(GTK_TABLE(pattern_edit), (GtkWidget *) pattern_edit->vscrollbar,
		   1, 2, 1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 1.0, (gdouble) pattern_edit->control_current.control_width, 1.0);
  pattern_edit->hscrollbar = (GtkHScrollbar *) gtk_hscrollbar_new(adjustment);
  gtk_table_attach(GTK_TABLE(pattern_edit), (GtkWidget *) pattern_edit->hscrollbar,
		   0, 1, 2, 3,
		   GTK_FILL, GTK_FILL,
		   0, 0);
}

void
ags_pattern_edit_connect(AgsConnectable *connectable)
{
  AgsEditor *editor;
  AgsPatternEdit *pattern_edit;

  pattern_edit = AGS_PATTERN_EDIT(connectable);

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);
  
  if(editor != NULL && editor->selected_machine != NULL){
    g_signal_connect_after(editor->selected_machine->audio, "set-audio-channels\0",
			   G_CALLBACK(ags_pattern_edit_set_audio_channels_callback), pattern_edit);
    g_signal_connect_after(editor->selected_machine->audio, "set-pads\0",
			   G_CALLBACK(ags_pattern_edit_set_pads_callback), pattern_edit);
  }
  
  g_signal_connect_after((GObject *) pattern_edit->drawing_area, "expose_event\0",
			 G_CALLBACK (ags_pattern_edit_drawing_area_expose_event), (gpointer) pattern_edit);

  g_signal_connect_after((GObject *) pattern_edit->drawing_area, "configure_event\0",
			 G_CALLBACK (ags_pattern_edit_drawing_area_configure_event), (gpointer) pattern_edit);

  g_signal_connect((GObject *) pattern_edit->drawing_area, "button_press_event\0",
		   G_CALLBACK (ags_pattern_edit_drawing_area_button_press_event), (gpointer) pattern_edit);

  g_signal_connect((GObject *) pattern_edit->drawing_area, "button_release_event\0",
		   G_CALLBACK (ags_pattern_edit_drawing_area_button_release_event), (gpointer) pattern_edit);

  g_signal_connect((GObject *) pattern_edit->drawing_area, "motion_notify_event\0",
		   G_CALLBACK (ags_pattern_edit_drawing_area_motion_notify_event), (gpointer) pattern_edit);
  			
  g_signal_connect((GObject *) pattern_edit->drawing_area, "key_press_event\0",
		   G_CALLBACK(ags_pattern_edit_drawing_area_key_press_event), (gpointer) pattern_edit);

  g_signal_connect((GObject *) pattern_edit->drawing_area, "key_release_event\0",
		   G_CALLBACK(ags_pattern_edit_drawing_area_key_release_event), (gpointer) pattern_edit);

  g_signal_connect_after((GObject *) pattern_edit->vscrollbar, "value-changed\0",
			 G_CALLBACK (ags_pattern_edit_vscrollbar_value_changed), (gpointer) pattern_edit);

  g_signal_connect_after((GObject *) pattern_edit->hscrollbar, "value-changed\0",
			 G_CALLBACK (ags_pattern_edit_hscrollbar_value_changed), (gpointer) pattern_edit);

}

void
ags_pattern_edit_disconnect(AgsConnectable *connectable)
{
  //TODO:JK: implement me
}

AtkObject*
ags_pattern_edit_get_accessible(GtkWidget *widget)
{
  AtkObject* accessible;

  accessible = g_object_get_qdata(G_OBJECT(widget),
				    quark_accessible_object);
  
  if(!accessible){
    accessible = g_object_new(ags_accessible_pattern_edit_get_type(),
			      NULL);
    
    g_object_set_qdata(G_OBJECT(widget),
		       quark_accessible_object,
		       accessible);
    gtk_accessible_set_widget(GTK_ACCESSIBLE(accessible),
			      widget);
  }
  
  return(accessible);
}

gboolean
ags_accessible_pattern_edit_do_action(AtkAction *action,
				      gint i)
{
  AgsPatternEdit *pattern_edit;
  
  GdkEventKey *key_press, *key_release;
  GdkEventKey *modifier_press, *modifier_release;
  
  if(!(i >= 0 && i < 11)){
    return(FALSE);
  }

  pattern_edit = gtk_accessible_get_widget(GTK_ACCESSIBLE(action));
  
  key_press = gdk_event_new(GDK_KEY_PRESS);
  key_release = gdk_event_new(GDK_KEY_RELEASE);

  /* create modifier */
  modifier_press = gdk_event_new(GDK_KEY_PRESS);
  modifier_release = gdk_event_new(GDK_KEY_RELEASE);
  
  modifier_press->keyval =
    modifier_release->keyval = GDK_KEY_Control_R;

  switch(i){
  case 0:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Left;
      
      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 1:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Right;
      
      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 2:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Up;
    
      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 3:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Down;
      
      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 4:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_space;
      
      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 5:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_Delete;
      
      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
    }
    break;
  case 6:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_c;

      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }    
    break;
  case 7:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_x;

      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }
    break;
  case 8:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_v;

      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }
    break;
  case 9:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_a;

      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }
    break;
  case 10:
    {
      key_press->keyval =
	key_release->keyval = GDK_KEY_i;

      /* send event */
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_press);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) key_release);
      gtk_widget_event((GtkWidget *) pattern_edit->drawing_area,
		       (GdkEvent *) modifier_release);      
    }
    break;
  }

  return(TRUE);
}

gint
ags_accessible_pattern_edit_get_n_actions(AtkAction *action)
{
  return(11);
}

const gchar*
ags_accessible_pattern_edit_get_description(AtkAction *action,
					    gint i)
{
  static const gchar **actions = {
    "move cursor left\0",
    "move cursor right\0",
    "move cursor up\0",
    "move cursor down\0",
    "add audio pattern\0",
    "remove audio pattern\0"
    "copy pattern to clipboard\0",
    "cut pattern to clipbaord\0",
    "paste pattern from clipboard\0",
    "select all pattern\0",
    "invert pattern\0",
  };

  if(i >= 0 && i < 11){
    return(actions[i]);
  }else{
    return(NULL);
  }
}

const gchar*
ags_accessible_pattern_edit_get_name(AtkAction *action,
				     gint i)
{
  static const gchar **actions = {
    "left\0",
    "right\0",
    "up\0",
    "down\0",
    "add\0",
    "remove\0",
    "copy\0",
    "cut\0",
    "paste\0",
    "select-all\0",
    "invert\0",
  };
  
  if(i >= 0 && i < 11){
    return(actions[i]);
  }else{
    return(NULL);
  }
}

const gchar*
ags_accessible_pattern_edit_get_keybinding(AtkAction *action,
					   gint i)
{
  static const gchar **actions = {
    "left\0",
    "right\0",
    "up\0",
    "down\0",
    "space",
    "Del\0",
    "Ctrl+c"
    "Ctrl+x",
    "Ctrl+v",
    "Ctrl+a",
    "Ctrl+i",
  };
  
  if(i >= 0 && i < 11){
    return(actions[i]);
  }else{
    return(NULL);
  }
}

gboolean
ags_accessible_pattern_edit_set_description(AtkAction *action,
					    gint i)
{
  //TODO:JK: implement me

  return(FALSE);
}

gchar*
ags_accessible_pattern_edit_get_localized_name(AtkAction *action,
					       gint i)
{
  //TODO:JK: implement me

  return(NULL);
}

/**
 * ags_pattern_edit_set_map_height:
 * @pattern_edit: the #AgsPatternEdit
 * @map_height: the new height
 *
 * Set the map height in pixel.
 *
 * Since: 0.4.2
 */
void
ags_pattern_edit_set_map_height(AgsPatternEdit *pattern_edit, guint map_height)
{
  pattern_edit->map_height = map_height;
  
  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_VERTICALLY;
  ags_pattern_edit_reset_vertically(pattern_edit, AGS_PATTERN_EDIT_RESET_VSCROLLBAR);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_VERTICALLY);
  
  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
  ags_pattern_edit_reset_horizontally(pattern_edit, AGS_PATTERN_EDIT_RESET_HSCROLLBAR);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);
}

/**
 * ags_pattern_edit_reset_vertically:
 * @pattern_edit: the #AgsPatternEdit
 * @flags: the #AgsPatternEditResetFlags
 *
 * Reset @pattern_edit as configured vertically.
 *
 * Since: 0.4.2
 */
void
ags_pattern_edit_reset_vertically(AgsPatternEdit *pattern_edit, guint flags)
{
  AgsEditor *editor;

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  if(editor->selected_machine != NULL){
    cairo_t *cr;
    gdouble value;

    value = GTK_RANGE(pattern_edit->vscrollbar)->adjustment->value;

    if((AGS_PATTERN_EDIT_RESET_VSCROLLBAR & flags) != 0){
      GtkWidget *widget;
      GtkAdjustment *adjustment;
      guint height;

      widget = GTK_WIDGET(pattern_edit->drawing_area);
      adjustment = GTK_RANGE(pattern_edit->vscrollbar)->adjustment;
      
      if(pattern_edit->map_height > widget->allocation.height){
	height = widget->allocation.height;
	gtk_adjustment_set_upper(adjustment,
				 (gdouble) (pattern_edit->map_height - height));

	if(adjustment->value > adjustment->upper){
	  gtk_adjustment_set_value(adjustment, adjustment->upper);
	}
      }else{
	height = pattern_edit->map_height;
	
	gtk_adjustment_set_upper(adjustment, 0.0);
	gtk_adjustment_set_value(adjustment, 0.0);
      }
      
      pattern_edit->height = height;
    }

    pattern_edit->y0 = ((guint) round((double) value)) % pattern_edit->control_height;

    if(pattern_edit->y0 != 0){
      pattern_edit->y0 = pattern_edit->control_height - pattern_edit->y0;
    }

    pattern_edit->y1 = (pattern_edit->height - pattern_edit->y0) % pattern_edit->control_height;

    pattern_edit->nth_y = (guint) ceil(round((double) value) / (double)(pattern_edit->control_height));
    pattern_edit->stop_y = pattern_edit->nth_y + (pattern_edit->height - pattern_edit->y0 - pattern_edit->y1) / pattern_edit->control_height;

    /* refresh display */
    if(GTK_WIDGET_VISIBLE(editor)){
      cr = gdk_cairo_create(GTK_WIDGET(pattern_edit->drawing_area)->window);

      cairo_surface_flush(cairo_get_target(cr));
      cairo_push_group(cr);

      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);

      if(editor->toolbar->selected_edit_mode == editor->toolbar->position){
	ags_pattern_edit_draw_position(pattern_edit, cr);
      }

      /* fader */
      if((AGS_PATTERN_EDIT_DRAW_FADER & (pattern_edit->flags)) != 0){
	AgsCountBeatsAudioRun *count_beats_audio_run;

	AgsMutexManager *mutex_manager;
	
	GList *recall;
	
	gdouble position;

	pthread_mutex_t *application_mutex;
	pthread_mutex_t *audio_mutex;

	mutex_manager = ags_mutex_manager_get_instance();
	application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

	/* lookup audio mutex */
	pthread_mutex_lock(application_mutex);
  
	audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) editor->selected_machine->audio);
  
	pthread_mutex_unlock(application_mutex);

	/* retrieve position */
	pthread_mutex_lock(audio_mutex);

	recall = editor->selected_machine->audio->play;

	while((recall = ags_recall_find_type(recall,
					     AGS_TYPE_COUNT_BEATS_AUDIO_RUN)) != NULL){
	  if(AGS_RECALL(recall->data)->recall_id != NULL && (AGS_RECALL_NOTATION & (AGS_RECALL(recall->data)->recall_id->flags)) != 0){
	    break;
	  }

	  recall = recall->next;
	}

	if(recall != NULL){
	  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(recall->data);

	  position = count_beats_audio_run->notation_counter * pattern_edit->control_unit.control_width;
	}
	
	pthread_mutex_unlock(audio_mutex);

	/* draw fader */
	if(recall != NULL){
	  ags_pattern_edit_draw_scroll(pattern_edit, cr,
				       position);
	}
      }

      cairo_pop_group_to_source(cr);
      cairo_paint(cr);

      cairo_surface_mark_dirty(cairo_get_target(cr));
      cairo_destroy(cr);
    }

    //    ags_meter_paint(editor->current_meter);
  }
}

/**
 * ags_pattern_edit_reset_horizontally:
 * @pattern_edit: the #AgsPatternEdit
 * @flags: the #AgsPatternEditResetFlags
 *
 * Reset @pattern_edit as configured horizontally.
 *
 * Since: 0.4.2
 */
void
ags_pattern_edit_reset_horizontally(AgsPatternEdit *pattern_edit, guint flags)
{
  AgsEditor *editor;
  double tact_factor, zoom_factor;
  double tact;
  gdouble value;

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  zoom_factor = 0.25;

  tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom));
  tact = exp2((double) gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom) - 2.0);

  if((AGS_PATTERN_EDIT_RESET_WIDTH & flags) != 0){
    pattern_edit->control_unit.control_width = (guint) (((double) pattern_edit->control_width * zoom_factor * tact));

    pattern_edit->control_current.control_count = (guint) ((double) pattern_edit->control_unit.control_count * tact);
    pattern_edit->control_current.control_width = (pattern_edit->control_width * zoom_factor * tact_factor * tact);

    pattern_edit->map_width = (guint) ((double) pattern_edit->control_current.control_count * (double) pattern_edit->control_current.control_width);

    /* reset ruler */
    pattern_edit->ruler->factor = tact_factor;
    pattern_edit->ruler->precision = tact;
    pattern_edit->ruler->scale_precision = 1.0 / tact;

    gtk_widget_queue_draw((GtkWidget *) pattern_edit->ruler);
  }

  value = GTK_RANGE(pattern_edit->hscrollbar)->adjustment->value;

  if((AGS_PATTERN_EDIT_RESET_HSCROLLBAR & flags) != 0){
    GtkWidget *widget;
    GtkAdjustment *adjustment;
    guint width;

    widget = GTK_WIDGET(pattern_edit->drawing_area);
    adjustment = GTK_RANGE(pattern_edit->hscrollbar)->adjustment;

    if(pattern_edit->map_width > widget->allocation.width){
      width = widget->allocation.width;
      //	gtk_adjustment_set_upper(adjustment, (double) (pattern_edit->map_width - width));
      gtk_adjustment_set_upper(adjustment,
			       (gdouble) (pattern_edit->map_width - width));
      gtk_adjustment_set_upper(pattern_edit->ruler->adjustment,
			       (gdouble) (pattern_edit->map_width - width) / pattern_edit->control_current.control_width);

      if(adjustment->value > adjustment->upper){
	gtk_adjustment_set_value(adjustment, adjustment->upper);

	/* reset ruler */
	gtk_adjustment_set_value(pattern_edit->ruler->adjustment, pattern_edit->ruler->adjustment->upper);
	gtk_widget_queue_draw((GtkWidget *) pattern_edit->ruler);
      }
    }else{
      width = pattern_edit->map_width;

      gtk_adjustment_set_upper(adjustment, 0.0);
      gtk_adjustment_set_value(adjustment, 0.0);
	
      /* reset ruler */
      gtk_adjustment_set_upper(pattern_edit->ruler->adjustment, 0.0);
      gtk_adjustment_set_value(pattern_edit->ruler->adjustment, 0.0);
      gtk_widget_queue_draw((GtkWidget *) pattern_edit->ruler);
    }

    pattern_edit->width = width;
  }

  /* reset AgsPatternEditControlCurrent */
  if(pattern_edit->map_width > pattern_edit->width){
    pattern_edit->control_current.x0 = ((guint) round((double) value)) % pattern_edit->control_current.control_width;

    if(pattern_edit->control_current.x0 != 0){
      pattern_edit->control_current.x0 = pattern_edit->control_current.control_width - pattern_edit->control_current.x0;
    }

    pattern_edit->control_current.x1 = (pattern_edit->width - pattern_edit->control_current.x0) % pattern_edit->control_current.control_width;

    pattern_edit->control_current.nth_x = (guint) ceil((double)(value) / (double)(pattern_edit->control_current.control_width));
  }else{
    pattern_edit->control_current.x0 = 0;
    pattern_edit->control_current.x1 = 0;
    pattern_edit->control_current.nth_x = 0;
  }

  /* reset AgsPatternEditControlUnit */
  if(pattern_edit->map_width > pattern_edit->width){
    pattern_edit->control_unit.x0 = ((guint)round((double) value)) % pattern_edit->control_unit.control_width;

    if(pattern_edit->control_unit.x0 != 0)
      pattern_edit->control_unit.x0 = pattern_edit->control_unit.control_width - pattern_edit->control_unit.x0;
      
    pattern_edit->control_unit.x1 = (pattern_edit->width - pattern_edit->control_unit.x0) % pattern_edit->control_unit.control_width;
      
    pattern_edit->control_unit.nth_x = (guint) ceil(round((double) value) / (double) (pattern_edit->control_unit.control_width));
    pattern_edit->control_unit.stop_x = pattern_edit->control_unit.nth_x + (pattern_edit->width - pattern_edit->control_unit.x0 - pattern_edit->control_unit.x1) / pattern_edit->control_unit.control_width;
  }else{
    pattern_edit->control_unit.x0 = 0;
    pattern_edit->control_unit.x1 = 0;
    pattern_edit->control_unit.nth_x = 0;
  }

  /* refresh display */
  if(editor->selected_machine != NULL &&
     editor->current_edit_widget == (GtkWidget *) pattern_edit){
    cairo_t *cr;

    if(GTK_WIDGET_VISIBLE(editor)){
      gdouble position;
      
      cr = gdk_cairo_create(GTK_WIDGET(pattern_edit->drawing_area)->window);

      cairo_surface_flush(cairo_get_target(cr));
      cairo_push_group(cr);

      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);

      if(editor->toolbar->selected_edit_mode == editor->toolbar->position){
	ags_pattern_edit_draw_position(pattern_edit, cr);
      }

      /* fader */
      if((AGS_PATTERN_EDIT_DRAW_FADER & (pattern_edit->flags)) != 0){
	AgsCountBeatsAudioRun *count_beats_audio_run;

	AgsMutexManager *mutex_manager;
	
	GList *recall;
	
	gdouble position;

	pthread_mutex_t *application_mutex;
	pthread_mutex_t *audio_mutex;

	mutex_manager = ags_mutex_manager_get_instance();
	application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
	
	/* lookup audio mutex */
	pthread_mutex_lock(application_mutex);
  
	audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					       (GObject *) editor->selected_machine->audio);
  
	pthread_mutex_unlock(application_mutex);

	/* retrieve position */
	pthread_mutex_lock(audio_mutex);

	recall = editor->selected_machine->audio->play;

	while((recall = ags_recall_find_type(recall,
					     AGS_TYPE_COUNT_BEATS_AUDIO_RUN)) != NULL){
	  if(AGS_RECALL(recall->data)->recall_id != NULL && (AGS_RECALL_NOTATION & (AGS_RECALL(recall->data)->recall_id->flags)) != 0){
	    break;
	  }

	  recall = recall->next;
	}

	if(recall != NULL){
	  count_beats_audio_run = AGS_COUNT_BEATS_AUDIO_RUN(recall->data);

	  position = count_beats_audio_run->notation_counter * pattern_edit->control_unit.control_width;
	}
	
	pthread_mutex_unlock(audio_mutex);

	/* draw fader */
	if(recall != NULL){
	  ags_pattern_edit_draw_scroll(pattern_edit, cr,
				       position);
	}
      }
      
      cairo_pop_group_to_source(cr);
      cairo_paint(cr);

      cairo_surface_mark_dirty(cairo_get_target(cr));
      cairo_destroy(cr);
    }
  }
}

/**
 * ags_pattern_edit_draw_segment:
 * @pattern_edit: the #AgsPatternEdit
 * @cr: the #cairo_t surface
 *
 * Draws horizontal and vertical lines.
 *
 * Since: 0.4.2
 */
void
ags_pattern_edit_draw_segment(AgsPatternEdit *pattern_edit, cairo_t *cr)
{
  AgsEditor *editor;
  GtkWidget *widget;

  GtkStyle *pattern_edit_style;
  
  double tact;
  guint i, j;
  guint j_set;

  static const gdouble white_gc = 65535.0;

  pattern_edit_style = gtk_widget_get_style(GTK_WIDGET(pattern_edit->drawing_area));
  
  widget = (GtkWidget *) pattern_edit->drawing_area;

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  /* clear with background color */
  cairo_set_source_rgb(cr,
		       pattern_edit_style->bg[0].red / white_gc,
		       pattern_edit_style->bg[0].green / white_gc,
		       pattern_edit_style->bg[0].blue / white_gc);
  
  cairo_rectangle(cr,
		  0.0, 0.0,
		  (double) widget->allocation.width, (double) widget->allocation.height);
  cairo_fill(cr);

  /* horizontal lines */
  cairo_set_line_width(cr, 1.0);

  cairo_set_source_rgb(cr,
		       pattern_edit_style->fg[0].red / white_gc,
		       pattern_edit_style->fg[0].green / white_gc,
		       pattern_edit_style->fg[0].blue / white_gc);

  for(i = pattern_edit->y0 ; i < pattern_edit->height;){
    cairo_move_to(cr, 0.0, (double) i);
    cairo_line_to(cr, (double) pattern_edit->width, (double) i);
    cairo_stroke(cr);

    i += pattern_edit->control_height;
  }

  cairo_move_to(cr,
		0.0, (double) i);
  cairo_line_to(cr,
		(double) pattern_edit->width, (double) i);
  cairo_stroke(cr);

  /* vertical lines */
  tact = exp2((double) gtk_combo_box_get_active(GTK_COMBO_BOX(editor->toolbar->zoom)) - 2.0);

  i = pattern_edit->control_current.x0;
  
  if(i < pattern_edit->width &&
     tact > 1.0 ){
    j_set = pattern_edit->control_current.nth_x % ((guint) tact);

    /* thin lines */
    cairo_set_source_rgb(cr,
		       pattern_edit_style->mid[0].red / white_gc,
		       pattern_edit_style->mid[0].green / white_gc,
		       pattern_edit_style->mid[0].blue / white_gc);

    if(j_set != 0){
      j = j_set;
      goto ags_pattern_edit_draw_segment0;
    }
  }

  for(; i < pattern_edit->width; ){
    /* strong lines */
    cairo_set_source_rgb(cr,
		       pattern_edit_style->fg[0].red / white_gc,
		       pattern_edit_style->fg[0].green / white_gc,
		       pattern_edit_style->fg[0].blue / white_gc);
    
    cairo_move_to(cr,
		  (double) i, 0.0);
    cairo_line_to(cr,
		  (double) i, (double) pattern_edit->height);
    cairo_stroke(cr);
    
    i += pattern_edit->control_current.control_width;
    
    /* thin lines */
    cairo_set_source_rgb(cr,
		       pattern_edit_style->mid[0].red / white_gc,
		       pattern_edit_style->mid[0].green / white_gc,
		       pattern_edit_style->mid[0].blue / white_gc);
    
    for(j = 1; i < pattern_edit->width && j < tact; j++){
    ags_pattern_edit_draw_segment0:
      cairo_move_to(cr,
		    (double) i, 0.0);
      cairo_line_to(cr,
		    (double) i, (double) pattern_edit->height);
      cairo_stroke(cr);
      
      i += pattern_edit->control_current.control_width;
    }
  }
}

/**
 * ags_pattern_edit_draw_position:
 * @pattern_edit: the #AgsPatternEdit
 * @cr: the #cairo_t surface
 *
 * Draws the cursor.
 *
 * Since: 0.4.2
 */
void
ags_pattern_edit_draw_position(AgsPatternEdit *pattern_edit, cairo_t *cr)
{
  GtkStyle *pattern_edit_style;
  
  guint selected_x, selected_y;
  guint x_offset[2], y_offset[2];
  guint x, y, width, height;
  gint size_width, size_height;

  static const gdouble white_gc = 65535.0;

  pattern_edit_style = gtk_widget_get_style(GTK_WIDGET(pattern_edit->drawing_area));
  
  selected_x = pattern_edit->selected_x * pattern_edit->control_unit.control_width;
  selected_y = pattern_edit->selected_y * pattern_edit->control_height;

  size_width = GTK_WIDGET(pattern_edit->drawing_area)->allocation.width;
  size_height = GTK_WIDGET(pattern_edit->drawing_area)->allocation.height;

  x_offset[0] = (guint) gtk_range_get_value(GTK_RANGE(pattern_edit->hscrollbar));
  x_offset[1] = x_offset[0] + (guint) size_width;

  y_offset[0] = (guint) gtk_range_get_value(GTK_RANGE(pattern_edit->vscrollbar));
  y_offset[1] = y_offset[0] + (guint) size_height;

  /* calculate horizontally values */
  if(selected_x < x_offset[0]){
    if(selected_x + pattern_edit->control_current.control_width > x_offset[0]){
      x = 0;
      width = selected_x + pattern_edit->control_current.control_width - x_offset[0];
    }else{
      return;
    }
  }else if(selected_x > x_offset[1]){
    return;
  }else{
    x = selected_x - x_offset[0];

    if(selected_x + pattern_edit->control_current.control_width < x_offset[1]){
      width = pattern_edit->control_current.control_width;
    }else{
      width = x_offset[1] - selected_x;
    }
  }

  /* calculate vertically values */
  if(selected_y < y_offset[0]){
    if(selected_y + pattern_edit->control_height > y_offset[0]){
      y = 0;
      height = selected_y + pattern_edit->control_height - y_offset[0];
    }else{
      return;
    }
  }else if(selected_y > y_offset[1]){
    return;
  }else{
    y = selected_y - y_offset[0];

    if(selected_y + pattern_edit->control_height < y_offset[1]){
      height = pattern_edit->control_height;
    }else{
      height = y_offset[1] - selected_y;
    }
  }

  /* draw */
  cairo_set_source_rgba(cr,
			pattern_edit_style->base[0].red / white_gc,
			pattern_edit_style->base[0].green / white_gc,
			pattern_edit_style->base[0].blue / white_gc,
			0.5);
  
  cairo_rectangle(cr,
		  (double) x, (double) y,
		  (double) width, (double) height);
  cairo_fill(cr);
}

/**
 * ags_pattern_edit_draw_notation:
 * @pattern_edit: the #AgsPatternEdit
 * @cr: the #cairo_t surface
 *
 * Draw the #AgsNotation of selected #AgsMachine on @pattern_edit.
 *
 * Since: 0.4.2
 */
void
ags_pattern_edit_draw_notation(AgsPatternEdit *pattern_edit, cairo_t *cr)
{
  AgsMachine *machine;
  AgsEditor *editor;
  GtkWidget *widget;

  GtkStyle *pattern_edit_style;
  AgsNote *note;

  AgsMutexManager *mutex_manager;

  GList *list_notation, *list_note;

  guint x_offset;
  guint control_height;
  guint x, y, width, height;
  gint selected_channel;
  gint i;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;

  static const gdouble white_gc = 65535.0;
  
  pattern_edit_style = gtk_widget_get_style(GTK_WIDGET(pattern_edit->drawing_area));
  
  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);
  
  if(editor->selected_machine == NULL ||
     (machine = editor->selected_machine) == NULL){
    return;
  }

  widget = (GtkWidget *) pattern_edit->drawing_area;
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* lookup audio mutex */
  pthread_mutex_lock(application_mutex);

  audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					 (GObject *) editor->selected_machine->audio);
  
  pthread_mutex_unlock(application_mutex);

  /* draw */
  pthread_mutex_lock(audio_mutex);

  cairo_set_source_rgb(cr,
		       pattern_edit_style->fg[0].red / white_gc,
		       pattern_edit_style->fg[0].green / white_gc,
		       pattern_edit_style->fg[0].blue / white_gc);

  i = 0;

  while((selected_channel = ags_notebook_next_active_tab(editor->current_notebook,
							 i)) != -1){
    list_notation = g_list_nth(machine->audio->notation,
			       selected_channel);

    if(list_notation == NULL){
      i++;
      continue;
    }
    
    list_note = AGS_NOTATION(list_notation->data)->notes;

    control_height = pattern_edit->control_height - 2 * pattern_edit->control_margin_y;

    x_offset = (guint) GTK_RANGE(pattern_edit->hscrollbar)->adjustment->value;

    /* draw controls smaller than pattern_edit->nth_x */
    while(list_note != NULL && (note = (AgsNote *) list_note->data)->x[0] < pattern_edit->control_unit.nth_x){
      if(note->x[1] >= pattern_edit->control_unit.nth_x){
	if(note->y >= pattern_edit->nth_y && note->y <= pattern_edit->stop_y){
	  x = 0;
	  y = (note->y - pattern_edit->nth_y) * pattern_edit->control_height + pattern_edit->y0 + pattern_edit->control_margin_y;

	  width = (guint) ((double) note->x[1] * pattern_edit->control_unit.control_width - (double) x_offset);

	  if(width > widget->allocation.width)
	    width = widget->allocation.width;

	  height = control_height;

	  /* draw note */
	  cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	  cairo_fill(cr);

	  /* check if note is selected */
	  if((AGS_NOTE_IS_SELECTED & (note->flags)) != 0){
	    cairo_set_source_rgba(cr,
				  pattern_edit_style->light[0].red / white_gc,
				  pattern_edit_style->light[0].green / white_gc,
				  pattern_edit_style->light[0].blue / white_gc,
				  0.7);
	    
	    cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	    cairo_stroke(cr);

	    cairo_set_source_rgb(cr,
				 pattern_edit_style->fg[0].red / white_gc,
				 pattern_edit_style->fg[0].green / white_gc,
				 pattern_edit_style->fg[0].blue / white_gc);	    
	  }
	}else if(note->y == (pattern_edit->nth_y - 1) && pattern_edit->y0 != 0){
	  if(pattern_edit->y0 > pattern_edit->control_margin_y){
	    x = 0;
	    width = (guint) ((double) note->x[1] * (double) pattern_edit->control_unit.control_width - x_offset);

	    if(width > widget->allocation.width)
	      width = widget->allocation.width;

	    if(pattern_edit->y0 > control_height + pattern_edit->control_margin_y){
	      y = pattern_edit->y0 - (control_height + pattern_edit->control_margin_y);
	      height = control_height;
	    }else{
	      y = 0;
	      height = pattern_edit->y0 - pattern_edit->control_margin_y;
	    }

	    /* draw note */
	    cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	    cairo_fill(cr);

	    /* check if note is selected */
	    if((AGS_NOTE_IS_SELECTED & (note->flags)) != 0){
	      cairo_set_source_rgba(cr,
				    pattern_edit_style->light[0].red / white_gc,
				    pattern_edit_style->light[0].green / white_gc,
				    pattern_edit_style->light[0].blue / white_gc,
				    0.7);
	    
	      cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	      cairo_stroke(cr);
	    
	      cairo_set_source_rgb(cr,
				   pattern_edit_style->fg[0].red / white_gc,
				   pattern_edit_style->fg[0].green / white_gc,
				   pattern_edit_style->fg[0].blue / white_gc);
	    }
	  }
	}else if(note->y == (pattern_edit->stop_y + 1) && pattern_edit->y1 != 0){
	  if(pattern_edit->y1 > pattern_edit->control_margin_y){
	    x = 0;
	    width = note->x[1] * pattern_edit->control_unit.control_width - x_offset;

	    if(width > widget->allocation.width)
	      width = widget->allocation.width;

	    y = (note->y - pattern_edit->nth_y) * pattern_edit->control_height + pattern_edit->control_margin_y;

	    if(pattern_edit->y1 > control_height + pattern_edit->control_margin_y){
	      height = control_height;
	    }else{
	      height = pattern_edit->y1 - pattern_edit->control_margin_y;
	    }

	    /* draw note */
	    cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	    cairo_fill(cr);
	  
	    /* check if note is selected */
	    if((AGS_NOTE_IS_SELECTED & (note->flags)) != 0){
	      cairo_set_source_rgba(cr,
				    pattern_edit_style->light[0].red / white_gc,
				    pattern_edit_style->light[0].green / white_gc,
				    pattern_edit_style->light[0].blue / white_gc,
				    0.7);
	    
	      cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	      cairo_stroke(cr);
	    
	      cairo_set_source_rgb(cr,
				   pattern_edit_style->fg[0].red / white_gc,
				   pattern_edit_style->fg[0].green / white_gc,
				   pattern_edit_style->fg[0].blue / white_gc);
	    }
	  }
	}
      }

      list_note = list_note->next;
    }

    /* draw controls equal or greater than pattern_edit->nth_x */
    while(list_note != NULL && (note = (AgsNote *) list_note->data)->x[0] <= pattern_edit->control_unit.stop_x){
      if(note->y >= pattern_edit->nth_y && note->y <= pattern_edit->stop_y){
	x = (guint) note->x[0] * pattern_edit->control_unit.control_width;
	y = (note->y - pattern_edit->nth_y) * pattern_edit->control_height +
	  pattern_edit->y0 +
	  pattern_edit->control_margin_y;

	width = note->x[1] * pattern_edit->control_unit.control_width - x;
	x -= x_offset;

	if(x + width > widget->allocation.width)
	  width = widget->allocation.width - x;

	height = control_height;

	/* draw note*/
	cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	cairo_fill(cr);

	/* check if note is selected */
	if((AGS_NOTE_IS_SELECTED & (note->flags)) != 0){
	  cairo_set_source_rgba(cr,
				pattern_edit_style->light[0].red / white_gc,
				pattern_edit_style->light[0].green / white_gc,
				pattern_edit_style->light[0].blue / white_gc,
				0.7);
	
	  cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	  cairo_stroke(cr);
	
	  cairo_set_source_rgb(cr,
			       pattern_edit_style->fg[0].red / white_gc,
			       pattern_edit_style->fg[0].green / white_gc,
			       pattern_edit_style->fg[0].blue / white_gc);
	}
      }else if(note->y == (pattern_edit->nth_y - 1) && pattern_edit->y0 != 0){
	if(pattern_edit->y0 > pattern_edit->control_margin_y){
	  x = note->x[0] * pattern_edit->control_unit.control_width - x_offset;
	  width = note->x[1] * pattern_edit->control_unit.control_width - x_offset - x;
      
	  if(x + width > widget->allocation.width)
	    width = widget->allocation.width - x;

	  if(pattern_edit->y0 > control_height + pattern_edit->control_margin_y){
	    y = pattern_edit->y0 - (control_height + pattern_edit->control_margin_y);
	    height = control_height;
	  }else{
	    y = 0;
	    height = pattern_edit->y0 - pattern_edit->control_margin_y;
	  }

	  /* draw note */
	  cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	  cairo_fill(cr);

	  /* check if note is selected */
	  if((AGS_NOTE_IS_SELECTED & (note->flags)) != 0){
	    cairo_set_source_rgba(cr,
				  pattern_edit_style->light[0].red / white_gc,
				  pattern_edit_style->light[0].green / white_gc,
				  pattern_edit_style->light[0].blue / white_gc,
				  0.7);

	    cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	    cairo_stroke(cr);

	    cairo_set_source_rgb(cr,
				 pattern_edit_style->fg[0].red / white_gc,
				 pattern_edit_style->fg[0].green / white_gc,
				 pattern_edit_style->fg[0].blue / white_gc);
	  }
	}
      }else if(note->y == (pattern_edit->stop_y + 1) && pattern_edit->y1 != 0){
	if(pattern_edit->y1 > pattern_edit->control_margin_y){
	  x = note->x[0] * pattern_edit->control_unit.control_width - x_offset;
	  width = note->x[1] * pattern_edit->control_unit.control_width - x_offset - x;
      
	  if(x + width > widget->allocation.width)
	    width = widget->allocation.width - x;

	  y = (note->y - pattern_edit->nth_y) * pattern_edit->control_height + pattern_edit->control_margin_y;

	  if(pattern_edit->y1 > control_height + pattern_edit->control_margin_y){
	    height = control_height;
	  }else{
	    height = pattern_edit->y1 - pattern_edit->control_margin_y;
	  }

	  /* draw note */
	  cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	  cairo_fill(cr);

	  /* check if note is selected */
	  if((AGS_NOTE_IS_SELECTED & (note->flags)) != 0){
	    cairo_set_source_rgba(cr,
				  pattern_edit_style->light[0].red / white_gc,
				  pattern_edit_style->light[0].green / white_gc,
				  pattern_edit_style->light[0].blue / white_gc,
				  0.7);

	    cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
	    cairo_stroke(cr);

	    cairo_set_source_rgb(cr,
				 pattern_edit_style->fg[0].red / white_gc,
				 pattern_edit_style->fg[0].green / white_gc,
				 pattern_edit_style->fg[0].blue / white_gc);
	  }
	}
      }

      list_note = list_note->next;
    }

    i++;
  }

  pthread_mutex_unlock(audio_mutex);
}

/**
 * ags_pattern_edit_draw_scroll:
 * @pattern_edit: the #AgsPatternEdit
 * @cr: the #cairo_t surface
 * @position: the new position
 *
 * Change visible x-position of @pattern_edit.
 *
 * Since: 0.4.2
 */
void
ags_pattern_edit_draw_scroll(AgsPatternEdit *pattern_edit, cairo_t *cr,
			     gdouble position)
{
  GtkStyle *pattern_edit_style;
  
  double x, y;
  double width, height;

  static const gdouble white_gc = 65535.0;

  pattern_edit_style = gtk_widget_get_style(GTK_WIDGET(pattern_edit->drawing_area));
  
  y = 0.0;
  x = (position) - (GTK_RANGE(pattern_edit->hscrollbar)->adjustment->value);

  height = (double) GTK_WIDGET(pattern_edit->drawing_area)->allocation.height;
  width = 3.0;

  /* draw */
  cairo_set_source_rgba(cr,
			pattern_edit_style->dark[0].red / white_gc,
			pattern_edit_style->dark[0].green / white_gc,
			pattern_edit_style->dark[0].blue / white_gc,
			0.5);
  cairo_rectangle(cr,
		  (double) x, (double) y,
		  (double) width, (double) height);
  cairo_fill(cr);
}

/**
 * ags_pattern_edit_new:
 *
 * Create a new #AgsPatternEdit.
 *
 * Returns: a new #AgsPatternEdit
 *
 * Since: 0.4.2
 */
AgsPatternEdit*
ags_pattern_edit_new()
{
  AgsPatternEdit *pattern_edit;

  pattern_edit = (AgsPatternEdit *) g_object_new(AGS_TYPE_PATTERN_EDIT, NULL);

  return(pattern_edit);
}

