/*
 * star_system_jump.cpp
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

#include "src/vegastrike.h"
#include "src/star_system.h"
#include "cmd/planet.h"
#include "root_generic/lin_time.h"
#include "src/hashtable.h"
#include "gfx/animation.h"
#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "cmd/container.h"
#include "root_generic/xml_support.h"
#include <assert.h>
#include "gfx/cockpit.h"
#include "src/audiolib.h"
#include "cmd/images.h"
#include "cmd/script/flightgroup.h"
#include <string>
#include <vector>

#include "root_generic/options.h"

void CacheJumpStar(bool destroy) {
    static Animation* cached_ani = nullptr;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        cached_ani = new Animation(configuration().graphics.jump_gate.c_str(), true, 0.1, MIPMAP, false);
    }
    if (destroy) {
        delete cached_ani;
        cached_ani = nullptr;
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
    return AddAnimation(pos, size, mvolatile, configuration().graphics.jump_gate, .95);
}

void StarSystem::VolitalizeJumpAnimation(const int ani) {
    if (ani != -1) {
        VolatileJumpAnimations.emplace_back(JumpAnimations[ani].a, configuration().graphics.jump_animation_shrink_flt);
        JumpAnimations[ani].a = nullptr;
        AnimationNulls.push_back(ani);
    }
}

void StarSystem::DrawJumpStars() {
    for (const auto& jump : pendingjump) {
        const int k = jump->animation;
        if (k != -1) {
            const Unit* un = jump->un.GetUnit();
            if (un) {
                Vector p, q, r;
                un->GetOrientation(p, q, r);

                JumpAnimations[k].a
                                 ->SetPosition(
                                     un->Position() + r.Cast() * un->rSize() * (jump->delay + .25));
                JumpAnimations[k].a->SetOrientation(p, q, r);
                const float dd = un->rSize() * configuration().graphics.jump_gate_size_dbl
                        * (un->jump_drive.Delay() - jump->delay) / un->jump_drive.Delay();
                JumpAnimations[k].a->SetDimensions(dd, dd);
            }
        }
    }
    for (const auto& jump_animation : JumpAnimations) {
        if (jump_animation.a) {
            jump_animation.a->Draw();
        }
    }
    for (const auto& jump_animation : VolatileJumpAnimations) {
        if (jump_animation.a) {
            float width;
            float height;
            jump_animation.a->GetDimensions(width, height);
            jump_animation.a->SetDimensions(jump_animation.percent * width, jump_animation.percent * height);
            if (!jump_animation.a->Done()) {
                jump_animation.a->Draw();
            }
        }
    }
    const auto first_to_remove = std::stable_partition(VolatileJumpAnimations.begin(), VolatileJumpAnimations.end(),
                                                 [](ResizeAni& ani) -> bool {
                                                     return !ani.a->Done();
                                                 });
    for (auto iter = first_to_remove; iter != VolatileJumpAnimations.end(); ++iter) {
        delete iter->a;
    }
    VolatileJumpAnimations.erase(first_to_remove, VolatileJumpAnimations.end());
}

void StarSystem::DoJumpingComeSightAndSound(Unit *un) {
    Vector p, q, r;
    un->GetOrientation(p, q, r);
    unsigned int myani = AddJumpAnimation(un->LocalPosition(), un->rSize() * configuration().graphics.jump_gate_size_flt, true);
    VolatileJumpAnimations[myani].a->SetOrientation(p, q, r);
}

int StarSystem::DoJumpingLeaveSightAndSound(Unit *un) {
    int ani;
    Vector p, q, r;
    un->GetOrientation(p, q, r);
    ani = AddJumpAnimation(un->Position() + r.Cast() * un->rSize() * (un->jump_drive.Delay() + .25),
            10 * un->rSize());
    static boost::optional<int> jump_leave{};
    if (jump_leave == boost::none) {
        jump_leave = AUDCreateSound(configuration().audio.unit_audio.jump_leave, false);
    }
    AUDPlay(jump_leave.get(), un->LocalPosition(), un->GetVelocity(), 1);
    return ani;
}

