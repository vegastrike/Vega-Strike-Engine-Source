#include "gfx_sphere.h"

SphereMesh::SphereMesh(float radius, int stacks, int slices, char *texture) : Mesh() {
  debugName = "Sphere";
  radialSize = radius;//MAKE SURE FRUSTUM CLIPPING IS DONE CORRECTLY!!!!!
   float rho, drho, theta, dtheta;
   float x, y, z;
   float s, t, ds, dt;
   int i, j, imin, imax;
  float nsign = 1.0;

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
      quadstrips = new GFXQuadstrip*[numQuadstrips];
      /* draw intermediate stacks as quad strips */
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

	  vertexlist[j*2].i = x * nsign;
	  vertexlist[j*2].j = y * nsign;
	  vertexlist[j*2].k = z * nsign;
	  vertexlist[j*2].s = s;
	  vertexlist[j*2].t = t;
	  vertexlist[j*2].x = x * radius;
	  vertexlist[j*2].y = y * radius;
	  vertexlist[j*2].z = z * radius;


	  x = -sin(theta) * sin(rho + drho);
	  y = cos(theta) * sin(rho + drho);
	  z = nsign * cos(rho + drho);

	  vertexlist[j*2+1].i = x * nsign;
	  vertexlist[j*2+1].j = y * nsign;
	  vertexlist[j*2+1].k = z * nsign;
	  vertexlist[j*2+1].s = s;
	  vertexlist[j*2+1].t = t - dt;
	  vertexlist[j*2+1].x = x * radius;
	  vertexlist[j*2+1].y = y * radius;
	  vertexlist[j*2+1].z = z * radius;

	  s += ds;
	}

	t -= dt;
	quadstrips[i] = new GFXQuadstrip((slices+1) * 2, vertexlist);
      }

      Decal = new Texture(texture, 0);
}
