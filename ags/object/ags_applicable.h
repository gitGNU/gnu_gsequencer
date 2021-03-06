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

#ifndef __AGS_APPLICABLE_H__
#define __AGS_APPLICABLE_H__

#include <glib.h>
#include <glib-object.h>

#define AGS_TYPE_APPLICABLE                    (ags_applicable_get_type())
#define AGS_APPLICABLE(obj)                    (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_APPLICABLE, AgsApplicable))
#define AGS_APPLICABLE_INTERFACE(vtable)       (G_TYPE_CHECK_CLASS_CAST((vtable), AGS_TYPE_APPLICABLE, AgsApplicableInterface))
#define AGS_IS_APPLICABLE(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_APPLICABLE))
#define AGS_IS_APPLICABLE_INTERFACE(vtable)    (G_TYPE_CHECK_CLASS_TYPE((vtable), AGS_TYPE_APPLICABLE))
#define AGS_APPLICABLE_GET_INTERFACE(obj)      (G_TYPE_INSTANCE_GET_INTERFACE((obj), AGS_TYPE_APPLICABLE, AgsApplicableInterface))

typedef struct _AgsApplicable AgsApplicable;
typedef struct _AgsApplicableInterface AgsApplicableInterface;

struct _AgsApplicableInterface
{
  GTypeInterface interface;
  
  void (*set_update)(AgsApplicable *applicable, gboolean update);
  void (*apply)(AgsApplicable *applicable);
  void (*reset)(AgsApplicable *applicable);
};

GType ags_applicable_get_type();

void ags_applicable_set_update(AgsApplicable *applicable, gboolean update);
void ags_applicable_apply(AgsApplicable *applicable);
void ags_applicable_reset(AgsApplicable *applicable);

#endif /*__AGS_APPLICABLE_H__*/
