#include "aux_texture.h"
class StreamTexture:public Texture {
  unsigned char * mutabledata;
  FILTER filtertype;
  int handle;
public:
  StreamTexture(int width, int height, enum FILTER filtertype, unsigned char * origdata);
  ~StreamTexture();
  unsigned char * Map();
  void UnMap(bool changed=true);
  void MakeActive(int stage);
  void MakeActive() {MakeActive(this->stage);}
  StreamTexture* Clone();
};
