/*
 * unit.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020-2021 Stephen G. Tuggy and other Vega Strike contributors
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "unit.h"
#include "vs_logging.h"
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

GameUnit::GameUnit( int )
{
    this->Unit::Init();
}

GameUnit::GameUnit( std::vector< Mesh* > &meshes, bool SubU, int fact ) :
    Unit( meshes, SubU, fact )
{}

GameUnit::GameUnit( const char *filename,
                                                           bool SubU,
                                                           int faction,
                                                           std::string unitModifications,
                                                           Flightgroup *flightgrp,
                                                           int fg_subnumber,
                                                           string *netxml )
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
    halos->AddHalo( filename, trans, size, col, halo_type, halo_speed );
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




extern int cloakVal( int cloakint, int cloakminint, int cloakrateint, bool cloakglass ); //short fix?


void GameUnit::DrawNow( const Matrix &mato, float lod )
{
    static const void *rootunit = NULL;
    if (rootunit == NULL) rootunit = (const void*) this;
    float damagelevel = 1.0;
    unsigned char chardamage    = 0;
    if (*this->current_hull < *this->max_hull) {
        damagelevel = (*this->current_hull)/(*this->max_hull);
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
        halos->Draw( mat, Scale, cloak, 0, this->GetHullPercent(), velocity, linaccel, angaccel, maxaccel, cmas, this->faction );
    if (rootunit == (const void*) this) {
        Mesh::ProcessZFarMeshes();
        Mesh::ProcessUndrawnMeshes();
        rootunit = NULL;
    }
}




extern double calc_blend_factor( double frac, unsigned int priority, unsigned int when_it_will_be_simulated, unsigned int cur_simulation_frame );




/*void GameUnit::Draw( const Transformation &quat )
{
    Draw( quat, identity_matrix );
}


void GameUnit::Draw()
{
    Draw( identity_transformation, identity_matrix );
}*/




