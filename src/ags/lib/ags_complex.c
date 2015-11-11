/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2015 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags/lib/ags_complex.h>

GType
ags_complex_get_type(void)
{
  static volatile gsize ags_define_type_id__volatile = 0;

  if(g_once_init_enter(&ags_define_type_id__volatile)){
    GType ags_define_type_id =
      g_boxed_type_register_static(g_intern_static_string ("AgsComplex\0"),
				   (GBoxedCopyFunc) ags_complex_copy,
				   (GBoxedFreeFunc) ags_complex_free);
    
    g_once_init_leave(&ags_define_type_id__volatile, ags_define_type_id);
  }
  
  return(ags_define_type_id__volatile);
}

AgsComplex*
ags_complex_alloc()
{
  AgsComplex *ptr;

  ptr = (AgsComplex *) malloc(sizeof(AgsComplex));
  
  *ptr[0] = 0;
  *ptr[1] = 0;

  return(ptr);
}

gpointer
ags_complex_copy(AgsComplex *z)
{
  AgsComplex *new_z;
  
  new_z = (AgsComplex *) malloc(sizeof(AgsComplex));
  
  *new_z[0] = *z[0];
  *new_z[1] = *z[1];

  return(new_z);
}

void
ags_complex_free(AgsComplex *z)
{
  g_free(z);
}

complex
ags_complex_get(AgsComplex *ptr)
{
  complex z;

  z = *ptr[0] + I * *ptr[1];

  return(z);
}

void
ags_complex_set(AgsComplex *ptr, complex z)
{
  *ptr[0] = creal(z);
  *ptr[1] = cimag(z);
}
