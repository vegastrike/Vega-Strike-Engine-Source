#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using std::vector;
class Vector {
public:
  float i;
  float j;
  float k;
  float s;
  float t;
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
class Tri {
public:
  bool quad;
  int a;
  float sa,ta;
  int b;
  float sb,tb;
  int c;
  float sc,tc;
  int d;
  float sd, td;
  Tri (int x, int y, int z) {c=x;b=y;a=z; quad=false;}
  Tri (int x, int y, int z, int w) {d=x;c=y;b=z;a=w;quad=true;}
  void Write (FILE * fp) {
    if (!quad)
      fprintf (fp,"<Tri>\n");
    else
      fprintf (fp,"<Quad>\n");
    fprintf (fp,"<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",a,sa,ta);
    fprintf (fp,"<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",b,sb,tb);
    fprintf (fp,"<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",c,sc,tc);
    if (!quad) {
      fprintf (fp,"</Tri>\n");
    }else {
      fprintf (fp,"<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",d,sd,td);
      fprintf (fp,"</Quad>\n");
    }
  }
};
struct asteroid {
  Vector center;
  float radius;
  Vector YawPitchRoll;
  int num_polys;
  vector <Vector> points;
  vector <Tri> polygon;
  asteroid (): center (0,0,0),YawPitchRoll(0,0,0){

  }
  void CenterAndRotate () {
    for (unsigned int i=0;i<points.size();i++) {
      //      points[i].Pitch (YawPitchRoll.i);
      //      points[i].Yaw (YawPitchRoll.j);
      //      points[i].Roll (YawPitchRoll.k);
      points[i].i+=center.i;
      points[i].j+=center.j;
      points[i].k+=center.k;

    }
  }
};

char texture [100] ="Asteroid.bmp";
float scale=1;
void determine_centers_and_radii (vector <asteroid> & field, const Vector &cube_sides, const float radiusmin, const float radiusmax, const int poly_min, const int poly_max) {
  for (unsigned int i=0;i<field.size();i++) {
    field[i].center.i = cube_sides.i * ((float)rand())/RAND_MAX-cube_sides.i/2;
    field[i].center.j = cube_sides.j * ((float)rand())/RAND_MAX-cube_sides.j/2;
    field[i].center.k = cube_sides.k * ((float)rand())/RAND_MAX-cube_sides.k/2;
    float radiusratio = ((float)rand())/RAND_MAX;
    field[i].radius = radiusmin+(radiusmax-radiusmin)*radiusratio;    
    radiusratio*=(poly_max+1-poly_min);
    field[i].num_polys = (int)radiusratio+poly_min;
    if (field[i].num_polys<4)
      field[i].num_polys=4;
    field[i].YawPitchRoll.i = 2*M_PI*((float)rand())/RAND_MAX;
    field[i].YawPitchRoll.j = 2*M_PI*((float)rand())/RAND_MAX;
    field[i].YawPitchRoll.k = 2*M_PI*((float)rand())/RAND_MAX;
    bool insideanother=false;
    for (unsigned int j=0;j<i;j++) {
      if (fabs (field[j].center.i-field[i].center.i)<1.2
*(field[j].radius+field[i].radius)&&
	  fabs (field[j].center.j-field[i].center.j)<1.2*(field[j].radius+field[i].radius)&&
	  fabs (field[j].center.j-field[i].center.j)<1.2*(field[j].radius+field[i].radius)) {
	insideanother =true;
	break;
      }
    }
    if (insideanother)
      i--;
  }
}

float getR(float minr,float maxr) {
  return ((maxr-minr)*((float)rand())/RAND_MAX)+minr;
}
void generateTet (vector <Vector> &v, vector <Tri> & p, const float minr, const float maxr) {

  double h = 1/sqrt (2);
  double r = getR (minr,maxr);  
  double rA =r;
  v.push_back (Vector (0 ,-r,h*r,0,0));
  r = getR (minr,maxr);  
  v.push_back (Vector (r,0,-h*r,.5,0));
  r = rA;
  v.push_back (Vector (0,-r,h*r,1,0));
  r = getR (minr,maxr);  
  double rD = r;
  v.push_back (Vector (0,r,h*r,1,1));
  r = getR (minr,maxr);
  v.push_back (Vector (-r,0,-h*r,.5,1));
  r = rD;
  v.push_back (Vector (0,r,h*r,0,1));
  p.push_back (Tri(0,5,1));//AFB
  p.push_back (Tri(3,2,4));//DCE
  p.push_back (Tri(4,2,1));//ECB
  p.push_back (Tri(1,5,4));//BFE
}
void generateNTet (vector <Vector> &v, vector <Tri> & p, const float minr, const float maxr,int stacks, int slices) {
  for (unsigned int i=0;i<stacks+2;i++) {
    float tempR = getR (minr,maxr);
    for (unsigned int j=0;j<slices;j++) {
      if (i!=0&&i!=stacks+1)
	tempR = getR (minr,maxr);///don't want the tip ot have different points
      float projR = tempR*sin (M_PI*i/(stacks+1));
      if ((i!=0&&i!=stacks+1)||j==0) {
	v.push_back (Vector (projR*cos (2*M_PI*j/(slices)),//i
			     tempR*cos (M_PI*i/(stacks+1)),//j
			     projR*sin (2*M_PI*j/(slices)),//k
			     ((float)j)/(slices-1)+((i==0||i==stacks+1)?.5:0),//s
			     ((float)i)/(stacks+1)));//t
      }
      if (i!=0&&i!=1&&i!=stacks+1) {
	p.push_back (Tri (1+(i-2)*slices+j,
			  1+(i-1)*slices+j,
			  1+(i-1)*slices+ ((j+1)%slices),
			  1+(i-2)*slices+ ((j+1)%slices)));
      }else if (i==1) {
	//do top pyr

	p.push_back (Tri (0,
			  1+j,
			  1+((j+1)%slices)));
      } else if (i==stacks+1) {
	p.push_back (Tri (1+(i-2)*slices+j,
			  1+(i-1)*slices,
			  1+(i-2)*slices+ ((j+1)%slices)));
	//do bottom pyr
      }
    }
  }
}
void generateDoubleTet (vector <Vector> &v, vector <Tri> & p, const float minr, const float maxr, int num) {
  generateNTet (v,p,minr,maxr,1,num);
}

void createShapes (asteroid & a, float dev) {
  if (a.num_polys<6) {
    generateTet (a.points,a.polygon,a.radius*(1-dev),a.radius*(1+dev));
  } else if (a.num_polys <8) {
    generateDoubleTet (a.points,a.polygon,a.radius*(1-dev),a.radius*(1+dev),3);
  } else if (a.num_polys<10) {
    generateDoubleTet (a.points,a.polygon, a.radius*(1-dev), a.radius*(1+dev),4);
  } else if (a.num_polys<12) {
    generateDoubleTet (a.points,a.polygon, a.radius*(1-dev),a.radius*(1+dev),6);
  }else if (a.num_polys<36) {
    generateNTet (a.points,a.polygon, a.radius*(1-dev),a.radius*(1+dev),a.num_polys/6,a.num_polys%6+3);
  } else {
    generateNTet (a.points,a.polygon, a.radius*(1-dev),a.radius*(1+dev),a.num_polys/12,a.num_polys%12+6);
  }
}

void write_mesh (FILE * fp, vector <asteroid> &field) {
  fprintf (fp,"<Mesh texture=\"%s\" sharevertex=\"1\" scale=\"%f\">\n<Points>\n",texture, scale);
  unsigned int i;
  unsigned int counter=0;
  for (i=0;i<field.size();i++) {
    unsigned int j;
    for (j=0;j<field[i].points.size();j++) {
      fprintf (fp,"<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\" t=\"%f\"/>\n</Point>\n",field[i].points[j].i,field[i].points[j].j,field[i].points[j].k,field[i].points[j].s,field[i].points[j].t);
    }
    for (j=0;j<field[i].polygon.size();j++) {
      field[i].polygon[j].sa=field[i].points[field[i].polygon[j].a].s;
      field[i].polygon[j].ta=field[i].points[field[i].polygon[j].a].t;
      field[i].polygon[j].a+=counter;
      field[i].polygon[j].sb=field[i].points[field[i].polygon[j].b].s;
      field[i].polygon[j].tb=field[i].points[field[i].polygon[j].b].t;
      field[i].polygon[j].b+=counter;
      field[i].polygon[j].sc=field[i].points[field[i].polygon[j].c].s;
      field[i].polygon[j].tc=field[i].points[field[i].polygon[j].c].t;
      field[i].polygon[j].c+=counter;
      if (field[i].polygon[j].quad) {
	      field[i].polygon[j].sd=field[i].points[field[i].polygon[j].d].s;
	      field[i].polygon[j].td=field[i].points[field[i].polygon[j].d].t;
	      field[i].polygon[j].d+=counter;
      }
    }
    counter+=field[i].points.size();
  }
  fprintf (fp,"</Points>\n<Polygons>\n");
  for (i=0;i<field.size();i++) {
    unsigned int j;
    for (j=0;j<field[i].polygon.size();j++) {
      field[i].polygon[j].Write (fp);
    }
  }
  fprintf (fp,"</Polygons>\n<Material reflect=\"%d\">\n<Specular red=\"%f\" green=\"%f\" blue=\"%f\" alpha=\"%f\"/>\n</Material>\n</Mesh>\n",0,0,0,0,1);
}


void createShapes (vector <asteroid> &field, float deviation) {
  for (unsigned int i=0;i<field.size();i++) {
    createShapes (field[i],deviation);
    field[i].CenterAndRotate();
  }
}


int main (int argc, char ** argv) {
  vector <asteroid> field;
  Vector cube_sides (1024,1024,1024);
  float radiusmin,radiusmax;
  int poly_min,poly_max;
  float deviation;

  char filename[16384];
  int numroids;
  if (argc>=9) {
    sscanf (argv[1],"%s",filename);
    sscanf (argv[2],"%f",&cube_sides.i);
    cube_sides.j= cube_sides.k = cube_sides.i;
    sscanf (argv[3],"%d", &numroids);
    sscanf (argv[4],"%f",&radiusmin);
    sscanf (argv[5],"%f",&radiusmax);
    sscanf (argv[6],"%f",&deviation);
    sscanf (argv[7],"%d",&poly_min);
    sscanf (argv[8],"%d",&poly_max);

  }else {
    printf ("Enter Ouput File: ");

    scanf ("%s",filename);
    printf ("Enter Size of field?\n");
    scanf ("%f",&cube_sides.i);
    cube_sides.j= cube_sides.k = cube_sides.i;
    printf ("Enter number of asteroids\n");
    scanf ("%d", &numroids);
    printf ("Enter asteroid minimum and max radius\n");
    scanf ("%f %f",&radiusmin,&radiusmax);
    printf ("Enter cragginess\n");  
    scanf ("%f",&deviation);
    printf ("Enter minimum number of polys and max num polys\n");
    scanf ("%d %d",&poly_min,&poly_max);
  }
  for (int i=0;i<numroids;i++) {
    field.push_back (asteroid());
  }
  determine_centers_and_radii (field, cube_sides,radiusmin, radiusmax, poly_min, poly_max);
  createShapes(field,deviation);
  FILE * fp= fopen (filename,"w");
  write_mesh (fp,field);
  fclose (fp);
  return 0;
}
