#include "aux_texture.h"


class PipelinedTexture: public Texture {
  int current;
  int last;
  //clone==2 means both current and last are clones 1 means last is a clone
  char clone;
 public:
  bool operator == (const Texture &b) {return false;}
  bool operator < (const Texture &b) {return true;}
  virtual Texture * Clone ();
  virtual Texture * Original ();
  PipelinedTexture();
  PipelinedTexture(unsigned int width, unsigned int height, unsigned char * current, unsigned char * last);
  ~PipelinedTexture ();
  unsigned char * beginMutate();
  void Swap();
  void endMutate (int xoffset, int yoffset, int width, int height);
  void MakeBothActive();
  void MakeActive();
  void MakeLastActive();
};


