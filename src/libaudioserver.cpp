#include <string>
#include "audiolib.h"
#include "gfx/cockpit_generic.h"

void AUDAdjustSound(int i, QVector const & qv, Vector const & vv) {}

bool AUDIsPlaying(int snd) { return false;}
void AUDSoundGain(int snd, float howmuch,bool){}
void	AUDRefreshSounds() {}
int		AUDCreateSoundWAV (const std::string &, const bool LOOP) {return -1;}
int		AUDCreateSoundMP3 (const std::string &, const bool LOOP) {return -1;}
int		AUDCreateSound (int sound,const bool LOOP) {return -1;}
int		AUDCreateSound (const std::string &,const bool LOOP) {return -1;}
void	AUDStartPlaying ( int i) {}
void	AUDStopPlaying( int i) {}
void	AUDDeleteSound( int i) {}
void	AUDDeleteSound( int i, bool b) {}
void	AUDPlay (const int sound, const QVector &pos, const Vector & vel, const float gain) {}
QVector AUDListenerLocation() {
  return QVector(0,0,0);
}

// From communication_xml.cpp
int createSound( std::string file, bool val) { return -1;}

//soundContainer::~soundContainer () {}

