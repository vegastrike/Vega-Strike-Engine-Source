#include "audiolib.h"
#include "hashtable.h"
#include <string>

int AUDCreateSoundWAV (const std::string &, const bool LOOP=false){
#ifdef HAVE_AL
#endif
}
int AUDCreateSoundMP3 (const std::string &, const bool LOOP=false){
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
