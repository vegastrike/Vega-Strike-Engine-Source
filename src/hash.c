/* 
 * Vega Strike 
 * Copyright (C) 1999-2000 Daniel
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
 */

#include TCL_HEADER
#include "hash.h"

hash_table_t 
create_hash_table(void)
{
  Tcl_HashTable *table;
  table = (Tcl_HashTable*)malloc(sizeof(Tcl_HashTable));
  Tcl_InitHashTable(table, TCL_STRING_KEYS);
  return (hash_table_t)table;
}

void
add_hash_entry(hash_table_t table, char* name, hash_entry_t entry)
{
  Tcl_HashEntry *hash_entry;
  int           newEntry;

  hash_entry = Tcl_CreateHashEntry((Tcl_HashTable*)table,
                              name,
                              &newEntry);

  check_assertion( newEntry, "Hash entry already in table");

  Tcl_SetHashValue(hash_entry, entry);
}

bool_t 
get_hash_entry(hash_table_t table, char *name, hash_entry_t *result)
{
  Tcl_HashEntry *entry;
 
  entry = Tcl_FindHashEntry((Tcl_HashTable*)table, name);

  if(entry == 0)
	return False;

  if (result != NULL)
    *result = (hash_entry_t)Tcl_GetHashValue(entry);

  return True;
}                                                                               


bool_t 
del_hash_entry(hash_table_t table, char *name, hash_entry_t *result)
{
    Tcl_HashEntry *entry;

    entry = Tcl_FindHashEntry((Tcl_HashTable*)table, name);
    if (entry) {
	if (result != NULL)
	    *result = (hash_entry_t)Tcl_GetHashValue(entry);

	Tcl_DeleteHashEntry(entry);
	return True;
    }
    
    return False;
}

void
del_hash_table(hash_table_t table)
{

    Tcl_DeleteHashTable((Tcl_HashTable*)table);
    free(table);
}

typedef struct hash_scan_t {
  Tcl_HashSearch search;
  Tcl_HashEntry *last;
} hash_scan_t;


void
begin_hash_scan(hash_table_t table, hash_search_t *sptr)
{
  hash_scan_t *scanPtr;

  scanPtr = (hash_scan_t*)malloc(sizeof(hash_scan_t));

  *sptr = (hash_search_t)scanPtr;

  scanPtr->last = Tcl_FirstHashEntry((Tcl_HashTable*)table, &(scanPtr->search));

}


bool_t
next_hash_entry(hash_search_t sptr, char** key, hash_entry_t *value)
{
    hash_scan_t *scanPtr;
    scanPtr = (hash_scan_t*)sptr;
    if (scanPtr->last) {
	if (value != NULL) {
	    *value = (hash_entry_t)Tcl_GetHashValue(scanPtr->last);
	}
	if (key != NULL) {
	    *key = Tcl_GetHashKey(scanPtr->search.tablePtr, scanPtr->last);
	}
	scanPtr->last = Tcl_NextHashEntry(&(scanPtr->search));
	return True;
    }

    return False;
}


void
end_hash_scan(hash_search_t sptr)
{
  free(sptr);
}




