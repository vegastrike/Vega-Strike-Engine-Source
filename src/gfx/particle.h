#ifndef __PARTICLE_H
#define __PARTICLE_H

#include <list>
#include "vec.h"

class ParticlePoint {
 public:
  QVector loc;
  Vector col;
  float size;
  //draw and change color...if too dark kill
  bool Draw (const Vector& delta, const double time, Vector p, Vector q);
};

class ParticleTrail {
  std::list <Vector> particleVel;
  std::list <ParticlePoint> particle; 
  unsigned int maxparticles;
 public:
  ParticleTrail (unsigned int max) {
    ChangeMax (max);
  }
  void DrawAndUpdate();
  void AddParticle (const ParticlePoint &,  const Vector &, float size);
  void PopParticle();
  void ChangeMax (unsigned int max);
};
extern ParticleTrail particleTrail;

#endif
