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
#include "vs_globals.h"
#include "config_xml.h"
Music::Music (Unit *parent):random(false), p(parent),song(-1) {
  if (parent) {
    maxhull = parent->GetHull();
  }else {
    maxhull=1;
  }
  LoadMusic(0,vs_config->getVariable ("audio","battleplaylist","battle.m3u").c_str());
  LoadMusic(1,vs_config->getVariable ("audio","peaceplaylist","peace.m3u").c_str());
  LoadMusic(2,vs_config->getVariable ("audio","panicplaylist","panic.m3u").c_str());
}

void Music::LoadMusic (int which, const char *file) {
  FILE *fp = fopen (file,"r");
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
      this->playlist[which].push_back (std::string(songname));
    }
    fclose (fp);
  }
}
int Music::SelectTracks(int &whichlist) {
  Unit * un;
  if ((un =p.GetUnit())==NULL) {
    whichlist=1;
    if (playlist[1].empty())
      return 0;
    return rand()%playlist[1].size();
  }
  bool perfect=true;
  float goodness=0;
  UnitCollection * drawlist = _Universe->activeStarSystem()->getUnitList();
  un_iter iter = drawlist->createIterator();
  Unit *target;
  while ((target = iter.current())!=NULL) {
    float ftmp;
    ftmp = 2*_Universe->GetRelation (un->faction,target->faction);
    if (ftmp<0)
      perfect=false;
    if (FINITE (ftmp))
      goodness += ftmp;
    iter.advance();
  }
  if (perfect||playlist[0].empty()) {
    whichlist=1;
    if (playlist[1].empty())
      return 0;
    return rand()%playlist[1].size();
  }
  float ftmp =(un->FShieldData()+2*un->GetHull()/maxhull+un->RShieldData()-2.7)*10;
  if (ftmp<-.5) {
    whichlist=2;
    if (playlist[2].empty())
      return 0;
    return (rand()%playlist[2].size());    
  }
  whichlist=0;
  if (FINITE (ftmp)) 
    goodness += ftmp;


  goodness -= playlist[0].size()/8;
  goodness += playlist[0].size()/2;
  //  fprintf (stderr,"Choosing Song %f",goodness);
  int tmp=0;
  if (!playlist[0].empty()) {
    tmp=(rand()%((playlist[0].size()+3)/4));
  }
  goodness+=tmp;
  //fprintf (stderr,"Choosing Song With Randomness %f\n",goodness);
  if (goodness<0)
    goodness=0;
  if (goodness>=playlist[0].size()) {
    goodness=playlist[0].size()-1;
  }
  return (int)goodness;
}


void Music::Listen() {
  if (g_game.music_enabled ) {
	  if ((!AUDIsPlaying (song))) {
    AUDDeleteSound (song,true);//delete buffer too;
    int whichlist;
    int songnum = SelectTracks(whichlist);
    if (!playlist[whichlist].empty ()) {
      song = AUDCreateMusic (playlist[whichlist][songnum],false);
      AUDStartPlaying (song);
	  AUDAdjustSound (song,_Universe->AccessCamera()->GetPosition(),_Universe->AccessCamera()->GetVelocity());

    }    
	  } else {
	    AUDAdjustSound (song,_Universe->AccessCamera()->GetPosition(),_Universe->AccessCamera()->GetVelocity());
	  }
  }
}

void Music::Skip() {
  AUDStopPlaying (song);
}
Music::~Music() {
  AUDDeleteSound (song,true);//delete buffer too;
}

