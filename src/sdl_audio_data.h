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

#ifndef AUDIO_DATA_H
#define AUDIO_DATA_H

#include "vegastrike.h"

#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)
   
#include "SDL.h"
#include "SDL_mixer.h"

#else

typedef int Mix_Chunk;
typedef int Mix_Music;

#endif /* defined(HAVE_SDL) && defined(HAVE_SDL_MIXER) */

void init_audio_data();

/* SOUND */
bool_t load_sound( char *name, char *filename ) ;
bool_t get_sound_data( char *name, Mix_Chunk **data );
void incr_sound_data_ref_ctr( char *name ) ;
void decr_sound_data_ref_ctr( char *name ) ;
void delete_unused_audio_data();
bool_t is_sound_data_dirty();
void mark_sound_data_clean();
  //void register_sound_data_tcl_callbacks( Tcl_Interp *ip );

/* MUSIC */
bool_t load_music( char *name, char *filename ) ;
bool_t get_music_data( char *name, Mix_Music **data );
void set_music_playing_status( char *name, bool_t playing );
bool_t get_music_playing_status( char *name );
void incr_music_data_ref_ctr( char *name ) ;
void decr_music_data_ref_ctr( char *name ) ;
bool_t is_music_data_dirty();
void mark_music_data_clean();

#endif /* AUDIO_DATA_H */

#ifdef __cplusplus
} /* extern "C" */
#endif
