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
#include "aux_texture.h"
#include <assert.h>
#include "gfxlib.h"
#include <string>
#include "endianness.h"
#include "hashtable.h"
#include "vs_path.h"
#include "png_texture.h"
using std::string;


#ifndef WIN32
typedef unsigned long DWORD;
typedef long  LONG;
typedef unsigned short WORD;
typedef unsigned char BYTE;
/**
 * Windows Bitmap format.  Caution about mips systems which cannot handle
 * misaligned structs
 * Caution about big endian systems (use endianness.h to read in things)
 */
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
/**
 * File header of a bitmap. Won't work on mips architecture with 
 * misaligned structs
 */
typedef struct {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
/**
 *  The color data of a bitmap
 */
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
///Defined for gcc which pads the size of structs
const int SIZEOF_BITMAPFILEHEADER=sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)+sizeof(DWORD);
///Defined for gcc which pads the size of structs
const int SIZEOF_BITMAPINFOHEADER= sizeof(DWORD)+sizeof(LONG)+sizeof(LONG)+2*sizeof(WORD)+2*sizeof(DWORD)+2*sizeof(LONG)+2*sizeof(DWORD);
///defined for gcc which pads size of structs (not entirely necessary)
const int SIZEOF_RGBQUAD=sizeof(BYTE)*4;
///holds all the textures in a huge hash table
Hashtable<string, Texture,char [127]> texHashTable;
///returns if a texture exists
Texture * Texture::Exists (string s, string a) {
  return Texture::Exists (s+a);
}
Texture * Texture::Exists (string s) {
  Texture * tmp = texHashTable.Get(GetHashName(s));
  if (tmp==NULL){
    string tmpo;
    tmp = texHashTable.Get (GetSharedTextureHashName (s));
  }
  if (tmp)
    return tmp->Original();
  return tmp;
}

bool Texture::operator < (Texture & b) {
  return Original()<b.Original();
}

bool Texture::operator == (Texture & b) {
  return Original()==b.Original();
}

GFXBOOL Texture::checkold(string s, bool shared, string & hashname)
{
  hashname = shared?GetSharedTextureHashName(s):GetHashName(s);
  Texture *oldtex= texHashTable.Get (hashname);
  if(oldtex!=NULL) {
	  //*this = *oldtex;//will be obsoleted--unpredictable results with string()
    memcpy (this, oldtex, sizeof (Texture));
    original = oldtex;
    original->refcount++;
    return GFXTRUE;
  } else {
    return GFXFALSE;
  }
}
void Texture::modold (string s, bool shared, string & hashname) {
  hashname = shared?GetSharedTextureHashName(s):GetHashName(s);
  Texture * oldtex = (Texture*)malloc(sizeof(Texture));
  oldtex->InitTexture();
  oldtex->name=-1;
  oldtex->refcount=1;
  oldtex->original=NULL;
  oldtex->palette=NULL;
  oldtex->data=NULL;
  texHashTable.Put(hashname, oldtex);
  original = oldtex;
}
Texture::Texture () {
  data=NULL;
  InitTexture();
  name=-1;
  palette=NULL;
}

void Texture::setold()
{
  //	*original = *this;//will be obsoleted in new C++ standard unpredictable results when using string() (and its strangeass copy constructor)
  memcpy (original, this, sizeof (Texture));
	original->original = NULL;
	original->refcount++;
}
Texture * Texture::Original() {
  if (original) {
    return original->Original();
  }else {
    return this;
  }
}

Texture *Texture::Clone () {
  Texture * retval = new Texture();
  Texture * target = Original();
  *retval = *target;
  //  memcpy (this, target, sizeof (Texture));
  if (retval->name!=-1) {
    retval->original = target;
    retval->original->refcount++;
  } else {
    retval->original = NULL;
  }
  retval->refcount = 0;
  return retval;
  //assert (!original->original);
  
}
Texture::Texture(const char * FileName, int stage, enum FILTER mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget)
{

  data = NULL;
  ismipmapped  = mipmap;
  InitTexture();
  palette = NULL;
  texture_target =target;
  image_target=imagetarget;
  this->stage = stage;
  string texfilename = string(FileName);
  string tempstr;
  if(checkold(texfilename,false,tempstr)) {
    return;
  } else {
    texfilename = string(FileName);
    if (checkold(texfilename,true,tempstr)) {
      return;
    }
  }
  fprintf (stderr,"Loading bmp file %s ",FileName);
	char t[64];
	strcpy(t, FileName);
	t[strlen(FileName)-3] = 'a';
	t[strlen(FileName)-2] = 'l';
	t[strlen(FileName)-1] = 'p';
	FILE *fp2 = fopen(t, "rb");


	
	if(fp2) {
	  //texfilename += string(t);
	}
	//	this->texfilename = texfilename;
	//strcpy (filename,texfilename.c_str());
	FILE *fp = NULL;
	fp = fopen (FileName, "rb");
	bool shared = (fp==NULL);
	modold (texfilename,shared,texfilename);
	if (shared) {
	  fp = fopen (GetSharedTexturePath (FileName).c_str(),"rb");
	}
	if (!fp)
	{
      	  fprintf (stderr, "%s, not found",FileName);

	  texHashTable.Delete (texfilename);
	  name=-1;
	  data = NULL;
	  free(original);
	  original=NULL;
	  if (fp2) {
	    fclose (fp2);
	  }
	  return;
	}
	//	strcpy(filename, FileName);
	int bpp;
	int format;
	this->texfilename = new char [texfilename.length()+1];
	strcpy(this->texfilename,texfilename.c_str());

	data = readImage (fp,bpp,format,sizeX,sizeY,palette,NULL/*texTransform*/,true);
	if (data) {
	  //FIXME deal with palettes and grayscale with alpha
	  if (!(format&PNG_HAS_COLOR)||(format&PNG_HAS_PALETTE)) {
	    mode=_8BIT;
	    if (!(format&PNG_HAS_COLOR)){
	      palette = new unsigned char [256*4+1];
	      for (unsigned int i =0;i<256;i++) {
		palette[i*4]=i;
		palette[i*4+1]=i;
		palette[i*4+2]=i;
		palette[i*4+3]=255;
	      }
	    }
	  } 
	  if (format&PNG_HAS_COLOR) {
	    if (format&PNG_HAS_ALPHA) {
	      mode=_24BITRGBA;
	    } else {
	      mode=_24BIT;
	    }
	  }
	}else {
	  //seek back to beginning
	  fseek (fp,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
	  //long temp;
	  BITMAPINFOHEADER info;
	  fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	  sizeX = le32_to_cpu(info.biWidth);	
	  sizeY = le32_to_cpu(info.biHeight);
	  
	  
	  //while(1);
	  
	  if(le16_to_cpu(info.biBitCount) == 24)
	    {
	      mode = _24BITRGBA;
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
	  else if(le16_to_cpu(info.biBitCount) == 8)
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
	}
	fprintf (stderr,"Bind... ");
	Bind();
 	fclose (fp);
	if (fp2)
	  fclose (fp2);
	if (data)
	  delete [] data;
	data = NULL;
	setold();
	fprintf (stderr," Load Success\n");
}

Texture::Texture (const char * FileNameRGB, const char *FileNameA, int stage, enum FILTER  mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, float alpha, int zeroval)
{
  data = NULL;
  ismipmapped  = mipmap;
  InitTexture();
  palette = NULL;

	refcount = 0;
	this->stage = stage;
	texture_target=target;
	image_target=imagetarget;
	string texfilename = string(FileNameRGB) + string(FileNameA);
	string tempstr;
	if(checkold(texfilename,false,tempstr)) {
	  return;
	} else {
	  if (checkold(texfilename,true,tempstr)) {
	    return;
	  }
	}
	fprintf (stderr,"Loading bmp file %s alp %s ",FileNameRGB,FileNameA);
	//this->texfilename = texfilename;
	//strcpy (filename,texfilename.c_str());
	FILE *fp = NULL;
	fp = fopen (FileNameRGB, "rb");
	bool shared = (fp==NULL);
	modold (texfilename,shared,texfilename);
	if (shared) {
	  fp = fopen (GetSharedTexturePath (FileNameRGB).c_str(),"rb");
	}
	if (!fp)
	{
	  texHashTable.Delete (texfilename);
	  palette = NULL;
	  data = NULL;
	  name=-1;
	  original->name=-1;
	  free (original);
	  original=NULL;
	  return;
	}
	int bpp;
	int format;
	FILE * fp1=NULL;
	this->texfilename = new char [texfilename.length()+1];
	strcpy(this->texfilename,texfilename.c_str());
	data = readImage (fp,bpp,format,sizeX,sizeY,palette,NULL/*texTransform*/,true);
	if (data) {
	  //FIXME deal with palettes and grayscale with alpha
	  if (!(format&PNG_HAS_COLOR)||(format&PNG_HAS_PALETTE)) {
	    mode=_8BIT;
	    if (!(format&PNG_HAS_COLOR)){
	      palette = new unsigned char [256*4+1];
	      for (unsigned int i =0;i<256;i++) {
		palette[i*4]=i;
		palette[i*4+1]=i;
		palette[i*4+2]=i;
		palette[i*4+3]=255;
	      }
	    }

	  } 
	  if (format&PNG_HAS_COLOR) {
	    if (format&PNG_HAS_ALPHA) {
	      mode=_24BITRGBA;
	    } else {
	      mode=_24BIT;
	    }
	  }
	}else {
	  ///seek back to the beginning
	  fseek (fp,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
	  //long temp;
	  BITMAPINFOHEADER info;
	  fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	  sizeX = le32_to_cpu(info.biWidth);
	  sizeY = le32_to_cpu(info.biHeight);
	  BITMAPINFOHEADER info1;
	  


	  
	  if (FileNameA)
	    {
	      std::string tmp = shared?GetSharedTexturePath(FileNameA).c_str():FileNameA;
	      fp1 = fopen (tmp.c_str(), "rb");
	      
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
		  if (sizeX != (unsigned int) le32_to_cpu(info1.biWidth)||sizeY!=(unsigned int)le32_to_cpu(info1.biHeight))
		    {
		      data = NULL;
		      fclose (fp1);
		      if (fp) 
			fclose (fp);
		      return;
		    }
		  RGBQUAD ptemp1;	
		  if (le16_to_cpu(info1.biBitCount) == 8)
		    {
		      for (int i=0; i<256; i++)
			fread(&ptemp1, sizeof(RGBQUAD), 1, fp1); //get rid of the palette for a b&w greyscale alphamap
		      
		    }
		}
	    }
	  if(le16_to_cpu(info.biBitCount) == 24) {
	    mode = _24BITRGBA;
	    data= new unsigned char [4*sizeY*sizeX];
	    for (int i=sizeY-1; i>=0;i--)
	      {
		int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		for (unsigned int j=0; j<sizeX;j++)
		  {
		    for (int k=2; k>=0;k--) {
		      fread (data+k+4*j+itimes4width,sizeof (unsigned char),1,fp);
		    }
		    if (FileNameA){
		      if (le16_to_cpu(info1.biBitCount)==24)
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
	  else if(le16_to_cpu(info.biBitCount) == 8) {
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
	    if (!data) {
	      if (fp)
		fclose (fp);
	      if (fp1)
		fclose (fp1);
	      return;
	    }
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
		      if (le16_to_cpu(info1.biBitCount)==24)
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
		data[4*j+itimes4width+3]= (unsigned char)tmpclamp;
		
		}
	    }
	    }
	    
	  }
	}
	fprintf (stderr,"Bind... ");
	Bind();
	if (fp)
	  fclose(fp);
	if (fp1) {
	  fclose (fp1);
	}
	if (data)
	  delete [] data;
	data = NULL;
	setold();
	fprintf (stderr,"Load Success\n");
}
Texture::~Texture()
	{
		if(original == NULL)
		{
		  /**DEPRECATED
			if(data != NULL)
			{
				delete [] data;

				data = NULL;
			}
		  */
			if (name!=-1) {
			  texHashTable.Delete (string(texfilename));
			  GFXDeleteTexture(name);
			  delete []texfilename;				
			}

				//glDeleteTextures(1, &name);
			
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
		GFXTransferTexture(data, name,RGBA32,image_target);
		break;
	case _24BIT:
		GFXTransferTexture(data, name,RGB24,image_target);
		break;
	case _8BIT:
		GFXTransferTexture(data, name,PALETTE8, image_target);
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
		GFXCreateTexture(sizeX, sizeY, RGB24, &name, NULL, stage,ismipmapped,texture_target);
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
  if (name==-1)
    GFXSelectTexture(0,stage);
  else {
    assert(name!=-1);
    GFXSelectTexture(name,stage);
  }
}

