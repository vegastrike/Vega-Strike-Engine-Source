
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
#include "vs_random.h"
#ifndef _WIN32
#include <ctype.h>
#endif
#ifndef M_PI
#define M_PI 3.1415926536
#endif
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
    unsigned int pos =wrap.find ("wrapx");
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
	  
	  // ( lower+ (((float(upper-lower))*ssrand())/(float (RAND_MAX)+1.))); WE THINK THIS IS BUGGY
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
  return float (ssrand())/RAND_MAX;
}
vector <Color> lights;
FILE * fp =NULL;

float difffunc (float inputdiffuse) {
  return sqrt(((inputdiffuse)));
}
void WriteLight (unsigned int i) {
  static float ambientColorFactor= XMLSupport::parse_float (vs_config->getVariable("galaxy","AmbientLightFactor","0"));
  float ambient =(lights[i].r+lights[i].g+lights[i].b);
  
  ambient*=ambientColorFactor;
  fprintf (fp,"<Light>\n\t<ambient red=\"%f\" green=\"%f\" blue=\"%f\"/>\n\t<diffuse red=\"%f\" green=\"%f\" blue=\"%f\"/>\n\t<specular red=\"%f\" green=\"%f\" blue=\"%f\"/>\n</Light>\n",
	   ambient,ambient,ambient,
	   difffunc (lights[i].r),difffunc (lights[i].g),difffunc (lights[i].b),
	   lights[i].nr,lights[i].ng,lights[i].nb);   

}
struct GradColor {
  float minrad;
  float r;
  float g;
  float b;
  float variance;
};

const int PLANET = 2;
const int GAS = 1;
const int STAR = 0;
const int MOON=3;
const int JUMP=4;
  //begin global variables


vector <string> entities[5];
vector <string> planet_entities;
vector <string> gradtex;
int nument[5];
int numun[2];
vector <string> units [2];
vector <string> background;
vector <string> names;
vector <string> rings;
const float moonofmoonprob=.01;
string systemname;
vector <float> radii;
const float minspeed=.001;
const float maxspeed=8;
vector <float> starradius;
string faction;
vector <GradColor> colorGradiant;
  float compactness=2;
  float jumpcompactness=2;
  void ResetGlobalVariables () {
    lights.clear();
    gradtex.clear();
    numun[0]=numun[1]=nument[STAR]=nument[GAS]=nument[PLANET]=nument[MOON]=nument[JUMP]=0;
    units[0].clear();
    units[1].clear();
    entities[STAR].clear();
    entities[GAS].clear();
    entities[PLANET].clear();entities[MOON].clear();
	planet_entities.clear();
    entities[JUMP].clear();
    background.clear();
    names.clear();
    rings.clear();
    systemname=string ("");
    starradius.clear();
    faction=string ("");
    colorGradiant.clear();
    compactness=2;    jumpcompactness=2;
    radii.clear();

  }

void Tab () {
  for (unsigned int i=0;i<radii.size();i++) {
    fprintf (fp,"\t");
  }

}
 


void readColorGrads (vector <string> &entity,const char * file) {
  FILE * fp = fopen (file,"r");
  static float radiusscale= XMLSupport::parse_float (vs_config->getVariable("galaxy","StarRadiusScale","50"));

  if (!fp) {
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
  while (!feof (fp)) {
    fgets (input_buffer,999,fp);
    if (sscanf (input_buffer,"%f %f %f %f %f %s",&g.minrad,&g.r,&g.g,&g.b,&g.variance,output_buffer)==6) {
      g.minrad *=radiusscale;
	  colorGradiant.push_back (g);
      entity.push_back (output_buffer);
    }
  }
  fclose (fp);
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
	  string fullpath = vs_config->getVariable("data","universe_path","universe")+"/stars.txt";
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
	static float radiusscale= XMLSupport::parse_float (vs_config->getVariable("galaxy","StarRadiusScale","50"));	
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
  entities[STAR].push_back (gradtex[gradindex]);
  float h = lights.back().r;
  if (h<lights.back().g) h=lights.back().g;
  if (h<lights.back().b) h=lights.back().b;
  float norm = (.5+.5/nument[0]);
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

vector <string> parseBigUnit (string input) {
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


void WriteUnit(string tag, string name, string filename, Vector r, Vector s, Vector center, string nebfile, string destination, bool faction, float thisloy=0) {
  Tab();
  fprintf (fp,"<%s name=\"%s\" file=\"%s\" ",tag.c_str(),name.c_str(),filename.c_str());
  if (nebfile.length()>0) {
    fprintf (fp,"nebfile=\"%s\" ",nebfile.c_str());
  }

  fprintf (fp,"ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",r.i,r.j,r.k,s.i,s.j,s.k);
  fprintf (fp,"x=\"%f\" y=\"%f\" z=\"%f\" ",center.i,center.j,center.k);
  float loy = LengthOfYear(r,s);
  if (loy||thisloy) {
    fprintf (fp,"year= \"%f\" ",thisloy?thisloy:loy);
  }
  if (destination.length()) {
    fprintf (fp, "destination=\"%s\" ",destination.c_str()); 
  } else if (faction){
    fprintf (fp,"faction=\"%s\" ",StarSystemGent::faction.c_str());
  }
  fprintf (fp," />\n");
}
string getJumpTo (string s) {
  char tmp[1000]="";
  if (1==sscanf (s.c_str(),"JumpTo%s",tmp)){
    tmp[0]=tolower(tmp[0]);
  }
  else return s;
  return string (tmp);
}
string starin (string input) {
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
  return string ("");
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
  return string ("");
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
  Vector r,S;

  string nam;
  string s=string ("");
  while (s.length()==0) {
    nam= getRandName (units[1]);
    if (nam.length()==0)
      return;
    string tmp;
    if ((tmp=starin(nam)).length()>0) {
      nam=(tmp);
      s = getRandName (entities[JUMP]);
    }else {
      break;
    }
  }

  string nebfile ("");
  float radius;
  string type = AnalyzeType(nam,nebfile,radius);
  Vector center=generateAndUpdateRS(r,S,radius,true);

  WriteUnit (type,"",nam,r,S,center,nebfile,s,true);

}



void MakePlanet(float radius, int entitytype, int callingentitytype,bool forceRS=false, Vector R=Vector (0,0,0), Vector S=Vector (0,0,0), Vector center=Vector (0,0,0), float loy=0);
void MakeBigUnit (int callingentitytype, string name=string (""),float orbitalradius=0) {
  vector <string> fullname;
  if (name.length()==0) {
    string s= getRandName (units[0]);
    if (s.length()==0)
      return;
    fullname = parseBigUnit(s);
  }else {
    fullname.push_back( name);
  }
  if (fullname.empty())
    return;
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
      MakePlanet(size,JUMP,callingentitytype,true,r,s,center,stdloy);
    }else if (1==sscanf (fullname[i].c_str(),"planet%f",&size)) {
      if (!first) {
	first= true;
	center=generateAndUpdateRS (r,s,size,false);
	stdloy=LengthOfYear (r,s);	
      }
      MakePlanet(size,PLANET,callingentitytype,true,r,s,center,stdloy);
    }else if (1==sscanf (fullname[i].c_str(),"moon%f",&size)) {      
      if (!first) {
	first= true;
	center=generateAndUpdateRS (r,s,size,false);
	stdloy=LengthOfYear (r,s);	
      }
      MakePlanet (size,MOON,callingentitytype,true,r,s,center,stdloy);
    } else if (1==sscanf (fullname[i].c_str(),"gas%f",&size)) {
      if (!first) {
	first= true;
	center=generateAndUpdateRS (r,s,size,false);
	stdloy=LengthOfYear (r,s);	
      }
      MakePlanet (size,GAS,callingentitytype,true,r,s,center,stdloy);
    }else if ((tmp=starin(fullname[i])).length()>0) {
      if (!first) {
	first= true;
	center=generateAndUpdateRS (r,s,size,callingentitytype!=STAR);
	stdloy=LengthOfYear (r,s);	
      }
      string S = getRandName (entities[JUMP]);
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
void MakeMoons (float radius, int entitytype, int callingentitytype, bool forceone=false);
void MakePlanet(float radius, int entitytype, int callingentitytype, bool forceRS, Vector R, Vector S, Vector center, float thisloy) {
  string s =  getRandName (entities[entitytype]);
  if (s.length()==0)
    return;
  Vector r,SS;
  if (forceRS) {
    r=R;SS=S;
    Updateradii (mmax(r.Mag(),SS.Mag()),radius);
  }else {
    center=generateAndUpdateRS (r,SS,radius,entitytype==JUMP);
  }
  string thisname;
  if (entitytype!=JUMP) {
    thisname=getRandName(names);
  }else {
    thisname = string("JumpTo")+getStarSystemName(s);
    if (thisname.length()>6) {
      *(thisname.begin()+6)=toupper(*(thisname.begin()+6));
    }
  }
  Tab();
  unsigned int atmos = s.find ("~");
  string atmosphere;
  if (atmos!=string::npos) {
    atmosphere = s.substr (atmos+1,s.length());
    if (atmosphere.empty()) {
      atmosphere = "sol/earthcloudmaptrans.png";
    }
    s = s.substr (0,atmos);
  }
  unsigned int pos = s.find ("^");
  string cname;
  if (pos==string::npos||entitytype==JUMP) {
    fprintf (fp,"<Planet name=\"%s\" file=\"%s\" ",thisname.c_str(),entitytype==JUMP?"jump.png":s.c_str());
  }else {
    string pname= s.substr (0,pos);
    cname = s.substr (pos+1,s.length());
    fprintf (fp,"<Planet name=\"%s\" file=\"%s\" ",thisname.c_str(),pname.c_str());
  }
  fprintf (fp,"ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",r.i,r.j,r.k,SS.i,SS.j,SS.k);
  fprintf (fp,"radius=\"%f\" ",radius);
  fprintf (fp,"x=\"%f\" y=\"%f\" z=\"%f\" ",center.i,center.j,center.k);
  float loy = LengthOfYear(r,SS);
  float temprandom=.1*fmod(loy,10);//use this so as not to alter state here
  if (loy||thisloy) {
    fprintf (fp,"year= \"%f\" ",thisloy?thisloy:loy);
    temprandom=grand();
    loy =864*temprandom;
    if (loy) {
      fprintf (fp, "day=\"%f\" ",loy);
    }
  }
  if (entitytype==JUMP) {
    fprintf (fp, "alpha=\"ONE ONE\" destination=\"%s\" faction=\"%s\" ",getJumpTo(s).c_str(),StarSystemGent::faction.c_str());

  }
  fprintf (fp," >\n");
  if (!cname.empty()) {
    
    int wrapx=1;
    int wrapy=1;
    cname = GetWrapXY(cname,wrapx,wrapy);
    Tab();fprintf (fp,"<CityLights file=\"%s\" wrapx=\"%d\" wrapy=\"%d\"/>\n",cname.c_str(),wrapx,wrapy);
  }
  if (entitytype==PLANET) {
	  entities[PLANET]=planet_entities;//this will make it so that while you have at least one habitable planet, not all ma
  }
  if ((entitytype==PLANET&&temprandom<.1)||(!atmosphere.empty())) {
    if (atmosphere.empty()) {
      atmosphere="sol/earthcloudmaptrans.png";
    }
    string NAME = thisname+" Atmosphere";
	{
	bool doalphaatmosphere = (temprandom<.08||temprandom>.3);
	if (doalphaatmosphere) {
		float fograd=radius*1.007;
		if (.007*radius>2500.0)
			fograd=  radius+2500.0;
		Tab();fprintf (fp,"<Atmosphere file=\"%s\" alpha=\"SRCALPHA INVSRCALPHA\" radius=\"%f\"/>\n",atmosphere.c_str(),fograd);
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
	Tab();fprintf (fp,"<Fog>\n");
	Tab();Tab();fprintf (fp,"<FogElement file=\"atmXatm.xmesh\" ScaleAtmosphereHeight=\"1.0\"  red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"%f\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"%f\" concavity=\".3\" focus=\".6\" minalpha=\"0\" maxalpha=\"0.7\"/>\n",r,g,b,a,dr,dg,db,da);
	Tab();Tab();fprintf (fp,"<FogElement file=\"atmXhalo.xmesh\" ScaleAtmosphereHeight=\"1.0\"  red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"%f\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"%f\" concavity=\"1\" focus=\".6\" minalpha=\"0\" maxalpha=\"0.7\"/>\n",r,g,b,a,dr,dg,db,da);		
	Tab();fprintf (fp,"</Fog>\n");
	}
  }

  radii.push_back (entitytype!=GAS?radius:1.4*radius);
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
	fprintf (fp,"<Ring file=\"%s\" innerradius=\"%f\" outerradius=\"%f\"  wrapx=\"%d\" wrapy=\"%d\" />\n",ringname.c_str(),inner_rad,outer_rad,wrapx, wrapy);
      }
      if (ringrand<dualringprob||ringrand>=(ringprob-dualringprob)){
	Vector r,s;
	makeRS(r,s,1,entitytype==JUMP);
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

	fprintf (fp,"<Ring file=\"%s\" ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" innerradius=\"%f\" outerradius=\"%f\" wrapx=\"%d\" wrapy=\"%d\" />",ringname.c_str(),r.i,r.j,r.k,s.i,s.j,s.k,inner_rad,outer_rad, wrapx, wrapy);
      }
    }
    //    WriteUnit ("unit","","planetary-ring",Vector (0,0,0), Vector (0,0,0), Vector (0,0,0), string (""), string (""),false);
  }
  if (entitytype!=JUMP&&(entitytype!=MOON||grand()<moonofmoonprob)) {
    int numu;
    if (entitytype==MOON||(nument[PLANET]+nument[GAS]==0)) {
	numu=1;
    }else {
      numu = numun[1]/(nument[PLANET]+nument[GAS])+ (grand()*(nument[PLANET]+nument[GAS])<(numun[1]%(nument[PLANET]+nument[GAS]))?1:0);
    }
    for (int i=0;i<numu;i++) {
      MakeSmallUnit ();
    }
  static float moon_size_compared_to_planet = XMLSupport::parse_float (vs_config->getVariable ("galaxy","MoonRelativeToPlanet",".4"));
  static float moon_size_compared_to_moon = XMLSupport::parse_float (vs_config->getVariable ("galaxy","MoonRelativeToMoon",".8"));
	
    MakeMoons ((entitytype!=JUMP&&entitytype!=MOON)?moon_size_compared_to_planet*radius:moon_size_compared_to_moon*radius,MOON,entitytype,entitytype==JUMP||entitytype==MOON);
    MakeMoons (100+grand()*300,JUMP,entitytype,entitytype==JUMP||entitytype==MOON);
  }
  radii.pop_back();

  Tab();fprintf (fp,"</Planet>\n"); 

  ///writes out some pretty planet tags
}

void MakeMoons (float radius, int entitytype, int callingentitytype, bool forceone      ) {
  unsigned int nummoon;
  if (nument[callingentitytype]&&!forceone) {
    nummoon = nument[entitytype]/nument[callingentitytype]+((grand ()*nument[callingentitytype]<(nument[entitytype]%nument[callingentitytype]))?1:0);
  }else {
    nummoon=1;
  }
  for (unsigned int i=0;i<nummoon;i++) {
    MakePlanet ((.5+.5*grand())*radius,entitytype,callingentitytype);
  }
}
void beginStar (float radius, unsigned int which) {
  Vector r,s;
  //Vector center=generateAndUpdateRS (r,s,radius);

  char b[3]=" A";
  b[1]+=which;
  Tab();fprintf (fp,"<Planet name=\"%s\" file=\"%s\" ",(systemname+b).c_str(),entities[0][which].c_str());
  fprintf (fp,"ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",r.i,r.j,r.k,s.i,s.j,s.k);
  fprintf (fp,"radius=\"%f\" x=\"0\" y=\"0\" z=\"0\" ",radius);
  float loy = LengthOfYear(r,s);
  if (loy) {
    fprintf (fp,"year= \"%f\" ",loy);
    loy *=grand();
    if (loy) {
      fprintf (fp, "day=\"%f\" ",loy);
    }
  }
  fprintf (fp," Red=\"%f\" Green=\"%f\" Blue=\"%f\" ReflectNoLight=\"true\" light=\"%d\">\n",lights[which].r,lights[which].g,lights[which].b,which);

  radii.push_back (1.5*radius);
  static float planet_size_compared_to_sun = XMLSupport::parse_float (vs_config->getVariable ("galaxy","RockyRelativeToPrinary",".05"));
  static float gas_size_compared_to_sun = XMLSupport::parse_float (vs_config->getVariable ("galaxy","GasRelativeToPrinary",".2"));

  MakeMoons (planet_size_compared_to_sun*radius,PLANET,STAR);
  MakeMoons (gas_size_compared_to_sun*radius,GAS,STAR);
  int numu;
  if (nument[0]) {
    numu= numun[0]/nument[0]+(grand()<float(numun[0]%nument[0])/nument[0]);
  } else {
    numu=1;
  }
  if ((int)which==nument[0]-1) {
    numu=numun[0];
  }
  numun[0]-=numu;
  for (int i=0;i<numu;i++) {
    MakeBigUnit(STAR);
  }
  MakeMoons (100+grand()*300,JUMP,STAR);
  if ((int)which+1>=nument[STAR]) {
    while (!entities[JUMP].empty()) {
      MakeMoons (300+grand()*800,JUMP,STAR);
    }
  }
}

void endStar () {
  radii.pop_back();
  Tab();fprintf (fp,"</Planet>\n");
}
void CreateStar (float radius, unsigned int which) {
  beginStar (radius,which);
  endStar();
}
void CreateFirstStar(float radius, unsigned int which){
  beginStar(radius,which);
  for (int i=which+1;i<nument[0];i++) {
    if (grand()>.5) {
      CreateFirstStar(starradius[i],i);
      break;
    } else {
      CreateStar (starradius[i],i);
    }
  }
  endStar();
}

void CreatePrimaries (float starradius) {
  int numprimaryunits=rnd(0,1+numun[0]);
  numun[0]-=numprimaryunits;
  int i;
  for (i=0;i<nument[0]||i==0;i++) {
    CreateLight(i);
  }
  for (i=0;i<numprimaryunits;i++) {
    MakeBigUnit(STAR);
  }
  CreateFirstStar(starradius,0);

}

void CreateStarSystem () {
  assert (!starradius.empty());
  assert (starradius[0]);
  fprintf (fp,"<system name=\"%s\" background=\"%s\" nearstars=\"%d\" stars=\"%d\" starspread=\"%d\">\n",systemname.c_str(),getRandName(background).c_str(),500,1000,150);
  CreatePrimaries (starradius[0]);
  fprintf (fp,"</system>\n");
}


void readentity (vector <string> & entity,const char * filename) {
  FILE * fp= fopen (filename,"r");
  if (!fp) {
    return;
  }
  ///warning... obvious vulnerability
  char input_buffer[1000];
  while (1==fscanf (fp,"%s", input_buffer)) {
    entity.push_back (input_buffer);
  }
  fclose (fp);
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
  FILE * fp= fopen (filename,"r");
  if (!fp) {
    return;
  }
  ///warning... obvious vulnerability
  char input_buffer[1000];
  while (!feof (fp)) {
    fgets (input_buffer,999,fp);
    if (input_buffer[0]=='\0'||input_buffer[0]=='\n'||input_buffer[0]=='\r')
      continue;
    for (unsigned int i=0;input_buffer[i]!='\0';i++) {
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
  fclose (fp);

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


void generateStarSystem (string datapath, int seed, string sector, string system, string outputfile, float sunradius, float compac,  int numstars, int numgasgiants, int numrockyplanets, int nummoons, bool nebulae, bool asteroids, int numnaturalphenomena, int numstarbases, string factions, const vector <string> &jumplocations, string namelist, string starlist, string planetlist, string gasgiantlist, string moonlist, string smallunitlist, string nebulaelist, string asteroidlist,string ringlist, string backgroundlist, bool force) {
  ResetGlobalVariables();
  static float radiusscale= XMLSupport::parse_float (vs_config->getVariable("galaxy","StarRadiusScale","1000"));
  sunradius *=radiusscale;
  systemname=system;
  static float compactness_scale = XMLSupport::parse_float (vs_config->getVariable("galaxy","CompactnessScale","1.5"));
  static float jump_compactness_scale = XMLSupport::parse_float (vs_config->getVariable("galaxy","JumpCompactnessScale","1.5"));
  //  static int meansuns = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanSuns","1.5"));
  static int meangas = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanGasGiants","1"));
  static int meanplanets = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanPlanets","5"));
  static int meanmoons = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanMoons","2"));
  static int meannaturalphenomena = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanNaturalPhenomena","1"));
  static int meanbases = XMLSupport::parse_int (vs_config->getVariable("galaxy","MeanStarBases","2"));
  compactness = compac*compactness_scale;
  jumpcompactness = compac*jump_compactness_scale;  
  if (seed)
    seedrand (seed);
  else
    seedrand (stringhash(sector+system));
  fprintf (stderr,"star %d gas %d plan %d moon %d, natural %d, bases %d",numstars,numgasgiants,numrockyplanets,nummoons,numnaturalphenomena,numstarbases); 
  nument[0]=numstars;
  nument[1]=pushTowardsMean(meangas,numgasgiants);
  nument[2]=pushTowardsMean(meanplanets,numrockyplanets);
  nument[3]=pushTowardsMean(meanmoons,nummoons);
  int nat = pushTowardsMean(meannaturalphenomena,numnaturalphenomena);
  numun[0]= nat>numnaturalphenomena?numnaturalphenomena:nat;
  numun[1]=pushTowardsMean(meanbases,numstarbases);
  static float smallUnitsMultiplier= XMLSupport::parse_float (vs_config->getVariable ("galaxy","SmallUnitsMultiplier","0"));
  numun[1]=(int) (numun[1]*smallUnitsMultiplier);
  fprintf (stderr,"star %d gas %d plan %d moon %d, natural %d, bases %d",nument[0],nument[1],nument[2],nument[3],numun[0],numun[1]); 
  starradius.push_back (sunradius);
  readColorGrads (gradtex,(datapath+starlist).c_str());
  readentity (entities[PLANET],(datapath+planetlist).c_str());
  planet_entities=entities[PLANET];
  string desolate=(string(datapath)+"planets.desolate.txt");
  readentity (planet_entities,desolate.c_str());
  readentity (entities[1],(datapath+gasgiantlist).c_str());
  readentity (entities[3],(datapath+moonlist).c_str());
  readentity (units[1],(datapath+smallunitlist).c_str());
  readentity (background,(datapath+backgroundlist).c_str());

  if (background.empty()) {
    background.push_back (backgroundlist);
  }
  if (nebulae) {
    readentity (units[0],(datapath+nebulaelist).c_str());
  }
  if (asteroids) {
    readentity (units[0],(datapath+asteroidlist).c_str());
  }
  readentity(rings,(datapath+ringlist).c_str());
  readnames (names,(datapath+namelist).c_str());

  fp = fopen (outputfile.c_str(),"w");
  if (fp) {
    for (unsigned int i=0;i<jumplocations.size();i++) {
      entities[JUMP].push_back (jumplocations[i]);
    }
    nument[JUMP]=entities[JUMP].size();
    StarSystemGent::faction = factions;
    CreateStarSystem();
    fclose (fp);
  }
}
#ifdef CONSOLE_APP
int main (int argc, char ** argv) {

  if (argc<9) {
    fprintf (stderr,"Usage: starsysgen <seed> <sector>/<system> <sunradius>/<compactness> <numstars> <numgasgiants> <numrockyplanets> <nummoons> [N][A]<numnaturalphenomena> <numstarbases> <faction> <namelist> [OtherSystemJumpNodes]...\n");
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
