/* 
 * Vega Strike 
 * Copyright (C) 1999-2000 Daniel Horn
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

#ifndef _HASH_H
#define _HASH_H

#include "vegastrike.h"

typedef void* hash_table_t;
typedef void* hash_entry_t;
typedef void* hash_search_t;

hash_table_t create_hash_table(void);
void         del_hash_table(hash_table_t table);

void         add_hash_entry(hash_table_t table, char* name, hash_entry_t entry);
bool_t       get_hash_entry(hash_table_t table, char *name, 
			    hash_entry_t *result);
bool_t       del_hash_entry(hash_table_t table, char *name,
			    hash_entry_t *result);

void	     begin_hash_scan(hash_table_t table, 
			     hash_search_t *sptr);
bool_t       next_hash_entry(hash_search_t sptr, 
			     char **key,
			     hash_entry_t *result);
void         end_hash_scan(hash_search_t sptr);

#endif /* _HASH_H */

#ifdef __cplusplus
} /* extern "C" */
#endif
