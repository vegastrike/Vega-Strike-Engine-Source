#include "aux_texture.h"
#include <stdio.h>
class AnimatedTexture: public Texture {
  int numframes;
  float timeperframe;
  double cumtime;
  Texture ** Decal;
  unsigned int active;
  double physicsactive;
  void AniInit();
 public:
  AnimatedTexture ();
  AnimatedTexture (const char * file, int stage, enum FILTER imm);
  AnimatedTexture (FILE * openedfile, int stage, enum FILTER imm);
  void Load (char * buffer, int length, int nframe, enum FILTER imm);
  void Load (FILE * openedfile, int stage, enum FILTER imm);
  void Destroy();
  virtual Texture *Original();
  ~AnimatedTexture();
  virtual Texture * Clone ();
  void MakeActive();
  static void UpdateAllPhysics();
  static void UpdateAllFrame();
  //resets the animation to beginning
  void Reset();
  bool Done();
};
