#include "gfx_sphere.h"
#include "vegastrike.h"
#ifndef M_PI
#define M_PI 3.1415926536F
#endif
SphereMesh::SphereMesh(float radius, int stacks, int slices, char *texture, bool Insideout,bool centeredOnShip) : Mesh() {
  debugName = "Sphere";

  insideout= Insideout;
  radialSize = radius;//MAKE SURE FRUSTUM CLIPPING IS DONE CORRECTLY!!!!!
  minSizeX = minSizeY = minSizeZ = -radialSize*1.414;
  maxSizeX = maxSizeY = maxSizeZ = radialSize*1.414;
  numlines = 0;
  numquads = 0;
  numtris =0;
  numvertex=0;
   float rho, drho, theta, dtheta;
   float x, y, z;
   float s, t, ds, dt;
   int i, j, imin, imax;
   centered = centeredOnShip; 
  float nsign = Insideout?-1.0:1.0;
  int fir=0;//Insideout?1:0;
    int sec=1;//Insideout?0:1;
  vlist = new GFXVertexList();
  /* Code below adapted from gluSphere */

   drho = M_PI / (GLfloat) stacks;
   dtheta = 2.0 * M_PI / (GLfloat) slices;

      ds = 1.0 / slices;
      dt = 1.0 / stacks;
      t = 1.0;			/* because loop now runs from 0 */

      imin = 0;
      imax = stacks;
      
      numQuadstrips = stacks;
      //      numQuadstrips = 0;

      quadstrips = new GFXQuadstrip*[numQuadstrips];
      // draw intermediate stacks as quad strips 
      vertexlist = new GFXVertex[stacks * (slices+1)*2];
      GFXVertex *vl = vertexlist;

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
	quadstrips[i] = new GFXQuadstrip((slices+1) * 2, vertexlist);
      }
      Decal = new Texture(texture, 0);
      centered?envMap = FALSE:envMap=TRUE;
}
void SphereMesh::Draw() {
  if (insideout) 
    GFXDisable (CULLFACE);
  if (centered) {
    GFXDisable(LIGHTING);
    GFXLoadIdentity(MODEL);
    SetPosition(_GFX->AccessCamera()->GetPosition());
    GFXDisable(DEPTHWRITE);
  }	
  
  Mesh::Draw();
  if (insideout)
    GFXEnable(CULLFACE);
  if (centered) {
    GFXEnable(LIGHTING);
    GFXEnable(DEPTHWRITE);
  }
}
void SphereMesh::Draw (const Vector &x, const Vector &y, const Vector &z, const Vector & pos) {
  if (insideout) {
    GFXDisable (CULLFACE);
  }
  if (centered) {
    GFXLoadIdentity(MODEL);
    SetPosition(_GFX->AccessCamera()->GetPosition());
    GFXDisable(DEPTHWRITE);
    GFXDisable(LIGHTING);
  }	
  Mesh::Draw(x,y,z,pos);
  if (centered) {
    GFXEnable(DEPTHWRITE);
    GFXEnable(LIGHTING);
  }
  if (insideout) {
    GFXEnable(CULLFACE);
  }

}
