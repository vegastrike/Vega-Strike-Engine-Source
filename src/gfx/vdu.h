#ifndef _GFX_VDU_H_
#define _GFX_VDU_H_

#include "sprite.h"



class Unit;
class TextPlane;


/**
 * Holds information for dedicated computer output
 * on a fighter craft. Can print out target info, local damage info
 * Local weapon info or Nav info.
 */
class VDU: public Sprite {
 private:
  unsigned short * StartArmor;
  float *maxhull;
  ///Holds a pointer to the font used to render text in the vdu
  TextPlane *tp;
  ///potential modes this computer is equipped for
  unsigned char posmodes;
  ///The current mode (VDU_MODE) that this unit is in
  unsigned char thismode;
  ///Rows and columns of text this VDU can display
  short rows,cols;
  ///Draws a shield display in the VDU
  void DrawVDUShield (Unit * parent);
  ///Draws the target display of Unit *target (only call with nonNULL)
  void DrawTarget(Unit * parent, Unit *target);
  ///Draws the message from the messagecenter
  void DrawMessages();
  ///Draws the nav point this unit is heading to
  void DrawNav(const Vector &navPoint);
  ///Draws the damage this unit has sustained
  void DrawDamage(Unit * parent);
  ///Draws the weapons activated on current unit
  void DrawWeapon(Unit *parent);
  ///Draws the sprite representing the current starship. Returns proper location and size for future use (with weaps and damage display)
  void DrawTargetSpr(Sprite * s,float percent, float &x, float &y, float &w, float &h);
public:
  enum VDU_MODE {TARGET=0x1,NAV=0x2,WEAPON=0x4, DAMAGE=0x8, SHIELD=0x10, VIEW=0x20, MSG=0x40};
  VDU(const char *file, TextPlane *textp,unsigned char modes, short rows, short cols, unsigned short *MaxArmor, float * maxhull);
  ///Draws the entire VDU, all data, etc
  void Draw(Unit * parent);
  ///Changes the mode of the current VDU to another legal mode
  void SwitchMode();
};
#endif
