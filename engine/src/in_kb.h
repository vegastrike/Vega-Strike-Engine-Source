/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INKB_H
#define INKB_H
//#ifdef HAVE_SDL
//#include <SDL/SDL_keysym.h>
//const int KEYMAP_SIZE =SDLK_LAST;
//const int KEY_SPECIAL_OFFSET=0;
//#else

enum KB_MODIFIER_ENUM {
    KB_MOD_ALT = 1,
    KB_MOD_CTRL = 2,
    KB_MOD_SHIFT = 4,
    KB_MOD_MASK = 7
};

#ifndef NO_GFX
#include "gldrv/winsys.h"
const int KEYMAP_SIZE = WSK_LAST;
const int LAST_MODIFIER = KB_MOD_MASK + 1;
#else
const int KEYMAP_SIZE   = 0;
const int LAST_MODIFIER = 0;
#endif
//#endif
#include "in.h"

unsigned int getActiveModifiers();
unsigned int pullActiveModifiers();
void setActiveModifiers(unsigned int mask);

unsigned int getModifier(const char *mod_name);
void ProcessKB(unsigned int player);
void BindKey(int key, unsigned int modifiers, unsigned int player, KBHandler handler, const KBData &data);
void UnbindKey(int key, unsigned int modifiers);
void InitKB();
void RestoreKB();
#endif

