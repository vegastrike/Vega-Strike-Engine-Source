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
#include "gfx_hud.h"
#include "lin_time.h"
#include "file.h"
float *mview = NULL;

HUDElement::HUDElement(char *filename):Unit(filename) {
	static Matrix view;
	//myMesh = mesh;
	if(mview==NULL) {
	  //	  fprintf (stderr,"I am very l337");
		mview = view;
		GFXLoadIdentity(VIEW);
		GFXLookAt(Vector(0,0,0), Vector(0,0,1), Vector(0,-1,0)); // optimization: cache this friggin' matrix
		GFXGetMatrix(VIEW, mview);
	}
}

void HUDElement::Draw() {
	Matrix tmatrix;
	GFXGetMatrix(VIEW, tmatrix);
	GFXLoadMatrix(VIEW, mview);
	
	Unit::Draw();

	GFXLoadMatrix(VIEW, tmatrix);
}

TextPlane::TextPlane(char *filename):HUDElement(filename),myColor(0,0,0)
{
	static Matrix view;
	//myMesh = mesh;
	if(mview==NULL) {
		mview = view;
		GFXLoadIdentity(VIEW);
		GFXLookAt(Vector(0,0,0), Vector(0,0,1), Vector(0,-1,0)); // optimization: cache this friggin' matrix
		GFXGetMatrix(VIEW, mview);
	}
	colpos = rowpos = 0;

	::LoadFile(filename);
	::SetPosition(fpos);
	//fseek(fpread, fpos, SEEK_SET);

	char font[64];

	ReadVector(myDims);
	ReadInt(myMaterial);

	ReadVector(myColor.r, myColor.g, myColor.b);
	ReadFloat(myColor.a);
	
	ReadString(font);
	myFont = new Texture(font);
	ReadVector(myFontMetrics);
	for(int a = 0; a < 96; a++) {
		ReadFloat(myGlyphPos[a].left);
		ReadFloat(myGlyphPos[a].right);
		ReadFloat(myGlyphPos[a].top);
		ReadFloat(myGlyphPos[a].bottom);
	}

	//fpos = ftell(fpread);
	fpos = ::GetPosition();
	::CloseFile();
}

TextPlane::~TextPlane()
{
	//delete myText;
	delete myFont;
}

void TextPlane::Draw()
{
	time += GetElapsedTime();
	UpdateMatrix();
	Vector np = Transform(pp,pq,pr,p), 
		nq = Transform(pp,pq,pr,q),
		nr = Transform(pp,pq,pr,r),
		npos = ppos+pos;
	for (int i=0;i<nummesh;i++) {
	  GFXLoadMatrix (MODEL,transformation) ;
	  meshdata[i]->Draw(np, nq, nr, npos);
	}
	Matrix tmatrix;
	GFXGetMatrix(VIEW, tmatrix);
	GFXLoadMatrix(VIEW, mview);
	
	// some stuff to draw the text stuff
	string::iterator text_it = myText.begin();
	float row = 0.0, col = 0.0;

	myFont->MakeActive();
	GFXSelectMaterial(myMaterial);
	GFXDisable(LIGHTING);
	GFXDisable(DEPTHTEST);
	GFXColor4f(1.0, 1.0, 1.0, 1.0);
	GFXBegin(QUADS);
	/*
	GFXTexCoord2f(0.0, 0.0);
	GFXVertex3f(0.0, 0.0, 0.0);
	GFXTexCoord2f(0.0, 1.0);
	GFXVertex3f(0.0, 1.0, 0.0);
	GFXTexCoord2f(1.0, 1.0);
	GFXVertex3f(1.0, 1.0, 0.0);
	GFXTexCoord2f(1.0, 0.0);
	GFXVertex3f(1.0, 0.0, 0.0);
	*/
	/*
	GFXTexCoord2f(0.0, 0.0);
	GFXVertex3f(0.0, 0.0, 0.0);
	GFXTexCoord2f(0.0, 0.0);
	GFXVertex3f(0.0, myDims.j, 0.0);
	GFXTexCoord2f(0.0, 0.0);
	GFXVertex3f(myDims.i, myDims.j, 0.0);
	GFXTexCoord2f(0.0, 0.0);
	GFXVertex3f(myDims.i, 0.0, 0.0);
	*/
	GFXBlendMode(ZERO, ZERO);

	while(*text_it != '\0' && row<myDims.j) {
	  if(*text_it>=32 && *text_it<=127) {//always true
			GlyphPosition g = myGlyphPos[*text_it-32];
			GFXTexCoord2f(g.left,g.top);
			GFXVertex3f(col, row, 0.0);
			GFXTexCoord2f(g.left,g.bottom);
			GFXVertex3f(col, row+myFontMetrics.j, 0.0);
			GFXTexCoord2f(g.right,g.bottom);
			GFXVertex3f(col+myFontMetrics.i, row+myFontMetrics.j, 0.0);
			GFXTexCoord2f(g.right,g.top);
			GFXVertex3f(col+myFontMetrics.i, row, 0.0);
		}

		if(*text_it=='\t')
			col+=myFontMetrics.i*5;
		else
			col+=myFontMetrics.i;
		if(col>myDims.i||*text_it == '\n') {
			col = 0.0;
			row += myFontMetrics.j;
		}
		text_it++;
	}

	GFXEnd();
	GFXEnable(LIGHTING);
	GFXEnable(DEPTHTEST);
	GFXLoadMatrix(VIEW, tmatrix);

	for(int subcount = 0; subcount < numsubunit; subcount++)
		subunits[subcount]->Draw(tmatrix, np, nq, nr, npos);
	if(aistate)
		aistate = aistate->Execute();
}

