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

#include <stdio.h>
#include "gfx_aux_texture.h"
#include <assert.h>
#include "gfxlib.h"
#include <string>

#include "hashtable.h"
using std::string;
#ifdef macintosh 
#include <swapbytes.h>
#define SWAPLONG(x) (bswap_32(x))
#define SWAPSHORT(x) (bswap_16(x))
#else
#define SWAPLONG(x) (x)
#define SWAPSHORT(x) (x)
#endif

#ifndef WIN32
typedef unsigned long DWORD;
typedef long  LONG;
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
const int SIZEOF_RGBQUAD=sizeof(BYTE)*4;

static Hashtable<string, Texture,char [1001]> texHashTable;

Texture * Texture::Exists (string s) {
    return texHashTable.Get(s);
}

GFXBOOL Texture::checkold(const string &s)
{
	Texture *oldtex;
	//FIX'D
	if(oldtex = texHashTable.Get(s))
	{
	  //*this = *oldtex;//will be obsoleted--unpredictable results with string()
	  memcpy (this, oldtex, sizeof (Texture));
	  original = oldtex;
	  original->refcount++;
	  return GFXTRUE;
	}
	else
	{
	  oldtex = (Texture*)malloc(sizeof(Texture));
	  texHashTable.Put(s, oldtex);
	  original = oldtex;
	  return GFXFALSE;
	}
}
void Texture::setold()
{
  //	*original = *this;//will be obsoleted in new C++ standard unpredictable results when using string() (and its strangeass copy constructor)
  memcpy (original, this, sizeof (Texture));
	original->original = NULL;
	original->refcount++;
}

Texture::Texture(const char * FileName, int stage, enum FILTER mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget)
{
  ismipmapped  = mipmap;
  InitTexture();
  palette = NULL;
  texture_target =target;
  image_target=imagetarget;
	refcount = 0;
	this->stage = stage;
	FILE *fp = NULL;
	fp = fopen (FileName, "r+b");
	if (!fp)
	{
	  fprintf (stderr, "%s, not found",FileName);
	  
		data = NULL;
		return;
	}
	//	strcpy(filename, FileName);
	fseek (fp,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
	//long temp;
	BITMAPINFOHEADER info;
	fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	sizeX = SWAPLONG(info.biWidth);	
	sizeY = SWAPLONG(info.biHeight);


	//while(1);
	char t[64];
	strcpy(t, FileName);
	t[strlen(FileName)-3] = 'a';
	t[strlen(FileName)-2] = 'l';
	t[strlen(FileName)-1] = 'p';
	FILE *fp2 = fopen(t, "r+b");

	string texfilename = string(FileName);
	
	if(fp2) {
	  //texfilename += string(t);
	}
	//	this->texfilename = texfilename;
	//strcpy (filename,texfilename.c_str());
	if(checkold(texfilename))
		return;
	this->texfilename = new char [texfilename.length()+1];
	strcpy(this->texfilename,texfilename.c_str());

	if(SWAPSHORT(info.biBitCount) == 24)
	{
		mode = _24BIT;
		if(fp2)
			mode = _24BITRGBA;
		data = NULL;
		data= new unsigned char [4*sizeY*sizeX]; // all bitmap data needs to be 32 bits
		if (!data)
			return;
		for (int i=sizeY-1; i>=0;i--)
		{
			int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
			for (unsigned int j=0; j<sizeX;j++)
			{
				if(fp2)
					fread(data+3, sizeof(unsigned char), 1, fp2);
				else
					*(data+3) = 0xff; // default alpha = 1
				for (int k=2; k>=0;k--)
				{
					fread (data+k+4*j+itimes4width,sizeof (unsigned char),1,fp);
				}
				
			}
		}
	}
	else if(SWAPSHORT(info.biBitCount) == 8)
	{
		mode = _8BIT;
		data = NULL;
		data= new unsigned char [sizeY*sizeX];
		palette = new unsigned char [256*4+1];
		unsigned char *paltemp = palette;
		unsigned char ctemp;
		for(int palcount = 0; palcount < 256; palcount++)
		{
		  fread(paltemp, SIZEOF_RGBQUAD, 1, fp);
			ctemp = paltemp[0];
			paltemp[0] = paltemp[2];
			paltemp[2] = ctemp;
			paltemp+=4; // pal size
		}
		if (!data)
			return;
		for (int i=sizeY-1; i>=0;i--)
		{
			for (unsigned int j=0; j<sizeX;j++)
			{
				fread (data+ j + i * sizeX,sizeof (unsigned char),1,fp);
			}
		}
	}
	Bind();
 	fclose (fp);
	setold();
}

Texture::Texture (const char * FileNameRGB, const char *FileNameA, int stage, enum FILTER  mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, float alpha, int zeroval)
{
  ismipmapped  = mipmap;
  InitTexture();
  palette = NULL;

	refcount = 0;
	this->stage = stage;
	texture_target=target;
	image_target=imagetarget;
	FILE *fp = NULL;
	fp = fopen (FileNameRGB, "r+b");
	if (!fp)
	{
		data = NULL;
		return;
	}
	fseek (fp,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
	//long temp;
	BITMAPINFOHEADER info;
	fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	sizeX = SWAPLONG(info.biWidth);
	sizeY = SWAPLONG(info.biHeight);
	BITMAPINFOHEADER info1;
	FILE *fp1 = NULL;

	string texfilename = string(FileNameRGB) + string(FileNameA);
	//this->texfilename = texfilename;
	//strcpy (filename,texfilename.c_str());

	if(checkold(texfilename))
		return;
	this->texfilename = new char [texfilename.length()+1];
	strcpy(this->texfilename,texfilename.c_str());

	if (FileNameA)
	{
		fp1 = fopen (FileNameA, "r+b");
		if (!fp1)
		{
			data = NULL;
			FileNameA = NULL;
			//fclose(fp);
			//*this = Texture(FileNameRGB, NULL);
			//return;
		}
		else
		{
			fseek (fp1,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
			
			fread (&info1,SIZEOF_BITMAPINFOHEADER,1,fp1);
			if (sizeX != (unsigned int) SWAPLONG(info1.biWidth)||sizeY!=(unsigned int)SWAPLONG(info1.biHeight))
			{
				data = NULL;
				return;
			}
			RGBQUAD ptemp1;	
			if (SWAPSHORT(info1.biBitCount) == 8)
			{
				for (int i=0; i<256; i++)
					fread(&ptemp1, sizeof(RGBQUAD), 1, fp1); //get rid of the palette for a b&w greyscale alphamap

			}
		}
	}
	if(SWAPSHORT(info.biBitCount) == 24) {
	  mode = _24BITRGBA;
	  data = NULL;
	  data= new unsigned char [4*sizeY*sizeX];
	  if (!data)
	    return;
	  for (int i=sizeY-1; i>=0;i--)
	    {
	      int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
	      for (unsigned int j=0; j<sizeX;j++)
		{
		  for (int k=2; k>=0;k--) {
		    fread (data+k+4*j+itimes4width,sizeof (unsigned char),1,fp);
		  }
		  if (FileNameA){
		    if (SWAPSHORT(info1.biBitCount)==24)
		      for (int k=2; k>=0;k--) {
			fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
			//*(data+3+4*j+itimes4width) = 30;
		      } else {
			fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
		      }
		  }
		  else {
		    if (!data[4*j+itimes4width]&&!data[4*j+itimes4width+1]&&!data[4*j+itimes4width+2])
		      data[4*j+itimes4width+3] = 0;
		    else
		      data[4*j+itimes4width+3] = 255;
		  }
		  //*(data+3+4*j+itimes4width) = 30;
				
				
		}
	    }
	}
	else if(SWAPSHORT(info.biBitCount) == 8) {
	  unsigned char index = 0;
	  mode = _24BITRGBA;
	  data = NULL;
	  data= new unsigned char [4*sizeY*sizeX];
	  palette = new unsigned char [256*4+1];
	  unsigned char *paltemp = palette;
	  unsigned char ctemp;
	  for(int palcount = 0; palcount < 256; palcount++) {
	    fread(paltemp, sizeof(RGBQUAD), 1, fp);
	    ctemp = paltemp[0];
	    paltemp[0] = paltemp[2];
	    paltemp[2] = ctemp;
	    paltemp+=4;
	  }
	  if (!data)
	    return;
		//FIXME VEGASTRIKE???		int k=0;
	  for (int i=sizeY-1; i>=0;i--) {
	    for (unsigned int j=0; j<sizeX;j++)
	      {
		fread (&index,sizeof (unsigned char),1,fp);
		data [4*(i*sizeX+j)] = palette[((short)index)*4];	
		data [4*(i*sizeX+j)+1] = palette[((short)index)*4+1];
		data [4*(i*sizeX+j)+2] = palette[((short)index)*4+2];
	      }
	  }
	  delete [] palette;
	  palette = NULL;
	  if (FileNameA)
	    {
	      for (int i=sizeY-1; i>=0;i--)
		{
		  int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		  for (unsigned int j=0; j<sizeX;j++) {
		    if (SWAPSHORT(info1.biBitCount)==24)
		      for (int k=2; k>=0;k--) {
			fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
		      }
		    else {
		      fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
		    }
		  }
		}
	    } else{
	      for (unsigned int i=0; i<sizeY;i++) {
		int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		for (unsigned int j=0; j<sizeX;j++){
		  if (!data[4*j+itimes4width]&&!data[4*j+itimes4width+1]&&!data[4*j+itimes4width+2])
		    data[4*j+itimes4width+3]=0;
		  else
		    data[4*j+itimes4width+3]=255;
		}
	      }
	      
	    }
	}

	if (alpha!=1) {
	  float tmpclamp;
	  for (unsigned int i=0; i<sizeY;i++) {
	    int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
	    for (unsigned int j=0; j<sizeX;j++){
	      tmpclamp = data[4*j+itimes4width+3];
	      if (tmpclamp>zeroval) {
		tmpclamp /=255.;
		tmpclamp =pow (tmpclamp,alpha);
		tmpclamp *=255;
		if (tmpclamp>255)
		  tmpclamp = 255;
		data[4*j+itimes4width+3]= tmpclamp;
			
	      }
	    }
	  }
	  
	}
	Bind();
	fclose(fp);
	
	setold();

}
Texture::~Texture()
	{
		if(original == NULL)
		{
			if(data != NULL)
			{
				delete [] data;

				data = NULL;
				texHashTable.Delete (string(texfilename));
				GFXDeleteTexture(name);
				delete []texfilename;				
				//glDeleteTextures(1, &name);
			}
			if (palette !=NULL) {
			  delete []palette;
			  palette = NULL;
			}
		}
		else
		{
			original->refcount--;
			if(original->refcount == 0)
				delete original;
		}
	}


void Texture::Transfer ()
{
	//Implement this in D3D
	//if(mode == _8BIT)
	//	glColorTable(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, palette);

	switch (mode)
	{
	case _24BITRGBA:
		GFXTransferTexture(data, name,image_target);
		break;
	case _24BIT:
		GFXTransferTexture(data, name,image_target);
		break;
	case _8BIT:
		GFXTransferTexture(data, name,image_target);
		//TODO: Do something about this, and put in some code to check that we can actually do 8 bit textures

		break;
	}
	
}
int Texture::Bind()
{
	switch(mode)
	{
	case _24BITRGBA:
		//GFXCreateTexture(sizeX, sizeY, RGBA32, &name, NULL, stage);
		GFXCreateTexture(sizeX, sizeY, RGBA32, &name, NULL, stage,ismipmapped, texture_target);
		break;
	case _24BIT:
		//not supported by most cards, so i use rgba32
		//GFXCreateTexture(sizeX, sizeY, RGB24, &name);
		GFXCreateTexture(sizeX, sizeY, RGB32, &name, NULL, stage,ismipmapped,texture_target);
		break;
	case _8BIT:
		GFXCreateTexture(sizeX, sizeY, PALETTE8, &name, (char *)palette, stage,ismipmapped,texture_target);
		break;
	}
	Transfer();

	return name;

}
void Texture::Prioritize (float priority) {
  GFXPrioritizeTexture (name, priority);
}

void Texture::MakeActive()
{
  assert(name!=-1);
  GFXSelectTexture(name,stage);
}
