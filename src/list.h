/* 
 * Vega Strike 
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LIST_H
#define LIST_H

#include "vegastrike.h"
    
typedef void* list_t;
typedef void* list_elem_t;
typedef void* list_elem_data_t;

/* Create a new list */
list_t create_list(void);

/* Delete the list.  Deletes all nodes in the list (but doesn't
   free the data) */
void del_list(list_t llist);

/* Returns the first node in the list */
list_elem_t get_list_head(list_t llist);

/* Returns the last node in the list */
list_elem_t get_list_tail(list_t llist);

/* Returns the node after _node_ in the list, or NULL if node is the
   last node */
list_elem_t get_next_list_elem(list_t llist, list_elem_t node);

/* Returns the node before _node_ in the list, or NULL if node is the
   first node */
list_elem_t get_prev_list_elem(list_t llist, list_elem_t node);

/* Inserts a new node with data _new_data_ after _node_before_.  Pass
   NULL for _node_before_ to insert at start of list. 
   Returns the new node created by the insertion. */
list_elem_t insert_list_elem(list_t llist, list_elem_t node_before, 
			     list_elem_data_t new_data);

/* Deletes _node_ from the list */
list_elem_data_t delete_list_elem(list_t llist, list_elem_t node);

/* Returns the data stored in _node_ */
list_elem_data_t get_list_elem_data( list_elem_t node );

/* Debugging routine -- make sure the list data makes sense */
void list_sanity_check( list_t llist );

/* Debugging routine -- print the contents of the list */
void print_list( list_t llist );

#endif /* LIST_H */

#ifdef __cplusplus
} /* extern "C" */
#endif
