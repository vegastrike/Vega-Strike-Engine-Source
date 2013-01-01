#include "particle.h"
#include "gfxlib.h"
#include "lin_time.h"
#include "vs_globals.h"
#include "cmd/unit_generic.h"
#include "config_xml.h"
#include "camera.h"
#include "aux_texture.h"
#include "gldrv/gl_globals.h"

#include <iterator>

ParticleTrail particleTrail( 500 );

void ParticleTrail::ChangeMax( unsigned int max )
{
    this->maxparticles = max;
}

static inline void Update( ParticlePoint & p, const Vector &vel, const float time, const float fade )
{
    p.loc += (vel * time).Cast();
    p.col  = ( p.col - GFXColor( fade*time, fade*time, fade*time, fade*time ) ).clamp();
}

//Write 3 pos and 4 col float values into v and increment v by 7
static inline void SetPointVertex( const ParticlePoint & p, const float grow, const float trans, std::back_insert_iterator<std::vector<float> > &v )
{
    float size    = p.size * (grow * (1 - p.col.a) + p.col.a);
    float maxsize = (p.size > size) ? p.size : size;
    float minsize = (p.size <= size) ? p.size : size;

    //Squared, surface-linked decay - looks nicer, more real for emmisive gasses
    //NOTE: maxsize/minsize allows for inverted growth (shrinkage) while still fading correctly. Cheers!
    GFXColor c = p.col * ( p.col.a * trans * ( minsize / ( (maxsize > 0) ? maxsize : 1.f ) ) );
    QVector  l = p.loc - _Universe->AccessCamera()->GetPosition();

    *v++ = l.x; *v++ = l.y; *v++ = l.y;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a; 
}

//Write 12 * 3 pos and 12 * 4 col and 12 * 2 tex float values into v and increment v by 108
static inline void SetQuadVertex( const ParticlePoint & p, const float grow, const float trans, std::back_insert_iterator<std::vector<float> > &v )
{
    float size    = p.size * (grow * (1 - p.col.a) + p.col.a);
    float maxsize = (p.size > size) ? p.size : size;
    float minsize = (p.size <= size) ? p.size : size;

    //Squared, surface-linked decay - looks nicer, more real for emmisive gasses
    //NOTE: maxsize/minsize allows for inverted growth (shrinkage) while still fading correctly. Cheers!
    GFXColor c = p.col * ( p.col.a * trans * ( minsize / ( (maxsize > 0) ? maxsize : 1.f ) ) );
    QVector  l = p.loc - _Universe->AccessCamera()->GetPosition();

    *v++ = l.i+size; *v++ = l.j+size; *v++ = l.k;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 0; *v++ = 0;
    *v++ = l.i+size; *v++ = l.j-size; *v++ = l.k;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 0; *v++ = 1;
    *v++ = l.i-size; *v++ = l.j-size; *v++ = l.k;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 1; *v++ = 1;
    *v++ = l.i-size; *v++ = l.j+size; *v++ = l.k;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 1; *v++ = 0;

    *v++ = l.i; *v++ = l.j+size; *v++ = l.k+size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 0; *v++ = 0;
    *v++ = l.i; *v++ = l.j-size; *v++ = l.k+size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 0; *v++ = 1;
    *v++ = l.i; *v++ = l.j-size; *v++ = l.k-size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 1; *v++ = 1;
    *v++ = l.i; *v++ = l.j+size; *v++ = l.k-size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 1; *v++ = 0;

    *v++ = l.i+size; *v++ = l.j; *v++ = l.k+size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 0; *v++ = 0;
    *v++ = l.i+size; *v++ = l.j; *v++ = l.k-size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 0; *v++ = 1;
    *v++ = l.i-size; *v++ = l.j; *v++ = l.k-size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 1; *v++ = 1;
    *v++ = l.i-size; *v++ = l.j; *v++ = l.k+size;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a;  *v++ = 1; *v++ = 0;
}

void ParticleTrail::DrawAndUpdate()
{
    static bool use_points = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "point_sparkles", "false" ) );
    static bool  pblend = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "sparkeblend", "false" ) );
    static float pgrow = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparkegrowrate", "200.0" ) );     //200x size when disappearing
    static float ptrans = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparklealpha", "2.5" ) );     //NOTE: It's the base transparency, before surface attenuation, so it needn't be within the [0-1] range.
    static float pfade = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparklefade", "0.1" ) );
    
    // Shortcircuit, not only an optimization, it avoids assertion failures in GFXDraw
    if (particle.empty())
        return;

    // Draw particles
    GFXDisable( CULLFACE );
    GFXDisable( LIGHTING );
    GFXLoadIdentity( MODEL );
    GFXTranslateModel( _Universe->AccessCamera()->GetPosition() );
    if (use_points) {
        static float psize = XMLSupport::parse_float( vs_config->getVariable( "graphics", "sparkesize", "1.5" ) );
        static bool psmooth = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "sparkesmooth", "false" ) );

        GFXDisable( TEXTURE0 );
        GFXPointSize( psize );
        if (psmooth && gl_options.smooth_points)
            glEnable( GL_POINT_SMOOTH );
        if (pblend)
            GFXBlendMode( SRCALPHA, INVSRCALPHA );
        else
            GFXBlendMode( ONE, ZERO );

        particleVert.clear();
        particleVert.reserve(particle.size() * (3 + 4));
        std::back_insert_iterator<std::vector<float> > v(particleVert);
        for (size_t i = 0; i < particle.size(); ++i) {
            SetPointVertex( particle[i], pgrow, ptrans, v );
        }
        GFXDraw( GFXPOINT, &particleVert[0], particle.size(), 3, 4 );

        glDisable( GL_POINT_SMOOTH );
        GFXPointSize( 1 );
    } else {
        static string s = vs_config->getVariable( "graphics", "sparkletexture", "supernova.bmp" );
        static Texture *t = new Texture( s.c_str() );

        GFXEnable( TEXTURE0 );
        GFXDisable( TEXTURE1 );
        GFXDisable( DEPTHWRITE );
        GFXBlendMode( ONE, ONE );
        t->MakeActive();

        particleVert.clear();
        particleVert.reserve(particle.size() * 12 * (3 + 4 + 2));
        std::back_insert_iterator<std::vector<float> > v(particleVert);
        for (size_t i = 0; i < particle.size(); ++i) {
            SetQuadVertex( particle[i], pgrow, ptrans, v );
        }
        GFXDraw( GFXQUAD, &particleVert[0], particle.size() * 12, 3, 4, 2 );
    }
    GFXLoadIdentity( MODEL );

    // Update particles
    float mytime = GetElapsedTime();
    for (size_t i = 0; i < particle.size(); ++i) {
        Update(particle[i], particleVel[i], mytime, pfade);
    }

    // Sort particles
    vector< Vector >::iterator v = particleVel.begin();
    vector< ParticlePoint >::iterator p = particle.begin();
    while ( p != particle.end() ) {
        if ( !(p->col.a > 0) ) {
            vector< Vector >::iterator vlast = particleVel.end() - 1;
            vector< ParticlePoint >::iterator plast = particle.end() - 1;
            if (p != plast) {
                *v = *vlast;
                *p = *plast;
            }
            size_t indexp  = p - particle.begin();
            size_t indexv = v-particleVel.begin();
            particle.pop_back();
            particleVel.pop_back();
            p = particle.begin() + indexp;
            v = particleVel.begin()+indexv;             //continue where we left off
        } else {
            ++p;
            ++v;
        }
    }
}

void ParticleTrail::AddParticle( const ParticlePoint &P, const Vector &V, float size )
{
    if (particle.size() > maxparticles) {
        vector< Vector >::iterator vel = particleVel.begin();
        vector< ParticlePoint >::iterator p = particle.begin();
        size_t off = ( (size_t) rand() )%particle.size();
        vel += off;
        p   += off;
        *p   = P;
        (*p).size = size;
        *vel = V;
    } else {
        particle.push_back( P );
        particle.back().size = size;
        particleVel.push_back( V );
    }
}

