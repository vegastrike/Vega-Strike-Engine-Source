/* 
 * Vega Strike 
 * Copyright (C) 2001 2002 Daniel Horn
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

#include "vegastrike.h"
#include "sdl_audio_data.h"
#include "sdl_audio.h"

#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)

#include "SDL.h"
#include "SDL_mixer.h"
#include "hash.h"
#include "list.h"

#define SOUNDFILE_MISSING_WARNING_LEVEL 10
#define SOUNDFILE_BUSY_WARNING_LEVEL 10

typedef struct {
    Mix_Chunk *data;    /* sound data */
    int ref_ctr;        /* reference counter */
} sound_record_t;

typedef struct {
    Mix_Music *data;    /* music data */
    int ref_ctr;        /* reference counter */
    bool_t playing;     /* is this music playing? */
} music_record_t;

static hash_table_t sound_hash_;
static hash_table_t music_hash_;
static bool_t initialized_ = False;
static bool_t sound_dirty_ = False;
static bool_t music_dirty_ = False;


/* Name:          init_sound_data
   Description:   initializes audio module
   Precondition:  none
   Postcondition: audio module initialized
   Return value:  none
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void 
init_audio_data()
{
    if (!initialized_) {
	sound_hash_ = create_hash_table();
	check_assertion( sound_hash_ != NULL, "sound_hash_ is null" );

	music_hash_ = create_hash_table();
	check_assertion( music_hash_ != NULL, "music_hash_ is null" );
	initialized_ = True;
    }
}


/* Name:          load_sound_or_music

   Description: Load a sound or music file.  Since the actions
   required to load a sound file are so similar to the actions
   required to load a music file, we do it in the same function so we
   can share code

   Precondition:  name, filename != NULL
   Return value:  True if load successful, False otherwise
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 200-08-13 
*/
static bool_t load_sound_or_music( char *name, char *filename, 
				   bool_t load_music )
{
    char *data_ptr;
    char *record_ptr;
    char *temp_record_ptr;
    hash_table_t hash;
    char *desc;
    int ref_ctr = 0;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );
    check_assertion( filename != NULL, "null filename" );

    if ( ! is_audio_open() ) {
	return False;
    }

    if ( load_music ) {
	hash = music_hash_;
	desc = "music";
    } else {
	hash = sound_hash_;
	desc = "sound";
    }

    if ( get_hash_entry( hash, name, (hash_entry_t*) &record_ptr ) ) {
	print_debug( DEBUG_SOUND, "Overwriting %s name %s", desc, name );

	/* Need to save ref_ctr */
	if ( load_music ) {
	    music_record_t *mrec = (music_record_t*)record_ptr;
	    if ( mrec->playing ) {
		/* Can't overwrite since music is playing */
		print_warning( SOUNDFILE_BUSY_WARNING_LEVEL, 
			       "Can't overwrite %s name %s since "
			       "it is playing", desc, name );
		return False;
	    }
	    ref_ctr = mrec->ref_ctr;
	} else {
	    sound_record_t *srec = (sound_record_t*)record_ptr;
	    ref_ctr = srec->ref_ctr;
	}

	del_hash_entry( hash, name, NULL );
    }

    if ( load_music ) {
	data_ptr = (char*) Mix_LoadMUS( filename );
    } else {
	data_ptr = (char*) Mix_LoadWAV( filename );
    }

    if ( data_ptr == NULL ) {
	print_warning( SOUNDFILE_MISSING_WARNING_LEVEL, 
		       "FAILED to load %s file %s: %s", 
		       desc, filename, Mix_GetError() );
	return False;
    }

    print_debug( DEBUG_SOUND, "Successfully loaded %s file %s", 
		 desc, filename );


    if ( load_music ) {
	music_record_t *mrec = (music_record_t*)malloc(sizeof(music_record_t));
	mrec->data = (Mix_Music*) data_ptr;
	mrec->ref_ctr = ref_ctr;
	mrec->playing = False;

	record_ptr = (char*) mrec;
    } else {
	sound_record_t *srec = (sound_record_t*)malloc(sizeof(sound_record_t));
	srec->data = (Mix_Chunk*) data_ptr;
	srec->ref_ctr = ref_ctr;

	record_ptr = (char*) srec;
    }

    add_hash_entry( hash, name, (hash_entry_t) record_ptr );

    /* Make sure it's there */
    check_assertion( get_hash_entry( hash, name, (void*) &temp_record_ptr ) &&
		     ( record_ptr == temp_record_ptr ), 
		     "failed addition to hash table" );

    if ( load_music ) {
	music_dirty_ = True;
    } else {
	sound_dirty_ = True;
    }

    return True;
}

/* Name:          load_sound
   Description:   Loads a sound from a file
   Precondition:  name, filename != null
   Return value:  True if successful, false otherwise
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool_t 
load_sound( char *name, char *filename ) 
{ 
    check_assertion( initialized_, "audio_data module not initialized" );

    return load_sound_or_music( name, filename, False );
}

/* Name:          load_music
   Description:   Loads music from a file
   Precondition:  name, filename != null
   Return value:  True if successful, false otherwise
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool_t 
load_music( char *name, char *filename ) 
{ 
    check_assertion( initialized_, "audio_data module not initialized" );

    return load_sound_or_music( name, filename, True );
}


/* Name:          get_sound_data
   Description:   returns the Mix_Chunk for the sound _name_
   Precondition:  name != NULL
   Return value:  True if entry for name exists, false otherwise
   Modified args: data if non-NULL
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool_t
get_sound_data( char *name, Mix_Chunk **data )
{
    sound_record_t *srec;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    if (!get_hash_entry( sound_hash_, name, (hash_entry_t*) &srec )) {
	return False;
    }

    if ( data ) {
	*data = srec->data;
    }
    return True;
}

/* Name:          get_music_data
   Description:   returns the Mix_Music for the music _name_
   Precondition:  name != NULL
   Return value:  True if entry for name exists, false otherwise
   Modified args: data if non-NULL
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool_t
get_music_data( char *name, Mix_Music **data )
{
    music_record_t *mrec;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    if (!get_hash_entry( music_hash_, name, (hash_entry_t*) &mrec )) {
	return False;
    }

    if ( data ) {
	*data = mrec->data;
    }
    return True;
}


/*! 

  Set the "playing" status of the music data (if music is marked as
  playing it can't be deleted).

  \brief   brief_desc
  \pre     name != NULL, named music exists 
  \arg \c  name name of music
  \arg \c  playing the new value for the playing status

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-14
  \date Modified: 2000-08-14 
*/
void
set_music_playing_status( char *name, bool_t playing )
{
    music_record_t *mrec;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    if (!get_hash_entry( music_hash_, name, (hash_entry_t*) &mrec )) {
	check_assertion(0, "couldn't find music" );
    }

    mrec->playing = playing;
}


/*! 
  Get the "playing" status of the music data
  \pre     name != NULL, named music exists
  \arg \c  name name of music

  \return  playing status of music
  \author  jfpatry
  \date    Created:  2000-08-14
  \date    Modified: 2000-08-14
*/
bool_t
get_music_playing_status( char *name )
{
    music_record_t *mrec;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    if (!get_hash_entry( music_hash_, name, (hash_entry_t*) &mrec )) {
	check_assertion(0, "couldn't find music" );
    }

    return mrec->playing;
}


/* Name:          incr_sound_data_ref_ctr
   Description:   Increments the reference count for sound _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void incr_sound_data_ref_ctr( char *name ) 
{
    sound_record_t *srec;
    bool_t found;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    found = get_hash_entry( sound_hash_, name, (hash_entry_t*) &srec );
    check_assertion( found, "hashtable entry not found" );

    srec->ref_ctr += 1;

    print_debug( DEBUG_SOUND, "incremented reference counter of sound %s "
		 "to %d", name, srec->ref_ctr );
}


/* Name:          decr_sound_data_ref_ctr
   Description:   Decrements the reference count for sound _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void decr_sound_data_ref_ctr( char *name ) 
{
    sound_record_t *srec;
    bool_t found;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    found = get_hash_entry( sound_hash_, name, (hash_entry_t*) &srec );
    check_assertion( found, "hashtable entry not found" );

    srec->ref_ctr -= 1;

    print_debug( DEBUG_SOUND, "decremented reference counter of sound %s "
		 "to %d", name, srec->ref_ctr );
}

/* Name:          incr_music_data_ref_ctr
   Description:   Increments the reference count for music _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void incr_music_data_ref_ctr( char *name ) 
{
    music_record_t *mrec;
    bool_t found;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    found = get_hash_entry( music_hash_, name, (hash_entry_t*) &mrec );
    check_assertion( found, "hashtable entry not found" );

    mrec->ref_ctr += 1;

    print_debug( DEBUG_SOUND, "incremented reference counter of music %s "
		 "to %d", name, mrec->ref_ctr );
}

/* Name:          decr_music_data_ref_ctr
   Description:   Decrements the reference count for music _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void decr_music_data_ref_ctr( char *name ) 
{
    music_record_t *mrec;
    bool_t found;

    check_assertion( initialized_, "audio_data module not initialized" );

    check_assertion( name != NULL, "null name" );

    found = get_hash_entry( music_hash_, name, (hash_entry_t*) &mrec );
    check_assertion( found, "hashtable entry not found" );

    mrec->ref_ctr -= 1;

    print_debug( DEBUG_SOUND, "decremented reference counter of music %s "
		 "to %d", name, mrec->ref_ctr );
}

/* Name:          delete_unused_audio_data
   Description:   Frees all sound and music data with reference counts == 0
   Precondition:  
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void delete_unused_audio_data()
{
    hash_search_t iter;
    sound_record_t *srec;
    music_record_t *mrec;
    char *key;
    list_t keys_to_delete;
    list_elem_t elem;
    bool_t found;

    check_assertion( initialized_, "audio_data module not initialized" );

    /* clean sounds first */
    keys_to_delete = create_list();

    begin_hash_scan( sound_hash_, &iter );

    while ( next_hash_entry( iter, &key, (hash_entry_t*) &srec ) ) {
	if ( srec->ref_ctr == 0 ) {
	    /* save key; we'll delete later */
	    insert_list_elem( keys_to_delete, NULL, key );
	    Mix_FreeChunk( srec->data );
	}
    }

    end_hash_scan( iter );

    for (elem = get_list_head( keys_to_delete ); elem != NULL;
	 elem = get_next_list_elem( keys_to_delete, elem))
    {
	key = (char*) get_list_elem_data( elem );
	found = del_hash_entry( sound_hash_, key, NULL );
	check_assertion( found, "key disappeared!" );
    }

    del_list( keys_to_delete );

    /* now clean music */
    keys_to_delete = create_list();

    begin_hash_scan( music_hash_, &iter );

    while ( next_hash_entry( iter, &key, (hash_entry_t*) &mrec ) ) {
	if ( mrec->ref_ctr == 0 ) {
	    /* we shouldn't be playing music with ref cnt of 0 */
	    check_assertion( mrec->playing == False, 
			     "playing music with reference count of 0" );

	    /* save key; we'll delete later */
	    insert_list_elem( keys_to_delete, NULL, key );
	    Mix_FreeMusic( mrec->data );
	}
    }

    end_hash_scan( iter );

    for (elem = get_list_head( keys_to_delete ); elem != NULL;
	 elem = get_next_list_elem( keys_to_delete, elem))
    {
	key = (char*) get_list_elem_data( elem );
	found = del_hash_entry( music_hash_, key, NULL );
	check_assertion( found, "key disappeared!" );
    }

    del_list( keys_to_delete );
}


/* Name:          is_sound_data_dirty

   Description: Returns true if new sound data has been loaded since
   last call to mark_sound_data_clean()

   Precondition:  
   Return value:  True iff sound data dirty
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool_t
is_sound_data_dirty()
{
    check_assertion( initialized_, "audio_data module not initialized" );

    return sound_dirty_;
}

/* Name:          is_music_data_dirty

   Description: Returns true if new music data has been loaded since
   last call to mark_music_data_clean()

   Precondition:  
   Return value:  True iff music data dirty
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool_t
is_music_data_dirty()
{
    check_assertion( initialized_, "audio_data module not initialized" );

    return music_dirty_;
}


/* Name:          mark_sound_data_clean
   Description:   Mark sound data as clean
   Precondition:  
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void
mark_sound_data_clean()
{
    check_assertion( initialized_, "audio_data module not initialized" );

    sound_dirty_ = False;
}


/* Name:          mark_music_data_clean
   Description:   Mark music data as clean
   Precondition:  
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void
mark_music_data_clean()
{
    check_assertion( initialized_, "audio_data module not initialized" );

    music_dirty_ = False;
}

#else

void 
init_audio_data()
{
}

bool_t 
load_sound( char *name, char *filename ) 
{ 
    return True;
}

bool_t 
load_music( char *name, char *filename ) 
{ 
    return True;
}

bool_t
get_sound_data( char *name, Mix_Chunk **data )
{
    return False;
}

bool_t
get_music_data( char *name, Mix_Music **data )
{
    return False;
}

void
set_music_playing_status( char *name, bool_t playing )
{
}


bool_t
get_music_playing_status( char *name )
{
    return False;
}

void incr_sound_data_ref_ctr( char *name ) 
{
}

void decr_sound_data_ref_ctr( char *name ) 
{
}

void incr_music_data_ref_ctr( char *name ) 
{
}

void decr_music_data_ref_ctr( char *name ) 
{
}
 
void delete_unused_audio_data()
{
}

bool_t
is_sound_data_dirty()
{
    return False;
}

bool_t
is_music_data_dirty()
{
    return False;
}

void
mark_sound_data_clean()
{
}

void
mark_music_data_clean()
{
}


static int load_sound_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, char *argv[]) 
{
    Tcl_Obj *result;

    result = Tcl_NewBooleanObj( 1 );
    Tcl_SetObjResult( ip, result );
    return TCL_OK;
} 

static int load_music_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, char *argv[]) 
{
    Tcl_Obj *result;

    result = Tcl_NewBooleanObj( 1 );
    Tcl_SetObjResult( ip, result );
    return TCL_OK;
} 


void register_sound_data_tcl_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_load_sound", load_sound_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_load_music", load_music_cb,  0,0);
}

#endif /* defined(HAVE_SDL) && defined(HAVE_SDL_MIXER) */
