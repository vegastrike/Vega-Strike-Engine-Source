#include "vdu.h"
#include "cmd/unit.h"
#include "hud.h"
#include "vs_globals.h"
#include "cockpit.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"

///ALERT to change must change enum in class
const std::string vdu_modes [] = {"Target","Nav","Weapon","Damage","Shield","View","Message"};

int vdu_lookup (char * &s) {
#ifdef _WIN32
#define strcasecmp stricmp
#endif
  int retval=0;
  char * t = strdup (s);
  int i;
  for (i=0;t[i]!='\0';i++) {
    if (isspace(t[i])) {
      s+=i+1;
      break;
    }
  } 
  if (t[i]=='\0') {
    s[0]='\0';
  }
  t[i]='\0';

  for (unsigned int j=0;j<((sizeof (vdu_modes)/sizeof (std::string)));j++) {
    if (0==strcasecmp (t,vdu_modes[j].c_str())) {
      retval|=(1<<j);
    }
  }
  free(t);
  return retval;
}
int parse_vdu_type (const char * x) {
  char * mystr = strdup (x);
  char *s = mystr;
  int retval=0;
  while (s[0]!='\0') {
    retval|=vdu_lookup (s);
  }
  free (mystr);
  return retval;
}




VDU::VDU (const char * file, TextPlane *textp, unsigned char modes, short rwws, short clls, unsigned short *ma, float *mh) :Sprite (file),scrolloffset(0),tp(textp),posmodes(modes),thismode(MSG), rows(rwws), cols(clls){
  viewStyle = CP_TARGET;
  StartArmor = ma;
  maxhull = mh;
  SwitchMode();

  //  printf("\nVDU rows=%d,col=%d\n",rows,cols);
  //cout << "vdu" << endl;
};

void VDU::DrawTargetSpr (Sprite *s, float per, float &sx, float &sy, float &w, float &h) {
  float nw,nh;
  GetPosition (sx,sy);
  GetSize (w,h);
  h=-fabs (h*per);

  if (!s) {
    h=-h;
    w = fabs (w*per);
    return;
  }
  s->SetPosition (sx,sy);
  s->GetSize (nw,nh);
  w= fabs(nw*h/nh);
  s->SetSize (w,h);
  s->Draw();
  s->SetSize (nw,nh);
  h = fabs(h);
}

void VDU::Scroll (int howmuch) {
  scrolloffset+=howmuch;
}

static std::string MangleString (const char * in, float probability) {
  vector <char> str;

  
  

  for (int i=0;in[i]!='\0';i++) {
    if (in[i]!='\n') {
      str.push_back (in[i]);
      if (rand()<probability*RAND_MAX){
	str.back()+=rand()%12-6;
      }
      if (rand()<.1*probability*RAND_MAX) {
	str.push_back ('a'+rand()%26);
      }
    } else {
      if (rand()<.1*probability*RAND_MAX) {
	while (rand()%5) {
	  str.push_back ('a'+rand()%26);
	}
      }
      str.push_back (in[i]);
    }
  }
  char * tmp = (char *)malloc (sizeof (char)*str.size()+1);
  tmp[str.size()]='\0';
  for (unsigned int kk=0;kk<str.size();kk++) {
    tmp[kk]=str[kk];
  }
  std::string retval = string (tmp);
  free (tmp);
  return retval;
}
static void DrawShield (float fs, float rs, float ls, float bs, float x, float y, float h, float w) { //FIXME why is this static?
  GFXBegin (GFXLINE);
  if (fs>.2) {
    GFXVertex3f (x-w/8,y+h/2,0);
    GFXVertex3f (x-w/3,y+.9*h/2,0);
    GFXVertex3f (x+w/8,y+h/2,0);
    GFXVertex3f (x+w/3,y+.9*h/2,0);
    GFXVertex3f (x+w/8,y+h/2,0);
    GFXVertex3f (x-w/8,y+h/2,0);
  }
  if (fs>.5) {
    GFXVertex3f (x-w/8,y+1.1*h/2,0);
    GFXVertex3f (x+w/8,y+1.1*h/2,0);
    GFXVertex3f (x-w/8,y+1.1*h/2,0);
    GFXVertex3f (x-w/3,y+h/2,0);
    GFXVertex3f (x+w/8,y+1.1*h/2,0);
    GFXVertex3f (x+w/3,y+h/2,0);
  }
  if (fs>.75) {
    GFXVertex3f (x-w/8,y+1.2*h/2,0);
    GFXVertex3f (x+w/8,y+1.2*h/2,0);
    GFXVertex3f (x-w/8,y+1.2*h/2,0);
    GFXVertex3f (x-w/3,y+1.1*h/2,0);
    GFXVertex3f (x+w/8,y+1.2*h/2,0);
    GFXVertex3f (x+w/3,y+1.1*h/2,0);
  }
  if (rs>.2) {
    GFXVertex3f (x+1*w/2,y-h/8,0);
    GFXVertex3f (x+.9*w/2,y-h/3,0);
    GFXVertex3f (x+1*w/2,y+h/8,0);
    GFXVertex3f (x+1*w/2,y-h/8,0);
    GFXVertex3f (x+.9*w/2,y+h/3,0);
    GFXVertex3f (x+1*w/2,y+h/8,0);
  }
  if (rs>.5) {
    GFXVertex3f (x+1.1*w/2,y-h/8,0);
    GFXVertex3f (x+1*w/2,y-h/3,0);
    GFXVertex3f (x+1.1*w/2,y+h/8,0);
    GFXVertex3f (x+1.1*w/2,y-h/8,0);
    GFXVertex3f (x+1*w/2,y+h/3,0);
    GFXVertex3f (x+1.1*w/2,y+h/8,0);
  }
  if (rs>.7) {
    GFXVertex3f (x+1.2*w/2,y-h/8,0);
    GFXVertex3f (x+1.1*w/2,y-h/3,0);
    GFXVertex3f (x+1.2*w/2,y+h/8,0);
    GFXVertex3f (x+1.2*w/2,y-h/8,0);
    GFXVertex3f (x+1.1*w/2,y+h/3,0);
    GFXVertex3f (x+1.2*w/2,y+h/8,0);
  }
  if (ls>.2) {
    GFXVertex3f (x-1*w/2,y-h/8,0);
    GFXVertex3f (x-.9*w/2,y-h/3,0);
    GFXVertex3f (x-1*w/2,y+h/8,0);
    GFXVertex3f (x-1*w/2,y-h/8,0);
    GFXVertex3f (x-.9*w/2,y+h/3,0);
    GFXVertex3f (x-1*w/2,y+h/8,0);
  }
  if (ls>.5) {
    GFXVertex3f (x-1.1*w/2,y-h/8,0);
    GFXVertex3f (x-1*w/2,y-h/3,0);
    GFXVertex3f (x-1.1*w/2,y+h/8,0);
    GFXVertex3f (x-1.1*w/2,y-h/8,0);
    GFXVertex3f (x-1*w/2,y+h/3,0);
    GFXVertex3f (x-1.1*w/2,y+h/8,0);
  }
  if (ls>.7) {
    GFXVertex3f (x-1.2*w/2,y-h/8,0);
    GFXVertex3f (x-1.1*w/2,y-h/3,0);
    GFXVertex3f (x-1.2*w/2,y+h/8,0);
    GFXVertex3f (x-1.2*w/2,y-h/8,0);
    GFXVertex3f (x-1.1*w/2,y+h/3,0);
    GFXVertex3f (x-1.2*w/2,y+h/8,0);
  }
  if (bs>.2) {
    GFXVertex3f (x-w/8,y-h/2,0);
    GFXVertex3f (x-w/3,y-.9*h/2,0);
    GFXVertex3f (x+w/8,y-h/2,0);
    GFXVertex3f (x+w/3,y-.9*h/2,0);
    GFXVertex3f (x+w/8,y-h/2,0);
    GFXVertex3f (x-w/8,y-h/2,0);
  }
  if (bs>.5) {
    GFXVertex3f (x-w/8,y-1.1*h/2,0);
    GFXVertex3f (x+w/8,y-1.1*h/2,0);
    GFXVertex3f (x-w/8,y-1.1*h/2,0);
    GFXVertex3f (x-w/3,y-h/2,0);
    GFXVertex3f (x+w/8,y-1.1*h/2,0);
    GFXVertex3f (x+w/3,y-h/2,0);
  }
  if (bs>.75) {
    GFXVertex3f (x-w/8,y-1.2*h/2,0);
    GFXVertex3f (x+w/8,y-1.2*h/2,0);
    GFXVertex3f (x-w/8,y-1.2*h/2,0);
    GFXVertex3f (x-w/3,y-1.1*h/2,0);
    GFXVertex3f (x+w/8,y-1.2*h/2,0);
    GFXVertex3f (x+w/3,y-1.1*h/2,0);
  }

  GFXEnd();

}

void VDU::DrawVDUShield (Unit * parent) {
  float fs = parent->FShieldData();
  float rs = parent->RShieldData();
  float ls = parent->LShieldData();
  float bs = parent->BShieldData();
  float x,y,w,h;
  GetPosition (x,y);
  GetSize (w,h);
  h=fabs (h*.6);
  w=fabs (w*.6);
  unsigned short armor[4];
  GFXColor4f (.4,.4,1,1);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  DrawShield (fs,rs,ls,bs,x,y,h,w);
  parent->ArmorData (armor);
  GFXColor4f (1,.6,0,1);
  DrawShield (armor[0]/(float)StartArmor[0],armor[2]/(float)StartArmor[2],armor[3]/(float)StartArmor[3],armor[1]/(float)StartArmor[1],x,y,h/2,w/2);
  GFXColor4f (1,1,1,1);
}
void VDU::DrawTarget(Unit * parent, Unit * target) {
  float x,y,w,h;
  float fs = target->FShieldData();
  float rs = target->RShieldData();
  float ls = target->LShieldData();
  float bs = target->BShieldData();
    
  //sprintf (t,"\n%4.1f %4.1f",target->FShieldData()*100,target->RShieldData()*100);


  char st[256];
  //  sprintf (st,"\n%s",target->name.c_str());
  sprintf (st,"\n%s:%s",target->getFgID().c_str(),target->name.c_str());

  int k = strlen (st);
  if (k>cols)
    k=cols;
  for (int i=0;i<rows-1&&i+k<128;i++) {
    st[i+k]='\n';
    st[i+k+1]='\0';
  }
  char qr[128];
  sprintf (qr,"Dis %.4f",(parent->Position()-target->Position()).Magnitude()*((target->isUnit()==PLANETPTR)?10:1));
  strcat (st,qr);
  tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0);  
  DrawTargetSpr (target->getHudImage (),.6,x,y,h,w);
  GFXColor4f (.4,.4,1,1);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  DrawShield (fs,rs,ls,bs,x,y,w,h);
  GFXColor4f (1,1,1,1);
}

void VDU::DrawMessages(Unit *target){
  string fullstr;

  char st[256];
  //  sprintf (st,"\n%s",target->name.c_str());
    double nowtime=mission->getGametime();
  if(target){
    string ainame;
    if(target->getFlightgroup()){
      ainame=target->getFlightgroup()->ainame;
    }
    else{
      ainame="unknown";
    }

    int nowtime_mins=(int)(nowtime/60.0);
    int nowtime_secs=(int)(nowtime - nowtime_mins*60);
    sprintf (st,"%s:%s:%s:%2d.%02d",target->getFgID().c_str(),target->name.c_str(),ainame.c_str(),nowtime_mins,nowtime_secs);
  }
  else{
    sprintf(st,"no target");
  }
  string targetstr=string(st)+"\n";
  int msglen=targetstr.size();
  int rows_needed=msglen/cols;

  MessageCenter *mc=mission->msgcenter;
  
  int rows_used=rows_needed+1;

  gameMessage *lastmsg=mc->last(0);
  for(int i=0;rows_used<=rows && lastmsg!=NULL;i++){
    lastmsg=mc->last(i);
    if(lastmsg!=NULL){
      char timebuf[100];
      double sendtime=lastmsg->time;
      if(sendtime<=nowtime){
	int sendtime_mins=(int)(sendtime/60.0);
	int sendtime_secs=(int)(sendtime - sendtime_mins*60);

	sprintf(timebuf,"%2d.%02d ",sendtime_mins,sendtime_secs);

	string mymsg=timebuf+lastmsg->message;
	int msglen=mymsg.size();
	int rows_needed=msglen/cols;
	fullstr=mymsg+"\n"+fullstr;
	//fullstr=fullstr+mymsg+"\n";

	rows_used+=rows_needed+1;
	//      cout << "nav  " << mymsg << " rows " << rows_needed << endl;
      }
    }
  }

  fullstr=targetstr+fullstr;

  tp->Draw(MangleString (fullstr.c_str(),_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),scrolloffset);
}

void VDU::DrawNav (const Vector & nav) {

  char navdata[256];
  sprintf (navdata,"\nNavigation\n----------\nRelativeLocation\nx: %.4f\ny:%.4f\nz:%.4f\nDistance:\n%f",nav.i,nav.j,nav.k,10*nav.Magnitude());
  tp->Draw (MangleString (navdata,_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),scrolloffset);  


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
  float th;
  char st[256];

  Unit * thr = parent->Threat();
  sprintf (st,"\nHull: %.3f",parent->GetHull());
  if (thr) {
    int k=strlen(st);
    if (k>cols)
      k=cols;
    for (int i=0;i<rows-2&&i+k<128;i++) {
      st[i+k]='\n';
      st[i+k+1]='\0';
    }
    char qr[256];
    sprintf (qr, "%6s\nThreat:%4.4f",thr->name.c_str(),thr->cosAngleTo (parent,th,100000000,10000000));
    strncat (st,qr,128);
  }
  tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.5:0),0);  
  DrawTargetSpr (parent->getHudImage (),.6,x,y,w,h);
  
}
void VDU::SetViewingStyle(VIEWSTYLE vs) {
  viewStyle = vs;
}
void DrawStarSystemAgain (float x,float y,float w,float h, VIEWSTYLE viewStyle) {
  VIEWSTYLE which=viewStyle;
  _Universe->AccessCamera(which)->SetSubwindow (x,y,w,h);
  _Universe->SelectCamera(which);
  VIEWSTYLE tmp = _Universe->AccessCockpit()->GetView ();
  _Universe->AccessCockpit()->SetView (viewStyle);
  _Universe->AccessCockpit()->SelectProperCamera();
   _Universe->AccessCockpit()->SetupViewPort(true);///this is the final, smoothly calculated cam
  GFXClear (GFXTRUE);
  _Universe->activeStarSystem()->Draw(false);
  _Universe->AccessCamera (which)->SetSubwindow (0,0,1,1);
  _Universe->AccessCockpit()->SetView (tmp);
  _Universe->AccessCockpit()->SelectProperCamera();
   _Universe->AccessCockpit()->SetupViewPort(true);///this is the final, smoothly calculated cam
  GFXLoadIdentity(MODEL);
  GFXLoadIdentityView();
  // _Universe->AccessCockpit()->RestoreViewPort();
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
  Unit * targ;
  h=fabs(h/2);  w=fabs (w/2);
  tp->SetPos (x-w,y+h);
  tp->SetSize (x+w,y-h-.9*fabs(w/cols));
  targ = parent->GetComputerData().target.GetUnit();
  switch (thismode) {
  case TARGET:
    if (targ)
      DrawTarget(parent,targ);
    break;
  case VIEW:
    GetPosition (x,y);
    GetSize (w,h);
    DrawStarSystemAgain (.5*(x-fabs(w/2)+1),.5*((y-fabs(h/2))+1),fabs(w/2),fabs(h/2),viewStyle);
    break;
  case NAV:
    DrawNav(parent->ToLocalCoordinates (parent->GetComputerData().NavPoint-parent->Position()));
    break;
  case MSG:
    DrawMessages(targ);
    break;
  case DAMAGE:
    DrawDamage(parent);
    break;
  case WEAPON:
    DrawWeapon(parent);
    break;
  case SHIELD:
    DrawVDUShield (parent);
    break;
  }

}
void UpdateViewstyle (VIEWSTYLE &vs) {
  switch (vs) {
  case CP_FRONT:
    vs = CP_TARGET;
    break;
  case CP_BACK:
    vs = CP_FRONT;
    break;
  case CP_LEFT:
    vs=CP_BACK;
    break;
  case CP_RIGHT:
    vs=CP_LEFT;
    break;
  case CP_CHASE:
    vs = CP_RIGHT;
    break;
  case CP_PAN:
    vs = CP_CHASE;
    break;
  case CP_PANTARGET:
    vs=CP_PAN;
    break;
  case CP_TARGET:
    vs=CP_PANTARGET;
    break;
  }
}
void VDU::SwitchMode() {
  if (thismode==VIEW&&viewStyle!=CP_BACK&&(thismode&posmodes)) {
    UpdateViewstyle (viewStyle);
  }else {
    viewStyle = CP_TARGET;
    thismode<<=1;
    while (!(thismode&posmodes)) {
      if (thismode>posmodes) {
	thismode=0x1;
      } else {
	thismode<<=1;
      }
    }
  }
}
