/*
 * drawgalaxy.cpp
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


/// Nav computer functions
/// Draws in-system map, and  galaxy map of known sectors and systems

#include <algorithm>
#include <cmath>
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "gfx/gauge.h"
#include "gfx/cockpit.h"
#include "src/universe.h"
#include "src/star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/collection.h"
#include "gfx/hud.h"
#include "gfx/vdu.h"
#include "root_generic/lin_time.h" //for fps
#include "src/config_xml.h"
#include "root_generic/lin_time.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include "src/main_loop.h"
#include <assert.h>     //needed for assert() calls
#include "vegadisk/savegame.h"
#include "gfx/animation.h"
#include "gfx_generic/mesh.h"
#include "src/universe_util.h"
#include "src/in_mouse.h"
#include "gui/glut_support.h"
#include "cmd/unit_util.h"
#include "src/hashtable.h"
#include "gfx/nav/navscreen.h"
#include "gfx/masks.h"
#include "gfx/nav/navscreenoccupied.h"

#include "drawgalaxy.h"

float SYSTEM_DEFAULT_SIZE = 0.02;
GFXColor GrayColor(.5, .5, .5, .5);

void DrawNodeDescription(string text,
        float x_,
        float y_,
        float size_x,
        float size_y,
        bool ignore_occupied_areas,
        const GFXColor &col,
        navscreenoccupied *screenoccupation) {
    //take the head and stick it in the back
    if (text.size() == 0) {
        return;
    }
    TextPlane displayname;   //will be used to display shits names
    displayname.color = static_cast<ImU32>(col);
    const float background_alpha = configuration().graphics.hud.text_background_alpha_flt;
    int length = text.size();
    float offset = (float(length) * 0.005);
    if (ignore_occupied_areas) {
        displayname.SetPos((x_ - offset), y_);
        displayname.SetText(text);
        displayname.SetCharSize(size_x, size_y);

        GFXColor tpbg(displayname.background_color);
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            GFXColor temp_background_color( 0, 0, 0, background_alpha );
            displayname.background_color = static_cast<ImU32>(temp_background_color);
        }
        displayname.Draw(text, 0, true, false, automatte);
        displayname.background_color = static_cast<ImU32>(tpbg);
    } else {
        float new_y = screenoccupation->findfreesector(x_, y_);
        displayname.SetPos((x_ - offset), new_y);
        displayname.SetText(text);
        displayname.SetCharSize(size_x, size_y);
        GFXColor tpbg(displayname.background_color);
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            GFXColor temp_background_color( 0, 0, 0, background_alpha );
            displayname.background_color = static_cast<ImU32>(temp_background_color);
        }
        displayname.Draw(text, 0, true, false, automatte);
        displayname.background_color = static_cast<ImU32>(tpbg);
    }
}


// TODO: fix this undocumented function
// Currently, it returns 'm' if system has been visited twice.
// It returns 'v', if a system hasn't been visited or has been visited once.
// It returns '?' if the system has been visited more than twice.

// If 'm', then race = GrayColor;
// 'v' does nothing
// '?' does nothing

// Worse, the visit counter keeps iterating every time you visit the system.
// This means that the color will turn grey for twice and then not.
char GetSystemColor(string source) {
    //FIXME: update me!!!
    vector<float> *v = &_Universe->AccessCockpit()->savegame->getMissionData("visited_" + source);
    if (v->size()) {
        float k = (*v)[0];
        if (k >= 2) {
            return k == 2 ? 'm' : '?';
        }
    }
    return 'v';
}

void DrawNode(int type,
        float size,
        float x,
        float y,
        std::string source,
        navscreenoccupied *screenoccupation,
        bool moused,
        GFXColor race,
        bool mouseover,
        bool willclick,
        string insector) {
    char color = GetSystemColor(source);
    if (moused) {
        return;
    }
    if (willclick == true && mouseover == false) {
        //Perhaps some key binding or mouseclick will be set in the future to do this.
        mouseover = true;
    }
    static GFXColor
            highlighted_tail_col = vs_config->getColor("nav", "highlighted_unit_on_tail", GFXColor(1, .3, .3, .8));
    static GFXColor
            highlighted_tail_text = vs_config->getColor("nav", "highlighted_text_on_tail", GFXColor(1, 1, .7, 1));
    if (color == 'm') {
        race = GrayColor;
    }
    if (mouseover) {
        if (willclick) {
            race = highlighted_tail_col;
        } else {
            //Leave just a faint resemblence of the original color,
            //but also make it look whiteish.
            race.r += .75;
            race.g += .75;
            race.b += .75;
        }
    }
    NavigationSystem::DrawCircle(x, y, size, race);
    if ((!mouseover) || (willclick)) {
        string tsector, nam;
        Beautify(source, tsector, nam);
        if (willclick) {
            race = highlighted_tail_text;
            nam = tsector + " / " + nam;
        }
        if (willclick || !(insector.compare("")) || !(insector.compare(tsector))) {
            DrawNodeDescription(nam, x, y, 1.0, 1.0, 0, race, screenoccupation);
        }
    }
}




//typedef Hashtable <std::string, const systemdrawnode, 127> systemdrawhashtable;

bool testandset(bool &b, bool val) {
    bool tmp = b;
    b = val;
    return tmp;
}





bool checkedVisited(const std::string &n) {
    const bool dontbothervisiting =
            !configuration().graphics.explore_for_map;
    if (dontbothervisiting) {
        return true;
    } else {
        string key(string("visited_") + n);
        vector<float> *v = &_Universe->AccessCockpit()->savegame->getMissionData(key);
        if (v->size() > 0) {
            return true;
        }
        return false;
    }
}

NavigationSystem::SystemIterator &NavigationSystem::SystemIterator::operator++() {
    which += 1;
    if (which >= vstack.size()) {
        vector<string> newsys;
        for (unsigned int i = 0; i < vstack.size(); ++i) {
            int nas = UniverseUtil::GetNumAdjacentSystems(vstack[i]);
            for (int j = 0; j < nas; ++j) {
                string n = UniverseUtil::GetAdjacentSystem(vstack[i], j);
                if (!testandset(visited[n], true)) {
//if (checkedVisited(n)) {
                    newsys.push_back(n);
                }
//}
            }
        }
        vstack.swap(newsys);
        count += 1;
        which = 0;
        if (count > maxcount) {
            vstack.clear();
        }
    }
    return *this;
}

//************************************************
//
//SYSTEM SECTION
//
//************************************************





