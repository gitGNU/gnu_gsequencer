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

#include <ags/X/task/ags_display_tact.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_navigation.h>

void ags_display_tact_class_init(AgsDisplayTactClass *display_tact);
void ags_display_tact_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_display_tact_init(AgsDisplayTact *display_tact);
void ags_display_tact_connect(AgsConnectable *connectable);
void ags_display_tact_disconnect(AgsConnectable *connectable);
void ags_display_tact_finalize(GObject *gobject);

void ags_display_tact_launch(AgsTask *task);

/**
 * SECTION:ags_display_tact
 * @short_description: display tact object
 * @title: AgsDisplayTact
 * @section_id:
 * @include: ags/audio/task/ags_display_tact.h
 *
 * The #AgsDisplayTact task displays #AgsTact and the GUI is updated.
 */

static gpointer ags_display_tact_parent_class = NULL;
static AgsConnectableInterface *ags_display_tact_parent_connectable_interface;

GType
ags_display_tact_get_type()
{
  static GType ags_type_display_tact = 0;

  if(!ags_type_display_tact){
    static const GTypeInfo ags_display_tact_info = {
      sizeof (AgsDisplayTactClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_display_tact_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsDisplayTact),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_display_tact_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_display_tact_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_display_tact = g_type_register_static(AGS_TYPE_TASK,
						   "AgsDisplayTact\0",
						   &ags_display_tact_info,
						   0);
    
    g_type_add_interface_static(ags_type_display_tact,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_display_tact);
}

void
ags_display_tact_class_init(AgsDisplayTactClass *display_tact)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_display_tact_parent_class = g_type_class_peek_parent(display_tact);

  /* gobject */
  gobject = (GObjectClass *) display_tact;

  gobject->finalize = ags_display_tact_finalize;

  /* task */
  task = (AgsTaskClass *) display_tact;

  task->launch = ags_display_tact_launch;
}

void
ags_display_tact_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_display_tact_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_display_tact_connect;
  connectable->disconnect = ags_display_tact_disconnect;
}

void
ags_display_tact_init(AgsDisplayTact *display_tact)
{
  display_tact->navigation = NULL;
}

void
ags_display_tact_connect(AgsConnectable *connectable)
{
  ags_display_tact_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_display_tact_disconnect(AgsConnectable *connectable)
{
  ags_display_tact_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_display_tact_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_display_tact_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_display_tact_launch(AgsTask *task)
{
  AgsDisplayTact *display_tact;
  AgsWindow *window;
  AgsNavigation *navigation;
  gchar *timestr;
  gdouble delay, tact;

  gdk_threads_enter();
  
  display_tact = AGS_DISPLAY_TACT(task);

  navigation = AGS_NAVIGATION(display_tact->navigation);
  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) navigation,
						 AGS_TYPE_WINDOW);

  navigation->note_offset = ags_soundcard_get_note_offset(AGS_SOUNDCARD(window->soundcard));
  tact = navigation->note_offset - navigation->start_tact;

  gdk_window_flush(GTK_WIDGET(navigation->duration_time)->window);
  timestr = ags_navigation_absolute_tact_to_time_string(tact,
							ags_soundcard_get_bpm(AGS_SOUNDCARD(window->soundcard)),
							ags_soundcard_get_delay_factor(AGS_SOUNDCARD(window->soundcard)));
  
  gtk_label_set_label(navigation->duration_time,
		      timestr);
  gtk_widget_queue_draw((GtkWidget *) navigation->duration_time);
  g_free(timestr);

  gdk_threads_leave();
}

/**
 * ags_display_tact_new:
 * @navigation: the #AgsNavigation to update
 *
 * Creates an #AgsDisplayTact.
 *
 * Returns: an new #AgsDisplayTact.
 *
 * Since: 0.4
 */
AgsDisplayTact*
ags_display_tact_new(GtkWidget *navigation)
{
  AgsDisplayTact *display_tact;

  display_tact = (AgsDisplayTact *) g_object_new(AGS_TYPE_DISPLAY_TACT,
						 NULL);

  display_tact->navigation = navigation;

  return(display_tact);
}
