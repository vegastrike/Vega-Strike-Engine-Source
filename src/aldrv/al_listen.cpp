#include "audiolib.h"
#include <AL/al.h>
#include <stdio.h>
void AUDListener (const Vector & pos, const Vector & vel) {
#ifdef HAVE_AL
  alListener3f (AL_POSITION, pos.i,pos.j,pos.k);
  alListener3f (AL_VELOCITY, vel.i,pos.j,pos.k);
  //  printf ("(%f,%f,%f) <%f %f %f>\n",pos.i,pos.j,pos.k,vel.i,vel.j,vel.k);
#endif
}

void AUDListenerOrientation (const Vector & p, const Vector & q, const Vector & r) {
#ifdef HAVE_AL  
  ALfloat orient [] = {r.i,r.j,r.k,q.i,q.j,q.k};
  //  printf ("R%f,%f,%f>Q<%f %f %f>",r.i,r.j,r.k,q.i,q.j,q.k);
  alListenerfv (AL_ORIENTATION,orient);
#endif
}
void AUDListenerGain (const float gain) {
#ifdef HAVE_AL
  alListenerf (AL_GAIN,gain);
#endif
}
