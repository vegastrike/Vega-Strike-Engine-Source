/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
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
#include <ctype.h>
#include "gfxlib.h"
#include "hud.h"
#include "lin_time.h"
#include "file_main.h"
#include "gfx/aux_texture.h"


TextPlane::TextPlane(char *filename) {
  myDims.i = 2;  myDims.j=2;
  char font[64]={0};
  char fonta[64]={0};
  FILE * fp = fopen (filename, "r");
  if (fp) {
    fscanf (fp,"%63s %63s",font, fonta);
    if (fonta[0]) {
      myFont = new Texture(font,0,BILINEAR);
    }else {
      myFont = new Texture(font,fonta,0,BILINEAR);
    }
    fscanf (fp,"%f %f %f\n",&myFontMetrics.i,&myFontMetrics.j,&myFontMetrics.k);
    fscanf (fp,"%d\n",&numlet);
    if (numlet>256)numlet=256;
    for(int a = 0; a < numlet; a++) {
      fscanf (fp, "%f %f %f %f\n", &myGlyphPos[a].left, &myGlyphPos[a].right,&myGlyphPos[a].top,&myGlyphPos[a].bottom);
    }
    fclose (fp);
  }
  SetPos (0,0);
}
TextPlane::~TextPlane () {
  delete myFont;
}
void TextPlane::Draw () {
  Draw (myText);
}

void TextPlane::Draw(const string & newText)
{
	// some stuff to draw the text stuff
  string::const_iterator text_it = newText.begin();
  float row, col;
  GetPos (row,col);
  myFont->MakeActive();
  GFXBlendMode (ONE,ONE);
  GFXDisable (DEPTHTEST);
  GFXDisable (CULLFACE);
  GFXDisable (LIGHTING);
  GFXEnable(TEXTURE0);
  GFXBegin(GFXQUAD);
  while(*text_it != '\0' && row<myDims.j) {
    if(*text_it>=32 && *text_it<=127) {//always true
      GlyphPosition g = myGlyphPos[*text_it-32];
      
      GFXTexCoord2f(g.right,g.bottom);
      GFXVertex3f((col+myFontMetrics.i), row, 0.0);
      GFXTexCoord2f(g.right,g.top);
      GFXVertex3f((col+myFontMetrics.i), row+myFontMetrics.j, 0.0);
      GFXTexCoord2f(g.left,g.top);
      GFXVertex3f(col, row+myFontMetrics.j, 0.0);
      GFXTexCoord2f(g.left,g.bottom);
      GFXVertex3f(col, row, 0.0);
    }
    
    if(*text_it=='\t')
      col+=myFontMetrics.i*5;
    else
      col+=myFontMetrics.i;
    if(col>myDims.i||*text_it == '\n') {
      col = 0.0;
      row -= myFontMetrics.j;
    }
    text_it++;
  }
  GFXEnd();
}

