#include "gfxlib.h"
class StreamTexture {
  unsigned char * data;
  FILTER filtertype;
  int handle;
public:
  int width,height;
  StreamTexture(int width, int height, enum FILTER filtertype, unsigned char * origdata);
  ~StreamTexture();
  unsigned char * Map();
  void UnMap(bool changed=true);
  void MakeActive(int stage);
  void MakeActive() {MakeActive(0);}
  StreamTexture* Clone();
};
