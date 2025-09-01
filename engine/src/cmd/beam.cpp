/*
 * beam.cpp
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
#include <vector>
#include "cmd/beam.h"
#include "cmd/unit_generic.h"
#include "gfx/aux_texture.h"
#include "gfx/decalqueue.h"
using std::vector;
#include "src/audiolib.h"
#include "root_generic/configxml.h"
#include "cmd/images.h"
#include "cmd/mount_size.h"
#include "cmd/weapon_info.h"
#include "cmd/damageable.h"
#include "src/universe.h"

#include <algorithm>
#include <cmath>

/*
 * Externs
 */
extern double interpolation_blend_factor;
extern void AdjustMatrixToTrackTarget(Matrix &mat, const Vector &vel, Unit *target, float speed, bool lead, float cone);
extern bool AdjustMatrix(Matrix &mat, const Vector &velocity, Unit *target, float speed, bool lead, float cone);

/*
 * Internal variables and structs
 */
struct BeamDrawContext {
    Matrix m;
    class GFXVertexList *vlist;
    Beam *beam;

    BeamDrawContext() {
    }

    BeamDrawContext(const Matrix &a, GFXVertexList *vl, Beam *b) : m(a), vlist(vl), beam(b) {
    }
};

static DecalQueue beamdecals;
static vector<vector<BeamDrawContext> > beamdrawqueue;

/*
 * Internal functions
 */
static bool beamCheckCollision(QVector pos, float len, const Collidable &un) {
    return (un.GetPosition() - pos).MagnitudeSquared() <= len * len + 2 * len * un.radius + un.radius * un.radius;
}



/*
 * Private Methods
 */
//NOTE: The order of the quad's vertices IS important - it ensures symmetric interpolation.
#define V(xx, yy, zz, ss, tt, aa) \
    do {                            \
        beam[a].x = xx;             \
        beam[a].y = yy;             \
        beam[a].z = zz;             \
        beam[a].s = ss;             \
        beam[a].t = tt;             \
        beam[a].r = this->Col.r*aa; \
        beam[a].g = this->Col.g*aa; \
        beam[a].b = this->Col.b*aa; \
        beam[a].a = 1.0f;           \
        a++;                        \
    }                               \
    while (0)

void Beam::RecalculateVertices(const Matrix &trans) {
    GFXColorVertex *beam = (vlist->BeginMutate(0))->colors;
    const float fadelocation = configuration().graphics.beam_fadeout_length_flt;
    const float hitfadelocation = configuration().graphics.beam_fadeout_hit_length_flt;
    static float scoopangle =
            //In radians - the /2 is because of the way in which we check against the cone.
            XMLSupport::parse_float(vs_config->getVariable("physics", "tractor.scoop_fov", "0.5")) / 2;
    static float scooptanangle = (float) tan(scoopangle);
    static bool scoop = XMLSupport::parse_bool(vs_config->getVariable("graphics", "tractor.scoop", "true"));
    static float scoopa =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "tractor.scoop_alpha_multiplier", "2.5"));
    static int radslices =
            XMLSupport::parse_int(vs_config->getVariable("graphics", "tractor.scoop_rad_slices", "10"))
                    | 1;         //Must be odd
    static int longslices =
            XMLSupport::parse_int(vs_config->getVariable("graphics", "tractor.scoop_long_slices", "10"));
    const float fadeinlength = 4;
    const bool tractor = (damagerate < 0 && phasedamage > 0);
    const bool repulsor = (damagerate > 0 && phasedamage < 0);
    float leftex = -texturespeed * (numframes * simulation_atom_var + interpolation_blend_factor * simulation_atom_var);
    float righttex = leftex + texturestretch * curlength / curthick;           //how long compared to how wide!
    float len = (impact == ALIVE)
            ? (curlength < range ? curlength - speed * simulation_atom_var * (1 - interpolation_blend_factor)
                    : range)
            : curlength;
    float fadelen = (impact == ALIVE) ? len * fadelocation : len * hitfadelocation;
    const bool doscoop = (scoop && (tractor || repulsor));
    float fadetex = leftex + (righttex - leftex) * fadelocation;
    const float touchtex = leftex - fadeinlength * .5 * texturestretch;
    float thick = curthick != thickness ? curthick - radialspeed * simulation_atom_var
            * (1 - interpolation_blend_factor) : thickness;
    float ethick = (thick / ((thickness > 0) ? thickness : 1.0f)) * (doscoop ? curlength * scooptanangle : 0);
    const float invfadelen = thick * fadeinlength;
    const float invfadealpha = std::max(0.0f, std::min(1.0f, 1.0f - std::sqrt(invfadelen / len)));
    const float fadealpha = std::max(0.0f, std::min(1.0f, 1.0f - std::sqrt(fadelen / len)));
    const float endalpha = 0.0f;
    const float peralpha = doscoop ? 0.25f : 0.0f;
    int a = 0;
    if (doscoop) {
        //Do the volumetric thingy
        Vector r(_Universe->AccessCamera()->GetR());
        Vector x(trans.getP()), y(trans.getQ()), z(trans.getR());
        r.Normalize();
        x.Normalize();
        y.Normalize();
        z.Normalize();
        const float xyalpha = std::max(0.0f, fabs(z * r));
        const float xzalpha = std::max(0.0f, fabs(y * r)) * 0.5f;
        const float yzalpha = std::max(0.0f, fabs(x * r)) * 0.5f;
        const float lislices = (longslices > 0) ? 1.0f / longslices : 0.0f;
        const float rislices = (radslices > 0) ? 1.0f / radslices : 0.0f;
        const float bxyalpha = xyalpha * lislices;
        const float bxzalpha = xzalpha * rislices;
        const float byzalpha = yzalpha * rislices;
        const float zs = lislices * (fadelen - invfadelen);
        const float ths = lislices * ethick * 1.2f;
        const float rim1 = (radslices - 1) * rislices * 2;
        for (int i = 0; i < longslices; i++) {
            float f = i * lislices;
            float xa = std::max(0.0f, 1.0f - std::sqrt(f)) * byzalpha * scoopa;
            float ya = std::max(0.0f, 1.0f - std::sqrt(f)) * bxzalpha * scoopa;
            float za = std::max(0.0f, 1.0f - std::sqrt(f)) * bxyalpha * scoopa;
            float th = f * ethick + thick;
            float z = i * zs + invfadelen;
            if (za > 0.03) {
                V(-th, +th, z, 0, 0.5f, za);
                V(-th, -th, z, 0, 0.0f, za);
                V(+th, -th, z, 1, 0.0f, za);
                V(+th, +th, z, 1, 0.5f, za);
            }
            if (i > 1) {
                if (ya > 0.03) {
                    for (int j = -radslices / 2; j <= radslices / 2; j++) {
                        float y = j * 2 * th * rislices;
                        float f = 1.0f - fabs(rim1 * j * rislices);
                        float sf = sqrt(f);
                        float aa = ya * f;
                        if (aa > 0.03) {
                            V(-(th + ths) * sf, y, z + zs, 1, 0.50f, aa);
                            V(-(th - ths) * sf, y, z - zs, 0, 0.50f, aa);
                            V(0, y, z - zs, 0, 0.75f, aa);
                            V(0, y, z + zs, 1, 0.75f, aa);
                            V(0, y, z + zs, 1, 0.75f, aa);
                            V(0, y, z - zs, 0, 0.75f, aa);
                            V(+(th - ths) * sf, y, z - zs, 0, 1.00f, aa);
                            V(+(th + ths) * sf, y, z + zs, 1, 1.00f, aa);
                        }
                    }
                }
                if (xa > 0.03) {
                    for (int j = -radslices / 2; j <= radslices / 2; j++) {
                        float x = j * 2 * th * rislices;
                        float f = 1.0f - fabs(rim1 * j * rislices);
                        float sf = sqrt(f);
                        float aa = xa * f;
                        if (aa > 0.03) {
                            V(x, -(th + ths) * sf, z + zs, 1, 0.50f, aa);
                            V(x, -(th - ths) * sf, z - zs, 0, 0.50f, aa);
                            V(x, 0, z - zs, 0, 0.75f, aa);
                            V(x, 0, z + zs, 1, 0.75f, aa);
                            V(x, 0, z + zs, 1, 0.75f, aa);
                            V(x, 0, z - zs, 0, 0.75f, aa);
                            V(x, +(th - ths) * sf, z - zs, 0, 1.00f, aa);
                            V(x, +(th + ths) * sf, z + zs, 1, 1.00f, aa);
                        }
                    }
                }
            }
        }
    } else {
        //main section
        V(0, 0, invfadelen, leftex, 0.5f, invfadealpha);
        V(0, thick, invfadelen, leftex, 1, peralpha * invfadealpha);
        V(0, thick, fadelen, fadetex, 1, peralpha * fadealpha);
        V(0, 0, fadelen, fadetex, 0.5f, fadealpha);
        V(0, 0, invfadelen, leftex, 0.5f, invfadealpha);
        V(0, 0, fadelen, fadetex, 0.5f, fadealpha);
        V(0, -thick, fadelen, fadetex, 0, peralpha * fadealpha);
        V(0, -thick, invfadelen, leftex, 0, peralpha * invfadealpha);
        //fade out
        V(0, 0, fadelen, fadetex, 0.5f, fadealpha);
        V(0, thick, fadelen, fadetex, 1, peralpha * fadealpha);
        V(0, thick, len, righttex, 1, peralpha * endalpha);
        V(0, 0, len, righttex, 0.5f, endalpha);
        V(0, 0, fadelen, fadetex, 0.5f, fadealpha);
        V(0, 0, len, righttex, 0.5f, endalpha);
        V(0, -thick, len, righttex, 0, peralpha * endalpha);
        V(0, -thick, fadelen, fadetex, 0, peralpha * fadealpha);
        //fade in
        V(0, 0, invfadelen, leftex, 0.5f, invfadealpha);
        V(0, thick, invfadelen, leftex, 1, peralpha * invfadealpha);
        V(0, thick, 0, touchtex, 1, peralpha);
        V(0, 0, 0, touchtex, 0.5f, 1.0f);
        V(0, 0, invfadelen, leftex, 0.5f, invfadealpha);
        V(0, 0, 0, touchtex, 0.5f, 1.0f);
        V(0, -thick, 0, touchtex, 0, peralpha);
        V(0, -thick, invfadelen, leftex, 0, peralpha * invfadealpha);
        //copy and rotate xy plane
        for (int i = 0, upto = a; i < upto; i++, a++) {
            beam[a] = beam[i];
            float aux = beam[a].x;
            beam[a].x = beam[a].y;
            beam[a].y = aux;
        }
    }
    vlist->EndMutate(a);
}

#undef V

void Beam::CollideHuge(const LineCollide &lc, Unit *targetToCollideWith, Unit *firer, Unit *superunit) {
    QVector x0 = center;
    QVector v = direction * curlength;
    if (is_null(superunit->location[Unit::UNIT_ONLY]) && curlength) {
        if (targetToCollideWith) {
            this->Collide(targetToCollideWith, firer, superunit);
        }
    } else if (curlength) {
        CollideMap *cm = _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY];

        CollideMap::iterator superloc = superunit->location[Unit::UNIT_ONLY];
        CollideMap::iterator tmore = superloc;
        if (!cm->Iterable(superloc)) {
            CollideArray::CollidableBackref *br = static_cast< CollideArray::CollidableBackref * > (superloc);
            CollideMap::iterator tmploc = cm->begin() + br->toflattenhints_offset;
            if (tmploc == cm->end()) {
                tmploc--;
            }
            tmore = superloc = tmploc;             //don't decrease tless
        } else {
            ++tmore;
        }
        double r0 = x0.i;
        double r1 = x0.i + v.i;
        double minlook = r0 < r1 ? r0 : r1;
        double maxlook = r0 < r1 ? r1 : r0;
        bool targcheck = false;
        maxlook += (maxlook - (*superunit->location[Unit::UNIT_ONLY])->getKey())
                + 2 * curlength;           //double damage, yo
        minlook += (minlook - (*superunit->location[Unit::UNIT_ONLY])->getKey()) - 2 * curlength * curlength;
        //(a+2*b)^2-(a+b)^2 = 3b^2+2ab = 2b^2+(a+b)^2-a^2
        if (superloc != cm->begin()
                && minlook < (*superunit->location[Unit::UNIT_ONLY])->getKey()) {
            //less traversal
            CollideMap::iterator tless = superloc;
            --tless;
            while ((*tless)->getKey() >= minlook) {
                CollideMap::iterator curcheck = tless;
                bool breakit = false;
                if (tless != cm->begin()) {
                    --tless;
                } else {
                    breakit = true;
                }
                if ((*curcheck)->radius > 0) {
                    if (beamCheckCollision(center, curlength, (**curcheck))) {
                        Unit *tmp = (**curcheck).ref.unit;
                        this->Collide(tmp, firer, superunit);
                        targcheck = (targcheck || tmp == targetToCollideWith);
                    }
                }
                if (breakit) {
                    break;
                }
            }
        }
        if (maxlook > (*superunit->location[Unit::UNIT_ONLY])->getKey()) {
            //greater traversal
            while (tmore != cm->end() && (*tmore)->getKey() <= maxlook) {
                if ((*tmore)->radius > 0) {
                    Unit *un = (*tmore)->ref.unit;
                    if (beamCheckCollision(center, curlength, **tmore++)) {
                        this->Collide(un, firer, superunit);
                        targcheck = (targcheck || un == targetToCollideWith);
                    }
                } else {
                    ++tmore;
                }
            }
        }
        if (targetToCollideWith && !targcheck) {
            this->Collide(targetToCollideWith, firer, superunit);
        }
    }
}

/*
 * Constructors
 */
Beam::Beam(const Transformation &trans, const WeaponInfo &clne, void *own, Unit *firer, int sound)
        : vlist(NULL), Col(clne.r, clne.g, clne.b, clne.a) {
    listen_to_owner = false;
#ifdef PERBOLTSOUND
    sound = AUDCreateSound( clne.sound, true );
#else
    this->sound = sound;
#endif
    decal = beamdecals.AddTexture(clne.file.c_str(), TRILINEAR);
    if (decal >= beamdrawqueue.size()) {
        beamdrawqueue.push_back(vector<BeamDrawContext>());
    }

    //Init( trans, clne, own, firer );
    CollideInfo.type = LineCollide::BEAM;

    local_transformation = trans;     //location on ship
    speed = clne.speed;
    texturespeed = clne.pulse_speed;
    range = clne.range;
    radialspeed = clne.radial_speed;
    thickness = clne.radius;
    stability = clne.stability;
    rangepenalty = clne.long_range;
    damagerate = clne.damage;
    phasedamage = clne.phase_damage;
    texturestretch = clne.texture_stretch;
    refiretime = 0;
    refire = clne.Refire();
    Col.r = clne.r;
    Col.g = clne.g;
    Col.b = clne.b;
    Col.a = clne.a;
    impact = ALIVE;
    owner = own;
    numframes = 0;
    static int radslices = XMLSupport::parse_int(vs_config->getVariable("graphics", "tractor.scoop_rad_slices", "10"))
            | 1;    //Must be odd
    static int
            longslices = XMLSupport::parse_int(vs_config->getVariable("graphics", "tractor.scoop_long_slices", "10"));
    lastlength = 0;
    curlength = simulation_atom_var * speed;
    lastthick = 0;
    curthick = simulation_atom_var * radialspeed;
    if (curthick > thickness) {      //clamp to max thickness - needed for large simulation atoms
        curthick = thickness;
    }
    static GFXVertexList *_vlist = 0;
    if (!_vlist) {
        int numvertex = float_to_int(std::max(48, ((4 * radslices) + 1) * longslices * 4));
        GFXColorVertex *beam =
                new GFXColorVertex[numvertex];         //regretably necessary: radslices and longslices come from the config file... so it's at runtime.
//        memset( beam, 0, sizeof (*beam)*numvertex );
        _vlist = new GFXVertexList(GFXQUAD, numvertex, beam, numvertex, true);         //mutable color contained list
        delete[] beam;
    }
    //Shared vlist - we recalculate it every time, so no loss
    vlist = _vlist;
#ifdef PERBOLTSOUND
    AUDStartPlaying( sound );
#endif

    impact = UNSTABLE;
}

Beam::~Beam() {
    VSDESTRUCT2
#ifdef PERBOLTSOUND
    AUDDeleteSound( sound );
#endif
#ifdef BEAMCOLQ
    RemoveFromSystem( true );
#endif
    //DO NOT DELETE - shared vlist
    //delete vlist;
}

// Called everytime the beam is fired
// Used to be Init
void Beam::Reinitialize() {
    CollideInfo.object.b = nullptr;
    CollideInfo.type = LineCollide::BEAM;

    impact = ALIVE;
    numframes = 0;
    static int radslices = XMLSupport::parse_int(vs_config->getVariable("graphics", "tractor.scoop_rad_slices", "10"))
            | 1;    //Must be odd
    static int
            longslices = XMLSupport::parse_int(vs_config->getVariable("graphics", "tractor.scoop_long_slices", "10"));
    lastlength = 0;
    curlength = simulation_atom_var * speed;
    lastthick = 0;
    curthick = simulation_atom_var * radialspeed;
    if (curthick > thickness) {      //clamp to max thickness - needed for large simulation atoms
        curthick = thickness;
    }
    static GFXVertexList *_vlist = 0;
    if (!_vlist) {
        int numvertex = float_to_int(std::max(48, ((4 * radslices) + 1) * longslices * 4));
        GFXColorVertex *beam =
                new GFXColorVertex[numvertex];         //regretably necessary: radslices and longslices come from the config file... so it's at runtime.
//        memset( beam, 0, sizeof (*beam)*numvertex );
        _vlist = new GFXVertexList(GFXQUAD, numvertex, beam, numvertex, true);         //mutable color contained list
        delete[] beam;
    }
    //Shared vlist - we recalculate it every time, so no loss
    vlist = _vlist;
#ifdef PERBOLTSOUND
    AUDStartPlaying( sound );
#endif
}

/*
 * Public Methods
 */

bool Beam::Collide(Unit *target, Unit *firer, Unit *superunit) {
    if (target == NULL) {
        VS_LOG(error, "Recovering from nonfatal beam error when beam inactive\n");
        return false;
    }
    float distance;
    Vector normal;     //apply shields

    QVector direction(this->direction.Cast());
    QVector end(center + direction.Scale(curlength));
    enum Vega_UnitType type = target->getUnitType();
    if (target == owner || type == Vega_UnitType::nebula || type == Vega_UnitType::asteroid) {
        const bool collideroids = configuration().physics.asteroid_weapon_collision;
        if (type != Vega_UnitType::asteroid || (!collideroids)) {
            return false;
        }
    }
    const bool collidejump = configuration().physics.jump_weapon_collision;
    if (type == Vega_UnitType::planet && (!collidejump) && !target->GetDestinations().empty()) {
        return false;
    }
    //A bunch of needed config variables - its best to have them here, so that they're loaded the
    //very first time Collide() is called. That way, we avoid hiccups.
    const float nbig = configuration().physics.tractor.percent_to_tractor_flt;
    int upgradesfaction = FactionUtil::GetUpgradeFaction();
    static int cargofaction = FactionUtil::GetFactionIndex("cargo");
    const bool c_fp = configuration().physics.tractor.cargo.force_push;
    const bool c_fi = configuration().physics.tractor.cargo.force_in;
    const bool u_fp = configuration().physics.tractor.upgrade.force_push;
    const bool u_fi = configuration().physics.tractor.upgrade.force_in;
    const bool f_fp = configuration().physics.tractor.faction.force_push;
    const bool f_fi = configuration().physics.tractor.faction.force_in;
    const bool d_fp = configuration().physics.tractor.disabled.force_push;
    const bool d_fi = configuration().physics.tractor.disabled.force_in;
    const bool o_fp = configuration().physics.tractor.others.force_push;
    const bool o_fi = configuration().physics.tractor.others.force_in;
    const bool scoop = configuration().physics.tractor.scoop;
    const float scoopangle = configuration().physics.tractor.scoop_angle_flt;     //In radians
    const float scoopcosangle = cos(scoopangle);
    const float maxrelspeed = configuration().physics.tractor.max_relative_speed;
    const float c_ors_m = configuration().physics.tractor.cargo.distance_own_rsize_flt;
    const float c_trs_m = configuration().physics.tractor.cargo.distance_tgt_rsize_flt;
    const float c_o = configuration().physics.tractor.cargo.distance_flt;
    const float u_ors_m = configuration().physics.tractor.upgrade.distance_own_rsize_flt;
    const float u_trs_m = configuration().physics.tractor.upgrade.distance_tgt_rsize_flt;
    const float u_o = configuration().physics.tractor.upgrade.distance_flt;
    const float f_ors_m = configuration().physics.tractor.faction.distance_own_rsize_flt;
    const float f_trs_m = configuration().physics.tractor.faction.distance_tgt_rsize_flt;
    const float f_o = configuration().physics.tractor.faction.distance_flt;
    const float o_ors_m = configuration().physics.tractor.others.distance_own_rsize_flt;
    const float o_trs_m = configuration().physics.tractor.others.distance_tgt_rsize_flt;
    const float o_o = configuration().physics.tractor.others.distance_flt;
    bool tractor = (damagerate < 0 && phasedamage > 0);
    bool repulsor = (damagerate > 0 && phasedamage < 0);
    if (scoop && (tractor || repulsor)) {
        QVector d2(target->Position() - center);
        d2.Normalize();
        float angle = this->direction * d2;
        if (angle > scoopcosangle) {
            end = center + d2 * curlength;
            direction = end - center;
            direction.Normalize();
        }
    }
    Unit *colidee;
    if ((colidee = target->rayCollide(center, end, normal, distance))) {
        if (!(scoop && (tractor || repulsor))) {
            this->curlength = distance;
        }
        float curlength = distance;
        impact |= IMPACT;
        GFXColor coltmp(Col);
        float tmp = (curlength / range);
        float appldam = (damagerate * simulation_atom_var * curthick / thickness) * ((1 - tmp) + tmp * rangepenalty);
        float phasdam = (phasedamage * simulation_atom_var * curthick / thickness) * ((1 - tmp) + tmp * rangepenalty);
        float owner_rsize = superunit->rSize();
        int owner_faction = superunit->faction;
        if (tractor || repulsor) {
            bool fp = o_fp, fi = o_fi;
            if (target->faction == owner_faction) {
                fp = f_fp, fi = f_fi;
            } else if (target->faction == upgradesfaction) {
                fp = u_fp, fi = u_fi;
            } else if (target->faction == cargofaction) {
                fp = c_fp, fi = c_fi;
            } else if (target->getAIState() == NULL) {
                fp = d_fp, fi = d_fi;
            }
            //tractor/repulsor beam!
            if (fp || target->isTractorable(Unit::tractorPush)) {
                //Compute relative speed - if it's higher than the maximum, don't accelerate it anymore
                //FIXME: Should predict the resulting velocity after applying the force,
                //and adjust the force to match the maximum relative velocity - but the
                //heterogeneous physics granularity makes it quite hard (it's not owr
                //own priority the one counting, but the target's).
                //The current hack - using the target's sim_atom_multiplier, only prevents
                //aberrations from becoming obvious, but it's not entirely correct.
                float relspeed = target->GetVelocity() * direction.Cast();
                if (relspeed < maxrelspeed) {
                    //Modulate force on little mass objects, so they don't slingshot right past you
                    target->ApplyForce(direction
                            * (appldam
                                    / sqrt( /*(target->sim_atom_multiplier
                                                 > 0) ? target->sim_atom_multiplier : */ 1.0)
                                    * std::min(1.0, target->GetMass())));
                }
            }
            float ors_m = o_ors_m, trs_m = o_trs_m, ofs = o_o;
            if (target->faction == owner_faction) {
                ors_m = f_ors_m, trs_m = f_trs_m, ofs = f_o;
            } else if (target->faction == upgradesfaction) {
                ors_m = u_ors_m, trs_m = u_trs_m, ofs = u_o;
            } else if (target->faction == cargofaction) {
                ors_m = c_ors_m, trs_m = c_trs_m, ofs = c_o;
            }
            if ((fi
                    || target->isTractorable(Unit::tractorIn))
                    && ((center - target->Position()).Magnitude()
                            < (ors_m * owner_rsize + trs_m * target->rSize() + ofs))) {
                Unit *un = superunit;
                if (target->faction == upgradesfaction || owner_rsize * nbig > target->rSize()) {
                    //we have our man!
                    //lets add our cargo to him

                    Cargo *c = nullptr;
                    try {
                        Cargo cargo = Manifest::MPL().GetCargoByName(target->name.get());
                        c = &cargo;
                    } catch (const std::exception& e) {}
                     
                    Cargo tmp;
                    bool isnotcargo = (c == NULL);
                    if (!isnotcargo) {
                        if (c->IsComponent()) {
                            isnotcargo = true;
                        }
                    }
                    //add upgrades as space junk
                    if (isnotcargo) {
                        c = &tmp;
                        tmp.SetName("Space_Salvage");
                        tmp.SetCategory("Uncategorized_Cargo");
                        const float spacejunk = configuration().cargo.space_junk_price_flt;
                        tmp.SetPrice(spacejunk);
                        tmp.SetQuantity(1);
                        tmp.SetMass(.001);
                        tmp.SetVolume(1);
                        if (target->faction != upgradesfaction) {
                            tmp.SetName(target->name);
                            tmp.SetCategory("starships");
                            const float starshipprice = configuration().cargo.junk_starship_price_flt;
                            const float starshipmass = configuration().cargo.junk_starship_mass_flt;
                            const float starshipvolume = configuration().cargo.junk_starship_volume_flt;
                            tmp.SetPrice(starshipprice);
                            tmp.SetQuantity(1);
                            tmp.SetMass(starshipmass);
                            tmp.SetVolume(starshipvolume);
                        }
                    }
                    if (c != NULL) {
                        Cargo adder = *c;
                        adder.SetQuantity(1);
                        if (un->cargo_hold.CanAddCargo(adder)) {
                            un->cargo_hold.AddCargo(un, adder);
                            if (un->IsPlayerShip()) {
                                static boost::optional<int> tractor_onboard;
                                if (tractor_onboard == boost::none) {
                                    tractor_onboard = AUDCreateSoundWAV(configuration().audio.unit_audio.player_tractor_cargo);
                                }
                                AUDPlay(tractor_onboard.get(), QVector(0, 0, 0), Vector(0, 0, 0), 1);
                            } else {
                                Unit *tmp = _Universe->AccessCockpit()->GetParent();
                                if (tmp && tmp->owner == un) {
                                    //Subunit of player (a turret)
                                    static boost::optional<int> tractor_onboard_fromturret;
                                    if (tractor_onboard_fromturret == boost::none) {
                                        tractor_onboard_fromturret = AUDCreateSoundWAV(configuration().audio.unit_audio.player_tractor_cargo_fromturret);
                                    }
                                    AUDPlay(tractor_onboard_fromturret.get(), QVector(0, 0, 0), Vector(0, 0, 0), 1);
                                }
                            }
                            target->Kill();
                            target = nullptr;
                        }
                    }
                }
            }
        } else {
            Damage damage(appldam, phasdam);
            target->ApplyDamage(center.Cast() + direction * curlength, normal, damage, colidee, coltmp, owner);
        }
        return true;
    }
    return false;
}

void Beam::Destabilize() {
    impact = UNSTABLE;
}

bool Beam::Dissolved() {
    return curthick == 0;
}

void Beam::Draw(const Transformation &trans, const Matrix &m, Unit *targ, float tracking_cone) {
    //hope that the correct transformation is on teh stack
    if (curthick == 0) {
        return;
    }
    Matrix cumulative_transformation_matrix;
    local_transformation.to_matrix(cumulative_transformation_matrix);
    Transformation cumulative_transformation = local_transformation;
    cumulative_transformation.Compose(trans, m);
    cumulative_transformation.to_matrix(cumulative_transformation_matrix);
    AdjustMatrixToTrackTarget(cumulative_transformation_matrix, Vector(0, 0, 0), targ, speed, false, tracking_cone);
#ifdef PERFRAMESOUND
    AUDAdjustSound( sound, cumulative_transformation.position, speed
                   *Vector( cumulative_transformation_matrix[8], cumulative_transformation_matrix[9],
                            cumulative_transformation_matrix[10] ) );
#endif
    AUDSoundGain(sound, curthick * curthick / (thickness * thickness));

    beamdrawqueue[decal].push_back(BeamDrawContext(cumulative_transformation_matrix, vlist, this));
}

QVector Beam::GetPosition() const {
    return local_transformation.position;
}

void Beam::ListenToOwner(bool listen) {
    listen_to_owner = listen;
}

void Beam::ProcessDrawQueue() {
    GFXDisable(LIGHTING);
    GFXDisable(CULLFACE);     //don't want lighting on this baby
    GFXDisable(DEPTHWRITE);
    GFXPushBlendMode();
    const bool blendbeams = configuration().graphics.blend_guns;
    GFXBlendMode(ONE, blendbeams ? ONE : ZERO);

    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    BeamDrawContext c;
    for (unsigned int decal = 0; decal < beamdrawqueue.size(); decal++) {
        Texture *tex = beamdecals.GetTexture(decal);
        if (tex) {
            tex->MakeActive(0);
            GFXTextureEnv(0, GFXMODULATETEXTURE);
            GFXToggleTexture(true, 0);
            if (beamdrawqueue[decal].size()) {
                while (beamdrawqueue[decal].size()) {
                    c = beamdrawqueue[decal].back();
                    beamdrawqueue[decal].pop_back();

                    c.beam->RecalculateVertices(c.m);
                    GFXLoadMatrixModel(c.m);
                    c.vlist->DrawOnce();
                }
            }
        }
    }
    GFXEnable(DEPTHWRITE);
    GFXEnable(CULLFACE);
    GFXDisable(LIGHTING);
    GFXPopBlendMode();
}

bool Beam::Ready() {
    return curthick == 0 && refiretime > refire;
}

float Beam::refireTime() {
    return refiretime;
}

// TODO: this does nothing!
void Beam::RemoveFromSystem(bool eradicate) {
}

void Beam::SetPosition(const QVector &k) {
    local_transformation.position = k;
}

void Beam::SetOrientation(const Vector &p, const Vector &q, const Vector &r) {
    local_transformation.orientation = Quaternion::from_vectors(p, q, r);
}

void Beam::UpdatePhysics(const Transformation &trans,
        const Matrix &m,
        Unit *targ,
        float tracking_cone,
        Unit *targetToCollideWith,
        Unit *firer,
        Unit *superunit) {
    curlength += simulation_atom_var * speed;
    if (curlength < 0) {
        curlength = 0;
    }
    if (curlength > range) {
        curlength = range;
    }
    if (curthick == 0) {
        if (AUDIsPlaying(sound) && refiretime >= simulation_atom_var) {
            AUDStopPlaying(sound);
        }
        refiretime += simulation_atom_var;
        return;
    }
    if (stability && numframes * simulation_atom_var > stability) {
        impact |= UNSTABLE;
    }
    numframes++;
    Matrix cumulative_transformation_matrix;
    Transformation cumulative_transformation = local_transformation;
    cumulative_transformation.Compose(trans, m);
    cumulative_transformation.to_matrix(cumulative_transformation_matrix);
    bool possible = AdjustMatrix(cumulative_transformation_matrix, Vector(0, 0, 0), targ, speed, false, tracking_cone);
    const bool firemissingautotrackers = configuration().physics.fire_missing_autotrackers;
    if (targ && possible == false && !firemissingautotrackers) {
        Destabilize();
    }
    //to help check for crashing.
    center = cumulative_transformation.position;
    direction = TransformNormal(cumulative_transformation_matrix, Vector(0, 0, 1));
#ifndef PERFRAMESOUND
    AUDAdjustSound(sound, cumulative_transformation.position, speed * cumulative_transformation_matrix.getR());
#endif
    curthick += (impact & UNSTABLE) ? -radialspeed * simulation_atom_var : radialspeed * simulation_atom_var;
    if (curthick > thickness) {
        curthick = thickness;
    }
    if (curthick <= 0) {
        curthick = 0; //die die die
#ifdef BEAMCOLQ
        RemoveFromSystem( false );
#endif
    } else {
        CollideHuge(CollideInfo, listen_to_owner ? targetToCollideWith : NULL, firer, superunit);
        if (!(curlength <= range && curlength > 0)) {
            //if curlength just happens to be nan --FIXME THIS MAKES NO SENSE AT ALL --chuck_starchaser
            if (curlength > range) {
                curlength = range;
            } else {
                curlength = 0;
            }
        }
        QVector tmpvec(center + direction.Cast().Scale(curlength));
        QVector tmpMini = center.Min(tmpvec);
        tmpvec = center.Max(tmpvec);
#ifdef BEAMCOLQ
        if ( TableLocationChanged( CollideInfo, tmpMini, tmpvec ) || (curthick > 0 && CollideInfo.object.b == NULL) ) {
            RemoveFromSystem( false );
#endif
        CollideInfo.object.b = this;
        CollideInfo.hhuge =
                (((CollideInfo.Maxi.i
                        - CollideInfo.Mini.i)
                        / coltableacc)
                        * ((CollideInfo.Maxi.j - CollideInfo.Mini.j) / coltableacc)
                        * (CollideInfo.Maxi.k - CollideInfo.Mini.k) / coltableacc
                        > tablehuge);
        CollideInfo.Mini = tmpMini;
        CollideInfo.Maxi = tmpvec;
#ifdef BEAMCOLQ
        AddCollideQueue( CollideInfo );
    } else {
        CollideInfo.Mini = tmpMini;
        CollideInfo.Maxi = tmpvec;
    }
#endif
    }
    //Check if collide...that'll change max beam length REAL quick
}
