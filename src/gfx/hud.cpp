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

TextPlane::TextPlane(const GFXColor & c, const GFXColor & bgcol) {
  col=c;
  this->bgcol=bgcol;
  myDims.i = 2;  myDims.j=-2;
  myFontMetrics.Set(.06,.08,0);
  SetPos (0,0);
}
TextPlane::~TextPlane () {
}
int TextPlane::Draw (int offset) {
  return Draw (myText,offset,true);
}

static char * CreateLists() {
  static char lists[256]={0};
  void * fnt = g_game.x_resolution>=800?GLUT_BITMAP_HELVETICA_12:GLUT_BITMAP_HELVETICA_10;
  static bool use_bit = XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
  bool use_display_lists = XMLSupport::parse_bool (vs_config->getVariable ("graphics","text_display_lists","true"));
  if (use_display_lists) {
    for (unsigned char i=32;i<128;i++){
      lists[i]= GFXCreateList();
      if (use_bit)
	glutBitmapCharacter (fnt,i);
      else
	glutStrokeCharacter (GLUT_STROKE_ROMAN,i);
      if (!GFXEndList ()) {
	lists[i]=0;
      }
    }
  }  
  return lists;
}

static unsigned char HexToChar (char a) {
  if (a>='0'&&a<='9') 
    return a-'0';
  else if (a>='a'&&a<='f') {
    return 10+a-'a';
  }else if (a>='A'&&a<='F') {
    return 10+a-'A';
  }
  return 0;
}
static unsigned char TwoCharToByte (char a, char b) {
  return 16*HexToChar(a)+HexToChar(b);
}
static float TwoCharToFloat(char a, char b) {
  return (TwoCharToByte(a,b)/255.);
}
void DrawSquare(float left,float right, float top, float bot) {
	GFXBegin (GFXQUAD);
	GFXVertex3f(left,top,0);
	GFXVertex3f(left,bot,0);
	GFXVertex3f(right,bot,0);
	GFXVertex3f(right,top,0);
	GFXVertex3f(right,top,0);
	GFXVertex3f(right,bot,0);
	GFXVertex3f(left,bot,0);
	GFXVertex3f(left,top,0);

	GFXEnd ();
}
int TextPlane::Draw(const string & newText, int offset,bool startlower, bool force_highquality)
{
  int retval=1;
  bool drawbg = (bgcol.a!=0);
  static char * display_lists=CreateLists ();
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
  float rowheight=(use_bit)?((fnt==GLUT_BITMAP_HELVETICA_12)?(26./g_game.y_resolution):(22./g_game.y_resolution)):(myFontMetrics.j);
  if (startlower) {
      row -= rowheight;

  }
  GFXPushBlendMode();
  glLineWidth (1);
  if (!use_bit&&font_antialias) {
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    glEnable(GL_LINE_SMOOTH);

  }else {
    GFXBlendMode (ONE,ZERO);
    glDisable (GL_LINE_SMOOTH);
  }
  GFXColorf(this->col);

  GFXDisable (DEPTHTEST);
  GFXDisable (CULLFACE);

  GFXDisable (LIGHTING);

  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);

  glPushMatrix();
  glLoadIdentity();
  if (drawbg) {
	GFXColorf(this->bgcol);
	DrawSquare(col,this->myDims.i,row-rowheight*.25,row+rowheight);
  }
  GFXColorf(this->col);

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
  int potentialincrease=0;
  if (!use_bit) {
    scalex=(_Universe->numPlayers()>3?_Universe->numPlayers()/2:_Universe->numPlayers())*myFontMetrics.i/glutStrokeWidth (GLUT_STROKE_ROMAN,'W');
    scaley=myFontMetrics.j/(119.05+33.33);
  }
  glScalef (scalex,scaley,1);
  while(text_it != newText.end() && row>myDims.j) {
    if (*text_it=='#') {
      if (newText.end()>text_it+6) {
	float r,g,b;
	r = TwoCharToFloat (*(text_it+1),*(text_it+2));
	g=TwoCharToFloat (*(text_it+3),*(text_it+4));
	b=TwoCharToFloat (*(text_it+5),*(text_it+6));
	if (r==0&&g==0&&b==0) {
	  GFXColorf(this->col);
	}else {
	  GFXColor4f(r,
		     g,
		     b,
		     this->col.a);
	}
	text_it = text_it+6;
      }
      text_it++;
      continue;
    }else if(*text_it>=32) {//always true
		unsigned char myc = *text_it;
		if (myc=='_') {
			myc = ' ';
		}
      //glutStrokeCharacter (GLUT_STROKE_ROMAN,*text_it);
      retval+=potentialincrease;
      potentialincrease=0;
      int lists = display_lists[myc];
      if (lists) {
	GFXCallList(lists);
      }else{
	if (use_bit)
	  glutBitmapCharacter (fnt,myc);
	else
	  glutStrokeCharacter (GLUT_STROKE_ROMAN,myc);
      }
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
      row -= rowheight;
      glPopMatrix();
      glPushMatrix ();
      glLoadIdentity();
	  if (drawbg) {
		GFXColorf(this->bgcol);
		DrawSquare(col,this->myDims.i,row-rowheight*.25,row+rowheight);
		GFXColorf(this->col);
	  }
      if (*text_it=='\n') {
		GFXColorf(this->col);
      }
      glTranslatef (col,row,0);
      glScalef(scalex,scaley,1);
      glRasterPos2f(0,0);
      potentialincrease++;
    }
    text_it++;
  }
  glDisable(GL_LINE_SMOOTH);
  glPopMatrix();

  
  GFXPopBlendMode();
  GFXColorf(this->col);
  return retval;
}


