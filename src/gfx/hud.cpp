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
#include "config_xml.h"
#include "xml_support.h"
//#include "glut.h"

TextPlane::TextPlane() {
  myDims.i = 2;  myDims.j=-2;
  char font[64]={0};
  char fonta[64]={0};
  myFontMetrics.Set(.06,.08,0);
  SetPos (0,0);
}
TextPlane::~TextPlane () {
}
void TextPlane::Draw (int offset) {
  Draw (myText,offset,true);
}

void TextPlane::Draw(const string & newText, int offset,bool startlower, bool force_highquality)
{
	// some stuff to draw the text stuff
  string::const_iterator text_it = newText.begin();
  static bool use_bit = force_highquality||XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
  static float font_point = XMLSupport::parse_float (vs_config->getVariable ("graphics","font_point","16"));
  static bool font_antialias = XMLSupport::parse_bool (vs_config->getVariable ("graphics","font_antialias","true"));
  void * fnt = g_game.x_resolution>=800?GLUT_BITMAP_HELVETICA_12:GLUT_BITMAP_HELVETICA_10;
  myFontMetrics.i=font_point*glutStrokeWidth (GLUT_STROKE_ROMAN,'W')/(119.05+33.33);
  myFontMetrics.j=font_point;
  myFontMetrics.i/=.5*g_game.x_resolution;
  myFontMetrics.j/=.5*g_game.y_resolution;
  float tmp,row, col;
  GetPos (row,col);
  if (startlower) {
      row -= (use_bit)?((fnt==GLUT_BITMAP_HELVETICA_12)?(26./g_game.y_resolution):(22./g_game.y_resolution)):(myFontMetrics.j);
  }
  GFXPushBlendMode();
  if (!use_bit&&font_antialias) {
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    glEnable(GL_LINE_SMOOTH);
  }else {
    GFXBlendMode (ONE,ZERO);
    glDisable (GL_LINE_SMOOTH);
  }

  GFXDisable (DEPTHTEST);
  GFXDisable (CULLFACE);

  GFXDisable (LIGHTING);

  GFXDisable (TEXTURE0);

  glPushMatrix();
  glRasterPos2f(0,0);
  int entercount=0;
  for (;entercount<offset&&text_it!=newText.end();text_it++) {
    if (*text_it=='\n')
      entercount++;
  }
  glTranslatef(col,row,0);  
  //  glRasterPos2f (g_game.x_resolution*(1-(col+1)/2),g_game.y_resolution*(row+1)/2);
  glRasterPos2f (0,0);
  float scalex=1;
  float scaley=1;
  
  if (!use_bit) {
    scalex=(_Universe->numPlayers()>3?_Universe->numPlayers()/2:_Universe->numPlayers())*myFontMetrics.i/glutStrokeWidth (GLUT_STROKE_ROMAN,'W');
    scaley=myFontMetrics.j/(119.05+33.33);
  }
  glScalef (scalex,scaley,1);
  while(text_it != newText.end() && row>myDims.j) {
    if(*text_it>=32) {//always true
      //glutStrokeCharacter (GLUT_STROKE_ROMAN,*text_it);
      if (use_bit)
	glutBitmapCharacter (fnt,*text_it);
      else
	glutStrokeCharacter (GLUT_STROKE_ROMAN,*text_it);
    }  
    
    if(*text_it=='\t') {
      col+=glutBitmapWidth (fnt,' ')*5./(2*g_game.x_resolution);;
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
    } else {
      if (use_bit) {
	col+=glutBitmapWidth (fnt,*text_it)/(float)(2*g_game.x_resolution);;
      }else {
	col+=myFontMetrics.i*glutStrokeWidth(GLUT_STROKE_ROMAN,*text_it)/glutStrokeWidth(GLUT_STROKE_ROMAN,'W');
      }
    }
    if(col+((text_it+1!=newText.end())?(use_bit?(glutBitmapWidth(fnt,*text_it)/(float)(2*g_game.x_resolution)):myFontMetrics.i):0)>=myDims.i||*text_it == '\n') {
      GetPos (tmp,col);
      row -= (use_bit)?((fnt==GLUT_BITMAP_HELVETICA_12)?(26./g_game.y_resolution):(22./g_game.y_resolution)):(myFontMetrics.j);
      glPopMatrix();
      glPushMatrix ();
      glTranslatef (col,row,0);
      glScalef(scalex,scaley,1);
      glRasterPos2f(0,0);
    }
    text_it++;
  }
  glDisable(GL_LINE_SMOOTH);
  glPopMatrix();

  
  GFXPopBlendMode();

}

