#include "vdu.h"
#include "cmd/unit.h"
#include "hud.h"
VDU::VDU (const char * file, TextPlane *textp, unsigned char modes, short rwws, short clls) :Sprite (file),tp(textp),posmodes(modes),thismode(WEAPON), rows(rwws), cols(clls){
  SwitchMode();

};

void VDU::DrawTarget(Unit * target) {
  tp->Draw ("\nYoDahdAhdas djsakljdlksajd");
}

void VDU::DrawNav (const Vector & nav) {

}
void VDU::DrawDamage(Unit * parent) {

}

void VDU::DrawWeapon (Unit * parent) {

}

void VDU::Draw (Unit * parent) {
  Sprite::Draw();
  if (!parent) {
    return;
  }
  //configure text plane;
  float x,y;
  float h,w;
  GetSize (w,h);
  GetPosition (x,y);
  tp->SetCharSize (fabs(w/cols),fabs(h/rows));
  
  h=fabs(h/2);  w=fabs (w/2);
  tp->SetPos (x-w,y+h);
  tp->SetSize (x+w,y-h);
  switch (thismode) {
  case TARGET:
    parent = parent->GetComputerData().target.GetUnit();
    if (parent)
      DrawTarget(parent);
    break;
  case NAV:
    DrawNav(parent->ToLocalCoordinates (parent->GetComputerData().NavPoint-parent->Position()));
    break;
  case DAMAGE:
    DrawDamage(parent);
    break;
  case WEAPON:
    DrawWeapon(parent);
    break;
  }

}

void VDU::SwitchMode() {
  thismode<<=1;
  while (!(thismode&posmodes)) {
    if (thismode>posmodes) {
      thismode=0x1;
    } else {
      thismode<<=1;
    }
  }
}
