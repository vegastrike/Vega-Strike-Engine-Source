#include "music.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include <stdio.h>
#include <stdlib.h>
#include "audiolib.h"
#include "universe.h"
#include "star_system.h"
#include "collection.h"
#include "unit.h"
Music::Music (const char *playlist,Unit *parent):p(parent),song(-1) {
  FILE *fp = fopen (playlist,"r");
  char songname[1024];
  if (fp) {
    while (!feof (fp)) {
      songname[0]='\0';
      fgets (songname,1022,fp);
      int size = strlen(songname);
      if (size>=1) 
	if (songname[size-1]=='\n') {
	  songname[size-1]='\0';
	}
      if (size>1)
	if (songname[size-2]=='\r'||songname[size-2]=='\n') {
	  songname[size-2]='\0';
	}
      if (songname[0]=='\0'||songname[0]=='#')
	continue;
      this->playlist.push_back (std::string(songname));
    }
    fclose (fp);
  }
}
int Music::SelectTracks() {
  Unit * un;
  if ((un =p.GetUnit())==NULL) {
    return rand()%playlist.size();
  }
  float goodness=0;
  UnitCollection * drawlist = _Universe->activeStarSystem()->getUnitList();
  Iterator * iter = drawlist->createIterator();
  Unit *target;
  while ((target = iter->current())!=NULL) {
    goodness += 2*_Universe->GetRelation (un->faction,target->faction);
    iter->advance();
  }
  delete iter;

  goodness += (un->FShieldData()+un->RShieldData()-1.7)*10;

  fprintf (stderr,"Choosing Song %f",goodness);
  goodness += -playlist.size()/8+(rand()%(playlist.size()/4));
  fprintf (stderr,"Choosing Song With Randomness %f",goodness);
  if (goodness<0)
    goodness=0;
  if (goodness>=playlist.size()) {
    goodness=playlist.size()-1;
  }
  return goodness;
}


void Music::Listen() {
  if (!AUDIsPlaying (song) ) {
    AUDDeleteSound (song,true);//delete buffer too;
    if (!playlist.empty()) {
      song = AUDCreateSoundMP3 (playlist[SelectTracks()],false);
      AUDStartPlaying (song);
    }
  }
}


Music::~Music() {
  AUDStopPlaying (song);
  AUDDeleteSound (song,true);//delete buffer too;
}

