#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <math.h>
using namespace std;




int rnd(int lower, int upper) {
  return (int)( lower+ (float(upper-lower)*rand())/(RAND_MAX+1));
}
const char nada[1]="";

string  getRandName(vector<string> &s) {
  if (s.empty())
    return string(nada);
  unsigned int i;
  string k=s[i=rnd(0,s.size())];
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
  return float (rand())/RAND_MAX;
}
vector <Color> lights;
FILE * fp =NULL;
void WriteLight (unsigned int i) {
  fprintf (fp,"<Lights>\n<ambient red=\"%f\" green=\"%f\" blue=\"f\"/>\n<diffuse red=\"%f\" green=\"%f\" blue=\"f\"/>\n<specular red=\"%f\" green=\"%f\" blue=\"f\"/>\n</Light>\n",
	   lights[i].r/4,lights[i].g/4,lights[i].b/4,
	   lights[i].r,lights[i].g,lights[i].b,
	   lights[i].nr,lights[i].ng,lights[i].nb);   

}
const int PLANET = 2;
const int GAS = 1;
const int STAR = 0;
const int MOON=3;
const int JUMP=4;
int nument[5];
vector <string> entities[5];
int numun[2];
vector <string> units [2];
vector <string> background;
vector <string> names;
string systemname;
vector <float> radii;

void CreateLight(unsigned int i) {
  lights.push_back (Color (grand(),grand(),grand()));
  float h = lights.back().r;
  if (h>lights.back().g) h=lights.back().g;
  if (h>lights.back().b) h=lights.back().b;
  float norm = .5+.5/nument[0];
  lights.back().nr=lights.back().r/h;
  lights.back().ng=lights.back().g/h;
  lights.back().nb=lights.back().b/h;

  lights.back().r*=norm/h;
  lights.back().g*=norm/h;
  lights.back().b*=norm/h;
  WriteLight (i);
}

float mmax (float a, float b) {
  return (a>b)?a:b;
}
float makeRS (Vector &r, Vector &s,float minradii) {
  r=Vector (grand(),grand(),grand());
  Vector k(grand(),grand(),grand());
  s = r.Cross (k);
  float sm = s.Mag();
  if (sm<.01) {
    return makeRS(r,s,minradii);
  }
  s.i/=sm;  s.j/=sm;  s.k/=sm;
  sm = r.Mag();
  r.i/=sm;  r.j/=sm;  r.k/=sm;
  bool tmp=false;
  float rm;
  rm= (2*grand()); if (rm<1) {rm=(1+grand()*.5);tmp=true;}
  r.i*=rm;r.j*=rm;r.k*=rm;
  sm= (2*grand()); if (tmp) sm=(1+grand()*.5);
  s.i*=sm;s.j*=sm;s.k*=sm;
  return mmax (rm,sm);
}

void generateAndUpdateRS (Vector &r, Vector & s, float thisplanetradius) {
  if (radii.empty()) {
    r=Vector (0,0,0);
    s=Vector (0,0,0);
    return;
  }
  float orbitsize=makeRS (r,s,radii.back());
  orbitsize+=thisplanetradius;
  radii.back()=orbitsize;
}


void MakePlanet(float radius, int entitytype) {
  ///writes out some pretty planet tags
}

void MakeMoons (float radius, int entitytype, int callingentitytype) {

}
void MakeBigUnit () {

}
void beginStar (float radius, unsigned int which) {
  Vector r,s;
  generateAndUpdateRS (r,s,radius);
  radii.push_back (1.5*radius);
  char b[2]="A";
  b[0]+=which;
  fprintf (fp,"<Planet name=\"%s\" file=\"%s\" ",(systemname+b).c_str(),getRandName(entities[0]).c_str());
  fprintf (fp,"ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ",r.i,r.j,r.k,s.i,s.j,s.k);
  fprintf (fp,"radius=\"%f\" x=\"0\" y=\"0\" z=\"0\" ",radius);

  fprintf (fp," Red=\"%f\" Green=\"%f\" Blue=\"%f\" light=\"%d\">\n",lights[which].r,lights[which].g,lights[which].b,which);
  MakeMoons (.2*radius,PLANET,STAR);
  MakeMoons (.5*radius,GAS,STAR);
  MakeMoons (100,JUMP,STAR);
  int numu = numun[0]/nument[0]+(grand()<float(numun[0]%nument[0])/nument[0]);
  if (which==nument[0]-1) {
    numu=numun[0];
  }
  numun[0]-=numu;
  for (unsigned int i=0;i<numu;i++) {
    MakeBigUnit();
  }
  radii.pop_back();
}

void endStar () {
  fprintf (fp,"</Planet>\n");
}
void CreateStar (float radius, unsigned int which) {
  beginStar (radius,which);
  endStar();
}
void CreateFirstStar(float radius, unsigned int which){
  beginStar(radius,which);
  for (unsigned int i=which+1;i<nument[0];i++) {
    if (rand()>RAND_MAX/2) {
      CreateFirstStar(.8*radius*grand(),i);
      break;
    } else {
      CreateStar (.8*radius*grand(),i);
    }
  }
  endStar();
}

void CreatePrimaries (float starradius) {
  unsigned int numprimaryunits=rnd(0,1+numun[0]);
  numun[0]-=numprimaryunits;
  for (unsigned int i=0;i<nument[0];i++) {
    CreateLight(i);
  }
  CreateFirstStar(starradius,0);

  for (unsigned int i=0;i<numprimaryunits;i++) {
    MakeBigUnit();
  }
}

void CreateStarSystem () {
  float starradius =0;
  assert (starradius);
  fprintf (fp,"<system name=\"%s\" background=\"%s\" nearstars=\"%d\" stars=\"%d\" starspread=\"150\">\n",systemname.c_str(),getRandName(background).c_str(),500,1000,150);
  CreatePrimaries (starradius);
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

int main (int argc, char ** argv) {
  readentity (entities[0],"stars.txt");
  readentity (entities[1],"planets.txt");
  readentity (entities[2],"gas_giants.txt");
  readentity (entities[3],"moons.txt");
  readentity (units[0],"bigunits.txt");
  readentity (units[1],"smallunits.txt");
  readentity (background,"background.txt");
  readentity (names,"names.txt");
  char filename[1000];
  strcpy (filename,argv[1]);
  strcat (filename,".system");
  fp = fopen (filename,"w");
  if (1!=sscanf (argv[1],"/%s",filename)) {
    strcpy (filename,argv[1]);
  }
  filename[0]= toupper (filename[0]);
  systemname=string(filename);
  
  fclose (fp);
  return 0;
}
