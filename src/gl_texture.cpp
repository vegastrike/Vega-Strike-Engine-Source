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
#include "vegastrike.h"
#include "gfxlib.h"
#include "gl_globals.h"

#define MAX_TEXTURES 256
struct GLTexture{
	unsigned char *texture;
	GLubyte * palette;
	unsigned int width;
	unsigned int height;
	int texturestage;
	GLuint name;
	BOOL alive;
	TEXTUREFORMAT textureformat;
	GLTexture ()
	{
		alive = FALSE;
		name = 0;
		width =0;
		height = 0;
		texture = NULL;
		palette = NULL;
	}
	~GLTexture()
	{
		if (palette)
			delete [] palette;
	}
};
static GLTexture *textures=NULL;

static void ConvertPalette(unsigned char *dest, unsigned char *src)
{
	for(int a=0; a<256; a++, dest+=3, src+=4)
		memcpy(dest, src, 3);

}

BOOL /*GFXDRVAPI*/ GFXCreateTexture(int width, int height, TEXTUREFORMAT textureformat, int *handle, char *palette , int texturestage )
{
	if (!textures)
		textures = new GLTexture [MAX_TEXTURES]; //if the dynamically allocated array is not made... make it
	if (g_game.Multitexture)
	{
		switch (texturestage )
		{
		case 0:
			glActiveTextureARB(GL_TEXTURE0_ARB);
			break;
		case 1:
			glActiveTextureARB(GL_TEXTURE1_ARB);
			break;
		//case 3:  ... 3 pass... are you insane? well look who's talking to himself! oh.. good point :)
		default:
			glActiveTextureARB(GL_TEXTURE0_ARB);
			break;
		}
	}
	
	*handle = 0;

	while (*handle<MAX_TEXTURES&&textures[*handle].alive)
		(*handle)++;
	if (*handle==MAX_TEXTURES)
		return FALSE;
	textures[*handle].name = *handle; //for those libs with stubbed out handle gen't
	//fprintf (stderr,"Texture Handle %d",*handle);
	textures[*handle].alive = TRUE;
	textures[*handle].texturestage = texturestage;
	glGenTextures (1,&textures[*handle].name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	textures[*handle].width = width;
	textures[*handle].height = height;
	if (palette&&textureformat == PALETTE8)
	{
		textures[*handle].palette = new GLubyte [769];
		ConvertPalette(textures[*handle].palette, (unsigned char *)palette);
		//memcpy (textures[*handle].palette,palette,768);
	}
	textures[*handle].textureformat = textureformat;

	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXAttachPalette (unsigned char *palette, int handle)
{
	ConvertPalette(textures[handle].palette, palette);
	//memcpy (textures[handle].palette,palette,768);
	return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXTransferTexture (unsigned char *buffer, int handle)
{
	//probably want to set a server state here
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &textures[handle].name);
	glGenTextures(1, &textures[handle].name);
	glBindTexture(GL_TEXTURE_2D, textures[handle].name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	int error;
	textures[handle].texture = buffer;
	
	switch(textures[handle].textureformat)
	{
	case DUMMY:
	case RGB24:
	  fprintf (stderr,"RGB24 bitmaps not yet supported");
	  break;
	case RGB32:
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;
	case RGBA32:
		glTexImage2D(GL_TEXTURE_2D, 0, 4, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;
	case RGBA16:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;
	case RGB16:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, textures[handle].width, textures[handle].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;
	case PALETTE8:
		if (g_game.PaletteExt)
		{
			
			glColorTable(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, textures[handle].palette);//shit on TNT
			error = glGetError();
			if (error) 
			{
				glColorTable(GL_TEXTURE_2D, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, textures[handle].palette);
				error = glGetError();
				if (error)
					return FALSE;
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, textures[handle].width, textures[handle].height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buffer);
			error = glGetError();
			if (error)
				return FALSE;
		}
		else
		{
			int nsize = 4*textures[handle].height*textures[handle].width;
			unsigned char * tbuf = NULL;
			tbuf = new unsigned char [nsize];
			textures[handle].texture = tbuf;
			int j =0;
			for (int i=0; i< nsize; i+=4)
			{
				tbuf[i] = textures[handle].palette[3*buffer[j]];
				tbuf[i+1] = textures[handle].palette[3*buffer[j]+1];
				tbuf[i+2] = textures[handle].palette[3*buffer[j]+2];
				tbuf[i+3]=255;
				j ++;
			}
			glTexImage2D(GL_TEXTURE_2D,0,3,textures[handle].width, textures[handle].height,0,GL_RGBA, GL_UNSIGNED_BYTE, tbuf);

			//delete [] buffer;
			//buffer = tbuf;
			
		}
		break;
	}
	glBindTexture(GL_TEXTURE_2D, textures[handle].name);
	return TRUE;

}
BOOL /*GFXDRVAPI*/ GFXDeleteTexture (int handle)
{
	
	if (textures[handle].texture)
	{
		textures[handle].texture = NULL;
		glDeleteTextures(1, &textures[handle].name);
	}
	if (textures[handle].palette)
		delete [] textures[handle].palette;
	textures[handle].alive = FALSE;
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSelectTexture(int handle, int stage)
{
	if (g_game.Multitexture)
	{
		switch(textures[handle].texturestage)
		{
		case 0:
			glActiveTextureARB(GL_TEXTURE0_ARB);	
			break;
		case 1:
			glActiveTextureARB(GL_TEXTURE1_ARB);			
			break;
		default:
			glActiveTextureARB(GL_TEXTURE0_ARB);			
			break;
		}
		glBindTexture(GL_TEXTURE_2D, textures[handle].name);
		//float ccolor[4] = {1.0,1.0,1.0,1.0};
		switch(textures[handle].texturestage)
		{
		case 0:
			glActiveTextureARB(GL_TEXTURE0_ARB);	
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glActiveTextureARB(GL_TEXTURE0_ARB);	
			break;
		case 1:
			glActiveTextureARB(GL_TEXTURE0_ARB);	
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glActiveTextureARB(GL_TEXTURE1_ARB);	
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glActiveTextureARB(GL_TEXTURE1_ARB);	
			break;
		default:
			glActiveTextureARB(GL_TEXTURE0_ARB);		
			glEnable (GL_TEXTURE_2D);		
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			break;
		}

	}
	else
	{
		Stage0Texture = TRUE;
		
		if (textures[handle].texturestage)
		{
			Stage1Texture = TRUE;
			Stage1TextureName = textures[handle].name;
		}	
		else
		{
			Stage1Texture = FALSE;
			glEnable (GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[handle].name);
			Stage0TextureName = textures[handle].name;
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		float ccolor[4] = {1.0,1.0,1.0,1.0};
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, ccolor);
	}
	
	if(g_game.PaletteExt&&textures[handle].textureformat == PALETTE8)
		glColorTable(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, textures[handle].palette);

	return TRUE;
}

