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
#include "vs_globals.h"
//#include "glut.h"

TextPlane::TextPlane(const char *filename) {
  myDims.i = 2;  myDims.j=-2;
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
  } else {
   myFont = new Texture ("9x12.bmp");
  }
  SetPos (0,0);
}
TextPlane::~TextPlane () {
  delete myFont;
}
void TextPlane::Draw (int offset) {
  Draw (myText,offset);
}

void TextPlane::Draw(const string & newText, int offset)
{
	// some stuff to draw the text stuff
  string::const_iterator text_it = newText.begin();
  void * fnt = g_game.x_resolution>=800?GLUT_BITMAP_HELVETICA_12:GLUT_BITMAP_HELVETICA_10;
  float tmp,row, col;
  GetPos (row,col);
  GFXPushBlendMode();
  GFXBlendMode (ONE,ZERO);
  GFXDisable (DEPTHTEST);
  GFXDisable (CULLFACE);
  GFXDisable (LIGHTING);
  GFXDisable (TEXTURE0);
  glPushMatrix();

  int entercount=0;
  for (;entercount<offset&&text_it!=newText.end();text_it++) {
    if (*text_it=='\n')
      entercount++;
  }
  glTranslatef(col,row,0);  
  //  glRasterPos2f (g_game.x_resolution*(1-(col+1)/2),g_game.y_resolution*(row+1)/2);
  glRasterPos2f (0,0);
  while(text_it != newText.end() && row>myDims.j) {
    if(*text_it>=32) {//always true
      GlyphPosition g = myGlyphPos[*text_it-32];
      //glutStrokeCharacter (GLUT_STROKE_ROMAN,*text_it);
      glutBitmapCharacter (fnt,*text_it);
      
    }
    
    if(*text_it=='\t') {
      col+=glutBitmapWidth (fnt,' ')*5./(2*g_game.x_resolution);;
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
    } else {
      col+=myFontMetrics.i;
      col+=glutBitmapWidth (fnt,*text_it)/(float)(2*g_game.x_resolution);;
    }
    if(col+((text_it+1!=newText.end())?(glutBitmapWidth(fnt,*text_it)/(float)(2*g_game.x_resolution)):0)>=myDims.i||*text_it == '\n') {
      GetPos (tmp,col);
      row -= (fnt==GLUT_BITMAP_HELVETICA_12)?(26./g_game.y_resolution):(22./g_game.y_resolution);
      glPopMatrix();
      glPushMatrix ();
      glTranslatef (col,row,0);
      glRasterPos2f(0,0);
    }
    text_it++;
  }
  glPopMatrix();
  GFXPopBlendMode();
}

