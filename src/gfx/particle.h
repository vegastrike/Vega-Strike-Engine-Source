#include <list>
#include "vec.h"

class ParticlePoint {
 public:
  QVector loc;
  Vector col;
  //draw and change color...if too dark kill
  bool Draw (const Vector& delta, const double time);
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
  void AddParticle (const ParticlePoint &,  const Vector &);
  void PopParticle();
  void ChangeMax (unsigned int max);
};
extern ParticleTrail particleTrail;
