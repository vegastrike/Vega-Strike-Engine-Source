#include <set>
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "gfx/gauge.h"
#include "gfx/cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_factory.h"
#include "cmd/iterator.h"
#include "cmd/collection.h"
#include "gfx/hud.h"
#include "gfx/vdu.h"
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
#include "gfx/animation.h"
#include "gfx/mesh.h"
#include "universe_util.h"
#include "in_mouse.h"
#include "gui/glut_support.h"
#include "networking/netclient.h"
#include "cmd/unit_util.h"
#include "math.h"
#include "save_util.h"



#include "navscreen.h"
#include "gfx/masks.h"
#include "navgetxmldata.h"
#include "navitemstodraw.h"
#include "navparse.h"



//	This sets up the items in the navscreen

//	**********************************

NavigationSystem::~NavigationSystem() {

	draw = 0; 
	//delete mesh; 
	delete screenoccupation;
	delete mesh[0];
	delete mesh[1];
	delete mesh[2];
	delete mesh[3];
	delete mesh[4];
	delete mesh[5];
	delete mesh[6];
	delete mesh[7];
	delete factioncolours;
}
void NavigationSystem::mouseDrag(int x,int y){
	mousex=x;
	mousey=y;
}
void NavigationSystem::mouseMotion (int x, int y){
	mousex=x;
	mousey=y;
	
}
void NavigationSystem::mouseClick (int button, int state, int x, int y){
	mousex=x;
	mousey=y;
	if (state==WS_MOUSE_DOWN)
		mousestat|=(1<<lookupMouseButton(button));
	else
		mousestat&= (~(1<<lookupMouseButton(button)));
}

void NavigationSystem::Setup()

{
	configmode = 0;

	rotations = 0;

	minimumitemscaledown = 0.2;
	maximumitemscaleup = 3.0;

	axis = 3;
	rx = 0.0;
	ry = 0.0;
	rz = 0.0;
	zoom = 1.2;
	camera_z = 1.0;	//	updated after a pass
	center_x = 0.0;	//	updated after a pass
	center_y = 0.0;	//	updated after a pass
	center_z = 0.0;	//	updated after a pass

	zshiftmultiplier = 2.5;	//	shrink the output
	item_zscalefactor = 2.0;	//	camera distance prespective multiplier for affecting item sizes



	system_item_scale = 1.0;


	mouse_previous_state[0] = 0;	//	could have used a loop, but this way the system uses immediate instead of R type.
	mouse_previous_state[1] = 0;
	mouse_previous_state[2] = 0;
	mouse_wentup[0] = 0;
	mouse_wentup[1] = 0;
	mouse_wentup[2] = 0;
	mouse_wentdown[0] = 0;
	mouse_wentdown[1] = 0;
	mouse_wentdown[2] = 0;
	mouse_x_previous = (-1+float(mousex)/(.5*g_game.x_resolution));
	mouse_y_previous = (1+float(-1*mousey)/(.5*g_game.y_resolution));



	buttonstates = 0;
	if (getSaveData(0,"436457r1K3574r7uP71m35",0)<=1+XMLSupport::parse_int(vs_config->getVariable("general","times_to_show_help_screen","3"))) {
		whattodraw=0;
	} else {
		whattodraw = (1|2);
	}
	currentselection = NULL;
	factioncolours = new GFXColor[FactionUtil::GetNumFactions()];
	unselectedalpha = 1.0;


	int p;
	for( p=0; p < FactionUtil::GetNumFactions(); p++)
	{
		factioncolours[p].r = 1;
		factioncolours[p].g = 1;
		factioncolours[p].b = 1;
		factioncolours[p].a = 1;
	}

	for( p=0; p < NAVTOTALMESHCOUNT; p++)
		meshcoordinate_x[p] = 0.0;


	for( p=0; p < NAVTOTALMESHCOUNT; p++)
		meshcoordinate_y[p] = 0.0;

	for( p=0; p < NAVTOTALMESHCOUNT; p++)
		meshcoordinate_z[p] = 0.0;

	for( p=0; p < NAVTOTALMESHCOUNT; p++)
		meshcoordinate_z_delta[p] = 0.0;


	//	select target
	//	NAV/MISSION toggle
	//	









//	HERE GOES THE PARSING

//	*************************
		screenskipby4[0] = .3;
		screenskipby4[1] = .7;
		screenskipby4[2] = .3;
		screenskipby4[3] = .7;

		buttonskipby4_1[0] = .75;
		buttonskipby4_1[1] = .95;
		buttonskipby4_1[2] = .85;
		buttonskipby4_1[3] = .90;


		buttonskipby4_2[0] = .75;
		buttonskipby4_2[1] = .95;
		buttonskipby4_2[2] = .75;
		buttonskipby4_2[3] = .80;


		buttonskipby4_3[0] = .75;
		buttonskipby4_3[1] = .95;
		buttonskipby4_3[2] = .65;
		buttonskipby4_3[3] = .70;


		buttonskipby4_4[0] = .75;
		buttonskipby4_4[1] = .95;
		buttonskipby4_4[2] = .55;
		buttonskipby4_4[3] = .60;


		buttonskipby4_5[0] = .75;
		buttonskipby4_5[1] = .95;
		buttonskipby4_5[2] = .45;
		buttonskipby4_5[3] = .50;


		buttonskipby4_6[0] = .75;
		buttonskipby4_6[1] = .95;
		buttonskipby4_6[2] = .35;
		buttonskipby4_6[3] = .40;


		buttonskipby4_7[0] = .75;
		buttonskipby4_7[1] = .95;
		buttonskipby4_7[2] = .25;
		buttonskipby4_7[3] = .30;



	if(!ParseFile("navdata.xml"))

	{
		//	start DUMMP VARS
		screenskipby4[0] = .3;
		screenskipby4[1] = .7;
		screenskipby4[2] = .3;
		screenskipby4[3] = .7;


		buttonskipby4_1[0] = .75;
		buttonskipby4_1[1] = .95;
		buttonskipby4_1[2] = .85;
		buttonskipby4_1[3] = .90;


		buttonskipby4_2[0] = .75;
		buttonskipby4_2[1] = .95;
		buttonskipby4_2[2] = .75;
		buttonskipby4_2[3] = .80;


		buttonskipby4_3[0] = .75;
		buttonskipby4_3[1] = .95;
		buttonskipby4_3[2] = .65;
		buttonskipby4_3[3] = .70;


		buttonskipby4_4[0] = .75;
		buttonskipby4_4[1] = .95;
		buttonskipby4_4[2] = .55;
		buttonskipby4_4[3] = .60;


		buttonskipby4_5[0] = .75;
		buttonskipby4_5[1] = .95;
		buttonskipby4_5[2] = .45;
		buttonskipby4_5[3] = .50;


		buttonskipby4_6[0] = .75;
		buttonskipby4_6[1] = .95;
		buttonskipby4_6[2] = .35;
		buttonskipby4_6[3] = .40;


		buttonskipby4_7[0] = .75;
		buttonskipby4_7[1] = .95;
		buttonskipby4_7[2] = .25;
		buttonskipby4_7[3] = .30;

		//	end DUMMY VARS
	}





	ScreenToCoord(screenskipby4[0]);
	ScreenToCoord(screenskipby4[1]);
	ScreenToCoord(screenskipby4[2]);
	ScreenToCoord(screenskipby4[3]);


	ScreenToCoord(buttonskipby4_1[0]);
	ScreenToCoord(buttonskipby4_1[1]);
	ScreenToCoord(buttonskipby4_1[2]);
	ScreenToCoord(buttonskipby4_1[3]);


	ScreenToCoord(buttonskipby4_2[0]);
	ScreenToCoord(buttonskipby4_2[1]);
	ScreenToCoord(buttonskipby4_2[2]);
	ScreenToCoord(buttonskipby4_2[3]);


	ScreenToCoord(buttonskipby4_3[0]);
	ScreenToCoord(buttonskipby4_3[1]);
	ScreenToCoord(buttonskipby4_3[2]);
	ScreenToCoord(buttonskipby4_3[3]);


	ScreenToCoord(buttonskipby4_4[0]);
	ScreenToCoord(buttonskipby4_4[1]);
	ScreenToCoord(buttonskipby4_4[2]);
	ScreenToCoord(buttonskipby4_4[3]);


	ScreenToCoord(buttonskipby4_5[0]);
	ScreenToCoord(buttonskipby4_5[1]);
	ScreenToCoord(buttonskipby4_5[2]);
	ScreenToCoord(buttonskipby4_5[3]);


	ScreenToCoord(buttonskipby4_6[0]);
	ScreenToCoord(buttonskipby4_6[1]);
	ScreenToCoord(buttonskipby4_6[2]);
	ScreenToCoord(buttonskipby4_6[3]);


	ScreenToCoord(buttonskipby4_7[0]);
	ScreenToCoord(buttonskipby4_7[1]);
	ScreenToCoord(buttonskipby4_7[2]);
	ScreenToCoord(buttonskipby4_7[3]);


//	reverse = XMLSupport::parse_bool (vs_config->getVariable ("joystick","reverse_mouse_spr","true"))?1:-1;

	reverse = -1;



	if((screenskipby4[1]-screenskipby4[0]) < (screenskipby4[3]-screenskipby4[2]))
		system_item_scale *= (screenskipby4[1]-screenskipby4[0]); // is actually over 1, which is itself 
	else
		system_item_scale *= (screenskipby4[3]-screenskipby4[2]);

	screenoccupation = new navscreenoccupied(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], 1);
}

//	**********************************












void visitSystemHelp (Cockpit * cp, string systemname) {
	string key (string("visited_")+systemname);
	vector<string> *v = &_Universe->AccessCockpit()->savegame->getMissionStringData(key);
	if (v->empty())v->push_back ("v"); else (*v)[0]="v";
	
}
void visitSystem (Cockpit * cp , string systemname ) {
	visitSystemHelp (cp,systemname);
	int adj = UniverseUtil::GetNumAdjacentSystems(systemname);
	for (int i=0;i<adj;++i) {
		visitSystemHelp (cp,UniverseUtil::GetAdjacentSystem(systemname,i));
	}

}
//	This is the main draw loop for the nav screen
//	**********************************
void NavigationSystem::Draw()
{
  if (!CheckDraw())
    return;

  if(_Universe->AccessCockpit()->GetParent() == NULL)
	return;
	//	DRAW THE SCREEN MODEL
	//	**********************************
  visitSystem (_Universe->AccessCockpit(),_Universe->activeStarSystem()->getFileName());
	Vector p,q,r;
	_Universe->AccessCamera()->GetOrientation(p,q,r);  
 

	GFXEnable (TEXTURE0);
	GFXEnable (DEPTHTEST);
	GFXClear (GFXFALSE);

	GFXEnable(LIGHTING);

	for(int i = 0; i<NAVTOTALMESHCOUNT; i++)
	{
		float screen_x = 0.0;
		float screen_y = 0.0;
		float screen_z = 0.0;

		screen_x = meshcoordinate_x[i];
		screen_y = meshcoordinate_y[i];
		screen_z = meshcoordinate_z[i];


		if(checkbit(buttonstates, (i-1)))	//	button1 = 0, starts at -1, returning 0, no addition done
		{
			screen_z += meshcoordinate_z_delta[i];
		}


		QVector pos (0,0,0);

		//offset horizontal
		//***************
		pos = (p.Cast()*screen_x)+pos;
		//***************


		//offset vertical
		//***************
		pos = (q.Cast()*screen_y)+pos;
		//***************


		//offset sink
		//***************
		pos = (r.Cast()*screen_z)+pos;
		//***************


		Matrix mat (p,q,r,pos);
		mesh[i]->DrawNow(1,true,mat);
	}



	GFXBlendMode (SRCALPHA,INVSRCALPHA);
    GFXColor4f (1,1,1,1);
	GFXEnable (TEXTURE0);
	GFXDisable (TEXTURE1);
	GFXDisable(LIGHTING);

	GFXHudMode (true);
	GFXDisable (DEPTHTEST);
	GFXDisable (DEPTHWRITE);
	//	**********************************





	screenoccupation->reset();





	//	Set Mouse
	//	**********************************
	SetMouseFlipStatus();	//	define bools 'mouse_wentdown[]' 'mouse_wentup[]'
	//	**********************************







	//	Draw the screen basics
	//	**********************************
	DrawCursor(-1+float(mousex)/(.5*g_game.x_resolution),1+float(-1*mousey)/(.5*g_game.y_resolution), .1, .2, GFXColor(1,1,1,0.5) );
	//	**********************************







	//	Draw the Navscreen Functions
	//	**********************************
	if(checkbit(whattodraw, 1))	
	{
		DrawGrid(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], GFXColor(1,1,1,0.2) );


		if(checkbit(whattodraw, 2))
			DrawGalaxy();

		else
			DrawSystem();
	}

	else
	{
		if (checkbit(whattodraw,2))
			DrawShip();
		else
			DrawMission();
	}

	//	**********************************






	//	Draw Button Outlines
	//	**********************************
	bool outlinebuttons = 0;
//	if(configmode > 0)
//		outlinebuttons = 1;

	DrawButton(buttonskipby4_1[0], buttonskipby4_1[1], buttonskipby4_1[2], buttonskipby4_1[3], 1, outlinebuttons);
	DrawButton(buttonskipby4_2[0], buttonskipby4_2[1], buttonskipby4_2[2], buttonskipby4_2[3], 2, outlinebuttons);
	DrawButton(buttonskipby4_3[0], buttonskipby4_3[1], buttonskipby4_3[2], buttonskipby4_3[3], 3, outlinebuttons);
	DrawButton(buttonskipby4_4[0], buttonskipby4_4[1], buttonskipby4_4[2], buttonskipby4_4[3], 4, outlinebuttons);
	DrawButton(buttonskipby4_5[0], buttonskipby4_5[1], buttonskipby4_5[2], buttonskipby4_5[3], 5, outlinebuttons);
	DrawButton(buttonskipby4_6[0], buttonskipby4_6[1], buttonskipby4_6[2], buttonskipby4_6[3], 6, outlinebuttons);
	DrawButton(buttonskipby4_7[0], buttonskipby4_7[1], buttonskipby4_7[2], buttonskipby4_7[3], 7, outlinebuttons);
	//	**********************************







	//	Save current mouse location as previous for next cycle
	//	**********************************
	mouse_x_previous = (-1+float(mousex)/(.5*g_game.x_resolution));
	mouse_y_previous = (1+float(-1*mousey)/(.5*g_game.y_resolution));
	//	**********************************




	GFXHudMode (false);
}
//	**********************************































//	This is the mission info screen
//	**********************************
void NavigationSystem::DrawMission()
{
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);


	navdrawlist factionlist(0, screenoccupation, factioncolours);


	float deltax = screenskipby4[1] - screenskipby4[0];
	float deltay = screenskipby4[3] - screenskipby4[2];
	float originx = screenskipby4[0]; // left
	float originy = screenskipby4[3]; // top
	vector <float> * killlist = &_Universe->AccessCockpit()->savegame->getMissionData (string("kills"));
	string relationskills="Relations";
	
	if (killlist->size()>0) {
		relationskills+=" | Kills";
		
	}
	
	factionlist.drawdescription(relationskills, (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor(.3,1,.3,1));
	factionlist.drawdescription(" ", (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor(.3,1,.3,1));

	factionlist.drawdescription(" ", (originx + (0.3*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor(.3,1,.3,1));
	factionlist.drawdescription(" ", (originx + (0.3*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor(.3,1,.3,1));


	int numfactions = FactionUtil::GetNumFactions();
	int i = 0;
	string factionname = "factionname";
	float relation = 0.0;

	while(i < numfactions)
	{
		factionname = FactionUtil::GetFactionName(i);
		if (factionname!="neutral"&&factionname!="privateer"&&factionname!="planets"&&factionname!="upgrades"&&factionname!="unknown") {
		relation = 	FactionUtil::GetIntRelation(i, ( UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()) )->faction );

		//	draw faction name
		factionlist.drawdescription(FactionUtil::GetFactionName(i), (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, factioncolours[i]);

		relation = relation * 0.5;
		relation = relation + 0.5;
		int percent = relation * 100.0;
		string relationtext (XMLSupport::tostring (percent));
		if (i<killlist->size()) {
			relationtext+=" | ";
			relationtext += XMLSupport::tostring ((int)(*killlist)[i]);
		}

		factionlist.drawdescription(relationtext, (originx + (0.3*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),1));
		}
		i+=1;
	}
	if (i<killlist->size()) {
		string relationtext("Total Kills: ");
		relation=1;
		
		relationtext += XMLSupport::tostring ((int)(*killlist)[i]);
		factionlist.drawdescription(relationtext, (originx + (0.3*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),1));
							
	}
   
//	factionlist.drawdescription(" Terran : ", (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor(.3,1,.3,1));
//	factionlist.drawdescription(" Rlaan : ", (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor(1,.3,.3,1));
//	factionlist.drawdescription(" Aera : ", (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, GFXColor(.3,.3,1,1));


//	float love_from_terran = FactionUtil::getRelation(1);
//	float love_from_rlaan = FactionUtil::getRelation(2);
//	float love_from_aera = FactionUtil::getRelation(3);


	TextPlane displayname;
	displayname.col = GFXColor(1,1,1,1);
	displayname.SetSize (.6,-.8);
	displayname.SetPos(originx+ (.1*deltax)+.37,   originy/*+(1*deltay)*/);
	displayname.SetText ("*******#00a6FFVega Strike 0.4#000000*********\nUse #8080FFTab#000000 to afterburn\n#8080FF+,-#000000 act as cruise controll\nYour ship undocks stopped;\nhit #8080FF+ or /#000000 to go to full speed,\n#8080FFarrows#000000 steer your ship. The #8080FFt#000000 key\ntargets objects; #8080FFspace#000000 fires at them\nThe #8080FFa#000000 key activates insystem warp\nto cross vast distances to your target.\nTime Compression: #8080FFF9; F10#000000 resets.\nBuy a jump drive for 10000\nFly into a green jump wireframe; hit #8080FFj#000000\nto warp to a system. Target a base.\nWhen you get close a green box shows.\nInside the box, #8080FFd#000000 will land.\nBe sure to click Options at the base,\nand select the Save option.\nPress #8080FFshift-M#000000 to toggle help.\n#FF0000If Vega Strike halts or acts oddly,#000000\n#FFFF00immediately#000000 post stderr.txt & stdout.txt\nto http://vegastrike.sourceforge.net/forums\nbefore you restart Vega Strike.\nPress #8080FFshift-M#000000 to get back here.\nRight buttons access the galaxy maps.");
	displayname.SetCharSize (1,1);
	displayname.Draw();
/*
	string exitinfo("To exit help press #8080FFshift-M#000000\n#8080FFShift-M#000000 will bring up this\nhelp menu any time.\nThe right buttons access the galaxy and system maps");

	displayname.SetSize (.6,-.8);
	displayname.SetPos(originx-.02,   originy-1.2);
	displayname.SetText (exitinfo);
	displayname.SetCharSize (1,1);
	displayname.Draw();*/
	GFXEnable(TEXTURE0);
}
//	**********************************



//	This is the mission info screen
//	**********************************
extern string MakeUnitXMLPretty(string str, Unit * un);
void NavigationSystem::DrawShip()
{
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);


	navdrawlist factionlist(0, screenoccupation, factioncolours);


	float deltax = screenskipby4[1] - screenskipby4[0];
	float deltay = screenskipby4[3] - screenskipby4[2];
	float originx = screenskipby4[0]; // left
	float originy = screenskipby4[3]; // top
	string writethis;
	Unit * par;
	int foundpos=0;
	
	if ((par = _Universe->AccessCockpit()->GetParent())) {
		writethis= MakeUnitXMLPretty ( par->WriteUnitString(),par);

	}
	
	TextPlane displayname;
	displayname.col = GFXColor(.3,1,.3,1);
	displayname.SetSize (.7,-.8);
	displayname.SetPos(originx- (.1*deltax),   originy/*+(1*deltay)*/);
	displayname.SetText (writethis);
	displayname.SetCharSize (1,1);
	displayname.Draw();
	

//	factionlist.drawdescription(writethis, (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 1, GFXColor(1,1,1,1));


	GFXEnable(TEXTURE0);
}








//	this sets weather to draw the screen or not
//	**********************************
void NavigationSystem::SetDraw(bool n)
{
  if (draw==-1){
    Setup();
    draw=0;
  }
	if(n != (draw==1))
	{
		ClearPriorities();
		draw = n?1:0;
	}
}
//	**********************************















//	This will set a wentdown and wentup flag just for the event of mouse button going down or up
//	this is an FF test. not a state test.
//	**********************************
void NavigationSystem::SetMouseFlipStatus()
{
//	getMouseButtonStatus()&1 = mouse button 1 standard = button 1 VS
//	getMouseButtonStatus()&2 = mouse button 3 standard = button 2 VS
//	getMouseButtonStatus()&4 = mouse button 2 standard = button 3 VS

	//	use the VS scheme, (1 2 3) , instead of standard (1 3 2)
	//	state 0 = up
	//	state 1 = down


	bool status = 0;
	int i;
	for(i = 0; i < 3; i++)
	{

		status = ( getMouseButtonStatus()&(1<<i) );

		if( (status == 1)&&(mouse_previous_state[i] == 0) )
		{
			mouse_wentdown[i] = 1;
			mouse_wentup[i] = 0;
		}

		else if( (status == 0)&&(mouse_previous_state[i] == 1) )
		{
			mouse_wentup[i] = 1;
			mouse_wentdown[i] = 0;
		}

		else
		{
			mouse_wentup[i] = 0;
			mouse_wentdown[i] = 0;
		}
	}


	for(i = 0; i < 3; i++)
	{
		mouse_previous_state[i] = (getMouseButtonStatus()&(1<<i));	//	button 'i+1' state VS
	}
}

//	**********************************






















//	returns a modified vector rotated by x y z radians
//	**********************************
QVector NavigationSystem::dxyz(QVector vector, double x_, double y_, double z_)
{
/*
	  void Roll (QFLOAT rad){
		QFLOAT theta = atan2 (j,i)+rad;
		QFloat len = XSQRT (j*j+i*i);
		j = sin (theta)*len;
		i = cos (theta)*len;
	  } 
*/


	if(x_ != 0) 
	{
		float distance_yz = sqrt((vector.j * vector.j) + (vector.k * vector.k));
		float current_angle = atan2(vector.k,vector.j);
		current_angle += x_;
		vector.j = cos(current_angle) * distance_yz;
		vector.k = sin(current_angle) * distance_yz;
	}



	if(y_ != 0) 
	{
		float distance_xz = sqrt((vector.i * vector.i) + (vector.k * vector.k));
		float current_angle = atan2(vector.k,vector.i);
		current_angle += y_;
		vector.i = cos(current_angle) * distance_xz;
		vector.k = sin(current_angle) * distance_xz;
	}



	if(z_ != 0) 
	{
		float distance_xy = sqrt((vector.i * vector.i) + (vector.j * vector.j));
		float current_angle = atan2(vector.j,vector.i);
		current_angle += z_;
		vector.i = cos(current_angle) * distance_xy;
		vector.j = sin(current_angle) * distance_xy;
	}


	return vector;
}

//	**********************************







void NavigationSystem::setCurrentSystem(string newCurrentSystem) {
	currentsystem = newCurrentSystem;
	systemIter.init(currentsystem);
}











//	Passes a draw button command, with colour
//	Tests for a mouse over, to set colour
//	**********************************
//	1 = nav/mission
//	2 = select currentselection
//	3 = up
//	4 = down
//	5 = toggle prespective rezoom
int NavigationSystem::mousey=0;
int NavigationSystem::mousex=0;
int NavigationSystem::mousestat;

void NavigationSystem::DrawButton(float &x1, float &x2, float &y1, float &y2, int button_number, bool outline)
{

	float mx = -1+float(mousex)/(.5*g_game.x_resolution);
	float my = 1+float(-1*mousey)/(.5*g_game.y_resolution);


	bool inrange = TestIfInRange(x1, x2, y1, y2, mx, my);

	if((inrange == 1)&&(mouse_wentdown[0] == 1))
	{	

		currentselection = NULL;	//	any new button depression means no depression on map, no selection made

		if( (button_number != 2) && ( checkbit(buttonstates, 1)) )	//	unset select if start soemthing else
		{
			unsetbit(buttonstates, 1);
		}


		if(button_number == 2)	//	toggle select, toggle map rezoom
		{
			flipbit(buttonstates, (button_number-1));
		}
		else
			dosetbit(buttonstates, (button_number-1));	//	all other buttons go down

	}


	if((inrange == 1) && (checkbit(buttonstates, (button_number-1))) && (mouse_wentup[0]))
	{
		if(button_number != 2)	//	#2 is for select. do not deactivate untill something is (selected)
		{
			unsetbit(buttonstates, (button_number-1));	//	all but 2 go up, all are up in mission mode
		}



		if(!checkbit(whattodraw, 1))
		{
			unsetbit(buttonstates, (button_number-1));	// all are up in mission mode
		}



		if(button_number == 5)	//	releasing #1, toggle the draw (nav / mission)
		{
			axis = axis - 1;
			if(axis == 0)
				axis = 3;
		}



		if(button_number == 1)	//	releasing #1, toggle the draw (nav / mission)
		{
			flipbit(whattodraw, 1);
		}


		if(button_number == 3)	//	hit up
		{
			if(checkbit(whattodraw, 1)) // if in nav system NOT mission
			{
				dosetbit(whattodraw, 2);	//	draw galaxy
				setCurrentSystem(systemselection= UniverseUtil::getSystemFile());
				
			}
			else	//	if in mission mode
			{
				dosetbit(whattodraw, 2);	//	draw shipstats	
			}
		}



		if(button_number == 4)	//	hit down
		{
			if(checkbit(whattodraw, 1)) // if in nav system NOT mission
			{
				unsetbit(whattodraw, 2);	//	draw system
			}

			else	//	if in mission mode
			{
				unsetbit(whattodraw, 2);	//	draw system
			}
		}
	}

	if(inrange == 0)
	{
		if(button_number != 2)	//	#2 is for select. do not deactivate untill something is (selected), leave 5 too 
		{
			unsetbit(buttonstates, (button_number-1));
		}


		if(!checkbit(whattodraw, 1))	//	unset in mission mode !!
		{
			unsetbit(buttonstates, (button_number-1));
		}
	}


	if(outline == 1)
	{
		if(inrange == 1)
		{
			if(checkbit(buttonstates, (button_number-1)))
				DrawButtonOutline(x1, x2, y1, y2, GFXColor(1,0,0,1)); 

			else
				DrawButtonOutline(x1, x2, y1, y2, GFXColor(1,1,0,1)); 
		}
		else
		{
			if(checkbit(buttonstates, (button_number-1)))
				DrawButtonOutline(x1, x2, y1, y2, GFXColor(1,0,0,1)); 
			else
				DrawButtonOutline(x1, x2, y1, y2, GFXColor(1,1,1,1)); 
		}
	}
}
//	**********************************








//	Draws the actual button outline
//	**********************************
void NavigationSystem::DrawButtonOutline(float &x1, float &x2, float &y1, float &y2, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);


	GFXBegin(GFXLINE);
	GFXVertex3f(x1,y1,0);
	GFXVertex3f(x1,y2,0);
	GFXVertex3f(x2,y1,0);
	GFXVertex3f(x2,y2,0);
	GFXVertex3f(x1,y1,0);
	GFXVertex3f(x2,y1,0);
	GFXVertex3f(x1,y2,0);
	GFXVertex3f(x2,y2,0);
	GFXEnd();

	GFXEnable(TEXTURE0);
}
//	**********************************









//	this gets rid of states that could be damaging
//	**********************************
void NavigationSystem::ClearPriorities()
{
	unsetbit(buttonstates, 1);
	currentselection = NULL;
	rx = 0.0;
	ry = 0.0;
	rz = 0.0;
}
//	**********************************















//	tests if given are in the range
//	**********************************
bool NavigationSystem::TestIfInRange(float &x1, float &x2, float &y1, float &y2, float tx, float ty)
{
	if ( ((tx < x2) && (tx > x1)) && ((ty < y2) && (ty > y1)) )
		return 1;
	else
		return 0;
}
//	**********************************











//	tests if given are in the circle range
//	**********************************
bool NavigationSystem::TestIfInRangeRad(float &x, float &y, float size, float tx, float ty)
{
	if(( ((x - tx)*(x - tx)) + ((y - ty)*(y - ty)) )<((0.5*size)*(0.5*size)) )
		return 1;
	else
		return 0;
}

//	**********************************









//	Tests if given are in block range
//	**********************************
bool NavigationSystem::TestIfInRangeBlk(float &x, float &y, float size, float tx, float ty)
{
	if( (Delta(tx,x) < (0.5 * size)) && (Delta(ty,y) < (0.5 * size)) )
		return 1;
	else
		return 0;
}
//	**********************************







/*
//	Gived the delta of 2 items
//	**********************************
float NavigationSystem::Delta(float a, float b)
{

	float ans = a-b;
	if(ans < 0)
		return (-1.0 * ans);
	else
		return ans;
}	
//	**********************************
*/







//	converts the % of screen system to 0-center system
//	**********************************
void NavigationSystem::ScreenToCoord(float &x)
{
	x -= .5;
	x *= 2;
}
//	**********************************








//	checks if the draw flag is 1
//	**********************************
bool NavigationSystem::CheckDraw()
{
	return draw==1;
}
//	**********************************

void NavigationSystem::Adjust3dTransformation()
{
	//	Adjust transformation
	//	**********************************
	if(	(mouse_previous_state[0] == 1) && TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], (-1+float(mousex)/(.5*g_game.x_resolution)), (1+float(-1*mousey)/(.5*g_game.y_resolution))))
	{
		float ndx = -1.0*( (1+float(-1*mousey)/(.5*g_game.y_resolution)) - mouse_y_previous);
		float ndy = -4.0*( (-1+float(mousex)/(.5*g_game.x_resolution)) - mouse_x_previous);
		float ndz = 0.0;

		rx = rx+=ndx;
		ry = ry+=ndy;
		rz = rz+=ndz;

		if(rx > 3.14/2)		rx = 3.14/2;
		if(rx < -3.14/2)	rx = -3.14/2;

		if(ry >= 6.28)		ry -= 6.28;
		if(ry <= -6.28)		ry += 6.28;

		if(rz >= 6.28)		rz -= 6.28;
		if(rz <= -6.28)		rz += 6.28;
	}
	//	**********************************



	//	Set the prespective zoom level
	//	**********************************
	if(	(mouse_previous_state[1] == 1) && TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], (-1+float(mousex)/(.5*g_game.x_resolution)), (1+float(-1*mousey)/(.5*g_game.y_resolution))))
	{
		zoom = zoom + 10*( /*1.0 +*/ 8*( (1+float(-1*mousey)/(.5*g_game.y_resolution)) - mouse_y_previous) );
//		if(zoom < 1.0)
//			zoom = 1.0;
		static float zoommax = XMLSupport::parse_float (vs_config->getVariable("graphics","nav_zoom_max","100"));
		if (zoom<-zoommax*MAXZOOM)
			zoom=-zoommax*MAXZOOM;
		if(zoom > MAXZOOM)
			zoom = MAXZOOM;
	}
	//	**********************************
}

void NavigationSystem::ReplaceAxes(QVector &pos) {
	//replace axes
	//*************************
	if(axis != 3)	//	3 == z == default
	{
		if(axis == 2)
		{
			float old_i = pos.i;
			float old_j = pos.j;
			float old_k = pos.k;
			pos.i = old_i;
			pos.j = -old_k;
			pos.k = old_j;
		}
		else	//	(axis == 1)
		{
			float old_i = pos.i;
			float old_j = pos.j;
			float old_k = pos.k;
			pos.i = old_j;
			pos.j = -old_k;
			pos.k = old_i;
		}
	}
	//*************************

}

void NavigationSystem::RecordMinAndMax (const QVector &pos, float &min_x, float &max_x,
		float &min_y, float &max_y, float &min_z, float &max_z, float &max_all)
{
	//Record min and max
	//**********************************
	if((float)pos.i > max_x)
		max_x = (float)pos.i;

	if((float)pos.i < min_x)
		min_x = (float)pos.i;
			
	if( fabs((float)pos.i) > max_all )
		max_all = fabs((float)pos.i);




	if((float)pos.j > max_y)
		max_y = (float)pos.j;

	if((float)pos.j < min_y)

		min_y = (float)pos.j;

	if( fabs((float)pos.j) > max_all )
		max_all = fabs((float)pos.j);




	if((float)pos.k > max_z)
		max_z = (float)pos.k;

	if((float)pos.k < min_z)
		min_z = (float)pos.k;

	if( fabs((float)pos.k) > max_all )
		max_all = fabs((float)pos.k);

	//**********************************	
}

void NavigationSystem::DrawOriginOrientationTri(float center_nav_x, float center_nav_y)
{



	//Draw Origin Orientation Tri
	//**********************************	
	QVector directionx;
	QVector directiony;
	QVector directionz;

	if(axis == 2)
	{
		directionx.i = 0.1;
		directionx.j = 0.0;
		directionx.k = 0.0;

		directionz.i = 0.0;
		directionz.j = 0.1;
		directionz.k = 0.0;

		directiony.i = 0.0;
		directiony.j = 0.0;
		directiony.k = 0.1;
	}

	else if(axis == 1)
	{
		directiony.i = 0.1;
		directiony.j = 0.0;
		directiony.k = 0.0;

		directionz.i = 0.0;
		directionz.j = 0.1;
		directionz.k = 0.0;

		directionx.i = 0.0;
		directionx.j = 0.0;
		directionx.k = 0.1;
	}
	else	//	(axis == 3)
	{
		directionx.i = 0.1;
		directionx.j = 0.0;
		directionx.k = 0.0;

		directiony.i = 0.0;
		directiony.j = 0.1;
		directiony.k = 0.0;

		directionz.i = 0.0;
		directionz.j = 0.0;
		directionz.k = 0.1;
	}

	directionx = dxyz(directionx, 0, ry, 0);
	directionx = dxyz(directionx, rx, 0, 0);

	directiony = dxyz(directiony, 0, ry, 0);
	directiony = dxyz(directiony, rx, 0, 0);

	directionz = dxyz(directionz, 0, ry, 0);
	directionz = dxyz(directionz, rx, 0, 0);


	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	float where_x = center_nav_x - 0.8*((screenskipby4[1] - screenskipby4[0])/2);
	float where_y = center_nav_y - 0.8*((screenskipby4[3] - screenskipby4[2])/2);

	GFXBegin(GFXLINE);

	GFXColorf(GFXColor(1,0,0,0.5));
	GFXVertex3f(where_x,where_y,0);
	GFXVertex3f(	where_x+(directionx.i * (0.3/(0.3 - directionx.k)) )
				,	where_y+(directionx.j * (0.3/(0.3 - directionx.k)) )
				,0);

	GFXColorf(GFXColor(0,1,0,0.5));
	GFXVertex3f(where_x,where_y,0);
	GFXVertex3f(	where_x+(directiony.i * (0.3/(0.3 - directiony.k)))
				,	where_y+(directiony.j * (0.3/(0.3 - directiony.k)))
				,0);

	GFXColorf(GFXColor(0,0,1,0.5));
	GFXVertex3f(where_x,where_y,0);
	GFXVertex3f(	where_x+(directionz.i * (0.3/(0.3 - directionz.k)))
				,	where_y+(directionz.j * (0.3/(0.3 - directionz.k)))
				,0);

	GFXEnd();
	GFXEnable(TEXTURE0);
	//**********************************
}

float NavigationSystem::CalculatePerspectiveAdjustment(float &zscale, float &zdistance,
		QVector &pos, QVector &pos_flat, float &system_item_scale_temp)
{
		
	ReplaceAxes(pos);
			
			
	pos_flat.i = pos.i;
	pos_flat.j = center_y;
//		pos_flat.j = 0.0;
	pos_flat.k = pos.k;


	//Modify by rotation amount
	//*************************
	pos = dxyz(pos, 0, ry, 0);
	pos = dxyz(pos, rx, 0, 0);

	pos_flat = dxyz(pos_flat, 0, ry, 0);
	pos_flat = dxyz(pos_flat, rx, 0, 0);
	//*************************

	pos.i -= center_x;
	pos.j -= center_y;
	pos.k -= center_z;

	pos_flat.i -= center_x;
	pos_flat.j -= center_y;
	pos_flat.k -= center_z;


	//CALCULATE PRESPECTIVE ADJUSTMENT
	//**********************************
	double real_z=(camera_z - pos.k);
	zdistance = sqrt( (pos.i * pos.i) + (pos.j * pos.j) + ( real_z * real_z ) );
	zscale = ( zdistance / (2.0*camera_z) ) * zshiftmultiplier;
	//zscale = 1.0;
	
	double real_zoom = ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / zscale );
	pos.i *= real_zoom;
	pos.j *= real_zoom;
	pos.k *= real_zoom;

	pos_flat.i *= real_zoom;
	pos_flat.j *= real_zoom;
	pos_flat.k *= real_zoom;
				

	float itemscale =  ( ((camera_z*item_zscalefactor)-zdistance) / (camera_z*item_zscalefactor) );	
				
//				if(zdistance > 2*camera_z)	//	outliers
//					itemscale = 1;

	if(itemscale < minimumitemscaledown)	//	dont shrink into nonexistance
		itemscale = minimumitemscaledown;

	if(itemscale > maximumitemscaleup)	//	dont expand too far
		itemscale = maximumitemscaleup;

	system_item_scale_temp = system_item_scale * itemscale;
	//**********************************
	return itemscale;
}

void NavigationSystem::TranslateAndDisplay (QVector &pos, QVector &pos_flat, float center_nav_x, float center_nav_y, float themaxvalue
		, float zscale, float zdistance, float &the_x, float &the_y, float &system_item_scale_temp)
{
	float itemscale = CalculatePerspectiveAdjustment(zscale, zdistance, pos, pos_flat, system_item_scale_temp);

	//TRANSLATE INTO SCREEN DISPLAY COORDINATES
	//**********************************
	the_x = (float)pos.i;
	the_y = (float)pos.j;
	float the_x_flat = (float)pos_flat.i;
	float the_y_flat = (float)pos_flat.j;


	the_x = (the_x / (	(themaxvalue - center_x)*2	));
	the_y = (the_y / (	(themaxvalue - center_y)*2	));
	the_x_flat = (the_x_flat / (	(themaxvalue - center_x)*2	));
	the_y_flat = (the_y_flat / (	(themaxvalue - center_y)*2	));


	the_x = (the_x * (screenskipby4[1] - screenskipby4[0]));
	the_x = the_x + center_nav_x;
	the_y = (the_y * (screenskipby4[3] - screenskipby4[2]));
	the_y = the_y + center_nav_y;

	the_x_flat = (the_x_flat * (screenskipby4[1] - screenskipby4[0]));
	the_x_flat = the_x_flat + center_nav_x;
	the_y_flat = (the_y_flat * (screenskipby4[3] - screenskipby4[2]));
	the_y_flat = the_y_flat + center_nav_y;
	//**********************************



	//Draw orientation lines
	//**********************************
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	GFXBegin(GFXLINE);

	//		GFXColorf(GFXColor(0,1,0,.5));
	//		GFXVertex3f(center_nav_x,	center_nav_y		,0);
	//		GFXVertex3f(the_x_flat,	the_y_flat	,0);


	if(the_y_flat > screenskipby4[3])
		the_y_flat = screenskipby4[3];

	if(the_y_flat < screenskipby4[2])
		the_y_flat = screenskipby4[2];


	if(the_y > screenskipby4[3])
		the_y = screenskipby4[3];

	if(the_y < screenskipby4[2])
		the_y = screenskipby4[2];


	if(	(the_x_flat > screenskipby4[0]) && (the_x_flat < screenskipby4[1]) )
		{
			GFXColorf(GFXColor(0,1,0,.2));
			GFXVertex3f(the_x_flat,	the_y_flat	,0);
			GFXVertex3f(the_x,		the_y		,0);
		}


	if(the_x_flat < screenskipby4[0])
		the_x_flat = screenskipby4[0];

	if(the_x_flat > screenskipby4[1])
		the_x_flat = screenskipby4[1];


	DrawCircle(the_x_flat, the_y_flat, (.005*system_item_scale), GFXColor(1,1,1,.2));

	GFXEnd();
	GFXEnable(TEXTURE0);
	//**********************************
}


