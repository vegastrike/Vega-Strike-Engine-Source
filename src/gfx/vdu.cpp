#include "vdu.h"
#include "cmd/unit_generic.h"
#include "hud.h"
#include "vs_globals.h"
#include "cockpit.h"
#include "cmd/script/mission.h"
#include "cmd/script/flightgroup.h"
#include "cmd/script/msgcenter.h"
#include "cmd/images.h"
#include "cmd/planet.h"
#include "config_xml.h"
#include "xml_support.h"
#include "gfx/animation.h"
#include "gfx/vsimage.h"

///ALERT to change must change enum in class
const std::string vdu_modes [] = {"Target","Nav","Objectives","Comm","Weapon","Damage","Shield", "Manifest", "TargetManifest","View","Message"};
string reformatName (string nam) {
	nam = nam.substr(0,nam.find("."));
	if (nam.length())
		nam[0]=toupper(nam[0]);
	return nam;
}
string getUnitNameAndFgNoBase (Unit * target) {
  Flightgroup* fg = target->getFlightgroup();
  if (target->isUnit()==PLANETPTR) {
    string hr = ((Planet *)target)->getHumanReadablePlanetType();
    if (!hr.empty()) {
      return hr+string(":")+reformatName(target->name);
    }
  }else if (target->isUnit()==UNITPTR){
	if (fg) {
	  if (fg->name!="Base"&&fg->name!="Asteroid"&&fg->name!="Nebula") {
		  return fg->name+":"+target->getFullname();
	  } else if(fg->name=="Base"){
		  return reformatName(target->name)+":"+target->getFullname();
	  }
	}	 
  }
  
  if (string("neutral")!=FactionUtil::GetFaction(target->faction)) {
    return /*string(_Universe->GetFaction(target->faction))+" "+*/reformatName(target->name);
  }
  return reformatName(target->name);
}



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




VDU::VDU (const char * file, TextPlane *textp, unsigned short modes, short rwws, short clls, float *ma, float *mh) :VSSprite (file),tp(textp),posmodes(modes), rows(rwws), cols(clls),scrolloffset(0){
  thismode.push_back(MSG);
  if (_Universe->numPlayers()>1) {
    posmodes&=(~VIEW);
  }
  comm_ani=NULL;
  viewStyle = CP_TARGET;
  StartArmor = ma;
  maxhull = mh;
  if( Network!=NULL)
    got_target_info = false;
  else
    got_target_info = true;
  SwitchMode( NULL);

  //  printf("\nVDU rows=%d,col=%d\n",rows,cols);
  //cout << "vdu" << endl;
};

void VDU::DrawTargetSpr (VSSprite *s, float per, float &sx, float &sy, float &w, float &h) {
  float nw,nh;
  static bool HighQTargetVSSprites = XMLSupport::parse_bool(vs_config->getVariable("graphics","high_quality_sprites","false"));
  static bool drawweapsprite = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_weapon_sprite","false"));

  GetPosition (sx,sy);
  GetSize (w,h);

  // Use margins specified from config file
  static float width_factor=XMLSupport::parse_float (vs_config->getVariable("graphics","reduced_vdus_width","0"));
  static float height_factor=XMLSupport::parse_float (vs_config->getVariable("graphics","reduced_vdus_height","0"));
  w = w-width_factor;
  h = h+height_factor;

  h=-fabs (h*per);

  if (!s) {
    h=-h;
    w = fabs (w*per);
  }else {
	  if (HighQTargetVSSprites) {
		  GFXBlendMode (SRCALPHA,INVSRCALPHA);
	  }else {
		  GFXBlendMode (ONE,ZERO);
		  GFXAlphaTest(GREATER,.4);
	  }	  
	  s->SetPosition (sx,sy);
	  s->GetSize (nw,nh);
	  w= fabs(nw*h/nh);
	  s->SetSize (w,h);
	  if (drawweapsprite||1) 
		  s->Draw();
	  s->SetSize (nw,nh);
	  h = fabs(h);
	  if (HighQTargetVSSprites) {
		  GFXBlendMode (ONE,ZERO);
	  }else {
		  GFXAlphaTest(ALWAYS,0);
	  }
  }

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
static void DrawShield (float fs, float rs, float ls, float bs, float x, float y, float w, float h) { //FIXME why is this static?
  GFXBegin (GFXLINE);
  if (fs>.2) {
    GFXVertex3d ((double)x-w/8,y+h/2,0.);
    GFXVertex3d ((double)x-w/3,y+.9*h/2,0.);
    GFXVertex3d ((double)x+w/8,y+h/2,0.);
    GFXVertex3d ((double)x+w/3,y+.9*h/2,0.);
    GFXVertex3d ((double)x+w/8,y+h/2,0.);
    GFXVertex3d ((double)x-w/8,y+h/2,0.);
  }
  if (fs>.5) {
    GFXVertex3d ((double)x-w/8,y+1.1*h/2,0.);
    GFXVertex3d ((double)x+w/8,y+1.1*h/2,0.);
    GFXVertex3d ((double)x-w/8,y+1.1*h/2,0.);
    GFXVertex3d ((double)x-w/3,y+h/2,0.);
    GFXVertex3d ((double)x+w/8,y+1.1*h/2,0.);
    GFXVertex3d ((double)x+w/3,y+h/2,0.);
  }
  if (fs>.75) {
    GFXVertex3d ((double)x-w/8,y+1.2*h/2,0.);
    GFXVertex3d ((double)x+w/8,y+1.2*h/2,0.);
    GFXVertex3d ((double)x-w/8,y+1.2*h/2,0.);
    GFXVertex3d ((double)x-w/3,y+1.1*h/2,0.);
    GFXVertex3d ((double)x+w/8,y+1.2*h/2,0.);
    GFXVertex3d ((double)x+w/3,y+1.1*h/2,0.);
  }
  if (rs>.2) {
    GFXVertex3d ((double)x+1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x+.9*w/2,y-h/3,0.);
    GFXVertex3d ((double)x+1*w/2,y+h/8,0.);
    GFXVertex3d ((double)x+1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x+.9*w/2,y+h/3,0.);
    GFXVertex3d ((double)x+1*w/2,y+h/8,0.);
  }
  if (rs>.5) {
    GFXVertex3d ((double)x+1.1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x+1*w/2,y-h/3,0.);
    GFXVertex3d ((double)x+1.1*w/2,y+h/8,0.);
    GFXVertex3d ((double)x+1.1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x+1*w/2,y+h/3,0.);
    GFXVertex3d ((double)x+1.1*w/2,y+h/8,0.);
  }
  if (rs>.7) {
    GFXVertex3d ((double)x+1.2*w/2,y-h/8,0.);
    GFXVertex3d ((double)x+1.1*w/2,y-h/3,0.);
    GFXVertex3d ((double)x+1.2*w/2,y+h/8,0.);
    GFXVertex3d ((double)x+1.2*w/2,y-h/8,0.);
    GFXVertex3d ((double)x+1.1*w/2,y+h/3,0.);
    GFXVertex3d ((double)x+1.2*w/2,y+h/8,0.);
  }
  if (ls>.2) {
    GFXVertex3d ((double)x-1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x-.9*w/2,y-h/3,0.);
    GFXVertex3d ((double)x-1*w/2,y+h/8,0.);
    GFXVertex3d ((double)x-1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x-.9*w/2,y+h/3,0.);
    GFXVertex3d ((double)x-1*w/2,y+h/8,0.);
  }
  if (ls>.5) {
    GFXVertex3d ((double)x-1.1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x-1*w/2,y-h/3,0.);
    GFXVertex3d ((double)x-1.1*w/2,y+h/8,0.);
    GFXVertex3d ((double)x-1.1*w/2,y-h/8,0.);
    GFXVertex3d ((double)x-1*w/2,y+h/3,0.);
    GFXVertex3d ((double)x-1.1*w/2,y+h/8,0.);
  }
  if (ls>.7) {
    GFXVertex3d ((double)x-1.2*w/2,y-h/8,0.);
    GFXVertex3d ((double)x-1.1*w/2,y-h/3,0.);
    GFXVertex3d ((double)x-1.2*w/2,y+h/8,0.);
    GFXVertex3d ((double)x-1.2*w/2,y-h/8,0.);
    GFXVertex3d ((double)x-1.1*w/2,y+h/3,0.);
    GFXVertex3d ((double)x-1.2*w/2,y+h/8,0.);
  }
  if (bs>.2) {
    GFXVertex3d ((double)x-w/8,y-h/2,0.);
    GFXVertex3d ((double)x-w/3,y-.9*h/2,0.);
    GFXVertex3d ((double)x+w/8,y-h/2,0.);
    GFXVertex3d ((double)x+w/3,y-.9*h/2,0.);
    GFXVertex3d ((double)x+w/8,y-h/2,0.);
    GFXVertex3d ((double)x-w/8,y-h/2,0.);
  }
  if (bs>.5) {
    GFXVertex3d ((double)x-w/8,y-1.1*h/2,0.);
    GFXVertex3d ((double)x+w/8,y-1.1*h/2,0.);
    GFXVertex3d ((double)x-w/8,y-1.1*h/2,0.);
    GFXVertex3d ((double)x-w/3,y-h/2,0.);
    GFXVertex3d ((double)x+w/8,y-1.1*h/2,0.);
    GFXVertex3d ((double)x+w/3,y-h/2,0.);
  }
  if (bs>.75) {
    GFXVertex3d ((double)x-w/8,y-1.2*h/2,0.);
    GFXVertex3d ((double)x+w/8,y-1.2*h/2,0.);
    GFXVertex3d ((double)x-w/8,y-1.2*h/2,0.);
    GFXVertex3d ((double)x-w/3,y-1.1*h/2,0.);
    GFXVertex3d ((double)x+w/8,y-1.2*h/2,0.);
    GFXVertex3d ((double)x+w/3,y-1.1*h/2,0.);
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

  // Use margins specified from config file
  static float width_factor=XMLSupport::parse_float (vs_config->getVariable("graphics","reduced_vdus_width","0"));
  static float height_factor=XMLSupport::parse_float (vs_config->getVariable("graphics","reduced_vdus_height","0"));
  w = w-width_factor;
  h = h+height_factor;

  h=fabs (h*.6);
  w=fabs (w*.6);
  float armor[8];
  GFXColor4f (.4,.4,1,1);
  GFXDisable (TEXTURE0);
  DrawShield (fs,rs,ls,bs,x,y,w,h);
  parent->ArmorData (armor);
  GFXColor4f (1,.6,0,1);
  DrawShield ((armor[0]+armor[2]+armor[4]+armor[6])/(float)(StartArmor[0]+StartArmor[2]+StartArmor[4]+StartArmor[6]),(armor[0]+armor[1]+armor[4]+armor[5])/(float)(StartArmor[0]+StartArmor[1]+StartArmor[4]+StartArmor[5]),(armor[2]+armor[3]+armor[6]+armor[7])/(float)(StartArmor[2]+StartArmor[3]+StartArmor[6]+StartArmor[7]),(armor[1]+armor[3]+armor[5]+armor[7])/(float)(StartArmor[1]+StartArmor[3]+StartArmor[5]+StartArmor[7]),x,y,w/2,h/2);
  GFXColor4f (1,parent->GetHullPercent(),parent->GetHullPercent(),1);
  GFXEnable (TEXTURE0);
  GFXColor4f (1,parent->GetHullPercent(),parent->GetHullPercent(),1);
  DrawTargetSpr (parent->getHudImage (),.25,x,y,w,h);

}
VSSprite * getJumpImage () {
  static VSSprite s("jump-hud.spr");
  return &s;
}
VSSprite * getSunImage () {
  static VSSprite s("sun-hud.spr");
  return &s;
}
VSSprite * getPlanetImage () {
  static VSSprite s("planet-hud.spr");
  return &s;
}

void VDU::DrawTarget(Unit * parent, Unit * target) {
  float x,y,w,h;

  float fs = target->FShieldData();
  float rs = target->RShieldData();
  float ls = target->LShieldData();
  float bs = target->BShieldData();
  GFXEnable (TEXTURE0);
  GFXColor4f (1,target->GetHullPercent(),target->GetHullPercent(),1);

  DrawTargetSpr ((target->isUnit()!=PLANETPTR?target->getHudImage ():
		  (target->GetDestinations().size()!=0? getJumpImage():
		   (((Planet *)target)->hasLights()?getSunImage():
		    getPlanetImage()))),.6,x,y,w,h);
  GFXDisable (TEXTURE0);    
  //sprintf (t,"\n%4.1f %4.1f",target->FShieldData()*100,target->RShieldData()*100);
  double mm=0;
  string unitandfg=getUnitNameAndFgNoBase(target).c_str();
  bool inrange=parent->InRange(target,mm,true,false,false);
  if (inrange) {
	  static int neut= FactionUtil::GetFaction("neutral");
    static int upgr= FactionUtil::GetFaction("upgrades");
    if (target->faction != neut&&target->faction!=upgr) {
      unitandfg+=std::string("\n")+FactionUtil::GetFaction(target->faction);
    }
    
  }
  tp->Draw (MangleString (unitandfg.c_str(),_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);  

  if (inrange) {  
  int i=0;
  char st[1024];
  for (i=0;i<rows-1&&i<128;i++) {
    st[i]='\n';

  }
  st[i]='\0';
  char qr[256];
  static float game_speed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
  static bool lie=XMLSupport::parse_bool (vs_config->getVariable("physics","game_speed_lying","true"));
  if(lie){
	  sprintf (qr,"Distance: %.2f",((parent->Position()-target->Position()).Magnitude()-((target->isUnit()==PLANETPTR)?target->rSize():0))*10./game_speed);
  } else {
	double dist=((parent->Position()-target->Position()).Magnitude()-((target->isUnit()==PLANETPTR)?target->rSize():0));
	if(dist<100000){
		sprintf (qr,"Distance: %.0f Meters",dist);
	} else if (dist <100000000){
		sprintf (qr,"Distance: %.0f Kilometers",dist/1000);
	}else {
		sprintf (qr,"Distance: %.2f LightSeconds",dist/300000000);
	}
  }
  strcat (st,qr);
  tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);  
  GFXColor4f (.4,.4,1,1);
  DrawShield (fs,rs,ls,bs,x,y,w,h);
  GFXColor4f (1,1,1,1);
  }else {
  tp->Draw (MangleString ("\n[OutOfRange]",_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);      

  }
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
    std::string blah=getUnitNameAndFgNoBase(target);
    sprintf (st,"%s:%s:%2d.%02d",blah.c_str(),ainame.c_str(),nowtime_mins,nowtime_secs);
  }
  else{
    sprintf(st,"no target");
  }
  string targetstr=string(st)+"\n";
  int msglen=targetstr.size();
  
  int rows_needed=msglen/(cols>0?cols:1);

  MessageCenter *mc=mission->msgcenter;
  
  int rows_used=rows_needed+1;
  vector <std::string> whoNOT;
  whoNOT.push_back ("briefing");
  whoNOT.push_back ("news");
  whoNOT.push_back ("bar");

  vector <std::string> message_people;//should be "all", parent's name
  gameMessage lastmsg;
  for(int i=scrolloffset<0?-scrolloffset:0;rows_used<rows && mc->last(i,lastmsg,message_people,whoNOT);i++){
      char timebuf[100];
      double sendtime=lastmsg.time;
      if(sendtime<=nowtime){
		  int sendtime_mins=(int)(sendtime/60.0);
		  int sendtime_secs=(int)(sendtime - sendtime_mins*60);
		  
		  sprintf(timebuf,"%d.%02d",sendtime_mins,sendtime_secs);
		  string mymsg;
		  if (lastmsg.from!="game") {
			  mymsg=lastmsg.from+" ("+timebuf+"): "+lastmsg.message;
		  } else {
			  mymsg=string(timebuf)+": "+lastmsg.message;
		  }
		  int msglen=mymsg.size();
		  int rows_needed=(int)(msglen/(1.6*cols));
		  fullstr=mymsg+"\n"+fullstr;
		  //fullstr=fullstr+mymsg+"\n";
		  
		  rows_used+=rows_needed+1;
		  //      cout << "nav  " << mymsg << " rows " << rows_needed << endl;
      }
  }

  fullstr=targetstr+fullstr;
  tp->Draw(MangleString (fullstr.c_str(),_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);
}

void	VDU::DrawScanningMessage()
{
  tp->Draw(MangleString ("Scanning target...",_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);
}

bool VDU::SetCommAnimation (Animation * ani) {
  if (comm_ani==NULL) {
    if (posmodes&COMM) {
      comm_ani = ani;
      if (ani){
	      thismode.push_back(COMM);
		  ani->Reset();
	  }
	  return true;
    }
  }
  return false;
}
void VDU::DrawNav (const Vector & nav) {
  Unit * you = _Universe->AccessCockpit()->GetParent();
  Unit * targ = you!=NULL?you->Target():NULL;
  char *navdata=new char [1024+(_Universe->activeStarSystem()->getName().length()+(targ?targ->name.length():0))];
  static float game_speed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
  static bool lie=XMLSupport::parse_bool (vs_config->getVariable("physics","game_speed_lying","true"));
  string nam="none";
  if (targ)
	  nam= reformatName(targ->name);
  sprintf (navdata,"Navigation\n----------\n%s\nTarget:\n  %s\nRelativeLocation\nx: %.4f\ny:%.4f\nz:%.4f\nDistance:\n%f",_Universe->activeStarSystem()->getName().c_str(),nam.c_str(),nav.i,nav.j,nav.k,lie?(10*nav.Magnitude()/game_speed):nav.Magnitude());
  tp->Draw (MangleString (navdata,_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),scrolloffset,true,true);  
  delete [] navdata;

}
void VDU::DrawComm () {
  if (comm_ani!=NULL) {
    GFXDisable (TEXTURE1);
    GFXEnable (TEXTURE0);
    GFXDisable(LIGHTING);

    comm_ani->DrawAsVSSprite(this);
    if (comm_ani->Done()) {
      if (thismode.size()>1) {
	if (XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","switch_back_from_comms","true"))) {
	  thismode.pop_back();
	} else {
	  unsigned int blah = thismode.back();
	  thismode.pop_back();
	  thismode.back()=blah;
	}
      }
      comm_ani=NULL;
    }
    GFXDisable (TEXTURE0);


  }else {
    tp->Draw (MangleString (_Universe->AccessCockpit()->communication_choices.c_str(),_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),scrolloffset,true);  
  }
}

void VDU::DrawManifest (Unit * parent, Unit * target) {
  string retval ("Manifest\n");
  if (target!=parent) {
    retval+=string ("Tgt: ")+reformatName(target->name)+string("\n");
  }else {
    retval+=string ("--------\nCredits: ")+tostring((int)_Universe->AccessCockpit()->credits)+/*string(".")+tostring (((int)(_Universe->AccessCockpit()->credits*100))%100) +*/string("\n");
  }
  unsigned int numCargo =target->numCargo();
  for (unsigned int i=0;i<numCargo;i++) {
    retval+=target->GetManifest (i,parent,parent->GetVelocity())+string (" (")+tostring (target->GetCargo(i).quantity)+string (")\n");
  }
  tp->Draw (MangleString (retval.c_str(),_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),scrolloffset,true);  
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
    GFXVertex3d (pos.i+oox,pos.j,0);
    GFXVertex3d (pos.i+oox,pos.j-h/15,0);
    GFXVertex3d (pos.i-oox,pos.j,0);
    GFXVertex3d (pos.i-oox,pos.j-h/15,0);
    GFXVertex3d (pos.i+oox,pos.j-h/15,0);
    GFXVertex3d (pos.i-oox,pos.j-h/15,0);
    if (sz==weapon_info::LIGHT) {
      GFXVertex3d (pos.i,pos.j,0);
      GFXVertex3d (pos.i,pos.j+h/4,0);
      GFXVertex3d (pos.i,pos.j+h/4+ooy*2,0);
      GFXVertex3d (pos.i,pos.j+h/4+ooy*5,0);
    }else if (sz==weapon_info::MEDIUM) {
      GFXVertex3d (pos.i,pos.j,0);
      GFXVertex3d (pos.i,pos.j+h/5,0);
      GFXVertex3d (pos.i,pos.j+h/5+ooy*4,0);
      GFXVertex3d (pos.i,pos.j+h/5+ooy*5,0);
      GFXVertex3d (pos.i+oox,pos.j+h/5+ooy*2,0);      
      GFXVertex3d (pos.i-oox,pos.j+h/5+ooy*2,0);      
    }else if (sz==weapon_info::HEAVY) {
      GFXVertex3d (pos.i,pos.j,0);
      GFXVertex3d (pos.i,pos.j+h/5,0);
      GFXVertex3d (pos.i,pos.j+h/5+ooy*4,0);
      GFXVertex3d (pos.i,pos.j+h/5+ooy*5,0);
      GFXVertex3d (pos.i+2*oox,pos.j+h/5+ooy*3,0);      
      GFXVertex3d (pos.i,pos.j+h/5+ooy*2,0);      
      GFXVertex3d (pos.i-2*oox,pos.j+h/5+ooy*3,0);      
      GFXVertex3d (pos.i,pos.j+h/5+ooy*2,0);      
    }else {//capship gun
      GFXVertex3d (pos.i,pos.j,0);
      GFXVertex3d (pos.i,pos.j+h/6,0);
      GFXVertex3d (pos.i,pos.j+h/6+ooy*6,0);
      GFXVertex3d (pos.i,pos.j+h/6+ooy*7,0);
      GFXVertex3d (pos.i-oox,pos.j+h/6+ooy*2,0);
      GFXVertex3d (pos.i+oox,pos.j+h/6+ooy*2,0);
      GFXVertex3d (pos.i-2*oox,pos.j+h/6+ooy*4,0);
      GFXVertex3d (pos.i+2*oox,pos.j+h/6+ooy*4,0);
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
    GFXVertex3d (pos.i,pos.j-h/8,0);
    GFXVertex3d (pos.i,pos.j+h/8,0);
    GFXVertex3d (pos.i+2*oox,pos.j-h/8+2*ooy,0);
    GFXVertex3d (pos.i-2*oox,pos.j-h/8+2*ooy,0);
    GFXEnd();
  }else if (sz<=weapon_info::CAPSHIPHEAVYMISSILE) {
    GFXBegin (GFXLINE);
    GFXVertex3d (pos.i,pos.j-h/6,0);
    GFXVertex3d (pos.i,pos.j+h/6,0);
    GFXVertex3d (pos.i+3*oox,pos.j-h/6+2*ooy,0);
    GFXVertex3d (pos.i-3*oox,pos.j-h/6+2*ooy,0);
    GFXVertex3d (pos.i+oox,pos.j-h/6,0);
    GFXVertex3d (pos.i+oox,pos.j+h/9,0);
    GFXVertex3d (pos.i-oox,pos.j-h/6,0);
    GFXVertex3d (pos.i-oox,pos.j+h/9,0);
    GFXEnd();
  }
  
}
void VDU::DrawDamage(Unit * parent) {
  float x,y,w,h;
  float th;
  char st[1024];
  GFXColor4f (1,parent->GetHull()/ (*maxhull),parent->GetHull()/(*maxhull),1);
  GFXEnable (TEXTURE0);
  DrawTargetSpr (parent->getHudImage (),.6,x,y,w,h);
  GFXDisable(TEXTURE0);
  Unit * thr = parent->Threat();
  std::string blah (getUnitNameAndFgNoBase(parent));
  sprintf (st,"%s\nHull: %.3f",blah.c_str(),parent->GetHull());
  tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.5:0),0,true);  
  int k=strlen(st);
  if (k>cols)
    k=cols;
  
  for (int i=0;i<rows-3&&i+k<128;i++) {
    st[i+k]='\n';
    st[i+k+1]='\0';
  }
  char ecmstatus[256];
  ecmstatus[0]='\0';
  if (parent->GetImageInformation().ecm>0) {
    GFXColor4f(0,1,0,.5);
    strcpy (ecmstatus,"ECM Active");
    
  }
  if (((parent->GetImageInformation().ecm<0))) {
    GFXColor4f(.6,.6,.6,.5);
    strcpy (ecmstatus,"ECM Inactive");
  }
  if (parent->GetImageInformation().ecm>0) {
    static float s=0;
    s+=.125*SIMULATION_ATOM;
    if (s>1)
      s=0;
    DrawShield (0, s, s, 0, x, y, w,h);
  }
  char qr[256];
  if (thr) {
    GFXColor4f (1,0,0,1);
	string nam =reformatName(thr->name);
    sprintf (qr, "%s\n%6s\nThreat:%4.4f",ecmstatus,nam.c_str(),thr->cosAngleTo (parent,th,100000000,10000000));
    strncat (st,qr,128);
  }else {
    if (parent->GetImageInformation().ecm!=0) {
      sprintf (qr, "%s\n",ecmstatus);
      strncat (st,qr,128);
    }
  }
  tp->Draw (MangleString (st+k,_Universe->AccessCamera()->GetNebula()!=NULL?.5:0),0,true);  
  GFXColor4f (1,1,1,1);
  
}
void VDU::SetViewingStyle(VIEWSTYLE vs) {
  viewStyle = vs;
}
void VDU::DrawStarSystemAgain (float x,float y,float w,float h, VIEWSTYLE viewStyle,Unit *parent,Unit *target) {
#ifdef CAR_SIM
  viewStyle=CP_BACK;
#endif

  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);
  VIEWSTYLE which=viewStyle;
  _Universe->AccessCamera(which)->SetSubwindow (x,y,w,h);
  _Universe->SelectCamera(which);
  VIEWSTYLE tmp = _Universe->AccessCockpit()->GetView ();
  _Universe->AccessCockpit()->SetView (viewStyle);
  _Universe->AccessCockpit()->SelectProperCamera();
   _Universe->AccessCockpit()->SetupViewPort(true);///this is the final, smoothly calculated cam
  GFXClear (GFXFALSE);
  GFXColor4f(1,1,1,1);
  _Universe->activeStarSystem()->Draw(false);
  _Universe->AccessCamera (which)->SetSubwindow (0,0,1,1);
  _Universe->AccessCockpit()->SetView (tmp);
  _Universe->AccessCockpit()->SelectProperCamera();
   _Universe->AccessCockpit()->SetupViewPort(true);///this is the final, smoothly calculated cam
  GFXRestoreHudMode();

  GFXBlendMode (ONE,ZERO);
  GFXDisable(TEXTURE1);
  GFXDisable(TEXTURE0);
  GFXDisable(DEPTHTEST);   
  GFXDisable(DEPTHWRITE);   
#ifndef CAR_SIM
  char buf[1024];
  if (target) {
    std::string blah(getUnitNameAndFgNoBase(target));
    sprintf(buf,"%s\n",blah.c_str());
  } else {
    sprintf (buf,"This is a test of the emergencyBroadcastSystem\n");
  }

  tp->Draw(buf,0,true);
#endif
  // _Universe->AccessCockpit()->RestoreViewPort();
}


void VDU::DrawWeapon (Unit * parent) {
  static bool drawweapsprite = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_weapon_sprite","false"));

  float x,y,w,h;
  const float percent = .6;
  string buf("#ff0000Guns:#000000");
  int len= buf.length();
  string mbuf("\n#ff0000Missiles:#000000");
  int mlen = mbuf.length();
  int count=1;int mcount=1;
  GFXColor4f (1,1,1,1);
  GFXEnable(TEXTURE0);
  DrawTargetSpr (drawweapsprite?parent->getHudImage ():NULL,percent,x,y,w,h);
  GFXDisable (TEXTURE0);
  GFXDisable(LIGHTING);
  for (int i=0;i<parent->GetNumMounts();i++) {
    Vector pos (parent->mounts[i].GetMountLocation());
    pos.i=-
pos.i*fabs(w)/parent->rSize()*percent+x;
    pos.j=pos.k*fabs(h)/parent->rSize()*percent+y;
    pos.k=0;
    string ammo =(parent->mounts[i].ammo>=0)?string("(")+tostring(parent->mounts[i].ammo)+string(")"):string("");
    switch (parent->mounts[i].ammo!=0?parent->mounts[i].status:127) {
    case Mount::ACTIVE:
      GFXColor4f (0,1,.2,1);
      if (parent->mounts[i].type->size<weapon_info::LIGHTMISSILE) 
	buf+=((buf.length()==(unsigned int)len)?string(""):string(","))+((count++%1==0)?"\n":"")+parent->mounts[i].type->weapon_name+ammo;
      else
	mbuf+=((mbuf.length()==(unsigned int)mlen)?string(""):string(","))+((mcount++%1==0)?"\n":"")+parent->mounts[i].type->weapon_name+ammo;;
      break;
    case Mount::INACTIVE:
      GFXColor4f (0,.5,0,1);
      break;
    case Mount::DESTROYED:
      GFXColor4f (.2,.2,.2,1);
      break;
    case Mount::UNCHOSEN:
      GFXColor4f (1,1,1,1);
      break;
    case 127:
      GFXColor4f (0,.2,0,1);
      break;
    }
    if (drawweapsprite) {

      DrawGun (pos,w,h,parent->mounts[i].type->size);
	}
  }
  GFXColor4f(0,1,.2,1);
  if (mbuf.length()!=(unsigned int)mlen) {
    buf+=mbuf;
  }
  tp->Draw (buf,0,true);
}
using std::vector;
/*
static GFXColor GetColorFromSuccess (float suc){ 
  suc +=1.;
  suc/=2.;
  return GFXColor(1-suc,suc,0);    
}
*/

//

char printHex (unsigned int hex) {
	if (hex<10) {
		return hex+'0';
	}
	return hex-10+'A';
}
static char suc_col_str [8]="#000000";
inline char *GetColorFromSuccess (float suc) {
  if (suc>=1)
    return "#00FF00";
  if (suc<=-1)
    return "#FF0000";
  suc +=1.;
  suc*=128;
  unsigned int tmp2 = suc;
  unsigned int tmp1 = 255-suc;
  suc_col_str[0]='#';
  suc_col_str[1]=printHex(tmp1/16);
  suc_col_str[2]=printHex(tmp1%16);
  suc_col_str[3]=printHex(tmp2/16);
  suc_col_str[4]=printHex(tmp2%16);
  suc_col_str[5]='0';
  suc_col_str[6]='0';
  
  return suc_col_str;
}

#if 0
int VDU::DrawVDUObjective (void * obj, int offset) {
  static bool VDU_DrawVDUObjective_is_now_outdated=false;
  assert(VDU_DrawVDUObjective_is_now_outdated==true);
  return 0;
}
#endif

void VDU::DrawVDUObjectives (Unit *parent) {
  std::string rez;
  int offset = scrolloffset;
  for (unsigned int i=0;i<active_missions.size();++i){
    if (!active_missions[i]->objectives.empty()) {
	rez+="#FFFFFF";
	if (active_missions[i]->mission_name.empty()) {
	  rez+="Mission "+XMLSupport::tostring((int)i)+"\n";
	}else {
	  rez+=active_missions[i]->mission_name+"\n";
	}
      vector<Mission::Objective>::iterator j=active_missions[i]->objectives.begin();
      for (;j!=active_missions[i]->objectives.end();++j) {
	if (j->owner==NULL||j->owner==parent) {
	  rez+=GetColorFromSuccess((*j).completeness);
	  rez+=(*j).objective;
	  rez+='\n';
	}
      }
	  rez+='\n';
    }
  }
  tp->Draw(rez,offset);
}

bool VDU::SetWebcamAnimation ( ) {
  if (comm_ani==NULL) {
    if (posmodes&WEBCAM) {
      comm_ani = new Animation();
	  thismode.push_back(WEBCAM);
	  comm_ani->Reset();
	  return true;
    }
  }
  return false;
}
void VDU::DrawWebcam( Unit * parent)
{
	int length;
	char * netcam;
	int playernum = _Universe->whichPlayerStarship( parent);
	if( Network[playernum].hasWebcam())
	{
		netcam = Network[playernum].getWebcamFromNetwork( length);
		if( netcam)
		{
			// Delete the previous displayed webcam shot if any
			if( this->webcam)
				delete webcam;
			// Read the new one
		    VSFile f( netcam, length, JPEGBuffer);
			this->webcam = new Animation( &f);
			delete netcam;
		    GFXDisable (TEXTURE1);
		    GFXEnable (TEXTURE0);
		    GFXDisable(LIGHTING);
			// Draw it
		    webcam->DrawAsVSSprite(this);
 		    GFXDisable (TEXTURE0);
		}
	}
	else
	{
	    tp->Draw (MangleString ("No webcam to view",_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),scrolloffset,true);  
	}
}

void VDU::Draw (Unit * parent, const GFXColor & color) {
  tp->col=color;
  GFXDisable(LIGHTING);
      GFXBlendMode(SRCALPHA,INVSRCALPHA);
      GFXEnable(TEXTURE0);
      GFXDisable(TEXTURE1);
  VSSprite::Draw();
  //glDisable( GL_ALPHA_TEST);
  if (!parent) {
    return;
  }
  //configure text plane;
  float x,y;
  float h,w;
  GetSize (w,h);

  static float width_factor=XMLSupport::parse_float (vs_config->getVariable("graphics","reduced_vdus_width","0"));
  static float height_factor=XMLSupport::parse_float (vs_config->getVariable("graphics","reduced_vdus_height","0"));
  w = w-width_factor;
  h = h+height_factor;

  GetPosition (x,y);
  //tp->SetCharSize (fabs(w/cols),fabs(h/rows));
  float csx,csy;
  tp->GetCharSize(csx,csy);
  cols=abs((int)ceil(w/csx));
  rows=abs((int)ceil(h/csy));

  Unit * targ;
  h=fabs(h/2);  w=fabs (w/2);
  tp->SetPos (x-w,y+h);
  tp->SetSize (x+w,y-h-.5*fabs(w/cols));
  targ = parent->GetComputerData().target.GetUnit();
  switch (thismode.back()) {
  case NETWORK:
  {
  	if( Network!=NULL)
	{
		int playernum = _Universe->whichPlayerStarship( parent);
		char buf[32];
		string str( "Lag: ");
		unsigned int lag = Network[playernum].getLag();
		memset( buf, 0, 32);
		sprintf( buf, "%.1Lf", (long double)lag);
		if( lag<50)
			str += "#00FF00";
		else if( lag<150)
			str += "#FFFF00";
		else if( lag>0)
			str += "#FF0000";
		str += string( buf)+"#000000 ms\n";
		if( Network[playernum].IsNetcommSecured())
			str += "#DD0000";
		memset( buf, 0, 32);
		sprintf( buf, "%g", Network[playernum].getCurrentFrequency());
		str += string( buf)+"/";
		memset( buf, 0, 32);
		sprintf( buf, "%g", Network[playernum].getSelectedFrequency());
		str += string( buf)+" GHz";
		if( Network[playernum].IsNetcommSecured())
			str += "#000000";
		if( Network[playernum].IsNetcommActive())
			str += " - #0000FFON";
		else
			str += " - #FF0000OFF";
		str += "#000000\n";
		str += "SD: "+_Universe->current_stardate.GetFullTrekDate();
  		tp->Draw(MangleString (str.c_str(),_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);
	}
  }
  break;
  case WEBCAM:
  {
  	if( Network!=NULL)
	{
		DrawWebcam( parent);
	}
  }
  case SCANNING:
		if( !got_target_info)
			DrawScanningMessage();
		/*
		else
			DrawScanner();
		*/
	break;
  case TARGET:
    if (targ)
   	    DrawTarget(parent,targ);
    break;
  case MANIFEST:
    DrawManifest (parent,parent);
    break;
  case TARGETMANIFEST:
    if (targ)
      DrawManifest(parent,targ);
    break;
  case VIEW:
    GetPosition (x,y);
    GetSize (w,h);
    DrawStarSystemAgain (.5*(x-fabs(w/2)+1),.5*((y-fabs(h/2))+1),fabs(w/2),fabs(h/2),viewStyle,parent,targ);
    break;
  case NAV:
    DrawNav(parent->ToLocalCoordinates (parent->GetComputerData().NavPoint-parent->Position().Cast()));
    break;
  case MSG:
    DrawMessages(targ);
    break;
  case COMM:
    DrawComm();
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
  case OBJECTIVES:
    DrawVDUObjectives (parent);
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
    vs = CP_TARGET;
    break;
  case CP_PAN:
    vs = CP_CHASE;
    break;
  case CP_PANTARGET:
    vs=CP_CHASE;
    break;
  case CP_TARGET:
    vs=CP_PANTARGET;
    break;
  }
}
void VDU::SwitchMode( Unit * parent) {
  if (!posmodes)
    return;
  // If we switch from SCANNING VDU VIEW_MODE we loose target info
  if( thismode.back()==SCANNING && Network!=NULL)
		got_target_info = false;
  // If we switch from WEBCAM VDU VIEW_MODE we stop dlding images
  if( thismode.back()==WEBCAM && Network!=NULL && parent!=NULL)
  {
		int playernum = _Universe->whichPlayerStarship( parent);
		Network[playernum].stopWebcamTransfer();
  }
  if (thismode.back()==VIEW&&viewStyle!=CP_CHASE&&(thismode.back()&posmodes)) {
    UpdateViewstyle (viewStyle);
  }else {
    viewStyle = CP_TARGET;
    thismode.back()<<=1;
    while (!(thismode.back()&posmodes)) {
      if (thismode.back()>posmodes) {
	thismode.back()=0x1;
      } else {
	thismode.back()<<=1;
      }
    }
  }
  // If we switch to SCANNING MODE we consider we lost target info
  if( thismode.back()==SCANNING && Network!=NULL)
		got_target_info = false;
  // If we switch to WEBCAM MODE we start dlding images
  if( thismode.back()==WEBCAM && Network!=NULL && parent!=NULL)
  {
		int playernum = _Universe->whichPlayerStarship( parent);
		Network[playernum].startWebcamTransfer();
  }
}
