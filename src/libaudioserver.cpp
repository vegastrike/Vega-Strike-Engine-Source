#include <string>
#include "audiolib.h"
#include "gfx/cockpit.h"

void AUDAdjustSound(int i, QVector const & qv, Vector const & vv) {}

bool AUDIsPlaying(int snd) { return false;}
void AUDSoundGain(int snd, float howmuch){}
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

// From communication_xml.cpp
int createSound( std::string file, bool val) { return -1;}

soundContainer::~soundContainer () {}