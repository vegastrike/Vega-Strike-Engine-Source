#include "gfx_sphere.h"
#include "vegastrike.h"

#include "xml_support.h"
#ifndef M_PI
#define M_PI 3.1415926536F
#endif

using XMLSupport::tostring;

SphereMesh::SphereMesh(float radius, int stacks, int slices, char *texture, bool Insideout,bool centeredOnShip) : Mesh() {
  //strcpy(name, "Sphere");

  SphereMesh *oldmesh;
  string hash_key = string("@@Sphere") + "#" + tostring(radius) + "#" + texture + "#" + tostring(stacks) + "#" + tostring(slices) + "#" + (Insideout?"yes":"no");
  if(0 != (oldmesh = (SphereMesh*)meshHashTable.Get(hash_key)))
    {
      *this = *oldmesh;
      oldmesh->refcount++;
      orig = oldmesh;
      return;
    }

  insideout= Insideout;
  radialSize = radius;//MAKE SURE FRUSTUM CLIPPING IS DONE CORRECTLY!!!!!
  minSizeX = minSizeY = minSizeZ = -radialSize;
  maxSizeX = maxSizeY = maxSizeZ = radialSize;
  //  minSizeX = minSizeY = minSizeZ = -radialSize*10;
  //  maxSizeX = maxSizeY = maxSizeZ = radialSize*10;

   float rho, drho, theta, dtheta;
   float x, y, z;
   float s, t, ds, dt;
   int i, j, imin, imax;
   centered = centeredOnShip; 
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
   numvertex=stacks*(slices+1)*2;
   vertexlist = new GFXVertex[numvertex];
   
   GFXVertex *vl = vertexlist;
   enum POLYTYPE *modes= new enum POLYTYPE [numQuadstrips];   
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
       vertexlist[j*2+fir].j = y * nsign;
       vertexlist[j*2+fir].k = z * nsign;
       vertexlist[j*2+fir].s = insideout?1-s:s;
       vertexlist[j*2+fir].t = t;
       vertexlist[j*2+fir].x = x * radius;
       vertexlist[j*2+fir].y = y * radius;
       vertexlist[j*2+fir].z = z * radius;


       x = -sin(theta) * sin(rho + drho);
       y = cos(theta) * sin(rho + drho);
       z = nsign * cos(rho + drho);

       vertexlist[j*2+sec].i = x * nsign;
       vertexlist[j*2+sec].j = y * nsign;
       vertexlist[j*2+sec].k = z * nsign;
       vertexlist[j*2+sec].s = insideout?1-s:s;
       vertexlist[j*2+sec].t = t - dt;
       vertexlist[j*2+sec].x = x * radius;
       vertexlist[j*2+sec].y = y * radius;
       vertexlist[j*2+sec].z = z * radius;

       s += ds;
     }

     t -= dt;
     QSOffsets[i]= (slices+1)*2;
     modes[i]=GFXQUADSTRIP;
   }

   vlist = new GFXVertexList(modes,numvertex, vertexlist, numQuadstrips ,QSOffsets);
   delete [] modes;
   delete [] QSOffsets;
   Decal = new Texture(texture, 0);
   centered?envMap = FALSE:envMap=TRUE;
   
   if(centered) {
     draw_sequence=0;
   }
   meshHashTable.Put(hash_key, this);
   orig = this;
   refcount++;
   draw_queue = new vector<MeshDrawContext>;
}
void SphereMesh::Draw(const Transformation &transform /*= identity_transformation*/) {
  if (centered) {
    SetPosition(_GFX->AccessCamera()->GetPosition());
  }		

  Mesh::Draw();
}

void SphereMesh::ProcessDrawQueue() {
  GFXSelectMaterial(myMatNum);
  //static float rot = 0;
  GFXColor(1.0, 1.0, 1.0, 1.0);
  GFXDisable (LIGHTING);
  GFXEnable(TEXTURE0);
  if(envMap) {
    GFXEnable(TEXTURE1);
  } else {
    GFXDisable(TEXTURE1);
  }
  Decal->MakeActive();
  GFXBlendMode(ONE, ZERO);
  
  GFXSelectTexcoordSet(0, 0);
  if(envMap) {
    //_GFX->getLightMap()->MakeActive();
    _GFX->activateLightMap();
    GFXSelectTexcoordSet(1, 1);
  }
  if (insideout) 
    GFXDisable (CULLFACE);
  if (centered) {
    GFXDisable(LIGHTING);
    GFXDisable(DEPTHWRITE);
    GFXDisable(DEPTHTEST);
    GFXDisable(TEXTURE1);
  }	
  vlist->LoadDrawState();
  static float theta=0;
  while(draw_queue->size()) {
    MeshDrawContext c = draw_queue->back();
    draw_queue->pop_back();
    Matrix tmp;	
    Matrix tmp2;
    Identity (tmp);
    if (!centered){
      VectorToMatrix (tmp,Vector (cos (theta),0,sin(theta)), Vector (0,1,0), Vector (-sin (theta),0,cos(theta)));
    }
    MultMatrix (tmp2, c.mat, tmp);
    GFXLoadMatrix(MODEL, tmp2);
    GFXPickLights (tmp2);
    theta+=.01;
    vlist->Draw();
    if(0!=forcelogos) {
      forcelogos->Draw(tmp2);
      squadlogos->Draw(tmp2);
    }
  }

  if (insideout)
    GFXEnable(CULLFACE);
  if (centered) {
    GFXEnable(LIGHTING);
    GFXEnable(DEPTHWRITE);
    GFXEnable(DEPTHTEST);
  }
}

