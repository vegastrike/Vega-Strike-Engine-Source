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
bool VDU::staticable() {
  return (posmodes&(posmodes-1))!=0;// check not power of two
}
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
		  char* fgnum;
		  vector<char> stack;
		  int tempint=target->getFgSubnumber();
		  do{
			  stack.push_back((char)((tempint%10)+48));
			  tempint/=10;
		  }while(tempint);
		  fgnum=(char*)malloc(sizeof(char)*(stack.size()+1));
		  fgnum[stack.size()]=0;
		  int offset=0;
		  int end=stack.size()-1;
		  while(end-offset>=0){
			fgnum[offset]=(char)(stack[end-offset]);
			offset++;
          }
		  string fgnstring=string(fgnum);
		  free(fgnum);
		  fgnum=NULL;
		  static bool confignums=XMLSupport::parse_bool (vs_config->getVariable ("graphics","printFGsubID","false"));
		  if(confignums){
			return fg->name+" ="+fgnstring+"= : "+reformatName(target->getFullname());
		  } else {
			return fg->name+" : "+reformatName(target->getFullname());
		  }
	  } else if(fg->name=="Base"){
		  if(reformatName(target->name)==(reformatName(target->getFullname()))){
		    return reformatName(target->name);
		  } else {
		    return reformatName(target->name)+":"+target->getFullname();
		  }
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

char tohexdigit(int x) {
  if (x<=9&&x>=0) {
    return (char)(x+'0');
  }else {
    return (char)(x-10+'A');    
  }

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

GFXColor getDamageColor(float armor,bool gradient=false) {
	static bool init=false;
	static float damaged[4]={1,0,0,1};
	static float half_damaged[4]={1,1,0,1};
	static float full[4]={1,1,1,1};
	if (!init) {
		vs_config->getColor("default","hud_target_damaged",damaged,true);
		vs_config->getColor("default","hud_target_half_damaged",half_damaged,true);
		vs_config->getColor("default","hud_target_full",full,true);
		init=true;
	}
	if (armor>=.9) {
		return GFXColor(full[0],full[1],full[2],full[3]);
	}
	float avghalf=armor>=.3?1:0;
	if (gradient&&armor>=.3) {
		avghalf=(armor-.3)/.6;
	}
	float avgdamaged=1-avghalf;
	return GFXColor(half_damaged[0]*avghalf+damaged[0]*avgdamaged,
		half_damaged[1]*avghalf+damaged[1]*avgdamaged,
		half_damaged[2]*avghalf+damaged[2]*avgdamaged,
		half_damaged[3]*avghalf+damaged[3]*avgdamaged);
}

static void DrawHUDSprite (VDU * thus, VSSprite* s, float per, float &sx, float &sy, float & w, float & h, float aup, float aright, float aleft, float adown,   float hull, bool drawsprite, bool invertsprite) {
  static bool HighQTargetVSSprites = XMLSupport::parse_bool(vs_config->getVariable("graphics","high_quality_sprites","false"));
  float nw,nh;
  thus->GetPosition (sx,sy);
  thus->GetSize (w,h);

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
	  s->SetSize (w,invertsprite?-h:h);
	  if (drawsprite) {
  		static const float middle_point = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","armor_hull_size",".55"));
                static bool top_view = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","top_view","false"));
  		float middle_point_small=1-middle_point;
  		Vector ll,lr,ur,ul,mll,mlr,mur,mul;
  		s->getTexture()->MakeActive();
  		GFXDisable (CULLFACE);
  		s->DrawHere(ll,lr,ur,ul);
 		mll=middle_point*ll+middle_point_small*ur;
  		mlr=middle_point*lr+middle_point_small*ul;
  		mur=middle_point*ur+middle_point_small*ll;
  		mul=middle_point*ul+middle_point_small*lr;
		bool tmax=1;
  		GFXBegin(GFXQUAD);
		GFXColorf(getDamageColor(top_view?aup:aleft));
  		GFXTexCoord2f(0, 0);
  		GFXVertexf(ul);
		GFXColorf(getDamageColor(aup));		
  		GFXTexCoord2f(1, 0);
  		GFXVertexf(ur);
		GFXColorf(getDamageColor(hull,true));		
  		GFXTexCoord2f(middle_point, middle_point_small);		
  		GFXVertexf(mur);
		GFXColorf(getDamageColor(hull,true));		
  		GFXTexCoord2f(middle_point_small, middle_point_small);
  		GFXVertexf(mul);
		GFXColorf(getDamageColor(top_view?aright:aup));
  		GFXTexCoord2f(1, 0);
  		GFXVertexf(ur);
		GFXColorf(getDamageColor(aright));
  		GFXTexCoord2f(1, 1);
  		GFXVertexf(lr);
		GFXColorf(getDamageColor(hull,true));				
  		GFXTexCoord2f(middle_point, middle_point);
  		GFXVertexf(mlr);
		GFXColorf(getDamageColor(hull,true));				
  		GFXTexCoord2f(middle_point, middle_point_small);
  		GFXVertexf(mur);
		GFXColorf(getDamageColor(top_view?adown:aright));
  		GFXTexCoord2f(1, 1);
  		GFXVertexf(lr);
		GFXColorf(getDamageColor(adown));		
  		GFXTexCoord2f(0, 1);
  		GFXVertexf(ll);
		GFXColorf(getDamageColor(hull,true));				
  		GFXTexCoord2f(middle_point_small, middle_point);
  		GFXVertexf(mll);
		GFXColorf(getDamageColor(hull,true));				
  		GFXTexCoord2f(middle_point, middle_point);
  		GFXVertexf(mlr);
		GFXColorf(getDamageColor(top_view?aleft:adown));
  		GFXTexCoord2f(0, 1);
  		GFXVertexf(ll);
		GFXColorf(getDamageColor(aleft));		
  		GFXTexCoord2f(0, 0);
  		GFXVertexf(ul);
  		GFXTexCoord2f(middle_point_small, middle_point_small);
		GFXColorf(getDamageColor(hull,true));		
  		GFXVertexf(mul);
		GFXColorf(getDamageColor(hull,true));		
  		GFXTexCoord2f(middle_point_small, middle_point);
  		GFXVertexf(mll);
		GFXColorf(getDamageColor(hull,true));
  		GFXTexCoord2f(middle_point_small, middle_point_small);
  		GFXVertexf(mul);
  		GFXTexCoord2f(middle_point, middle_point_small);
  		GFXVertexf(mur);
  		GFXTexCoord2f(middle_point, middle_point);
  		GFXVertexf(mlr);
  		GFXTexCoord2f(middle_point_small, middle_point);
  		GFXVertexf(mll);
  		GFXEnd();
		
  		GFXEnable (CULLFACE);
	  }
	  s->SetSize (nw,nh);
	  h = fabs(h);
	  if (HighQTargetVSSprites) {
		  GFXBlendMode (ONE,ZERO);
	  }else {
		  GFXAlphaTest(ALWAYS,0);
	  }
  }

}
void VDU::DrawTargetSpr (VSSprite *s, float per, float &sx, float &sy, float &w, float &h) {
  DrawHUDSprite (this, s, per, sx, sy, w, h, 1, 1, 1, 1, 1, true,false);
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
static void DrawShield (float fs, float rs, float ls, float bs, float x, float y, float w, float h, bool invert) { //FIXME why is this static?
  GFXBegin (GFXLINE);
  if (invert ) {
    float tmp = fs;
    fs = bs;
    bs = tmp;
  }
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
static void DrawShieldArmor(Unit * parent, const float StartArmor[8], float x, float y, float w, float h,bool invertfrontback) {
  float fs = parent->FShieldData();
  float rs = parent->RShieldData();
  float ls = parent->LShieldData();
  float bs = parent->BShieldData();
  float armor[8];
  GFXColor4f (.4,.4,1,1);
  GFXDisable (TEXTURE0);
  DrawShield (fs,rs,ls,bs,x,y,w,h,invertfrontback);
  parent->ArmorData (armor);
  GFXColor4f (1,.6,0,1);
  DrawShield ((armor[0]+armor[2]+armor[4]+armor[6])/(float)(StartArmor[0]+StartArmor[2]+StartArmor[4]+StartArmor[6]),(armor[0]+armor[1]+armor[4]+armor[5])/(float)(StartArmor[0]+StartArmor[1]+StartArmor[4]+StartArmor[5]),(armor[2]+armor[3]+armor[6]+armor[7])/(float)(StartArmor[2]+StartArmor[3]+StartArmor[6]+StartArmor[7]),(armor[1]+armor[3]+armor[5]+armor[7])/(float)(StartArmor[1]+StartArmor[3]+StartArmor[5]+StartArmor[7]),x,y,w/2,h/2, invertfrontback);
}
void VDU::DrawVDUShield (Unit * parent) {
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
  static bool invert_friendly_shields = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","invert_friendly_shields","false"));
  DrawShieldArmor(parent,StartArmor,x,y,w,h,invert_friendly_shields);
  GFXColor4f (1,parent->GetHullPercent(),parent->GetHullPercent(),1);
  GFXEnable (TEXTURE0);
  GFXColor4f (1,parent->GetHullPercent(),parent->GetHullPercent(),1);
  static bool invert_friendly_sprite = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","invert_friendly_sprite","false"));
  DrawHUDSprite (this,parent->getHudImage (),.25,x,y,w,h,parent->GetHullPercent(),parent->GetHullPercent(),parent->GetHullPercent(),parent->GetHullPercent(),parent->GetHullPercent(),true,invert_friendly_sprite);

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
VSSprite * getNavImage () {
  static VSSprite s("nav-hud.spr");
  return &s;
}

double DistanceTwoTargets(Unit *parent, Unit *target) {
  return ((parent->Position()-target->Position()).Magnitude()-((target->isUnit()==PLANETPTR)?target->rSize():0));
}

struct retString128{
  char str [128];
};

retString128 PrettyDistanceString(double distance) {
	struct retString128 qr;
	static float game_speed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
	static bool lie=XMLSupport::parse_bool (vs_config->getVariable("physics","game_speed_lying","true"));
	if(lie){
		sprintf (qr.str,"Distance: %.2lf",distance/game_speed);
	} else {
		if(distance<100000){
			sprintf (qr.str,"Distance: %.0lf Meters",distance);
		} else if (distance<100000000){
			sprintf (qr.str,"Distance: %.0lf Kilometers",distance/1000);
		} else {
			sprintf (qr.str,"Distance: %.2lf LightSeconds",distance/300000000);
		}
	}
	return qr;
}
static float OneOfFour(float a, float b, float c, float d){
	int aa=  a!=0?1:0;
	int bb=  b!=0?1:0;
	int cc=  c!=0?1:0;
	int dd=  d!=0?1:0;
	if (aa+bb+cc+dd==4)
		return 1;
	if (aa+bb+cc+dd==3)
		return .85;
	if (aa+bb+cc+dd==2)
		return .4;
	return 0;
}
void VDU::DrawTarget(Unit * parent, Unit * target) {
    float x,y,w,h;

  float fs = target->FShieldData();
  float rs = target->RShieldData();
  float ls = target->LShieldData();
  float bs = target->BShieldData();
  GFXEnable (TEXTURE0);

  static bool invert_target_sprite = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","invert_target_sprite","false"));
  static bool invert_target_shields = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","invert_target_shields","false"));
  float armor[8];
  target->ArmorData(armor);
  float au,ar,al,ad;
  au=OneOfFour(armor[0],armor[2],armor[4],armor[6]);
  ar=OneOfFour(armor[0],armor[1],armor[4],armor[5]);
  al=OneOfFour(armor[2],armor[3],armor[6],armor[7]);
  ad=OneOfFour(armor[1],armor[3],armor[5],armor[7]);

  if (invert_target_shields){
	  float tmp =au;
	  au = ad;ad=tmp;
  }
  if (target->isUnit()==PLANETPTR){
	  au=ar=al=ad=target->GetHullPercent();
  }
  
  DrawHUDSprite (this,((target->isUnit()!=PLANETPTR||target->getHudImage()!=NULL)?target->getHudImage ():
                       (target->GetDestinations().size()!=0? getJumpImage():
                        (((Planet *)target)->hasLights()?getSunImage():
                         (target->getFullname().find("invisible")!=string::npos?getNavImage():getPlanetImage())))),.6,x,y,w,h,
				 au,ar,al,ad,
				 target->GetHullPercent(),
				 true,invert_target_sprite);
  
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
  retString128 qr=PrettyDistanceString(DistanceTwoTargets(parent,target));
  strcat (st,qr.str);
  tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);  
  GFXColor4f (.4,.4,1,1);
  DrawShield (fs,rs,ls,bs,x,y,w,h,invert_target_shields);
  GFXColor4f (1,1,1,1);
  }else {
  tp->Draw (MangleString ("\n[OutOfRange]",_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);      

  }
}

void VDU::DrawMessages(Unit *target){
  string fullstr;
  double nowtime=mission->getGametime();

  /*
  char st[256];
  //  sprintf (st,"\n%s",target->name.c_str());
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
*/
  string targetstr;
  int msglen=targetstr.size();
  
  int rows_needed=0;//msglen/(cols>0?cols:1);

  MessageCenter *mc=mission->msgcenter;
  
  int rows_used=rows_needed;
  vector <std::string> whoNOT;
  whoNOT.push_back ("briefing");
  whoNOT.push_back ("news");
  whoNOT.push_back ("bar");
  static float oldtime = XMLSupport::parse_float(vs_config->getVariable("graphics","last_message_time","5"));
  static int num_messages=XMLSupport::parse_int(vs_config->getVariable("graphics","num_messages","2"));
  vector <std::string> message_people;//should be "all", parent's name
  gameMessage lastmsg;
  for(int i=scrolloffset<0?-scrolloffset-1:0;rows_used<((scrolloffset<0||num_messages>rows)?rows:num_messages)&&mc->last(i,lastmsg,message_people,whoNOT);i++){
      char timebuf[100];
      double sendtime=lastmsg.time;
      if (scrolloffset>=0&&sendtime<nowtime-oldtime*4){
        break;
      }
      if(sendtime<=nowtime&&(sendtime>nowtime-oldtime||scrolloffset<0)){
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
  //tp->Draw(MangleString ("Scanning target...",_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);
}

bool VDU::SetCommAnimation (Animation * ani, bool force) {
  if (comm_ani==NULL||force) {
    if (posmodes&COMM) {
      if (ani!=NULL&&comm_ani==NULL)
        thismode.push_back(COMM);
      else if (comm_ani!=NULL&&thismode.size()>1&&ani!=NULL)
        thismode.back()=COMM;
      if (ani){
        comm_ani = ani;       
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
  sprintf (navdata,"Navigation\n----------\n%s\nTarget:\n  %s\nRelativeLocation\nx: %.4f\ny:%.4f\nz:%.4f\nDistance:\n%f",_Universe->activeStarSystem()->getName().c_str(),nam.c_str(),nav.i,nav.j,nav.k,lie?(nav.Magnitude()/game_speed):nav.Magnitude());
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

void VDU::DrawManifest (Unit * parent, Unit * target) {	//	zadeVDUmanifest
  string retval ("Manifest\n");
  if (target!=parent) {
    retval+=string ("Tgt: ")+reformatName(target->name)+string("\n");
  }else {
    retval+=string ("--------\nCredits: ")+tostring((int)_Universe->AccessCockpit()->credits)+/*string(".")+tostring (((int)(_Universe->AccessCockpit()->credits*100))%100) +*/string("\n");
  }
  unsigned int numCargo =target->numCargo();
  for (unsigned int i=0;i<numCargo;i++) {
    if (target->GetCargo(i).category.find("upgrades/")!=0)
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
extern float PercentOperational(Unit*,string,string);
void VDU::DrawDamage(Unit * parent) {	//	VDUdamage
  float x,y,w,h;
  float th;
  //  char st[1024];
  GFXColor4f (1,parent->GetHull()/ (*maxhull),parent->GetHull()/(*maxhull),1);
  GFXEnable (TEXTURE0);
  float armor[8];
  parent->ArmorData (armor);
  static bool draw_damage_sprite=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_damage_sprite","true"));
  DrawHUDSprite (this,draw_damage_sprite? parent->getHudImage ():NULL,.6,x,y,w,h,
    (armor[0]+armor[2]+armor[4]+armor[6])/(float)(StartArmor[0]+StartArmor[2]+StartArmor[4]+StartArmor[6]),(armor[0]+armor[1]+armor[4]+armor[5])/(float)(StartArmor[0]+StartArmor[1]+StartArmor[4]+StartArmor[5]),(armor[2]+armor[3]+armor[6]+armor[7])/(float)(StartArmor[2]+StartArmor[3]+StartArmor[6]+StartArmor[7]),(armor[1]+armor[3]+armor[5]+armor[7])/(float)(StartArmor[1]+StartArmor[3]+StartArmor[5]+StartArmor[7]),parent->GetHull()/ (*maxhull),true,false);
  GFXDisable(TEXTURE0);
  Unit * thr = parent->Threat();
  std::string fullname (getUnitNameAndFgNoBase(parent));
  //sprintf (st,"%s\nHull: %.3f",blah.c_str(),parent->GetHull());
  //tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.5:0),0,true);  
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
    DrawShield (0, s, s, 0, x, y, w,h,false);
  }
  GFXColor4f (1,1,1,1);
  

/*

  Cargo & GetCargo (unsigned int i);
  void GetCargoCat (const std::string &category, vector <Cargo> &cat);
  ///below function returns NULL if not found
  Cargo * GetCargo (const std::string &s, unsigned int &i);

*/


    //*******************************************************zade

    char hullval[128];
    sprintf (hullval,"%.3f",parent->GetHull());
    string retval (fullname+"\nHull: "+hullval+"\n");
    unsigned int numCargo =parent->numCargo();
    double percent_working = 0.88;
    for (unsigned int i=0;i<numCargo;i++) {
      
      percent_working = 0.88;// cargo.damage
      Cargo& the_cargo = parent->GetCargo(i);
      if(the_cargo.GetCategory().find("upgrades/")==0){
        percent_working = PercentOperational(parent,the_cargo.content,the_cargo.category);
	//	retval+=parent->GetManifest (i,parent,parent->GetVelocity())+string (" (")+tostring (int(percent_working*100))+string ("%)" +the_cargo.GetCategory()+"\n");
        GFXColor final_color ((1.0*percent_working)+(1.0*(1.0-percent_working)),
                              (1.0*percent_working)+(0.0*(1.0-percent_working)),
                              (0.0*percent_working)+(0.0*(1.0-percent_working)),
                              (1.0*percent_working)+(1.0*(1.0-percent_working)));
        if(percent_working == 0.0){final_color = GFXColor(0.2,0.2,0.2);}	//	dead = grey
        std::string trailer;
        if (percent_working<1.0) {
          int r = (int)(final_color.r*255);
          if (r>255)r=255;
          int rl = r%16;
          int rh = r/16;
          
          int g = (int)(final_color.g*255);
          if (g>255)g=255;
          int gl = g%16;
          int gh = g/16;
          int b = (int)(final_color.b*255);
          if (b>255)b=255;
          int bl = b%16;
          int bh = b/16;
          retval+='#';
          retval+=tohexdigit(rh);
          retval+=tohexdigit(rl);
          retval+=tohexdigit(gh);
          retval+=tohexdigit(gl);
          retval+=tohexdigit(bh);
          retval+=tohexdigit(bl);
          
          trailer="#FFFFFF";
        }
        retval+=parent->GetManifest (i,parent,parent->GetVelocity())+string (" (")+tostring (int(percent_working*100))+string ("%)")+trailer+std::string("\n");
        
        
      }
    }
    retval+=ecmstatus;
    tp->Draw (MangleString (retval.c_str(),_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),scrolloffset,true);     
    //*******************************************************
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
  float tmpaspect=g_game.aspect;
  g_game.aspect=w/h;
  _Universe->AccessCamera(which)->SetSubwindow (x,y,w,h);
  _Universe->SelectCamera(which);
  VIEWSTYLE tmp = _Universe->AccessCockpit()->GetView ();
  _Universe->AccessCockpit()->SetView (viewStyle);
  _Universe->AccessCockpit()->SelectProperCamera();
   _Universe->AccessCockpit()->SetupViewPort(true);///this is the final, smoothly calculated cam
  GFXClear (GFXFALSE);
  GFXColor4f(1,1,1,1);
  _Universe->activeStarSystem()->Draw(false);
  g_game.aspect=tmpaspect;
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
  bool inrange=false;
  if (target) {
    double mm=0;
    std::string blah(getUnitNameAndFgNoBase(target));
    sprintf(buf,"%s\n",blah.c_str());
    inrange=parent->InRange(target,mm,true,false,false);
  }
  tp->Draw(MangleString (buf,_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);

  if (inrange) {  
    int i=0;
    char st[1024];
    for (i=0;i<rows-1&&i<128;i++) {
      st[i]='\n';
    }
    st[i]='\0';
    struct retString128 qr=PrettyDistanceString(DistanceTwoTargets(parent,target));
    strcat (st,qr.str);
    tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);  
    GFXColor4f (.4,.4,1,1);
    GetPosition (x,y);
    GetSize (w,h);
    static bool draw_vdu_target_info=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_vdu_view_shields","true"));
    if (target&&draw_vdu_target_info){
      if (viewStyle==CP_PANTARGET) {
		  DrawHUDSprite(this,getSunImage(),1,x,y,w,h,1,1,1,1,1,false,false);
        h=fabs (h*.6);
        w=fabs (w*.6);
        static bool invert_view_shields = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","invert_view_shields","false"));
        DrawShield(target->FShieldData(),target->RShieldData(),target->LShieldData(),target->BShieldData(),x,y,w,h,invert_view_shields);
      }
    }
    GFXColor4f (1,1,1,1);
  }else {
    if (target)
      tp->Draw (MangleString ("\n[OutOfRange]",_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);      
  }
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
  unsigned int tmp2 = (unsigned int)suc;
  unsigned int tmp1 = (unsigned int)(255-suc);
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
  std::string rez("\n");
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
	if (j->getOwner()==NULL||j->getOwner()==parent) {
          if ((*j).objective.length()) {
            rez+=GetColorFromSuccess((*j).completeness);
            rez+=(*j).objective;
            rez+='\n';
          }
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
  if (thismode.back()!=COMM&&comm_ani!=NULL) {    
    if (comm_ani->Done())
      comm_ani=NULL;
  }
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

