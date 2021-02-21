

#include <vector>
#include <string>
#include <algorithm>

#include "bolt.h"

#include "gfx/boltdrawmanager.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "unit.h"
#include "audiolib.h"
#include "config_xml.h"
#include "gfx/camera.h"
#include "options.h"
#include "universe.h"


using std::vector;
using std::string;



extern double interpolation_blend_factor;

inline void BlendTrans( Matrix &drawmat, const QVector &cur_position, const QVector &prev_position )
{
    drawmat.p = prev_position.Scale( 1-interpolation_blend_factor )+cur_position.Scale( interpolation_blend_factor );
}

// Bolts have texture
int Bolt::AddTexture( BoltDrawManager *q, std::string file )
{
    int decal = q->boltdecals.AddTexture( file.c_str(), MIPMAP );
    if ( decal >= (int) q->bolts.size() ) {
        q->bolts.push_back( vector< Bolt > () );
        int blargh = q->boltdecals.AddTexture( file.c_str(), MIPMAP );
        if ( blargh >= (int) q->bolts.size() )
            q->bolts.push_back( vector< Bolt > () );
    }
    return decal;
}

// Balls have animation
int Bolt::AddAnimation( BoltDrawManager *q, std::string file, QVector cur_position )
{
    int decal = -1;
    for (unsigned int i = 0; i < q->animationname.size(); i++)
        if (file == q->animationname[i])
            decal = i;
    if (decal == -1) {
        decal = q->animations.size();
        q->animationname.push_back( file );
        q->animations.push_back( new Animation( file.c_str(), true, .1, MIPMAP, false ) );         //balls have their own orientation
        q->animations.back()->SetPosition( cur_position );
        q->balls.push_back( vector< Bolt > () );
    }
    return decal;
}

void Bolt::Draw()
{
    BoltDrawManager& qq = BoltDrawManager::getInstance();
    GFXDisable( LIGHTING );
    GFXDisable( CULLFACE );


    GFXBlendMode( ONE, game_options.BlendGuns ? ONE : ZERO );
    GFXTextureCoordGenMode( 0, NO_GEN, NULL, NULL );

    GFXAlphaTest( GREATER, .1 );
    vector< vector< Bolt > >::iterator i;
    vector< Bolt >::iterator j;
    vector< Animation* >::iterator     k = qq.animations.begin();
    float etime = GetElapsedTime();
    float pixel_angle = 2
                        *sin( g_game.fov*M_PI/180.0
                             /(g_game.y_resolution
                               > g_game.x_resolution ? g_game.y_resolution : g_game.x_resolution) )*game_options.bolt_pixel_size;
    pixel_angle *= pixel_angle;
    Vector  p, q, r;
    _Universe->AccessCamera()->GetOrientation( p, q, r );
    QVector campos = _Universe->AccessCamera()->GetPosition();
    for (i = qq.balls.begin(); i != qq.balls.end(); i++, k++) {
        Animation *cur = *k;
        if ( i->begin() != i->end() ) {
            float bolt_size = 2*i->begin()->type->Radius*2;
            bolt_size *= bolt_size;
            //Matrix result;
            //FIXME::MuST USE DRAWNO	TRANSFORMNOW cur->CalculateOrientation (result);
            for (j = i->begin(); j != i->end(); j++) {
                //don't update time more than once
                Bolt *bolt     = &*j;
                float distance = (bolt->cur_position-campos).MagnitudeSquared();
                if (distance*pixel_angle < bolt_size) {
                    const weapon_info *type = bolt->type;
                    BlendTrans( bolt->drawmat, bolt->cur_position, bolt->prev_position );
                    Matrix tmp;
                    VectorAndPositionToMatrix( tmp, p, q, r, bolt->drawmat.p );
                    cur->SetDimensions( bolt->type->Radius, bolt->type->Radius );
                    GFXLoadMatrixModel( tmp );
                    GFXColor4f( type->r, type->g, type->b, type->a );
                    cur->DrawNoTransform( false, true );
                }
            }
        }
    }
    GFXVertexList *qmesh = qq.boltmesh;
    if (qmesh && qq.bolts.begin() != qq.bolts.end()) {
        GFXAlphaTest( ALWAYS, 0 );
        GFXDisable( DEPTHWRITE );
        GFXDisable( TEXTURE1 );
        GFXEnable( TEXTURE0 );
        GFXTextureCoordGenMode( 0, NO_GEN, NULL, NULL );
        
        BLENDFUNC bsrc, bdst;
        if (game_options.BlendGuns == true)
            GFXBlendMode( bsrc=ONE, bdst=ONE );
        else
            GFXBlendMode( bsrc=ONE, bdst=ZERO );
        
        qmesh->LoadDrawState();
        qmesh->BeginDrawState();
        int decal = 0;
        for (i = qq.bolts.begin(); i != qq.bolts.end(); decal++, i++) {
            Texture *dec = qq.boltdecals.GetTexture( decal );
            if ( dec && i->begin() != i->end() ) {
                float bolt_size = 2*i->begin()->type->Radius+i->begin()->type->Length;
                bolt_size *= bolt_size;
                for (size_t pass = 0, npasses = dec->numPasses(); pass < npasses; ++pass) {
                    GFXTextureEnv( 0, GFXMODULATETEXTURE );
                    if (dec->SetupPass(0, bsrc, bdst)) {
                        dec->MakeActive();
                        GFXToggleTexture( true, 0 );
                        for (j = i->begin(); j != i->end(); j++) {
                            Bolt &bolt     = *j;
                            float distance = (bolt.cur_position-campos).MagnitudeSquared();
                            if (distance*pixel_angle < bolt_size) {
                                const weapon_info *wt = bolt.type;

                                BlendTrans( bolt.drawmat, bolt.cur_position, bolt.prev_position );
                                Matrix drawmat( bolt.drawmat );
                                if (game_options.StretchBolts > 0)
                                    ScaleMatrix( drawmat, Vector( 1, 1, bolt.type->Speed*etime*game_options.StretchBolts/bolt.type->Length ) );
                                GFXLoadMatrixModel( drawmat );
                                GFXColor4f( wt->r, wt->g, wt->b, wt->a );
                                qmesh->Draw();
                            }
                        }
                    }
                }
            }
        }
        qmesh->EndDrawState();
    }
    GFXEnable( LIGHTING );
    GFXEnable( CULLFACE );
    GFXBlendMode( ONE, ZERO );
    GFXEnable( DEPTHTEST );
    GFXEnable( DEPTHWRITE );
    GFXEnable( TEXTURE0 );
    GFXColor4f( 1, 1, 1, 1 );
}

extern void BoltDestroyGeneric( Bolt *whichbolt, unsigned int index, int decal, bool isBall );
void Bolt::Destroy( unsigned int index )
{
    VSDESTRUCT2
    bool isBall  = true;
    if (type->type == weapon_info::BOLT) {
        isBall = false;
    } else {}
    BoltDestroyGeneric( this, index, decal, isBall );
}

// A bolt is created when fired
Bolt::Bolt( const weapon_info *typ,
            const Matrix &orientationpos,
            const Vector &shipspeed,
            void *owner,
            CollideMap::iterator hint ) : cur_position( orientationpos.p )
    , ShipSpeed( shipspeed )
{
    VSCONSTRUCT2( 't' )
    BoltDrawManager& q = BoltDrawManager::getInstance();
    prev_position = cur_position;
    this->owner   = owner;
    this->type    = typ;
    curdist = 0;
    CopyMatrix( drawmat, orientationpos );
    Vector vel = shipspeed+orientationpos.getR()*typ->Speed;
    if (typ->type == weapon_info::BOLT) {
        ScaleMatrix( drawmat, Vector( typ->Radius, typ->Radius, typ->Length ) );
        decal = Bolt::AddTexture( &q, typ->file );
        this->location =
            _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT]->insert( Collidable( Bolt::BoltIndex( q.bolts[decal].
                                                                                                             size(),
                                                                                                             decal,
                                                                                                             false ).bolt_index,
                                                                                            (shipspeed+orientationpos.getR()
                                                                                             *typ->Speed).Magnitude()*.5,
                                                                                            cur_position+vel*simulation_atom_var*.5 ),
                                                                                hint );
        q.bolts[decal].push_back( *this );
    } else {
        ScaleMatrix( drawmat, Vector( typ->Radius, typ->Radius, typ->Radius ) );
        decal = Bolt::AddAnimation( &q, typ->file, cur_position );

        this->location =
            _Universe->activeStarSystem()->collide_map[Unit::UNIT_BOLT]->insert( Collidable( Bolt::BoltIndex( q.balls[decal].
                                                                                                             size(),
                                                                                                             decal,
                                                                                                             true ).bolt_index,
                                                                                            (shipspeed+orientationpos.getR()
                                                                                             *typ->Speed).Magnitude()*.5,
                                                                                            cur_position+vel*simulation_atom_var*.5 ),
                                                                                hint );
        q.balls[decal].push_back( *this );
    }
}

size_t nondecal_index( Collidable::CollideRef b )
{
    return b.bolt_index>>8;
}

bool Bolt::Update( Collidable::CollideRef index )
{
    const weapon_info *type = this->type;
    float speed = type->Speed;
    curdist += speed*simulation_atom_var;
    prev_position = cur_position;
    cur_position +=
        ( ( ShipSpeed+drawmat.getR()*speed
           /( (type->type
               == weapon_info::BALL)*type->Radius+(type->type != weapon_info::BALL)*type->Length ) ).Cast()*simulation_atom_var );
    if (curdist > type->Range) {
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
            Bolt *thus = Bolt::BoltFromIndex( collidable.ref );
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

void Bolt::UpdatePhysics(StarSystem *ss)
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
        distance = curdist/this->type->Range;
        GFXColor    coltmp( this->type->r, this->type->g, this->type->b, this->type->a );
        target->ApplyDamage( (prev_position+tmp).Cast(),
                            normal,
                            this->type->Damage*( (1-distance)+distance*this->type->Longrange ),
                            affectedSubUnit,
                            coltmp,
                            owner,
                            this->type->PhaseDamage*( (1-distance)+distance*this->type->Longrange ) );
        return true;
    }
    return false;
}

Bolt* Bolt::BoltFromIndex( Collidable::CollideRef b )
{
    BoltDrawManager& bolt_draw_manager = BoltDrawManager::getInstance();
    size_t ind = nondecal_index( b );
    if (b.bolt_index&128)
        return &bolt_draw_manager.balls[b.bolt_index&0x7f][ind];
    else
        return &bolt_draw_manager.bolts[b.bolt_index&0x7f][ind];
}

bool Bolt::CollideAnon( Collidable::CollideRef b, Unit *un )
{
    Bolt *tmp = BoltFromIndex( b );
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
    BoltDrawManager& q = BoltDrawManager::getInstance();
    vector< vector< Bolt > > *target;
    if (!isBall)
        target = &q.bolts;
    else
        target = &q.balls;
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

