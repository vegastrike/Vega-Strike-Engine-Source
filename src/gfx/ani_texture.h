#include "aux_texture.h"
#include <stdio.h>
class AnimatedTexture: public Texture {
  int numframes;
  float timeperframe;
  float cumtime;
  Texture ** Decal;
  int active;
  float physicsactive;
 public:
  AnimatedTexture ();
  AnimatedTexture (const char * file, int stage, enum FILTER imm);
  AnimatedTexture (FILE * openedfile, int stage, enum FILTER imm);
  void Load (FILE * openedfile, int stage, enum FILTER imm);
  void Destroy();
  virtual Texture *Original() const;
  ~AnimatedTexture();
  virtual Texture * Clone ();
  void MakeActive();
  static void UpdateAllPhysics();
  static void UpdateAllFrame();
  bool Done();
};
