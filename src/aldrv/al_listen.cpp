#include "audiolib.h"
#include <AL/al.h>
#include <stdio.h>
void AUDListener (const Vector & pos, const Vector & vel) {
  alListener3f (AL_POSITION, pos.i,pos.j,pos.k);
  alListener3f (AL_VELOCITY, vel.i,pos.j,pos.k);
  //  printf ("(%f,%f,%f) <%f %f %f>\n",pos.i,pos.j,pos.k,vel.i,vel.j,vel.k);
}
void AUDListenerOrientation (const Vector & p, const Vector & q, const Vector & r) {
  
  ALfloat orient [] = {r.i,r.j,r.k,q.i,q.j,q.k};
  //  printf ("R%f,%f,%f>Q<%f %f %f>",r.i,r.j,r.k,q.i,q.j,q.k);
  alListenerfv (AL_ORIENTATION,orient);
}
void AUDListenerGain (const float gain) {
  alListenerf (AL_GAIN,gain);
}
