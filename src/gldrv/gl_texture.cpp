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

#define  MAX_TEXTURES 256
static int MAX_TEXTURE_SIZE=256;
struct GLTexture{
  //  unsigned char *texture;
  GLubyte * palette;
  int width;
  int height;
  int texturestage;
  GLuint name;
  GFXBOOL alive;
  TEXTUREFORMAT textureformat;
  GLenum targets;
  enum FILTER mipmapped;
};
//static GLTexture *textures=NULL;
//static GLEnum * targets=NULL;

static GLTexture textures[MAX_TEXTURES];
static int activetexture[4]={-1,-1};

static void ConvertPalette(unsigned char *dest, unsigned char *src)
{
  for(int a=0; a<256; a++, dest+=4, src+=4) {
		memcpy(dest, src, 3);
		dest[3] = 255;
  }

}

GFXBOOL /*GFXDRVAPI*/ GFXCreateTexture(int width, int height, TEXTUREFORMAT textureformat, int *handle, char *palette , int texturestage, enum FILTER mipmap, enum TEXTURE_TARGET texture_target)
{
  GFXActiveTexture(texturestage);
  //case 3:  ... 3 pass... are you insane? well look who's talking to himself! oh.. good point :)
  *handle = 0;
  while (*handle<MAX_TEXTURES&&textures[*handle].alive)
    (*handle)++;
  if (*handle==MAX_TEXTURES)
    return GFXFALSE;
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
  
  textures[*handle].name = *handle+1; //for those libs with stubbed out handle gen't
  //fprintf (stderr,"Texture Handle %d",*handle);
  textures[*handle].alive = GFXTRUE;
  textures[*handle].texturestage = texturestage;
  textures[*handle].mipmapped = mipmap;
  glGenTextures (1,&textures[*handle].name);
  glBindTexture (textures[*handle].targets,textures[*handle].name);
  
  glTexParameteri(textures[*handle].targets, GL_TEXTURE_WRAP_S, WrapMode);
  glTexParameteri(textures[*handle].targets, GL_TEXTURE_WRAP_T, WrapMode);
  if (textures[*handle].mipmapped&(TRILINEAR|MIPMAP)&&g_game.mipmap>=2) {
    glTexParameteri (textures[*handle].targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (textures[*handle].mipmapped&TRILINEAR&&g_game.mipmap>=3) {
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }else {
      glTexParameteri (textures[*handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    }
  } else {
    if (textures[*handle].mipmapped==NEAREST||g_game.mipmap==0) {
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
  if (palette&&textureformat == PALETTE8)
    {
      textures[*handle].palette = (GLubyte *)malloc (sizeof (GLubyte)*1024);
      ConvertPalette(textures[*handle].palette, (unsigned char *)palette);
    }
  textures[*handle].textureformat = textureformat;
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
static void DownSampleTexture (unsigned char **newbuf,const unsigned char * oldbuf, int height, int width, int pixsize, int handle) {
  int i,j,k,l,m;
  float *temp = (float *)malloc (pixsize*sizeof(float));
  int newwidth = width>MAX_TEXTURE_SIZE?MAX_TEXTURE_SIZE:width;
  int scalewidth = width/newwidth;
  int newheight = height>MAX_TEXTURE_SIZE?MAX_TEXTURE_SIZE:height;
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
	(*newbuf)[m+pixsize*(j+i*newwidth)] = temp[m]/(scaleheight*scalewidth);
      }
    }
  }
  free (temp);
  textures[handle].width=newwidth;
  textures[handle].height=newheight;
} 

GFXBOOL /*GFXDRVAPI*/ GFXTransferTexture (unsigned char *buffer, int handle,  enum TEXTURE_IMAGE_TARGET imagetarget)
{	
  int error;
  unsigned char * tempbuf = NULL;
  unsigned char * tbuf =NULL;
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
  if (image2D!=GL_TEXTURE_2D) {
    fprintf (stderr, "gotcha %d", imagetarget);
  }	

  glBindTexture(textures[handle].targets, textures[handle].name);
  if (textures[handle].width>MAX_TEXTURE_SIZE||textures[handle].height>MAX_TEXTURE_SIZE) {
    DownSampleTexture (&tempbuf,buffer,textures[handle].height,textures[handle].width,(textures[handle].textureformat==PALETTE8?1:4)* sizeof(unsigned char ), handle);
    buffer = tempbuf;
  }
  switch(textures[handle].textureformat){
  case DUMMY:
  case RGB24:
    fprintf (stderr,"RGB24 bitmaps not yet supported");
    break;
  case RGB32:
    if (textures[handle].mipmapped&&g_game.mipmap>=2)
      gluBuild2DMipmaps(image2D, 3, textures[handle].width, textures[handle].height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    else
      glTexImage2D(image2D, 0, 3, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    break;
  case RGBA32:
    if (textures[handle].mipmapped&&g_game.mipmap>=2)
      gluBuild2DMipmaps(image2D, 4, textures[handle].width, textures[handle].height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    else
      glTexImage2D(image2D, 0, 4, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    break;
  case RGBA16:
    if (textures[handle].mipmapped&&g_game.mipmap>=2)
      gluBuild2DMipmaps(image2D, GL_RGBA16, textures[handle].width, textures[handle].height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    else
      glTexImage2D(image2D, 0, GL_RGBA16, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    break;
  case RGB16:
    if (textures[handle].mipmapped&&g_game.mipmap>=2)
      gluBuild2DMipmaps(image2D, GL_RGB16, textures[handle].width, textures[handle].height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    else
      glTexImage2D(image2D, 0, GL_RGB16, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    break;
  case PALETTE8:
    if (g_game.PaletteExt) {
      error = glGetError();
      glColorTable(textures[handle].targets, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_BYTE, textures[handle].palette);
      error = glGetError();
      if (error) {
	if (tempbuf)
	  free(tempbuf);
	return GFXFALSE;
      }
      if (textures[handle].mipmapped&&g_game.mipmap>=2)
	gluBuild2DMipmaps(image2D, GL_COLOR_INDEX8_EXT, textures[handle].width, textures[handle].height, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buffer);
      else
	glTexImage2D(image2D, 0, GL_COLOR_INDEX8_EXT, textures[handle].width, textures[handle].height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buffer);
      break;
      error = glGetError();
      if (error) {
	if (tempbuf)
	  free(tempbuf);
	return GFXFALSE;
      }
    } else{
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
      if (textures[handle].mipmapped&&g_game.mipmap>=2)
	gluBuild2DMipmaps(image2D, 4, textures[handle].width, textures[handle].height, GL_RGBA, GL_UNSIGNED_BYTE, tbuf);
      else
	glTexImage2D(image2D, 0, 4, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tbuf);
      
      //unashiehized			glTexImage2D(image2D,0,3,textures[handle].width, textures[handle].height,0,GL_RGBA, GL_UNSIGNED_BYTE, tbuf);
      free (tbuf);
      //delete [] buffer;
      //buffer = tbuf;
    }
    break;
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
    delete [] textures[handle].palette;
    textures[handle].palette=0;
  }
  textures[handle].alive = GFXFALSE;
}
void GFXInitTextureManager() {
  for (int handle=0;handle<MAX_TEXTURES;handle++) {
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
  for (int handle=0;handle<MAX_TEXTURES;handle++) {
    GFXDeleteTexture (handle);
  }
}


void /*GFXDRVAPI*/ GFXSelectTexture(int handle, int stage)
{
  //FIXME? is this legit?
  if (activetexture[stage]==handle) {
    GFXActiveTexture(textures[handle].texturestage);
    return ;
  } else
    activetexture[stage] = handle;
  if (g_game.Multitexture) {
    GFXActiveTexture(textures[handle].texturestage);
    glBindTexture(textures[handle].targets, textures[handle].name);
    /*		if ((textures[handle].mipmapped&(TRILINEAR|MIPMAP))&&g_game.mipmap>=2) {
		glTexParameteri (textures[handle].targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (textures[handle].mipmapped&TRILINEAR) {
		glTexParameteri (textures[handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}else {
		glTexParameteri (textures[handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}
		} else {
		if (textures[handle].mipmapped==NEAREST||g_game.mipmap==0) {
		glTexParameteri (textures[handle].targets, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (textures[handle].targets, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else {
		glTexParameteri (textures[handle].targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (textures[handle].targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		}*/

    if(g_game.PaletteExt&&textures[handle].textureformat == PALETTE8) {
      //memset(textures[handle].palette, 255, 1024);
    }
  } else {
    Stage0Texture = GFXTRUE;
    if (textures[handle].texturestage) {
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

