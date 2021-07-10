/**
 * collide.cpp
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
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


#include "cmd/collide.h"
#include "vegastrike.h"
#include "unit_generic.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "universe.h"

#include "collide2/CSopcodecollider.h"
#include "collide2/csgeom2/optransfrm.h"
#include "collide2/basecollider.h"

#include "hashtable.h"
#include <string>
#include "vs_globals.h"
#include "configxml.h"
#include "vsfilesystem.h"

static Hashtable< std::string, collideTrees, 127 >unitColliders;
collideTrees::collideTrees( const std::string &hk, csOPCODECollider *cT,
                            csOPCODECollider *cS ) : hash_key( hk )
    , colShield( cS )
{
    for (unsigned int i = 0; i < collideTreesMaxTrees; ++i) {
        rapidColliders[i] = nullptr;
    }
    rapidColliders[0] = cT;

    refcount = 1;
    unitColliders.Put( hash_key, this );
}
float loge2 = log( 2.f );

csOPCODECollider* collideTrees::colTree( Unit *un, const Vector &othervelocity )
{
    const float const_factor = 1;
    float magsqr = un->GetVelocity().MagnitudeSquared();
    float newmagsqr    = (un->GetVelocity()-othervelocity).MagnitudeSquared();
    float speedsquared = const_factor*const_factor*(magsqr > newmagsqr ? newmagsqr : magsqr);
    static unsigned int max_collide_trees = static_cast<unsigned int>(XMLSupport::parse_int( vs_config->getVariable( "physics", "max_collide_trees", "16384" ) ));
    if (un->rSize()*un->rSize() > simulation_atom_var*simulation_atom_var*speedsquared || max_collide_trees == 1)
        return rapidColliders[0];
    if (rapidColliders[0] == NULL)
        return NULL;
    if (un->rSize() <= 0.)      //Shouldn't happen bug I've seen this for asteroid fields...
        return NULL;
    //Force pow to 0 in order to avoid nan problems...
    unsigned int   pow = 0;
    if (pow >= collideTreesMaxTrees || pow >= max_collide_trees)
        pow = collideTreesMaxTrees-1;
    int val = 1<<pow;
    if (rapidColliders[pow] == NULL)
        rapidColliders[pow] = un->getCollideTree( Vector( 1, 1, val ) );
    return rapidColliders[pow];
}

collideTrees* collideTrees::Get( const std::string &hash_key )
{
    return unitColliders.Get( hash_key );
}

void collideTrees::Dec()
{
    refcount--;
    if (refcount == 0) {
        unitColliders.Delete( hash_key );
        for (unsigned int i = 0; i < collideTreesMaxTrees; ++i) {
            if (rapidColliders[i]) {
                delete rapidColliders[i];
                rapidColliders[i] = nullptr;
            }
        }
        if (colShield) {
            delete colShield;
            colShield = nullptr;
        }
        delete this;    // SGT 2021-07-09 ?!?
        return;
    }
}

bool TableLocationChanged( const QVector &Mini, const QVector &minz )
{
    return _Universe->activeStarSystem()->collide_table->c.hash_int( Mini.i )
           != _Universe->activeStarSystem()->collide_table->c.hash_int( minz.i )
           || _Universe->activeStarSystem()->collide_table->c.hash_int( Mini.j )
           != _Universe->activeStarSystem()->collide_table->c.hash_int( minz.j )
           || _Universe->activeStarSystem()->collide_table->c.hash_int( Mini.k )
           != _Universe->activeStarSystem()->collide_table->c.hash_int( minz.k );
}

bool TableLocationChanged( const LineCollide &lc, const QVector &minx, const QVector &maxx )
{
    return TableLocationChanged( lc.Mini, minx ) || TableLocationChanged( lc.Maxi, maxx );
}

void KillCollideTable( LineCollide *lc, StarSystem *ss )
{
    if (lc->type == LineCollide::UNIT) {
        ss->collide_table->c.Remove( lc, lc->object.u );
    } else {
        BOOST_LOG_TRIVIAL(warning) << boost::format("such collide types as %1$d not allowed") % lc->type;
    }
}

bool EradicateCollideTable( LineCollide *lc, StarSystem *ss )
{
    if (lc->type == LineCollide::UNIT) {
        return ss->collide_table->c.Eradicate( lc->object.u );
    } else {
        BOOST_LOG_TRIVIAL(warning) << boost::format("such collide types as %1$d not allowed") % lc->type;
        return false;
    }
}

void AddCollideQueue( LineCollide &tmp, StarSystem *ss )
{
    if (tmp.type == LineCollide::UNIT) {
        ss->collide_table->c.Put( &tmp, tmp.object.u );
    } else {
        BOOST_LOG_TRIVIAL(warning) << boost::format("such collide types as %1$d not allowed") % tmp.type;
    }
}

bool lcwithin( const LineCollide &lc, const LineCollide &tmp )
{
    return lc.Mini.i< tmp.Maxi.i
                      && lc.Mini.j< tmp.Maxi.j
                                    && lc.Mini.k< tmp.Maxi.k
                                                  && lc.Maxi.i >tmp.Mini.i
                                    && lc.Maxi.j >tmp.Mini.j
                      && lc.Maxi.k >tmp.Mini.k;
}

bool usehuge_table()
{
    const unsigned int  A    = 9301;
    const unsigned int  C    = 49297;
    const unsigned int  M    = 233280;
    static unsigned int seed = 3259235;
    seed = (seed*A+C)%M;
    return seed < (M/100);
}

bool Bolt::Collide( Collidable::CollideRef index )
{
    return _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT]->CheckCollisions( this, **location );
}

static bool beamCheckCollision( QVector pos, float len, const Collidable &un )
{
    return (un.GetPosition()-pos).MagnitudeSquared() <= len*len+2*len*un.radius+un.radius*un.radius;
}

void Beam::CollideHuge( const LineCollide &lc, Unit *targetToCollideWith, Unit *firer, Unit *superunit )
{
    QVector x0 = center;
    QVector v  = direction*curlength;
    if (is_null( superunit->location[Unit::UNIT_ONLY] ) && curlength) {
        if (targetToCollideWith)
            this->Collide( targetToCollideWith, firer, superunit );
    } else if (curlength) {
        CollideMap *cm = _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY];

        CollideMap::iterator superloc = superunit->location[Unit::UNIT_ONLY];
        CollideMap::iterator tmore    = superloc;
        if ( !cm->Iterable( superloc ) ) {
            CollideArray::CollidableBackref *br = static_cast< CollideArray::CollidableBackref* > (superloc);
            CollideMap::iterator tmploc = cm->begin()+br->toflattenhints_offset;
            if ( tmploc == cm->end() )
                tmploc--;
            tmore = superloc = tmploc;             //don't decrease tless
        } else {
            ++tmore;
        }
        double r0 = x0.i;
        double r1 = x0.i+v.i;
        double minlook   = r0 < r1 ? r0 : r1;
        double maxlook   = r0 < r1 ? r1 : r0;
        bool   targcheck = false;
        maxlook += ( maxlook-(*superunit->location[Unit::UNIT_ONLY])->getKey() )+2*curlength;           //double damage, yo
        minlook += ( minlook-(*superunit->location[Unit::UNIT_ONLY])->getKey() )-2*curlength*curlength;
        //(a+2*b)^2-(a+b)^2 = 3b^2+2ab = 2b^2+(a+b)^2-a^2
        if ( superloc != cm->begin()
            && minlook < (*superunit->location[Unit::UNIT_ONLY])->getKey() ) {
            //less traversal
            CollideMap::iterator tless = superloc;
            --tless;
            while ( (*tless)->getKey() >= minlook ) {
                CollideMap::iterator curcheck = tless;
                bool breakit = false;
                if ( tless != cm->begin() )
                    --tless;
                else
                    breakit = true;
                if ( (*curcheck)->radius > 0 ) {
                    if ( beamCheckCollision( center, curlength, (**curcheck) ) ) {
                        Unit *tmp = (**curcheck).ref.unit;
                        this->Collide( tmp, firer, superunit );
                        targcheck = (targcheck || tmp == targetToCollideWith);
                    }
                }
                if (breakit)
                    break;
            }
        }
        if ( maxlook > (*superunit->location[Unit::UNIT_ONLY])->getKey() ) {
            //greater traversal
            while (tmore != cm->end() && (*tmore)->getKey() <= maxlook) {
                if ( (*tmore)->radius > 0 ) {
                    Unit *un = (*tmore)->ref.unit;
                    if ( beamCheckCollision( center, curlength, **tmore++ ) ) {
                        this->Collide( un, firer, superunit );
                        targcheck = (targcheck || un == targetToCollideWith);
                    }
                } else {++tmore; }}
        }
        if (targetToCollideWith && !targcheck)
            this->Collide( targetToCollideWith, firer, superunit );
    }
}

