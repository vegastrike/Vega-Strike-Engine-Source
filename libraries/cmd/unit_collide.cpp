/**
 * unit_collide.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


/// Unit mesh collision detection
/// Provides various functions for collision detection

#include "src/vegastrike.h"
#include "cmd/beam.h"

#include "cmd/bolt.h"
#include "gfx_generic/mesh.h"
#include "cmd/unit_collide.h"
#include "src/physics.h"

#include "collide2/CSopcodecollider.h"
#include "collide2/csgeom2/optransfrm.h"
#include "collide2/basecollider.h"

#include "src/hashtable.h"

#include <string>
#include "root_generic/vs_globals.h"
#include "root_generic/configxml.h"
#include "cmd/collide.h"
#include "src/vs_logging.h"

#include "cmd/collision.h"
#include "src/universe.h"

static bool operator==(const Collidable &a, const Collidable &b) {
    return memcmp(&a, &b, sizeof(Collidable)) == 0;
}

void Unit::RemoveFromSystem() {
    for (unsigned int locind = 0; locind < NUM_COLLIDE_MAPS; ++locind) {
        if (!is_null(this->location[locind])) {
            if (activeStarSystem == NULL) {
                VS_LOG(error, "NONFATAL NULL activeStarSystem detected...please fix");
                activeStarSystem = _Universe->activeStarSystem();
            }
            static bool collidemap_sanity_check =
                    XMLSupport::parse_bool(vs_config->getVariable("physics", "collidemap_sanity_check", "false"));
            if (collidemap_sanity_check) {
                if (0) {
                    CollideMap::iterator i;
                    CollideMap::iterator j = activeStarSystem->collide_map[locind]->begin();

                    bool found = false;
                    for (i = activeStarSystem->collide_map[locind]->begin();
                            i != activeStarSystem->collide_map[locind]->end(); ++i) {
                        if (i == this->location[locind]) {
                            VS_LOG(info, (boost::format("hussah %1$b") % (*i == *this->location[locind])));
                            found = true;
                        }
                        if (**i < **j) {
                            VS_LOG(warning, (boost::format("(%1$f %2$f %3$f) and (%4$f %5$f %6$f) %7$f < %8$f %9$b!!!")
                                    % (**i).GetPosition().i
                                    % (**i).GetPosition().j
                                    % (**i).GetPosition().k
                                    % (**j).GetPosition().i
                                    % (**j).GetPosition().j
                                    % (**j).GetPosition().k
                                    % (**i).GetPosition().MagnitudeSquared()
                                    % (**j).GetPosition().MagnitudeSquared()
                                    % ((**i).GetPosition().MagnitudeSquared()
                                            < (**j).GetPosition().MagnitudeSquared())));
                        }
                        j = i;
                    }
                    VS_LOG(info, (boost::format("fin %1$p %2$b ") % (*(int *) &i) % found));
                    activeStarSystem->collide_map[locind]->checkSet();
                    assert(0);
                }
            }
            activeStarSystem->collide_map[locind]->erase(this->location[locind]);
            set_null(this->location[locind]);
        }
    }
    for (int j = 0; j < getNumMounts(); ++j) {
        if (mounts[j].type->type == WEAPON_TYPE::BEAM) {
            if (mounts[j].ref.gun) {
                mounts[j].ref.gun->RemoveFromSystem(true);
            }
        }
    }
    activeStarSystem = NULL;
}

void Unit::UpdateCollideQueue(StarSystem *ss, CollideMap::iterator hint[NUM_COLLIDE_MAPS]) {
    if (activeStarSystem == NULL) {
        activeStarSystem = ss;
    } else
        assert(activeStarSystem == ss);
    for (unsigned int locind = 0; locind < NUM_COLLIDE_MAPS; ++locind) {
        if (is_null(location[locind])) {
            assert(!isSubUnit());
            if (!isSubUnit()) {
                location[locind] = ss->collide_map[locind]->insert(Collidable(this), hint[locind]);
            }
        }
    }
}

void Unit::CollideAll() {
    if (isSubUnit() || killed || vega_config::config->physics.no_unit_collisions) {
        return;
    }
    for (unsigned int locind = 0; locind < NUM_COLLIDE_MAPS; ++locind) {
        if (is_null(this->location[locind])) {
            this->location[locind] = this->getStarSystem()->collide_map[locind]->insert(Collidable(this));
        }
    }
    CollideMap *cm = this->getStarSystem()->collide_map[Unit::UNIT_BOLT];
    cm->CheckCollisions(this, *this->location[Unit::UNIT_BOLT]);
}

Vector Vabs(const Vector &in) {
    return Vector(in.i >= 0 ? in.i : -in.i,
            in.j >= 0 ? in.j : -in.j,
            in.k >= 0 ? in.k : -in.k);
}

//Slated for removal 0.5
Matrix WarpMatrixForCollisions(Unit *un, const Matrix &ctm) {
    if (un->GetWarpVelocity().MagnitudeSquared() * simulation_atom_var * simulation_atom_var
            < un->rSize() * un->rSize()) {
        return ctm;
    } else {
        Matrix k(ctm);
        const Vector v(Vector(1, 1, 1) + Vabs(ctm.getR() * ctm.getR().Dot(un->GetWarpVelocity().Scale(
                100 * simulation_atom_var / un->rSize()))));
        k.r[0] *= v.i;
        k.r[1] *= v.j;
        k.r[2] *= v.k;

        k.r[3] *= v.i;
        k.r[4] *= v.j;
        k.r[5] *= v.k;

        k.r[6] *= v.i;
        k.r[7] *= v.j;
        k.r[8] *= v.k;
        return k;
    }
}

//do each of these bubbled subunits collide with the other unit?
bool Unit::Inside(const QVector &target, const float radius, Vector &normal, float &dist) {
    if (!querySphere(target, radius)) {
        return false;
    }
    normal = (target - Position()).Cast();
    ::Normalize(normal);
    //if its' in the sphre, that's enough
    if (isPlanet()) {
        return true;
    }
    return false;
}

bool Unit::InsideCollideTree(Unit *smaller,
        QVector &bigpos,
        Vector &bigNormal,
        QVector &smallpos,
        Vector &smallNormal,
        bool bigasteroid,
        bool smallasteroid) {
    if (smaller->colTrees == NULL || this->colTrees == NULL) {
        return false;
    }
    if (Destroyed()) {
        return false;
    }
    if (smaller->colTrees->usingColTree() == false || this->colTrees->usingColTree() == false) {
        return false;
    }
    csOPCODECollider::ResetCollisionPairs();
    Unit *bigger = this;

    csReversibleTransform bigtransform(bigger->cumulative_transformation_matrix);
    csReversibleTransform smalltransform(smaller->cumulative_transformation_matrix);
    smalltransform.SetO2TTranslation(csVector3(smaller->cumulative_transformation_matrix.p
            - bigger->cumulative_transformation_matrix.p));
    bigtransform.SetO2TTranslation(csVector3(0, 0, 0));
    //we're only gonna lerp the positions for speed here... gahh!

    // Check for shield collisions here prior to checking for mesh on mesh or ray collisions below.
    csOPCODECollider *tmpCol = smaller->colTrees->colTree(smaller, bigger->GetWarpVelocity());
    if (tmpCol
            && (tmpCol->Collide(*bigger->colTrees->colTree(bigger,
                            smaller->GetWarpVelocity()),
                    &smalltransform,
                    &bigtransform))) {
        csCollisionPair *mycollide = csOPCODECollider::GetCollisions();
        unsigned int numHits = csOPCODECollider::GetCollisionPairCount();
        if (numHits) {
            smallpos.Set((mycollide[0].a1.x + mycollide[0].b1.x + mycollide[0].c1.x) / 3.0f,
                    (mycollide[0].a1.y + mycollide[0].b1.y + mycollide[0].c1.y) / 3.0f,
                    (mycollide[0].a1.z + mycollide[0].b1.z + mycollide[0].c1.z) / 3.0f);
            smallpos = Transform(smaller->cumulative_transformation_matrix, smallpos);
            bigpos.Set((mycollide[0].a2.x + mycollide[0].b2.x + mycollide[0].c2.x) / 3.0f,
                    (mycollide[0].a2.y + mycollide[0].b2.y + mycollide[0].c2.y) / 3.0f,
                    (mycollide[0].a2.z + mycollide[0].b2.z + mycollide[0].c2.z) / 3.0f);
            bigpos = Transform(bigger->cumulative_transformation_matrix, bigpos);
            csVector3 sn, bn;
            sn.Cross(mycollide[0].b1 - mycollide[0].a1, mycollide[0].c1 - mycollide[0].a1);
            bn.Cross(mycollide[0].b2 - mycollide[0].a2, mycollide[0].c2 - mycollide[0].a2);
            sn.Normalize();
            bn.Normalize();
            smallNormal.Set(sn.x, sn.y, sn.z);
            bigNormal.Set(bn.x, bn.y, bn.z);
            smallNormal = TransformNormal(smaller->cumulative_transformation_matrix, smallNormal);
            bigNormal = TransformNormal(bigger->cumulative_transformation_matrix, bigNormal);
            return true;
        }
    }
    un_iter i;
    static float
            rsizelim = XMLSupport::parse_float(vs_config->getVariable("physics", "smallest_subunit_to_collide", ".2"));
    Vega_UnitType bigtype = bigasteroid ? Vega_UnitType::asteroid : bigger->isUnit();
    Vega_UnitType smalltype = smallasteroid ? Vega_UnitType::asteroid : smaller->isUnit();
    if (bigger->SubUnits.empty() == false
            && (bigger->graphicOptions.RecurseIntoSubUnitsOnCollision == true || bigtype == Vega_UnitType::asteroid)) {
        i = bigger->getSubUnits();
        float rad = smaller->rSize();
        for (Unit *un; (un = *i); ++i) {
            float subrad = un->rSize();
            if ((bigtype != Vega_UnitType::asteroid) && (subrad / bigger->rSize() < rsizelim)) {
                break;
            }
            if ((un->Position() - smaller->Position()).Magnitude() <= subrad + rad) {
                if ((un->InsideCollideTree(smaller,
                        bigpos,
                        bigNormal,
                        smallpos,
                        smallNormal,
                        bigtype == Vega_UnitType::asteroid,
                        smalltype == Vega_UnitType::asteroid))) {
                    return true;
                }
            }
        }
    }
    if (smaller->SubUnits.empty() == false
            && (smaller->graphicOptions.RecurseIntoSubUnitsOnCollision == true || smalltype == Vega_UnitType::asteroid)) {
        i = smaller->getSubUnits();
        float rad = bigger->rSize();
        for (Unit *un; (un = *i); ++i) {
            float subrad = un->rSize();
            if ((smalltype != Vega_UnitType::asteroid) && (subrad / smaller->rSize() < rsizelim)) {
                break;
            }
            if ((un->Position() - bigger->Position()).Magnitude() <= subrad + rad) {
                if ((bigger->InsideCollideTree(un,
                        bigpos,
                        bigNormal,
                        smallpos,
                        smallNormal,
                        bigtype == Vega_UnitType::asteroid,
                        smalltype == Vega_UnitType::asteroid))) {
                    return true;
                }
            }
        }
    }
    //FIXME
    //doesn't check all i*j options of subunits vs subunits
    return false;
}

inline float mysqr(float a) {
    return a * a;
}

bool Unit::Collide(Unit *target) {
    //now first OF ALL make sure they're within bubbles of each other...
    if ((Position() - target->Position()).MagnitudeSquared() > mysqr(radial_size + target->radial_size)) {
        return false;
    }
    Vega_UnitType targetisUnit = target->isUnit();
    Vega_UnitType thisisUnit = this->isUnit();
    static float
            NEBULA_SPACE_DRAG = XMLSupport::parse_float(vs_config->getVariable("physics", "nebula_space_drag", "0.01"));
    if (targetisUnit == Vega_UnitType::nebula) {
        //why? why not?
        this->Velocity *= (1 - NEBULA_SPACE_DRAG);
    }
    if (target == this
            || ((targetisUnit != Vega_UnitType::nebula
                    && thisisUnit != Vega_UnitType::nebula)
                    && (owner == target || target->owner == this
                            || (owner != NULL
                                    && target->owner == owner)))) {
        return false;
    }
    if (targetisUnit == Vega_UnitType::asteroid && thisisUnit == Vega_UnitType::asteroid) {
        return false;
    }
    std::multimap<Unit *, Unit *> *last_collisions = &_Universe->activeStarSystem()->last_collisions;
    last_collisions->insert(std::pair<Unit *, Unit *>(this, target));
    //unit v unit? use point sampling?
    if ((this->DockedOrDocking() & (DOCKED_INSIDE | DOCKED))
            || (target->DockedOrDocking() & (DOCKED_INSIDE | DOCKED))) {
        return false;
    }
    //now do some serious checks
    Unit *bigger;
    Unit *smaller;
    if (radial_size < target->radial_size) {
        bigger = target;
        smaller = this;
    } else {
        bigger = this;
        smaller = target;
    }
    bool usecoltree = (this->colTrees && target->colTrees)
            ? this->colTrees->colTree(this, Vector(0, 0, 0))
                    && target->colTrees->colTree(this, Vector(0, 0, 0))
            : false;
    if (usecoltree) {
        QVector bigpos, smallpos;
        Vector bigNormal, smallNormal;
        if (bigger->InsideCollideTree(smaller, bigpos, bigNormal, smallpos, smallNormal)) {
            if (!bigger->isDocked(smaller) && !smaller->isDocked(bigger)) {
                //bigger->reactToCollision( smaller, bigpos, bigNormal, smallpos, smallNormal, 10 );
                Collision::collide(bigger, bigpos, bigNormal, smaller, smallpos, smallNormal, 10);
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        Vector normal(-1, -1, -1);
        float dist = 0.0;
        if (bigger->Inside(smaller->Position(), smaller->rSize(), normal, dist)) {
            if (!bigger->isDocked(smaller) && !smaller->isDocked(bigger)) {
                //bigger->reactToCollision( smaller, bigger->Position(), normal, smaller->Position(), -normal, dist );
                Collision::collide(bigger, bigger->Position(), normal, smaller,
                        smaller->Position(), -normal, dist);
            } else {
                return false;
            }
        } else {
            return (false);
        }

    }
    return true;
}

float globQueryShell(QVector st, QVector dir, float radius) {
    float temp1 = radius;
    float a, b, c;
    c = st.Dot(st);
    c = c - temp1 * temp1;
    b = 2.0f * (dir.Dot(st));
    a = dir.Dot(dir);
    //b^2-4ac
    c = b * b - 4.0f * a * c;
    if (c < 0 || a == 0) {
        return 0.0f;
    }
    a *= 2.0f;

    float tmp = (-b + sqrt(c)) / a;
    c = (-b - sqrt(c)) / a;
    if (tmp > 0 && tmp <= 1) {
        return (c > 0 && c < tmp) ? c : tmp;
    } else if (c > 0 && c <= 1) {
        return c;
    }
    return 0.0f;
}

float globQuerySphere(QVector start, QVector end, QVector pos, float radius) {
    QVector st = start - pos;
    const double radius_temp = radius;
    if (st.MagnitudeSquared() < radius_temp * radius_temp) {
        return 1.0e-6f;
    }
    return globQueryShell(st, end - start, radius);
}

/*
    * This is our ray / bolt collision routine for now.
    * Basically, this is called on a ship unit to see if any ray or bolt given by some simple vectors collide with it
    *  We should probably first check against shields and then against the colTree to see if we hit the shields first
    *  Not sure yet if that would work though...  more importantly, we might have to modify end in here in order
    *  to tell calling code that the bolt should stop at a given point.
*/
Unit *Unit::rayCollide(const QVector &start, const QVector &end, Vector &norm, float &distance) {
    Unit *tmp;
    float rad = this->rSize();
    if ((!SubUnits.empty()) && graphicOptions.RecurseIntoSubUnitsOnCollision) {
        if ((tmp = *SubUnits.fastIterator())) {
            rad += tmp->rSize();
        }
    }
    if (!globQuerySphere(start, end, cumulative_transformation_matrix.p, rad)) {
        return NULL;
    }
    if (graphicOptions.RecurseIntoSubUnitsOnCollision) {
        if (!SubUnits.empty()) {
            un_fiter i(SubUnits.fastIterator());
            for (Unit *un; (un = *i); ++i) {
                if ((tmp = un->rayCollide(start, end, norm, distance)) != 0) {
                    return tmp;
                }
            }
        }
    }
    QVector st(InvTransform(cumulative_transformation_matrix, start));
    QVector ed(InvTransform(cumulative_transformation_matrix, end));
    static bool sphere_test = XMLSupport::parse_bool(vs_config->getVariable("physics", "sphere_collision", "true"));
    distance = querySphereNoRecurse(start, end);
    if (distance > 0.0f || (this->colTrees && this->colTrees->colTree(this, this->GetWarpVelocity()) && !sphere_test)) {
        Vector coord;
        /* Set up points and ray to send to ray collider. */
        Opcode::Point rayOrigin(st.i, st.j, st.k);
        Opcode::Point rayDirection(ed.i, ed.j, ed.k);
        Opcode::Ray boltbeam(rayOrigin, rayDirection);
        if (this->colTrees) {
            // Retrieve the correct scale'd collider from the unit's collide tree.
            csOPCODECollider *tmpCol = this->colTrees->colTree(this, this->GetWarpVelocity());
            QVector del(end - start);
            //Normalize(del);
            norm = ((start + del * distance) - Position()).Cast();
            Normalize(norm);
            //RAY COLLIDE does not yet set normal, use that of the sphere center to current loc
            if (tmpCol == NULL) {

                return this;
            }
            if (tmpCol->rayCollide(boltbeam, norm, distance)) {
                // compute real distance
                distance = (end - start).Magnitude() * distance;

                // NOTE:   Here is where we need to retrieve the point on the ray that we collided with the mesh, and set it to end, create the normal and set distance
                VS_LOG(trace, (boost::format("Beam collide with %1$p, distance %2%") % this % distance));
                return (this);
            }
        } else {//no col trees = a sphere
            // compute real distance
            distance = (end - start).Magnitude() * distance;

            VS_LOG(trace, (boost::format("Beam collide with %1$p, distance %2%") % this % distance));
            return (this);
        }
    } else {
        return (NULL);
    }
    return (NULL);
}

bool Unit::querySphere(const QVector &pnt, float err) const {
    unsigned int i;
    const Matrix *tmpo = &cumulative_transformation_matrix;

    Vector TargetPoint(tmpo->getP());
#ifdef VARIABLE_LENGTH_PQR
    //adjust the ship radius by the scale of local coordinates
    double SizeScaleFactor = sqrt( TargetPoint.Dot( TargetPoint ) );
#endif
    if (nummesh() < 1 && isPlanet()) {
        TargetPoint = (tmpo->p - pnt).Cast();
        if (TargetPoint.Dot(TargetPoint)
                < err * err
                        + radial_size * radial_size
#ifdef VARIABLE_LENGTH_PQR
                        *SizeScaleFactor*SizeScaleFactor
#endif
                        +
#ifdef VARIABLE_LENGTH_PQR
                                SizeScaleFactor*
#endif
                                2.0f * err * radial_size
                ) {
            return true;
        }
    } else {
        for (i = 0; i < nummesh(); i++) {
            TargetPoint = (Transform(*tmpo, meshdata[i]->Position().Cast()) - pnt).Cast();
            if (TargetPoint.Dot(TargetPoint)
                    < err * err
                            + meshdata[i]->rSize() * meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
                            *SizeScaleFactor*SizeScaleFactor
#endif
                            +
#ifdef VARIABLE_LENGTH_PQR
                                    SizeScaleFactor*
#endif
                                    2.0f * err * meshdata[i]->rSize()
                    ) {
                return true;
            }
        }
    }
    if (graphicOptions.RecurseIntoSubUnitsOnCollision) {
        if (!SubUnits.empty()) {
            un_fkiter i = SubUnits.constFastIterator();
            for (const Unit *un; (un = *i); ++i) {
                if ((un)->querySphere(pnt, err)) {
                    return true;
                }
            }
        }
    }
    return false;
}

float Unit::querySphere(const QVector &start, const QVector &end, float min_radius) const {
    if (!SubUnits.empty()) {
        un_fkiter i = SubUnits.constFastIterator();
        for (const Unit *un; (un = *i); ++i) {
            float tmp;
            if ((tmp = un->querySphere(start, end, min_radius)) != 0) {
                return tmp;
            }
        }
        if (nummesh()) {
            return querySphereNoRecurse(start, end, min_radius);
        } else {
            return 0.0f;
        }
    } else {
        return querySphereNoRecurse(start, end, min_radius);
    }
}

//does not check inside sphere
float Unit::querySphereNoRecurse(const QVector &start, const QVector &end, float min_radius) const {
    unsigned int i;
    double tmp;
    for (i = 0; i < nummesh(); i++) {
        if ((meshdata[i]->Position().Magnitude() > this->rSize()) || (meshdata[i]->rSize() > 30 + this->rSize())) {
            continue;
        }
        if (isUnit() == Vega_UnitType::planet && i > 0) {
            break;
        }
        double a, b, c;
        QVector st = start - Transform(cumulative_transformation_matrix, meshdata[i]->Position().Cast());

        QVector dir = end - start;         //now start and end are based on mesh's position
        c = st.Dot(st);
        double temp1 = (min_radius + meshdata[i]->rSize());
        //if (st.MagnitudeSquared()<temp1*temp1) //UNCOMMENT if you want inside sphere to count...otherwise...
        //return 1.0e-6;
        if (min_radius != -FLT_MAX) {
            c = c - temp1 * temp1;
        } else {
            c = temp1;
        }
#ifdef VARIABLE_LENGTH_PQR
        c *= SizeScaleFactor*SizeScaleFactor;
#endif
        b = 2.0f * (dir.Dot(st));
        a = dir.Dot(dir);
        //b^2-4ac
        if (min_radius != -FLT_MAX) {
            c = b * b - 4.0f * a * c;
        } else {
            c = FLT_MAX;
        }
        if (c < 0 || a == 0) {
            continue;
        }
        a *= 2.0f;

        tmp = (-b + sqrt(c)) / a;
        c = (-b - sqrt(c)) / a;
        if (tmp > 0 && tmp <= 1) {
            return (c > 0 && c < tmp) ? c : tmp;
        } else if (c > 0 && c <= 1) {
            return c;
        }
    }
    return 0.0f;
}
