#include "gfxlib.h"
#include "gfx/matrix.h"
#include "gl_matrix.h"
using namespace GFXMatrices;  //causes problems with g_game
float BoxFrust [6][4];
float frust [6][4];

float /*GFXDRVAPI*/ GFXSphereInFrustum (const Vector &Cnt, float radius) {
  return GFXSphereInFrustum (frust,Cnt,radius);
}
CLIPSTATE GFXBoxInFrustum (const Vector &min, const Vector &max) {
  return GFXBoxInFrustum (BoxFrust,min,max);
}
CLIPSTATE GFXTransformedBoxInFrustum (const Vector &min, const Vector &max) {
  return GFXBoxInFrustum (frust,min,max);
}

CLIPSTATE GFXBoxInFrustum (float f [6][4], const Vector &min, const Vector &max) {
  	// Doesn't do a perfect test for NOT_VISIBLE.  Just checks to
	// see if all box vertices are outside at least one frustum
	// plane.  Some pathological boxes could return SOME_CLIP even
	// though they're really fully outside the frustum.  But that
	// won't hurt us too much if it isn't a common case; the
	// contents will just be culled/clipped at a later stage in the
	// pipeline.
	
	// Check each vertex of the box against the view frustum, and compute
	// bit codes for whether the point is outside each plane.
	int	OrCodes = 0, AndCodes = ~0;

	for (int i = 0; i < 8; i++) {
		
		Vector	v(min.i, min.j, min.k);
		if (i & 1) v.i=(max.i);
		if (i & 2) v.j=(max.j);
		if (i & 4) v.k=(max.k);
		
		// Now check against the frustum planes.
		int	Code = 0;
		int	Bit = 1;
		for (int j = 0; j < 6; j++, Bit <<= 1) {
			
		  if (v.i * f[j][0]+v.j*f[j][1]+v.k*f[j][2] + f[j][3] < 0) {
		    // The point is outside this plane.
		    Code |= Bit;
		  }
		}
		OrCodes |= Code;
		AndCodes &= Code;
	}

	// Based on bit-codes, return culling results.
	if (OrCodes == 0) {
		// The box is completely within the frustum.
		return GFX_TOTALLY_VISIBLE;
	} else if (AndCodes != 0) {
		// All the points are outside one of the frustum planes.
		return GFX_NOT_VISIBLE;
	} else {
		return GFX_PARTIALLY_VISIBLE;
	}
}

float /*GFXDRVAPI*/ GFXSphereInFrustum (float f [6][4],const Vector &Cnt, float radius) {
   int p;
   float d;
   for( p = 0; p < 5; p++ )//does not evaluate for yon
   {
      d = f[p][0] * Cnt.i + f[p][1] * Cnt.j + f[p][2] * Cnt.k + f[p][3];
      if( d <= -radius )
         return 0;
   }
   return d;
}

void GFXGetFrustumVars (bool retr, float *l, float *r, float *b, float *t, float *n, float *f) {
  static float nnear,ffar,left,right,bot,top;//Visual C++ reserves near and far
  if (!retr) {
    nnear = *n;
    ffar = *f;
    left = *l;
    right = *r;
    bot = *b;
    top = *t;
  } else {
    *l = left;
    *r = right;
    *b = bot;
    *t = top;
    *n = nnear;
    *f = ffar;
  }
  //  fprintf (stderr,"<FUN%f,%f,%f,%f,%f,%f>>",near,far,left,right,bot,top);

}

void /*GFXDRVAPI*/ GFXGetFrustum(float f[6][4]) {
  f = frust;
}
void /*GFXDRVAPI*/ GFXBoxInFrustumModel (const Matrix model) {
  Matrix tmp;
  MultMatrix (tmp,view,model);
  GFXCalculateFrustum (BoxFrust,tmp,projection);
}
void /*GFXDRVAPI*/ GFXCalculateFrustum() {
  GFXCalculateFrustum (frust,view,projection);
}

void /*GFXDRVAPI*/ GFXCalculateFrustum (float frustum[6][4], float *modl,float * proj){
////float   *proj=projection;
////float   *modl=view;
   float   clip[16];
   float   t;

   /* Get the current PROJECTION matrix from OpenGL */
   //glGetFloatv( GL_PROJECTION_MATRIX, proj );

   /* Get the current MODELVIEW matrix from OpenGL */
   //glGetFloatv( GL_MODELVIEW_MATRIX, modl );

   /* Combine the two matrices (multiply projection by modelview) */
   clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
   clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
   clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
   clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

   clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
   clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
   clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
   clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

   clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
   clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
   clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
   clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

   clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
   clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
   clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
   clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

   /* Extract the numbers for the RIGHT plane */
   frustum[0][0] = clip[ 3] - clip[ 0];
   frustum[0][1] = clip[ 7] - clip[ 4];
   frustum[0][2] = clip[11] - clip[ 8];
   frustum[0][3] = clip[15] - clip[12];

   /* Normalize the result */
   t = sqrtf( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
   frustum[0][0] /= t;
   frustum[0][1] /= t;
   frustum[0][2] /= t;
   frustum[0][3] /= t;

   /* Extract the numbers for the LEFT plane */
   frustum[1][0] = clip[ 3] + clip[ 0];
   frustum[1][1] = clip[ 7] + clip[ 4];
   frustum[1][2] = clip[11] + clip[ 8];
   frustum[1][3] = clip[15] + clip[12];

   /* Normalize the result */
   t = sqrtf( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
   frustum[1][0] /= t;
   frustum[1][1] /= t;
   frustum[1][2] /= t;
   frustum[1][3] /= t;

   /* Extract the BOTTOM plane */
   frustum[2][0] = clip[ 3] + clip[ 1];
   frustum[2][1] = clip[ 7] + clip[ 5];
   frustum[2][2] = clip[11] + clip[ 9];
   frustum[2][3] = clip[15] + clip[13];

   /* Normalize the result */
   t = sqrtf( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
   frustum[2][0] /= t;
   frustum[2][1] /= t;
   frustum[2][2] /= t;
   frustum[2][3] /= t;

   /* Extract the TOP plane */
   frustum[3][0] = clip[ 3] - clip[ 1];
   frustum[3][1] = clip[ 7] - clip[ 5];
   frustum[3][2] = clip[11] - clip[ 9];
   frustum[3][3] = clip[15] - clip[13];

   /* Normalize the result */
   t = sqrtf( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
   frustum[3][0] /= t;
   frustum[3][1] /= t;
   frustum[3][2] /= t;
   frustum[3][3] /= t;

   /* Extract the FAR plane */
   frustum[5][0] = clip[ 3] - clip[ 2];
   frustum[5][1] = clip[ 7] - clip[ 6];
   frustum[5][2] = clip[11] - clip[10];
   frustum[5][3] = clip[15] - clip[14];

   /* Normalize the result */
   t = sqrtf( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
   frustum[5][0] /= t;
   frustum[5][1] /= t;
   frustum[5][2] /= t;
   frustum[5][3] /= t;

   /* Extract the NEAR plane */
   frustum[4][0] = clip[ 3] + clip[ 2];
   frustum[4][1] = clip[ 7] + clip[ 6];
   frustum[4][2] = clip[11] + clip[10];
   frustum[4][3] = clip[15] + clip[14];

   /* Normalize the result */
   t = sqrtf( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
   frustum[4][0] /= t;
   frustum[4][1] /= t;
   frustum[4][2] /= t;
   frustum[4][3] /= t;
   
}

/**
 * GFXGetXPerspective () returns the number that x/z is multiplied by to 
 * land a pixel on the screen.  
 * | xs 0  a 0 |[x]   [xs + az]          [1/xs 0   0  a/xs][x]   [x/xs+ aw/xs]
 * | 0  ys b 0 |[y] = [ys + bz]    ^-1   [ 0  1/ys 0  b/ys][y] = [y/ys+ bw/ys]
 * | 0  0  c d |[z]   [cz + dw]          [ 0   0   0  -1  ][z]   [0          ]
 * | 0  0 -1 0 |[w]   [-z     ]          [ 0   0  1/d c/d ][w]   [z/d + cw/d ]
 * therefore   return 1/(xs *d) and 1/(ys * d) 
 * I'm not good with matrix math...tell me if I should ret 1/xs+c/d instead
 * for test cases I can think of, it doesn't matter--- */
float GFXGetZPerspective (const float z) {
#if 0
  float left,right,bottom,top,nearval,farval;
  GFXGetFrustumVars(true,&left,&right,&bottom,&top,&nearval,&farval);
   float c = (farval+nearval) / ( farval-nearval);
   float d = (farval*(right-left)) / (farval-nearval);  /* error? */
   //return c*z+d;
#endif
   return z;
}
