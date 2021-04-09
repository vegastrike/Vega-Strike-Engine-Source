#include <algorithm>
#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include <vector>

#include <string>
#include <algorithm>
#include "unit_generic.h"
#include "audiolib.h"
#include "config_xml.h"
#include "star_system.h"
#include "universe.h"

using std::vector;
using std::string;

Bolt::Bolt( const weapon_info *typ,
            const Matrix &orientationpos,
            const Vector &shipspeed,
            void *owner,
            CollideMap::iterator hint ) : cur_position( orientationpos.p )
    , ShipSpeed( shipspeed )
{
    VSCONSTRUCT2( 't' )
    bolt_draw*q   = _Universe->activeStarSystem()->bolts;
    prev_position = cur_position;
    this->owner   = owner;
    this->type    = typ;
    curdist = 0;
    CopyMatrix( drawmat, orientationpos );
    Vector vel = shipspeed+orientationpos.getR()*typ->speed;
    if (typ->type == WEAPON_TYPE::BOLT) {
        ScaleMatrix( drawmat, Vector( typ->radius, typ->radius, typ->length ) );
        decal = Bolt::AddTexture( q, typ->file );
        this->location =
            _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT]->insert( Collidable( Bolt::BoltIndex( q->bolts[decal].
                                                                                                             size(),
                                                                                                             decal,
                                                                                                             false ).bolt_index,
                                                                                            (shipspeed+orientationpos.getR()
                                                                                             *typ->speed).Magnitude()*.5,
                                                                                            cur_position+vel*simulation_atom_var*.5 ),
                                                                                hint );
        q->bolts[decal].push_back( *this );
    } else {
        ScaleMatrix( drawmat, Vector( typ->radius, typ->radius, typ->radius ) );
        decal = Bolt::AddAnimation( q, typ->file, cur_position );

        this->location =
            _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT]->insert( Collidable( Bolt::BoltIndex( q->balls[decal].
                                                                                                             size(),
                                                                                                             decal,
                                                                                                             true ).bolt_index,
                                                                                            (shipspeed+orientationpos.getR()
                                                                                             *typ->speed).Magnitude()*.5,
                                                                                            cur_position+vel*simulation_atom_var*.5 ),
                                                                                hint );
        q->balls[decal].push_back( *this );
    }
}

size_t nondecal_index( Collidable::CollideRef b )
{
    return b.bolt_index>>8;
}

bool Bolt::Update( Collidable::CollideRef index )
{
    const weapon_info *type = this->type;
    float speed = type->speed;
    curdist += speed*simulation_atom_var;
    prev_position = cur_position;
    cur_position +=
        ( ( ShipSpeed+drawmat.getR()*speed
           /( (type->type
               == WEAPON_TYPE::BALL)*type->radius+(type->type != WEAPON_TYPE::BALL)*type->length ) ).Cast()*simulation_atom_var );
    if (curdist > type->range) {
        this->Destroy( nondecal_index( index ) );         //risky
        return false;
    }
    Collidable updated( **location );
    updated.SetPosition( .5*(prev_position+cur_position) );
    location = _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT]->changeKey( location, updated );
    return true;
}

class UpdateBolt
{
    CollideMap *collide_map;
    StarSystem *starSystem;
public: UpdateBolt( StarSystem *ss, CollideMap *collide_map )
    {
        this->starSystem = ss;
        this->collide_map = collide_map;
    }
    void operator()( Collidable &collidable )
    {
        if (collidable.radius < 0) {
            Bolt *thus = Bolt::BoltFromIndex( starSystem, collidable.ref );
            if ( !collide_map->CheckCollisions( thus, collidable ) )
                thus->Update( collidable.ref );
        }
    }
};

namespace vsalg
{
//

template < typename IT, typename F >
void for_each( IT start, IT end, F f )
{
    //This way, deletion of current item is allowed
    //- drawback: iterator copy each iteration
    while (start != end)
        f( *start++ );
}

//
}

class UpdateBolts
{
    UpdateBolt sub;
public: UpdateBolts( StarSystem *ss, CollideMap *collide_map ) : sub( ss, collide_map ) {}
    template < class T >
    void operator()( T &collidableList )
    {
        vsalg::for_each( collidableList.begin(), collidableList.end(), sub );
    }
};

void Bolt::UpdatePhysics( StarSystem *ss )
{
    CollideMap *cm = ss->collide_map[Unit::UNIT_BOLT];
    vsalg::for_each( cm->sorted.begin(), cm->sorted.end(), UpdateBolt( ss, cm ) );
    vsalg::for_each( cm->toflattenhints.begin(), cm->toflattenhints.end(), UpdateBolts( ss, cm ) );
}

bool Bolt::Collide( Unit *target )
{
    Vector normal;
    float  distance;
    Unit  *affectedSubUnit;
    if ( ( affectedSubUnit = target->rayCollide( prev_position, cur_position, normal, distance ) ) ) {
        //ignore return
        if (target == owner) return false;
        enum _UnitType type = target->isUnit();
        if (type == _UnitType::nebula || type == _UnitType::asteroid) {
            static bool collideroids =
                XMLSupport::parse_bool( vs_config->getVariable( "physics", "AsteroidWeaponCollision", "false" ) );
            if ( type != _UnitType::asteroid || (!collideroids) )
                return false;
        }
        static bool collidejump = XMLSupport::parse_bool( vs_config->getVariable( "physics", "JumpWeaponCollision", "false" ) );
        if ( type == _UnitType::planet && (!collidejump) && !target->GetDestinations().empty() )
            return false;
        QVector     tmp = (cur_position-prev_position).Normalize();
        tmp = tmp.Scale( distance );
        distance = curdist/this->type->range;
        GFXColor    coltmp( this->type->r, this->type->g, this->type->b, this->type->a );
        target->ApplyDamage( (prev_position+tmp).Cast(),
                            normal,
                            this->type->damage*( (1-distance)+distance*this->type->long_range ),
                            affectedSubUnit,
                            coltmp,
                            owner,
                            this->type->phase_damage*( (1-distance)+distance*this->type->long_range ) );
        return true;
    }
    return false;
}

Bolt* Bolt::BoltFromIndex( StarSystem *ss, Collidable::CollideRef b )
{
    size_t ind = nondecal_index( b );
    if (b.bolt_index&128)
        return &ss->bolts->balls[b.bolt_index&0x7f][ind];
    else
        return &ss->bolts->bolts[b.bolt_index&0x7f][ind];
}

bool Bolt::CollideAnon( Collidable::CollideRef b, Unit *un )
{
    Bolt *tmp = BoltFromIndex( _Universe->activeStarSystem(), b );
    if ( tmp->Collide( un ) ) {
        tmp->Destroy( nondecal_index( b ) );
        return true;
    }
    return false;
}

Collidable::CollideRef Bolt::BoltIndex( int index, int decal, bool isBall )
{
    Collidable::CollideRef temp;
    temp.bolt_index   = index;
    temp.bolt_index <<= 8;
    temp.bolt_index  |= decal;
    temp.bolt_index  |= isBall ? 128 : 0;
    return temp;
}

void BoltDestroyGeneric( Bolt *whichbolt, unsigned int index, int decal, bool isBall )
{
    VSDESTRUCT2
    bolt_draw *q = _Universe->activeStarSystem()->bolts;
    vector< vector< Bolt > > *target;
    if (!isBall)
        target = &q->bolts;
    else
        target = &q->balls;
    vector< Bolt > *vec = &(*target)[decal];
    if (&(*vec)[index] == whichbolt) {
        unsigned int tsize = vec->size();
        CollideMap  *cm    = _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT];
        cm->UpdateBoltInfo( vec->back().location, (*(*vec)[index].location)->ref );

        assert( index < tsize );
        cm->erase( (*vec)[index].location );
        if ( index+1 != vec->size() )
            (*vec)[index] = vec->back();                //just a memcopy, yo
        vec->pop_back();         //pop that back up
    } else {
        BOOST_LOG_TRIVIAL(fatal) << "Bolt Fault Nouveau! Not found in draw queue! No Chance to recover";
        VSFileSystem::flushLogs();
        assert( 0 );
    }
}

