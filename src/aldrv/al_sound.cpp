#include "audiolib.h"

int AUDCreateSound (const char *, const bool LOOP=false){
#ifdef HAVE_AL
#endif
}
///copies other sound loaded through AUDCreateSound
int AUDCreateSound (int sound,const bool LOOP=false){
#ifdef HAVE_AL
#endif
}
int AUDDeleteSound (int sound){
#ifdef HAVE_AL
#endif
}
void AUDAdjustSound (const int sound, const Vector &pos, const Vector &vel){
#ifdef HAVE_AL
#endif
}
void AUDPlayOnce(int sound, const Vector &pos, const Vector &vel){
#ifdef HAVE_AL
#endif
}
void AUDPlayOnce (const char *sound, const Vector &pos, const Vector &vel){
#ifdef HAVE_AL
#endif
}
bool AUDIsPlaying (const int sound){
#ifdef HAVE_AL
#endif
}
void AUDStopPlaying (const int sound){
#ifdef HAVE_AL
#endif
}
void AUDStartPlaying (const int sound){
#ifdef HAVE_AL
#endif
}
void AUDPausePlaying (const int sound){
#ifdef HAVE_AL
#endif
}
