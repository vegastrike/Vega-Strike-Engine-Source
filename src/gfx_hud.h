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
#include <string>
#include "gfxlib.h"
#include "cmd_unit.h"

class HUDElement: public Unit {
public:
	HUDElement(char *filename);

	void Draw();
};
using namespace std ;
class TextPlane: public HUDElement {
	string myText;

	int myMaterial;
	GFXColor myColor;

	Texture *myFont;
	Vector myFontMetrics; // i = width, j = height
	Vector myDims;

	struct GlyphPosition {
		float left, right, top, bottom;
	} myGlyphPos[256];


	float rowpos, colpos;
public:
	TextPlane(char *filename);
	~TextPlane();

	void Draw();
	void SetText(const string &newText) {
		myText = newText;
	}

	void Scroll(float deltar, float deltac = 0) {
		rowpos+=deltar;
		colpos+=deltac;
	}
};
