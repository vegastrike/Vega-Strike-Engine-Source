/*
 * in_kb.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <queue>
#include <list>
#include "src/vegastrike.h"
#include "root_generic/vs_globals.h"
#include "src/in_kb.h"
#include "src/in_handler.h"
#include "gldrv/winsys.h"
#include "src/in_kb_data.h"
#include "src/universe.h"


std::map<std::string, HandlerCall> keyBindings;
std::map<std::string, unsigned int> playerBindings;
KBSTATE mouseButtonState;

static void kbGetInput(int key, int modifiers, bool release, int x, int y) {
    const std::string map_key = std::to_string(key) + "-" + std::to_string(modifiers);

    int i = _Universe->CurrentCockpit();
    _Universe->SetActiveCockpit(playerBindings[map_key]);
    if ((keyBindings[map_key].state == RESET || keyBindings[map_key].state == UP) && !release) {
        keyBindings[map_key].function(keyBindings[map_key].data, PRESS);
    }
    if ((keyBindings[map_key].state == DOWN || keyBindings[map_key].state == RESET) && release) {
        keyBindings[map_key].function(keyBindings[map_key].data, RELEASE);
    }
    keyBindings[map_key].state = release ? UP : DOWN;
    _Universe->SetActiveCockpit(i);
}

static bool kbHasBinding(int key, int modifiers) {
    const std::string map_key = std::to_string(key) + "-" + std::to_string(modifiers);
    static HandlerCall defaultHandler;
    return keyBindings[map_key].function != defaultHandler.function;
}

static const char _lomap[] = "0123456789-=\';/.,`\\";
static const char _himap[] = ")!@#$%^&*(_+\":?><~|";

int shiftup(int ch) {
    if (ch == (ch & 0xFF)) {
        const char *c = strchr(_lomap, ch);
        if (c) {
            return _himap[c - _lomap];
        } else {
            return toupper(ch);
        }
    } else {
        return ch;
    }
}

int shiftdown(int ch) {
    if (ch == (ch & 0xFF)) {
        const char *c = strchr(_himap, ch);
        if (c) {
            return _lomap[c - _himap];
        } else {
            return tolower(ch);
        }
    } else {
        return ch;
    }
}

static unsigned int _activeModifiers = 0;

void setActiveModifiers(unsigned int mask) {
    _activeModifiers = mask;
}

#ifdef SDL_WINDOWING

void setActiveModifiersSDL(SDL_Keymod mask) {
    setActiveModifiers(
            ((mask & (KMOD_LSHIFT | KMOD_RSHIFT)) ? KB_MOD_SHIFT : 0)
                    | ((mask & (KMOD_LCTRL | KMOD_RCTRL)) ? KB_MOD_CTRL : 0)
                    | ((mask & (KMOD_LALT | KMOD_RALT)) ? KB_MOD_ALT : 0));
}

#endif

unsigned int getActiveModifiers() {
    return _activeModifiers;
}

unsigned int pullActiveModifiers() {
#ifdef SDL_WINDOWING
    setActiveModifiersSDL(SDL_GetModState());
#endif
    return getActiveModifiers();
}

unsigned int getModifier(const std::string modifier) {
    // TODO: convert to SDL_Keymod
    unsigned int modifier_value = 0;
    if (modifier.find("shift") != std::string::npos) {
        modifier_value |= KB_MOD_SHIFT;
    }
    if (modifier.find("ctrl") != std::string::npos) {
        modifier_value |= KB_MOD_CTRL;
    }
    if (modifier.find("alt") != std::string::npos) {
        modifier_value |= KB_MOD_ALT;
    }
    return modifier_value;
}

int getModifier(bool alton, bool cntrlon, bool shifton) {
    return cntrlon ? KB_MOD_CTRL : (alton ? KB_MOD_ALT : (shifton ? KB_MOD_SHIFT : 0));
}

void glut_keyboard_cb(unsigned int ch, unsigned int mod, bool release, int x, int y) {
    bool shifton = false;
    int alton = false;
    int ctrlon = false;

    unsigned int modmask = KB_MOD_MASK;
    if ((WSK_MOD_LSHIFT == (mod & WSK_MOD_LSHIFT)) || (WSK_MOD_RSHIFT == (mod & WSK_MOD_RSHIFT))) {
        //This is ugly, but we have to support legacy config files...
        //...maybe add config option to disable this soooo ugly thing...
        if (!kbHasBinding(ch, KB_MOD_SHIFT)) {
            ch = shiftup(ch);
            modmask &= ~KB_MOD_SHIFT;
        }
        shifton = true;
    }
    if ((WSK_MOD_LALT == (mod & WSK_MOD_LALT)) || (WSK_MOD_RALT == (mod & WSK_MOD_RALT))) {
        alton = true;
    }
    if ((WSK_MOD_LCTRL == (mod & WSK_MOD_LCTRL)) || (WSK_MOD_RCTRL == (mod & WSK_MOD_RCTRL))) {
        ctrlon = true;
    }
    //Polling state
    setActiveModifiers(
            (shifton ? KB_MOD_SHIFT : 0)
                    | (alton ? KB_MOD_ALT : 0)
                    | (ctrlon ? KB_MOD_CTRL : 0));

    int curmod = getModifier(alton, ctrlon, shifton) & modmask;
    kbGetInput(ch, curmod, release, x, y);
    if (release) {
        for (int i = 0; i < LAST_MODIFIER; ++i) {
            const std::string shiftdown_key = std::to_string(shiftdown(ch)) + "-" + std::to_string(i);
            const std::string shiftup_key = std::to_string(shiftup(ch)) + "-" + std::to_string(i);

            if (i != curmod) {
                if (keyBindings[shiftdown_key].state == DOWN) {
                    kbGetInput(shiftdown(ch), i, release, x, y);
                }

                if (keyBindings[shiftup_key].state == DOWN) {
                    kbGetInput(shiftup(ch), i, release, x, y);
                }
            } else {
                if (shifton) {
                    if (((unsigned int) shiftdown(ch)) != ch && keyBindings[shiftdown_key].state == DOWN) {
                        kbGetInput(shiftdown(ch), i, release, x, y);
                    }
                } else if (((unsigned int) shiftup(ch)) != ch && keyBindings[shiftup_key].state == DOWN) {
                    kbGetInput(shiftup(ch), i, release, x, y);
                }
            }
        }
    }
}

void RestoreKB() {
    for (int i = 0; i < LAST_MODIFIER; ++i) {
        for (int a = 0; a < KEYMAP_SIZE; a++) {
            const std::string map_key = std::to_string(i) + "-" + std::to_string(a);

            if (keyBindings[map_key].state == DOWN) {
                keyBindings[map_key].function(keyBindings[map_key].data, RELEASE);
                keyBindings[map_key].state = UP;
            }
        }
    }
    winsys_set_keyboard_func(glut_keyboard_cb);
}



void ProcessKB() {
    for(const std::pair<std::string, HandlerCall>& binding : keyBindings) {
        HandlerCall handler = binding.second;
        KBSTATE state = keyBindings[binding.first].state;
        binding.second.function(binding.second.data, state);
    }
}

void BindKey(int key, unsigned int mod, unsigned int player, KBHandler handler, const KBData &data) {
    const std::string map_key = std::to_string(key) + "-" + std::to_string(mod);
    HandlerCall handler_call;
    handler_call.function = handler;
    handler_call.data = data;
    keyBindings[map_key] = handler_call;
    playerBindings[map_key] = player;
    handler(std::string(), RESET);     //key is not used in handler
    keyBindings[map_key].state = UP;
}


