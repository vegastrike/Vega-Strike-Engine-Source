#include "gfx_mesh.h"

#define PBEHIND -1
#define PFRONT 1
#define PUNK 0

#define WHICHSID(v) (((v.x*a+v.y*b+v.z*c+d)>0)*2-1)


static int whichside (GFXVertex * t, int numvertex, float a, float b, float c, float d) {
  int count = PUNK;
  for (int i=0;i<numvertex;i++) {
    count += WHICHSID (t[i]);
  }
  if (count==PUNK) {
    count = (rand()%2)*2-1;
  }
  return ((count>0)*2)-1;
}

void Mesh::Fork (Mesh *&x, Mesh *&y, float a, float b, float c, float d) {
  if (orig) {
    orig->Fork (x,y,a,b,c,d);
    return;
  }
  int numtris, numquads;
  GFXVertex * Orig;
  vlist->GetPolys (&Orig, &numquads, &numtris);
  int numtqx[2]= {0,0};
  int numtqy[2]= {0,0};

  GFXVertex * X = new GFXVertex [numquads*4+numtris*3];
  GFXVertex * xnow = X;
  GFXVertex * Y = new GFXVertex [numquads*4+numtris*3];
  GFXVertex *ynow = Y;
  int i,j;
  int inc =3;
  int offset=0;
  int last = numtris;
  bsp_polygon tmppolygon;
  for (int l=0;l<2;l++) {
    for (i=0;i<last;i++) {
      if (whichside (&Orig[offset+i*inc],inc,a,b,c,d)==PFRONT) {
	numtqx[l]++;
	for (j=0;j<inc;j++) {
	  memcpy (xnow, &Orig[offset+i*inc+j], sizeof (GFXVertex));
	  xnow++;
	}
      }else {
	numtqy[l]++;
	for (j=0;j<inc;j++) {
	  memcpy (ynow, &Orig[offset+i*inc+j], sizeof (GFXVertex));	  
	  ynow++;
	}
      }

    }
    inc=4;
    offset = numtris*3;
    last = numquads;
  } 
  free (Orig);
  enum POLYTYPE polytypes [2]={GFXTRI,GFXQUAD};
  x = (Mesh *)malloc (sizeof (Mesh));
  y = (Mesh *)malloc (sizeof (Mesh));
  x->forcelogos = x->squadlogos = y->squadlogos=y->forcelogos = NULL;
  x->numforcelogo = x->numsquadlogo = y->numforcelogo = y->numsquadlogo = 0;
  x->Decal = new Texture (Decal);
  y->Decal = new Texture (Decal);//use copy constructor;
  x->vlist = new GFXVertexList (polytypes, numtqx[0]+numtqx[1], X, NULL, 2, numtqx, true); 
  y->vlist = new GFXVertexList (polytypes, numtqy[0]+numtqy[1], Y, NULL, 2, numtqy, true); 

  
  delete [] X;
  delete [] Y;
}

void Mesh::GetPolys (vector <bsp_polygon> & polys) {
    int numtris;
    int numquads;
    if (orig) {
      orig->GetPolys (polys);
      return;
    }
    GFXVertex * tmpres;
    bsp_vector vv;
    vlist->GetPolys (&tmpres,&numquads,&numtris);
    numquads-=numtris;
    int i;
    int inc =3;
    int offset=0;
    int last = numtris;
    bsp_polygon tmppolygon;
    for (int l=0;l<2;l++) {
	for (i=0;i<last;i++) {
	    polys.push_back (tmppolygon);
	    for (int j=0;j<inc;j++) {
	      vv.x=tmpres[offset+i*inc+j].x;//+local_pos.i;
	      vv.y=tmpres[offset+i*inc+j].y;//+local_pos.j;
	      vv.z=tmpres[offset+i*inc+j].z;//+local_pos.k;
	      polys[polys.size()-1].v.push_back (vv);
	    }
	}
	inc=4;
	offset = numtris*3;
	last = numquads;
    }
    free (tmpres);
}
