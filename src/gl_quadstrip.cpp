/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "gfxlib.h"
#include "vegastrike.h"
#include "gl_globals.h"

extern BOOL bTex0;
extern BOOL bTex1;


GFXQuadstrip::GFXQuadstrip(int numVertices,GFXVertex *vertices): numVertices(numVertices)
{
  // error check # of vertices
	myVertices = new GFXVertex[numVertices];
	memcpy(myVertices, vertices, sizeof(GFXVertex)*numVertices);
}

GFXQuadstrip::~GFXQuadstrip()
{
	if(myVertices)
		delete [] myVertices;
}

GFXTVertex *GFXQuadstrip::LockTransformed()
{
	return NULL;
} // Stuff to support environment mapping

void GFXQuadstrip::UnlockTransformed()
{
}

GFXVertex *GFXQuadstrip::LockUntransformed()
{
	
	return myVertices;
}// Stuff to support environment mapping

void GFXQuadstrip::UnlockUntransformed()
{
}

BOOL GFXQuadstrip::SwapUntransformed()
{
	return FALSE;
}

BOOL GFXQuadstrip::SwapTransformed()
{
	return FALSE;
}

BOOL GFXQuadstrip::Draw()
{



#ifdef STATS_QUEUE
  statsqueue.back() += GFXStats(0, (numVertices-2)/2, 0);
#endif
	if(g_game.Multitexture) {
		glActiveTextureARB(GL_TEXTURE0_ARB);	
		if(bTex0) 
			glEnable (GL_TEXTURE_2D);		
		else
			glDisable(GL_TEXTURE_2D);

		glActiveTextureARB(GL_TEXTURE1_ARB);	
		if(bTex1)
#ifdef NV_CUBE_MAP
		  glEnable (GL_TEXTURE_CUBE_MAP_EXT);
#else
			glEnable (GL_TEXTURE_2D);
#endif
		else
			glDisable(GL_TEXTURE_2D);
	}
#ifdef STATS_QUEUE
	statsqueue.back() += GFXStats(0, (numVertices-2)/2, 0);
#endif
	//int num3tri = numTriangles*3;

	//float *texcoords = NULL;

	if (g_game.Multitexture)
	{
	  //GLenum err;
		//texcoords = new float[numVertices*4];
		//for(int a=0; a<numVertices; a++) {
		//	texcoords[4*a] = myVertices[a].s;
		//	texcoords[4*a+1] = myVertices[a].t;
		//	texcoords[4*a+2] = myVertices[a].u;
		//	texcoords[4*a+3] = myVertices[a].v;
		//}

		glVertexPointer(3, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].x);
		glNormalPointer(GL_FLOAT, sizeof(GFXVertex), &myVertices[0].i);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		glClientActiveTextureARB (GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);
		
		glClientActiveTextureARB (GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage1*2);

		glDrawArrays(GL_QUAD_STRIP, 0, numVertices);
	}
	else
	{ 
		//transfer vertex, texture coords, and normal pointer
	  //GLenum err;
		glVertexPointer(3, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].x);
		glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);
		glNormalPointer(GL_FLOAT, sizeof(GFXVertex), &myVertices[0].i);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		if(bTex0) {
		  glDrawArrays(GL_QUAD_STRIP, 0, numVertices);
		}

		if (Stage1Texture&&bTex1)
		{
/*			int ssrc,ddst;
			glGetIntegerv (GL_BLEND_SRC, &ssrc);
			glGetIntegerv (GL_BLEND_DST, &ddst);
			glBindTexture(GL_TEXTURE_2D, Stage1TextureName);
			glEnable (GL_BLEND);
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);

			//now transfer textures that correspond to second set of coords
			glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage1*2);
			glDrawArrays(GL_QUAD_STRIP, 0, numVertices);

			glBlendFunc (ssrc,ddst);
			glBindTexture(GL_TEXTURE_2D, Stage0TextureName);
			//glDisable (GL_BLEND);

			//reload the old texture pointer
			glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);
			*/   //if hardware doesn't support multitexture, don't even try it
		}

	}

	return TRUE;
}
