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

#ifndef AUDIO_H
#define AUDIO_H

#include "vegastrike.h"

void init_audio();

bool_t is_audio_open();

void bind_sounds_to_context( char *sound_context, char **names, int num_sounds );
bool_t play_sound( char *sound_context, int loop );
bool_t halt_sound( char *sound_context );
bool_t set_sound_volume( char *sound_context, int volume );
    

void bind_music_to_context( char *music_context, char *name, int loop );
bool_t play_music( char *music_context );
bool_t is_music_playing();
    
void update_audio();

  //void register_sound_tcl_callbacks( Tcl_Interp *ip );

#endif /* AUDIO_H */

#ifdef __cplusplus
} /* extern "C" */
#endif
