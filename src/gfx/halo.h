#include "gfxlib.h"
#include "vec.h"
#include "quaternion.h"

class Halo {
  Vector position;
  float sizex;
  float sizey;
  int decal;
  int quadnum;
  //static enum BLENDFUNC blendSrc;
  //static enum BLENDFUNC blendDst;
 public:
  Halo (const char * texture, const GFXColor &col=GFXColor (1,1,1,.5), const Vector & pos= Vector (0,0,0), float sizx= 1, float sizy =1 );
  ~Halo();
  void Draw (const Transformation &quat = identity_transformation, const Matrix m = identity_matrix, float alpha=-1);
  static void ProcessDrawQueue();
  void SetDimensions (float wid, float hei) {sizex = wid; sizey=hei;}
  void SetPosition (float x,float y, float z) {position = Vector (x,y,z);}
  void SetPosition (const Vector &k) {position = k;}
  Vector &Position(){return position;}
  void SetColor (const GFXColor &col);


};
