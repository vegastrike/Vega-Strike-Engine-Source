#include "gfx_mesh.h"
#include "vegastrike.h"
void Mesh::LoadBinary (const char * filename, Mesh * oldmesh) {
  GFXBOOL objtex;
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
		if (x[ii]>mx.i)
		  mx.i = x[ii];
		if (x[ii]<mn.i)
		  mn.i=x[ii];
		y[ii] = readf (fp);

		if (y[ii]>mx.j)
		  mx.j = y[ii];
		if (y[ii]<mn.j)
		  mn.j=y[ii];
		z[ii] = readf (fp);

		if (z[ii]>mx.k)
		  mx.k = z[ii];
		if (z[ii]<mn.k)
		  mn.k=z[ii];
		i[ii] = -readf (fp);
		j[ii] = -readf (fp);
		k[ii] = -readf (fp);
		
	}
	//below, the square fo teh radial size, because sqrtf will be useless l8r
	radialSize = .5*(mx-mn).Magnitude();	
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
	int numvertex = NumTris*3+NumQuads*4;
	GFXVertex *vertexlist;
	//GFXVertex *alphalist;

	vertexlist = new GFXVertex [numvertex];

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
	forcelogos = new Logo(numforcelogo,center,PolyNormal,sizes ,rotations, 0.01F, _Universe->getForceLogo(),Ref);
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
	

	squadlogos = new Logo(numsquadlogo,center,PolyNormal,sizes ,rotations, (float)0.01, _Universe->getSquadLogo(), Ref);
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
