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
#include "cmd/unit_generic.h"
#include "hud.h"
#include "lin_time.h"
#include "file_main.h"
#include "gfx/aux_texture.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "xml_support.h"
//#include "glut.h"
void * getFont() {
  static std::string whichfont=vs_config->getVariable("graphics","font","helvetica12");
  void * retval=NULL;
  if (retval==NULL) {
    if (whichfont=="helvetica10")
      retval=GLUT_BITMAP_HELVETICA_10;
    else if (whichfont=="helvetica18")
      retval=GLUT_BITMAP_HELVETICA_18;
    else if (whichfont=="times24")
      retval=GLUT_BITMAP_TIMES_ROMAN_24;
    else if (whichfont=="times10")
      retval=GLUT_BITMAP_TIMES_ROMAN_10;
    else if (whichfont=="fixed13")
      retval=GLUT_BITMAP_8_BY_13;
    else if (whichfont=="fixed15")
      retval=GLUT_BITMAP_9_BY_15;
    else
      retval=GLUT_BITMAP_HELVETICA_12;
  }
  return retval;            
}
float getFontHeight() {
  static std::string whichfont=vs_config->getVariable("graphics","font","helvetica12");
  static float point=0;
  if (point==0) {
    if (whichfont=="helvetica10")
      point=22;
    else if (whichfont=="helvetica18")
      point=40;
    else if (whichfont=="times24")
      point=50;
    else if (whichfont=="times10")
      point=22;
    else if (whichfont=="fixed13")
      point=30;
    else if (whichfont=="fixed15")
      point=34;
    else
      point=26;      
  }
  return point/g_game.y_resolution;            
}


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
  void * fnt = getFont();
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
float charWidth (char c, float myFontMetrics) {
  static bool use_bit = XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
  void * fnt = use_bit?getFont():GLUT_STROKE_ROMAN;  
  float charwid = use_bit?glutBitmapWidth(fnt,c):glutStrokeWidth(fnt,c);
  float dubyawid = use_bit?glutBitmapWidth(fnt,'W'):glutStrokeWidth(fnt,'W');
  return charwid*myFontMetrics/dubyawid;
}
bool doNewLine(string::const_iterator begin,
               string::const_iterator end,
               float cur_pos,
               float end_pos,
               float metrics, 
               bool last_row) {
  if (*begin=='\n')
    return true;
  if (*begin==' '&&!last_row) {
    cur_pos+=charWidth(*begin,metrics);
    *begin++;
    for (;begin!=end&&cur_pos<=end_pos&&!isspace(*begin);begin++) {
      cur_pos+=charWidth(*begin,metrics);
    }
    return cur_pos>end_pos;
  }
  return cur_pos+((begin+1!=end)?charWidth(*begin,metrics):0)>=end_pos;
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
  void * fnt = getFont();
  static float std_wid=glutStrokeWidth (GLUT_STROKE_ROMAN,'W');
  myFontMetrics.i=font_point*std_wid/(119.05+33.33);
  if (use_bit)
	  myFontMetrics.i=glutBitmapWidth(fnt,'W');
  myFontMetrics.j=font_point;
  myFontMetrics.i/=.5*g_game.x_resolution;
  myFontMetrics.j/=.5*g_game.y_resolution;
  float tmp,row, col;
  float origcol,origrow;
  GetPos (row,col);
  GetPos(row,origcol);
  float rowheight=use_bit?getFontHeight():myFontMetrics.j;
  myFontMetrics.j=rowheight;

	  
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
    scalex=(_Universe->numPlayers()>3?_Universe->numPlayers()/2:_Universe->numPlayers())*myFontMetrics.i/std_wid;
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
        static bool setRasterPos= XMLSupport::parse_bool(vs_config->getVariable("graphics","set_raster_text_color","true"));
        if (use_bit&&setRasterPos)
          glRasterPos2f(col-origcol,0);
	text_it = text_it+6;
      } else {
        break;
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
      col+=glutBitmapWidth (fnt,' ')*5./(.5*g_game.x_resolution);;
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
    } else {
      if (use_bit) {
	col+=glutBitmapWidth (fnt,*text_it)/(float)(.5*g_game.x_resolution);;
      }else {
		  col+=myFontMetrics.i*glutStrokeWidth(GLUT_STROKE_ROMAN,*text_it)/std_wid;
      }
    }
    if(doNewLine(text_it,newText.end(),col,myDims.i, myFontMetrics.i,row-rowheight<=myDims.j)){
      GetPos (tmp,col);
      row -= rowheight;
      glPopMatrix();
      glPushMatrix ();
      glLoadIdentity();
	  if (drawbg) {
		GFXColorf(this->bgcol);
		DrawSquare(col,this->myDims.i,row-rowheight*.25,row+rowheight*.75);
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


