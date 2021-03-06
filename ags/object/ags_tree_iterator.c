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

#include <ags/object/ags_tree_iterator.h>

#include <stdio.h>

void ags_tree_iterator_base_init(AgsTreeIteratorInterface *interface);

/**
 * SECTION:ags_tree_iterator
 * @short_description: treehronous run
 * @title: AgsTreeIterator
 * @section_id: AgsTreeIterator
 * @include: ags/object/ags_tree_iterator.h
 *
 * The #AgsTreeIterator interface supports you iterating trees.
 */

GType
ags_tree_iterator_get_type()
{
  static GType ags_type_tree_iterator = 0;

  if(!ags_type_tree_iterator){
    static const GTypeInfo ags_tree_iterator_info = {
      sizeof(AgsTreeIteratorInterface),
      (GBaseInitFunc) ags_tree_iterator_base_init,
      NULL, /* base_finalize */
    };

    ags_type_tree_iterator = g_type_register_static(G_TYPE_INTERFACE,
						 "AgsTreeIterator\0", &ags_tree_iterator_info,
						 0);
  }

  return(ags_type_tree_iterator);
}

void
ags_tree_iterator_base_init(AgsTreeIteratorInterface *interface)
{
  /* empty */
}

void
ags_tree_iterator_set_inverse_mode(AgsTreeIterator *tree_iterator, gboolean mode)
{
  AgsTreeIteratorInterface *tree_iterator_interface;

  g_return_if_fail(AGS_IS_TREE_ITERATOR(tree_iterator));
  tree_iterator_interface = AGS_TREE_ITERATOR_GET_INTERFACE(tree_iterator);
  g_return_if_fail(tree_iterator_interface->set_inverse_mode);
  tree_iterator_interface->set_inverse_mode(tree_iterator, mode);
}

gboolean
ags_tree_iterator_is_inverse_mode(AgsTreeIterator *tree_iterator)
{
  AgsTreeIteratorInterface *tree_iterator_interface;

  g_return_val_if_fail(AGS_IS_TREE_ITERATOR(tree_iterator), FALSE);
  tree_iterator_interface = AGS_TREE_ITERATOR_GET_INTERFACE(tree_iterator);
  g_return_val_if_fail(tree_iterator_interface->is_inverse_mode, FALSE);
  tree_iterator_interface->is_inverse_mode(tree_iterator);
}

void
ags_tree_iterator_iterate(AgsTreeIterator *tree_iterator,
			  gpointer node_id)
{
  AgsTreeIteratorInterface *tree_iterator_interface;

  g_return_if_fail(AGS_IS_TREE_ITERATOR(tree_iterator));
  tree_iterator_interface = AGS_TREE_ITERATOR_GET_INTERFACE(tree_iterator);
  g_return_if_fail(tree_iterator_interface->iterate);
  tree_iterator_interface->iterate(tree_iterator,
				   node_id);
}

void
ags_tree_iterator_iterate_nested(AgsTreeIterator *tree_iterator,
				 gpointer node_id)
{
  AgsTreeIteratorInterface *tree_iterator_interface;

  g_return_if_fail(AGS_IS_TREE_ITERATOR(tree_iterator));
  tree_iterator_interface = AGS_TREE_ITERATOR_GET_INTERFACE(tree_iterator);
  g_return_if_fail(tree_iterator_interface->iterate_nested);
  tree_iterator_interface->iterate_nested(tree_iterator,
					  node_id);
}

void
ags_tree_iterator_safe_iterate(AgsTreeIterator *toplevel, AgsTreeIterator *current,
			       gpointer node_id)
{
  AgsTreeIteratorInterface *tree_iterator_interface;

  g_return_if_fail(AGS_IS_TREE_ITERATOR(toplevel));
  tree_iterator_interface = AGS_TREE_ITERATOR_GET_INTERFACE(toplevel);
  g_return_if_fail(tree_iterator_interface->iterate);
  tree_iterator_interface->safe_iterate(toplevel, current,
					node_id);
}

void
ags_tree_iterator_safe_iterate_nested(AgsTreeIterator *toplevel, AgsTreeIterator *current,
				      gpointer node_id)
{
  AgsTreeIteratorInterface *tree_iterator_interface;

  g_return_if_fail(AGS_IS_TREE_ITERATOR(toplevel));
  tree_iterator_interface = AGS_TREE_ITERATOR_GET_INTERFACE(toplevel);
  g_return_if_fail(tree_iterator_interface->iterate);
  tree_iterator_interface->safe_iterate_nested(toplevel, current,
					       node_id);
}
