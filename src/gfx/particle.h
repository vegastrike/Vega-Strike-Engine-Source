#ifndef __PARTICLE_H
#define __PARTICLE_H

#include <vector>
#include "vec.h"
#include "gfxlib_struct.h"

class ParticlePoint {
 public:
  QVector loc;
  GFXColor col;
  float size;
  //draw and change color...if too dark kill
  bool Draw (const Vector& delta, const double time, Vector p, Vector q);
};

class ParticleTrail {
  std::vector <Vector> particleVel;
  std::vector <ParticlePoint> particle; 
  unsigned int maxparticles;
 public:
  ParticleTrail (unsigned int max) {
    ChangeMax (max);
  }
  void DrawAndUpdate();
  void AddParticle (const ParticlePoint &,  const Vector &, float size);
  //void PopParticle();not efficient right now
  void ChangeMax (unsigned int max);
};
extern ParticleTrail particleTrail;

#endif
