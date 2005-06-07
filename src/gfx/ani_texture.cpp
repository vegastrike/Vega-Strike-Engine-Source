#include "cmd/unit_generic.h"
#include "ani_texture.h"

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "lin_time.h"
#include "vegastrike.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "../gldrv/gl_globals.h"

static vector <AnimatedTexture *> myvec;



void AnimatedTexture::MakeActive () {
    if (!vidMode) {
        if (Decal) Decal[active%numframes]->MakeActive(); 
    } else MakeActive(texstage);
}
void AnimatedTexture::MakeActive (int stage) {
    if (!vidMode) {
        if (Decal) Decal[active%numframes]->MakeActive(stage);
    } else {
        if (Decal&&*Decal) {
            if (active != activebound)
                LoadFrame(active%numframes);
            (*Decal)->MakeActive(stage);
        }
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
  activebound=-1;
  active=0;
  original = NULL;
  loadSuccess=false;
  texstage=0;
  ismipmapped=BILINEAR;
  detailTex=false;
  vidMode=false;
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
    int nf=vidMode?1:numframes;
    retval->Decal = new Texture * [nf];
    for (int i=0;i<nf;i++) {
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
  int i,nf;
  if (Decal) {
    for (i=0;i<(int)myvec.size();i++) {
      if (myvec[i]==this) {
	myvec.erase (myvec.begin()+i);
      }
    }
    nf = vidMode?1:numframes;
    for (i=0;i<nf;i++) {
      delete Decal[i];
    }
    delete []Decal;
    Decal=NULL;
  }
}
void AnimatedTexture::Reset () {
  cumtime=0;
  active=0;
  activebound=-1;
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
  char options[1024]; 
  f.Fscanf ("%d %f",&numframes,&timeperframe);
  f.ReadLine(options,sizeof(options)-sizeof(*options)); options[sizeof(options)/sizeof(*options)-1]=0;
  cumtime=0;
  Reset();
  frames.clear();

  vidMode = (strstr(options,"video")!=NULL);

  int midframe;
  bool loadall;
  if (!vidMode)
      loadall=!(g_game.use_animations==0||(g_game.use_animations!=0&&g_game.use_textures==0)); else
      loadall=!(g_game.use_videos==0||(g_game.use_videos!=0&&g_game.use_textures==0)); 
  if (!loadall) { timeperframe *= numframes; midframe = numframes/2; numframes=1; }; //Added by Klauss

  active=0;
  int nf = (vidMode?1:numframes);
  Decal = new Texture * [nf];
  if (vidMode) Decal[0]=new Texture;
  char temp[512]="white.bmp";
  char file[512]="white.bmp";
  char alp[512]="white.bmp";
  int i=0,j=0;

  for (;i<numframes;i++) if (loadall||(i==midframe)) { //if() added by Klauss
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
        if (vidMode) {
            frames.push_back(string(temp));
      }else {
          if (numgets==2)
              Decal[j++]=new Texture (file,alp,stage,ismipmapped,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_animations)?GFXTRUE:GFXFALSE,65536,detailtex?GFXTRUE:GFXFALSE); else //j++ was i, changed by Klauss
	Decal[j++]=new Texture (file,stage,ismipmapped,TEXTURE2D,TEXTURE_2D,(g_game.use_animations)?GFXTRUE:GFXFALSE,65536,detailtex?GFXTRUE:GFXFALSE); //j++ was i, changed by Klauss
      }    
    }
  }

  if (vidMode) {
      this->texstage = stage;
      this->detailTex = detailtex;
      this->ismipmapped = ismipmapped;

      wrapper_file_path = f.GetFilename();
      wrapper_file_type = f.GetType();
    }

  original = NULL;
  loadSuccess=true;

  myvec.push_back(this);
}
 
void AnimatedTexture::LoadFrame(int frame) {
  if (!vidMode||(Decal==NULL)||(*Decal==NULL)) return;
  if ((frame<0)||(frame>=numframes)) return;
  if ((activebound>=0)&&(activebound<numframes)&&(frames[frame]==frames[activebound])) return;

  const char *temp = frames[frame].c_str();
  char file[512]="white.bmp";
  char alp[512]="white.bmp";
  int numgets=0;
  numgets = sscanf (temp,"%s %s",file,alp);

  //Override compression options temporarily
  //    NOTE: This is ugly, but otherwise we would have to hack Texture way too much,
  //    or double the code. Until Texture supports overriding compression options,
  //    let's use this.
  bool os3tc = gl_options.s3tc;
  int ocompression = gl_options.compression;
  gl_options.s3tc = false;
  gl_options.compression = 0;

  //Without this, VSFileSystem won't find the file -- ugly, but it's how it is.
  VSFile f;
  VSError err=f.OpenReadOnly( wrapper_file_path, wrapper_file_type );

  //Override mipmaping for video mode - too much overhead in generating the mipmamps.
  enum FILTER ismip2 = ((ismipmapped==BILINEAR)||(ismipmapped==TRILINEAR)||(ismipmapped==MIPMAP))?BILINEAR:NEAREST;

  loadSuccess=true;
  if (numgets==2)
      (*Decal)->Load(file,alp,texstage,ismip2,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_videos)?GFXTRUE:GFXFALSE,65536,(detailTex?GFXTRUE:GFXFALSE),GFXTRUE); else if (numgets==1)
      (*Decal)->Load(file,texstage,ismip2,TEXTURE2D,TEXTURE_2D,(g_game.use_videos)?GFXTRUE:GFXFALSE,65536,(detailTex?GFXTRUE:GFXFALSE),GFXTRUE); else
      loadSuccess=false;

  if (err==Ok) f.Close();

  gl_options.s3tc = os3tc;
  gl_options.compression = ocompression;

  original = NULL;
  loadSuccess = loadSuccess && (*Decal)->LoadSuccess();
  if (loadSuccess) activebound = frame;
}

 bool AnimatedTexture::LoadSuccess (){
  return loadSuccess!=false;
}
