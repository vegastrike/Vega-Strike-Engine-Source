#include <vector>
#include <string>
#include "vec.h"
#include "matrix.h"
#include "cmd/unit_generic.h"
#include "halo_system.h"
#include "universe.h"
#include <stdlib.h>
#include <stdio.h>
#include "vegastrike.h"
#include "mesh.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "gfx/particle.h"
#include "lin_time.h"
#include "animation.h"
#include "car_assist.h"
#include "cmd/collide2/CSopcodecollider.h"

#include "cmd/unit_collide.h"

#define HALO_SMOOTHING_UP_FACTOR (0.02)
#define HALO_SMOOTHING_DOWN_FACTOR (0.01)
#define HALO_STEERING_UP_FACTOR (0.00)
#define HALO_STEERING_DOWN_FACTOR (0.01)
#define HALO_STABILIZATION_RANGE (0.25)

static float mymin( float a, float b )
{
    return a > b ? b : a;
}

static float mymax( float a, float b )
{
    return a > b ? a : b;
}

void DoParticles( QVector pos, float percent, const Vector &velocity, float radial_size, float particle_size, int faction )
{
    static float scale      = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparklescale", "8" ) );
    static float sspeed     = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparklespeed", ".5" ) );
    static float flare      = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparkleflare", ".15" ) );
    static float spread     = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparklespread", ".04" ) );
    static float absspeed   = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparkleabsolutespeed", ".02" ) );
    static float sciz       = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparklesizeenginerelative", ".125" ) );
    static bool  fixed_size = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "sparklefixedsize", "0" ) );

    percent = 1-percent;
    int i = rand();
    if (i < RAND_MAX*percent*scale) {
        ParticlePoint pp;
        float   r1 = rand()/( (float) RAND_MAX*.5 )-1;
        float   r2 = rand()/( (float) RAND_MAX*.5 )-1;
        float   r3 = rand()/( (float) RAND_MAX*.5 )-1;
        QVector rand( r1, r2, r3 );
        pp.loc   = pos+rand*radial_size*flare;
        const float *col = FactionUtil::GetSparkColor( faction );
        pp.col.r = col[0];
        pp.col.g = col[1];
        pp.col.b = col[2];
        pp.col.a = 1.0f;
        particleTrail.AddParticle( pp, rand*(mymax(
                                                 velocity.Magnitude(),
                                                 absspeed )*spread+absspeed)+velocity*sspeed,
                                  fixed_size ? sciz : (sqrt( particle_size )*sciz) );
    }
}

void LaunchOneParticle( const Matrix &mat, const Vector &vel, unsigned int seed, Unit *mush, float hull, int faction )
{
    static float sciz =
        XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparkleenginesizerelativetoship", "0.1875" ) );
    if (mush) {
        bool done = false;
        collideTrees *colTrees = mush->colTrees;
        if (colTrees) {
            if ( colTrees->usingColTree() ) {
                csOPCODECollider *colTree = colTrees->rapidColliders[0];
                unsigned int numvert = colTree->getNumVertex();
                if (numvert) {
                    unsigned int whichvert = seed%numvert;
                    QVector v( colTree->getVertex( whichvert ).Cast() );
                    v    = Transform( mat, v );
                    DoParticles( v, hull, vel, 0, mush->rSize()*sciz, faction );
                    done = true;
                }
            }
        }
        if (!done) {
            // maybe ray collision?
        }
        if (!done) {
            unsigned int siz = (unsigned int) ( 2*mush->rSize() );
            if (siz != 0) {
                QVector v( (seed%siz)-siz/2,
                          (seed%siz)-siz/2,
                          (seed%siz)-siz/2 );
                DoParticles( v, hull, vel, 0, mush->rSize()*sciz, faction );
                done = true;
            }
        }
    }
}

HaloSystem::HaloSystem() : sparkle_accum(0)
{
    VSCONSTRUCT2( 'h' )
}

unsigned int HaloSystem::AddHalo( const char *filename,
                                  const Matrix &trans,
                                  const Vector &size,
                                  const GFXColor &col,
                                  std::string type,
                                  float activation_accel )
{
    static float engine_scale  = XMLSupport::parse_float( vs_config->getVariable( "graphics", "engine_radii_scale", ".4" ) );
    static float engine_length = XMLSupport::parse_float( vs_config->getVariable( "graphics", "engine_length_scale", "1.25" ) );
    static float gs = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );

    int neutralfac = FactionUtil::GetNeutralFaction();
    halo.push_back( Halo() );
    halo.back().trans = trans;
    halo.back().size = Vector(size.i*engine_scale, size.j*engine_scale, size.k*engine_length);
    halo.back().mesh = Mesh::LoadMesh( ( string( filename ) ).c_str(), Vector( 1, 1, 1 ), neutralfac, NULL );
    halo.back().activation = activation_accel * gs;
    halo.back().oscale = 0;
    return halo.size()-1;
}

static float HaloAccelSmooth( float linaccel, float olinaccel, float maxlinaccel )
{
    linaccel = mymax( 0, mymin( maxlinaccel, linaccel ) );     //Clamp input, somehow, sometimes it's not clamped
    float phase = pow( ( (linaccel > olinaccel) ? HALO_SMOOTHING_UP_FACTOR : HALO_SMOOTHING_DOWN_FACTOR ), GetElapsedTime() );
    float olinaccel2;
    if (linaccel > olinaccel)
        olinaccel2 = mymin( linaccel, olinaccel+maxlinaccel*HALO_STEERING_UP_FACTOR );
    else
        olinaccel2 = mymax( linaccel, olinaccel-maxlinaccel*HALO_STEERING_DOWN_FACTOR );
    linaccel = (1-phase)*linaccel+phase*olinaccel2;
    linaccel = mymax( 0, mymin( maxlinaccel, linaccel ) );
    return linaccel;
}

void HaloSystem::Draw( const Matrix &trans,
                       const Vector &scale,
                       int halo_alpha,
                       float nebdist,
                       float hullpercent,
                       const Vector &velocity,
                       const Vector &linaccel,
                       const Vector &angaccel,
                       float maxaccel,
                       float maxvelocity,
                       int faction )
{
    static bool halos_by_velocity = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "halos_by_velocity", "false" ) );
    static float percentColorChange = XMLSupport::parse_float(vs_config->getVariable("graphics","percent_afterburner_color_change",".5"));
    static float abRedness = XMLSupport::parse_float(vs_config->getVariable("graphics","afterburner_color_red","1.0"));
    static float abGreenness = XMLSupport::parse_float(vs_config->getVariable("graphics","afterburner_color_green","0.0"));
    static float abBlueness = XMLSupport::parse_float(vs_config->getVariable("graphics","afterburner_color_blue","0.0"));
    static float percentRedness = XMLSupport::parse_float(vs_config->getVariable("graphics","engine_color_red","1.0"));
    static float percentGreenness = XMLSupport::parse_float(vs_config->getVariable("graphics","engine_color_green","1.0"));
    static float percentBlueness = XMLSupport::parse_float(vs_config->getVariable("graphics","engine_color_blue","1.0"));
    static float sparklerate = XMLSupport::parse_float( vs_config->getVariable( "graphics", "halosparklerate", "20" ) );

    if ( halo_alpha >= 0 ) {
        halo_alpha /= 2;
        if ( (halo_alpha & 0x1) == 0 )
            halo_alpha += 1;
    }
    if ( maxaccel <= 0 ) maxaccel = 1;
    if ( maxvelocity <= 0 ) maxvelocity = 1;

    for ( std::vector< Halo >::iterator i = halo.begin(); i != halo.end(); ++i ) {
        Vector thrustvector = TransformNormal( trans, i->trans.getR() ).Normalize();
        float value, maxvalue, minvalue;
        if (halos_by_velocity) {
            value = velocity.Dot( thrustvector );
            maxvalue = sqrt( maxvelocity );
            minvalue = i->activation;
        } else {
            Vector relpos = TransformNormal( trans, i->trans.p );
            Vector accel = linaccel + relpos.Cross( angaccel );
            float accelmag = accel.Dot( thrustvector );
            i->oscale = HaloAccelSmooth( accelmag / maxaccel, i->oscale, 1.0f );
            value = i->oscale;
            maxvalue = 1.0f;
            minvalue = i->activation / maxaccel;
        }
        if ( (value > minvalue) && (scale.k > 0) ) {
            Matrix m = trans * i->trans;
            ScaleMatrix( m, Vector( scale.i*i->size.i, scale.j*i->size.j, scale.k*i->size.k*value/maxvalue ) );

            GFXColor blend = GFXColor( percentRedness, percentGreenness, percentBlueness, 1 );
            if (value > maxvalue*percentColorChange) {
                float test = value-maxvalue*percentColorChange;
                test /= maxvalue*percentColorChange;
                if (!(test<1.0)) test = 1.0;
                float r = abRedness*test+percentRedness*(1.0-test);
                float g = abGreenness*test+percentGreenness*(1.0-test);
                float b = abBlueness*test+percentBlueness*(1.0-test);
                blend = GFXColor( r, g, b, 1.0 );
            }

            MeshFX xtraFX=MeshFX(1.0,1.0,
                                 true,
                                 GFXColor(1,1,1,1),
                                 GFXColor(1,1,1,1),
                                 GFXColor(1,1,1,1),
                                 blend);
            i->mesh->Draw( 50000000000000.0, m, 1, halo_alpha, nebdist, 0, false, &xtraFX );

            if ( hullpercent < .99 ) {
                sparkle_accum += GetElapsedTime()*sparklerate;
                int spawn = (int) (sparkle_accum);
                sparkle_accum -= spawn;
                while (spawn-- > 0)
                    DoParticles( m.p, hullpercent, velocity, i->mesh->rSize()*scale.i, i->mesh->rSize()*scale.i, faction );
            }
        }
    }
}

HaloSystem::~HaloSystem()
{
    VSDESTRUCT2
    for ( std::vector< Halo >::iterator i = halo.begin(); i != halo.end(); ++i ) {
        if ( i->mesh )
            delete i->mesh;
	}
}
