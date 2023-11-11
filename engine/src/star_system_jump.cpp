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

#include "vegastrike.h"
#include "star_system.h"
#include "cmd/planet.h"
#include "lin_time.h"
#include "hashtable.h"
#include "gfx/animation.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "cmd/container.h"
#include "xml_support.h"
#include <assert.h>
#include "gfx/cockpit.h"
#include "audiolib.h"
#include "cmd/images.h"
#include "cmd/script/flightgroup.h"
#include <string>
#include <vector>

#include "options.h"

void CacheJumpStar(bool destroy) {
    static Animation *cachedani = new Animation(game_options()->jumpgate.c_str(), true, .1, MIPMAP, false);
    if (destroy) {
        delete cachedani;
        cachedani = nullptr;
    }
}

extern std::vector<unorigdest *> pendingjump;
static std::vector<unsigned int> AnimationNulls;
class ResizeAni {
public:
    Animation *a;
    float percent;

    ResizeAni(Animation *ani, float percent) {
        a = ani;
        this->percent = percent;
    }
};
static std::vector<ResizeAni> JumpAnimations;
static std::vector<ResizeAni> VolatileJumpAnimations;

Animation *GetVolatileAni(unsigned int which) {
    if (which < VolatileJumpAnimations.size()) {
        return VolatileJumpAnimations[which].a;
    }
    return NULL;
}

unsigned int AddAnimation(const QVector &pos,
        const float size,
        bool mvolatile,
        const std::string &name,
        float percentgrow) {
    std::vector<ResizeAni> *ja = mvolatile ? &VolatileJumpAnimations : &JumpAnimations;

    Animation *ani = new Animation(name.c_str(), true, .1, MIPMAP, false);
    unsigned int i;
    if (mvolatile || AnimationNulls.empty()) {
        i = ja->size();
        ja->push_back(ResizeAni(ani, percentgrow));
    } else {
        assert(JumpAnimations[AnimationNulls.back()].a == NULL);
        JumpAnimations[AnimationNulls.back()] = ResizeAni(ani, percentgrow);
        i = AnimationNulls.back();
        AnimationNulls.pop_back();
    }
    (*ja)[i].a->SetDimensions(size, size);
    (*ja)[i].a->SetPosition(pos);
    return i;
}

static unsigned int AddJumpAnimation(const QVector &pos, const float size, bool mvolatile = false) {
    return AddAnimation(pos, size, mvolatile, game_options()->jumpgate, .95);
}

void StarSystem::VolitalizeJumpAnimation(const int ani) {
    if (ani != -1) {
        VolatileJumpAnimations.push_back(ResizeAni(JumpAnimations[ani].a, game_options()->jumpanimationshrink));
        JumpAnimations[ani].a = NULL;
        AnimationNulls.push_back(ani);
    }
}

void StarSystem::DrawJumpStars() {
    for (unsigned int kk = 0; kk < pendingjump.size(); ++kk) {
        int k = pendingjump[kk]->animation;
        if (k != -1) {
            Unit *un = pendingjump[kk]->un.GetUnit();
            if (un) {
                Vector p, q, r;
                un->GetOrientation(p, q, r);

                JumpAnimations[k].a
                        ->SetPosition(
                                un->Position() + r.Cast() * un->rSize() * (pendingjump[kk]->delay + .25));
                JumpAnimations[k].a->SetOrientation(p, q, r);
                float dd = un->rSize() * game_options()->jumpgatesize
                        * (un->jump.delay - pendingjump[kk]->delay) / (float) un->jump.delay;
                JumpAnimations[k].a->SetDimensions(dd, dd);
            }
        }
    }
    for (size_t i = 0; i < JumpAnimations.size(); ++i) {
        if (JumpAnimations[i].a) {
            JumpAnimations[i].a->Draw();
        }
    }
    for (size_t i = 0; i < VolatileJumpAnimations.size(); ++i) {
        if (VolatileJumpAnimations[i].a) {
            float hei, wid;
            VolatileJumpAnimations[i].a->GetDimensions(hei, wid);
            VolatileJumpAnimations[i].a->SetDimensions(VolatileJumpAnimations[i].percent * hei,
                    VolatileJumpAnimations[i].percent * wid);
            if (VolatileJumpAnimations[i].a->Done()) {
                delete VolatileJumpAnimations[i].a;
                VolatileJumpAnimations[i].a = nullptr;
                VolatileJumpAnimations.erase(VolatileJumpAnimations.begin() + i);
                --i;
            } else {
                VolatileJumpAnimations[i].a->Draw();
            }
        }
    }
}

void StarSystem::DoJumpingComeSightAndSound(Unit *un) {
    Vector p, q, r;
    un->GetOrientation(p, q, r);
    unsigned int myani = AddJumpAnimation(un->LocalPosition(), un->rSize() * game_options()->jumpgatesize, true);
    VolatileJumpAnimations[myani].a->SetOrientation(p, q, r);
}

int StarSystem::DoJumpingLeaveSightAndSound(Unit *un) {
    int ani;
    Vector p, q, r;
    un->GetOrientation(p, q, r);
    ani = AddJumpAnimation(un->Position() + r.Cast() * un->rSize() * (un->jump.delay + .25),
            10 * un->rSize());
    static int jumpleave = AUDCreateSound(game_options()->jumpleave, false);
    AUDPlay(jumpleave, un->LocalPosition(), un->GetVelocity(), 1);
    return ani;
}

