#include "cmd/unit_generic.h"
#include "ani_texture.h"
#include "audiolib.h"

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "lin_time.h"
#include "vegastrike.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "vs_random.h"
#include "../gldrv/gl_globals.h"
#include <set>
#include <math.h>

using std::set;
static set<AnimatedTexture *> anis;

static enum ADDRESSMODE parseAddressMode(const string& addrmodestr, ADDRESSMODE defaultAddressMode) {
  enum ADDRESSMODE addrmode = defaultAddressMode;
  if (addrmodestr=="wrap")
      addrmode = WRAP; else if (addrmodestr=="mirror")
      addrmode = MIRROR; else if (addrmodestr=="clamp")
      addrmode = CLAMP; else if (addrmodestr=="border")
      addrmode = BORDER;
  return addrmode;
}

static void ActivateWhite(int stage) {
	static Texture * white = new Texture("white.bmp",0,MIPMAP,TEXTURE2D,TEXTURE_2D, 1 );
	if (white->LoadSuccess())
		white->MakeActive(stage);
}

void AnimatedTexture::MakeActive (int stage, int pass) {
    switch (pass) {
    case 0:
        if (!vidMode) {
            if (GetInterpolateFrames()&&(active!=nextactive)) {
                if (gl_options.Multitexture&&((stage+1)<gl_options.Multitexture)) {
                    if (Decal&&Decal[nextactive%numframes])
                        Decal[nextactive%numframes]->MakeActive(stage+1); else
                        ActivateWhite(stage+1);
                    GFXTextureEnv(stage+1,GFXINTERPOLATETEXTURE,active_fraction);
                    if (Decal&&Decal[active%numframes])
                        Decal[active%numframes]->MakeActive(stage); else
                        ActivateWhite(stage);
                    //GFXTextureEnv(stage,GFXMODULATETEXTURE);
                } else {
                    if (Decal&&Decal[active%numframes])
                        Decal[active%numframes]->MakeActive(stage); else
                        ActivateWhite(stage);
                    multipass_interp_basecolor=GFXColorf();
                    GFXColor color=multipass_interp_basecolor;
                    color.r *= (1.0-active_fraction);
                    color.g *= (1.0-active_fraction);
                    color.b *= (1.0-active_fraction);
                    GFXColorf(color);
                    //GFXTextureEnv(stage,GFXMODULATETEXTURE);
                }
            } else {
                if (Decal&&Decal[active%numframes]) 
                    Decal[active%numframes]->MakeActive(stage); else
                    ActivateWhite(stage);
            };
        } else if(!vidSource) {
            // No frame interpolation anything supported
            if (Decal&&*Decal) {
                if (active != activebound)
                    LoadFrame(active%numframes);
                (*Decal)->MakeActive(stage);
            }
        } else {
            try {
                // vidSource leaves frame data in its framebuffer, and our image data is initialized
                // to point to that framebuffer, so all we need to do is transfer it to the GL.
                if (vidSource->seek(curtime))
                    Transfer( 65535, GFXFALSE );
            } catch(::VideoFile::EndOfStreamException e) {
                if (GetLoop() && curtime > 0) {
                    setTime(0);
                    MakeActive(stage,pass);
                    return;
                }
            } catch(::VideoFile::Exception e) {
                VSFileSystem::vs_fprintf(stderr, "\nVideoFile exception: %s\n", e.what());
            }
            Texture::MakeActive(stage, pass);
        }
        break;
    case 1:
        if (!vidMode&&GetInterpolateFrames()&&(active!=nextactive)&&!(gl_options.Multitexture&&((stage+1)<gl_options.Multitexture))) {
            if (Decal&&Decal[nextactive%numframes])
                Decal[nextactive%numframes]->MakeActive(stage); else
                ActivateWhite(stage);
            GFXColor color=multipass_interp_basecolor;
            color.r *= active_fraction;
            color.g *= active_fraction;
            color.b *= active_fraction;
            GFXColorf(color);
            //GFXTextureEnv(stage,GFXMODULATETEXTURE);
        } else ActivateWhite(stage);
        break;
    };
}
bool AnimatedTexture::SetupPass(int pass, int stage, const enum BLENDFUNC src, const enum BLENDFUNC dst)
{
    switch (pass) {
    case -1:
        if (!vidMode&&GetInterpolateFrames()) {
            if (!(gl_options.Multitexture&&((stage+1)<gl_options.Multitexture))) {
                GFXColorf(multipass_interp_basecolor); //Restore old color
            } else {
                //GFXTextureEnv(texstage,GFXMODULATETEXTURE); //Most expect this
                GFXTextureEnv(stage+1,GFXADDTEXTURE); //Most expect this
            }
        }
        return true;
    default:
        // Hey! Nothing to do! Yippie!
        return true;
    }
}
void AnimatedTexture::UpdateAllPhysics() {
    for (set<AnimatedTexture *>::iterator iter=anis.begin(); iter!=anis.end(); iter++)
        (*iter)->physicsactive-=SIMULATION_ATOM;
}
void AnimatedTexture::UpdateAllFrame() {
  double elapsed = GetElapsedTime();
  for (set<AnimatedTexture *>::iterator iter=anis.begin(); iter!=anis.end(); iter++) {
    if ( (*iter)->GetTimeSource() )
        (*iter)->setTime( AUDGetCurrentPosition( (*iter)->GetTimeSource() ) ); else
        (*iter)->setTime( (*iter)->curTime()+elapsed );
  }
}

bool AnimatedTexture::Done() {
  //return physicsactive<0;
  // Explosions aren't working right, and this would fix them.
  // I don't see the reason for using physics frames as reference, all AnimatedTextures
  // I've seen are gaphic-only entities (bolts use their own time-keeping system, for instance)
  // If I'm wrong, and the above line is crucial, well... feel free to fix it.
  return curtime >= numframes*timeperframe; 
}
static unsigned int intmin(unsigned int a, unsigned int b){
  return a<b?a:b;
}
static unsigned int intmax(unsigned int a, unsigned int b){
  return a<b?b:a;
}
void AnimatedTexture::setTime (double tim) {
    curtime=tim;
    
    if (timeperframe && !vidSource) {
      unsigned int numframes=numFrames();
      unsigned int active=((unsigned int)(curtime/timeperframe));
      if (GetLoop()) 
          active %= numframes; else 
          active = intmin(active,numframes-1);
      unsigned int nextactive=(GetLoopInterp()?((active+1)%numframes):intmin(active+1,numframes-1));
      float fraction = (curtime/timeperframe)-(unsigned int)(curtime/timeperframe);
      if (fraction<0) fraction += 1.0f;
      this->active = active;
      this->nextactive = nextactive;
      this->active_fraction = fraction;
      if (!vidMode) {
          if (GetInterpolateTCoord()&&(active!=nextactive)) {
              if (Decal&&Decal[active]&&Decal[nextactive]) {
                  this->maxtcoord = (1-fraction)*Decal[active]->maxtcoord + fraction*Decal[nextactive]->maxtcoord;
                  this->mintcoord = (1-fraction)*Decal[active]->mintcoord + fraction*Decal[nextactive]->mintcoord;
              }
          } else {
              if (Decal&&Decal[active]) {
                  this->maxtcoord = Decal[active]->maxtcoord;
                  this->mintcoord = Decal[active]->mintcoord;
              }
          }
      } else {
          if (GetInterpolateTCoord()&&(active!=nextactive)) {
              if (frames_maxtc.size()<intmax(active,nextactive)) {
                  this->maxtcoord = (1-fraction)*frames_maxtc[active] + fraction*frames_maxtc[nextactive];
                  this->mintcoord = (1-fraction)*frames_mintc[active] + fraction*frames_mintc[nextactive];
              }
          } else {
              if (active<frames_maxtc.size()) {
                  this->maxtcoord = frames_maxtc[active];
                  this->mintcoord = frames_mintc[active];
              }
          }
      }
      
      active = ((unsigned int)(curtime/timeperframe))%numframes;
    }
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
  curtime=0;
  original = NULL;
  loadSuccess=false;
  texstage=0;
  ismipmapped=BILINEAR;
  detailTex=false;
  vidMode=false;
  vidSource=0;
  constframerate=true;
  options=optLoop;
  defaultAddressMode=DEFAULT_ADDRESS_MODE;
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

AnimatedTexture::AnimatedTexture (int stage, enum FILTER imm, bool detailtex) :
    Texture(stage, imm)
{
  AniInit();
}

Texture *AnimatedTexture::Original(){
  return Decal?Decal[active]->Original():this;
}

Texture *AnimatedTexture::Clone () {
  AnimatedTexture * retval = new AnimatedTexture ();
  
  if (Decal) {
    *retval = *this;
    int nf=vidMode?1:numframes;
    retval->Decal = new Texture * [nf];
    for (int i=0;i<nf;i++) {
      retval->Decal[i]= Decal[i]->Clone ();
    }
  } else if (vidSource) {
    *retval = *this;
  }
  
  if (vidSource) {
    retval->name = -1;
    retval->bound = false;
    VSFileSystem::VSFile f;
    f.OpenReadOnly(wrapper_file_path, wrapper_file_type);
    retval->LoadVideoSource(f);
  } else if (Decal) {
    // LoadVideoSource adds to anis, otherwise we'll have to add ourselves
    anis.insert(retval);
  }
  
  return retval;
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
  anis.erase(this);
  
  if (vidSource) {
    delete vidSource;
    vidSource = 0;
  }
  
  if (Decal) {
    int i,nf;
    nf = vidMode?1:numframes;
    for (i=0;i<nf;i++)
      delete Decal[i];
    delete []Decal;
    Decal=NULL;
  }
}
void AnimatedTexture::Reset () {
  curtime=0;
  active=0;
  activebound=-1;
  physicsactive = numframes*timeperframe;
}

static void alltrim(string &str)
{
    string::size_type ltrim=str.find_first_not_of(" \t\r\n");
    string::size_type rtrim=str.find_last_not_of(" \t\r\n"); 
    if (rtrim!=string::npos) str.resize(rtrim+1);
    str.erase(0,ltrim);
}

static void alltrim(char *_str)
{
    string str=_str;
    alltrim(str);
    strcpy(_str,str.c_str());
}

void AnimatedTexture::Load(VSFileSystem::VSFile & f, int stage, enum FILTER ismipmapped, bool detailtex) 
{
    curtime=0;
    frames.clear();
    frames_maxtc.clear();
    frames_mintc.clear();

    if (f.GetType() == VSFileSystem::VideoFile) {
        LoadVideoSource(f);
    } else {
        LoadAni(f, stage, ismipmapped, detailtex);
    }
}

void AnimatedTexture::LoadVideoSource(VSFileSystem::VSFile & f)
{
    wrapper_file_path = f.GetFilename();
    wrapper_file_type = f.GetType();
    f.Close();
    
    Reset();
    
    vidMode = true;
    
    try {
        vidSource = new ::VideoFile();
        vidSource->open(wrapper_file_path, gl_options.max_movie_dimension);
        
        physicsactive = vidSource->getDuration();
        timeperframe = 1.0 / vidSource->getFrameRate();
        numframes = (unsigned int)(physicsactive * timeperframe);
        
        loadSuccess = true;
    } catch(::VideoFile::Exception e) {
        loadSuccess = false;
    }
    
    if (loadSuccess) {
        sizeX = vidSource->getWidth();
        sizeY = vidSource->getHeight();
        mode  = _PNG24BIT;
        data  = (unsigned char*)vidSource->getFrameBuffer();
        Bind(65535, GFXFALSE);
        
        anis.insert(this);
    }
}

AnimatedTexture* AnimatedTexture::CreateVideoTexture(const std::string &fname, int stage, enum FILTER ismipmapped, bool detailtex)
{
    AnimatedTexture *rv = new AnimatedTexture(stage, ismipmapped, detailtex);
    VSFileSystem::VSFile f;
    VSError err=f.OpenReadOnly(fname, VSFileSystem::VideoFile);
    if (err <= Ok) {
      rv->LoadVideoSource(f);
    } else {
      fprintf(stderr, "CreateVideoTexture could not find %s\n", fname.c_str());
    }
    return rv;
}

void AnimatedTexture::LoadAni(VSFileSystem::VSFile & f, int stage, enum FILTER ismipmapped, bool detailtex) {
  char options[1024]; 
  f.Fscanf("%d %f",&numframes,&timeperframe);
  f.ReadLine(options,sizeof(options)-sizeof(*options)); options[sizeof(options)/sizeof(*options)-1]=0;
  alltrim(options);
  
  Reset();

  vidMode = XMLSupport::parse_option_ispresent(options,"video");
  SetInterpolateFrames(XMLSupport::parse_option_ispresent(options,"interpolateFrames"));
  SetInterpolateTCoord(XMLSupport::parse_option_ispresent(options,"interpolateTCoord"));
  if (XMLSupport::parse_option_ispresent(options,"forceLoopInterp"))
      SetLoopInterp(true); else if (XMLSupport::parse_option_ispresent(options,"forceNoLoopInterp"))
      SetLoopInterp(false);
  if (XMLSupport::parse_option_ispresent(options,"forceLoop"))
      SetLoop(true); else if (XMLSupport::parse_option_ispresent(options,"forceNoLoop"))
      SetLoop(false);
  if (XMLSupport::parse_option_ispresent(options,"startRandom")) {
	  int curf = vsrandom.genrand_int32()%numframes;
	  setTime((0.00001+curf)*timeperframe);
  }
  string addrmodestr = XMLSupport::parse_option_value(options,"addressMode","");
  defaultAddressMode = parseAddressMode(addrmodestr,DEFAULT_ADDRESS_MODE);

  string defms = XMLSupport::parse_option_value(options,"mins","0");
  string defmt = XMLSupport::parse_option_value(options,"mint","0");
  string defmr = XMLSupport::parse_option_value(options,"minr","0");
  string defMs = XMLSupport::parse_option_value(options,"maxs","1");
  string defMt = XMLSupport::parse_option_value(options,"maxt","1");
  string defMr = XMLSupport::parse_option_value(options,"maxr","1");

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
  char opt[512]="";
  int i=0,j=0;

  for (;i<numframes;i++) if (loadall||(i==midframe)) { //if() added by Klauss
    int numgets=0;
    while (numgets<=0&&!f.Eof()) {
        if (f.ReadLine(temp,511)==Ok) {
            temp[511]='\0';
            file[0]='z';file[1]='\0';
            alp[0]='z';alp[1]='\0';//windows crashes on null
            opt[0]='z';opt[1]='\0';

            numgets = sscanf (temp,"%s %s %[^\r\n]",file,alp,opt);
            if ((numgets<2)||(strcmp(alp,"-")==0)) alp[0]='\0';
            alltrim(opt);
        } else break;
    }
    if (vidMode) {
        frames.push_back(StringPool::Reference(string(temp)));
        frames_mintc.push_back(Vector(
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"mins",defms)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"mint",defmt)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"minr",defmr))));
        frames_maxtc.push_back(Vector(
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"maxs",defMs)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"maxt",defMt)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"maxr",defMr))));
    } else {
      enum ADDRESSMODE addrmode = parseAddressMode(XMLSupport::parse_option_value(opt,"addressMode",""),defaultAddressMode);
      if (alp[0]!='\0')
          Decal[j++]=new Texture (file,alp,stage,ismipmapped,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_animations)?GFXTRUE:GFXFALSE,65536,(detailtex?GFXTRUE:GFXFALSE),GFXFALSE,addrmode); else
          Decal[j++]=new Texture (file,stage,ismipmapped,TEXTURE2D,TEXTURE_2D,(g_game.use_animations)?GFXTRUE:GFXFALSE,65536,(detailtex?GFXTRUE:GFXFALSE),GFXFALSE,addrmode);
      if (Decal[j-1]) {
          Decal[j-1]->mintcoord=Vector(
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"mins",defms)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"mint",defmt)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"minr",defmr)));
          Decal[j-1]->maxtcoord=Vector(
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"maxs",defMs)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"maxt",defMt)),
            XMLSupport::parse_float(XMLSupport::parse_option_value(opt,"maxr",defMr)));
      }
    }
  }

  this->texstage = stage;
  this->detailTex = detailtex;
  this->ismipmapped = ismipmapped;

  if (vidMode) {
      wrapper_file_path = f.GetFilename();
      wrapper_file_type = f.GetType();
  }

  original = NULL;
  loadSuccess=true;

  anis.insert(this);

  //Needed - must do housekeeping, tcoord stuff and the like.
  setTime(curtime);
}
 
void AnimatedTexture::LoadFrame(int frame) {
  if (!vidMode||(Decal==NULL)||(*Decal==NULL)) return;
  if ((frame<0)||(frame>=numframes)) return;
  if ((activebound>=0)&&(activebound<numframes)&&(frames[frame]==frames[activebound])) return;

  const char *temp = frames[frame].get().c_str();
  char file[512]="white.bmp";
  char alp[512]="white.bmp";
  char opt[512]="";
  int numgets=0;
  numgets = sscanf (temp,"%s %s %[^\r\n]",file,alp,opt);
  if ((numgets<2)||(strcmp(alp,"-")==0)) alp[0]='\0';
  string addrmodestr = XMLSupport::parse_option_value(opt,"addressMode","");
  enum ADDRESSMODE addrmode = parseAddressMode(addrmodestr,defaultAddressMode);

  //Override compression options temporarily
  //    NOTE: This is ugly, but otherwise we would have to hack Texture way too much,
  //    or double the code. Let's use this then.
  int ocompression = gl_options.compression;
  gl_options.compression = 0;
   
  //Without this, VSFileSystem won't find the file -- ugly, but it's how it is.
  VSFile f;
  VSError err=f.OpenReadOnly( wrapper_file_path, wrapper_file_type );
  
  //Override mipmaping for video mode - too much overhead in generating the mipmamps.
  enum FILTER ismip2 = ((ismipmapped==BILINEAR)||(ismipmapped==TRILINEAR)||(ismipmapped==MIPMAP))?BILINEAR:NEAREST;
  loadSuccess=true;
  if (alp[0]!='\0')
      (*Decal)->Load(file,alp,texstage,ismip2,TEXTURE2D,TEXTURE_2D,1,0,(g_game.use_videos)?GFXTRUE:GFXFALSE,65536,(detailTex?GFXTRUE:GFXFALSE),GFXTRUE,addrmode); else if (numgets==1)
      (*Decal)->Load(file,texstage,ismip2,TEXTURE2D,TEXTURE_2D,(g_game.use_videos)?GFXTRUE:GFXFALSE,65536,(detailTex?GFXTRUE:GFXFALSE),GFXTRUE,addrmode); else
      loadSuccess=false;

  if (err<=Ok) f.Close();
  
  gl_options.compression = ocompression;

  original = NULL;
  loadSuccess = loadSuccess && (*Decal)->LoadSuccess();
  if (loadSuccess) activebound = frame;
}

bool AnimatedTexture::LoadSuccess () {
  return loadSuccess!=false;
}

unsigned int AnimatedTexture::numLayers() const {
  if (GetInterpolateFrames()&&(active!=nextactive)&&gl_options.Multitexture&&((texstage+1)<gl_options.Multitexture))
      return 2; else
      return 1;
}
unsigned int AnimatedTexture::numPasses() const {
    if (GetInterpolateFrames()&&(active!=nextactive)) {
        if (gl_options.Multitexture&&((texstage+1)<gl_options.Multitexture))
            return 1; else
            return 2;
    } else return 1;
}
