/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_IN_KB_H
#define VEGA_STRIKE_ENGINE_IN_KB_H

#include <map>
#include <string>

#include "src/in.h"
#include "src/in_kb_data.h"

//#ifdef HAVE_SDL
#include <SDL2/SDL.h>
//const int KEYMAP_SIZE =SDLK_LAST;
//const int KEY_SPECIAL_OFFSET=0;
//#else //HAVE_SDL

static void DefaultKBHandler(const KBData &, KBSTATE newState) // FIXME ?
{
    //do nothing
}

struct HandlerCall {
    KBHandler function;
    KBData data;
    KBSTATE state;

    HandlerCall() {
        function = DefaultKBHandler;
    }
};


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
#else //NO_GFX
const int KEYMAP_SIZE   = 0;
const int LAST_MODIFIER = 0;
#endif //NO_GFX

//#endif //HAVE_SDL

#include "src/in.h"

unsigned int getActiveModifiers();
unsigned int pullActiveModifiers();
void setActiveModifiers(unsigned int mask);

unsigned int getModifier(const std::string modifier);
void ProcessKB();
void BindKey(int key, unsigned int modifiers, unsigned int player, KBHandler handler, const KBData &data);
void UnbindKey(int key, unsigned int modifiers);
void InitKB();
void RestoreKB();

#endif //VEGA_STRIKE_ENGINE_IN_KB_H
