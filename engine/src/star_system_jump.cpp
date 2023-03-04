/*
 * star_system_jump.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
#include <utility>
#include <vector>

#include "options.h"
#include "preferred_types.h"

void CacheJumpStar(bool destroy) {
    static vega_types::SharedPtr<Animation> cachedani = Animation::createAnimation(game_options()->jumpgate.c_str(), true, .1, MIPMAP, false);
    if (destroy) {
        cachedani.reset();
    }
}

extern std::vector<unorigdest *> pendingjump;
static std::vector<unsigned int> AnimationNulls;
class ResizeAni {
public:
    vega_types::SharedPtr<Animation> a;
    float percent;

    ResizeAni(vega_types::SharedPtr<Animation> ani, float percent) {
        a = std::move(ani);
        this->percent = percent;
    }
};
static std::vector<ResizeAni> JumpAnimations;
static std::vector<ResizeAni> VolatileJumpAnimations;

vega_types::SharedPtr<Animation> GetVolatileAni(unsigned int which) {
    if (which < VolatileJumpAnimations.size()) {
        return VolatileJumpAnimations[which].a;
    }
    return nullptr;
}

unsigned int AddAnimation(const QVector &pos,
        const float size,
        bool mvolatile,
        const std::string &name,
        float percentgrow) {
    std::vector<ResizeAni> *ja = mvolatile ? &VolatileJumpAnimations : &JumpAnimations;

    vega_types::SharedPtr<Animation> ani = Animation::createAnimation(name.c_str(), true, .1, MIPMAP, false);
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
        VolatileJumpAnimations.emplace_back(JumpAnimations[ani].a, game_options()->jumpanimationshrink);
        JumpAnimations[ani].a.reset();
        AnimationNulls.push_back(ani);
    }
}

void StarSystem::DrawJumpStars() {
    for (auto & kk : pendingjump) {
        int k = kk->animation;
        if (k != -1) {
            Unit *un = kk->un.GetUnit();
            if (un) {
                Vector p, q, r;
                un->GetOrientation(p, q, r);

                JumpAnimations[k].a
                        ->SetPosition(
                                un->Position() + r.Cast() * un->rSize() * (kk->delay + .25));
                JumpAnimations[k].a->SetOrientation(p, q, r);
                float dd = un->rSize() * game_options()->jumpgatesize
                        * (un->GetJumpStatus().delay - kk->delay) / (float) un->GetJumpStatus().delay;
                JumpAnimations[k].a->SetDimensions(dd, dd);
            }
        }
    }
    for (auto & JumpAnimation : JumpAnimations) {
        if (JumpAnimation.a) {
            JumpAnimation.a->Draw();
        }
    }
    for (size_t i = 0; i < VolatileJumpAnimations.size(); ++i) {
        if (VolatileJumpAnimations[i].a) {
            float wid, hei;
            VolatileJumpAnimations[i].a->GetDimensions(wid, hei);
            VolatileJumpAnimations[i].a->SetDimensions(VolatileJumpAnimations[i].percent * wid,
                    VolatileJumpAnimations[i].percent * hei);
            if (VolatileJumpAnimations[i].a->Done()) {
                VolatileJumpAnimations[i].a.reset();
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
    ani = AddJumpAnimation(un->Position() + r.Cast() * un->rSize() * (un->GetJumpStatus().delay + .25),
            10 * un->rSize());
    static int jumpleave = AUDCreateSound(game_options()->jumpleave, false);
    AUDPlay(jumpleave, un->LocalPosition(), un->GetVelocity(), 1);
    return ani;
}

