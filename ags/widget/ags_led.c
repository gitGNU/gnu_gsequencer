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

#include "ags_led.h"

#include <stdlib.h>
#include <math.h>

void ags_led_class_init(AgsLedClass *led);
void ags_led_init(AgsLed *led);

void ags_led_size_request(AgsLed *led,
			  GtkRequisition *requisition);
void ags_led_size_allocate(AgsLed *led,
			   GtkAllocation *allocation);
void ags_led_realize(GtkWidget *widget);
gint ags_led_expose(GtkWidget *widget,
		     GdkEventExpose *event);
gboolean ags_led_expose(GtkWidget *widget,
			 GdkEventExpose *event);
void ags_led_show(GtkWidget *widget);

void ags_led_draw(AgsLed *led);

/**
 * SECTION:ags_led
 * @short_description: A led widget
 * @title: AgsLed
 * @section_id:
 * @include: ags/widget/ags_led.h
 *
 * #AgsLed is a widget visualizing a #gboolean value.
 */

static gpointer ags_led_parent_class = NULL;

GtkStyle *led_style;

GType
ags_led_get_type(void)
{
  static GType ags_type_led = 0;

  if(!ags_type_led){
    static const GTypeInfo ags_led_info = {
      sizeof(AgsLedClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_led_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsLed),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_led_init,
    };

    ags_type_led = g_type_register_static(GTK_TYPE_BIN,
					  "AgsLed\0", &ags_led_info,
					  0);
  }

  return(ags_type_led);
}

void
ags_led_class_init(AgsLedClass *led)
{
  GtkWidgetClass *widget;

  ags_led_parent_class = g_type_class_peek_parent(led);

  widget = (GtkWidgetClass *) led;

  widget->size_request = ags_led_size_request;
  widget->size_allocate = ags_led_size_allocate;
  widget->realize = ags_led_realize;
  widget->expose_event = ags_led_expose;
  widget->show = ags_led_show;
}

void
ags_led_init(AgsLed *led)
{
  g_object_set(G_OBJECT(led),
	       "app-paintable\0", TRUE,
	       NULL);

  gtk_widget_set_style((GtkWidget *) led,
		       led_style);

  led->flags = 0;
}

void
ags_led_size_allocate(AgsLed *led,
		      GtkAllocation *allocation)
{
  GTK_WIDGET(led)->allocation = *allocation;  

  allocation->width = AGS_LED_DEFAULT_WIDTH;
  allocation->height = AGS_LED_DEFAULT_HEIGHT;
}

void
ags_led_size_request(AgsLed *led,
		     GtkRequisition *requisition)
{
  requisition->height = AGS_LED_DEFAULT_HEIGHT;
  requisition->width = AGS_LED_DEFAULT_WIDTH;
}

void
ags_led_realize(GtkWidget *widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (AGS_IS_LED (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  /*  */
  //TODO:JK: apply borders of container widgets
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) | 
    GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);

  widget->style = gtk_style_attach(widget->style, widget->window);

  gdk_window_set_user_data(widget->window, widget);

  gtk_style_set_background(widget->style, widget->window, GTK_STATE_ACTIVE);


  GTK_WIDGET_CLASS(ags_led_parent_class)->realize(widget);
}

void
ags_led_show(GtkWidget *widget)
{
  GTK_WIDGET_CLASS(ags_led_parent_class)->show(widget);
}

gboolean
ags_led_expose(GtkWidget *widget,
		GdkEventExpose *event)
{
  GTK_WIDGET_CLASS(ags_led_parent_class)->expose_event(widget, event);

  ags_led_draw(AGS_LED(widget));

  return(FALSE);
}

void
ags_led_draw(AgsLed *led)
{
  GtkWidget *widget;
  GdkWindow *window;

  GtkStyle *led_style;
  cairo_t *cr;
  
  static const gdouble white_gc = 65535.0;

  widget = GTK_WIDGET(led);
  led_style = gtk_widget_get_style(widget);
  
  cr = gdk_cairo_create(widget->window);

  /*  */
  if((AGS_LED_ACTIVE & (led->flags)) != 0){
    /* active */
    cairo_set_source_rgb(cr,
			 led_style->light[0].red / white_gc,
			 led_style->light[0].green / white_gc,
			 led_style->light[0].blue / white_gc);
  }else{
    /* normal */
    cairo_set_source_rgb(cr,
			 led_style->dark[0].red / white_gc,
			 led_style->dark[0].green / white_gc,
			 led_style->dark[0].blue / white_gc);
  }

  cairo_rectangle(cr,
		  widget->allocation.x, widget->allocation.y,
		  widget->allocation.width, widget->allocation.height);
  cairo_fill(cr);

  /* outline */
  cairo_set_source_rgb(cr,
		       led_style->fg[0].red / white_gc,
		       led_style->fg[0].green / white_gc,
		       led_style->fg[0].blue / white_gc);
  cairo_set_line_width(cr, 1.0);
  
  cairo_rectangle(cr,
		  widget->allocation.x, widget->allocation.y,
		  widget->allocation.width, widget->allocation.height);
  cairo_stroke(cr);

  /*  */
  cairo_destroy(cr);
}

void
ags_led_set_active(AgsLed *led)
{
  led->flags |= AGS_LED_ACTIVE;

  gtk_widget_queue_draw((GtkWidget *) led);

  //  ags_led_draw(led);
}

void
ags_led_unset_active(AgsLed *led)
{
  led->flags &= (~AGS_LED_ACTIVE);

  gtk_widget_queue_draw((GtkWidget *) led);

  //  ags_led_draw(led);
}

/**
 * ags_led_new:
 *
 * Creates an #AgsLed.
 *
 * Returns: a new #AgsLed
 *
 * Since: 0.4
 */
AgsLed*
ags_led_new()
{
  AgsLed *led;

  led = (AgsLed *) g_object_new(AGS_TYPE_LED,
				NULL);
  
  return(led);
}
