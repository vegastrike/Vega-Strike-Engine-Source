#include "sphere.h"
#include "aux_texture.h"
#include "vegastrike.h"

#include "xml_support.h"
#ifndef M_PI
#define M_PI 3.1415926536F
#endif

using XMLSupport::tostring;
const int pixelscalesize=30;
SphereMesh::SphereMesh(float radius, int stacks, int slices, char *texture, char *alpha,bool Insideout,  const BLENDFUNC a, const BLENDFUNC b, bool envMapping) : Mesh() {
  int numspheres = (stacks+slices)/8;
  if (numspheres<1)
    numspheres =1;
  Mesh *oldmesh;
  char ab[3];
  ab[2]='\0';
  ab[1]=a+'0';
  ab[2]=a+'0';
  hash_name = string("@@Sphere") + "#" + tostring(radius) + "#" + texture + "#" + tostring(stacks) + "#" + tostring(slices) +  ab;
  if (LoadExistant (hash_name.c_str())) {
    return;
  }
  oldmesh = new Mesh[numspheres];//have to!
  numlods=numspheres;
  
  meshHashTable.Put (hash_name, oldmesh);
  this->orig = oldmesh;
  radialSize = radius;//MAKE SURE FRUSTUM CLIPPING IS DONE CORRECTLY!!!!!
  mn = Vector (-radialSize,-radialSize,-radialSize);
  mx = Vector (radialSize,radialSize,radialSize);
  vector <MeshDrawContext> *odq=NULL;
  int origst = stacks;
  int origsl = slices;
  for (int l=0;l<numspheres;l++) {
    
    draw_queue = new vector<MeshDrawContext>;
    if (!odq)
      odq = draw_queue;
    //    stacks = origst/(l+1);
    //slices = origsl/(l+1);
    if (stacks>12) {
      stacks -=4;
      slices-=4;
    } else {
      stacks-=2;
      slices-=2;
    }
    float rho, drho, theta, dtheta;
    float x, y, z;
    float s, t, ds, dt;
    int i, j, imin, imax;
    float nsign = Insideout?-1.0:1.0;
    int fir=0;//Insideout?1:0;
    int sec=1;//Insideout?0:1;
    vlist = NULL;
    /* Code below adapted from gluSphere */
    drho = M_PI / (GLfloat) stacks;
    dtheta = 2.0 * M_PI / (GLfloat) slices;
    
    ds = 1.0 / slices;
    dt = 1.0 / stacks;
      t = 1.0;			/* because loop now runs from 0 */
      
      imin = 0;
      imax = stacks;
      
      int numQuadstrips = stacks;
      //      numQuadstrips = 0;
      int *QSOffsets = new int [numQuadstrips];
      
      // draw intermediate stacks as quad strips 
      int numvertex=stacks*(slices+1)*2;
      GFXVertex *vertexlist = new GFXVertex[numvertex];
    
      GFXVertex *vl = vertexlist;
      enum POLYTYPE *modes= new enum POLYTYPE [numQuadstrips];   
      /*   SetOrientation(Vector(1,0,0),
	   Vector(0,0,-1),
	   Vector(0,1,0));//that's the way prop*///taken care of in loading
      
      
      for (i = imin; i < imax; i++) {
	GFXVertex *vertexlist = vl + (i * (slices+1)*2);
	rho = i * drho;
	
	s = 0.0;
	for (j = 0; j <= slices; j++) {
	  theta = (j == slices) ? 0.0 : j * dtheta;
	  x = -sin(theta) * sin(rho);
	  y = cos(theta) * sin(rho);
	  z = nsign * cos(rho);
	
	  vertexlist[j*2+fir].i = x * nsign;
	  vertexlist[j*2+fir].k = -y * nsign;
	  vertexlist[j*2+fir].j = z * nsign;
	  vertexlist[j*2+fir].s = 1-s;//insideout?1-s:s;
	  vertexlist[j*2+fir].t = t;
	  vertexlist[j*2+fir].x = x * radius;
	  vertexlist[j*2+fir].z = -y * radius;
	  vertexlist[j*2+fir].y = z * radius;

	  
	  x = -sin(theta) * sin(rho + drho);
	  y = cos(theta) * sin(rho + drho);
	  z = nsign * cos(rho + drho);

	  vertexlist[j*2+sec].i = x * nsign;
	  vertexlist[j*2+sec].k = -y * nsign;
	  vertexlist[j*2+sec].j = z * nsign;
	  vertexlist[j*2+sec].s = 1-s;//insideout?1-s:s;
	  vertexlist[j*2+sec].t = t - dt;
	  vertexlist[j*2+sec].x = x * radius;
	  vertexlist[j*2+sec].z = -y * radius;
	  vertexlist[j*2+sec].y = z * radius;
	
	  s += ds;
	}
	
	t -= dt;
	QSOffsets[i]= (slices+1)*2;
	modes[i]=GFXQUADSTRIP;
      }
      
      vlist = new GFXVertexList(modes,numvertex, vertexlist, numQuadstrips ,QSOffsets);
      delete [] vertexlist;
      delete [] modes;
      delete [] QSOffsets;
      if (alpha) {
	blendSrc = SRCALPHA;
	blendDst = INVSRCALPHA;
	Decal = new Texture(texture, alpha);
      }else {
	blendSrc = a;
	blendDst = b;
	Decal = new Texture (texture);
      }
      
      Insideout?envMap = GFXFALSE:envMap=envMapping;
      
      if(Insideout) {
      draw_sequence=0;
      }
      
      Mesh * oldorig = orig;
      refcount=1;
      orig=NULL;
      if (l>=1) {
	lodsize=(numspheres+1-l)*pixelscalesize;
	if (l==1) {
	  lodsize*=2;
	}else if (l==2) {
	  lodsize*=1.75;
	} else if (l==3) {
	  lodsize*=1.5;
	}
      }
      oldmesh[l]=*this;
      refcount =0;
      orig = oldorig;
      lodsize = FLT_MAX;
  }
  draw_queue = odq;
}
void SphereMesh::Draw(float lod,  bool centered, const Transformation &transform /*= identity_transformation*/, const Matrix m) {
  if (centered) {
    float m1[16];
    memcpy (m1,m,sizeof (float)*16);
    Vector pos(_Universe->AccessCamera()->GetPosition().Transform(m1));
    m1[12]=pos.i;
    m1[13]=pos.j;
    m1[14]=pos.k;
    Transformation tmp = transform;
    tmp.position = pos;
    Mesh::Draw (lod,tmp,m1);
  } else {	
    Mesh::Draw(lod,transform,m);
  } 
}



