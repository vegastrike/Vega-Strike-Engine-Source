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