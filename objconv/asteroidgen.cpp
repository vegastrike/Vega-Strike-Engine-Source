#include <vector>
#include <stdio.h>
#include <stdlib.h>


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
};
class Tri {
public:
  int a;
  float sa,ta;
  int b;
  float sb,tb;
  int c;
  float sc,tc;
  Tri (int x, int y, int z) {a=x;b=y;c=z;}
};
struct asteroid {
  Vector center;
  float radius;
  vector <Vector> points;
  vector <Tri> polygon;
};


char texture [100] ="asteroid";
float scale=1;

void write_mesh (FILE * fp, vector <asteroid> &field) {
  fprintf (fp,"<MESH texture=\"%s\" scale=\"%d\">\n<Points>\n",texture, scale);
  unsigned int i;
  unsigned int counter=0;
  for (i=0;i<field.size();i++) {
    unsigned int j;
    for (j=0;j<field[i].points.size();j++) {
      fprintf (fp,"<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\"/>\n</Point>\n",field[i].points[i].i,field[i].points[i].j,field[i].points[i].k);
    }
    for (j=0;j<field[i].polygon.size();j++) {
      field[i].polygon[j].sa=field[i].points[field[i].polygon[j].a].s;
      field[i].polygon[j].ta=field[i].points[field[i].polygon[j].a].t;
      field[i].polygon[j].a+=counter;
      field[i].polygon[j].sb=field[i].points[field[i].polygon[j].b].s;
      field[i].polygon[j].tb=field[i].points[field[i].polygon[j].b].t;
      field[i].polygon[j].b+=counter;
      field[i].polygon[j].c+=counter;
      field[i].polygon[j].sc=field[i].points[field[i].polygon[j].c].s;
      field[i].polygon[j].tc=field[i].points[field[i].polygon[j].c].t;
    }
    counter+=field[i].points.size();
  }
  fprintf (fp,"</Points>\n<Polygons>\n");
  for (i=0;i<field.size();i++) {
    unsigned int j;
    for (j=0;j<field[i].polygon.size();j++) {
      Tri t = field[i].polygon[j];
      fprintf (fp,"<Tri>\n");
      fprintf (fp,"<Vertex point=\"%d\" s=\"%f\" s=\"%f\"/>\n",t.a,t.sa,t.ta);
      fprintf (fp,"<Vertex point=\"%d\" s=\"%f\" s=\"%f\"/>\n",t.b,t.sb,t.tb);
      fprintf (fp,"<Vertex point=\"%d\" s=\"%f\" s=\"%f\"/>\n",t.c,t.sc,t.tc);
      fprintf (fp,"</Tri>\n");
    }
  }
  fprintf (fp,"</Polygons>\n<Material reflect=\"%d\">\n<Specular red=\"%f\" green=\"%f\" blue=\"%f\" alpha=\"%f\"/>\n</Material>\n</Mesh>",0,0,0,0,1);
}




int main () {
  
  return 0;

}
