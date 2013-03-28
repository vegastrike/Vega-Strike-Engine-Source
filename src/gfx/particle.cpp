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
#include <limits>

ParticleTrail particleTrail( "sparkle", 500, SRCALPHA, ONE, 0.05, false, true );
ParticleTrail smokeTrail( "smoke", 500, SRCALPHA, INVSRCALPHA );
ParticleTrail debrisTrail( "debris", 500, SRCALPHA, INVSRCALPHA, 0.5, true );


static float mymin( float a, float b )
{
    return a > b ? b : a;
}

static float mymax( float a, float b )
{
    return a > b ? a : b;
}

void ParticleTrail::ChangeMax( unsigned int max )
{
    unsigned int vertices_per_particle;
    unsigned int indices_per_particle;
    
    if (config.use_points)
        vertices_per_particle = indices_per_particle = 1;
    else
        vertices_per_particle = indices_per_particle = 12;
    
    if (gl_options.max_array_vertices > 0 && 
            (max * vertices_per_particle) > gl_options.max_array_vertices)
    {
        max = gl_options.max_array_vertices / vertices_per_particle;
        if (max > 0)
            max--; // for a margin
    }
    if (gl_options.max_array_indices > 0 && 
            (max * indices_per_particle) > gl_options.max_array_indices)
    {
        max = gl_options.max_array_indices / indices_per_particle;
        if (max > 0)
            max--; // for a margin
    }
    if (max < 4)
        max = 4;
    
    this->maxparticles = max;
}

static inline void UpdateColor( ParticlePoint & p, const Vector &vel, const float time, const float fade )
{
    float fadetime = fade * time;
    p.loc += vel.Cast() * time;
    p.col  = ( p.col - GFXColor( fadetime, fadetime, fadetime, fadetime ) ).clamp();
}

static inline void UpdateAlpha( ParticlePoint & p, const Vector &vel, const float time, const float fade )
{
    p.loc += vel.Cast() * time;
    p.col.a = mymax(0.0f, p.col.a - fade * time);
}

//Write 3 pos and 4 col float values into v and increment v by 7
static inline void SetPointVertex( const ParticlePoint & p, const float grow, const float trans, std::back_insert_iterator<std::vector<float> > &v, const QVector &campos )
{
    float size    = p.size * (grow * (1.0f - p.col.a) + p.col.a);
    float maxsize = (p.size > size) ? p.size : size;
    float minsize = (p.size <= size) ? p.size : size;

    //Squared, surface-linked decay - looks nicer, more real for emmisive gasses
    //NOTE: maxsize/minsize allows for inverted growth (shrinkage) while still fading correctly. Cheers!
    GFXColor c = p.col * ( p.col.a * trans * ( minsize / ( (maxsize > 0) ? maxsize : 1.f ) ) );
    QVector  l = p.loc - campos;

    *v++ = l.x; *v++ = l.y; *v++ = l.y;  *v++ = c.r; *v++ = c.g; *v++ = c.b; *v++ = c.a; 
}

//Write 12 * 3 pos and 12 * 4 col and 12 * 2 tex float values into v and increment v by 108
static inline void SetQuadVertex( const ParticlePoint & p, const float grow, const float trans, std::back_insert_iterator<std::vector<float> > &v, const QVector &campos )
{
    float size    = p.size * (grow * (1 - p.col.a) + p.col.a);
    float maxsize = (p.size > size) ? p.size : size;
    float minsize = (p.size <= size) ? p.size : size;

    //Squared, surface-linked decay - looks nicer, more real for emmisive gasses
    //NOTE: maxsize/minsize allows for inverted growth (shrinkage) while still fading correctly. Cheers!
    GFXColor c = p.col * ( p.col.a * trans * ( minsize / ( (maxsize > 0) ? maxsize : 1.f ) ) );
    QVector  l = p.loc - campos;

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

template <typename R, typename T> class IndexCompare {
    const std::vector<R> &ref;
    
public:
    IndexCompare(const std::vector<R> &ref_) : ref(ref_) 
    {
    }
    
    bool operator()(const T &a, const T &b) const
    {
        return ref[a] > ref[b];
    }
};

ParticleTrail::Config::Config(const std::string &prefix)
{
    texture = NULL;
    initialized = false;
    this->prefix = prefix;
}

ParticleTrail::Config::~Config()
{
    if (texture != NULL)
        delete texture;
}

void ParticleTrail::Config::init()
{
    if (initialized)
        return;
    
    use         = XMLSupport::parse_bool( vs_config->getVariable( "graphics", prefix, "true" ) );
    use_points  = XMLSupport::parse_bool( vs_config->getVariable( "graphics", prefix + "point", "false" ) );
    pblend      = XMLSupport::parse_bool( vs_config->getVariable( "graphics", prefix + "blend", "false" ) );
    pgrow       = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "growrate", "50.0" ) );     //50x size when disappearing
    ptrans      = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "alpha", "2.5" ) );     //NOTE: It's the base transparency, before surface attenuation, so it needn't be within the [0-1] range.
    pfade       = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "fade", "0.1" ) );
    
    if (use_points) {
        psize       = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "size", "1.5" ) );
        psmooth     = XMLSupport::parse_bool( vs_config->getVariable( "graphics", prefix + "smooth", "false" ) );
    } else {
        std::string s = vs_config->getVariable( "graphics", prefix + "texture", "supernova.bmp" );
        texture     = new Texture( s.c_str() );
    }
    
    initialized = true;
}

void ParticleEmitter::Config::init(const std::string &prefix)
{
    rate      = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "scale", "8" ) );
    speed     = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "speed", ".5" ) );
    locSpread = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "flare", ".15" ) );
    spread    = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "spread", ".04" ) );
    absSpeed  = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "absolutespeed", ".02" ) );
    relSize   = XMLSupport::parse_float( vs_config->getVariable( "graphics", prefix + "sizerelative", ".125" ) );
    fixedSize = XMLSupport::parse_bool ( vs_config->getVariable( "graphics", prefix + "fixedsize", "0" ) );
}

void ParticleTrail::DrawAndUpdate()
{
    // Shortcircuit, not only an optimization, it avoids assertion failures in GFXDraw
    if (!config.initialized) {
        config.init();
        ChangeMax( maxparticles );
        VSFileSystem::vs_dprintf(1, "Configured particle system %s with %d particles\n", 
            config.prefix.c_str(), maxparticles);
    }
    if (!config.use || particle.empty())
        return;

    bool  use_points = config.use_points;
    bool  pblend = config.pblend;
    float pgrow = config.pgrow;
    float ptrans = config.ptrans;
    float pfade = config.pfade;
    
    const QVector campos = _Universe->AccessCamera()->GetPosition();
    
    // Draw particles
    GFXDisable( CULLFACE );
    GFXDisable( LIGHTING );
    GFXLoadIdentity( MODEL );
    GFXTranslateModel( campos );
    if (use_points) {
        float psize = config.psize;
        bool psmooth = config.psmooth;

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
            SetPointVertex( particle[i], pgrow, ptrans, v, campos );
        }
        GFXDraw( GFXPOINT, &particleVert[0], particle.size(), 3, 4 );

        glDisable( GL_POINT_SMOOTH );
        GFXPointSize( 1 );
    } else {
        Texture *t = config.texture;
        const int vertsPerParticle = 12;
        bool dosort = blenddst != ONE && (blenddst != ZERO || !writeDepth);

        GFXEnable( TEXTURE0 );
        GFXDisable( TEXTURE1 );
        GFXBlendMode( blendsrc, blenddst );
        if (writeDepth) {
            GFXEnable( DEPTHWRITE );
        } else {
            GFXDisable( DEPTHWRITE );
        }
        if (alphaMask > 0) {
            GFXAlphaTest(GEQUAL, alphaMask);
        }
        t->MakeActive();
    
        if (dosort) {
            // Must sort
            distances.clear();
            distances.reserve(particle.size());
            { for (std::vector<ParticlePoint>::const_iterator it = particle.begin(); it != particle.end(); ++it) {
                distances.push_back((campos - it->loc).MagnitudeSquared());
            } }
            IndexCompare<float, unsigned short> dcomp(distances);
            
            unsigned short nindices;
            if ( particle.size() >= (size_t)std::numeric_limits< unsigned short >::max() / vertsPerParticle )
                nindices = (unsigned short)std::numeric_limits< unsigned short >::max() / vertsPerParticle;
            else
                nindices = (unsigned short)particle.size();
            pointIndices.clear();
            pointIndices.reserve(nindices);
            { for (unsigned short i=0; i < nindices; ++i) {
                pointIndices.push_back(i);
            } }
            
            std::sort(pointIndices.begin(), pointIndices.end(), dcomp);
            
            indices.clear();
            indices.reserve(nindices *  vertsPerParticle);
            { for (std::vector<unsigned short>::const_iterator it = pointIndices.begin(); it != pointIndices.end(); ++it) {
                for (int i = 0; i < vertsPerParticle; ++i)
                    indices.push_back(*it * vertsPerParticle + i);
            } }
        }
        
        particleVert.clear();
        particleVert.reserve(particle.size() * vertsPerParticle * (3 + 4 + 2));
        std::back_insert_iterator<std::vector<float> > v(particleVert);
        for (size_t i = 0; i < particle.size(); ++i) {
            SetQuadVertex( particle[i], pgrow, ptrans, v, campos );
        }
        
        if (dosort) {
            VSFileSystem::vs_dprintf(3, "Drawing %d/%d sorted particles\n", particle.size(), maxparticles);
            GFXDrawElements( GFXQUAD, 
                &particleVert[0], particle.size() * vertsPerParticle, 
                &indices[0], indices.size(),
                3, 4, 2 );
        } else {
            VSFileSystem::vs_dprintf(3, "Drawing %d/%d unsorted particles\n", particle.size(), maxparticles);
            GFXDraw( GFXQUAD, &particleVert[0], particle.size() * 12, 3, 4, 2 );
        }
        
        if (alphaMask > 0) {
            GFXAlphaTest(ALWAYS, 0);
        }
        
        GFXBlendMode( ONE, ZERO );
    }
    GFXLoadIdentity( MODEL );
    
    // Update particles
    float mytime = GetElapsedTime();
    if (fadeColor) {
        for (size_t i = 0, n = particle.size(); i < n; ++i) {
            UpdateColor(particle[i], particleVel[i], mytime, pfade);
        }
    } else {
        for (size_t i = 0, n = particle.size(); i < n; ++i) {
            UpdateAlpha(particle[i], particleVel[i], mytime, pfade);
        }
    }

    // Quickly remove dead particles at the end
    while ( !particle.empty() && !(particle.back().col.a > alphaMask) ) {
        particleVel.pop_back();
        particle.pop_back();
    }
    
    // Remove dead particles anywhere
    vector< Vector >::iterator v = particleVel.begin();
    vector< ParticlePoint >::iterator p = particle.begin();
    while ( p != particle.end() ) {
        if ( !(p->col.a > alphaMask) ) {
            vector< Vector >::iterator vlast = particleVel.end() - 1;
            vector< ParticlePoint >::iterator plast = particle.end() - 1;
            if (p != plast) {
                *v = *vlast;
                *p = *plast;
            }
            size_t indexp  = p - particle.begin();
            size_t indexv = v - particleVel.begin();
            particle.pop_back();
            particleVel.pop_back();
            p = particle.begin() + indexp;
            v = particleVel.begin() + indexv;             //continue where we left off
        } else {
            ++p;
            ++v;
        }
    }
}

void ParticleTrail::AddParticle( const ParticlePoint &P, const Vector &V, float size )
{
    config.init();
    if (!config.use)
        return;
    
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

void ParticleEmitter::doParticles(const QVector& pos, float rSize, float percent, const Vector& basevelocity, const Vector& velocity, float pSize, const GFXColor& color)
{
    percent = 1-percent;
    int i = rand();
    if (i < RAND_MAX*percent*config.rate) {
        ParticlePoint pp;
        float   r1 = rand()/( (float) RAND_MAX*.5 )-1;
        float   r2 = rand()/( (float) RAND_MAX*.5 )-1;
        float   r3 = rand()/( (float) RAND_MAX*.5 )-1;
        float   r4 = rand()/( (float) RAND_MAX*.5 )-1;
        Vector rand( r1, r2, r3 );
        rand.Normalize();
        pp.loc   = pos+rand*rSize*config.locSpread;
        
        // Make randomization direction-centric
        Vector direction = velocity;
        direction.Normalize();
        rand *= config.spread;
        rand += direction;
        rand.Normalize();
        rand *= 1.0 + config.spread * r4;
        
        pp.col = color;
        particleTrail.AddParticle( pp, 
            rand*( mymax( velocity.Magnitude(), config.absSpeed )*config.spread+config.absSpeed)
                +velocity*config.speed+basevelocity,
            config.fixedSize ? config.relSize : (pSize*config.relSize) 
        );
    }
}
