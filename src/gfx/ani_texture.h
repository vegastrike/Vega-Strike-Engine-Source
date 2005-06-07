#include "aux_texture.h"
#include "vsfilesystem.h"
#include <stdio.h>
class AnimatedTexture: public Texture {
  unsigned int numframes;
  float timeperframe;
  double cumtime;
  Texture ** Decal;
  unsigned int active;
  unsigned int activebound; //For video mode
  double physicsactive;
  bool loadSuccess;
  void AniInit();

  //For video mode
  bool vidMode;
  bool detailTex;
  enum FILTER ismipmapped;
  int texstage;

  vector<string> frames;
  string wrapper_file_path;
  VSFileType wrapper_file_type;

 public:
  virtual void setTime(double tim);
  virtual double curTime() const { return cumtime; }
  virtual unsigned int numFrames() const { return numframes; }
  virtual float framesPerSecond()const {return 1/timeperframe;}
  AnimatedTexture ();
  AnimatedTexture (const char * file, int stage, enum FILTER imm, bool detailtexture=false);
  AnimatedTexture (VSFileSystem::VSFile &openedfile, int stage, enum FILTER imm,bool detailtexture=false);
  void Load(VSFileSystem::VSFile & f, int stage, enum FILTER ismipmapped, bool detailtex=false);
  virtual void LoadFrame(int num); //For video mode
  void Destroy();
  virtual Texture *Original();
  ~AnimatedTexture();
  virtual Texture * Clone ();
  void MakeActive();
  void MakeActive(int stage);
  static void UpdateAllPhysics();
  static void UpdateAllFrame();
  //resets the animation to beginning
  void Reset();
  bool Done();
  virtual bool LoadSuccess ();
};
