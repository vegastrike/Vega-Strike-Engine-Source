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
//#include <gl/gl.h>
#include "gfx_aux_texture.h"

#include "gfxlib.h"
#include <string>
using namespace std;
#include "hashtable.h"
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

static Hashtable<string, Texture> texHashTable;

int Texture::checkold(const string &s)
{
	Texture *oldtex;
	//FIX'D
	if(oldtex = texHashTable.Get(s))//NOT problem???  not == last time
	{
		*this = *oldtex;
		original = oldtex;
		original->refcount++;
		return TRUE;
	}
	else
	{
	  oldtex = (Texture*)malloc(sizeof(Texture));
	  texHashTable.Put(s, oldtex);
	  original = oldtex;
	  return FALSE;
	}
}

void Texture::setold()
{
	*original = *this;
	original->original = NULL;
	original->refcount++;
}

Texture::Texture(char * FileName, int stage)
{
	refcount = 0;
	this->stage = stage;
	FILE *fp = NULL;
	fp = fopen (FileName, "r+b");
	if (!fp)
	{
		data = NULL;
		return;
	}
	fseek (fp,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
	//long temp;
	BITMAPINFOHEADER info;
	fread(&info, SIZEOF_BITMAPINFOHEADER,1,fp);
	sizeX = info.biWidth;	
	sizeY = info.biHeight;
	printf ("sizex%d sizey%d",sizeX,sizeY);


	//while(1);
	char t[64];
	strcpy(t, FileName);
	t[strlen(FileName)-3] = 'a';
	t[strlen(FileName)-2] = 'l';
	t[strlen(FileName)-1] = 'p';
	FILE *fp2 = fopen(t, "r+b");

	string texfilename = string(FileName);
	if(fp2) {
		texfilename += string(t);
	}
	if(checkold(texfilename))
		return;

	if(info.biBitCount == 24)
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
	else if(info.biBitCount == 8)
	{
		mode = _8BIT;
		data = NULL;
		data= new unsigned char [sizeY*sizeX];
		unsigned char *paltemp = palette;
		unsigned char ctemp;
		for(int palcount = 0; palcount < 256; palcount++)
		{
			fread(paltemp, sizeof(RGBQUAD), 1, fp);
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

Texture::Texture (char * FileNameRGB, char *FileNameA, int stage)
{
	refcount = 0;
	this->stage = stage;
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
	sizeX = info.biWidth;
	sizeY = info.biHeight;
	BITMAPINFOHEADER info1;
	FILE *fp1 = NULL;

	string texfilename = string(FileNameRGB) + string(FileNameA);
	if(checkold(texfilename))
		return;

	if (FileNameA)
	{
		fp1 = fopen (FileNameA, "r+b");
		if (!fp1)
		{
			data = NULL;
			printf("Alpha file %s not found\n",FileNameA);
			FileNameA = NULL;
			//fclose(fp);
			//*this = Texture(FileNameRGB, NULL);
			//return;
		}
		else
		{
			fseek (fp1,SIZEOF_BITMAPFILEHEADER,SEEK_SET);
			
			fread (&info1,SIZEOF_BITMAPINFOHEADER,1,fp1);
			if (sizeX != (unsigned int) info1.biWidth||sizeY!=(unsigned int)info1.biHeight)
			{
				data = NULL;
				return;
			}
			RGBQUAD ptemp1;	
			if (info1.biBitCount == 8)
			{
				for (int i=0; i<256; i++)
					fread(&ptemp1, sizeof(RGBQUAD), 1, fp1); //get rid of the palette for a b&w greyscale alphamap

			}
		}
	}
	if(info.biBitCount == 24)
	{
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
				for (int k=2; k>=0;k--)
				{
					fread (data+k+4*j+itimes4width,sizeof (unsigned char),1,fp);
				}
				if (FileNameA)
				{
					if (info1.biBitCount==24)
						for (int k=2; k>=0;k--)
						{
							fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
							//*(data+3+4*j+itimes4width) = 30;
						}
					else
						fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
				}
				else
				{
					if (!data[4*j+itimes4width]&&!data[4*j+itimes4width+1]&&!data[4*j+itimes4width+2])
						data[4*j+itimes4width+3] = 0;
					else
						data[4*j+itimes4width+3] = 255;
				}
					//*(data+3+4*j+itimes4width) = 30;
				
				
			}
		}
	}
	else if(info.biBitCount == 8)
	{
		unsigned char index = 0;
		mode = _24BITRGBA;
		data = NULL;
		data= new unsigned char [4*sizeY*sizeX];
		unsigned char *paltemp = palette;
		unsigned char ctemp;
		for(int palcount = 0; palcount < 256; palcount++)
		{
			fread(paltemp, sizeof(RGBQUAD), 1, fp);
			ctemp = paltemp[0];
			paltemp[0] = paltemp[2];
			paltemp[2] = ctemp;
			paltemp+=4;
		}
		if (!data)
			return;
		//FIXME VEGASTRIKE???		int k=0;
		for (int i=sizeY-1; i>=0;i--)
		{
			for (unsigned int j=0; j<sizeX;j++)
			{
				fread (&index,sizeof (unsigned char),1,fp);
				data [4*(i*sizeX+j)] = palette[((short)index)*3];	
				data [4*(i*sizeX+j)+1] = palette[((short)index)*3+1];
				data [4*(i*sizeX+j)+2] = palette[((short)index)*3+2];
			}
		}
		if (FileNameA)
		{
			for (int i=sizeY-1; i>=0;i--)
			{
				int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
				for (unsigned int j=0; j<sizeX;j++)
				{
					if (info1.biBitCount==24)
						for (int k=2; k>=0;k--)
						{
							fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
						}
					else
						fread (data+3+4*j+itimes4width,sizeof (unsigned char),1,fp1);
				}
			}
		}
		else
		{
			for (unsigned int i=0; i<sizeY;i++)
			{
				int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
				for (unsigned int j=0; j<sizeX;j++)
				{
					if (!data[4*j+itimes4width]&&!data[4*j+itimes4width+1]&&!data[4*j+itimes4width+2])
						data[4*j+itimes4width+3]=0;
					else
						data[4*j+itimes4width+3]=255;
				}
			}

		}
	}
	Bind();
	fclose(fp);
	
	setold();

}
void Texture::Transfer ()
{
	//Implement this in D3D
	//if(mode == _8BIT)
	//	glColorTable(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, palette);

	switch (mode)
	{
	case _24BITRGBA:
		GFXTransferTexture(data, name);
		break;
	case _24BIT:
		GFXTransferTexture(data, name);
		break;
	case _8BIT:
		GFXTransferTexture(data, name);
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
		GFXCreateTexture(sizeX, sizeY, RGBA32, &name, NULL, stage);
		break;
	case _24BIT:
		//not supported by most cards, so i use rgba32
		//GFXCreateTexture(sizeX, sizeY, RGB24, &name);
		GFXCreateTexture(sizeX, sizeY, RGB32, &name, NULL, stage);
		break;
	case _8BIT:
		GFXCreateTexture(sizeX, sizeY, PALETTE8, &name, (char *)palette, stage);
		break;
	}
	Transfer();

	return name;

}

void Texture::MakeActive(int stage)
{
	GFXSelectTexture(name, this->stage);
}
