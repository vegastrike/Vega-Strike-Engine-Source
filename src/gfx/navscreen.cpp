#include "vs_path.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "gauge.h"
#include "cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_factory.h"
#include "cmd/iterator.h"
#include "cmd/collection.h"
#include "hud.h"
#include "vdu.h"
#include "lin_time.h"//for fps
#include "config_xml.h"
#include "lin_time.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include "main_loop.h"
#include <assert.h>	// needed for assert() calls
#include "savegame.h"
#include "animation.h"
#include "mesh.h"
#include "universe_util.h"
#include "in_mouse.h"
#include "gui/glut_support.h"
#include "networking/netclient.h"
#include "navscreen.h"
void NavigationSystem::Setup()
{
	meshes = 2;
//	vschdir ("nav"); 
//	vschdir ("default");
	mesh[0] = new Mesh("nav/default/console_body.xmesh",1,0,NULL);
	mesh[1] = new Mesh("nav/default/console_screen.xmesh",1,0,NULL);
//	vscdup(); 
//	vscdup(); 
}

void NavigationSystem::Draw()
{
	Vector p,q,r;
	QVector pos (0,0,0);
	_Universe->AccessCamera()->GetOrientation(p,q,r);  
	pos = pos + (r*100);
	Matrix mat (p,q,r,pos); 

	GFXEnable (TEXTURE0);

	GFXClear (GFXFALSE);
        GFXEnable (LIGHTING);
        GFXEnable (DEPTHTEST);
        GFXEnable(DEPTHWRITE);
        GFXEnable(CULLFACE);
//	UniverseUtil::IOmessage(0,"game","all","DRAW SHIT NOW");
	for(int i = 0; i<meshes; i++)
		mesh[i]->DrawNow(1,true,mat);


	GFXBlendMode (SRCALPHA,INVSRCALPHA);
    GFXColor4f (1,1,1,1);

	GFXEnable (TEXTURE0);
	GFXDisable (DEPTHTEST);
        GFXDisable (DEPTHWRITE);
	GFXDisable (TEXTURE1);


//    static int revspr = XMLSupport::parse_bool (vs_config->getVariable ("joystick","reverse_mouse_spr","true"))?1:-1;
//    static string blah = vs_config->getVariable("joystick","mouse_crosshair","crosshairs.spr");
//    static Sprite MouseSprite (blah.c_str(),BILINEAR,GFXTRUE);
	static Sprite MouseSprite ("white.bmp",BILINEAR,GFXTRUE);
//	MouseSprite.SetPosition (-1+float(mousex)/(.5*g_game.x_resolution),-revspr+float(revspr*mousey)/(.5*g_game.y_resolution));
	MouseSprite.SetPosition(.5,.5);
    MouseSprite.Draw();


}

void NavigationSystem::SetDraw(bool n)
{
	draw = n;
}

bool NavigationSystem::CheckDraw()
{
	return draw;
}
