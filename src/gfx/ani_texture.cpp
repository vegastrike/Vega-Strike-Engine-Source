#include "cmd/unit_generic.h"
#include "ani_texture.h"

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "lin_time.h"
#include "vegastrike.h"
#include "vsfilesystem.h"
#include "vs_globals.h"

static vector <AnimatedTexture *> myvec;



void AnimatedTexture::MakeActive () {
  if (Decal) {
    Decal[active%numframes]->MakeActive();
  }
}
void AnimatedTexture::MakeActive (int stage) {
  if (Decal) {
    Decal[active%numframes]->MakeActive(stage);
  }
}
void AnimatedTexture::UpdateAllPhysics() {
  for (unsigned int i=0;i<myvec.size();i++) {
    myvec[i]->physicsactive-=SIMULATION_ATOM;
  }
}
void AnimatedTexture::UpdateAllFrame() {
  for (unsigned int i=0;i<myvec.size();i++) {
    myvec[i]->cumtime+=GetElapsedTime();
    if (myvec[i]->timeperframe) {
      myvec[i]->active = ((unsigned int)(myvec[i]->cumtime/myvec[i]->timeperframe))%myvec[i]->numframes;
    }
  }
}
bool AnimatedTexture::Done() {
  return physicsactive<0;
}
void AnimatedTexture::setTime (double tim) {
	cumtime=tim;
	active = ((unsigned int)(cumtime/timeperframe))%numframes;
}
using namespace VSFileSystem;

AnimatedTexture::AnimatedTexture (const char *file,int stage, enum FILTER imm,bool detailtex){
  AniInit();
  VSFile f;
  VSError err = f.OpenReadOnly( file, AnimFile);
  //bool setdir=false;
  if (err<=Ok) {
    float width,height;
    f.Fscanf("%f %f", &width, &height);//it's actually an animation in global animation shares
    //setdir=true;
    
  }
  if (err<=Ok){
    Load (f,stage,imm,detailtex);
    f.Close();
  }else {
    loadSuccess=false;
  }
/*
if (setdir) {
    VSFileSystem::vs_resetdir();
}
*/
}
void AnimatedTexture::AniInit() {
  numframes=1;
  timeperframe=1;
  Decal=NULL;
  physicsactive=0;
  name=-1;
  active=0;
  original = NULL;
}
//AnimatedTexture::AnimatedTexture (FILE * fp, int stage, enum FILTER imm, bool detailtex){
//  AniInit();
//  if (fp)
//    Load (fp,stage,imm,detailtex);
//}

AnimatedTexture::AnimatedTexture (VSFileSystem::VSFile &fp, int stage, enum FILTER imm, bool detailtex){
  AniInit();
  Load (fp,stage,imm,detailtex);
}

Texture *AnimatedTexture::Original(){
  return Decal?Decal[active]->Original():this;
}

Texture *AnimatedTexture::Clone () {
  if (Decal) {
    AnimatedTexture * retval = new AnimatedTexture ();
    *retval = *this;
    retval->Decal = new Texture * [numframes];
    for (int i=0;i<numframes;i++) {
      retval->Decal[i]= Decal[i]->Clone ();
    }
    myvec.push_back (retval);
    return retval;
  }else {
    return new AnimatedTexture();
  }
}

AnimatedTexture::~AnimatedTexture () {
  Destroy();
  data= NULL;
  active=0;
  palette=NULL;
}
AnimatedTexture::AnimatedTexture () {
  AniInit();
}

void AnimatedTexture::Destroy() {
  int i;
  if (Decal) {
    for (i=0;i<(int)myvec.size();i++) {
      if (myvec[i]==this) {
	myvec.erase (myvec.begin()+i);
      }
    }
    for (i=0;i<numframes;i++) {
      delete Decal[i];
    }
    delete []Decal;
    Decal=NULL;
  }
}
void AnimatedTexture::Reset () {
  cumtime=0;
  active=0;
  physicsactive = numframes*timeperframe;
}
/*
void AnimatedTexture::Load( char * buffer, int length, int nframe, enum FILTER ismipmapped,bool detailtex)
{
	myvec.push_back (this);
	numframes = nframe;
	timeperframe = 100;
	cumtime=0;
	int i=0;
	Reset();

	active=0;
	Decal = new Texture * [numframes];
  bool loadall=true;
  if (g_game.use_animations==0||(g_game.use_animations!=0&&g_game.use_textures==0)) {
    loadall=false;
  }
  for (;i<numframes;i++) {
	Decal[i]=new Texture (buffer,length,stage,ismipmapped,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_animations)?GFXTRUE:GFXFALSE,65536,detailtex?GFXTRUE:GFXFALSE);
  }
  if (!loadall) {
    Texture * dec = Decal[numframes/2];
    timeperframe*=numframes;
    numframes=1;
    if (Decal) {
      delete [] Decal;
    }
    Decal = new Texture * [1];
    Decal[0]=dec;
  }
  original = NULL;
}
*/

void AnimatedTexture::Load(VSFileSystem::VSFile & f, int stage, enum FILTER ismipmapped, bool detailtex) {
  myvec.push_back (this);
  f.Fscanf ("%d %f",&numframes,&timeperframe);
  cumtime=0;
  Reset();

  active=0;
  Decal = new Texture * [numframes];
  char temp[512]="white.bmp";
  char file[512]="white.bmp";
  char alp[512]="white.bmp";
  int i=0;
  bool loadall=true;
  if (g_game.use_animations==0||(g_game.use_animations!=0&&g_game.use_textures==0)) {
    
    loadall=false;
  }


  for (;i<numframes;i++) {
    int numgets=0;
    while (numgets<=0&&!f.Eof()) {
		if (f.ReadLine(temp,511)==Ok) {
			temp[511]='\0';
			file[0]='z';file[1]='\0';
			alp[0]='z';alp[1]='\0';//windo	ws crashes on null
  
			numgets = sscanf (temp,"%s %s",file,alp);
		}else break;
    }
    if (loadall||i==numframes/2) {
      if (numgets==2) {
	Decal[i]=new Texture (file,alp,stage,ismipmapped,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_animations)?GFXTRUE:GFXFALSE,65536,detailtex?GFXTRUE:GFXFALSE);
      }else {
	Decal[i]=new Texture (file,stage,ismipmapped,TEXTURE2D,TEXTURE_2D,(g_game.use_animations)?GFXTRUE:GFXFALSE,65536,detailtex?GFXTRUE:GFXFALSE);
      }    
    }
  }
  if (!loadall) {
    Texture * dec = Decal[numframes/2];
    timeperframe*=numframes;
    numframes=1;
    if (Decal) {
      delete [] Decal;
    }
    Decal = new Texture * [1];
    Decal[0]=dec;
  }
  original = NULL;
  loadSuccess=true;
}
 bool AnimatedTexture::LoadSuccess (){
  return loadSuccess!=false;
}