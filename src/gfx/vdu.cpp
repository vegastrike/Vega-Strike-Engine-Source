#include "vdu.h"
#include "cmd/unit.h"
#include "hud.h"
#include "vs_globals.h"
VDU::VDU (const char * file, TextPlane *textp, unsigned char modes, short rwws, short clls) :Sprite (file),tp(textp),posmodes(modes),thismode(VIEW), rows(rwws), cols(clls){
  SwitchMode();

};

void VDU::DrawTargetSpr (Sprite *s, float per, float &sx, float &sy, float &w, float &h) {
  float nw,nh;
  GetPosition (sx,sy);
  GetSize (w,h);
  if (!s)
    return;
  s->SetPosition (sx,sy);
  s->GetSize (nw,nh);
  h=-fabs (h*per);
  w= fabs(nw*h/nh);
  s->SetSize (w,h);
  s->Draw();
  s->SetSize (nw,nh);
}

void VDU::DrawTarget(Unit * target) {
  float x,y,w,h;
  char t[32];
  sprintf (t,"\n%4.1f %4.1f",target->FShieldData()*100,target->RShieldData()*100);
  tp->Draw (std::string("\n")+target->name+t);
  DrawTargetSpr (target->getHudImage (),.6,x,y,w,h);
  
}

void VDU::DrawNav (const Vector & nav) {

}
static void DrawGun (Vector  pos, float w, float h, weapon_info::MOUNT_SIZE sz) {
  w=fabs (w);
  h=fabs(h);
  float oox = 1./g_game.x_resolution;
  float ooy = 1./g_game.y_resolution;
  pos.j-=h/3.8;
  if (sz==weapon_info::NOWEAP) {
    GFXPointSize (4);
    GFXBegin (GFXPOINT);
    GFXVertexf (pos);
    GFXEnd ();
    GFXPointSize (1);
  } else if (sz<weapon_info::SPECIAL) {
    GFXBegin (GFXLINE);
    GFXVertex3f (pos.i+oox,pos.j,0);
    GFXVertex3f (pos.i+oox,pos.j-h/15,0);
    GFXVertex3f (pos.i-oox,pos.j,0);
    GFXVertex3f (pos.i-oox,pos.j-h/15,0);
    GFXVertex3f (pos.i+oox,pos.j-h/15,0);
    GFXVertex3f (pos.i-oox,pos.j-h/15,0);
    if (sz==weapon_info::LIGHT) {
      GFXVertex3f (pos.i,pos.j,0);
      GFXVertex3f (pos.i,pos.j+h/4,0);
      GFXVertex3f (pos.i,pos.j+h/4+ooy*2,0);
      GFXVertex3f (pos.i,pos.j+h/4+ooy*5,0);
    }else if (sz==weapon_info::MEDIUM) {
      GFXVertex3f (pos.i,pos.j,0);
      GFXVertex3f (pos.i,pos.j+h/5,0);
      GFXVertex3f (pos.i,pos.j+h/5+ooy*4,0);
      GFXVertex3f (pos.i,pos.j+h/5+ooy*5,0);
      GFXVertex3f (pos.i+oox,pos.j+h/5+ooy*2,0);      
      GFXVertex3f (pos.i-oox,pos.j+h/5+ooy*2,0);      
    }else if (sz==weapon_info::HEAVY) {
      GFXVertex3f (pos.i,pos.j,0);
      GFXVertex3f (pos.i,pos.j+h/5,0);
      GFXVertex3f (pos.i,pos.j+h/5+ooy*4,0);
      GFXVertex3f (pos.i,pos.j+h/5+ooy*5,0);
      GFXVertex3f (pos.i+2*oox,pos.j+h/5+ooy*3,0);      
      GFXVertex3f (pos.i,pos.j+h/5+ooy*2,0);      
      GFXVertex3f (pos.i-2*oox,pos.j+h/5+ooy*3,0);      
      GFXVertex3f (pos.i,pos.j+h/5+ooy*2,0);      
    }else {//capship gun
      GFXVertex3f (pos.i,pos.j,0);
      GFXVertex3f (pos.i,pos.j+h/6,0);
      GFXVertex3f (pos.i,pos.j+h/6+ooy*6,0);
      GFXVertex3f (pos.i,pos.j+h/6+ooy*7,0);
      GFXVertex3f (pos.i-oox,pos.j+h/6+ooy*2,0);
      GFXVertex3f (pos.i+oox,pos.j+h/6+ooy*2,0);
      GFXVertex3f (pos.i-2*oox,pos.j+h/6+ooy*4,0);
      GFXVertex3f (pos.i+2*oox,pos.j+h/6+ooy*4,0);
    }  
    GFXEnd ();
  }else if (sz==weapon_info::SPECIAL||sz==weapon_info::SPECIALMISSILE) {
    GFXPointSize (4);
    GFXBegin (GFXPOINT);
    GFXVertexf (pos);
    GFXEnd ();
    GFXPointSize (1);//classified...  FIXME    
  }else if (sz<weapon_info::HEAVYMISSILE) {
    GFXBegin (GFXLINE);
    GFXVertex3f (pos.i,pos.j-h/8,0);
    GFXVertex3f (pos.i,pos.j+h/8,0);
    GFXVertex3f (pos.i+2*oox,pos.j-h/8+2*ooy,0);
    GFXVertex3f (pos.i-2*oox,pos.j-h/8+2*ooy,0);
    GFXEnd();
  }else if (sz<=weapon_info::CAPSHIPHEAVYMISSILE) {
    GFXBegin (GFXLINE);
    GFXVertex3f (pos.i,pos.j-h/6,0);
    GFXVertex3f (pos.i,pos.j+h/6,0);
    GFXVertex3f (pos.i+3*oox,pos.j-h/6+2*ooy,0);
    GFXVertex3f (pos.i-3*oox,pos.j-h/6+2*ooy,0);
    GFXVertex3f (pos.i+oox,pos.j-h/6,0);
    GFXVertex3f (pos.i+oox,pos.j+h/9,0);
    GFXVertex3f (pos.i-oox,pos.j-h/6,0);
    GFXVertex3f (pos.i-oox,pos.j+h/9,0);
    GFXEnd();
  }
  
}
void VDU::DrawDamage(Unit * parent) {
  float x,y,w,h;
  DrawTargetSpr (parent->getHudImage (),.6);
  
}

void VDU::DrawWeapon (Unit * parent) {
    
  float x,y,w,h;
  const float percent = .6;
  DrawTargetSpr (parent->getHudImage (),percent,x,y,w,h);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING); 
  for (int i=0;i<parent->nummounts;i++) {
    Vector pos (parent->mounts[i].GetMountLocation().position);
    pos.i=-pos.i*fabs(w)/parent->rSize()*percent+x;
    pos.j=pos.k*fabs(h)/parent->rSize()*percent+y;
    pos.k=0;
    switch (parent->mounts[i].ammo!=0?parent->mounts[i].status:127) {
    case Unit::Mount::ACTIVE:
      GFXColor4f (0,1,.2,1);
      break;
    case Unit::Mount::INACTIVE:
      GFXColor4f (0,.5,0,1);
      break;
    case Unit::Mount::DESTROYED:
      GFXColor4f (.2,.2,.2,1);
      break;
    case Unit::Mount::UNCHOSEN:
      GFXColor4f (1,1,1,1);
      break;
    case 127:
      GFXColor4f (0,.2,0,1);
      break;
    }
    DrawGun (pos,w,h,parent->mounts[i].type.size);
  }
  GFXColor4f(1,1,1,1);
  
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
