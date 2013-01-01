#ifndef __PARTICLE_H
#define __PARTICLE_H

#include <vector>
#include "vec.h"
#include "gfxlib_struct.h"

struct ParticlePoint
{
    QVector  loc;
    GFXColor col;
    float    size;
};

class ParticleTrail
{
    std::vector< Vector >particleVel;
    std::vector< ParticlePoint >particle;
    std::vector< float >particleVert;
    unsigned int maxparticles;
public: ParticleTrail( unsigned int max )
    {
        ChangeMax( max );
    }
    void DrawAndUpdate();
    void AddParticle( const ParticlePoint&, const Vector&, float size );
    void ChangeMax( unsigned int max );
};
extern ParticleTrail particleTrail;

#endif

