/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
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

#ifndef INKB_H
#define INKB_H
//#ifdef HAVE_SDL
//#include <SDL/SDL_keysym.h>
//const int KEYMAP_SIZE =SDLK_LAST;
//const int KEY_SPECIAL_OFFSET=0;
//#else
const int KEYMAP_SIZE =256;
const int KEY_SPECIAL_OFFSET=128;
//#endif

enum KBSTATE {
	UP,
	DOWN,
	PRESS,
	RELEASE,
	RESET
};

typedef void (*KBHandler)(int, KBSTATE);

void ProcessKB(unsigned int player);
void BindKey(int key, unsigned int player, KBHandler handler);
void UnbindKey(int key);
void InitKB ();
void RestoreKB ();
#endif
