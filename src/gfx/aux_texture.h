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
#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include "gfxlib.h"
#include "gfxlib_struct.h"
//#include <gl/gl.h>
#ifdef __cplusplus
#include <string>
#else
#include <string.h>
#endif
using namespace std;

struct Texture{
  char * texfilename;
  //  char filename[64];
  enum FILTER ismipmapped;
	unsigned int sizeX;
	unsigned int sizeY;
	unsigned char *data;
	enum {_8BIT, _24BIT, _24BITRGBA} mode;
  //unsigned char palette[256*4+1];
  unsigned char * palette;  ///if we statically allocate it, then gl_texture will kill it when destructor is called...and if we delete this texture we be ph00ked
	int name;
	int stage;

	Texture *original;
	int refcount;
        enum TEXTURE_TARGET texture_target;
        enum TEXTURE_IMAGE_TARGET image_target;
        GFXBOOL checkold(const string &s);
	void setold();
  //char  texfilename;
  void InitTexture() {
    original = 0;
    refcount = 0;
    name = -1;
  }
  int Bind();
  void Transfer();
public:

  Texture(const char *,const char *, int stage = 0, enum FILTER mipmap= MIPMAP, enum TEXTURE_TARGET target=TEXTURE2D, enum TEXTURE_IMAGE_TARGET imagetarget=TEXTURE_2D, float alpha=1, int zeroval=0);
  Texture(const char * FileName, int stage = 0, enum FILTER mipmap = MIPMAP, enum TEXTURE_TARGET target=TEXTURE2D, enum TEXTURE_IMAGE_TARGET imagetarget=TEXTURE_2D);
  Texture (Texture *t);
  ~Texture();
  static Texture * Exists (string s);
  static Texture * Exists (string s, string a);
  bool operator < (const Texture &b);
  bool operator == (const Texture &b);
  
  void MakeActive();
  bool LoadSuccess () {
    return (data!=NULL);
  }
  void Prioritize (float);
  //void Filter();
  //void NoFilter();
};
//Texture * LoadAlphaMap (char *FileName, float alpha=1);
//Texture * LoadTexture (char * FileName);
//Texture * LoadRGBATexture (char * FileNameRGB, char *FileNameA, float alpha=1);

//void DelTexDat (Texture *); //note does not remove from OpenGL
#endif
