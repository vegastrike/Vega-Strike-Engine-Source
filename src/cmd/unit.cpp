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

#include "unit_factory.h"

#include "gfx/sprite.h"
#include "lin_time.h"

#include "gfx/vsbox.h"
#include "bolt.h"
#include "gfx/lerp.h"
//#include "gfx/bsp.h"
#include "audiolib.h"
#include "gfx/cockpit.h"
#include "config_xml.h"
#include "images.h"
//#include "gfx/planetary_transform.h"
///for saving features
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
//#include "unit_template.h"
//#include "gfx/animation.h"
#include "gfx/point_to_cam.h"

//#include "unit_bsp.h"
//these .cpp files should be renamed to .h; they contain template functions...
//--some of them GameUnit<> member functions, at that --chuck_starchaser
// Files changed from .cpp to .h 11-25
/* #include "unit_jump.cpp"
#include "unit_customize.cpp"
#include "unit_damage.cpp"
#include "unit_physics.cpp"
#include "unit_click.cpp" */
#include "base_util.h"
#include "unit_jump.h"
#include "unit_customize.h"
#include "unit_damage.h"
#include "unit_physics.h"
#include "unit_click.h"

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

template < class UnitType >GameUnit< UnitType >::GameUnit( int ) : sparkle_accum( 0 )
    , phalos( new HaloSystem() )
{
    this->Unit::Init();
}

template < class UnitType >GameUnit< UnitType >::GameUnit( std::vector< Mesh* > &meshes, bool SubU, int fact ) :
    UnitType( meshes, SubU, fact )
    , sparkle_accum( 0 )
    , phalos( new HaloSystem() )
{}

template < class UnitType >GameUnit< UnitType >::GameUnit( const char *filename,
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

template < class UnitType >GameUnit< UnitType >::~GameUnit()
{
/*    if (this->pImage->pHudImage)
 *       delete this->pImage->pHudImage;
 *   if (this->pImage->explosion) {
 *       delete this->pImage->explosion;
 *       this->pImage->explosion = NULL;
 *   }*/
    //VSFileSystem::vs_fprintf (stderr,"Freeing Unit %s\n",name.c_str());
    for (unsigned int meshcount = 0; meshcount < this->meshdata.size(); meshcount++)
        if (this->meshdata[meshcount])
            delete this->meshdata[meshcount];
    this->meshdata.clear();
    //delete phalos;
}

template < class UnitType >
int GameUnit< UnitType >::nummesh() const
{
    return ( (int) this->meshdata.size() )-1;
}

template < class UnitType >
void GameUnit< UnitType >::UpgradeInterface( Unit *baseun )
{
    string basename = ( ::getCargoUnitName( baseun->getFullname().c_str() ) );
    if (baseun->isUnit() != PLANETPTR)
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

/*
 *  template <class UnitType>
 *  void GameUnit<UnitType>::Init()
 *  {
 *  this->Unit::Init();
 *  //  unsigned int numg= 1+MAXVDUS+UnitImages::NUMGAUGES;
 *  //  image->cockpit_damage=(float*)malloc((numg)*sizeof(float));
 *  //  for (unsigned int damageiterator=0;damageiterator<numg;damageiterator++) {
 *  //	image->cockpit_damage[damageiterator]=1;
 *  //  }
 *  CollideInfo.object.u = NULL;
 *  CollideInfo.type = LineCollide::UNIT;
 *  CollideInfo.Mini.Set (0,0,0);
 *  CollideInfo.Maxi.Set (0,0,0);
 *  SetAI (new Order());
 *  //yprrestricted=0;
 *  //ymin = pmin = rmin = -PI;
 *  //ymax = pmax = rmax = PI;
 *  //ycur = pcur = rcur = 0;
 *  //static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
 *  //static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
 *  //static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics", "autotracking", ".93"));// DO NOT CHANGE see unit_customize.cpp
 *  //static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
 *  //  Fire();
 *  }
 */

template < class UnitType >
VSSprite*GameUnit< UnitType >::getHudImage() const
{
    return this->pImage->pHudImage;
}

template < class UnitType >
void GameUnit< UnitType >::addHalo( const char *filename,
                                    const QVector &loc,
                                    const Vector &size,
                                    const GFXColor &col,
                                    std::string halo_type,
                                    float halo_speed )
{
    phalos->AddHalo( filename, loc, size, col, halo_type, halo_speed );
}

template < class UnitType >
void GameUnit< UnitType >::Cloak( bool engage )
{
    if (Network != NULL && !SERVER) {
        int which = UnitUtil::isPlayerStarship( this );
        if (which >= 0)
            Network[which].sendCloak( engage );
        else
            UnitType::Cloak( engage );              //client side unit
    } else {
        UnitType::Cloak( engage );         //client side unit
    }
}

template < class UnitType >
bool GameUnit< UnitType >::queryFrustum( double frustum[6][4] ) const
{
    int    i;
#ifdef VARIABLE_LENGTH_PQR
    Vector TargetPoint( cumulative_transformation_matrix[0],
                        cumulative_transformation_matrix[1],
                        cumulative_transformation_matrix[2] );
    float  SizeScaleFactor = sqrtf( TargetPoint.Dot( TargetPoint ) );
#else
    Vector TargetPoint;
#endif
    for (i = 0; i < nummesh(); i++) {
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
        if ( ( (GameUnit< UnitType >*)un )->queryFrustum( frustum ) )
            return true;
    return false;
}

template < class UnitType >
void GameUnit< UnitType >::UpdateHudMatrix( int whichcam )
{
    Matrix m;
    Matrix ctm = this->cumulative_transformation_matrix;
    /*if (this->planet) {
     *  Transformation ct (linear_interpolate(this->prev_physical_state, this->curr_physical_state, interpolation_blend_factor));
     *  ct.to_matrix (m);
     *  ctm=m;
     *  }*/
    Vector q( ctm.getQ() );
    Vector r( ctm.getR() );
    Vector tmp;
    CrossProduct( r, q, tmp );
    _Universe->AccessCamera( whichcam )->SetOrientation( tmp, q, r );

    _Universe->AccessCamera( whichcam )->SetPosition( Transform( ctm,
                                                                this->pImage->CockpitCenter.Cast() ),
                                                     this->GetWarpVelocity(), this->GetAngularVelocity(), this->GetAcceleration() );
}

extern bool flickerDamage( Unit *un, float hullpercent );
extern int cloakVal( int cloakint, int cloakminint, int cloakrateint, bool cloakglass ); //short fix?

template < class UnitType >
void GameUnit< UnitType >::DrawNow( const Matrix &mato, float lod )
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
        cloak = cloakVal( cloak, this->cloakmin, this->pImage->cloakrate, this->pImage->cloakglass );
    for (i = 0; (int) i < this->nummesh(); i++) {
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
    for (un_kiter iter = this->SubUnits.constIterator(); (un = *iter); ++iter) {
        Matrix temp;
        un->curr_physical_state.to_matrix( temp );
        Matrix submat;
        MultMatrix( submat, mat, temp );
        (un)->DrawNow( submat, lod );
    }
    float  haloalpha   = 1;
    if (cloak >= 0)
        haloalpha = ( (float) cloak )/2147483647;
#ifdef CAR_SIM
    Vector Scale( 1, pImage->ecm, computer.set_speed );
#else
    float  cmas = this->computer.max_ab_speed()*this->computer.max_ab_speed();
    if (cmas == 0)
        cmas = 1;
        Vector Scale( 1, 1, 1 );         //Now, HaloSystem handles that
#endif
    int    nummounts = this->GetNumMounts();
    for (i = 0; (int) i < nummounts; i++) {
        static bool draw_mounts = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_weapons", "false" ) );
        Mount *mahnt = &this->mounts[i];
        if (draw_mounts)
            if (mahnt->xyscale != 0 && mahnt->zscale != 0) {
                Mesh *gun = mahnt->type->gun;
                if (gun && mahnt->status != Mount::UNCHOSEN) {
                    Transformation mountLocation( mahnt->GetMountOrientation(), mahnt->GetMountLocation().Cast() );
                    mountLocation.Compose( Transformation::from_matrix( mat ), this->WarpMatrix( mat ) );
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
    Vector accel    = this->GetAcceleration();
    float  maxaccel = this->GetMaxAccelerationInDirectionOf( mat.getR(), true );
    Vector velocity = this->GetVelocity();
    if ( phalos->ShouldDraw( mat, velocity, accel, maxaccel,
                             cmas ) && !( this->docked&(UnitType::DOCKED|UnitType::DOCKED_INSIDE) ) )
        phalos->Draw( mat, Scale, cloak, 0, this->GetHullPercent(), velocity, accel, maxaccel, cmas, this->faction );
    if (rootunit == (const void*) this) {
        Mesh::ProcessZFarMeshes();
        Mesh::ProcessUndrawnMeshes();
        rootunit = NULL;
    }
}

template < class UnitType >
void GameUnit< UnitType >::DrawNow()
{
    DrawNow( identity_matrix, 1000000000 );
}

extern double calc_blend_factor( double frac, int priority, int when_it_will_be_simulated, int cur_simulation_frame );

template < class UnitType >
void GameUnit< UnitType >::Draw( const Transformation &parent, const Matrix &parentMatrix )
{
    //Quick shortcut for camera setup phase
    bool myparent = ( this == _Universe->AccessCockpit()->GetParent() );

#ifdef VARIABLE_LENGTH_PQR
    const float     vlpqrScaleFactor = SizeScaleFactor;
#else
    const float     vlpqrScaleFactor = 1.f;
#endif

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

    int cloak = this->cloaking;
    if (this->cloaking > this->cloakmin) {
        cloak = (int) (this->cloaking-interpolation_blend_factor*this->pImage->cloakrate*SIMULATION_ATOM);
        cloak = cloakVal( cloak, this->cloakmin, this->pImage->cloakrate, this->pImage->cloakglass );
    }
    
    unsigned int i;
    if ( (this->hull < 0) && (!cam_setup_phase) )
        Explode( true, GetElapsedTime() );
    
    float damagelevel = 1.0f;
    unsigned char chardamage = 0;
    if (this->hull < this->maxhull && !cam_setup_phase) {
        damagelevel = this->hull/this->maxhull;
        chardamage  = ( 255-(unsigned char) (damagelevel*255) );
    }
    
    bool On_Screen = false;
    if ( ( !(this->invisible&UnitType::INVISUNIT) ) && ( ( !(this->invisible&UnitType::INVISCAMERA) ) || (!myparent) ) ) {
        if (!cam_setup_phase) {
            Camera *camera = _Universe->AccessCamera();
            QVector camerapos = camera->GetPosition();
            
            float minmeshradius =
                ( camera->GetVelocity().Magnitude()+this->Velocity.Magnitude() )*SIMULATION_ATOM;
            
            unsigned int numKeyFrames = this->graphicOptions.NumAnimationPoints;
            for (i = 0; i < this->meshdata.size(); i++) {
                //NOTE LESS THAN OR EQUALS...to cover shield mesh
                if (this->meshdata[i] == NULL)
                    continue;
                if ( (int) i == this->nummesh() && (this->meshdata[i]->numFX() == 0 || this->hull < 0) )
                    continue;
                if (this->meshdata[i]->getBlendDst() == ONE) {
                    if ( (this->invisible&UnitType::INVISGLOW) != 0 )
                        continue;
                    if (damagelevel < .9)
                        if ( flickerDamage( this, damagelevel ) )
                            continue;
                }
                QVector TransformedPosition = Transform( *ctm, this->meshdata[i]->Position().Cast() );

                double d = GFXSphereInFrustum( TransformedPosition,
                                               minmeshradius+this->meshdata[i]->clipRadialSize()*vlpqrScaleFactor );
                if (d) {
                    //d can be used for level of detail shit
                    d = ( TransformedPosition-camerapos ).Magnitude();
                    double rd  = d-this->meshdata[i]->rSize();
                    double pixradius = this->meshdata[i]->rSize()*perspectiveFactor(
                        (rd < g_game.znear) ? g_game.znear : rd );
                    double lod = pixradius*g_game.detaillevel;
                    if (lod >= 0.5 && pixradius >= 2.5) {
                        //if the radius is at least half a pixel at detail 1 (equivalent to pixradius >= 0.5 / detail)
                        float currentFrame = this->meshdata[i]->getCurrentFrame();
                        this->meshdata[i]->Draw( lod, this->WarpMatrix( *ctm ), d, 
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
        }
        {
            Unit  *un;
            double backup = interpolation_blend_factor;
            int    cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
            for (un_kiter iter = this->SubUnits.constIterator(); (un = *iter); ++iter) {
                float backup = SIMULATION_ATOM;
                if (this->sim_atom_multiplier && un->sim_atom_multiplier)
                    SIMULATION_ATOM = SIMULATION_ATOM*un->sim_atom_multiplier/this->sim_atom_multiplier;
                interpolation_blend_factor = calc_blend_factor( saved_interpolation_blend_factor,
                                                                un->sim_atom_multiplier,
                                                                un->cur_sim_queue_slot,
                                                                cur_sim_frame );
                (un)->Draw( *ct, *ctm );

                SIMULATION_ATOM = backup;
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
    int nummounts = this->GetNumMounts();
    for (i = 0; (int) i < nummounts; i++) {
        static bool draw_mounts = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_weapons", "false" ) );
        Mount *mahnt = &this->mounts[i];
        if (draw_mounts && On_Screen)
//Mesh * gun = WeaponMeshCache::getCachedMutable (mounts[i]->type->weapon_name);
            if (mahnt->xyscale != 0 && mahnt->zscale != 0) {
                Mesh *gun = mahnt->type->gun;
                if (gun && mahnt->status != Mount::UNCHOSEN) {
                    Transformation mountLocation( mahnt->GetMountOrientation(), mahnt->GetMountLocation().Cast() );
                    mountLocation.Compose( *ct, this->WarpMatrix( *ctm ) );
                    Matrix mat;
                    mountLocation.to_matrix( mat );
                    if (GFXSphereInFrustum( mountLocation.position, gun->rSize()*vlpqrScaleFactor ) > 0) {
                        float d   = ( mountLocation.position-_Universe->AccessCamera()->GetPosition() ).Magnitude();
                        float lod = gun->rSize()*g_game.detaillevel*perspectiveFactor(
                            (d-gun->rSize() < g_game.znear) ? g_game.znear : d-gun->rSize() );
                        ScaleMatrix( mat, Vector( mahnt->xyscale, mahnt->xyscale, mahnt->zscale ) );
                        gun->setCurrentFrame( this->mounts[i].ComputeAnimatedFrame( gun ) );
                        gun->Draw( lod, mat, d, cloak,
                                   (_Universe->AccessCamera()->GetNebula() == this->nebula && this->nebula != NULL) ? -1 : 0,
                                   chardamage,
                                   true );                                                                                                                                      //cloakign and nebula
                        if (mahnt->type->gun1) {
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
        if (this->mounts[i].type->type == weapon_info::BEAM)
            if (this->mounts[i].ref.gun)
                this->mounts[i].ref.gun->Draw( *ct, this->WarpMatrix( *ctm ),
                                               ( (this->mounts[i].size&weapon_info::AUTOTRACKING)
                                                && (this->mounts[i].time_to_lock <= 0)
                                                && (this->computer.radar.trackingactive) ) ? Unit::Target() : NULL, 
                                               this->computer.radar.trackingcone );
    }
    float haloalpha = 1;
    if (cloak >= 0)
        haloalpha = ( (float) cloak )/2147483647;
    if ( On_Screen && (phalos->NumHalos() > 0) && !( this->docked&(UnitType::DOCKED|UnitType::DOCKED_INSIDE) ) ) {
        Vector accel    = this->GetAcceleration();
        float  maxaccel = this->GetMaxAccelerationInDirectionOf( this->WarpMatrix( *ctm ).getR(), true );
        Vector velocity = this->GetVelocity();
        //float enginescale = this->GetVelocity().MagnitudeSquared();
#ifdef CAR_SIM
        Vector Scale( 1, pImage->ecm, computer.set_speed );
#else
        float  cmas = this->computer.max_ab_speed()*this->computer.max_ab_speed();
        if (cmas == 0)
            cmas = 1;
        /*if (enginescale>cmas)
         *  enginescale=cmas;
         *  Vector Scale (1,1,enginescale/(cmas));*/
        Vector Scale( 1, 1, 1 );         //Now, HaloSystem handles that
#endif
        //WARNING: cmas is not a valid maximum speed for the upcoming multi-direction thrusters,
        //nor is maxaccel. Instead, each halo should have its own limits specified in units.csv
        if ( phalos->ShouldDraw( this->WarpMatrix( *ctm ), velocity, accel, maxaccel, cmas ) )
            phalos->Draw( this->WarpMatrix( *ctm ), Scale, cloak,
                          (_Universe->AccessCamera()->GetNebula() == this->nebula && this->nebula != NULL) ? -1 : 0,
                          this->GetHull() > 0 ? damagelevel : 1.0, velocity, accel, maxaccel, cmas, this->faction );
    }
    if ( On_Screen && !UnitType::graphicOptions.NoDamageParticles
        && !( this->docked&(UnitType::DOCKED|UnitType::DOCKED_INSIDE) ) ) {
        int numm = this->nummesh();
        if (damagelevel < .99 && numm > 0 && this->GetHull() > 0) {
            unsigned int switcher    = (damagelevel > .8) ? 1
                                       : (damagelevel > .6) ? 2 : (damagelevel > .4) ? 3 : (damagelevel > .2) ? 4 : 5;
            static float sparklerate = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparklerate", "5" ) );
            sparkle_accum += GetElapsedTime()*sparklerate;
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
        }
    }
}

template < class UnitType >
void GameUnit< UnitType >::Draw( const Transformation &quat )
{
    Draw( quat, identity_matrix );
}

template < class UnitType >
void GameUnit< UnitType >::Draw()
{
    Draw( identity_transformation, identity_matrix );
}

    
static float parseFloat( const std::string &s )
{
    if ( s.empty() ) {
        VSFileSystem::vs_dprintf(1, "WARNING: invalid float: %s\n", s.c_str());
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
    if (i >= 4 && ini != string::npos)
        VSFileSystem::vs_dprintf(1, "WARNING: invalid float4: %s\n", s.c_str());
    while (i < 4)
        value[i++] = 0;
}

template < class UnitType >
void GameUnit< UnitType >::applyTechniqueOverrides(const map<string, string> &overrides)
{
    for (vector<Mesh*>::iterator mesh = this->meshdata.begin(); mesh != this->meshdata.end(); ++mesh) {
        if (*mesh != NULL) {
            // First check to see if the technique holds any parameter being overridden
            TechniquePtr technique = (*mesh)->getTechnique();
            if (technique.get() != NULL) {
                bool doOverride = false;
                for (int passno = 0; !doOverride && passno < technique->getNumPasses(); ++passno) {
                    const Technique::Pass &pass = technique->getPass(passno);
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
                        Technique::Pass &pass = technique->getPass(passno);
                        for (size_t paramno = 0; paramno < pass.getNumShaderParams(); ++paramno) {
                            Technique::Pass::ShaderParam &param = pass.getShaderParam(paramno);
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

template < class UnitType >
Matrix GameUnit< UnitType >::WarpMatrix( const Matrix &ctm ) const
{
    static float cutoff =
        XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_stretch_cutoff",
                                                         "50000" ) )*XMLSupport::parse_float(
            vs_config->getVariable( "physics", "game_speed", "1" ) );
    static float cutoffcutoff = cutoff*cutoff;
    static bool  only_stretch_in_warp =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "only_stretch_in_warp", "true" ) );
    if ( this->GetWarpVelocity().MagnitudeSquared() < cutoffcutoff
        || (only_stretch_in_warp && this->graphicOptions.InWarp == 0) ) {
        return ctm;
    } else {
        Matrix k( ctm );

        float  speed = this->GetWarpVelocity().Magnitude();
        //Matrix scalar=identity_matrix;
        static float maxregion0stretch =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_stretch_region0_max", "1" ) );

        static float maxstretch = XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_stretch_max", "4" ) );
        static float maxspeed   =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_stretch_max_speed",
                                                             "1000000" ) )
            *XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
        static float maxregion0speed =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_stretch_max_region0_speed",
                                                             "100000" ) )
            *XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
        float stretchregion0length = maxregion0stretch*(speed-cutoff)/(maxregion0speed-cutoff);
        float stretchlength =
            (maxstretch
             -maxregion0stretch)*(speed-maxregion0speed)/(maxspeed-maxregion0speed+.06125)+maxregion0stretch;
        if (stretchlength > maxstretch)
            stretchlength = maxstretch;
        if (stretchregion0length > maxregion0stretch)
            stretchregion0length = maxregion0stretch;
        ScaleMatrix( k, Vector( 1, 1, 1+(speed > maxregion0speed ? stretchlength : stretchregion0length) ) );
        return k;
    }
}

using Orders::FireAt;

#if 0
template < class UnitType >
void GameUnit< UnitType >::SwapOutHalos()
{
    for (int i = 0; i < numhalos; i++)
        //float x,y;
        //halos[i]->GetDimensions (x,y);    //halos[i]->SetDimensions (x/(1024),y/(1024));
        halos[i]->Draw( cumulative_transformation, cumulative_transformation_matrix, 0 );
}
template < class UnitType >
void GameUnit< UnitType >::SwapInHalos()
{
    for (int i = 0; i < numhalos; i++) {
        //float x,y;
        //halos[i]->GetDimensions (x,y);
        //halos[i]->SetDimensions (x*(1024),y*(1024));
    }
}
#endif

/////////////////////////////////////////////////////
//explicit instantiations, added by chuck_starchaser:


 #include "cmd/asteroid_generic.h"
template class GameUnit< Asteroid >;

 #include "cmd/building_generic.h"
template class GameUnit< Building >;

 #include "cmd/planet_generic.h"
template class GameUnit< Planet >;

 #include "cmd/unit_generic.h"
template class GameUnit< Unit >;

 #include "cmd/missile_generic.h"
template class GameUnit< Missile >;

 #include "cmd/nebula.h"
template class GameUnit< Nebula >;

 #include "cmd/enhancement.h"
template class GameUnit< Enhancement >;

//The unit types below don't compile; --probably they don't inherit from Unit.

//#include "gfx/mesh.h"
//template class GameUnit< Mesh >;

//#include "cmd/script/flightgroup.h"
//template class GameUnit< Flightgroup >;

//#include "cmd/terrain.h"
//template class GameUnit< Terrain >;

//#include "cmd/cont_terrain.h"
//template class GameUnit< ContinuousTerrain >;


/////////////////////////////////////////////////////


