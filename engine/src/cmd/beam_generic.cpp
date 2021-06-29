/**
 * beam_generic.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
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


#include "vegastrike.h"
#include <vector>
#include "beam.h"
#include "unit_generic.h"
#include "audiolib.h"
#include "configxml.h"
#include "images.h"
#include "gfx/camera.h"
#include "universe.h"
#include "weapon_info.h"

using namespace XMLSupport;
extern double interpolation_blend_factor;
extern bool AdjustMatrix( Matrix &mat, const Vector &velocity, Unit *target, float speed, bool lead, float cone );

inline static float mysqr( float a )
{
    return a*a;
}

inline static float mymax( float a, float b )
{
    return (a > b) ? a : b;
}

inline static float mymin( float a, float b )
{
    return (a < b) ? a : b;
}

float Beam::refireTime()
{
    return refiretime;
}

void Beam::SetPosition( const QVector &k )
{
    local_transformation.position = k;
}

void Beam::SetOrientation( const Vector &p, const Vector &q, const Vector &r )
{
    local_transformation.orientation = Quaternion::from_vectors( p, q, r );
}

void ScaleByAlpha( GFXColorVertex &vert, float alpha )
{
    if (alpha < 1) {
        vert.r *= alpha;
        vert.g *= alpha;
        vert.b *= alpha;
    }
}

void Beam::Init( const Transformation &trans, const weapon_info &cln, void *own, Unit *firer )
{
    //Matrix m;
    CollideInfo.object.b = NULL;
    CollideInfo.type     = LineCollide::BEAM;
    //DO NOT DELETE - shared vlist
    //if (vlist)
    //delete vlist;
    local_transformation = trans;     //location on ship
    //cumalative_transformation =trans;
    //trans.to_matrix (cumalative_transformation_matrix);
    speed = cln.speed;
    texturespeed   = cln.pulse_speed;
    range          = cln.range;
    radialspeed    = cln.radial_speed;
    thickness      = cln.radius;
    stability      = cln.stability;
    rangepenalty   = cln.long_range;
    damagerate     = cln.damage;
    phasedamage    = cln.phase_damage;
    texturestretch = cln.texture_stretch;
    refiretime     = 0;
    refire         = cln.Refire();
    Col.r          = cln.r;
    Col.g          = cln.g;
    Col.b          = cln.b;
    Col.a          = cln.a;
    impact         = ALIVE;
    owner          = own;
    numframes      = 0;
    static int  radslices  = XMLSupport::parse_int( vs_config->getVariable( "graphics", "tractor.scoop_rad_slices", "10" ) )|1;    //Must be odd
    static int  longslices = XMLSupport::parse_int( vs_config->getVariable( "graphics", "tractor.scoop_long_slices", "10" ) );
    lastlength = 0;
    curlength  = simulation_atom_var*speed;
    lastthick  = 0;
    curthick   = simulation_atom_var*radialspeed;
    if (curthick > thickness)      //clamp to max thickness - needed for large simulation atoms
        curthick = thickness;
    static GFXVertexList *_vlist = 0;
    if (!_vlist) {
        int numvertex = float_to_int( mymax( 48, ( (4*radslices)+1 )*longslices*4 ) );
        GFXColorVertex *beam = new GFXColorVertex[numvertex];         //regretably necessary: radslices and longslices come from the config file... so it's at runtime.
//        memset( beam, 0, sizeof (*beam)*numvertex );
        _vlist = new GFXVertexList( GFXQUAD, numvertex, beam, numvertex, true );         //mutable color contained list
       delete[] beam;
    }
    //Shared vlist - we recalculate it every time, so no loss
    vlist = _vlist;
#ifdef PERBOLTSOUND
    AUDStartPlaying( sound );
#endif
}

//NOTE: The order of the quad's vertices IS important - it ensures symmetric interpolation.
#define V( xx, yy, zz, ss, tt, aa ) \
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

void Beam::RecalculateVertices( const Matrix &trans )
{
    GFXColorVertex *beam = ( vlist->BeginMutate( 0 ) )->colors;
    static float    fadelocation    = XMLSupport::parse_float( vs_config->getVariable( "graphics", "BeamFadeoutLength", ".8" ) );
    static float    hitfadelocation =
        XMLSupport::parse_float( vs_config->getVariable( "graphics", "BeamFadeoutHitLength", ".95" ) );
    static float    scoopangle    =
        //In radians - the /2 is because of the way in which we check against the cone.
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.scoop_fov", "0.5" ) )/2;
    static float    scooptanangle = (float) tan( scoopangle );
    static bool     scoop  = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "tractor.scoop", "true" ) );
    static float    scoopa =
        XMLSupport::parse_float( vs_config->getVariable( "graphics", "tractor.scoop_alpha_multiplier", "2.5" ) );
    static int  radslices    =
        XMLSupport::parse_int( vs_config->getVariable( "graphics", "tractor.scoop_rad_slices", "10" ) )|1;         //Must be odd
    static int  longslices   =
        XMLSupport::parse_int( vs_config->getVariable( "graphics", "tractor.scoop_long_slices", "10" ) );
    const float fadeinlength = 4;
    const bool  tractor = (damagerate < 0 && phasedamage > 0);
    const bool  repulsor     = (damagerate > 0 && phasedamage < 0);
    float leftex             = -texturespeed*(numframes*simulation_atom_var+interpolation_blend_factor*simulation_atom_var);
    float righttex           = leftex+texturestretch*curlength/curthick;           //how long compared to how wide!
    float len                = (impact == ALIVE)
                               ? (curlength < range ? curlength-speed*simulation_atom_var*(1-interpolation_blend_factor) : range)
                               : curlength;
    float fadelen            = (impact == ALIVE) ? len*fadelocation : len*hitfadelocation;
    const bool doscoop       = ( scoop && (tractor || repulsor) );
    float fadetex            = leftex+(righttex-leftex)*fadelocation;
    const float touchtex     = leftex-fadeinlength*.5*texturestretch;
    float thick              = curthick != thickness ? curthick-radialspeed*simulation_atom_var
                               *(1-interpolation_blend_factor) : thickness;
    float ethick             = ( thick/( (thickness > 0) ? thickness : 1.0f ) )*(doscoop ? curlength*scooptanangle : 0);
    const float invfadelen   = thick*fadeinlength;
    const float invfadealpha = mymax( 0.0f, mymin( 1.0f, 1.0f-mysqr( invfadelen/len ) ) );
    const float fadealpha    = mymax( 0.0f, mymin( 1.0f, 1.0f-mysqr( fadelen/len ) ) );
    const float endalpha     = 0.0f;
    const float peralpha     = doscoop ? 0.25f : 0.0f;
    int a = 0;
    if (doscoop) {
        //Do the volumetric thingy
        Vector r( _Universe->AccessCamera()->GetR() );
        Vector x( trans.getP() ), y( trans.getQ() ), z( trans.getR() );
        r.Normalize();
        x.Normalize();
        y.Normalize();
        z.Normalize();
        const float xyalpha  = mymax( 0, fabs( z*r ) );
        const float xzalpha  = mymax( 0, fabs( y*r ) )*0.5f;
        const float yzalpha  = mymax( 0, fabs( x*r ) )*0.5f;
        const float lislices = (longslices > 0) ? 1.0f/longslices : 0.0f;
        const float rislices = (radslices > 0) ? 1.0f/radslices : 0.0f;
        const float bxyalpha = xyalpha*lislices;
        const float bxzalpha = xzalpha*rislices;
        const float byzalpha = yzalpha*rislices;
        const float zs = lislices*(fadelen-invfadelen);
        const float ths = lislices*ethick*1.2f;
        const float rim1     = (radslices-1)*rislices*2;
        for (int i = 0; i < longslices; i++) {
            float f  = i*lislices;
            float xa = mymax( 0, 1.0f-mysqr( f ) )*byzalpha*scoopa;
            float ya = mymax( 0, 1.0f-mysqr( f ) )*bxzalpha*scoopa;
            float za = mymax( 0, 1.0f-mysqr( f ) )*bxyalpha*scoopa;
            float th = f*ethick+thick;
            float z  = i*zs+invfadelen;
            if (za > 0.03) {
                V( -th, +th, z, 0, 0.5f, za );
                V( -th, -th, z, 0, 0.0f, za );
                V( +th, -th, z, 1, 0.0f, za );
                V( +th, +th, z, 1, 0.5f, za );
            }
            if (i > 1) {
                if (ya > 0.03) {
                    for (int j = -radslices/2; j <= radslices/2; j++) {
                        float y  = j*2*th*rislices;
                        float f  = 1.0f-fabs( rim1*j*rislices );
                        float sf = sqrt( f );
                        float aa = ya*f;
                        if (aa > 0.03) {
                            V( -(th+ths)*sf, y, z+zs, 1, 0.50f, aa );
                            V( -(th-ths)*sf, y, z-zs, 0, 0.50f, aa );
                            V( 0, y, z-zs, 0, 0.75f, aa );
                            V( 0, y, z+zs, 1, 0.75f, aa );
                            V( 0, y, z+zs, 1, 0.75f, aa );
                            V( 0, y, z-zs, 0, 0.75f, aa );
                            V( +(th-ths)*sf, y, z-zs, 0, 1.00f, aa );
                            V( +(th+ths)*sf, y, z+zs, 1, 1.00f, aa );
                        }
                    }
                }
                if (xa > 0.03) {
                    for (int j = -radslices/2; j <= radslices/2; j++) {
                        float x  = j*2*th*rislices;
                        float f  = 1.0f-fabs( rim1*j*rislices );
                        float sf = sqrt( f );
                        float aa = xa*f;
                        if (aa > 0.03) {
                            V( x, -(th+ths)*sf, z+zs, 1, 0.50f, aa );
                            V( x, -(th-ths)*sf, z-zs, 0, 0.50f, aa );
                            V( x, 0, z-zs, 0, 0.75f, aa );
                            V( x, 0, z+zs, 1, 0.75f, aa );
                            V( x, 0, z+zs, 1, 0.75f, aa );
                            V( x, 0, z-zs, 0, 0.75f, aa );
                            V( x, +(th-ths)*sf, z-zs, 0, 1.00f, aa );
                            V( x, +(th+ths)*sf, z+zs, 1, 1.00f, aa );
                        }
                    }
                }
            }
        }
    } else {
        //main section
        V( 0, 0, invfadelen, leftex, 0.5f, invfadealpha );
        V( 0, thick, invfadelen, leftex, 1, peralpha*invfadealpha );
        V( 0, thick, fadelen, fadetex, 1, peralpha*fadealpha );
        V( 0, 0, fadelen, fadetex, 0.5f, fadealpha );
        V( 0, 0, invfadelen, leftex, 0.5f, invfadealpha );
        V( 0, 0, fadelen, fadetex, 0.5f, fadealpha );
        V( 0, -thick, fadelen, fadetex, 0, peralpha*fadealpha );
        V( 0, -thick, invfadelen, leftex, 0, peralpha*invfadealpha );
        //fade out
        V( 0, 0, fadelen, fadetex, 0.5f, fadealpha );
        V( 0, thick, fadelen, fadetex, 1, peralpha*fadealpha );
        V( 0, thick, len, righttex, 1, peralpha*endalpha );
        V( 0, 0, len, righttex, 0.5f, endalpha );
        V( 0, 0, fadelen, fadetex, 0.5f, fadealpha );
        V( 0, 0, len, righttex, 0.5f, endalpha );
        V( 0, -thick, len, righttex, 0, peralpha*endalpha );
        V( 0, -thick, fadelen, fadetex, 0, peralpha*fadealpha );
        //fade in
        V( 0, 0, invfadelen, leftex, 0.5f, invfadealpha );
        V( 0, thick, invfadelen, leftex, 1, peralpha*invfadealpha );
        V( 0, thick, 0, touchtex, 1, peralpha );
        V( 0, 0, 0, touchtex, 0.5f, 1.0f );
        V( 0, 0, invfadelen, leftex, 0.5f, invfadealpha );
        V( 0, 0, 0, touchtex, 0.5f, 1.0f );
        V( 0, -thick, 0, touchtex, 0, peralpha );
        V( 0, -thick, invfadelen, leftex, 0, peralpha*invfadealpha );
        //copy and rotate xy plane
        for (int i = 0, upto = a; i < upto; i++, a++) {
            beam[a]   = beam[i];
            float aux = beam[a].x;
            beam[a].x = beam[a].y;
            beam[a].y = aux;
        }
    }
    vlist->EndMutate( a );
}

#undef V

void Beam::RemoveFromSystem( bool eradicate )
{}

void Beam::UpdatePhysics( const Transformation &trans,
                          const Matrix &m,
                          Unit *targ,
                          float tracking_cone,
                          Unit *targetToCollideWith,
                          float HeatSink,
                          Unit *firer,
                          Unit *superunit )
{
    curlength += simulation_atom_var*speed;
    if (curlength < 0)
        curlength = 0;
    if (curlength > range)
        curlength = range;
    if (curthick == 0) {
        if (AUDIsPlaying( sound ) && refiretime >= simulation_atom_var)
            AUDStopPlaying( sound );
        refiretime += simulation_atom_var*HeatSink;
        return;
    }
    if (stability && numframes*simulation_atom_var > stability)
        impact |= UNSTABLE;
    numframes++;
    Matrix cumulative_transformation_matrix;
    Transformation cumulative_transformation = local_transformation;
    cumulative_transformation.Compose( trans, m );
    cumulative_transformation.to_matrix( cumulative_transformation_matrix );
    bool possible = AdjustMatrix( cumulative_transformation_matrix, Vector( 0, 0, 0 ), targ, speed, false, tracking_cone );
    static bool firemissingautotrackers =
        XMLSupport::parse_bool( vs_config->getVariable( "physics", "fire_missing_autotrackers", "true" ) );
    if (targ && possible == false && !firemissingautotrackers)
        Destabilize();
    //to help check for crashing.
    center    = cumulative_transformation.position;
    direction = TransformNormal( cumulative_transformation_matrix, Vector( 0, 0, 1 ) );
#ifndef PERFRAMESOUND
    AUDAdjustSound( sound, cumulative_transformation.position, speed*cumulative_transformation_matrix.getR() );
#endif
    curthick += (impact&UNSTABLE) ? -radialspeed*simulation_atom_var : radialspeed*simulation_atom_var;
    if (curthick > thickness)
        curthick = thickness;
    if (curthick <= 0) {
        curthick = 0; //die die die
#ifdef BEAMCOLQ
        RemoveFromSystem( false );
#endif
    } else {
        CollideHuge( CollideInfo, listen_to_owner ? targetToCollideWith : NULL, firer, superunit );
        if ( !(curlength <= range && curlength > 0) ) {
            //if curlength just happens to be nan --FIXME THIS MAKES NO SENSE AT ALL --chuck_starchaser
            if (curlength > range)
                curlength = range;
            else
                curlength = 0;
        }
        QVector tmpvec( center+direction.Cast().Scale( curlength ) );
        QVector tmpMini = center.Min( tmpvec );
        tmpvec = center.Max( tmpvec );
#ifdef BEAMCOLQ
        if ( TableLocationChanged( CollideInfo, tmpMini, tmpvec ) || (curthick > 0 && CollideInfo.object.b == NULL) ) {
            RemoveFromSystem( false );
#endif
          CollideInfo.object.b = this;
          CollideInfo.hhuge    =
              ( ( (CollideInfo.Maxi.i
                   -CollideInfo.Mini.i)
                 /coltableacc )
               *( (CollideInfo.Maxi.j-CollideInfo.Mini.j)/coltableacc )*(CollideInfo.Maxi.k-CollideInfo.Mini.k)/coltableacc
               > tablehuge );
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

extern Cargo * GetMasterPartList( const char* );

bool Beam::Collide( Unit *target, Unit *firer, Unit *superunit )
{
    if (target == NULL) {
        BOOST_LOG_TRIVIAL(error) << "Recovering from nonfatal beam error when beam inactive\n";
        return false;
    }
    float distance;
    Vector normal;     //apply shields

    QVector direction( this->direction.Cast() );
    QVector end( center+direction.Scale( curlength ) );
    enum _UnitType type = target->isUnit();
    if (target == owner || type == _UnitType::nebula || type == _UnitType::asteroid) {
        static bool collideroids =
            XMLSupport::parse_bool( vs_config->getVariable( "physics", "AsteroidWeaponCollision", "false" ) );
        if ( type != _UnitType::asteroid || (!collideroids) )
            return false;
    }
    static bool collidejump = XMLSupport::parse_bool( vs_config->getVariable( "physics", "JumpWeaponCollision", "false" ) );
    if ( type == _UnitType::planet && (!collidejump) && !target->GetDestinations().empty() )
        return false;
    //A bunch of needed config variables - its best to have them here, so that they're loaded the
    //very first time Collide() is called. That way, we avoid hiccups.
    static float nbig = XMLSupport::parse_float( vs_config->getVariable( "physics", "percent_to_tractor", ".1" ) );
    int upgradesfaction     = FactionUtil::GetUpgradeFaction();
    static int cargofaction = FactionUtil::GetFactionIndex( "cargo" );
    static bool c_fp  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.cargo.force_push", "true" ) );
    static bool c_fi  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.cargo.force_in", "true" ) );
    static bool u_fp  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.upgrade.force_push", "true" ) );
    static bool u_fi  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.upgrade.force_in", "true" ) );
    static bool f_fp  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.faction.force_push", "true" ) );
    static bool f_fi  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.faction.force_in", "true" ) );
    static bool d_fp  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.disabled.force_push", "true" ) );
    static bool d_fi  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.disabled.force_in", "true" ) );
    static bool o_fp  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.others.force_push", "false" ) );
    static bool o_fi  = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.others.force_in", "false" ) );
    static bool scoop = XMLSupport::parse_bool( vs_config->getVariable( "physics", "tractor.scoop", "true" ) );
    static float scoopangle    = XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.scoop_angle", "0.5" ) );     //In radians
    static float scoopcosangle = (float) cos( scoopangle );
    static float maxrelspeed   =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.max_relative_speed", "150" ) );
    static float c_ors_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.cargo.distance_own_rsize", "1.5" ) );
    static float c_trs_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.cargo.distance_tgt_rsize", "1.1" ) );
    static float c_o     = XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.cargo.distance", "0" ) );
    static float u_ors_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.ugprade.distance_own_rsize", "1.5" ) );
    static float u_trs_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.upgrade.distance_tgt_rsize", "1.1" ) );
    static float u_o     = XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.upgrade.distance", "0" ) );
    static float f_ors_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.faction.distance_own_rsize", "2.2" ) );
    static float f_trs_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.faction.distance_tgt_rsize", "2.2" ) );
    static float f_o     = XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.faction.distance", "0" ) );
    static float o_ors_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.others.distance_own_rsize", "1.1" ) );
    static float o_trs_m =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.others.distance_tgt_rsize", "1.1" ) );
    static float o_o     = XMLSupport::parse_float( vs_config->getVariable( "physics", "tractor.others.distance", "0" ) );
    bool tractor  = (damagerate < 0 && phasedamage > 0);
    bool repulsor = (damagerate > 0 && phasedamage < 0);
    if ( scoop && (tractor || repulsor) ) {
        QVector d2( target->Position()-center );
        d2.Normalize();
        float angle = this->direction*d2;
        if (angle > scoopcosangle) {
            end = center+d2*curlength;
            direction = end-center;
            direction.Normalize();
        }
    }
    Unit *colidee;
    if ( ( colidee = target->rayCollide( center, end, normal, distance ) ) ) {
        if ( !( scoop && (tractor || repulsor) ) ){
            this->curlength = distance;
        }
        float curlength = distance;
        impact |= IMPACT;
        GFXColor coltmp( Col );
        float tmp = (curlength/range);
        float appldam     = (damagerate*simulation_atom_var*curthick/thickness)*( (1-tmp)+tmp*rangepenalty );
        float phasdam     = (phasedamage*simulation_atom_var*curthick/thickness)*( (1-tmp)+tmp*rangepenalty );
        float owner_rsize = superunit->rSize();
        int owner_faction = superunit->faction;
        if (tractor || repulsor) {
            bool fp = o_fp, fi = o_fi;
            if (target->faction == owner_faction)
                fp = f_fp, fi = f_fi;

            else if (target->faction == upgradesfaction)
                fp = u_fp, fi = u_fi;

            else if (target->faction == cargofaction)
                fp = c_fp, fi = c_fi;

            else if (target->getAIState() == NULL)
                fp = d_fp, fi = d_fi;
            //tractor/repulsor beam!
            if ( fp || target->isTractorable( Unit::tractorPush ) ) {
                //Compute relative speed - if it's higher than the maximum, don't accelerate it anymore
                //FIXME: Should predict the resulting velocity after applying the force,
                //and adjust the force to match the maximum relative velocity - but the
                //heterogeneous physics granularity makes it quite hard (it's not owr
                //own priority the one counting, but the target's).
                //The current hack - using the target's sim_atom_multiplier, only prevents
                //aberrations from becoming obvious, but it's not entirely correct.
                float relspeed = target->GetVelocity()*direction.Cast();
                if (relspeed < maxrelspeed) {
                    //Modulate force on little mass objects, so they don't slingshot right past you
                    target->ApplyForce( direction
                                       *( appldam
                                         /sqrt( /*(target->sim_atom_multiplier
                                                 > 0) ? target->sim_atom_multiplier : */ 1.0 )*mymin( 1, target->getMass() ) ) );
                }
            }
            float ors_m = o_ors_m, trs_m = o_trs_m, ofs = o_o;
            if (target->faction == owner_faction)
                ors_m = f_ors_m, trs_m = f_trs_m, ofs = f_o;

            else if (target->faction == upgradesfaction)
                ors_m = u_ors_m, trs_m = u_trs_m, ofs = u_o;

            else if (target->faction == cargofaction)
                ors_m = c_ors_m, trs_m = c_trs_m, ofs = c_o;
            if ( ( fi
                  || target->isTractorable( Unit::tractorIn ) )
                && ( ( center-target->Position() ).Magnitude() < (ors_m*owner_rsize+trs_m*target->rSize()+ofs) ) ) {
                Unit *un = superunit;
                if ( target->faction == upgradesfaction || owner_rsize*nbig > target->rSize() ) {
                    //we have our man!
                    //lets add our cargo to him
                    Cargo *c = GetMasterPartList( target->name.get().c_str() );
                    Cargo tmp;
                    bool isnotcargo = (c == NULL);
                    if (!isnotcargo)
                        if (c->GetCategory().find( "upgrades" ) == 0)
                            isnotcargo = true;
                    //add upgrades as space junk
                    if (isnotcargo) {
                        c = &tmp;
                        tmp.content  = "Space_Salvage";
                        tmp.category = "Uncategorized_Cargo";
                        static float spacejunk = parse_float( vs_config->getVariable( "cargo", "space_junk_price", "10" ) );
                        tmp.price    = spacejunk;
                        tmp.quantity = 1;
                        tmp.mass     = .001;
                        tmp.volume   = 1;
                        if (target->faction != upgradesfaction) {
                            tmp.content  = target->name;
                            tmp.category = "starships";
                            static float starshipprice  =
                                XMLSupport::parse_float( vs_config->getVariable( "cargo", "junk_starship_price", "100000" ) );
                            static float starshipmass   =
                                XMLSupport::parse_float( vs_config->getVariable( "cargo", "junk_starship_mass", "50" ) );
                            static float starshipvolume =
                                XMLSupport::parse_float( vs_config->getVariable( "cargo", "junk_starship_volume", "1500" ) );
                            tmp.price    = starshipprice;
                            tmp.quantity = 1;
                            tmp.mass     = starshipmass;
                            tmp.volume   = starshipvolume;
                        }
                    }
                    if (c != NULL) {
                        Cargo adder = *c;
                        adder.quantity = 1;
                        if ( un->CanAddCargo( adder ) ) {
                            un->AddCargo( adder );
                            if ( _Universe->isPlayerStarship( un ) ) {
                                static int tractor_onboard =
                                    AUDCreateSoundWAV( vs_config->getVariable( "unitaudio", "player_tractor_cargo",
                                                                               "tractor_onboard.wav" ) );
                                AUDPlay( tractor_onboard, QVector( 0, 0, 0 ), Vector( 0, 0, 0 ), 1 );
                            } else {
                                Unit *tmp = _Universe->AccessCockpit()->GetParent();
                                if (tmp && tmp->owner == un) {
                                    //Subunit of player (a turret)
                                    static int tractor_onboard_fromturret =
                                        AUDCreateSoundWAV( vs_config->getVariable( "unitaudio",
                                                                                   "player_tractor_cargo_fromturret",
                                                                                   "tractor_onboard.wav" ) );
                                    AUDPlay( tractor_onboard_fromturret, QVector( 0, 0, 0 ), Vector( 0, 0, 0 ), 1 );
                                }
                            }
                            target->Kill();
                        }
                    }
                }
            }
        } else {
            target->ApplyDamage( center.Cast()+direction*curlength, normal, appldam, colidee, coltmp, owner, phasdam );
        }
        return true;
    }
    return false;
}

