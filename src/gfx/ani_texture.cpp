#include "ani_texture.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "lin_time.h"
#include "vegastrike.h"
//extern Hashtable<string, Texture,char [127]> texHashTable;

static vector <AnimatedTexture *> myvec;



void AnimatedTexture::MakeActive () {
  if (Decal)
    Decal[active]->MakeActive();
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
      myvec[i]->active = ((int)(myvec[i]->cumtime/myvec[i]->timeperframe))%myvec[i]->numframes;
    }
  }
}
bool AnimatedTexture::Done() {
  return physicsactive<0;
}

AnimatedTexture::AnimatedTexture (const char *file,int stage, enum FILTER imm){
  Decal=NULL;
  FILE * fp = fopen (file,"r");
  if (fp){
    Load (fp,stage,imm);
  }
  fclose (fp);
}

AnimatedTexture::AnimatedTexture (FILE * fp, int stage, enum FILTER imm){
  Decal=NULL;
  if (fp)
    Load (fp,stage,imm);
}

Texture *AnimatedTexture::Original(){
  return Decal[active]->Original();
}

Texture *AnimatedTexture::Clone () {
  AnimatedTexture * retval = new AnimatedTexture ();
  *retval = *this;
  retval->Decal = new Texture * [numframes];
  for (int i=0;i<numframes;i++) {
    retval->Decal[i]= Decal[i]->Clone ();
  }
  myvec.push_back (retval);
  return retval;
}

AnimatedTexture::~AnimatedTexture () {
  Destroy();
  data= NULL;
  palette=NULL;
}
AnimatedTexture::AnimatedTexture () {
  Decal=NULL;
  original = NULL;
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
    delete Decal;
    Decal=NULL;
  }
}
void AnimatedTexture::Load(FILE * fp, int stage, enum FILTER ismipmapped) {
  myvec.push_back (this);
  fscanf (fp,"%d %f",&numframes,&timeperframe);
  cumtime=0;
  physicsactive = numframes*timeperframe;
  active=0;
  Decal = new Texture * [numframes];
  char temp[512];
  char file[512];
  char alp[512];
  for (int i=0;i<numframes;i++) {
    int numgets=0;
    while (numgets<=0&&!feof (fp)) {
      fgets (temp,511,fp);
      temp[511]='\0';
      file[0]='z';file[1]='\0';
      alp[0]='z';alp[1]='\0';//windows crashes on null
  
      numgets = sscanf (temp,"%s %s",file,alp);
    }
    if (numgets==2) {
      Decal[i]=new Texture (file,alp,stage,ismipmapped);
    }else {
      Decal[i]=new Texture (file,stage,ismipmapped);
    }    
  }
  original = NULL;
}
