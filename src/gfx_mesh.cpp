/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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
#include <memory.h>
#include "gfx_aux.h"
#include "gfx_mesh.h"
#include "gfx_transform.h"
#include "gfx_aux.h"
#include "gfx_camera.h"
#include "gfx_bounding_box.h"
#include "gfx_bsp.h"
#include <assert.h>

#include <math.h>

#include <string>
#include <fstream>
using namespace std;

#include "gfxlib.h"

#include "hashtable.h"
#include "vegastrike.h"

#include <GL/gl.h>

Hashtable<string, Mesh, char [513]> Mesh::meshHashTable;
list<Mesh*> undrawn_meshes[NUM_MESH_SEQUENCE]; // lower priority means draw first
extern list<Logo*> undrawn_logos;
Vector mouseline;



void Mesh::SetPosition (float x,float y, float z) {
  local_transformation.position = Vector (x,y,z);
}
void Mesh::SetPosition (const Vector &k) {
  local_transformation.position = k;
}
void Mesh::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  local_transformation.orientation = Quaternion::from_vectors(p,q,r);
}

void Mesh::ProcessUndrawnMeshes() {
  GFXEnable(DEPTHWRITE);
  GFXEnable(DEPTHTEST);
  for(int a=0; a<NUM_MESH_SEQUENCE; a++) {
    while(undrawn_meshes[a].size()) {
      Mesh *m = undrawn_meshes[a].back();
      undrawn_meshes[a].pop_back();
      m->ProcessDrawQueue();
      m->will_be_drawn = false;
    }
  }
  while(undrawn_logos.size()) {
    Logo *l = undrawn_logos.back();
    undrawn_logos.pop_back();
    l->ProcessDrawQueue();
    l->will_be_drawn = false;
    }
}

int Mesh::dlist_count = 1;

void DrawVector(const Vector &start, const Vector &vect)
{ 
  //float x = start.i, y = start.j, z = start.k;
	//glBegin(GL_LINES);
	//glVertex3f(x, y, z);
	//glVertex3f(vect.i + x, vect.j + y, vect.k + z);
	//glEnd();
}

void Mesh::InitUnit()
{
	forcelogos = NULL;
	squadlogos = NULL;

	local_transformation = identity_transformation;
	blendSrc=ONE;
	blendDst=ZERO;
	changed = GFXTRUE;
	vlist=NULL;
	
	radialSize=minSizeX=minSizeY=minSizeZ=maxSizeY=maxSizeZ=maxSizeX=0;
	//GFXVertex *vertexlist;
	GFXVertex *alphalist;

	vertexlist = NULL;
	//	stcoords = NULL;
	Decal = NULL;
	
	bspTree = NULL;

	alphalist = NULL;
	
	//	texturename[0] = -1;
	numforcelogo = numsquadlogo = 0;
	GFXGetMaterial(0, myMat);
	myMatNum = 0;
	//	scale = Vector(1.0,1.0,1.0);
	refcount = 1;  //FIXME VEGASTRIKE  THIS _WAS_ zero...NOW ONE
	orig = NULL;
	
	envMap = GFXTRUE;
	dlist = 0;
	draw_queue = NULL;
	hash_name = NULL;
	will_be_drawn = false;
	draw_sequence = 1;
}

Mesh::Mesh()
{
	InitUnit();
}

Mesh:: Mesh(const char * filename, bool xml)
{
	int TexNameLength;
	char *TexName=NULL;
	char * TexNameA=NULL;
	int NumPoints;
	float *x;
	float *y;
	float *z;
	float *i;
	float *j;
	float *k;
	int NumTris;
	int *Tris;
	int NumQuads;
	int *Quads;
	GFXBOOL AlphaMap = GFXFALSE;

	InitUnit();

	Mesh *oldmesh;
	if(0 != (oldmesh = meshHashTable.Get(string(filename))))//h4w h4s#t4bl3 1z 1337
	{
		*this = *oldmesh;
		oldmesh->refcount++;
		orig = oldmesh;
		return;
	}
	else
	{
	  //oldmesh = (Mesh*)malloc(sizeof(Mesh));
	  oldmesh = new Mesh();
	  meshHashTable.Put(string(filename), oldmesh);
	  hash_name = new string(filename);
	  draw_queue = new vector<MeshDrawContext>;
	}

	//	strcpy(name, filename);
	if(xml) {
	  LoadXML(filename, oldmesh);
	  return;
	} else {
	  this->xml= NULL;
	}

	FILE* fp = NULL;
	int jj;
	fp = fopen (filename, "r+b");
	if (!fp)
	{
	  fprintf (stderr,"Failed to load file %s",filename);
	  exit(1);
	}
		
	TexNameLength = readi(fp);
	if (TexNameLength)
	{
		if (TexNameLength < 0) 
		{
			AlphaMap = GFXTRUE;
			TexNameLength = -TexNameLength;
		}
		
		objtex = GFXTRUE;
		TexName = new char [TexNameLength +5];
		fread (TexName, TexNameLength,1,fp);
		TexName[TexNameLength+4] = '\0';
		TexName[TexNameLength+0] = '.';
		TexName[TexNameLength+1] = 'b';
		TexName[TexNameLength+2] = 'm';
		TexName[TexNameLength+3] = 'p';
	}
	else
	{
		objtex = GFXFALSE;
	}
	NumPoints = readi(fp);
	x = new float [NumPoints];
	y = new float [NumPoints];
	z = new float [NumPoints];
	i = new float [NumPoints];
	j = new float [NumPoints];
	k = new float [NumPoints]; 
	int ii;
	
	for (ii=0; ii<NumPoints; ii++)
	{
	  
		x[ii] = readf (fp);
		if (x[ii]>maxSizeX)
		  maxSizeX = x[ii];
		if (x[ii]<minSizeX)
		  minSizeX=x[ii];
		y[ii] = readf (fp);

		if (y[ii]>maxSizeY)
		  maxSizeY = y[ii];
		if (y[ii]<minSizeY)
		  minSizeY=y[ii];
		z[ii] = readf (fp);

		if (z[ii]>maxSizeZ)
		  maxSizeZ = z[ii];
		if (z[ii]<minSizeZ)
		  minSizeZ=z[ii];
		i[ii] = -readf (fp);
		j[ii] = -readf (fp);
		k[ii] = -readf (fp);
		
	}
	//below, the square fo teh radial size, because sqrtf will be useless l8r
	radialSize = .5*sqrtf ((maxSizeX-minSizeX)*(maxSizeX-minSizeX)+(maxSizeY-minSizeY)*(maxSizeY-minSizeY)+(maxSizeX-minSizeZ)*(maxSizeX-minSizeZ));	
	NumTris = readi (fp);
	Tris = new int [NumTris*3];

	for (ii=0; ii< NumTris;ii++)
		for (int jj=0; jj<3; jj++)
			Tris[ii*3+jj] = readi(fp);
	NumQuads = readi (fp);
	Quads = new int [NumQuads*4];
	for (ii=0; ii< NumQuads;ii++)
		for (int jj=0; jj<4; jj++)
			Quads[ii*4+jj] = readi(fp);

	
	//	int numtrivertex = NumTris*3;
	//	int numquadvertex = NumQuads*4;
	numvertex = NumTris*3+NumQuads*4;
	//GFXVertex *vertexlist;
	//GFXVertex *alphalist;

	vertexlist = new GFXVertex [numvertex];
	//	stcoords = new float[numvertex * 2+10];

	jj=0;
	for (ii=0; ii<NumTris; ii++)
	{
		vertexlist[jj].x = x[Tris[ii*3+0]];
		vertexlist[jj].y = y[Tris[ii*3+0]];
		vertexlist[jj].z = z[Tris[ii*3+0]];
		vertexlist[jj].i = i[Tris[ii*3+0]];
		vertexlist[jj].j = j[Tris[ii*3+0]];
		vertexlist[jj].k = k[Tris[ii*3+0]];
		jj++;
		vertexlist[jj].x = x[Tris[ii*3+1]];
		vertexlist[jj].y = y[Tris[ii*3+1]];
		vertexlist[jj].z = z[Tris[ii*3+1]];
		vertexlist[jj].i = i[Tris[ii*3+1]];
		vertexlist[jj].j = j[Tris[ii*3+1]];
		vertexlist[jj].k = k[Tris[ii*3+1]];
		jj++;
		vertexlist[jj].x = x[Tris[ii*3+2]];
		vertexlist[jj].y = y[Tris[ii*3+2]];
		vertexlist[jj].z = z[Tris[ii*3+2]];
		vertexlist[jj].i = i[Tris[ii*3+2]];
		vertexlist[jj].j = j[Tris[ii*3+2]];
		vertexlist[jj].k = k[Tris[ii*3+2]];
		jj++;
	}
	
	for (ii=0; ii<NumQuads; ii++)
	{
		vertexlist[jj].x = x[Quads[ii*4+0]];
		vertexlist[jj].y = y[Quads[ii*4+0]];
		vertexlist[jj].z = z[Quads[ii*4+0]];
		vertexlist[jj].i = i[Quads[ii*4+0]];
		vertexlist[jj].j = j[Quads[ii*4+0]];
		vertexlist[jj].k = k[Quads[ii*4+0]];
		jj++;
		vertexlist[jj].x = x[Quads[ii*4+1]];
		vertexlist[jj].y = y[Quads[ii*4+1]];
		vertexlist[jj].z = z[Quads[ii*4+1]];
		vertexlist[jj].i = i[Quads[ii*4+1]];
		vertexlist[jj].j = j[Quads[ii*4+1]];
		vertexlist[jj].k = k[Quads[ii*4+1]];
		jj++;
		vertexlist[jj].x = x[Quads[ii*4+2]];
		vertexlist[jj].y = y[Quads[ii*4+2]];
		vertexlist[jj].z = z[Quads[ii*4+2]];
		vertexlist[jj].i = i[Quads[ii*4+2]];
		vertexlist[jj].j = j[Quads[ii*4+2]];
		vertexlist[jj].k = k[Quads[ii*4+2]];
		jj++;
		vertexlist[jj].x = x[Quads[ii*4+3]];
		vertexlist[jj].y = y[Quads[ii*4+3]];
		vertexlist[jj].z = z[Quads[ii*4+3]];
		vertexlist[jj].i = i[Quads[ii*4+3]];
		vertexlist[jj].j = j[Quads[ii*4+3]];
		vertexlist[jj].k = k[Quads[ii*4+3]];
		jj++;

	}

	if (objtex)
	{

		jj=0;
		int temp = NumTris*3;
		//float oo256 = .00390625;
		/*long pos =*/ ftell(fp);
		for (ii=0; ii< temp; ii++)
		{
			vertexlist[ii].s = readf(fp);//*oo256;  
			vertexlist[ii].t = readf (fp);//*oo256;
		}
		
		temp = NumTris*3+NumQuads*4;
		for (; ii<temp; ii++)
		{
			vertexlist[ii].s = readf(fp);//*oo256;
			vertexlist[ii].t = readf(fp);//*oo256;
		}
		if (AlphaMap)
		{
			TexNameA = new char [TexNameLength +5];
			for (int kk = 0; kk < TexNameLength;kk++)
				TexNameA[kk] = TexName[kk];
			TexNameA[TexNameLength+4] = '\0';
			TexNameA[TexNameLength+0] = '.';
			TexNameA[TexNameLength+1] = 'a';
			TexNameA[TexNameLength+2] = 'l';
			TexNameA[TexNameLength+3] = 'p';
			//Decal =	new Texture(TexName, TexNameA);
			Decal =	new Texture(TexName, 0);
		}
		else 
		{
			Decal = new Texture (TexName, 0);
		}
		if (!Decal)
			objtex = GFXFALSE;
	}
	numforcelogo = readi (fp);
	Vector *PolyNormal = new Vector [numforcelogo];
	Vector *center = new Vector [numforcelogo];
	float *sizes = new float [numforcelogo];
	float *rotations = new float [numforcelogo];
	float *offset = new float [numforcelogo];
	char polytype;
	int offst;
	Vector *Ref;
	Ref = new Vector [numforcelogo];
	for (ii=0; ii<numforcelogo;ii++)
	{
		Ref[ii] = Vector (0,0,0);
		center[ii].i = readf(fp);
		center[ii].j = readf(fp);
		center[ii].k = readf(fp);
		polytype = readc (fp);
		switch (polytype)
		{
		case 'T':
		case 't': offst = 3*readi(fp);
			break;
		case 'D':
		case 'A': offst = 3*readi(fp);
			Ref[ii].i= vertexlist[offst+1].x - vertexlist[offst].x;
			Ref[ii].j= vertexlist[offst+1].y - vertexlist[offst].y;
			Ref[ii].k= vertexlist[offst+1].z - vertexlist[offst].z;
			break;
		case 'E':
		case 'B': offst = 3*readi(fp);
			Ref[ii].i= vertexlist[offst+2].x - vertexlist[offst+1].x;
			Ref[ii].j= vertexlist[offst+2].y - vertexlist[offst+1].y;
			Ref[ii].k= vertexlist[offst+2].z - vertexlist[offst+1].z;
			break;
		case 'F':
		case 'C': offst = 3*readi(fp);
			Ref[ii].i= vertexlist[offst].x - vertexlist[offst+2].x;
			Ref[ii].j= vertexlist[offst].y - vertexlist[offst+2].y;
			Ref[ii].k= vertexlist[offst].z - vertexlist[offst+2].z;
			break;
		case 'Q':
		case 'q': offst = 3*NumTris+4*readi(fp);
			break;
		case '0':
		case '4':
			offst = 3*NumTris+4*readi(fp);
			Ref[ii].i= vertexlist[offst+1].x - vertexlist[offst].x;
			Ref[ii].j= vertexlist[offst+1].y - vertexlist[offst].y;
			Ref[ii].k= vertexlist[offst+1].z - vertexlist[offst].z;
			break;
		case '5':
		case '1': offst = 3*NumTris+4*readi(fp);
			Ref[ii].i= vertexlist[offst+2].x - vertexlist[offst+1].x;
			Ref[ii].j= vertexlist[offst+2].y - vertexlist[offst+1].y;
			Ref[ii].k= vertexlist[offst+2].z - vertexlist[offst+1].z;
			break;
		case '6':
		case '2': offst = 3*NumTris+4*readi(fp);
			Ref[ii].i= vertexlist[offst+3].x - vertexlist[offst+2].x;
			Ref[ii].j= vertexlist[offst+3].y - vertexlist[offst+2].y;
			Ref[ii].k= vertexlist[offst+3].z - vertexlist[offst+2].z;
			break;
		case '7':
		case '3': offst = 3*NumTris+4*readi(fp); //total number of triangles incl pents
			Ref[ii].i= vertexlist[offst].x - vertexlist[offst+3].x;
			Ref[ii].j= vertexlist[offst].y - vertexlist[offst+3].y;
			Ref[ii].k= vertexlist[offst].z - vertexlist[offst+3].z;
			break;
		}
		switch (polytype)
		{
		case '4':
		case '5':
		case '6':
		case '7':
		case 'D':
		case 'E':
		case 'F':
			Ref[ii].i = -Ref[ii].i;
			Ref[ii].j = -Ref[ii].j;
			Ref[ii].k = -Ref[ii].k;
		break;
		}
		PolyNormal[ii] = PolygonNormal(
			Vector(vertexlist[offst].x,vertexlist[offst].y,vertexlist[offst].z),
			Vector(vertexlist[offst+1].x,vertexlist[offst+1].y,vertexlist[offst+1].z),
			Vector(vertexlist[offst+2].x,vertexlist[offst+2].y,vertexlist[offst+2].z)
			);

		sizes[ii] = readf (fp);
		rotations[ii] = readf (fp);
		offset [ii] = readf (fp);
		
	}
	forcelogos = new Logo(numforcelogo,center,PolyNormal,sizes ,rotations, 0.01F, _GFX->getForceLogo(),Ref);
	delete [] Ref;
	delete []PolyNormal;
	delete []center;
	delete [] sizes;
	delete [] rotations;
	delete [] offset;
	for (ii=0; ii< NumTris*3; ii+=3)
	{
		Vector Norm1 (vertexlist[ii+1].x-vertexlist[ii].x,vertexlist[ii+1].y-vertexlist[ii].y,vertexlist[ii+1].z-vertexlist[ii].z);
		Vector Norm2 (vertexlist[ii+2].x-vertexlist[ii].x,vertexlist[ii+2].y-vertexlist[ii].y,vertexlist[ii+2].z-vertexlist[ii].z);
		Vector Normal;
				CrossProduct (Norm2, Norm1, Normal);
		//CrossProduct (Norm1,Norm2,Normal);
		Normalize(Normal);
		vertexlist[ii].i =  vertexlist[ii+1].i = vertexlist[ii+2].i =Normal.i;
		vertexlist[ii].j =  vertexlist[ii+1].j = vertexlist[ii+2].j =Normal.j;
		vertexlist[ii].k =  vertexlist[ii+1].k = vertexlist[ii+2].k =Normal.k;
	}
	for (ii=NumTris*3; ii< NumTris*3+NumQuads*4; ii+=4)
	{
		Vector Norm1 (vertexlist[ii+1].x-vertexlist[ii].x,vertexlist[ii+1].y-vertexlist[ii].y,vertexlist[ii+1].z-vertexlist[ii].z);
		Vector Norm2 (vertexlist[ii+3].x-vertexlist[ii].x,vertexlist[ii+3].y-vertexlist[ii].y,vertexlist[ii+3].z-vertexlist[ii].z);
		Vector Normal;
				CrossProduct (Norm2, Norm1, Normal);
				//CrossProduct (Norm1,Norm2,Normal);
		Normalize(Normal);
		vertexlist[ii].i =  vertexlist[ii+1].i = vertexlist[ii+2].i = vertexlist[ii+3].i =Normal.i;
		vertexlist[ii].j =  vertexlist[ii+1].j = vertexlist[ii+2].j = vertexlist[ii+3].j =Normal.j;
		vertexlist[ii].k =  vertexlist[ii+1].k = vertexlist[ii+2].k = vertexlist[ii+3].k =Normal.k;

	}	

	numsquadlogo = readi (fp);
	PolyNormal = new Vector [numsquadlogo];
	center = new Vector [numsquadlogo];
	sizes = new float [numsquadlogo];
	rotations = new float [numsquadlogo];
	offset = new float [numsquadlogo];
	//char polytype;
	//int offset;
	Ref = new Vector [numsquadlogo];
	for (ii=0; ii<numsquadlogo;ii++)
	{
		Ref[ii] = Vector (0,0,0);
		center[ii].i = readf(fp);
		center[ii].j = readf(fp);
		center[ii].k = readf(fp);
		polytype = readc (fp);
		switch (polytype)
		{
		case 'T':
		case 't': offst = 3*readi(fp);
			break;
		case 'D':
		case 'A': offst = 3*readi(fp);
			Ref[ii].i= vertexlist[offst+1].x - vertexlist[offst].x;
			Ref[ii].j= vertexlist[offst+1].y - vertexlist[offst].y;
			Ref[ii].k= vertexlist[offst+1].z - vertexlist[offst].z;
			break;
		case 'E':
		case 'B': offst = 3*readi(fp);
			Ref[ii].i= vertexlist[offst+2].x - vertexlist[offst+1].x;
			Ref[ii].j= vertexlist[offst+2].y - vertexlist[offst+1].y;
			Ref[ii].k= vertexlist[offst+2].z - vertexlist[offst+1].z;
			break;
		case 'F':
		case 'C': offst = 3*readi(fp);
			Ref[ii].i= vertexlist[offst].x - vertexlist[offst+2].x;
			Ref[ii].j= vertexlist[offst].y - vertexlist[offst+2].y;
			Ref[ii].k= vertexlist[offst].z - vertexlist[offst+2].z;
			break;
		case 'Q':
		case 'q': offst = 3*NumTris+4*readi(fp);
			break;
		case '0':
		case '4':
			offst = 3*NumTris+4*readi(fp);
			Ref[ii].i= vertexlist[offst+1].x - vertexlist[offst].x;
			Ref[ii].j= vertexlist[offst+1].y - vertexlist[offst].y;
			Ref[ii].k= vertexlist[offst+1].z - vertexlist[offst].z;
			break;
		case '5':
		case '1': offst = 3*NumTris+4*readi(fp);
			Ref[ii].i= vertexlist[offst+2].x - vertexlist[offst+1].x;
			Ref[ii].j= vertexlist[offst+2].y - vertexlist[offst+1].y;
			Ref[ii].k= vertexlist[offst+2].z - vertexlist[offst+1].z;
			break;
		case '6':
		case '2': offst = 3*NumTris+4*readi(fp);
			Ref[ii].i= vertexlist[offst+3].x - vertexlist[offst+2].x;
			Ref[ii].j= vertexlist[offst+3].y - vertexlist[offst+2].y;
			Ref[ii].k= vertexlist[offst+3].z - vertexlist[offst+2].z;
			break;
		case '7':
		case '3': offst = 3*NumTris+4*readi(fp); //total number of triangles incl pents
			Ref[ii].i= vertexlist[offst].x - vertexlist[offst+3].x;
			Ref[ii].j= vertexlist[offst].y - vertexlist[offst+3].y;
			Ref[ii].k= vertexlist[offst].z - vertexlist[offst+3].z;
			break;
		}
		switch (polytype)
		{
		case '4':
		case '5':
		case '6':
		case '7':
		case 'D':
		case 'E':
		case 'F':
			Ref[ii].i = -Ref[ii].i;
			Ref[ii].j = -Ref[ii].j;
			Ref[ii].k = -Ref[ii].k;
		break;
		}
		PolyNormal[ii] = PolygonNormal(
			Vector(vertexlist[offst].x,vertexlist[offst].y,vertexlist[offst].z),
			Vector(vertexlist[offst+1].x,vertexlist[offst+1].y,vertexlist[offst+1].z),
			Vector(vertexlist[offst+2].x,vertexlist[offst+2].y,vertexlist[offst+2].z)
			);
		sizes[ii] = readf (fp);
		rotations[ii] = readf (fp);
		offset [ii] = readf (fp);
	}
	

	squadlogos = new Logo(numsquadlogo,center,PolyNormal,sizes ,rotations, (float)0.01, _GFX->getSquadLogo(), Ref);
	delete [] Ref;
	//fprintf (stderr, "Ri:%f Rj: %f Rk %f",vertexlist[0].i,vertexlist[0].j,vertexlist[0].k);
	int vert_offset[2];
	vert_offset[0]=NumTris*3;
	vert_offset[1]=NumQuads*4;
	enum POLYTYPE modes [2];
	modes[0]=GFXTRI;
	modes[1]=GFXQUAD;
	vlist = new GFXVertexList(modes,NumTris*3+NumQuads*4, vertexlist,2,vert_offset);
	//vlist = new GFXVertexList(numtris*4,0,numquads*4, vertexlist+numtris*3);
	/*long pos =*/ ftell(fp);
	myMatNum = readi(fp);;
	fclose(fp);

	// Load the BSP tree
	//bspTree = new BSPTree((filename + string(".bsp")).c_str());

	this->orig = oldmesh;
	*oldmesh = *this;
	oldmesh->orig = NULL;
	oldmesh->refcount++;

	delete [] vertexlist;
	delete []PolyNormal;
	delete []center;
	delete [] sizes;
	delete [] rotations;
	delete [] offset;

}

Mesh::~Mesh()
{
	if(!orig||orig==this)
	{
	  delete vlist;
		//if(vertexlist != NULL)
		//	delete [] vertexlist;
	  //	  if(stcoords != NULL) {
	  //			delete [] stcoords;
	  //			stcoords = NULL;
	  //	  }
		//if(alphalist != NULL)
		//	delete [] alphalist;
	  if(Decal != NULL) {
	    delete Decal;
	    Decal = NULL;
	  }
	  if (squadlogos!=NULL) {
	    delete squadlogos;
	    squadlogos= NULL;
	  }
	  if (forcelogos!=NULL) {
	    delete forcelogos;
	    forcelogos = NULL;
	  }
	  if(bspTree!=NULL) {
	    
	    delete bspTree;
	    bspTree= NULL;
	  }
	  if(hash_name!=NULL) {
	    meshHashTable.Delete(*hash_name);
	    delete hash_name;
	  }
	  if(draw_queue!=NULL)
	    delete draw_queue;
	} else {
	  orig->refcount--;
	  //printf ("orig refcount: %d",refcount);
	  if(orig->refcount == 0)
	    delete orig;
	}
}
float const ooPI = 1.00F/3.1415926535F;

//#include "d3d_internal.h"

void Mesh::Draw(const Transformation &trans, const Matrix m)
{
  Matrix cumulative_transformation_matrix;
  Transformation cumulative_transformation = local_transformation;
  cumulative_transformation.Compose(trans, m);
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  MeshDrawContext c(cumulative_transformation_matrix);
  orig->draw_queue->push_back(c);
  if(!orig->will_be_drawn) {
    orig->will_be_drawn = true;
    undrawn_meshes[draw_sequence].push_back(orig);
  }
}

void Mesh::ProcessDrawQueue() {
  assert(draw_queue->size());
  GFXSelectMaterial(myMatNum);
  //GFXEnable(LIGHTING);
  GFXDisable (LIGHTING);
  //static float rot = 0;
  //GFXColor(1.0, 1.0, 1.0, 1.0);
  
  GFXEnable(TEXTURE0);
  GFXEnable(CULLFACE);
  if(envMap) {
    GFXEnable(TEXTURE1);
  } else {
    GFXDisable(TEXTURE1);
  }
  if(Decal)
    Decal->MakeActive();
  
  GFXSelectTexcoordSet(0, 0);
  if(envMap) {
    //_GFX->getLightMap()->MakeActive();
    _GFX->activateLightMap();
    GFXSelectTexcoordSet(1, 1);
  }
  GFXBlendMode(blendSrc, blendDst);
  vlist->LoadDrawState();	
  while(draw_queue->size()) {
    MeshDrawContext c = draw_queue->back();
    draw_queue->pop_back();
    GFXLoadMatrix(MODEL, c.mat);
    GFXPickLights (c.mat/*GetPosition()*/);
    vlist->Draw();
    if(0!=forcelogos) {
      forcelogos->Draw(c.mat);
    }
    if (0!=squadlogos){
      squadlogos->Draw(c.mat);
    }
  }
}

void Mesh::Destroy()
{
}

bool Mesh::intersects(const Vector &start, const Vector &end) {
	return bspTree->intersects(start, end);
}


BoundingBox * Mesh::getBoundingBox() {
  
  BoundingBox * tbox = new BoundingBox (Vector (minSizeX,0,0),Vector (maxSizeX,0,0),
					Vector (0,minSizeY,0),Vector (0,maxSizeY,0),
					Vector (0,0,minSizeZ),Vector (0,0,maxSizeZ));
  tbox->Transform (local_transformation);
  return tbox;
}

bool Mesh::intersects(const Vector &pt/*, Transformation cumulative_transformation*/) {

  /*
  Transformation tmp = cumulative_transformation;
  tmp.Invert();
  Matrix t;
  tmp.to_matrix(t);
//UNIT SHOULD HANDLE THIS...it knows about the cumulative_transformation based on last physics round

  Vector a = pt;
  a = a.Transform(t);
  */
  return bspTree->intersects(/*a*/pt);
}

bool Mesh::intersects(Mesh *mesh) {
  // Needs to adapt coordinate systems
	return bspTree->intersects(mesh->bspTree);
}
