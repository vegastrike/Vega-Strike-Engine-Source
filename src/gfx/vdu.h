#include "sprite.h"



class Unit;
class TextPlane;



class VDU: public Sprite {
 private:
  TextPlane *tp;
  unsigned char posmodes;
  unsigned char thismode;
  short rows,cols;
  void DrawTarget(Unit *target);
  void DrawNav(const Vector &navPoint);
  void DrawDamage(Unit * parent);
  void DrawWeapon(Unit *parent);
  void DrawTargetSpr(Sprite * s,float percent);
public:
  enum VDU_MODE {TARGET=0x1,NAV=0x2,DAMAGE=0x4,WEAPON=0x8};
  VDU(const char *file, TextPlane *textp,unsigned char modes, short rows, short cols);
  void Draw(Unit * parent);
  void SwitchMode();
};
