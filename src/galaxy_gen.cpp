
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "macosx_math.h"
#include <math.h>
#include <time.h>
#include <assert.h>

#include "configxml.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "gfxlib.h"
#include "galaxy_xml.h"
#include "galaxy_gen.h"
#include "vs_random.h"


#ifndef _WIN32
#include <ctype.h>
#endif
#ifndef M_PI
#define M_PI 3.1415926536
#endif
#include "vsfilesystem.h"

using namespace VSFileSystem;
using namespace std;

static VSRandom starsysrandom(time(NULL));
static void seedrand(unsigned long seed) {
  starsysrandom=VSRandom(seed);
} 
static int stringhash(const std::string &key) {
	unsigned int k = 0;
	std::string::const_iterator start = key.begin();
	for(;start!=key.end(); start++) {
		k += (k * 128) + *start;
	}
	return k;
}	

static unsigned int ssrand()
{
        return starsysrandom.rand();
}



static std::string GetWrapXY(std::string cname, int & wrapx, int & wrapy){
    std::string wrap = cname;
    wrapx=wrapy=1;
    string::size_type pos =wrap.find ("wrapx");
    if (pos!=string::npos) {
      string Wrapx = wrap.substr (pos+5,wrap.length());
      cname = cname.substr (0,pos);
      sscanf(Wrapx.c_str(),"%d",&wrapx);
      pos = Wrapx.find ("wrapy");
      if (pos!=string::npos) {
	string Wrapy = Wrapx.substr (pos+5,Wrapx.length());
	sscanf (Wrapy.c_str(),"%d",&wrapy);
      }
    }
    return cname;
}


string getStarSystemName (string in);

namespace StarSystemGent {

float mmax (float a, float b) {
  return (a>b)?a:b;
}

int rnd(int lower, int upper) {
  if (upper>lower) 
	  return lower+ ssrand()%(upper-lower); 
	  else return lower;
	  
	  // ( lower+ (((float(upper-lower))*ssrand())/(float (VS_RAND_MAX)+1.))); WE THINK THIS IS BUGGY
}
const char nada[1]="";
string  getGenericName(vector<string> &s) {
  if (s.empty())
    return string(nada);
  
  return s[rnd(0,s.size())];
}



string  getRandName(vector<string> &s) {
  if (s.empty())
    return string(nada);
  unsigned int i=rnd(0,s.size());
  string k=s[i];
  s.erase(s.begin()+i);
  return k;
}
struct Color {
  float r,g,b,a;
  float nr,ng,nb,na;
  Color (float rr, float gg, float bb) {
    r=rr;b=bb;g=gg;
  }
};
class Vector {
public:
  float i;
  float j;
  float k;
  float s;
  float t;
  Vector () {i=j=k=0;}
  Vector (float x, float y, float z) {
    i=x;
    j=y;
    k=z;
  }
  Vector (float x, float y, float z, float s, float t) {
    i=x;
    j=y;
    k=z;
    this->s=s;
    this->t=t;
  }
  float Mag () {return sqrtf(i*i+j*j+k*k);}
  Vector Cross(const Vector &v) const {return Vector ( this->j*v.k-this->k*v.j, 
						       this->k*v.i-this->i*v.k,
						       this->i*v.j-this->j*v.i);}
  void Yaw(float rad) //only works with unit vector
  {
    float theta;
    float m = Mag();
    if (i>0)
      theta = (float)atan(k/i);
    else if (i<0)
      theta = M_PI+(float)atan(k/i);
    else if (k<=0 && i==0)
      theta = -M_PI/2;
	else if (k>0 && i==0)
	  theta = M_PI/2;
    
    theta += rad;
    i = m*cosf(theta);
    k = m*sinf(theta); 
  }
  
  void Roll(float rad)
  {
    float theta;
    float m = Mag();
    if (i>0)
      theta = (float)atan(j/i);
    else if (i<0)
		theta = M_PI+(float)atan(j/i);
    else if (j<=0 && i==0)
      theta = -M_PI/2;
    else if (j>0 && i==0)
      theta = M_PI/2;
    
    theta += rad; 
    i = m*cosf(theta);
    j = m*sinf(theta); 
  }
  
  void Pitch(float rad)
  {
    float theta;
    float m = Mag();
    if (k>0)
      theta = (float)atan(j/k);
    else if (k<0)
      theta = M_PI+(float)atan(j/k);
    else if (j<=0 && k==0)
      theta = -M_PI/2;
    else if (j>0 && k==0)
      theta = M_PI/2;
    
    theta += rad;
    k = m*cosf(theta);
    j = m*sinf(theta);
  }
};

float grand () {
  return float (ssrand())/VS_RAND_MAX;
}
vector <Color> lights;
VSFile f;
int xmllevel;

static void Tab () {
  for (int i=0;i<xmllevel;i++)
    f.Fprintf ("\t");
}
 
float difffunc (float inputdiffuse) {
  return sqrt(((inputdiffuse)));
}
void WriteLight (unsigned int i) {
  static float ambientColorFactor= XMLSupport::parse_float (vs_config->getVariable("galaxy","AmbientLightFactor","0"));
  float ambient =(lights[i].r+lights[i].g+lights[i].b);
  
  ambient*=ambientColorFactor;
  Tab();f.Fprintf ("<Light>\n");
  xmllevel++;
  Tab();f.Fprintf("<ambient red=\"%f\" green=\"%f\" blue=\"%f\"/>\n",ambient,ambient,ambient);
  Tab();f.Fprintf("<diffuse red=\"%f\" green=\"%f\" blue=\"%f\"/>\n",difffunc (lights[i].r),difffunc (lights[i].g),difffunc (lights[i].b));
  Tab();f.Fprintf("<specular red=\"%f\" green=\"%f\" blue=\"%f\"/>\n",lights[i].nr,lights[i].ng,lights[i].nb);
  xmllevel--;
  Tab();f.Fprintf("</Light>\n");
}
struct GradColor {
  float minrad;
  float r;
  float g;
  float b;
  float variance;
};

const int STAR   = 0;
const int PLANET = 1;
const int MOON   = 2;
const int JUMP   = 3;
  //begin global variables


vector <string> starentities;
vector <string> jumps;
vector <string> gradtex;
vector <string> naturalphenomena;
vector <string> starbases;
unsigned int numstarbases;
unsigned int numnaturalphenomena;
unsigned int numstarentities;
vector <string> background;
vector <string> names;
vector <string> rings;
const float moonofmoonprob=.01;
string systemname;
//int numnaturalphenomena;
//int numstarbases;
vector <float> radii;
const float minspeed=.001;
const float maxspeed=8;
vector <float> starradius;
string faction;
vector <GradColor> colorGradiant;
float compactness=2;
float jumpcompactness=2;

struct PlanetInfo {
  string name;
  unsigned int num; // The texture number for the city lights
  unsigned int moonlevel; // 0==top-level planet, 1==first-level moon, 2 is second-level moon... probably won't be used.
  unsigned int numstarbases; // Number of starbases allocated to orbit around this planet. Usually 1 or 0 but quite possibly more.
  unsigned int numjumps; // Number of jump points.
  PlanetInfo()
    : num(0),moonlevel(0),numstarbases(0),numjumps(0) {
  }
};

struct StarInfo {
  vector <PlanetInfo> planets;
  unsigned int numjumps;
  unsigned int numstarbases;
  StarInfo()
      : numjumps(0),numstarbases(0) {
  }
};
vector <StarInfo> stars;
unsigned int planetoffset, staroffset, moonlevel;

void ResetGlobalVariables () {
  xmllevel=0;
  lights.clear();
  gradtex.clear();
//  numun[0]=numun[1]=numstarentities=0;
  naturalphenomena.clear();
  starbases.clear();
  starentities.clear();
  numstarentities=0;
  numstarbases=0;
  numnaturalphenomena=0;
  background.clear();
  stars.clear();
  planetoffset=0;
  staroffset=0;
  moonlevel=0;
  names.clear();
  jumps.clear();
  rings.clear();
  systemname.erase();
  starradius.clear();
  faction.erase();
  colorGradiant.clear();
  compactness=2;
  jumpcompactness=2;
  radii.clear();
}

void readColorGrads (vector <string> &entity,const char * file) {
  VSFile f;
  VSError err = f.OpenReadOnly( file, UniverseFile);
  static float radiusscale= XMLSupport::parse_float (vs_config->getVariable("galaxy","StarRadiusScale","50"));

  if (err>Ok) {
    printf ("Failed to load %s",file);
    GradColor (g);
    g.minrad=0;
    g.r=g.g=g.b=.9;
    g.variance=.1;
    entity.push_back ("white_star.png");
    colorGradiant.push_back (g);
    return;
  }
  char input_buffer[1000];
  char output_buffer[1000];
  GradColor g;
  while (!f.Eof()) {
    f.ReadLine(input_buffer,999);
    if (sscanf (input_buffer,"%f %f %f %f %f %s",&g.minrad,&g.r,&g.g,&g.b,&g.variance,output_buffer)==6) {
      g.minrad *=radiusscale;
	  colorGradiant.push_back (g);
      entity.push_back (output_buffer);
    }
  }
  f.Close();
}


float clamp01 (float a) {
  if (a>1)
    a=1;
  if (a<0)
    a=0;
  return a;
}
float getcolor (float c, float var) {
  return clamp01(c-var+2*var*grand());
}
GradColor whichGradColor (float r,unsigned int &j) {
  unsigned int i;
  if (colorGradiant.empty()) {
	  vector<string> entity;
	  string fullpath = "stars.txt";
	  readColorGrads(entity,fullpath.c_str());
  }
  for (i=1;i<colorGradiant.size();i++) {
    if (colorGradiant[i].minrad>r) {
      break;
    }
  }
  j=i-1;
  return colorGradiant[i-1];
}

Color StarColor (float radius, unsigned int &entityindex) {
  GradColor gc=whichGradColor (radius,entityindex);
  float r=getcolor(gc.r,gc.variance);
  float g=getcolor(gc.g,gc.variance);
  float b=getcolor(gc.b,gc.variance);
  return Color (r,g,b);
}
GFXColor getStarColorFromRadius(float radius) {
	unsigned int myint=0;
	static float radiusscale= XMLSupport::parse_float (vs_config->getVariable("galaxy","StarRadiusScale","1000"));	
	Color tmp = StarColor(radius*radiusscale,myint);
	return GFXColor(tmp.r,tmp.g,tmp.b,1);
}
float LengthOfYear (Vector r,Vector s) {
  float a=2*M_PI*mmax (r.Mag(),s.Mag());
  float speed = minspeed+(maxspeed-minspeed)*grand();
  return a/speed;
}


void CreateLight(unsigned int i) {
  if (i==0) {
    assert (!starradius.empty());
    assert (starradius[0]);
  } else {
    assert (starradius.size()==i);
    starradius.push_back (starradius[0]*(.5+grand()*.5));
  }
  unsigned int gradindex;
  lights.push_back (StarColor (starradius[i],gradindex));
  starentities.push_back (gradtex[gradindex]);
  float h = lights.back().r;
  if (h<lights.back().g) h=lights.back().g;
  if (h<lights.back().b) h=lights.back().b;
  float norm = (.5+.5/numstarentities);
  if (h>.001) {
    lights.back().nr=lights.back().r/h;
    lights.back().ng=lights.back().g/h;
    lights.back().nb=lights.back().b/h;
  
    lights.back().r*=norm/h;
    lights.back().g*=norm/h;
    lights.back().b*=norm/h;
  }else {
    lights.back().nr=0;
    lights.back().ng=0;
    lights.back().nb=0;
  }
  WriteLight (i);
}


Vector generateCenter (float minradii,bool jumppoint) {
  Vector r;
  float tmpcompactness = compactness;
  if (jumppoint)
	  tmpcompactness=jumpcompactness;
  r = Vector (tmpcompactness*grand()+1,tmpcompactness*grand()+1,tmpcompactness*grand()+1);
  r.i*=minradii;
  r.j*=minradii;
  r.k*=minradii;
  int i=(rnd(0,8));
  r.i=(i&1)?-r.i:r.i;
  r.j=(i&2)?-r.j:r.j;
  r.k=(i&4)?-r.k:r.k;
  return r;
}
float makeRS (Vector &r, Vector &s,float minradii,bool jumppoint) {
  r=Vector (grand(),grand(),grand());
  int i=(rnd(0,8));
  r.i=(i&1)?-r.i:r.i;
  r.j=(i&2)?-r.j:r.j;
  r.k=(i&4)?-r.k:r.k;

  Vector k(grand(),grand(),grand());
  i=(rnd(0,8));
  k.i=(i&1)?-k.i:k.i;
  k.j=(i&2)?-k.j:k.j;
  k.k=(i&4)?-k.k:k.k;

  s = r.Cross (k);
  float sm = s.Mag();
  if (sm<.01) {
    return makeRS(r,s,minradii,jumppoint);
  }
  s.i/=sm;  s.j/=sm;  s.k/=sm;
  sm = r.Mag();
  r.i/=sm;  r.j/=sm;  r.k/=sm;
  bool tmp=false;
  float rm;
  float tmpcompactness=compactness;
  if (jumppoint)
	  tmpcompactness=  jumpcompactness;
  rm= (tmpcompactness*grand()+1); if (rm<1) {rm=(1+grand()*.5);tmp=true;}
  rm*=minradii;
  r.i*=rm;r.j*=rm;r.k*=rm;
  sm= (tmpcompactness*grand()+1); if (tmp) sm=(1+grand()*.5);
  sm*=minradii;
  s.i*=sm;s.j*=sm;s.k*=sm;
  return mmax (rm,sm);
}


void Updateradii (float orbitsize, float thisplanetradius) {
#ifdef HUGE_SYSTEMS

  orbitsize+=thisplanetradius;
  radii.back()=orbitsize;
#endif


}

Vector generateAndUpdateRS (Vector &r, Vector & s, float thisplanetradius,bool jumppoint) {
  if (radii.empty()) {
    r=Vector (0,0,0);
    s=Vector (0,0,0);
    return generateCenter (starradius[0],jumppoint);
  }
  float tmp=radii.back()+thisplanetradius;
  Updateradii(makeRS (r,s,tmp,jumppoint),thisplanetradius);
  return generateCenter (tmp,jumppoint);
}

vector <string> parseBigUnit (const string &input) {
  char * mystr = strdup (input.c_str());
  char * ptr = mystr;
  char * oldptr=mystr;
  vector <string> ans;
  while (*ptr!='\0') {
    while (*ptr!='&'&&*ptr!='\0') {
      ptr++;
    }
    if (*ptr=='&') {
      *ptr='\0';
      ptr++;
    }
    ans.push_back (string(oldptr));
    oldptr=ptr;
  }
  free(mystr);    
  return ans;
}


void WriteUnit(const string &tag, const string &name, const string &filename, const Vector &r, const Vector &s, const Vector &center, const string &nebfile, const string &destination, bool faction, float thisloy=0) {
  Tab();
  f.Fprintf ("<%s name=\"%s\" file=\"%s\" ",tag.c_str(),name.c_str(),filename.c_str());
  if (nebfile.length()>0) {
    f.Fprintf ("nebfile=\"%s\" ",nebfile.c_str());
  }

  f.Fprintf ("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",r.i,r.j,r.k,s.i,s.j,s.k);
  f.Fprintf ("x=\"%f\" y=\"%f\" z=\"%f\" ",center.i,center.j,center.k);
  float loy = LengthOfYear(r,s);
  if (loy||thisloy) {
    f.Fprintf ("year= \"%f\" ",thisloy?thisloy:loy);
  }
  if (destination.length()) {
    f.Fprintf("destination=\"%s\" ",destination.c_str()); 
  } else if (faction){
    f.Fprintf ("faction=\"%s\" ",StarSystemGent::faction.c_str());
  }
  f.Fprintf ("/>\n");
}
string getJumpTo (const string &s) {
  char tmp[1000]="";
  if (1==sscanf (s.c_str(),"Jump_To_%s",tmp)){
    tmp[0]=tolower(tmp[0]);
  }
  else return s;
  return string (tmp);
}
string starin (const string &input) {
  char * tmp = strdup (input.c_str());
  for (unsigned int i=0;tmp[i]!='\0';i++) {
    if (tmp[i]=='*') {
      tmp[i]='\0';
      string ans (tmp);
      free (tmp);
      return ans;
    }
  }

  free (tmp);
  return string ();
}

string GetNebFile (string &input) {
  string ip = input.c_str();
  char * ptr = strdup(ip.c_str());
  for (unsigned int i=0;ptr[i]!='\0';i++) {
    if (ptr[i]=='^') {
      ptr[i]='\0';
      string ans (ptr);
      input = ptr+i+1;
      free (ptr);
      return ans;
    }
  }
  free (ptr);
  return string ();
}


string AnalyzeType (string &input, string &nebfile, float & radius) {
  if (input.empty())
    return "";
  char  ptr = *input.begin();  
  string ip;
  if (0==sscanf (GetNebFile(input).c_str(),"%f",&radius)) {
    radius=100;
    ip=(input.c_str()+1);
  }
  else {
    ip=(input);
  }


  string retval;
  switch (ptr) {
  case 'N':
    nebfile=GetNebFile(input);
    retval="Nebula";
    break;
  case 'A':
    retval="Asteroid";
    break;
  case 'B':
    retval="Building";
    break;
  case 'E':
    retval="Enhancement";
    break;
  case 'U':
  default:
    retval="Unit";

  }
  return retval;

}
void MakeSmallUnit () {
  Vector R,S;

  string nam;
  string s=string ("");
  while (s.length()==0) {
    nam= getRandName (starbases);
    if (nam.length()==0)
      return;
    string tmp;
    if ((tmp=starin(nam)).length()>0) {
      nam=(tmp);
      s = getRandName (jumps);
    }else {
      break;
    }
  }
  numstarbases--;
  string nebfile ("");
  float radius;
  string type = AnalyzeType(nam,nebfile,radius);
  Vector center=generateAndUpdateRS(R,S,radius,true);

  WriteUnit (type,"",nam,R,S,center,nebfile,s,true);

}



void MakeJump(float radius, bool forceRS=false, Vector R=Vector (0,0,0), Vector S=Vector (0,0,0), Vector center=Vector (0,0,0), float thisloy=0) {
  string s =  getRandName (jumps);
  if (s.length()==0)
    return;
  Vector RR,SS;
  if (forceRS) {
    RR=R;SS=S;
    Updateradii (mmax(RR.Mag(),SS.Mag()),radius);
  }else {
    center=generateAndUpdateRS (RR,SS,radius,true);
  }
  string thisname;
  thisname = string("Jump_To_")+getStarSystemName(s);
  if (thisname.length()>8) {
    *(thisname.begin()+8)=toupper(*(thisname.begin()+8));
  }
  Tab();
  f.Fprintf ("<Jump name=\"%s\" file=\"%s\" ",thisname.c_str(),"jump.png");
  f.Fprintf ("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",RR.i,RR.j,RR.k,SS.i,SS.j,SS.k);
  f.Fprintf ("radius=\"%f\" ",radius);
  f.Fprintf ("x=\"%f\" y=\"%f\" z=\"%f\" ",center.i,center.j,center.k);
  float loy = LengthOfYear(RR,SS);
  float temprandom=.1*fmod(loy,10);//use this so as not to alter state here
  if (loy||thisloy) {
    f.Fprintf ("year= \"%f\" ",thisloy?thisloy:loy);
    temprandom=grand();
    loy =864*temprandom;
    if (loy) {
      f.Fprintf ( "day=\"%f\" ",loy);
    }
  }
  f.Fprintf ( "alpha=\"ONE ONE\" destination=\"%s\" faction=\"%s\" />\n",getJumpTo(s).c_str(),StarSystemGent::faction.c_str());

  ///writes out some pretty planet tags
}
void MakeBigUnit (int callingentitytype, string name=string(),float orbitalradius=0) {
  vector <string> fullname;
  if (name.length()==0) {
    string s= getRandName (naturalphenomena);
    if (s.length()==0)
      return;
    fullname = parseBigUnit(s);
  }else {
    fullname.push_back( name);
  }
  if (fullname.empty())
    return;
  numnaturalphenomena--;
  Vector r,s;

  float stdloy=0;
  bool first=false;
  float size=0;
  string tmp;
  Vector center(0,0,0);
  string nebfile("");
  for (unsigned int i=0;i<fullname.size();i++) {
    if (1==sscanf (fullname[i].c_str(),"jump%f",&size)) {
      if (!first) {
	first= true;
	center=generateAndUpdateRS (r,s,size,callingentitytype!=STAR);
	stdloy=LengthOfYear (r,s);	
      }
      MakeJump(size,true,r,s,center,stdloy);
	  // We still want jumps inside asteroid fields, etcVvv.
    }else if (1==sscanf (fullname[i].c_str(),"planet%f",&size)||1==sscanf (fullname[i].c_str(),"moon%f",&size)||1==sscanf (fullname[i].c_str(),"gas%f",&size)) {
/*
      if (!first) {
	first= true;
	center=generateAndUpdateRS (r,s,size,false);
	stdloy=LengthOfYear (r,s);	
      }
      MakePlanet (size,<TYPE>,callingentitytype,true,r,s,center,stdloy);
*/
		//FIXME: Obsolete/not supported/too lazy to implement.
    }else if ((tmp=starin(fullname[i])).length()>0) {
      if (!first) {
	first= true;
	center=generateAndUpdateRS (r,s,size,callingentitytype!=STAR);
	stdloy=LengthOfYear (r,s);	
      }
      string S = getRandName (jumps);
      if (S.length()>0) {
	string type = AnalyzeType(tmp,nebfile,size);
	WriteUnit (type, S,tmp,r,s,center,nebfile,getJumpTo(S),false,stdloy);
      }
    } else {
		string type = AnalyzeType(fullname[i],nebfile,size);
		if (!first) {
 			first= true;
			center=generateAndUpdateRS (r,s,size,callingentitytype!=STAR?type=="Unit":false);
			stdloy=LengthOfYear (r,s);	
		}
		WriteUnit(type,"",fullname[i],r,s,center,nebfile,string(""),i!=0,stdloy);
		
    }
  }


}
void MakeMoons (float callingradius, int callingentitytype);
void MakeJumps (float callingradius, int callingentitytype, int numberofjumps );
void MakePlanet(float radius, int entitytype, string texturename, int texturenum, int numberofjumps, int numberofstarbases) {
  if (entitytype==JUMP){
    MakeJump(radius);
    return;
  }
  if (texturename.length()==0) // FIXME?
    return;
  Vector RR,SS;
  Vector center=generateAndUpdateRS (RR,SS,radius,false);
  string thisname;
  thisname=getRandName(names);
  Tab();
  static string default_atmosphere=vs_config->getVariable("galaxy","DefaultAtmosphereTexture","sol/earthcloudmaptrans.png");
  string atmosphere=_Universe->getGalaxy()->getPlanetVariable(texturename,"atmosphere","false");
  if (atmosphere=="false") {
    atmosphere="";
  } else if (atmosphere=="true") {
	atmosphere=default_atmosphere;
  }
  string cname;
  string planetlites=_Universe->getGalaxy()->getPlanetVariable(texturename,"lights","");
  if (!planetlites.empty()) {
    planetlites=' '+planetlites;
    std::vector<std::string::size_type> lites;
    lites.push_back(0);
    while(lites.back()!=std::string::npos) {
		lites.push_back(planetlites.find(lites.back()+1,' '));
    }
	unsigned randomnum=rnd(0,lites.size()-1);
    cname=planetlites.substr(lites[randomnum]+1,lites[randomnum+1]);
  }
  f.Fprintf ("<Planet name=\"%s\" file=\"%s\" ",thisname.c_str(),texturename.c_str());
  f.Fprintf ("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",RR.i,RR.j,RR.k,SS.i,SS.j,SS.k);
  f.Fprintf ("radius=\"%f\" ",radius);
  f.Fprintf ("x=\"%f\" y=\"%f\" z=\"%f\" ",center.i,center.j,center.k);
  float loy = LengthOfYear(RR,SS);
  float temprandom=.1*fmod(loy,10);//use this so as not to alter state here
  if (loy) {
    f.Fprintf ("year= \"%f\" ",loy);
    temprandom=grand();
    loy =864*temprandom;
    if (loy) {
      f.Fprintf ( "day=\"%f\" ",loy);
    }
  }
  f.Fprintf (">\n");
  xmllevel++;
  if (!cname.empty()) {
    
    int wrapx=1;
    int wrapy=1;
    cname = GetWrapXY(cname,wrapx,wrapy);
    string::size_type t;
    while ((t=cname.find('*'))!=string::npos) {
      cname.replace(t,1,texturenum==0?"":XMLSupport::tostring(texturenum));
    }
    Tab();f.Fprintf ("<CityLights file=\"%s\" wrapx=\"%d\" wrapy=\"%d\"/>\n",cname.c_str(),wrapx,wrapy);
  }
  static float atmosphere_prob=XMLSupport::parse_float(vs_config->getVariable("galaxy","AtmosphereProbability","0.5"));
  if ((entitytype==PLANET&&temprandom<atmosphere_prob)&&(!atmosphere.empty())) {
    string NAME = thisname+" Atmosphere";
	{
	bool doalphaatmosphere = (temprandom<.08||temprandom>.3);
	if (doalphaatmosphere) {
		float fograd=radius*1.007;
		if (.007*radius>2500.0)
			fograd=  radius+2500.0;
		Tab();f.Fprintf ("<Atmosphere file=\"%s\" alpha=\"SRCALPHA INVSRCALPHA\" radius=\"%f\"/>\n",atmosphere.c_str(),fograd);
	}

	float r=.9,g=.9,b=1,a=1;
	float dr=.9,dg=.9,db=1,da=1;
	if (!doalphaatmosphere) {
		if (temprandom>.26||temprandom<.09) {
			r=.5;g=1;b=.5;
		}else if (temprandom>.24||temprandom<.092) {
			r=1;g=.6;b=.5;
		}
	}
//	static float concavity = XMLSupport::parse_float (vs_config->getVariable ("graphics","fog","concavity","0"));
//	static float focus = XMLSupport::parse_float (vs_config->getVariable ("graphics","fog","focus",".5"));


/*----------------------------------------------------------------------------------------*\
| **************************************************************************************** |
| ********************************* FIXME: USE BFXM  ************************************* |
| **************************************************************************************** |
\*----------------------------------------------------------------------------------------*/
	
	Tab();f.Fprintf ("<Fog>\n");
	xmllevel++;
	Tab();f.Fprintf ("<FogElement file=\"atmXatm.bfxm\" ScaleAtmosphereHeight=\"1.0\"  red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"%f\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"%f\" concavity=\".3\" focus=\".6\" minalpha=\"0\" maxalpha=\"0.7\"/>\n",r,g,b,a,dr,dg,db,da);
	Tab();f.Fprintf ("<FogElement file=\"atmXhalo.bfxm\" ScaleAtmosphereHeight=\"1.0\"  red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"%f\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"%f\" concavity=\"1\" focus=\".6\" minalpha=\"0\" maxalpha=\"0.7\"/>\n",r,g,b,a,dr,dg,db,da);
	xmllevel--;
	Tab();f.Fprintf ("</Fog>\n");
	}
  }

//  FIRME: need scaling of radius based on planet type.
//  radii.push_back (entitytype!=GAS?radius:1.4*radius);
  radii.push_back (radius);

  static float ringprob = XMLSupport::parse_float (vs_config->getVariable ("galaxy","RingProbability",".1"));
  static float dualringprob = XMLSupport::parse_float (vs_config->getVariable ("galaxy","DoubleRingProbability",".025"));
  if (entitytype==PLANET) {
    float ringrand = grand();
    if (ringrand<ringprob) {
      string ringname = getRandName(rings);
      static float innerRingRadMin = XMLSupport::parse_float (vs_config->getVariable ("galaxy","InnerRingRadius","1.5"));
      static float outerRingRadMin = XMLSupport::parse_float (vs_config->getVariable ("galaxy","OuterRingRadius","2.5"));
      double inner_rad= (innerRingRadMin*(1+grand()*.5))*radius;
      double outer_rad = inner_rad+(outerRingRadMin*grand())*radius;
      int wrapx=1;
      int wrapy=1;
      if (ringname.empty()) {
	ringname = vs_config->getVariable ("galaxy","DefaultRingTexture","planets/ring.pngwrapx36wrapy2");
      }
      ringname = GetWrapXY(ringname,wrapx,wrapy);
      if (ringrand<(1-dualringprob)) {
		  Tab();f.Fprintf ("<Ring file=\"%s\" innerradius=\"%f\" outerradius=\"%f\"  wrapx=\"%d\" wrapy=\"%d\" />\n",ringname.c_str(),inner_rad,outer_rad,wrapx, wrapy);
      }
      if (ringrand<dualringprob||ringrand>=(ringprob-dualringprob)){
	Vector r,s;
	makeRS(r,s,1,false);
	float rmag = r.Mag();
	if (rmag>.001) {
	  r.i/=rmag;  r.j/=rmag;  r.k/=rmag;
	  
	}
	float smag = s.Mag();
	if (smag>.001){
	  s.i/=smag;  s.j/=smag;  s.k/=smag;
	}
	double movable = grand();
	static float second_ring_move = XMLSupport::parse_float (vs_config->getVariable ("galaxy","SecondRingDifference",".4"));
	inner_rad *= (1-.5*second_ring_move)+second_ring_move*movable;
	outer_rad *= (1-.5*second_ring_move)+second_ring_move*movable;

	Tab();f.Fprintf ("<Ring file=\"%s\" ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" innerradius=\"%f\" outerradius=\"%f\" wrapx=\"%d\" wrapy=\"%d\" />\n",ringname.c_str(),r.i,r.j,r.k,s.i,s.j,s.k,inner_rad,outer_rad, wrapx, wrapy);
      }
    }
    //    WriteUnit ("unit","","planetary-ring",Vector (0,0,0), Vector (0,0,0), Vector (0,0,0), string (""), string (""),false);
  }
  for (int i=0;i<numberofstarbases;i++) {
    MakeSmallUnit ();
  }
  static float moon_size_compared_to_planet = XMLSupport::parse_float (vs_config->getVariable ("galaxy","MoonRelativeToPlanet",".4"));
  static float moon_size_compared_to_moon = XMLSupport::parse_float (vs_config->getVariable ("galaxy","MoonRelativeToMoon",".8"));
  moonlevel++;
  MakeMoons (entitytype!=MOON?moon_size_compared_to_planet*radius:moon_size_compared_to_moon*radius,entitytype);
  MakeJumps (100+grand()*300,entitytype,numberofjumps);
  moonlevel--;
  radii.pop_back();
  xmllevel--;
  Tab();f.Fprintf ("</Planet>\n"); 

  // writes out some pretty planet tags
}


void MakeJumps (float callingradius, int callingentitytype, int numberofjumps) {
  for (int i=0;i<numberofjumps;i++) {
    MakeJump ((.5+.5*grand())*callingradius);
  }
}
void MakeMoons (float callingradius, int callingentitytype) {
  while(planetoffset<stars[staroffset].planets.size()&&stars[staroffset].planets[planetoffset].moonlevel==moonlevel) {
    PlanetInfo &infos=stars[staroffset].planets[planetoffset++];
    MakePlanet((.5+.5*grand())*callingradius, callingentitytype==STAR?PLANET:MOON, infos.name, infos.num, infos.numjumps, infos.numstarbases);
  }
}
void beginStar () {
  float radius=starradius[staroffset];
  Vector r,s;
  int i;
  //Vector center=generateAndUpdateRS (r,s,radius);
  planetoffset=0;

  char b[3]=" A";
  b[1]+=staroffset;
  Tab();f.Fprintf ("<Planet name=\"%s%s\" file=\"%s\" ",systemname.c_str(),b,starentities[staroffset].c_str());
  f.Fprintf ("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",r.i,r.j,r.k,s.i,s.j,s.k);
  f.Fprintf ("radius=\"%f\" x=\"0\" y=\"0\" z=\"0\" ",radius);
  float loy = LengthOfYear(r,s);
  if (loy) {
    f.Fprintf ("year= \"%f\" ",loy);
    loy *=grand();
    if (loy) {
      f.Fprintf ("day=\"%f\" ",loy);
    }
  }
  f.Fprintf (" Red=\"%f\" Green=\"%f\" Blue=\"%f\" ReflectNoLight=\"true\" light=\"%d\">\n",lights[staroffset].r,lights[staroffset].g,lights[staroffset].b,staroffset);
  radii.push_back (1.5*radius);
  static float planet_size_compared_to_sun = XMLSupport::parse_float (vs_config->getVariable ("galaxy","RockyRelativeToPrinary",".05"));
//  static float gas_size_compared_to_sun = XMLSupport::parse_float (vs_config->getVariable ("galaxy","GasRelativeToPrinary",".2"));
  xmllevel++;
  int numu;
  if (numstarentities) {
    numu= numnaturalphenomena/(numstarentities-staroffset)+(grand()<float(numnaturalphenomena%(numstarentities-staroffset))/(numstarentities-staroffset));
  } else {
    numu=1;
  }
  if ((int)staroffset==(int)(numstarentities-staroffset)-1) {
    numu=numnaturalphenomena;
  }
  for (i=0;i<numu;i++) {
    MakeBigUnit(STAR);
  }
  for (i=0;i<stars[staroffset].numstarbases;i++) {
    MakeSmallUnit ();
  }
  MakeJumps (100+grand()*300,STAR,stars[staroffset].numjumps);
  MakeMoons (planet_size_compared_to_sun*radius,STAR);
  // Fixme: no jumps should be made around the star.
  if (!jumps.empty()) {
    VSFileSystem::vs_fprintf(stderr,"ERROR: jumps not empty() Size==%u!!!!!\n",jumps.size());
  }
  staroffset++;
  /*
  //  No jump for you!!

  MakeJumps (100+grand()*300,STAR);
  if ((int)staroffset+1>=nument[STAR]) {
    while (!jumps.empty()) {
      MakeJumps (300+grand()*800,STAR);
    }
  }
  */
}

void endStar () {
  radii.pop_back();
  xmllevel--;
  Tab();f.Fprintf ("</Planet>\n");
}
void CreateStar () {
  beginStar ();
  endStar();
}
void CreateFirstStar(){
  beginStar();
  while (staroffset<numstarentities) {
    if (grand()>.5) {
      CreateFirstStar();
      break;
    } else {
      CreateStar ();
    }
  }
  endStar();
}

void CreatePrimaries () {
  unsigned int i;
   for (i=0;i<numstarentities||i==0;i++) {
     CreateLight(i);
   }
   /*
  int numprimaryunits=rnd(0,1+naturalphenomena.size());
  for (i=0;i<numprimaryunits;i++) {
    MakeBigUnit(STAR);
  }
  */
  CreateFirstStar();

}

void CreateStarSystem () {
  assert (!starradius.empty());
  assert (starradius[0]);
  xmllevel=0;
  Tab();f.Fprintf ("<?xml version=\"1.0\" ?>\n<system name=\"%s\" background=\"%s\">\n",systemname.c_str(),getRandName(background).c_str());
  xmllevel++;
  CreatePrimaries ();
  xmllevel--;
  Tab();f.Fprintf ("</system>\n");
}


void readentity (vector <string> & entity,const char * filename) {
  VSFile f;
  VSError err = f.OpenReadOnly( filename, UniverseFile);
  if (err>Ok) {
    return;
  }
  ///warning... obvious vulnerability
  char input_buffer[1000];
  while (1==f.Fscanf ("%s", input_buffer)) {
    entity.push_back (input_buffer);
  }
  f.Close();
}


const char * noslash (const char * in) {
  const char * tmp=in;
  while (*tmp!='\0'&&*tmp!='/') {
    tmp++;
  }
  if (*tmp!='\0') {
    tmp++;
  } else {
    return in;
  }
  const char * tmp2=tmp;
  tmp2=noslash (tmp2);
  if (tmp2[0]!='\0') {
    return tmp2;
  }
  else
    return tmp;
}
}
using namespace StarSystemGent;
string getStarSystemFileName (string input) {
  return input+string (".system");

}
string getStarSystemName (string in) {
  return string(noslash (in.c_str()));
}
string getStarSystemSector (string in) {

  char * tmp = strdup (in.c_str());
  char * freer=tmp;
  bool found=false;
  while (*tmp) {
    if (*tmp=='/') {
      found=true;
      break;
    }
    tmp++;
  }
  if (*tmp=='/') {
    *tmp='\0';
    tmp++;
  }
  string sectorname (freer);
  free (freer);
  if (found)
    return sectorname; 
  else
    return string (".");
}
void readnames (vector <string> &entity, const char * filename) {
  VSFile f;
  VSError err = f.OpenReadOnly( filename, UniverseFile);
  if (err>Ok) {
    return;
  }
  ///warning... obvious vulnerability
  char input_buffer[1000];
  while (!f.Eof()) {
    f.ReadLine (input_buffer,999);
    if (input_buffer[0]=='\0'||input_buffer[0]=='\n'||input_buffer[0]=='\r')
      continue;
    for (unsigned int i=0;input_buffer[i]!='\0'&&i<999;i++) {
      if (input_buffer[i]=='\r') {
	input_buffer[i]='\0';
      }
      if (input_buffer[i]=='\n') {
	input_buffer[i]='\0';
	break;
      }
    }
    entity.push_back (input_buffer);
  }
  f.Close();

}

void readplanetentity(vector <StarInfo> &starinfos, std::string planetlist, unsigned int numstars) {
  if (numstars<1) {
    numstars=1;
    vs_fprintf(stderr,"No stars exist in this system!\n");
  }
  string::size_type i,j;
  unsigned int u;
  starinfos.reserve(numstars);
  for (u=0;u<numstars;++u)
    starinfos.push_back(StarInfo());
  u--;
  while (i=planetlist.find(' '),1) {
    if (i==0) {
      planetlist=planetlist.substr(1);
      continue;
    }
    int nummoon=0;
    for (j=0;j<i&&j<planetlist.size()&&planetlist[j]=='*';++j) {
      nummoon++;
    }
    if (nummoon==0)
      u++;
    if (j==string::npos||j>=planetlist.size()) break;
    starinfos[u%numstars].planets.push_back(PlanetInfo());
    starinfos[u%numstars].planets.back().moonlevel=nummoon;
    {
      GalaxyXML::Galaxy *galaxy=_Universe->getGalaxy();
      string planetname=galaxy->getPlanetNameFromInitial(planetlist.substr(j,i==string::npos?string::npos:i-j));
      string texturename=galaxy->getPlanetVariable(planetname,"texture","No texture supplied in <planets>!");
      starinfos[u%numstars].planets.back().num=rnd(XMLSupport::parse_int(galaxy->getPlanetVariable(planetname,"texture_min","0")),XMLSupport::parse_int(galaxy->getPlanetVariable(planetname,"texture_max","0")));
      string ext=galaxy->getPlanetVariable(planetname,"texture_ext","png");
      starinfos[u%numstars].planets.back().name=texturename+(starinfos[u%numstars].planets.back().num==0?"":XMLSupport::tostring(starinfos[u%numstars].planets.back().num))+'.'+ext;
      // should now have texname[min-max].png
    }
    starinfos[u%numstars].planets.back().numstarbases=0;
    starinfos[u%numstars].planets.back().numjumps=0;
    if (i==string::npos) break;
    planetlist=planetlist.substr(i+1);
  }
  unsigned int k;
  if (starinfos.size()) {
    bool size=0;
    for (k=0;k<starinfos.size();++k) {
      if (starinfos[k].planets.size()) {
        size=true;
        break;
      }
    }
    if (!size) {
      int oldjumps=jumps.size();
      int oldstarbases=numstarbases;
      int newstuff;
      for (k=starinfos.size();k>0;--k) {
        newstuff=oldjumps/k;
        starinfos[k-1].numjumps=newstuff;
        oldjumps-=newstuff;
        newstuff=oldstarbases/k;
        starinfos[k-1].numstarbases=newstuff;
        oldstarbases-=newstuff;
      }
    } else {
      for (k=0;k<jumps.size();++k) {
        vector<PlanetInfo> *temp; // & doesn't like me so I use *.
        do {
          temp=&starinfos[rnd(0,starinfos.size())].planets;
        } while (!temp->size());
        (*temp)[rnd(0,temp->size())].numjumps++;
      }
      for (k=0;k<numstarbases;++k) {
        vector<PlanetInfo> *temp; // & appears to still have dislike for me.
        do {
          temp=&starinfos[rnd(0,starinfos.size())].planets;
        } while (!temp->size());
        (*temp)[rnd(0,temp->size())].numstarbases++;
      }
    }
  }
}

static int pushDown (int val) {
  while (grand()>(1/val)) {
    val--;
  }
  return val;
}
static int pushDownTowardsMean (int mean, int val) {
  int delta = mean -1;
  return delta + pushDown (val-delta);
}
static int pushTowardsMean (int mean, int val) {
	static bool dopushingtomean = XMLSupport::parse_bool (vs_config->getVariable("galaxy","PushValuesToMean","true"));
	if (!dopushingtomean)
		return val;
  if (val < mean) {
    return -pushDownTowardsMean (-mean,-val);
  }
  return pushDownTowardsMean (mean,val);
}


//void generateStarSystem (string datapath, int seed, string sector, string system, string outputfile, float sunradius, int numstars, bool nebulae, bool asteroids, int numnaturalphenomena, int numstarbases, string factions, const vector <string> &jumplocations, string namelist, string starlist, string planetlist, string smallunitlist, string nebulaelist, string asteroidlist,string ringlist, string backgroundlist) {
void generateStarSystem(SystemInfo &si) {
  ResetGlobalVariables();
  static float radiusscale= XMLSupport::parse_float (vs_config->getVariable("galaxy","StarRadiusScale","1000"));
  si.sunradius *=radiusscale;
  systemname=si.name;
  static float compactness_scale = XMLSupport::parse_float (vs_config->getVariable("galaxy","CompactnessScale","1.5"));
  static float jump_compactness_scale = XMLSupport::parse_float (vs_config->getVariable("galaxy","JumpCompactnessScale","1.5"));
  //  static int meansuns = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanSuns","1.5"));
  static int meannaturalphenomena = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanNaturalPhenomena","1"));
  static int meanbases = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanStarBases","2"));
  compactness = si.compactness*compactness_scale;
  jumpcompactness = si.compactness*jump_compactness_scale;  
  if (si.seed)
    seedrand (si.seed);
  else
    seedrand (stringhash(si.sector+'/'+si.name));
  VSFileSystem::vs_fprintf (stderr,"star %d, natural %d, bases %d",si.numstars,si.numun1,si.numun2); 
  int nat = pushTowardsMean(meannaturalphenomena,si.numun1);
  numnaturalphenomena= nat>si.numun1?si.numun1:nat;
  numstarbases=pushTowardsMean(meanbases,si.numun2);
  static float smallUnitsMultiplier= XMLSupport::parse_float (vs_config->getVariable ("galaxy","SmallUnitsMultiplier","0"));
  numstarbases=(int)(si.numun2*smallUnitsMultiplier);
  numstarentities=si.numstars;
  VSFileSystem::vs_fprintf (stderr,"star %d, natural %d, bases %d",numstarentities,numnaturalphenomena,numstarbases); 
  starradius.push_back (si.sunradius);
  readColorGrads (gradtex,(si.stars).c_str());

  readentity (starbases,(si.smallun).c_str());
  readentity (background,(si.backgrounds).c_str());
  
  if (background.empty()) {
    background.push_back (si.backgrounds);
  }
  if (si.nebulae) {
    readentity (naturalphenomena,(si.nebulaelist).c_str());
  }
  if (si.asteroids) {
    readentity (naturalphenomena,(si.asteroidslist).c_str());
  }
  for (unsigned int i=0;i<si.jumps.size();i++) {
    jumps.push_back (si.jumps[i]);
  }
  faction = si.faction;

  readplanetentity (stars,si.planetlist,numstarentities);

  readentity(rings,(si.ringlist).c_str());
  readnames (names,(si.names).c_str());

  CreateDirectoryHome( VSFileSystem::sharedsectors+"/"+VSFileSystem::universe_name+"/"+si.sector);

  VSError err = f.OpenCreateWrite( si.filename, SystemFile);
  if (err<=Ok) {
		  CreateStarSystem();
  		  f.Close();
  }
  ResetGlobalVariables(); // deallocate any unused memory in vectors.
}
#ifdef CONSOLE_APP
int main (int argc, char ** argv) {

  if (argc<9) {
    VSFileSystem::vs_fprintf (stderr,"Usage: starsysgen <seed> <sector>/<system> <sunradius>/<compactness> <numstars> [N][A]<numnaturalphenomena> <numstarbases> <faction> <namelist> [OtherSystemJumpNodes]...\n");
    return 1;
  }
  int seed;
  if (1!=sscanf (argv[1],"%d",&seed)) {
    return 1;
  }
  string sectorname (getStarSystemSector(argv[2]));
  string filen (getStarSystemFileName (argv[2]));
  string systemname=string(getStarSystemName ( argv[2]));
  int numbigthings;
  bool nebula=true;
  bool asteroid=true;
  float srad;
  float comp;
  sscanf (argv[3],"%f/%f", &srad &comp);
  vector <string> jumps;
  for (unsigned int i=12;i<argc;i++) {
    jumps.push_back (string(argv[i]));
    
  }
  if (1==sscanf(argv[8],"AN%d",&numbigthings)||1==sscanf (argv[8],"NA%d",&numun[0])) {
    nebula=asteroid=true;
  } else if (1==sscanf (argv[8],"A%d",&numbigthings)) {
    asteroid=true;
  } else if (1==sscanf (argv[8],"N%d",&numbigthings)) {
    nebula=true;
  }else if (1==sscanf (argv[8],"%d",&numbigthings)) {
    nebula=asteroid=true;
  } else {
    return -1;
  }

  generateStarSystem ("./",
		      seed,
		      sectorname,
		      systemname,
		      filen,
		      srad,comp;
		      strtol (argv[4],NULL,10),
		      strtol (argv[5],NULL,10),
		      strtol (argv[6],NULL,10),
		      strtol (argv[7],NULL,10),
		      nebula,
		      asteroid,
		      numbigthings,
		      strtol (argv[9],NULL,10),
		      argv[10],
		      argv[11],
		      jumps);

  return 0;
}
#endif
