#ifndef _NAVSCREEN_H_
#define _NAVSCREEN_H_

#include "gui/glut_support.h"

#include "navscreenoccupied.h"
#include "drawlist.h"
#include "navitemtypes.h"
#include "gfx/masks.h"


#define NAVTOTALMESHCOUNT 8	//	same as the button count, 1 mesh for screen and 1 per button(1+7)
#define MAXZOOM 10

void visitSystem (class Cockpit * cp, std::string systemname) ;

class NavigationSystem
{
private:
	std::string currentsystem;//FIXME
	std::string systemselection;

class navscreenoccupied* screenoccupation;
class Mesh * mesh[NAVTOTALMESHCOUNT];
int reverse;
int rotations;
int axis;
int configmode;
float rx;
float ry;
float rz;
float zoom;
float camera_z;
float zshiftmultiplier;
float item_zscalefactor;
float minimumitemscaledown;
float maximumitemscaleup;


float center_x;
float center_y;
float center_z;


float mouse_x_previous;
float mouse_y_previous;
signed char draw;
bool mouse_previous_state[3];
bool mouse_wentup[3];
bool mouse_wentdown[3];
UnitContainer currentselection;
GFXColor* factioncolours;



int whattodraw;
//	bit 0 = undefined
//	bit	1 = draw system screen / mission screen
//	bit 2 = draw galaxy screen


							//	coordinates done 'over left->right' by 'up bottom->top'
							//	values are 1/100 of the screen width and height
float screenskipby4[4];		//	0 = x-small	1 = x-large	2 = y-small	3 = y-large
float buttonskipby4_1[4];
float buttonskipby4_2[4];
float buttonskipby4_3[4];
float buttonskipby4_4[4];
float buttonskipby4_5[4];
float buttonskipby4_6[4];
float buttonskipby4_7[4];
float meshcoordinate_x[NAVTOTALMESHCOUNT];
float meshcoordinate_y[NAVTOTALMESHCOUNT];
float meshcoordinate_z[NAVTOTALMESHCOUNT];
float meshcoordinate_z_delta[NAVTOTALMESHCOUNT];


int   buttonstates;	//	bit0 = button1, bit1 = button2, etc
float system_item_scale;
float unselectedalpha;





public:
NavigationSystem() {draw = -1; whattodraw=(1|2);}
 ~NavigationSystem();
static void DrawCircle(float x, float y, float size, const GFXColor &col );
static void DrawHalfCircleTop(float x, float y, float size, const GFXColor &col );
static void DrawHalfCircleBottom(float x, float y, float size, const GFXColor &col );
static void DrawPlanet(float x, float y, float size, const GFXColor &col );
static void DrawStation(float x, float y, float size, const GFXColor &col );
static void DrawJump(float x, float y, float size, const GFXColor &col );
static void DrawMissile(float x, float y, float size, const GFXColor &col );
static void DrawTargetCorners(float x, float y, float size, const GFXColor &col );


void DrawButton(float &x1, float &x2, float &y1, float &y2, int button_number, bool outline);
void DrawButtonOutline(float &x1, float &x2, float &y1, float &y2, const GFXColor &col);
void DrawCursor(float x, float y, float wid, float hei, const GFXColor &col);
void DrawGrid(float &screen_x1, float &screen_x2, float &screen_y1, float &screen_y2, const GFXColor &col);


bool TestIfInRange(float &x1, float &x2, float &y1, float &y2, float tx, float ty);
bool TestIfInRangeBlk(float &x1, float &x2, float size, float tx, float ty);
bool TestIfInRangeRad(float &x, float &y, float size, float tx, float ty);
bool ParseFile(string filename);
bool CheckDraw();
void DrawSystem();
void DrawGalaxy();
void DrawMission();
void DrawShip();
void SetMouseFlipStatus();
void ScreenToCoord(float &x);
void Draw();
void Setup();
void SetDraw(bool n);
void ClearPriorities();
static int mousex;
static int mousey;
static int mousestat;
static void mouseDrag(int x,int y);
static void mouseMotion (int x, int y);
static void mouseClick (int button, int state, int x, int y);
static int getMouseButtonStatus() {return mousestat;}
class QVector dxyz(class QVector, double x_, double y_, double z_);
//float Delta(float a, float b);

};

#endif

