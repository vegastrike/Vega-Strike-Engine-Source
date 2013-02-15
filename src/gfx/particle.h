#ifndef __PARTICLE_H
#define __PARTICLE_H

#include <vector>
#include "vec.h"
#include "gfxlib_struct.h"

class Texture;

struct ParticlePoint
{
    QVector  loc;
    GFXColor col;
    float    size;
};

class ParticleTrail
{
    std::vector< Vector > particleVel;
    std::vector< ParticlePoint > particle;
    std::vector< float > particleVert;
    std::vector< float > distances;
    std::vector< unsigned short > pointIndices;
    std::vector< unsigned short > indices;
    unsigned int maxparticles;
    BLENDFUNC blendsrc, blenddst;
    float alphaMask;
    bool writeDepth;
    
    struct Config {
        bool  use;
        bool  use_points;
        bool  pblend;
        bool  psmooth;
        float pgrow;
        float ptrans;
        float pfade;
        float psize;
        Texture *texture;
        
        Config();
        ~Config();
        
        void init(const std::string &prefix);
    } config;
    
public: 
    ParticleTrail( std::string configPrefix, unsigned int max, BLENDFUNC blendsrc=ONE, BLENDFUNC blenddst=ONE, float alphaMask = 0, bool writeDepth = false )
    {
        config.init(configPrefix);
        ChangeMax( max );
        this->blendsrc = blendsrc;
        this->blenddst = blenddst;
        this->alphaMask = alphaMask;
        this->writeDepth = writeDepth;
    }
    void DrawAndUpdate();
    void AddParticle( const ParticlePoint&, const Vector&, float size );
    void ChangeMax( unsigned int max );
};

class ParticleEmitter 
{
    ParticleTrail *particles;
    
public:
    struct Config {
        bool  fixedSize;
        
        float rate;
        float speed;
        float locSpread;
        float spread;
        float absSpeed;
        float relSize;
        
        void init(const std::string &prefix);
    } config;
    
    explicit ParticleEmitter(ParticleTrail *particleType) : particles(particleType) {}
    explicit ParticleEmitter(ParticleTrail *particleType, const std::string &prefix) : particles(particleType) 
    {
        config.init(prefix);
    }
    
    /**
     * Launches (maybe) a particle, according to:
     * 
     * @param pos Emitter center
     * @param rSize Emitter radial size
     * @param percent Emitter rate relative to configured rate (inverse rate, effective_rate = rate/percent) 
     * @param basevelocity Emitter velocity directly translated to particle velocity
     * @param velocity Particle velocity relative to emitter velocity
     * @param pSize Particle size - ignored if emitter configured with fixed particle size
     * @param color Particle color
     * 
     */
    void doParticles( const QVector &pos, float rSize, float percent, const Vector &basevelocity, const Vector &velocity, float pSize, const GFXColor &color );
};

extern ParticleTrail particleTrail;
extern ParticleTrail smokeTrail;
extern ParticleTrail debrisTrail;

#endif

