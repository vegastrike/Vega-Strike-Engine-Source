#include "endianness.h"
#include <float.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "vs_path.h"
#include "png_texture.h"
#ifndef WIN32
typedef unsigned int DWORD;
typedef int LONG;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef struct {
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

typedef struct {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef struct {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

#else
#include <windows.h>
#include <wingdi.h>
#endif
  const int SIZEOF_BITMAPFILEHEADER=sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)+sizeof(DWORD);
  const int SIZEOF_BITMAPINFOHEADER= sizeof(DWORD)+sizeof(LONG)+sizeof(LONG)+2*sizeof(WORD)+2*sizeof(DWORD)+2*sizeof(LONG)+2*sizeof(DWORD);

#define NumLights 1
static char *InputName=NULL;
static char *OutputName=NULL;
static bool pushdown=false;
static float affine=0;
static float multiplicitive=1;
static float power=1;

struct Vector {
	float i;
	float j;
	float k;
};
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
static float oo128 = 1./128.;
static float PIo128 = 3.1415926535/128.;
static int lmwid =256;
static int lmhei =256;
static char bytepp=4;
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
	unsigned char *LightMap= new unsigned char [lmwid*lmhei*4];
	for (int t=0; t<lmhei; t++) //keep in mind that t = 128 (sin phi) +128
	{
		SinPhi = ((float)t)*oo128 -1;
		CosPhi = sqrt (1-SinPhi*SinPhi);//yes I know (-) ... but -PI/2<Phi<PI/2 so cos >0 like I said
		for (int s = 0; s < lmwid; s++) // is is none other than Theta * 128/PI
		{
			Theta = s*PIo128;// 128oPI = 128/3.1415926535
			//now that we have all this wonderful stuff, we must calculate lighting on this one point.
			// first calc the normal
			Normal.i = CosPhi * cos (Theta);
			Normal.j = CosPhi * sin (Theta);
			Normal.k = SinPhi;
			Lighting (Col, Normal);//find what the lighting is
			LightMap[lmwid*bytepp*t+bytepp*s] = (unsigned char) 255*Col.r;
			LightMap[lmwid*bytepp*t+bytepp*s+1] = (unsigned char) 255*Col.g;
			LightMap[lmwid*bytepp*t+bytepp*s+2] = (unsigned char) 255*Col.b;
			LightMap[lmwid*bytepp*t+bytepp*s+3] = 255;
		}
	}




	char tmp [256];
	assert (0);
	strcpy (tmp,OutputName);
	FILE *fp = fopen (strcat (tmp,"1.bmp"), "wb");
	png_write (strcat (tmp,"1.bmp"),LightMap, lmwid,lmhei,true,bytepp*8);
	fclose (fp);
}
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
    Tex.t = vert*128+128;
    Tex.s = Theta;//((int)((int)Theta+128)%256);
  } else {
    Tex.t = ((int)(vert*128))%255;
    Tex.s = Theta;
  }

  /*  float horiz;
  if (Normal.i>0)
    horiz = 1- (Normal.k+1)*.5;
  else {
    horiz = (Normal.k+1)*.5-1;
  }
  Tex.s=horiz*128+128;
  */
}
static void TexMap (CubeCoord & Tex, Vector Normal)
{
	float r[6];
	Normal.i = Normal.i;
	Normal.j = -Normal.j;
	Normal.k = -Normal.k;
	const float CubeSize = 128; //half of the length of any of the cube's sides
		r[0] = CubeSize / Normal.k; // find what you need to multiply to get to the cube
		r[1] = -r[0];
		r[2] = CubeSize / Normal.i; // find what you need to multiply to get to the cube
		r[3] = -r[2];
		r[4] = CubeSize / Normal.j; // find what you need to multiply to get to the cube
		r[5] = -r[4];
		if (!Normal.k)
			r[0] = r[1] = CubeSize*1000;
		if (!Normal.i)
			r[2] = r[3] = CubeSize*1000;
		if (!Normal.j)
			r[4] = r[5] = CubeSize*1000;

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
		Tex.s = rf*Normal.i+128; // btw 0 and 256
		Tex.t = 128- rf*Normal.j;// top left is 0,0
		break;
	case 1:
		Tex.s = 128-rf*Normal.i; // btw 0 and 256
		Tex.t = 128- rf*Normal.j;// top left is 0,0
		break;
	case 2:
		Tex.s = 128 - rf*Normal.k;
		Tex.t = 128 - rf*Normal.j;
		break;
	case 3:
	  Tex.s = 128 + rf*Normal.k;
	    Tex.t = 128 - rf*Normal.j;
		break;
	case 4:
		Tex.t = 128 -rf*Normal.i;
		Tex.s = 128 + rf*Normal.k;
		break;
	case 5:
	  Tex.t = 128 + rf*Normal.i;
	    Tex.s = 128 - rf*Normal.k;
		break;


	}
}
const int ltwid = 256;
const int lthei = 256;
static bool LoadTex(char * FileName, unsigned char scdata [lthei][ltwid][3]){

  unsigned char ctemp;
  FILE *fp = NULL;
  fp = fopen (FileName, "rb");
  long sizeX;
  long sizeY;
	if (!fp)
	{
		return false;
	}
	int bpp=8;
	int format=0;
	unsigned char * palette;
	unsigned char * data = readImage (fp,bpp,format,*(unsigned long*)&sizeX,*(unsigned long*)&sizeY,palette,texTransform,true);
	bpp/=8;
	if (format&PNG_HAS_ALPHA) {
	  bpp*=4;
	}else {
	  bpp*=3;
	}
	if (data) {
	  int ii;
	  int jj;
	  for (int i=0;i<lthei;i++) {
	    ii=(i*sizeY)/lthei;
	    for (int j=0;j<ltwid;j++) {
	      jj= (j*sizeX)/ltwid;
	      scdata[i][j][0]=data[(ii*sizeX+jj)*bpp];
	      scdata[i][j][1]=data[(ii*sizeX+jj)*bpp+1];
	      scdata[i][j][2]=data[(ii*sizeX+jj)*bpp+2];
	    }
	  }
	}else {
	  fseek (fp,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
	  //long temp;
	  BITMAPINFOHEADER info;
	  fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	  sizeX = le32_to_cpu(info.biWidth);
	  sizeY = le32_to_cpu(info.biHeight);

	  if(le16_to_cpu(info.biBitCount) == 24)
	    {
	      data = NULL;
	      data= (unsigned char *)malloc(3*sizeY*sizeX);
	      if (!data)
		return false;
	      for (int i=sizeY-1; i>=0;i--)
		{
		  int itimes3width= 3*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		  for (int j=0; j<sizeX;j++)
		    {
				//for (int k=2; k>=0;k--)
				//{
		      fread (data+3*j+itimes3width,sizeof (unsigned char)*3,1,fp);
		      unsigned char tmp = data[3*j+itimes3width];
		      data[3*j+itimes3width]= data[3*j+itimes3width+2];
		      data[3*j+itimes3width+2]=tmp;
				//}
		      
		    }
		}
	    }
	  else if(le16_to_cpu(info.biBitCount) == 8)
	    {
	      data = NULL;
	      data= (unsigned char *)malloc(sizeY*sizeX*3);
	      unsigned char palette[256*3+1];
	      unsigned char * paltemp = palette;
	      
		for(int palcount = 0; palcount < 256; palcount++)
		  {
		    fread(paltemp, sizeof(RGBQUAD), 1, fp);
		    //			ctemp = paltemp[0];//don't reverse
		    //			paltemp[0] = paltemp[2];
		    //			paltemp[2] = ctemp;
		    paltemp+=3;
		  }
		if (!data)
		  return false;
		//int k=0;
		for (int i=sizeY-1; i>=0;i--)
		  {
			for (int j=0; j<sizeX;j++)
			  {
			    fread (&ctemp,sizeof (unsigned char),1,fp);
			    data [3*(i*sizeX+j)+2] = palette[((short)ctemp)*3];	
			    data [3*(i*sizeX+j)+1] = palette[((short)ctemp)*3+1];
			    data [3*(i*sizeX+j)] = palette[((short)ctemp)*3+2];
			  }
		  }
	    }
	  float scaledconstX = sizeX/256;
	  float scaledconstY = sizeY/256;
	  for (int t=0; t<256; t++)
	    {
	      for (int s=0; s<256;s++)
		{
		  int index = (int) (scaledconstX*3*s)+(scaledconstY*3*t*sizeX);
		  
		  
		  scdata[t][s][0] = data[index];
		  scdata[t][s][1] = data[index+1];
		  scdata[t][s][2] = data[index+2];
		  
		  
		}
	    }
	}
	
	free  (data);
 	fclose (fp);
	return true;
}
struct Texmp
{
	unsigned char D [256][256][3];
};
static void Spherize (CubeCoord Tex [256][256],CubeCoord gluSph [256][256],unsigned char Col[])
{
	Texmp * Data = NULL;
	bool sphere = false;
	Data = new Texmp[6];
	if (!Data) 
	  return;//borken down and down Data[5], right Data[3]
	char *tmp= (char *)malloc (strlen (InputName)+60);;
	if (!(LoadTex (strcat (strcpy(tmp,InputName),"_front.bmp"),Data[0].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_back.bmp"),Data[1].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_left.bmp"),Data[2].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_right.bmp"),Data[3].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_up.bmp"),Data[4].D)&&LoadTex (strcat (strcpy(tmp,InputName),"_down.bmp"),Data[5].D))) {
	  if (!LoadTex (strcat (strcpy(tmp,InputName),"_sphere.bmp"),Data[0].D )) {
	    LoadTex (strcat (strcpy(tmp,InputName),".bmp"),Data[0].D);
	  }
	  sphere=true;
	  Tex = gluSph;
	  
	}
	free (tmp);
	tmp=NULL;
	//int NumPix;
	float sleft,sright,tdown,tup;
	for (int t=0; t<256;t++)
	{
		for (int s=0; s<256; s++)
		{

			//find the region enclosed by the box;
/*			if (s==130&&t == 25)
				int h = 1;
			sright = sleft = Tex[t][s].s;
			tdown = tup = Tex[t][s].t;
			if (s<255)
			{
				if (Tex[t][s+1].TexMap == Tex[t][s].TexMap)
				{
					if (Tex[t][s+1].s>sright)
						sright = Tex[t][s+1].s;
					if (Tex[t][s+1].s < sleft)
						sleft = Tex[t][s+1].s;
					if (Tex[t][s+1].t>tdown)
						tdown = Tex[t][s+1].t;
					if (Tex[t][s+1].t < tup)
						tup = Tex[t][s+1].t;
				}
			}
			if (s>0)
			{
				if (Tex[t][s-1].TexMap == Tex[t][s].TexMap)
				{
					if (Tex[t][s-1].s>sright)
						sright = Tex[t][s-1].s;
					if (Tex[t][s-1].s < sleft)
						sleft = Tex[t][s-1].s;
					if (Tex[t][s-1].t>tdown)
						tdown = Tex[t][s-1].t;
					if (Tex[t][s-1].t < tup)
						tup = Tex[t][s-1].t;
				}
			}
			if (t<255)
			{
				if (Tex[t+1][s].TexMap == Tex[t][s].TexMap)
				{
					if (Tex[t+1][s].s>sright)
						sright = Tex[t+1][s].s;
					if (Tex[t+1][s].s < sleft)
						sleft = Tex[t+1][s].s;
					if (Tex[t+1][s].t>tdown)
						tdown = Tex[t+1][s].t;
					if (Tex[t+1][s].t < tup)
						tup = Tex[t+1][s].t;
				}
			}
			if (t>0)
			{
				if (Tex[t-1][s].TexMap == Tex[t][s].TexMap)
				{
					if (Tex[t-1][s].s>sright)
						sright = Tex[t-1][s].s;
					if (Tex[t-1][s].s < sleft)
						sleft = Tex[t-1][s].s;
					if (Tex[t-1][s].t>tdown)
						tdown = Tex[t-1][s].t;
					if (Tex[t-1][s].t < tup)
						tup = Tex[t-1][s].t;
				}
			}
			sright = .5 * (sright + Tex[t][s].s);
			sleft = .5 * (sleft + Tex[t][s].s);
			tdown = .5 * (tdown + Tex[t][s].t);
			tup = .5 * (tup + Tex[t][s].t);
			if (sright == Tex[t][s].s&&sright > 245)
			{
				sright = 255.99;
			}
			if (tdown == Tex[t][s].t&& tdown > 245)
			{
				tdown = 255.999;
			}
			if (sleft == Tex[t][s].s && sleft < 10)
			{
				sleft = 0.001;
			}
			if (tup == Tex[t][s].t&&sleft < 10)
			{
				tup = .001;
			}*/
		  //			float NumPixs = sright-sleft;
			//			float NumPixt = tdown - tup; //bitmpas are top/down
			float r =0;
			float g =0;
			float b = 0;
			if (/*NumPixs&&NumPixt*/0)
			{
				float oonps = 1/(sright-sleft);
				//float oonpt = 1/(tdown-tup);
				
				int stemp;
				for (stemp = (int) ceil (sleft);stemp <floor (sright); stemp ++)
				{
					for (int ttemp = (int)ceil (tup);ttemp <floor (tdown); ttemp ++)
					{
						r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0];
						g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1];
						b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2];
					}	
				}
				float tupavcoef = abso (ceil(tup)-tup);
				float tdownavcoef = abso (tdown - floor (tdown));
				float srightavcoef = abso (sright-floor(sright));
				float sleftavcoef = abso (ceil (sleft) - sleft);
				//do upper border
				int ttemp = (int)floor (tup);
				for (stemp = (int)ceil (sleft);stemp <floor (sright); stemp ++)
				{
						r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*tupavcoef;
						g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*tupavcoef;
						b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*tupavcoef;
				}
				ttemp = floor (tdown);
				for (stemp = ceil (sleft);stemp <floor (sright); stemp ++)
				{
						r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*tdownavcoef;
						g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*tdownavcoef;
						b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*tdownavcoef;
				}
				stemp = floor (sright);
				for (ttemp = ceil (tup);ttemp <floor (tdown); ttemp ++)
				{
						r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*srightavcoef;
						g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*srightavcoef;
						b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*srightavcoef;
				}
				stemp = floor (sleft);
				for (ttemp = ceil (tup);ttemp <floor (tdown); ttemp ++)
				{
						r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*sleftavcoef;
						g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*sleftavcoef;
						b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*sleftavcoef;
				}
				//now for the four corners
				// up left
				stemp = floor (sleft);
				ttemp = floor (tup);
				r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*sleftavcoef*tupavcoef;
				g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*sleftavcoef*tupavcoef;
				b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*sleftavcoef*tupavcoef;
				//up right
				stemp = floor (sright);
				ttemp = floor (tup);
				r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*srightavcoef*tupavcoef;
				g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*srightavcoef*tupavcoef;
				b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*srightavcoef*tupavcoef;
				//down left
				stemp = floor (sleft);
				ttemp = floor (tdown);
				r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*sleftavcoef*tdownavcoef;
				g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*sleftavcoef*tdownavcoef;
				b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*sleftavcoef*tdownavcoef;
				//down right
				stemp = floor (sright);
				ttemp = floor (tdown);
				r += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][0]*srightavcoef*tdownavcoef;
				g += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][1]*srightavcoef*tdownavcoef;
				b += oonps * Data[Tex[t][s].TexMap].D[ttemp][stemp][2]*srightavcoef*tdownavcoef;
			}
			else
			{
				float avg = 1;
				if ((int)floor (Tex[t][s].s)  >255)
					Tex[t][s].s = 255;
				if ((int)floor (Tex[t][s].t)  >255)
					Tex[t][s].t = 255;
				if ((int)floor (Tex[t][s].t)  <0)
					Tex[t][s].t = 0;
				if ((int)floor (Tex[t][s].s)  <0)
					Tex[t][s].s = 0;
				r = Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s)][0];
				g = Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s)][1];
				b = Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s)][2];
				if ((int)floor (Tex[t][s].s)  <255)
				{
					avg ++;
					r += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s+1)][0];
					g += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s+1)][1];
					b += Data[Tex[t][s].TexMap].D[(int)floor (Tex[t][s].t)][(int)floor (Tex[t][s].s+1)][2];
				}
				if ((int)floor (Tex[t][s].t) <255)
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
			/** pre031001 <SPHERE_MAP extention>			
			Col[3*256*(255-t)+3*(255-s)] = r;
			Col[3*256*(255-t)+3*(255-s)+1] = g;
			Col[3*256*(255-t)+3*(255-s)+2] = b;
			*/
			unsigned int rr=r;
			unsigned int gg=g;
			unsigned int bb=b;
			if (affine!=0||multiplicitive!=1||power!=1) {
			  rr = affine + ((pow ((float)r,power)) * multiplicitive);
			  gg = affine + ((pow ((float)g,power)) * multiplicitive);
			  bb = affine + ((pow ((float)b,power)) * multiplicitive);
			}
			if (rr>255) rr= 255;
			if (gg>255) gg=255;
			if (bb>255) bb=255;

			Col[4*(256*(255-t)+(255-s))] = rr;
			Col[4*(256*(255-t)+(255-s))+1] = gg;
			Col[4*(256*(255-t)+(255-s))+2] = bb;
			Col[4*(256*(255-t)+(255-s))+3] = 255;
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
	static CubeCoord TexCoord [256][256];
	static CubeCoord gluSphereCoord [256][256];
	unsigned char *LightMap =(unsigned char *)malloc (lmwid*lmhei*4);
	int t;
	for (t=0; t<256; t++) //keep in mind that t = 128 (sin phi) +128
	{
		float to256 = t / 104. -1.23;
		for (int s = 0; s < 256; s++) // is is none other than Theta * 128/PI
		{
			float so256 = s / 104. -1.23;
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
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	bmfh.bfType=19778;
	bmfh.bfSize=3;
	bmfh.bfReserved1=0;
	bmfh.bfReserved2=54;
	bmfh.bfOffBits=0;
	info.biSize=40;
	info.biWidth=256;
	info.biHeight=256;
	info.biPlanes=1;
	info.biBitCount=24;
	info.biCompression=0;
	info.biSizeImage=196608;
	info.biXPelsPerMeter=2834;
	info.biYPelsPerMeter=2834;
	info.biClrUsed=0;
	info.biClrImportant=0;

	/** used to determine the consts
	FILE * fp = fopen ("blank.bmp", "rb");
	
	fread (&bmfh,SIZEOF_BITMAPFILEHEADER,1,fp);
	long temp;

	fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	fclose (fp);
	fprintf (stderr,"bfType %d bfSize %d bfReserved1 %d, bfReserved2 %d, bfOffBits %d", bmfh.bfType, bmfh.bfSize, bmfh.bfReserved1, bmfh.bfReserved2, bmfh.bfOffBits);
	fprintf (stderr,"        DWORD      biSize; %d
        LONG       biWidth; %d
        LONG       biHeight;%d
        WORD       biPlanes;%d
        WORD       biBitCount;%d
        DWORD      biCompression;%d
        DWORD      biSizeImage;%d
        LONG       biXPelsPerMeter;%d
        LONG       biYPelsPerMeter;%d
        DWORD      biClrUsed;%d
        DWORD      biClrImportant;%d",info.biSize,info.biWidth, info.biHeight,info.biPlanes, info.biBitCount, info.biCompression, info.biSizeImage, info.biXPelsPerMeter, info.biYPelsPerMeter, info.biClrUsed, info.biClrImportant);
        */
	/*

	strcpy (tmp,OutputName);
	*/
	//	fp = fopen (strcat (tmp,".png"), "wb");
	//	fwrite (&bmfh,SIZEOF_BITMAPFILEHEADER,1,fp);
	//	fwrite (&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	png_write (OutputName,LightMap,256,256,true,8);
	//fclose (fp);
		

}
static void GenerateTexMap ()
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
		SinPhi = ((float)t)*oo128 -1;
		CosPhi = sqrt (1-SinPhi*SinPhi);//yes I know (-) ... but -PI/2<Phi<PI/2 so cos >0 like I said
		for (int s = 0; s < 256; s++) // is is none other than Theta * 128/PI
		{
			Theta = s*PIo128;// 128oPI = 128/3.1415926535
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
	for (t=0; t<256; t++) //keep in mind that t = 128 (sin phi) +128
	{
		SinPhi = ((float)t)*oo128 -1;
		CosPhi = sqrt (1-SinPhi*SinPhi);//yes I know (-) ... but -PI/2<Phi<PI/2 so cos >0 like I said
		for (int s = 0; s < 256; s++) // is is none other than Theta * 128/PI
		{
			Theta = s*PIo128;// 128oPI = 128/3.1415926535
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
	for (t=0; t<128; t++)
	{
		for (int s=0; s<256; s++)
		{
			Disc [768*t+3*s] = (LightMap[768*2*t+3*s]+ LightMap[768*(2*t+1)+3*s])/2; //int divis
			Disc [768*t+3*s+1] = (LightMap[768*2*t+3*s+1]+ LightMap[768*(2*t+1)+3*s+1])/2; //int divis
			Disc [768*t+3*s+2] = (LightMap[768*2*t+3*s+2]+ LightMap[768*(2*t+1)+3*s+2])/2; //int divis
	
		}
	}
	for (t=0; t<128; t++)
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
	FILE * fp = fopen (strcat (tmp,".bmp"), "wb");
	fwrite (Disc,256*256*3,1,fp);
	fclose (fp);
	delete [] Disc;	
}

void EnvironmentMapGeneratorMain(const char * inpt, const char *outpt, float a, float m, float p, bool w)
{

    affine=a;
    multiplicitive=m;
    power=p;
    pushdown =w;
    char * tmp = (char *) malloc (sizeof(char)*strlen(inpt)+40);
    strcpy (tmp,inpt);
    FILE * fp = fopen (strcat (tmp,"_sphere.bmp"),"rb");
    if (!fp)
      fp = fopen (strcat (tmp,"_up.bmp"),"rb");
    //bool share = false;
    std::string s;
    if (!fp) {
      s = GetSharedTexturePath (std::string (inpt));
      InputName = (char *) malloc (sizeof (char)*(s.length()+2));
      strcpy (InputName,s.c_str());
    } else {
      fclose (fp);
      InputName = (char *) malloc (sizeof (char)*(strlen(inpt)+2));
      strcpy (InputName,inpt);
    }
    OutputName=strdup (outpt);
    free(tmp);
    tmp=NULL;
  fprintf (stderr, "input name %s, output name %s\nAffine %f Mult %f Pow %f\n",InputName, OutputName, affine, multiplicitive, power);
  GenerateSphereMap();
  free (InputName);
  free (OutputName);
}

