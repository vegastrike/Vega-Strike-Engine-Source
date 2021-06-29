/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "unit.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "file_main.h"
#include "gfx/halo.h"
#include "gfx/halo_system.h"
#include "gfx/quaternion.h"
#include "gfx/matrix.h"
#include "gfx/technique.h"
#include "gfx/occlusion.h"



#include "gfx/sprite.h"
#include "lin_time.h"

#include "gfx/vsbox.h"
#include "bolt.h"
#include "gfx/lerp.h"
#include "audiolib.h"
#include "gfx/cockpit.h"
#include "config_xml.h"
#include "images.h"
#include "main_loop.h"
#include "script/mission.h"
#include "script/flightgroup.h"
#include "savegame.h"
#include "xml_serializer.h"
#include "python/python_class.h"
#include "cmd/ai/missionscript.h"
#include "gfx/particle.h"
#include "cmd/ai/aggressive.h"
#include "cmd/base.h"
#include "gfx/point_to_cam.h"

#include "base_util.h"
#include "unit_jump.h"
#include "unit_customize.h"
#include "unit_damage.h"
#include "unit_physics.h"
#include "unit_click.h"
#include "options.h"

#include "weapon_info.h"
#include "mount_size.h"

using std::vector;
using std::string;
using std::map;

//if the PQR of the unit may be variable...for radius size computation
//#define VARIABLE_LENGTH_PQR

//#define DESTRUCTDEBUG
#include "beam.h"
#include "python/init.h"
#include "unit_const_cache.h"
extern double interpolation_blend_factor;
extern double saved_interpolation_blend_factor;
extern bool   cam_setup_phase;

/**** MOVED FROM BASE_INTERFACE.CPP ****/
extern string getCargoUnitName( const char *name );

GameUnit::GameUnit( int ) : sparkle_accum( 0 )
    , phalos( new HaloSystem() )
{
    this->Unit::Init();
}

GameUnit::GameUnit( std::vector< Mesh* > &meshes, bool SubU, int fact ) :
    Unit( meshes, SubU, fact )
    , sparkle_accum( 0 )
    , phalos( new HaloSystem() )
{}

GameUnit::GameUnit( const char *filename,
                                                           bool SubU,
                                                           int faction,
                                                           std::string unitModifications,
                                                           Flightgroup *flightgrp,
                                                           int fg_subnumber,
                                                           string *netxml ) : sparkle_accum( 0 )
    , phalos( new HaloSystem() )
{
    Unit::Init( filename, SubU, faction, unitModifications, flightgrp, fg_subnumber, netxml );
}

GameUnit::~GameUnit()
{
    for (unsigned int meshcount = 0; meshcount < this->meshdata.size(); meshcount++)
        if (this->meshdata[meshcount]) {
            delete this->meshdata[meshcount];
            this->meshdata[meshcount] = nullptr;
        }
    this->meshdata.clear();
    //delete phalos;
}

/*
unsigned int GameUnit::nummesh() const {
    // return number of meshes but not the shield
    return (this->meshdata.size() - 1 );
}*/


void GameUnit::UpgradeInterface( Unit *baseun )
{
    string basename = ( ::getCargoUnitName( baseun->getFullname().c_str() ) );
    if (baseun->isUnit() != _UnitType::planet)
        basename = baseun->name;
    BaseUtil::LoadBaseInterfaceAtDock( basename, baseun, this );
}

#define PARANOIA .4

inline static float perspectiveFactor( float d )
{
    if (d > 0)
        return g_game.x_resolution*GFXGetZPerspective( d );
    else
        return 1.0f;
}


VSSprite*GameUnit::getHudImage() const
{
    return this->pImage->pHudImage;
}


void GameUnit::addHalo( const char *filename,
                                    const Matrix &trans,
                                    const Vector &size,
                                    const GFXColor &col,
                                    std::string halo_type,
                                    float halo_speed )
{
    phalos->AddHalo( filename, trans, size, col, halo_type, halo_speed );
}


void GameUnit::Cloak( bool engage )
{
    Unit::Cloak( engage );         //client side unit
}


bool GameUnit::queryFrustum( double frustum[6][4] ) const
{
    unsigned int    i;
#ifdef VARIABLE_LENGTH_PQR
    Vector TargetPoint( cumulative_transformation_matrix[0],
                        cumulative_transformation_matrix[1],
                        cumulative_transformation_matrix[2] );
    float  SizeScaleFactor = sqrtf( TargetPoint.Dot( TargetPoint ) );
#else
    Vector TargetPoint;
#endif
    for (i = 0; i < nummesh() && this->meshdata[i]; i++) {
        TargetPoint = Transform( this->cumulative_transformation_matrix, this->meshdata[i]->Position() );
        if ( GFXSphereInFrustum( frustum,
                                 TargetPoint,
                                 this->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
                                 *SizeScaleFactor
#endif
                               ) )
            return true;
    }
    const Unit *un;
    for (un_fkiter iter = this->SubUnits.constFastIterator(); (un = *iter); ++iter)
        if ( ( (GameUnit*)un )->queryFrustum( frustum ) )
            return true;
    return false;
}


void GameUnit::UpdateHudMatrix( int whichcam )
{
    Matrix m;
    Matrix ctm = this->cumulative_transformation_matrix;
    Vector q( ctm.getQ() );
    Vector r( ctm.getR() );
    Vector tmp;
    CrossProduct( r, q, tmp );
    _Universe->AccessCamera( whichcam )->SetOrientation( tmp, q, r );

    _Universe->AccessCamera( whichcam )->SetPosition( Transform( ctm,
                                                                this->pImage->CockpitCenter.Cast() ),
                                                     this->GetWarpVelocity(), this->GetAngularVelocity(), this->GetAcceleration() );
}

extern int cloakVal( int cloakint, int cloakminint, int cloakrateint, bool cloakglass ); //short fix?


void GameUnit::DrawNow( const Matrix &mato, float lod )
{
    static const void *rootunit = NULL;
    if (rootunit == NULL) rootunit = (const void*) this;
    float damagelevel = 1.0;
    unsigned char chardamage    = 0;
    if (this->hull < this->maxhull) {
        damagelevel = this->hull/this->maxhull;
        chardamage  = ( 255-(unsigned char) (damagelevel*255) );
    }
#ifdef VARIABLE_LENGTH_PQR
    const float  vlpqrScaleFactor = SizeScaleFactor;
#else
    const float  vlpqrScaleFactor = 1.f;
#endif
    unsigned int i;
    Matrix mat( mato );
    if (this->graphicOptions.FaceCamera) {
        Vector  p, q, r;
        QVector pos( mato.p );
        float   wid, hei;
        CalculateOrientation( pos, p, q, r, wid, hei, 0, false, &mat );
        pos = mato.p;
        VectorAndPositionToMatrix( mat, p, q, r, pos );
    }
    int cloak = this->cloaking;
    if (this->cloaking > this->cloakmin)
        cloak = cloakVal( cloak, this->cloakmin, this->cloakrate, this->cloakglass );
    for (i = 0; i <= this->nummesh(); i++) {
        //NOTE LESS THAN OR EQUALS...to cover shield mesh
        if (this->meshdata[i] == NULL)
            continue;
        QVector TransformedPosition = Transform( mat,
                                                this->meshdata[i]->Position().Cast() );
        float   d = GFXSphereInFrustum( TransformedPosition, this->meshdata[i]->clipRadialSize()*vlpqrScaleFactor );
        if (d)          //d can be used for level of detail
                        //this->meshdata[i]->DrawNow(lod,false,mat,cloak);//cloakign and nebula
            this->meshdata[i]->Draw( lod, mat, d, cloak );
    }
    Unit *un;
    /*for (un_iter iter = this->getSubUnits(); (un = *iter); ++iter) {
        Matrix temp;
        un->curr_physical_state.to_matrix( temp );
        Matrix submat;
        MultMatrix( submat, mat, temp );
        (un)->DrawNow( submat, lod );*/
    if (this->hasSubUnits()) {
        for (un_iter iter = this->getSubUnits(); (un = *iter); ++iter) {
            Matrix temp;
            un->curr_physical_state.to_matrix( temp );
            Matrix submat;
            MultMatrix( submat, mat, temp );
            (un)->DrawNow( submat, lod );
        }
    }
    float  cmas = this->computer.max_ab_speed()*this->computer.max_ab_speed();
    if (cmas == 0)
        cmas = 1;
        Vector Scale( 1, 1, 1 );         //Now, HaloSystem handles that
    int    nummounts = this->getNumMounts();
    Matrix wmat = this->WarpMatrix( mat );
    for (i = 0; (int) i < nummounts; i++) {
        Mount *mahnt = &this->mounts[i];
        if (game_options.draw_weapons)
            if (mahnt->xyscale != 0 && mahnt->zscale != 0) {
                Mesh *gun = mahnt->type->gun;
                if (gun && mahnt->status != Mount::UNCHOSEN) {
                    Transformation mountLocation( mahnt->GetMountOrientation(), mahnt->GetMountLocation().Cast() );
                    mountLocation.Compose( Transformation::from_matrix( mat ), wmat );
                    Matrix mmat;
                    mountLocation.to_matrix( mmat );
                    if (GFXSphereInFrustum( mountLocation.position, gun->rSize()*vlpqrScaleFactor ) > 0) {
                        float d   = ( mountLocation.position-_Universe->AccessCamera()->GetPosition() ).Magnitude();
                        float lod = gun->rSize()*g_game.detaillevel*perspectiveFactor(
                            (d-gun->rSize() < g_game.znear) ? g_game.znear : d-gun->rSize() );
                        ScaleMatrix( mmat, Vector( mahnt->xyscale, mahnt->xyscale, mahnt->zscale ) );
                        gun->setCurrentFrame( this->mounts[i].ComputeAnimatedFrame( gun ) );
                        gun->Draw( lod, mmat, d, cloak,
                                   (_Universe->AccessCamera()->GetNebula() == this->nebula && this->nebula != NULL) ? -1 : 0,
                                   chardamage,
                                   true );                                                                                                                                       //cloakign and nebula
                        if (mahnt->type->gun1) {
                            gun = mahnt->type->gun1;
                            gun->setCurrentFrame( this->mounts[i].ComputeAnimatedFrame( gun ) );
                            gun->Draw( lod, mmat, d, cloak,
                                       (_Universe->AccessCamera()->GetNebula() == this->nebula && this->nebula
                                        != NULL) ? -1 : 0,
                                       chardamage, true );                                                                                                                               //cloakign and nebula
                        }
                    }
                }
            }
    }
    Vector linaccel = this->GetAcceleration();
    Vector angaccel = this->GetAngularAcceleration();
    float  maxaccel = this->GetMaxAccelerationInDirectionOf( mat.getR(), true );
    Vector velocity = this->GetVelocity();
    if ( !( this->docked & (DOCKED | DOCKED_INSIDE) ) )
        phalos->Draw( mat, Scale, cloak, 0, this->GetHullPercent(), velocity, linaccel, angaccel, maxaccel, cmas, this->faction );
    if (rootunit == (const void*) this) {
        Mesh::ProcessZFarMeshes();
        Mesh::ProcessUndrawnMeshes();
        rootunit = NULL;
    }
}


void GameUnit::DrawNow()
{
    DrawNow( identity_matrix, 1000000000 );
}

extern double calc_blend_factor( double frac, unsigned int priority, unsigned int when_it_will_be_simulated, unsigned int cur_simulation_frame );


void GameUnit::Draw( const Transformation &parent, const Matrix &parentMatrix )
{
    //Quick shortcut for camera setup phase
    bool myparent = ( this == _Universe->AccessCockpit()->GetParent() );


    Matrix         *ctm;
    Matrix invview;
    Transformation *ct;

    this->cumulative_transformation = linear_interpolate( this->prev_physical_state,
                                                          this->curr_physical_state,
                                                          interpolation_blend_factor );
    this->cumulative_transformation.Compose( parent, parentMatrix );
    this->cumulative_transformation.to_matrix( this->cumulative_transformation_matrix );

    ctm = &this->cumulative_transformation_matrix;
    ct  = &this->cumulative_transformation;
    if (this->graphicOptions.FaceCamera == 1) {
        Vector  p, q, r;
        QVector pos( ctm->p );
        float   wid, hei;
        float   magr = parentMatrix.getR().Magnitude();
        float   magp = parentMatrix.getP().Magnitude();
        float   magq = parentMatrix.getQ().Magnitude();
        CalculateOrientation( pos, p, q, r, wid, hei, 0, false, ctm );
        VectorAndPositionToMatrix( invview, p*magp, q*magq, r*magr, ctm->p );
        ctm = &invview;
    }

#ifdef PERFRAMESOUND
    AUDAdjustSound( sound.engine, cumulative_transformation.position, GetVelocity() );
#endif

    unsigned int i, n;
    if ( (this->hull < 0) && (!cam_setup_phase) )
        Explode( true, GetElapsedTime() );

    float damagelevel = 1.0f;
    unsigned char chardamage = 0;

    // We might need to scale rSize, this "average scale" takes the transform matrix into account
    float avgscale = 1.0f;

    bool On_Screen = false;
    bool Unit_On_Screen = false;
    float Apparent_Size = 0.0f;
    int cloak = this->cloaking;
    Matrix wmat;

    if (!cam_setup_phase) {
        // Following stuff is only needed in actual drawing phase
        if (this->cloaking > this->cloakmin) {
            cloak = (int) (this->cloaking-interpolation_blend_factor*this->cloakrate * simulation_atom_var );
            cloak = cloakVal( cloak, this->cloakmin, this->cloakrate, this->cloakglass );
        }
        if (this->hull < this->maxhull) {
            damagelevel = this->hull/this->maxhull;
            chardamage  = ( 255-(unsigned char) (damagelevel*255) );
        }
        avgscale = sqrt((ctm->getP().MagnitudeSquared() + ctm->getR().MagnitudeSquared()) * 0.5);
        wmat = this->WarpMatrix( *ctm );
    }

    if ( ( !(this->invisible&INVISUNIT) ) && ( ( !(this->invisible&INVISCAMERA) ) || (!myparent) ) ) {
        if (!cam_setup_phase) {
            Camera *camera = _Universe->AccessCamera();
            QVector camerapos = camera->GetPosition();

            float minmeshradius =
                ( camera->GetVelocity().Magnitude()+this->Velocity.Magnitude() ) * simulation_atom_var;

            unsigned int numKeyFrames = this->graphicOptions.NumAnimationPoints;
            for (i = 0, n = this->nummesh(); i <= n; i++) {
                //NOTE LESS THAN OR EQUALS...to cover shield mesh
                if (this->meshdata[i] == NULL)
                    continue;
                if (  i == n && (this->meshdata[i]->numFX() == 0 || this->hull < 0) )
                    continue;
                if (this->meshdata[i]->getBlendDst() == ONE) {
                    if ( (this->invisible & INVISGLOW) != 0 )
                        continue;
                    if (damagelevel < .9)
                        if ( flickerDamage() )
                            continue;
                }
                QVector TransformedPosition = Transform( *ctm, this->meshdata[i]->Position().Cast() );

                //d can be used for level of detail shit
                float mSize = this->meshdata[i]->rSize() * avgscale;
                double d = ( TransformedPosition-camerapos ).Magnitude();
                double rd  = d-mSize;
                float pixradius = Apparent_Size = mSize*perspectiveFactor(
                    (rd < g_game.znear) ? g_game.znear : rd );
                float lod = pixradius*g_game.detaillevel;
                if (this->meshdata[i]->getBlendDst() == ZERO) {
                    if (isUnit() == _UnitType::planet && pixradius > 10) {
                        Occlusion::addOccluder(TransformedPosition, mSize, true);
                    } else if (pixradius >= 10.0) {
                        Occlusion::addOccluder(TransformedPosition, mSize, false);
                    }
                }
                if (lod >= 0.5 && pixradius >= 2.5) {
                    double frustd = GFXSphereInFrustum(
                        TransformedPosition,
                        minmeshradius+mSize );
                    if (frustd) {
                        //if the radius is at least half a pixel at detail 1 (equivalent to pixradius >= 0.5 / detail)
                        float currentFrame = this->meshdata[i]->getCurrentFrame();
                        this->meshdata[i]->Draw( lod, wmat, d,
                                                 i == this->meshdata.size()-1 ? -1 : cloak,
                                                 (camera->GetNebula() == this->nebula && this->nebula != NULL) ? -1 : 0,
                                                 chardamage );                                                                                                                                                            //cloakign and nebula
                        On_Screen = true;
                        unsigned int numAnimFrames = 0;
                        static const string default_animation;
                        if ( this->meshdata[i]->getFramesPerSecond()
                            && ( numAnimFrames = this->meshdata[i]->getNumAnimationFrames( default_animation ) ) ) {
                            float currentprogress = floor(
                                this->meshdata[i]->getCurrentFrame()*numKeyFrames/(float) numAnimFrames );
                            if (numKeyFrames
                                && floor( currentFrame*numKeyFrames/(float) numAnimFrames ) != currentprogress)
                            {
                                this->graphicOptions.Animating = 0;
                                this->meshdata[i]->setCurrentFrame( .1+currentprogress*numAnimFrames/(float) numKeyFrames );
                            } else if (!this->graphicOptions.Animating) {
                                this->meshdata[i]->setCurrentFrame( currentFrame );                                 //dont' budge
                            }
                        }
                    }
                }
            }

            Unit_On_Screen = On_Screen || !!GFXSphereInFrustum(
                ct->position,
                minmeshradius+this->rSize() );
        } else Unit_On_Screen = true;
        if (Unit_On_Screen && this->hasSubUnits()) {
            Unit  *un;
            double backup = interpolation_blend_factor;
            int    cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
            for (un_iter iter = this->getSubUnits(); (un = *iter); ++iter) {
                float sim_atom_backup = simulation_atom_var;
                /*if (sim_atom_backup != SIMULATION_ATOM) {
                    BOOST_LOG_TRIVIAL(debug) << boost::format("void GameUnit::Draw( const Transformation &parent, const Matrix &parentMatrix ): sim_atom as backed up != SIMULATION_ATOM: %1%") % sim_atom_backup;
                }*/
                if (this->sim_atom_multiplier && un->sim_atom_multiplier) {
                    //BOOST_LOG_TRIVIAL(trace) << boost::format("void GameUnit::Draw( const Transformation &parent, const Matrix &parentMatrix ): simulation_atom_var as backed up  = %1%") % simulation_atom_var;
                    simulation_atom_var = simulation_atom_var * un->sim_atom_multiplier / this->sim_atom_multiplier;
                    //BOOST_LOG_TRIVIAL(trace) << boost::format("void GameUnit::Draw( const Transformation &parent, const Matrix &parentMatrix ): simulation_atom_var as multiplied = %1%") % simulation_atom_var;
                }
                interpolation_blend_factor = calc_blend_factor( saved_interpolation_blend_factor,
                                                                un->sim_atom_multiplier,
                                                                un->cur_sim_queue_slot,
                                                                cur_sim_frame );
                (un)->Draw( *ct, *ctm );

                simulation_atom_var = sim_atom_backup;
            }
            interpolation_blend_factor = backup;
        }
    } else {
        _Universe->AccessCockpit()->SetupViewPort();         ///this is the final, smoothly calculated cam
        //UpdateHudMatrix();
    }
    /***DEBUGGING cosAngleFromMountTo
     *  UnitCollection *dL = _Universe->activeStarSystem()->getUnitList();
     *  UnitCollection::UnitIterator *tmpiter = dL->createIterator();
     *  GameUnit<UnitType> * curun;
     *  while (curun = tmpiter->current()) {
     *  if (curun->selected) {
     *   float tmpdis;
     *   float tmpf = cosAngleFromMountTo (curun, tmpdis);
     *   VSFileSystem::vs_fprintf (stderr,"%s: <%f d: %f\n", curun->name.c_str(), tmpf, tmpdis);
     *
     *  }
     *  tmpiter->advance();
     *  }
     *  delete tmpiter;
     **/
    if (cam_setup_phase) return;
    int nummounts = this->getNumMounts();
    for (i = 0; (int) i < nummounts; i++) {
        Mount *mahnt = &this->mounts[i];
        if (game_options.draw_weapons && On_Screen)
            if (mahnt->xyscale != 0 && mahnt->zscale != 0) {
                Mesh *gun = mahnt->type->gun;
                if (gun && mahnt->status != Mount::UNCHOSEN) {
                    Transformation mountLocation( mahnt->GetMountOrientation(), mahnt->GetMountLocation().Cast() );
                    mountLocation.Compose( *ct, wmat );
                    Matrix mat;
                    mountLocation.to_matrix( mat );
                    if (GFXSphereInFrustum( mountLocation.position, gun->rSize()*avgscale ) > 0) {
                        float d   = ( mountLocation.position-_Universe->AccessCamera()->GetPosition() ).Magnitude();
                        float pixradius = gun->rSize()*perspectiveFactor(
                            (d-gun->rSize() < g_game.znear) ? g_game.znear : d-gun->rSize() );
                        float lod = pixradius * g_game.detaillevel;
                        if (lod > 0.5 && pixradius > 2.5) {
                            ScaleMatrix( mat, Vector( mahnt->xyscale, mahnt->xyscale, mahnt->zscale ) );
                            gun->setCurrentFrame( this->mounts[i].ComputeAnimatedFrame( gun ) );
                            gun->Draw( lod, mat, d, cloak,
                                       (_Universe->AccessCamera()->GetNebula() == this->nebula && this->nebula != NULL) ? -1 : 0,
                                       chardamage,
                                       true );                                                                                                                                      //cloakign and nebula
                        }
                        if (mahnt->type->gun1) {
                            pixradius = gun->rSize()*perspectiveFactor(
                                (d-gun->rSize() < g_game.znear) ? g_game.znear : d-gun->rSize() );
                            lod = pixradius * g_game.detaillevel;
                            if (lod > 0.5 && pixradius > 2.5) {
                                gun = mahnt->type->gun1;
                                gun->setCurrentFrame( this->mounts[i].ComputeAnimatedFrame( gun ) );
                                gun->Draw( lod, mat, d, cloak,
                                           (_Universe->AccessCamera()->GetNebula() == this->nebula && this->nebula
                                            != NULL) ? -1 : 0,
                                           chardamage, true );                                                                                                                              //cloakign and nebula
                            }
                        }
                    }
                }
            }
        if (this->mounts[i].type->type == WEAPON_TYPE::BEAM)
            if (this->mounts[i].ref.gun)
                this->mounts[i].ref.gun->Draw( *ct, wmat,
                                               ( isAutoTrackingMount(this->mounts[i].size)
                                                && (this->mounts[i].time_to_lock <= 0)
                                                && Unit::TargetTracked() ) ? Unit::Target() : NULL,
                                               this->computer.radar.trackingcone );
    }
    if ( On_Screen && (phalos->NumHalos() > 0) && !( this->docked&(DOCKED|DOCKED_INSIDE) ) && (Apparent_Size > 5.0f) ) {
        Vector linaccel = this->GetAcceleration();
        Vector angaccel = this->GetAngularAcceleration();
        float  maxaccel = this->GetMaxAccelerationInDirectionOf( wmat.getR(), true );
        Vector velocity = this->GetVelocity();

        float  cmas = this->computer.max_ab_speed()*this->computer.max_ab_speed();
        if (cmas == 0)
            cmas = 1;
        Vector Scale( 1, 1, 1 );         //Now, HaloSystem handles that
        //WARNING: cmas is not a valid maximum speed for the upcoming multi-direction thrusters,
        //nor is maxaccel. Instead, each halo should have its own limits specified in units.csv
        float nebd = (_Universe->AccessCamera()->GetNebula() == this->nebula && this->nebula != NULL) ? -1 : 0;
        float hulld = this->GetHull() > 0 ? damagelevel : 1.0;
        phalos->Draw( wmat, Scale, cloak, nebd, hulld, velocity, linaccel, angaccel, maxaccel, cmas, this->faction );
    }
    if ( On_Screen && !graphicOptions.NoDamageParticles
        && !( this->docked&(DOCKED|DOCKED_INSIDE) ) ) {
        if (damagelevel < .99 && this->nummesh() > 0 && this->GetHull() > 0) {
            unsigned int switcher    = (damagelevel > .8) ? 1
                                       : (damagelevel > .6) ? 2 : (damagelevel > .4) ? 3 : (damagelevel > .2) ? 4 : 5;
            sparkle_accum += GetElapsedTime()*game_options.sparklerate;
            int spawn = (int) (sparkle_accum);
            sparkle_accum -= spawn;
            while (spawn-- > 0) {
                switch (switcher)
                {
                case 5:
                    LaunchOneParticle( *ctm, this->GetVelocity(), ( (long) this )+165, this, damagelevel, this->faction );
                case 4:
                    LaunchOneParticle( *ctm, this->GetVelocity(), ( (long) this )+47, this, damagelevel, this->faction );
                case 3:
                    LaunchOneParticle( *ctm, this->GetVelocity(), ( (long) this )+61, this, damagelevel, this->faction );
                case 2:
                    LaunchOneParticle( *ctm, this->GetVelocity(), ( (long) this )+65537, this, damagelevel, this->faction );
                default:
                    LaunchOneParticle( *ctm, this->GetVelocity(), ( (long) this )+257, this, damagelevel, this->faction );
                }
            }
        } else {
            sparkle_accum = 0;
        }
    } else {
        sparkle_accum = 0;
    }
}


void GameUnit::Draw( const Transformation &quat )
{
    Draw( quat, identity_matrix );
}


void GameUnit::Draw()
{
    Draw( identity_transformation, identity_matrix );
}


static float parseFloat( const std::string &s )
{
    if ( s.empty() ) {
        BOOST_LOG_TRIVIAL(info) << boost::format("WARNING: invalid float: %1%") % s;
        return 0.f;
    } else {
        return XMLSupport::parse_floatf( s );
    }
}

static void parseFloat4( const std::string &s, float value[4] )
{
    string::size_type ini = 0, end;
    int i = 0;
    while (i < 4 && ini != string::npos) {
        value[i++] = parseFloat( s.substr( ini, end = s.find_first_of( ',', ini ) ) );
        ini = ( (end == string::npos) ? end : (end+1) );
    }
    //if (i >= 4 && ini != string::npos) {
    //    BOOST_LOG_TRIVIAL(info) << boost::format("WARNING: invalid float4: %1%") % s;
    //}
    while (i < 4) {
        value[i++] = 0;
    }
}


void GameUnit::applyTechniqueOverrides(const map<string, string> &overrides)
{
    for (vector<Mesh*>::iterator mesh = this->meshdata.begin(); mesh != this->meshdata.end(); ++mesh) {
        if (*mesh != NULL) {
            // First check to see if the technique holds any parameter being overridden
            TechniquePtr technique = (*mesh)->getTechnique();
            if (technique.get() != NULL) {
                bool doOverride = false;
                for (int passno = 0; !doOverride && passno < technique->getNumPasses(); ++passno) {
                    const Pass &pass = technique->getPass(passno);
                    for (size_t paramno = 0; !doOverride && paramno < pass.getNumShaderParams(); ++paramno) {
                        if (overrides.count(pass.getShaderParam(paramno).name) > 0)
                            doOverride = true;
                    }
                }

                if (doOverride) {
                    // Prepare a new technique with the overrides
                    // (make sure the technique has been compiled though -
                    // parameter values don't really need recompilation)
                    TechniquePtr newtechnique = TechniquePtr(new Technique(*technique));
                    for (int passno = 0; passno < technique->getNumPasses(); ++passno) {
                        Pass &pass = technique->getPass(passno);
                        for (size_t paramno = 0; paramno < pass.getNumShaderParams(); ++paramno) {
                            Pass::ShaderParam &param = pass.getShaderParam(paramno);
                            map<string, string>::const_iterator override = overrides.find(param.name);
                            if (override != overrides.end())
                                parseFloat4(override->second, param.value);
                        }
                    }

                    (*mesh)->setTechnique(newtechnique);
                }
            }
        }
    }
}


Matrix GameUnit::WarpMatrix( const Matrix &ctm ) const
{
    if ( this->GetWarpVelocity().MagnitudeSquared() < (game_options.warp_stretch_cutoff * game_options.warp_stretch_cutoff * game_options.game_speed * game_options.game_speed )
        || (game_options.only_stretch_in_warp && this->graphicOptions.InWarp == 0) ) {
        return ctm;
    } else {
        Matrix k( ctm );

        float  speed = this->GetWarpVelocity().Magnitude();
        float stretchregion0length = game_options.warp_stretch_region0_max*(speed-(game_options.warp_stretch_cutoff * game_options.game_speed))/((game_options.warp_stretch_max_region0_speed * game_options.game_speed)-(game_options.warp_stretch_cutoff * game_options.game_speed));
        float stretchlength =
            (game_options.warp_stretch_max
             -game_options.warp_stretch_region0_max)*(speed-(game_options.warp_stretch_max_region0_speed * game_options.game_speed))/((game_options.warp_stretch_max_speed * game_options.game_speed)-(game_options.warp_stretch_max_region0_speed * game_options.game_speed)+.06125f)+game_options.warp_stretch_region0_max;
        if (stretchlength > game_options.warp_stretch_max)
            stretchlength = game_options.warp_stretch_max;
        if (stretchregion0length > game_options.warp_stretch_region0_max)
            stretchregion0length = game_options.warp_stretch_region0_max;
        ScaleMatrix( k, Vector( 1, 1, 1+(speed > (game_options.warp_stretch_max_region0_speed * game_options.game_speed) ? stretchlength : stretchregion0length) ) );
        return k;
    }
}

using Orders::FireAt;

#if 0

void GameUnit::SwapOutHalos()
{
    for (int i = 0; i < numhalos; i++)
        //float x,y;
        //halos[i]->GetDimensions (x,y);    //halos[i]->SetDimensions (x/(1024),y/(1024));
        halos[i]->Draw( cumulative_transformation, cumulative_transformation_matrix, 0 );
}

void GameUnit::SwapInHalos()
{
    for (int i = 0; i < numhalos; i++) {
        //float x,y;
        //halos[i]->GetDimensions (x,y);
        //halos[i]->SetDimensions (x*(1024),y*(1024));
    }
}
#endif
