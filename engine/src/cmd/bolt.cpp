/*
 * bolt.cpp
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


#include <vector>
#include <string>
#include <algorithm>

#include "cmd/bolt.h"

#include "gfx_generic/boltdrawmanager.h"
#include "src/gfxlib.h"
#include "gfx_generic/mesh.h"
#include "src/gfxlib_struct.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "src/config_xml.h"
#include "gfx/camera.h"
#include "root_generic/options.h"
#include "src/universe.h"
#include "cmd/damageable.h"
#include "src/vs_logging.h"
#include "gfx_generic/texture_manager.h"

using std::vector;
using std::string;

extern double interpolation_blend_factor;

inline void BlendTrans(Matrix &drawmat, const QVector &cur_position, const QVector &prev_position) {
    drawmat.p = prev_position.Scale(1 - interpolation_blend_factor) + cur_position.Scale(interpolation_blend_factor);
}

// Bolts have texture
int Bolt::AddTexture(BoltDrawManager *q, std::string file) {
    size_t decal = q->boltdecals.AddTexture(file.c_str(), MIPMAP);
    if (decal >= q->bolts.size()) {
        q->bolts.push_back(vector<Bolt>());
        size_t blargh = q->boltdecals.AddTexture(file.c_str(), MIPMAP);
        if (blargh >= q->bolts.size()) {
            q->bolts.push_back(vector<Bolt>());
        }
    }

    return decal;
}

// Balls have animation
int Bolt::AddAnimation(BoltDrawManager *q, std::string file, QVector cur_position) {
    int decal = -1;
    for (unsigned int i = 0; i < q->animationname.size(); i++) {
        if (file == q->animationname[i]) {
            decal = i;
        }
    }
    if (decal == -1) {
        decal = q->animations.size();
        q->animationname.push_back(file);
        q->animations
                .push_back(new Animation(file.c_str(),
                        true,
                        .1,
                        MIPMAP,
                        false));         //balls have their own orientation
        q->animations.back()->SetPosition(cur_position);
        q->balls.push_back(vector<Bolt>());
    }
    return decal;
}

void Bolt::DrawAllBolts() {
    BoltDrawManager &bolt_draw_manager = BoltDrawManager::GetInstance();
    GFXVertexList *qmesh = BoltDrawManager::boltmesh;

    if (!qmesh || bolt_draw_manager.bolts.empty()) {
        return;
    }

    GFXAlphaTest(ALWAYS, 0);
    GFXDisable(DEPTHWRITE);
    GFXDisable(TEXTURE1);
    GFXEnable(TEXTURE0);
    GFXTextureCoordGenMode(0, NO_GEN, nullptr, nullptr);

    BLENDFUNC bsrc, bdst;
    if (configuration().graphics.blend_guns) {
        GFXBlendMode(bsrc = ONE, bdst = ONE);
    } else {
        GFXBlendMode(bsrc = ONE, bdst = ZERO);
    }

    qmesh->LoadDrawState();
    qmesh->BeginDrawState();
    int decal = 0;

    // Iterate over specific types of bolts (with same texture)
    for (auto &&bolt_types : bolt_draw_manager.bolts) {
        if (bolt_types.empty()) {
            continue;
        }

        Bolt bolt = bolt_types[0];

        Texture *texture = TextureManager::GetInstance().GetTexture(bolt.bolt_name, MIPMAP);
        if (!texture) {
            VS_LOG(error, (boost::format("No texture found for bolt named %1$s") % bolt.bolt_name));
            continue;
        }

        for (size_t pass = 0, npasses = texture->numPasses(); pass < npasses; ++pass) {
            GFXTextureEnv(0, GFXMODULATETEXTURE);
            if (texture->SetupPass(0, bsrc, bdst)) {
                texture->MakeActive();
                GFXToggleTexture(true, 0);
                for (auto &&each_bolt : bolt_types) {
                    each_bolt.DrawBolt(qmesh);
                }
            }
        }
        decal++;
    }

    qmesh->EndDrawState();
}

void Bolt::DrawAllBalls() {
    BoltDrawManager &bolt_draw_manager = BoltDrawManager::GetInstance();
    vector<Animation *>::iterator k = bolt_draw_manager.animations.begin();

    for (auto &&ball_types : bolt_draw_manager.balls) {
        if (ball_types.size() == 0) {
            continue;
        }

        Animation *cur = *k;

        float bolt_size = 2 * ball_types[0].type->radius * 2;
        bolt_size *= bolt_size;
        //Matrix result;
        //FIXME::MuST USE DRAWNO	TRANSFORMNOW cur->CalculateOrientation (result);

        // Iterate over specific balls
        for (auto &&bolt : ball_types) { // really ball
            bolt.DrawBall(bolt_size, cur);
        }
    }
}

void Bolt::DrawBolt(GFXVertexList *qmesh) {
    float distance = (cur_position - BoltDrawManager::camera_position).MagnitudeSquared();

    if (distance * BoltDrawManager::pixel_angle >= bolt_size) {
        return;
    }

    const WeaponInfo *wt = type;

    BlendTrans(drawmat, cur_position, prev_position);
    Matrix drawmat(this->drawmat);
    if (configuration().graphics.stretch_bolts_flt > 0) {
        ScaleMatrix(drawmat,
                    Vector(1,
                           1,
                           static_cast<double>(type->speed) * BoltDrawManager::elapsed_time * configuration().graphics.
                           stretch_bolts_dbl / type->length));
    }
    GFXLoadMatrixModel(drawmat);
    GFXColor4f(wt->r, wt->g, wt->b, wt->a);
    qmesh->Draw();
}

void Bolt::DrawBall(float &bolt_size, Animation *cur) {
    // TODO: move up to DrawBalls
    Vector p, q, r;
    _Universe->AccessCamera()->GetOrientation(p, q, r);

    //don't update time more than once
    float distance = (cur_position - BoltDrawManager::camera_position).MagnitudeSquared();
    if (distance * BoltDrawManager::pixel_angle < bolt_size) {
        BlendTrans(drawmat, cur_position, prev_position);
        Matrix tmp;
        VectorAndPositionToMatrix(tmp, p, q, r, drawmat.p);
        cur->SetDimensions(type->radius, type->radius);
        GFXLoadMatrixModel(tmp);
        GFXColor4f(type->r, type->g, type->b, type->a);
        cur->DrawNoTransform(false, true);
    }
}

void Bolt::Destroy(unsigned int index) {
    bool isBall = true;
    if (type->type == WEAPON_TYPE::BOLT) {
        isBall = false;
    } else {
    }
    //BoltDestroyGeneric( this, index, decal, isBall );

    BoltDrawManager &q = BoltDrawManager::GetInstance();
    vector<vector<Bolt> > *target;
    if (!isBall) {
        target = &q.bolts;
    } else {
        target = &q.balls;
    }

    vector<Bolt> *vec = &(*target)[decal];
    if (&(*vec)[index] == this) {
        unsigned int tsize = vec->size();
        CollideMap *cm = _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT];
        cm->UpdateBoltInfo(vec->back().location, (*(*vec)[index].location)->ref);

        assert(index < tsize);
        cm->erase((*vec)[index].location);
        if (index + 1 != vec->size()) {
            (*vec)[index] = vec->back();                //just a memcopy, yo
        }

        vec->pop_back();         //pop that back up
    } else {
        VS_LOG_AND_FLUSH(fatal, "Bolt Fault Nouveau! Not found in draw queue! No Chance to recover");
        assert(0);
    }
}

// A bolt is created when fired
Bolt::Bolt(const WeaponInfo *typ,
        const Matrix &orientationpos,
        const Vector &shipspeed,
        void *owner,
        bool player_fired,
        CollideMap::iterator hint) : cur_position(orientationpos.p), ShipSpeed(shipspeed) {
    VSCONSTRUCT2('t')
    BoltDrawManager &q = BoltDrawManager::GetInstance();
    prev_position = cur_position;
    this->owner = owner;
    this->player_fired = player_fired;
    this->type = typ;
    bolt_name = typ->file;
    curdist = 0;
    bolt_size = std::pow(2 * type->radius + type->length, 2);
    ball_size = std::pow(4 * type->radius, 2);

    CopyMatrix(drawmat, orientationpos);
    Vector vel = shipspeed + orientationpos.getR() * typ->speed;

    StarSystem *current_star_system = _Universe->activeStarSystem();
    CollideMap *bolt_collide_map = current_star_system->collide_map[Unit::UNIT_BOLT];

    if (typ->type == WEAPON_TYPE::BOLT) {
        ScaleMatrix(drawmat, Vector(typ->radius, typ->radius, typ->length));
        decal = Bolt::AddTexture(&q, typ->file);

        int bolt_index = Bolt::BoltIndex(q.bolts[decal].size(),
                decal,
                false).bolt_index;

        this->location = bolt_collide_map->insert(Collidable(bolt_index,
                        (shipspeed
                                + orientationpos.getR() * typ->speed).Magnitude()
                                * .5,
                        cur_position + vel * simulation_atom_var * .5),
                hint);

        q.bolts[decal].push_back(*this);

        BoltDrawManager &bolt_draw_manager = BoltDrawManager::GetInstance();
        bolt_texture = bolt_draw_manager.boltdecals.GetTexture(decal);
    } else {
        ScaleMatrix(drawmat, Vector(typ->radius, typ->radius, typ->radius));
        decal = Bolt::AddAnimation(&q, typ->file, cur_position);

        int bolt_index = Bolt::BoltIndex(q.balls[decal].size(),
                decal,
                true).bolt_index;
        Collidable collidable = Collidable(bolt_index,
                (shipspeed + orientationpos.getR()
                        * typ->speed).Magnitude() * .5,
                cur_position + vel * simulation_atom_var * .5);
        this->location = bolt_collide_map->insert(collidable, hint);
        q.balls[decal].push_back(*this);
    }
}

size_t nondecal_index(Collidable::CollideRef b) {
    return b.bolt_index >> 8;
}

bool Bolt::Update(Collidable::CollideRef index) {
    const WeaponInfo *type = this->type;
    float speed = type->speed;
    curdist += speed * simulation_atom_var;
    prev_position = cur_position;
    cur_position +=
            ((ShipSpeed + drawmat.getR() * speed
                    / ((type->type
                            == WEAPON_TYPE::BALL) * type->radius
                            + (type->type != WEAPON_TYPE::BALL) * type->length)).Cast() * simulation_atom_var);
    if (curdist > type->range) {
        this->Destroy(nondecal_index(index));         //risky
        return false;
    }
    Collidable updated(**location);
    updated.SetPosition(.5 * (prev_position + cur_position));
    location = _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT]->changeKey(location, updated);
    return true;
}

class UpdateBolt {
    CollideMap *collide_map;
    StarSystem *starSystem;
public:
    UpdateBolt(StarSystem *ss, CollideMap *collide_map) {
        this->starSystem = ss;
        this->collide_map = collide_map;
    }

    void operator()(Collidable &collidable) {
        if (collidable.radius < 0) {
            Bolt *thus = Bolt::BoltFromIndex(collidable.ref);
            if (!collide_map->CheckCollisions(thus, collidable)) {
                thus->Update(collidable.ref);
            }
        }
    }
};

namespace vsalg {
//

template<typename IT, typename F>
void for_each(IT start, IT end, F f) {
    //This way, deletion of current item is allowed
    //- drawback: iterator copy each iteration
    while (start != end) {
        f(*start++);
    }
}

//
}

class UpdateBolts {
    UpdateBolt sub;
public:
    UpdateBolts(StarSystem *ss, CollideMap *collide_map) : sub(ss, collide_map) {
    }

    template<class T>
    void operator()(T &collidableList) {
        vsalg::for_each(collidableList.begin(), collidableList.end(), sub);
    }
};

void Bolt::UpdatePhysics(StarSystem *ss) {
    CollideMap *cm = ss->collide_map[Unit::UNIT_BOLT];
    vsalg::for_each(cm->sorted.begin(), cm->sorted.end(), UpdateBolt(ss, cm));
    vsalg::for_each(cm->toflattenhints.begin(), cm->toflattenhints.end(), UpdateBolts(ss, cm));
}

bool Bolt::Collide(Unit *target) {
    Vector normal;
    float distance;
    Unit *affectedSubUnit;
    if ((affectedSubUnit = target->rayCollide(prev_position, cur_position, normal, distance))) {
        //ignore return
        if (target == owner) {
            return false;
        }
        enum Vega_UnitType type = target->getUnitType();
        if (type == Vega_UnitType::nebula || type == Vega_UnitType::asteroid) {
            const bool collideroids = configuration().physics.asteroid_weapon_collision;
            if (type != Vega_UnitType::asteroid || (!collideroids)) {
                return false;
            }
        }
        const bool collidejump = configuration().physics.jump_weapon_collision;
        if (type == Vega_UnitType::planet && (!collidejump) && !target->GetDestinations().empty()) {
            return false;
        }
        QVector tmp = (cur_position - prev_position).Normalize();
        tmp = tmp.Scale(distance);
        distance = curdist / this->type->range;
        GFXColor coltmp(this->type->r, this->type->g, this->type->b, this->type->a);
        Damage damage(this->type->damage * ((1 - distance) + distance * this->type->long_range),
                this->type->phase_damage * ((1 - distance) + distance * this->type->long_range));

        target->ApplyDamage((prev_position + tmp).Cast(),
                normal,
                damage,
                affectedSubUnit,
                coltmp,
                owner, player_fired);
        return true;
    }
    return false;
}

Bolt *Bolt::BoltFromIndex(Collidable::CollideRef b) {
    BoltDrawManager &bolt_draw_manager = BoltDrawManager::GetInstance();
    size_t ind = nondecal_index(b);
    if (b.bolt_index & 128) {
        return &bolt_draw_manager.balls[b.bolt_index & 0x7f][ind];
    } else {
        return &bolt_draw_manager.bolts[b.bolt_index & 0x7f][ind];
    }
}

bool Bolt::CollideAnon(Collidable::CollideRef b, Unit *un) {
    Bolt *tmp = BoltFromIndex(b);
    if (tmp->Collide(un)) {
        tmp->Destroy(nondecal_index(b));
        return true;
    }
    return false;
}

Collidable::CollideRef Bolt::BoltIndex(int index, int decal, bool isBall) {
    Collidable::CollideRef temp;
    temp.bolt_index = index;
    temp.bolt_index <<= 8;
    temp.bolt_index |= decal;
    temp.bolt_index |= isBall ? 128 : 0;
    return temp;
}


