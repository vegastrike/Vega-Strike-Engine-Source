#include "endianness.h"
#include <float.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "vsfilesystem.h"
#include "vsimage.h"
#include "aux_texture.h"
#include "gldrv/sdds.h"
#ifndef WIN32
#else
#include <windows.h>
#include <wingdi.h>
#endif

#define NumLights 1
static char *InputName=NULL;
static char *OutputName=NULL;
static bool pushdown=false;
static float affine=0;
static float multiplicitive=1;
static float power=1;

using namespace VSFileSystem;

/*
struct Vector {
	float i;
	float j;
	float k;
};
*/
struct RGBColor {
	float r,b,g;
};
struct Light {
	Vector Dir;
	RGBColor Ambient;
	RGBColor Intensity;
};
struct Material {
	RGBColor Ka;
	RGBColor Kd;
	RGBColor Ks;
	int exp;
};

static Light L [NumLights];
static Material M;
static float Power (float A, int B)
{
	float res = 1.;
	for (int i=0; i< B;i++)
	{
		res *= A;

	}
	return res;

}
static void Lighting (RGBColor &Col, const Vector &Norm)
{
	static float OONL = 1./NumLights;
	Col.r = Col.g = Col.b= 0;
	for (int i=0; i< NumLights; i++)
	{
		float dot = L[i].Dir.i*Norm.i+L[i].Dir.j*Norm.j+L[i].Dir.k*Norm.k;
		if (dot<0) dot =0;
		Col.r += OONL*L[i].Ambient.r*M.Ka.r + L[i].Intensity.r*(M.Kd.r*dot+M.Ks.r*Power(dot,M.exp));
		Col.g += OONL*L[i].Ambient.g*M.Ka.g + L[i].Intensity.g*(M.Kd.g*dot+M.Ks.g*Power(dot,M.exp));
		Col.b += OONL*L[i].Ambient.b*M.Ka.b + L[i].Intensity.b*(M.Kd.b*dot+M.Ks.b*Power(dot,M.exp));
		if (Col.r >1) Col.r = 1;if (Col.b >1) Col.b = 1;if (Col.g >1) Col.g = 1;
		if (Col.r <0) Col.r = 0;if (Col.b <0) Col.b = 0;if (Col.g <0) Col.g = 0;
	}



}
const int lmwid =512;
const int lmwido2=lmwid/2;
const float ooWIDTHo2 = 2./lmwid;
const float PIoWIDTHo2 = 2*3.1415926535/lmwid;

const char bytepp=3;
struct CubeCoord {
	float s;
	float t;
	char TexMap; //0 = front, 1=back,2=right,3=left,4=up,5=down
};
static float abso (float x)
{
	if (x>0)
		return x;
	else return -x;
}
static void gluSphereMap (CubeCoord &Tex, Vector Normal, float Theta) {
  Tex.TexMap=0;
  float vert = Normal.j;
  if (!pushdown) {
    Tex.t = vert*lmwido2+lmwido2;
    Tex.s = Theta;
  } else {
    Tex.t = ((int)(vert*lmwido2))%lmwid;
    Tex.s = Theta;
  }

}
static void TexMap (CubeCoord & Tex, Vector Normal)
{
	float r[6];
	Normal.i = Normal.i;
	Normal.j = -Normal.j;
	Normal.k = -Normal.k;
	const float CubeSize = lmwido2; //half of the length of any of the cube's sides
		if (Normal.k)
			r[0] = CubeSize / Normal.k; // find what you need to multiply to get to the cube
		if (Normal.i)
			r[2] = CubeSize / Normal.i; // find what you need to multiply to get to the cube
		if (Normal.j)
			r[4] = CubeSize / Normal.j; // find what you need to multiply to get to the cube
		if (!Normal.k)
			r[0] = r[1] = CubeSize*1000;
		if (!Normal.i)
			r[2] = r[3] = CubeSize*1000;
		if (!Normal.j)
			r[4] = r[5] = CubeSize*1000;
		r[1] = -r[0];
		r[3] = -r[2];
		r[5] = -r[4];

	float rf;
	rf  = CubeSize * 1000;
	for (int i=0; i<6; i++)
	{
		if (r[i] >= CubeSize)
		{
			if (rf > r[i])
			{
				rf = r[i];
				Tex.TexMap = i;
			}
		}
	}
	///find coordinates on this tex map 'box in space'
	switch (Tex.TexMap)
	{
	case 0:
		Tex.s = rf*Normal.i+lmwido2; // btw 0 and 256
		Tex.t = lmwido2- rf*Normal.j;// top left is 0,0
		break;
	case 1:
		Tex.s = lmwido2-rf*Normal.i; // btw 0 and 256
		Tex.t = lmwido2- rf*Normal.j;// top left is 0,0
		break;
	case 2:
		Tex.s = lmwido2 - rf*Normal.k;
		Tex.t = lmwido2 - rf*Normal.j;
		break;
	case 3:
	  Tex.s = lmwido2 + rf*Normal.k;
	    Tex.t = lmwido2 - rf*Normal.j;
		break;
	case 4:
		Tex.t = lmwido2 -rf*Normal.i;
		Tex.s = lmwido2 + rf*Normal.k;
		break;
	case 5:
	  Tex.s = lmwido2 - rf*Normal.i;
	    Tex.t = lmwido2 + rf*Normal.k;
		break;


	}
}
static bool LoadTex(char * FileName, unsigned char scdata [lmwid][lmwid][3]){
using namespace VSFileSystem;

  unsigned char ctemp;
  VSFile f;
  VSError err = f.OpenReadOnly( FileName, TextureFile);
	if (err>Ok)
	{
		return false;
	}
	Texture tex;
	unsigned char * data = tex.ReadImage( &f, texTransform, true);
	int bpp = tex.Depth();
	int format = tex.Format();

	unsigned char *buffer= NULL;
	bpp/=8;	
	// 999 is the code for DDS file, we must decompress them.
	if(format == 999){
		unsigned char *tmpbuffer = data + 2;
		TEXTUREFORMAT internformat;
		bpp = 1;
		// Make sure we are reading a DXT1 file. All backgrounds 
		// should be DXT1
		switch(tex.mode){
			case ::VSImage::_DXT1:
				internformat = DXT1;
				break;
			default:
				return(false);
		}
		// we could hardware decompress, but that involves more
		// pollution of gl in gfx. 
		ddsDecompress(tmpbuffer,buffer,internformat, tex.sizeY,tex.sizeX);
		// We are done with the DDS file data.  Remove it. 
		free(data);
		data = buffer;
		
		// stride and row_pointers are used for the texTransform 
		unsigned long stride = 4 * sizeof(unsigned char);
		unsigned char **row_pointers = (unsigned char**)malloc(sizeof(unsigned char*)*tex.sizeY);
		for(unsigned int i = 0;i < tex.sizeY;++i){
			row_pointers[i] = &data[i*stride*tex.sizeX];
		}
		// texTransform demands that the first argument (bpp) be 8. So we abide
		int tmp = 8;
		int tmp2 = PNG_HAS_COLOR + PNG_HAS_ALPHA;
		buffer = texTransform(tmp,tmp2,tex.sizeX,tex.sizeY,row_pointers);
		// We're done with row_pointers, free it
		free(row_pointers);
		row_pointers = NULL;
		// We're done with the decompressed dds data, free it
		free(data);
		// We set data to the transformed image data 
		data = buffer;
		buffer = NULL;
		// it's 3 because 24/8
		bpp =  4;
	} else if (format&PNG_HAS_ALPHA) {
	  bpp*=4;
	}else {
	  bpp*=3;
	}
	if (data) {
	  int ii;
	  int jj;
	  for (int i=0;i<lmwid;i++) {
	    ii=(i*tex.sizeY)/lmwid;
	    for (int j=0;j<lmwid;j++) {
	      jj= (j*tex.sizeX)/lmwid;
	      scdata[i][j][0]=data[(ii*tex.sizeX+jj)*bpp];
	      scdata[i][j][1]=data[(ii*tex.sizeX+jj)*bpp+1];
	      scdata[i][j][2]=data[(ii*tex.sizeX+jj)*bpp+2];
	    }
	  }
	  free(data);
	}
	else
		return false;

 	f.Close();
	return true;
}
struct Texmp
{
	unsigned char D [lmwid][lmwid][3];
};
static void Spherize (CubeCoord Tex [lmwid][lmwid],CubeCoord gluSph [lmwid][lmwid],unsigned char Col[])
{
	Texmp * Data = NULL;
	bool sphere = false;
	Data = new Texmp[6];
	if (!Data) 
	  return;//borken down and down Data[5], right Data[3]
	char *tmp= (char *)malloc (strlen (InputName)+60);;
	if (!(LoadTex (strcat (strcpy(tmp,InputName),"_front.image"),Data[0].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_back.image"),Data[1].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_left.image"),Data[2].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_right.image"),Data[3].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_up.image"),Data[4].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_down.image"),Data[5].D))) {
	  if (!LoadTex (strcat (strcpy(tmp,InputName),"_sphere.image"),Data[0].D )) {
	    LoadTex (strcat (strcpy(tmp,InputName),".image"),Data[0].D);
	  }
	  sphere=true;
	  Tex = gluSph;
	  
	}
	free (tmp);
	tmp=NULL;
	//int NumPix;
	float sleft,sright,tdown,tup;
	for (int t=0; t<lmwid;t++)
	{
		for (int s=0; s<lmwid; s++)
		{

			float r =0;
			float g =0;
			float b = 0;
			{
				float avg = 1;
				if ((int)floor (Tex[t][s].s)  >lmwid-1)
					Tex[t][s].s = lmwid-1;
				if ((int)floor (Tex[t][s].t)  >lmwid-1)
					Tex[t][s].t = lmwid-1;
				if ((int)floor (Tex[t][s].t)  <0)
					Tex[t][s].t = 0;
				if ((int)floor (Tex[t][s].s)  <0)
					Tex[t][s].s = 0;
				r = Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s)][0];
				g = Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s)][1];
				b = Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s)][2];
				if ((int)floor (Tex[t][s].s)  <lmwid-1)
				{
					avg ++;
					r += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s+1)][0];
					g += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s+1)][1];
					b += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s+1)][2];
				}
				if ((int)floor (Tex[t][s].t) <lmwid-1)
				{
					avg ++;
					r += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t+1)][(int)floor (Tex[t][s].s)][0];
					g += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t+1)][(int)floor (Tex[t][s].s)][1];
					b += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t+1)][(int)floor (Tex[t][s].s)][2];
				}
				if ((int)floor (Tex[t][s].t) >0)
				{
					avg ++;
					r += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t-1)][(int)floor (Tex[t][s].s)][0];
					g += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t-1)][(int)floor (Tex[t][s].s)][1];
					b += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t-1)][(int)floor (Tex[t][s].s)][2];
				}
				if ((int)floor (Tex[t][s].s)  >0)
				{
					avg ++;
					r += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s-1)][0];
					g += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s-1)][1];
					b += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s-1)][2];
				}
				r /= avg;
				g /= avg;
				b/= avg;
			}
			unsigned int rr=(unsigned int)r;
			unsigned int gg=(unsigned int)g;
			unsigned int bb=(unsigned int)b;
			if (affine!=0||multiplicitive!=1||power!=1) {
			  rr = (unsigned int)(affine + ((pow ((float)r,power)) * multiplicitive));
			  gg = (unsigned int)(affine + ((pow ((float)g,power)) * multiplicitive));
			  bb = (unsigned int)(affine + ((pow ((float)b,power)) * multiplicitive));
			}
			if (rr>255) rr= 255;
			if (gg>255) gg=255;
			if (bb>255) bb=255;

			Col[bytepp*(lmwid*(lmwid-1-t)+(lmwid-1-s))] = rr;
			Col[bytepp*(lmwid*(lmwid-1-t)+(lmwid-1-s))+1] = gg;
			Col[bytepp*(lmwid*(lmwid-1-t)+(lmwid-1-s))+2] = bb;
			//			Col[4*(256*(255-t)+(255-s))+3] = 255;
		}
	}



	delete []Data;
}
static void GenerateSphereMap()
{
	//float SinPhi;
	//float CosPhi;
	//float Theta;
	Vector Normal;
	//RGBColor Col;
	static CubeCoord TexCoord [lmwid][lmwid];
	static CubeCoord gluSphereCoord [lmwid][lmwid];
	unsigned char *LightMap =(unsigned char *)malloc (lmwid*lmwid*4);
	int t;
	for (t=0; t<lmwid; t++) //keep in mind that t = lmwido2 (sin phi) +lmwido2
	{
		float to256 = t / (104.*lmwid/256) -1.23;
		for (int s = 0; s < lmwid; s++) // is is none other than Theta * lmwido2/PI
		{
			float so256 = s / (104.*lmwid/256) -1.23;
			Normal.k = 2 *(1- so256*so256 - to256*to256);
			float double_one_more_normal = 2*(Normal.k+1);
			if( double_one_more_normal >=0)
			{
				Normal.i = so256 *sqrt (2*(Normal.k+1));
				Normal.j = to256 *sqrt (2*(Normal.k+1));
			}
			else
			{
				Normal.i = Normal.j = 0.0;
			}
			float sz = sqrt (Normal.k*Normal.k+Normal.j*Normal.j+Normal.i*Normal.i);
			Normal.k /= -sz;
			Normal.i /= sz;
			Normal.j /= sz;
			TexMap (TexCoord[t][s], Normal);//find what the lighting is
			gluSphereMap (gluSphereCoord[t][s],Normal,s);
		}	
	}
	Spherize (TexCoord,gluSphereCoord,LightMap);	
/*  We dont use this crap anymore.  png backgrounds, not bitmap
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	bmfh.bfType=19778;
	bmfh.bfSize=3;
	bmfh.bfReserved1=0;
	bmfh.bfReserved2=54;
	bmfh.bfOffBits=0;
	info.biSize=40;
	info.biWidth=lmwid;
	info.biHeight=lmwid;
	info.biPlanes=1;
	info.biBitCount=24;
	info.biCompression=0;
	info.biSizeImage=196608;
	info.biXPelsPerMeter=2834;
	info.biYPelsPerMeter=2834;
	info.biClrUsed=0;
	info.biClrImportant=0;
*/
	::VSImage image;
	image.WriteImage( (char *)OutputName, LightMap, PngImage, lmwid, lmwid, false, 8, TextureFile);
 
}
void EnvironmentMapGeneratorMain(const char * inpt, const char *outpt, float a, float m, float p, bool w)
{

    affine=a;
    multiplicitive=m;
    power=p;
    pushdown =w;
	int size = sizeof(char)*strlen(inpt)+40;
    char * tmp = (char *) malloc (size);
    strcpy (tmp,inpt);
	VSFile f;
	VSError err = f.OpenReadOnly( strcat (tmp,"_sphere.image"), TextureFile);
    if (err>Ok)
	{
	  memset( tmp, 0, size);
	  strcpy( tmp, inpt);
	  err = f.OpenReadOnly( strcat (tmp,"_up.image"), TextureFile);
	}
    //bool share = false;
    std::string s;
    if (err>Ok) {
      //s = VSFileSystem::GetSharedTexturePath (std::string (inpt));
      s = VSFileSystem::sharedtextures+"/"+string(inpt);
      InputName = (char *) malloc (sizeof (char)*(s.length()+2));
      strcpy (InputName,s.c_str());
    } else {
      f.Close();
      InputName = (char *) malloc (sizeof (char)*(strlen(inpt)+2));
      strcpy (InputName,inpt);
    }
    OutputName=strdup (outpt);
    free(tmp);
    tmp=NULL;
  VSFileSystem::vs_fprintf (stderr, "input name %s, output name %s\nAffine %f Mult %f Pow %f\n",InputName, OutputName, affine, multiplicitive, power);
  GenerateSphereMap();
  free (InputName);
  free (OutputName);
}














#if USEDEPRECATEDENVMAPGENT
/* SO PEOPLE KNOW ITS DEPRECATED
static void GenerateLightMap ()
{
	L[0].Dir.i = 0;//.403705173615;
	L[0].Dir.j = 1;//-.897122608033;
	L[0].Dir.k = 0;//.179424521607;	
	L[0].Ambient.r = 0;
	L[0].Ambient.g = 0;
	L[0].Ambient.b = 0;
	L[0].Intensity.r = 1;
	L[0].Intensity.g = 1;
	L[0].Intensity.b = 1;
	M.Ka.r = 0;
	M.Ka.g = 0;
	M.Ka.b = 0;
	M.Kd.r = 0;
	M.Kd.g = 0;
	M.Kd.b = 0;
	M.Ks.r = 1;
	M.Ks.g = 1;
	M.Ks.b = 1;
	M.exp = 60;
	float SinPhi;
	float CosPhi;
	float Theta;
	Vector Normal;
	RGBColor Col;
	unsigned char *LightMap= new unsigned char [lmwid*lmwid*4];
	for (int t=0; t<lmwid; t++) //keep in mind that t = lmwido2 (sin phi) +lmwido2
	{
		SinPhi = ((float)t)*ooWIDTHo2 -1;
		CosPhi = sqrt (1-SinPhi*SinPhi);//yes I know (-) ... but -PI/2<Phi<PI/2 so cos >0 like I said
		for (int s = 0; s < lmwid; s++) // is is none other than Theta * 128/PI
		{
			Theta = s*PIoWIDTHo2;// 128oPI = 128/3.1415926535
			//now that we have all this wonderful stuff, we must calculate lighting on this one point.
			// first calc the normal
			Normal.i = CosPhi * cos (Theta);
			Normal.j = CosPhi * sin (Theta);
			Normal.k = SinPhi;
			Lighting (Col, Normal);//find what the lighting is
			LightMap[lmwid*bytepp*t+bytepp*s] = (unsigned char) 255*Col.r;
			LightMap[lmwid*bytepp*t+bytepp*s+1] = (unsigned char) 255*Col.g;
			LightMap[lmwid*bytepp*t+bytepp*s+2] = (unsigned char) 255*Col.b;
		}
	}




	char tmp [lmwid];
	assert (0);
	strcpy (tmp,OutputName);
	png_write (strcat (tmp,"1.image"),LightMap, lmwid,lmwid,false,8);
}

static void GenerateTexMap ()//DEPRECATED
{
	float SinPhi;
	float CosPhi;
	float Theta;
	Vector Normal;
	//RGBColor Col;
	static CubeCoord TexCoord [256][256];
	static CubeCoord gluSphereCoord[256][256];
	unsigned char LightMap [65536*3];
	unsigned char LightMap1 [65536*3];
	unsigned char * Disc = new unsigned char [65536*3];
	int t;
	for (t=0; t<256; t++) //keep in mind that t = 128 (sin phi) +128
	{
		SinPhi = ((float)t)*ooWIDTHo2 -1;
		CosPhi = sqrt (1-SinPhi*SinPhi);//yes I know (-) ... but -PI/2<Phi<PI/2 so cos >0 like I said
		for (int s = 0; s < 256; s++) // is is none other than Theta * 128/PI
		{
			Theta = s*PIoWIDTHo2;// 128oPI = 128/3.1415926535
			//now that we have all this wonderful stuff, we must calculate lighting on this one point.
			// first calc the normal
			Normal.i = CosPhi * cos (Theta);
			Normal.k = CosPhi * sin (Theta);
			Normal.j = SinPhi;

			TexMap (TexCoord[t][s], Normal);//find what the lighting is
			gluSphereMap (gluSphereCoord[t][s],Normal,s);
		}
	}
	Spherize (TexCoord,gluSphereCoord,LightMap);	
	for (t=0; t<lmwid; t++) //keep in mind that t = 128 (sin phi) +128
	{
		SinPhi = ((float)t)*ooWIDTHo2 -1;
		CosPhi = sqrt (1-SinPhi*SinPhi);//yes I know (-) ... but -PI/2<Phi<PI/2 so cos >0 like I said
		for (int s = 0; s < lmwid; s++) // is is none other than Theta * 128/PI
		{
			Theta = s*PIoWIDTHo2;// 128oPI = 128/3.1415926535
			//now that we have all this wonderful stuff, we must calculate lighting on this one point.
			// first calc the normal
			Normal.i = CosPhi * cos (Theta);
			Normal.j = CosPhi * sin (Theta);
			Normal.k = SinPhi;

			TexMap (TexCoord[t][s], Normal);//find what the lighting is
			gluSphereMap (gluSphereCoord[t][s],Normal,s);
		}
	}
	Spherize (TexCoord,gluSphereCoord,LightMap1);
	for (t=0; t<lmwido2; t++)
	{
		for (int s=0; s<256; s++)
		{
			Disc [768*t+3*s] = (LightMap[768*2*t+3*s]+ LightMap[768*(2*t+1)+3*s])/2; //int divis
			Disc [768*t+3*s+1] = (LightMap[768*2*t+3*s+1]+ LightMap[768*(2*t+1)+3*s+1])/2; //int divis
			Disc [768*t+3*s+2] = (LightMap[768*2*t+3*s+2]+ LightMap[768*(2*t+1)+3*s+2])/2; //int divis
	
		}
	}
	for (t=0; t<lmwido2; t++)
	{
		for (int s=0; s<256; s++)
		{
			Disc [98304+768*t+3*s] = (LightMap1[768*(2*t+1)+3*s]+ LightMap1[768*((2*t)+1)+3*s])/2; //int divis
			Disc [98304+768*t+3*s+1] = (LightMap1[768*(2*t+1)+3*s+1]+ LightMap1[768*(2*t+1)+3*s+1])/2; //int divis
			Disc [98304+768*t+3*s+2] = (LightMap1[768*(2*t+1)+3*s+2]+ LightMap1[768*(2*t+1)+3*s+2])/2; //int divis
	
		}
	}
	char tmp [256]="";
	strcpy (tmp,OutputName);
	VSFile f;
	VSError err = f.OpenCreateWrite( strcat (tmp,".image"), TextureFile);
	f.Write (Disc,256*256*3);
	f.Close();
	delete [] Disc;	
}
*/
#endif
