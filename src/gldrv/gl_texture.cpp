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
#include "gl_globals.h"
#include "vs_globals.h"
#include "vegastrike.h"

#include "gfxlib.h"

#ifndef GL_TEXTURE_CUBE_MAP_EXT
#define GL_TEXTURE_CUBE_MAP_EXT           0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 0x851A
#endif

//#define  MAX_TEXTURES 16384
static GLint MAX_TEXTURE_SIZE=256;

GLenum GetUncompressedTextureFormat (TEXTUREFORMAT textureformat) {
  switch (textureformat) {
  case RGB24:
    return GL_RGB;
  case RGB32:
    return GL_RGB;
  case RGBA32:
    return GL_RGBA;
  case RGBA16:
    return GL_RGBA16;
  case RGB16:
    return GL_RGB16;
  default:
    return GL_RGBA;
  }
}

struct GLTexture{
  //  unsigned char *texture;
  GLubyte * palette;
  int width;
  int height;
  int texturestage;
  GLuint name;
  GFXBOOL alive;
  GLenum textureformat;
  GLenum targets;
  enum FILTER mipmapped;
};
//static GLTexture *textures=NULL;
//static GLEnum * targets=NULL;

static vector <GLTexture> textures;
static int activetexture[4]={-1,-1};

static void ConvertPalette(unsigned char *dest, unsigned char *src)
{
  for(int a=0; a<256; a++, dest+=4, src+=4) {
		memcpy(dest, src, 3);
		dest[3] = 255;
  }

}
int tmp_abs (int num) {
  return num<0?-num:num;
}
bool isPowerOfTwo (int num) {
  while (tmp_abs(num)>1) {
    if ((num/2)*2!=num) {
      return false;
    }
    num/=2;
  }
  return true;
}
GFXBOOL /*GFXDRVAPI*/ GFXCreateTexture(int width, int height, TEXTUREFORMAT textureformat, int *handle, char *palette , int texturestage, enum FILTER mipmap, enum TEXTURE_TARGET texture_target)
{
  if (!isPowerOfTwo (width)) {
    fprintf (stderr,"Width %d not a power of two",width);
    //    assert (false);
  }
  if (!isPowerOfTwo (height)) {
    fprintf (stderr,"Height %d not a power of two",height);
    //    assert (false);
    
  }
  GFXActiveTexture(texturestage);
  //case 3:  ... 3 pass... are you insane? well look who's talking to himself! oh.. good point :)
  *handle = 0;
  while (*handle<textures.size()) {
    if (!textures[*handle].alive){
      fprintf (stderr,"got dead tex");
      break;
    }else{
      (*handle)++;
    }
  }
  if ((*handle)==textures.size()) {
    fprintf (stderr,"!");
    textures.push_back(GLTexture());
    textures.back().palette=NULL;
    textures.back().alive=GFXTRUE;
    textures.back().name=-1;
    textures.back().width=textures.back().height=1;
  }

  GLenum WrapMode;
  switch (texture_target) {
  case TEXTURE2D: textures [*handle].targets=GL_TEXTURE_2D;
    WrapMode = GL_REPEAT;
    break;
  case CUBEMAP: textures [*handle].targets=GL_TEXTURE_CUBE_MAP_EXT;
    WrapMode = GL_CLAMP;
    fprintf (stderr, "stage %d, wid %d, hei %d",texturestage,width,height);
    break;
  }
  fprintf (stderr,"!");  
  textures[*handle].name = *handle+1; //for those libs with stubbed out handle gen't
  //fprintf (stderr,"Texture Handle %d",*handle);
  textures[*handle].alive = GFXTRUE;
  textures[*handle].texturestage = texturestage;
  textures[*handle].mipmapped = mipmap;
  fprintf (stderr,"!");
  glGenTextures (1,&textures[*handle].name);
  glBindTexture (textures[*handle].targets,textures[*handle].name);
  activetexture[texturestage]=*handle;
  glTexParameteri(textures[*handle].targets, GL_TEXTURE_WRAP_S, WrapMode);
  glTexParameteri(textures[*handle].targets, GL_TEXTURE_WRAP_T, WrapMode);
  if (textures[*handle].mipmapped&(TRILINEAR|MIPMAP)&&gl_options.mipmap>=2) {
    glTexParameteri (textures[*handle].targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (textures[*handle].mipmapped&TRILINEAR&&gl_options.mipmap>=3) {
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }else {
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    }
  } else {
    if (textures[*handle].mipmapped==NEAREST||gl_options.mipmap==0) {
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } else {
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
  }
  glTexParameterf (textures[*handle].targets,GL_TEXTURE_PRIORITY,.5);
  textures[*handle].width = width;
  textures[*handle].height = height;
  textures[*handle].palette=NULL;
  if (palette&&textureformat == PALETTE8){
    fprintf (stderr," palette ");  
    textures[*handle].palette = (GLubyte *)malloc (sizeof (GLubyte)*1024);
      ConvertPalette(textures[*handle].palette, (unsigned char *)palette);
    }
  textures[*handle].textureformat = GetUncompressedTextureFormat(textureformat);
  fprintf (stderr,"!");
  //  GFXActiveTexture(0);
  return GFXTRUE;
}
void /*GFXDRVAPI*/ GFXPrioritizeTexture (unsigned int handle, float priority) {
  glPrioritizeTextures (1,&handle,&priority); 
}
void /*GFXDRVAPI*/ GFXAttachPalette (unsigned char *palette, int handle)
{
  ConvertPalette(textures[handle].palette, palette);
  //memcpy (textures[handle].palette,palette,768);
}
static void DownSampleTexture (unsigned char **newbuf,const unsigned char * oldbuf, int height, int width, int pixsize, int handle, int maxdimension) {
  int i,j,k,l,m;
  if (MAX_TEXTURE_SIZE<maxdimension)
    maxdimension=MAX_TEXTURE_SIZE;
  float *temp = (float *)malloc (pixsize*sizeof(float));
  int newwidth = width>maxdimension?maxdimension:width;
  int scalewidth = width/newwidth;
  int newheight = height>maxdimension?maxdimension:height;
  int scaleheight = height/newheight;
  *newbuf = (unsigned char*)malloc (newwidth*newheight*pixsize*sizeof(unsigned char));
  for (i=0;i<newheight;i++) {
    for (j=0;j<newwidth;j++) {
      for (m=0;m<pixsize;m++) {
	temp[m]=0;
      }
      for (k=0;k<scaleheight;k++) {
	for (l=0;l<scalewidth;l++) {
	  for (m=0;m<pixsize;m++) {
	    temp[m] += oldbuf[m+pixsize*(j*scalewidth+l+width*(i*scaleheight+k))];
	  }
	}
      }
      for (m=0;m<pixsize;m++) {
	(*newbuf)[m+pixsize*(j+i*newwidth)] = (unsigned char)(temp[m]/(scaleheight*scalewidth));
      }
    }
  }
  free (temp);
  textures[handle].width=newwidth;
  textures[handle].height=newheight;
} 


static GLenum RGBCompressed (GLenum internalformat) {
    if (gl_options.compression) {
      internalformat = GL_COMPRESSED_RGB_ARB;
      if (gl_options.s3tc) {
	internalformat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      }
    }
    return internalformat;
}
static GLenum RGBACompressed (GLenum internalformat) {
    if (gl_options.compression) {
      internalformat = GL_COMPRESSED_RGBA_ARB;
      if (gl_options.s3tc) {
	switch (gl_options.compression) {
	case 3:
	  internalformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	  break;
	case 2:
	  internalformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	  break;
	case 1:
	  internalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	  break;
	}
      }
    }
    return internalformat;
}

GLenum GetTextureFormat (TEXTUREFORMAT textureformat) {
  switch(textureformat){
  case RGB32:
    return RGBCompressed (GL_RGB);
  case RGBA32:
    return RGBACompressed(GL_RGBA);
  case RGBA16:
    return RGBACompressed (GL_RGBA16);
  case RGB16:
    return RGBCompressed (GL_RGB16);
  default:
  case DUMMY:
  case RGB24:
    return RGBCompressed (GL_RGB);
  }
}
GLenum GetImageTarget (TEXTURE_IMAGE_TARGET imagetarget) {
  GLenum image2D;
  switch (imagetarget) {
  case TEXTURE_2D:
    image2D = GL_TEXTURE_2D;
    break;
  case CUBEMAP_POSITIVE_X:
    image2D = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT;
    break;
  case CUBEMAP_NEGATIVE_X:
    image2D=GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT;
    break;
  case CUBEMAP_POSITIVE_Y:
    image2D = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT;
    break;
  case CUBEMAP_NEGATIVE_Y:
    image2D=GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT;
    break;
  case CUBEMAP_POSITIVE_Z:
    image2D = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT;
    break;
  case CUBEMAP_NEGATIVE_Z:
    image2D=GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT;
    break;
  }
  return image2D;
}
GFXBOOL /*GFXDRVAPI*/ GFXTransferSubTexture (unsigned char * buffer, int handle, int x, int y, unsigned int width, unsigned int height, enum TEXTURE_IMAGE_TARGET imagetarget) {
  GLenum image2D=GetImageTarget (imagetarget);
  glBindTexture(textures[handle].targets, textures[handle].name);
  
  //  internalformat = GetTextureFormat (handle);
  glTexSubImage2D(image2D, 0, x,y,width,height,textures[handle].textureformat,GL_UNSIGNED_BYTE,buffer);
  return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXTransferTexture (unsigned char *buffer, int handle,  TEXTUREFORMAT internformat, enum TEXTURE_IMAGE_TARGET imagetarget,int maxdimension)
{	
  if (handle<0)
    return GFXFALSE;
  if (!isPowerOfTwo (textures[handle].width)|| !isPowerOfTwo (textures[handle].height)) {
    static unsigned char NONPOWEROFTWO[1024]={255,127,127,255,
					    255,255,0,255,
					    255,255,0,255,
					    255,127,127,255};
    buffer=NONPOWEROFTWO;
    textures[handle].width=2;
    textures[handle].height=2;
    //    assert (false);
  }
  if (maxdimension==65536) {
    maxdimension = gl_options.max_texture_dimension;
  }
  int error;
  unsigned char * tempbuf = NULL;
  unsigned char * tbuf =NULL;
  GLenum internalformat;
  GLenum image2D=GetImageTarget (imagetarget);
  glBindTexture(textures[handle].targets, textures[handle].name);
  if (textures[handle].width>maxdimension||textures[handle].height>maxdimension||textures[handle].width>MAX_TEXTURE_SIZE||textures[handle].height>MAX_TEXTURE_SIZE) {
    DownSampleTexture (&tempbuf,buffer,textures[handle].height,textures[handle].width,(internformat==PALETTE8?1:(internformat==RGBA32?4:3))* sizeof(unsigned char ), handle,maxdimension);
    buffer = tempbuf;
  }
  if (internformat!=PALETTE8) {
    internalformat = GetTextureFormat (internformat);
    if (textures[handle].mipmapped&&gl_options.mipmap>=2)
      gluBuild2DMipmaps(image2D, internalformat, textures[handle].width, textures[handle].height, textures[handle].textureformat, GL_UNSIGNED_BYTE, buffer);
    else
      glTexImage2D(image2D, 0, internalformat, textures[handle].width, textures[handle].height, 0, textures[handle].textureformat, GL_UNSIGNED_BYTE, buffer);
  }else {
#if defined(GL_COLOR_INDEX8_EXT)	// IRIX has no GL_COLOR_INDEX8 extension
    if (gl_options.PaletteExt) {
      error = glGetError();
      glColorTable_p(textures[handle].targets, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_BYTE, textures[handle].palette);
      error = glGetError();
      if (error) {
	if (tempbuf)
	  free(tempbuf);
	return GFXFALSE;
      }
      if (textures[handle].mipmapped&&gl_options.mipmap>=2)
	gluBuild2DMipmaps(image2D, GL_COLOR_INDEX8_EXT, textures[handle].width, textures[handle].height, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buffer);
      else
	glTexImage2D(image2D, 0, GL_COLOR_INDEX8_EXT, textures[handle].width, textures[handle].height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buffer);
#if 0
      error = glGetError();
      if (error) {
	if (tempbuf)
	  free(tempbuf);
	return GFXFALSE;
      }
#endif
    } else
#endif
    {
      int nsize = 4*textures[handle].height*textures[handle].width;
      tbuf =(unsigned char *) malloc (sizeof(unsigned char)*nsize);
      //      textures[handle].texture = tbuf;
      int j =0;
      for (int i=0; i< nsize; i+=4)
	{
	  tbuf[i] = textures[handle].palette[4*buffer[j]];
	  tbuf[i+1] = textures[handle].palette[4*buffer[j]+1];
	  tbuf[i+2] = textures[handle].palette[4*buffer[j]+2];
	  tbuf[i+3]= textures[handle].palette[4*buffer[j]+3];//used to be 255
	  j ++;
	}
      if (textures[handle].mipmapped&&gl_options.mipmap>=2)
	gluBuild2DMipmaps(image2D, 4, textures[handle].width, textures[handle].height, GL_RGBA, GL_UNSIGNED_BYTE, tbuf);
      else
	glTexImage2D(image2D, 0, 4, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tbuf);
      
      //unashiehized			glTexImage2D(image2D,0,3,textures[handle].width, textures[handle].height,0,GL_RGBA, GL_UNSIGNED_BYTE, tbuf);
      free (tbuf);
      //delete [] buffer;
      //buffer = tbuf;
    }
 
  }
  if (tempbuf)
    free(tempbuf);
  return GFXTRUE;
}
void /*GFXDRVAPI*/ GFXDeleteTexture (int handle) {
  if (textures[handle].alive) {
    glDeleteTextures(1, &textures[handle].name);
  }
  if (textures[handle].palette) {
    free  (textures[handle].palette);
    textures[handle].palette=0;
  }
  textures[handle].alive = GFXFALSE;
}
void GFXInitTextureManager() {
  for (int handle=0;handle<textures.size();handle++) {
    textures[handle].palette=NULL;
    textures[handle].width=textures[handle].height=0;
    textures[handle].texturestage=0;
    textures[handle].name=0;
    textures[handle].alive=0;
    textures[handle].textureformat=DUMMY;
    textures[handle].targets=0;
    textures[handle].mipmapped=NEAREST;
  }
  glGetIntegerv (GL_MAX_TEXTURE_SIZE,&MAX_TEXTURE_SIZE);
}
void GFXDestroyAllTextures () {
  for (int handle=0;handle<textures.size();handle++) {
    GFXDeleteTexture (handle);
  }
}


void /*GFXDRVAPI*/ GFXSelectTexture(int handle, int stage)
{
  //FIXME? is this legit?
  if (activetexture[stage]==handle) {
    GFXActiveTexture(stage);
    return ;
  } else
    activetexture[stage] = handle;
  if (gl_options.Multitexture) {
    GFXActiveTexture(stage);
    glBindTexture(textures[handle].targets, textures[handle].name);
  } else {
    Stage0Texture = GFXTRUE;
    if (stage) {
      Stage1Texture = GFXTRUE;
      Stage1TextureName = textures[handle].name;
    } else {
      Stage1Texture = GFXFALSE;
      glEnable (textures[handle].targets);
      glBindTexture(textures[handle].targets, textures[handle].name);
      Stage0TextureName = textures[handle].name;
    }
  }
}


