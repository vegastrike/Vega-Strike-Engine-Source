#include <set>
#include "vs_path.h"
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



#include "navscreen.h"
#include "gfx/masks.h"
#include "navscreenoccupied.h"
using std::set;
using std::string;
using std::vector;
static GFXColor GetColor(std::string source) {
		const float * col = (FactionUtil::GetSparkColor(FactionUtil::GetFactionIndex(UniverseUtil::GetGalaxyFaction(source))));
		return GFXColor (col[0],col[1],col[2],col[3]);
}
void Beautify (string systemfile, string & sector, string & system) {
	int slash = systemfile.find ("/");
	if (slash==string::npos) {
		sector="";
		system=systemfile;
	}else {
		sector = systemfile.substr(0,slash);
		system = systemfile.substr(slash+1);
	}
}

float SYSTEM_DEFAULT_SIZE=.05;
const int systemambiguous=0;
class systemdrawnode {
public:
	bool operator < (const systemdrawnode & a)const {
		return source<a.source;
	}
	bool operator == (const systemdrawnode & a)const {
		return source==a.source;
	}
	int type;
	float size;
	float x;
	float y;
	std::string source;
	bool moused;
	navscreenoccupied * screenoccupation;
	systemdrawnode(int type,float size,float x, float y, std::string source,navscreenoccupied * so, bool moused):type(type),size(size),x(x),y(y),source(source) {
		screenoccupation = so;
		this->moused=moused;
	}
	systemdrawnode () {
		size=SYSTEM_DEFAULT_SIZE;
		x=y=0;
		type=0;
		source="";
	}	
	void drawdescription(string text, float x_, float y_, float size_x, float size_y, bool ignore_occupied_areas, const GFXColor &col) const {	//	take the head and stick it in the back
	if(text.size() == 0)
		return;
	TextPlane displayname;	//	will be used to display shits names
	displayname.col = col;
	
	int length = text.size();
	float offset = (float(length)*0.005);
	if(ignore_occupied_areas)
		{
			displayname.SetPos((x_-offset), y_);
			displayname.SetText(text);
			displayname.SetCharSize(size_x, size_y);
			displayname.Draw();
		}
	else
		{
			float new_y = screenoccupation->findfreesector(x_, y_);
			displayname.SetPos((x_-offset), new_y);
			displayname.SetText(text);
			displayname.SetCharSize(size_x, size_y);
			displayname.Draw();
		}
	}

	void draw()const {
		if (moused)
			return;
		GFXColor race(GetColor (source));
//		race=GFXColor (1,1,1,1);
		NavigationSystem::DrawCircle(x, y, size, race);
		string blah,nam;
		Beautify (source,blah,nam);
		
		drawdescription (nam,x,y,1.0,1.0,0,race);
	}
};

typedef set <systemdrawnode> systemdrawset;
typedef vector <systemdrawnode> systemdrawlist;

bool testandset (bool &b,bool val) {
	bool tmp = b;
	b=val;
	return tmp;
}
float screensmash=1;//arbitrary constant used in calculating position below
class SystemIterator {
	vector <string> vstack;
	unsigned int which;
	unsigned int count;
	unsigned int maxcount;
	map <string,bool>visited;
public:
	SystemIterator (string current_system, unsigned int max =2){
		count=0;
		maxcount=max;
		vstack.push_back(current_system);
		visited[current_system]=true;
		which=0;
		
	}
	bool done ()const {
		return which>=vstack.size();
	}
	QVector Position () {
		
		if (done())
			return QVector(0,0,0);
		float ratio = ((float)count)/maxcount;
		float locatio =((float)which)/vstack.size();
	    unsigned int k=0;
		
		std::string::const_iterator start = vstack[which].begin();
		std::string::const_iterator end = vstack[which].end();
		for(;start!=end; start++) {
			k += (k * 128) + *start;
		}
		k %= 200000;
		float y = (k-100000)/(200000.);
		return QVector(ratio*cos(locatio*2*3.1415926536),ratio*sin(locatio*2*3.1415926536),0)*screensmash;
		
	}
	string operator * () {
		if (which<vstack.size())
			return vstack[which];
		return "-";
	}
	SystemIterator & next () {
		return ++(*this);
	}
	SystemIterator& operator ++ () {
		which +=1;
		if (which>=vstack.size()) {
			vector <string> newsys;
			for (unsigned int i=0;i<vstack.size();++i) {
				int nas = UniverseUtil::GetNumAdjacentSystems(vstack[i]);
				for (int j=0;j<nas;++j) {
					string n = UniverseUtil::GetAdjacentSystem(vstack[i],j);
					if (!testandset(visited[n],true)) {
						static bool dontbothervisiting = !XMLSupport::parse_bool (vs_config->getVariable ("graphics","explore_for_map","true"));
						if (dontbothervisiting||_Universe->AccessCockpit()->savegame->getMissionStringData(string("visited_")+n).size()>0) {
							newsys.push_back(n);
						}
					}
						
				}
			}
			vstack.swap(newsys);
			count+=1;
			which=0;
			if (count>maxcount)
				vstack.clear();
		}
		return *this;
	}
};
void NavigationSystem::DrawGalaxy()
{
	if (currentsystem.empty())
		currentsystem = UniverseUtil::getSystemFile();
	
	string csector,csystem;

	Beautify (currentsystem,csector,csystem);
	//what's my name
	//***************************
	TextPlane systemname;	//	will be used to display shits names
	string systemnamestring = "Current System : " + csector+" "+csystem;

//////////////////
////////THE FOLLOWING IS A COPY OF WHATS IN  NAVSCREEN.CPP IT WILL BE REWRITTEN
	int length = systemnamestring.size();
	float offset = (float(length)*0.005);
	systemname.col = GFXColor(1, 1, .7, 1);
	systemname.SetPos( (((screenskipby4[0]+screenskipby4[1])/2)-offset) , screenskipby4[3]);
	systemname.SetText(systemnamestring);
//	systemname.SetCharSize(1, 1);
	systemname.Draw();
	//***************************


	QVector pos;	//	item position
	QVector pos_flat;	//	item position flat on plane
	systemdrawset mainlist;//(0, screenoccupation, factioncolours);		//	lists of items to draw
	
	systemdrawlist mouselist;//(1, screenoccupation, factioncolours);	//	lists of items to draw that are in mouse range

	float zdistance = 0.0;
	float zscale = 0.0;

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
		zoom = zoom * ( 1.0 + 8*( (1+float(-1*mousey)/(.5*g_game.y_resolution)) - mouse_y_previous) );
		if(zoom < 1.0)
			zoom = 1.0;

		if(zoom > MAXZOOM)
			zoom = MAXZOOM;
	}
	//	**********************************

/////////////////
////////THE PREVIOUS IS A COPY OF WHATS IN  NAVSCREEN.CPP IT WILL BE REWRITTEN
/////////////////

#if 0	
	//	Set up first item to compare to + centres
	//	**********************************
	while( (*bleh) && (_Universe->AccessCockpit()->GetParent() != (*bleh)) &&(UnitUtil::isSun(*bleh) || !UnitUtil::isSignificant (*bleh))  )	//	no sun's in initial setup
	{
		++bleh;
	}

	if(!(*bleh)){	//	nothing there that's significant, just do it all
		bleh = UniverseUtil::getUnitList();
	}

		//GET THE POSITION
		//*************************
		pos = (*bleh)->Position();
#else
		pos.Set(0,0,0);
#endif
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


			//Modify by old rotation amount
			//*************************
			pos = dxyz(pos, 0, ry, 0);
			pos = dxyz(pos, rx, 0, 0);
			//*************************
		//*************************

#if 0
	float max_x = (float)pos.i;
	float min_x = (float)pos.i;
	float max_y = (float)pos.j;
	float min_y = (float)pos.j;
	float max_z = (float)pos.k;
	float min_z = (float)pos.k;
	float themaxvalue = abs(pos.i);
		
#else
	float max_x = (float)1;
	float min_x = (float)-1;
	float max_y = (float)1;
	float min_y = (float)-1;
	float max_z = (float)1;
	float min_z = (float)-1;
	float themaxvalue=1;
#endif

	float center_nav_x = ((screenskipby4[0] + screenskipby4[1]) / 2);
	float center_nav_y = ((screenskipby4[2] + screenskipby4[3]) / 2);
	//	**********************************

#if 0
	//Retrieve unit data min/max
	//**********************************
	while (*bleh)	//	this goes through one time to get the major components locations, and scales its output appropriately
	{
		if(UnitUtil::isSun(*bleh))
		{
			++bleh;
			continue;
		}
		string temp = (*bleh)->name; 



		pos = (*bleh)->Position();
		//replace axes
		//**************************
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
		//***************************



		//Modify by old rotation amount
		//*************************
		pos = dxyz(pos, 0, ry, 0);
		pos = dxyz(pos, rx, 0, 0);
		//*************************



		//Record min and max
		//**********************************
		if( (UnitUtil::isSignificant (*bleh)) || (_Universe->AccessCockpit()->GetParent() == (*bleh)) )
		{
			if((float)pos.i > max_x)
				max_x = (float)pos.i;

			if((float)pos.i < min_x)
				min_x = (float)pos.i;
			
			if( abs((float)pos.i) > themaxvalue )
				themaxvalue = abs((float)pos.i);




			if((float)pos.j > max_y)
				max_y = (float)pos.j;

			if((float)pos.j < min_y)

				min_y = (float)pos.j;

			if( abs((float)pos.j) > themaxvalue )
				themaxvalue = abs((float)pos.j);




			if((float)pos.k > max_z)
				max_z = (float)pos.k;

			if((float)pos.k < min_z)
				min_z = (float)pos.k;

			if( abs((float)pos.k) > themaxvalue )
				themaxvalue = abs((float)pos.k);

		}
		//**********************************

		++bleh;
	} 
	//**********************************

#endif

	//Find Centers
	//**********************************
	center_x = (min_x + max_x)/2;
	center_y = (min_y + max_y)/2;
	center_z = (min_z + max_z)/2;
	//**********************************


	themaxvalue = sqrt(themaxvalue*themaxvalue + themaxvalue*themaxvalue + themaxvalue*themaxvalue);


	//Set Camera Distance
	//**********************************
	camera_z = zoom * sqrt( 
				   ( (0.5*(max_x - min_x)) * (0.5*(max_x - min_x)) ) 
				+  ( (0.5*(max_y - min_y)) * (0.5*(max_y - min_y)) )
				+  ( (0.5*(max_z - min_z)) * (0.5*(max_z - min_z)) )
				);
	//**********************************






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




	SystemIterator blah(currentsystem);

	//	Enlist the items and attributes
	//	**********************************
	while (!blah.done())	//	this draws the points
	{

			//	Retrieve unit data
			//	**********************************
			string temp = (*blah);
			


			pos = (blah).Position();
			//replace axes
			//**************************
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
			//***************************



			pos_flat.i = pos.i;
			pos_flat.j = center_y;
//			pos_flat.j = 0.0;
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
				zdistance = sqrt( (pos.i * pos.i) + (pos.j * pos.j) + ( (camera_z - pos.k) * (camera_z - pos.k) ) );
				zscale = ( zdistance / (2.0*camera_z) ) * zshiftmultiplier;

				//zscale = 1.0;

				pos.i = pos.i * ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / zscale );
				pos.j = pos.j * ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / zscale );
				pos.k = pos.k * ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / zscale );

				pos_flat.i = pos_flat.i * ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / zscale );
				pos_flat.j = pos_flat.j * ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / zscale );
				pos_flat.k = pos_flat.k * ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / zscale );
				

				float itemscale =  ( ((camera_z*item_zscalefactor)-zdistance) / (camera_z*item_zscalefactor) );	
				
//				if(zdistance > 2*camera_z)	//	outliers
//					itemscale = 1;

				if(itemscale < minimumitemscaledown)	//	dont shrink into nonexistance
					itemscale = minimumitemscaledown;

				if(itemscale > maximumitemscaleup)	//	dont expand too far
					itemscale = maximumitemscaleup;

				float system_item_scale_temp = system_item_scale * itemscale;
				//**********************************





				//TRANSLATE INTO SCREEN DISPLAY COORDINATES
				//**********************************
				float the_x = (float)pos.i;
				float the_y = (float)pos.j;
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



				//IGNORE OFF SCREEN
				//**********************************
				if(	!TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], the_x, the_y))
				{
					++blah;
					continue;
				}
				//**********************************


			//	Now starts the test that determines the type of things and inserts
			//	|
			//	|
			//	\/


			float insert_size = 0.0;
			int insert_type = systemambiguous;

			

#if 0
			if(	(*blah)->isUnit()==UNITPTR )	//	unit
			{
				if(UnitUtil::isPlayerStarship(*blah) > -1)	//	is a PLAYER SHIP
				{
					if (UnitUtil::isPlayerStarship (*blah)==UniverseUtil::getCurrentPlayer()) //	is THE PLAYER
					{
						insert_type = navcurrentplayer;
						insert_size = navcurrentplayersize;
					}
					else	//	is A PLAYER
					{
						insert_type = navplayer;
						insert_size = navplayersize;
					}
				}
				else	//	is a non player ship
				{
					if(UnitUtil::isSignificant (*blah))	//	capship or station
					{
						if((*blah)->GetComputerData().max_speed()==0)		//	is this item STATIONARY?
						{
							insert_type = navstation;
							insert_size = navstationsize;
						}


						else // it moves = capship
						{
							if(ThePlayer->InRange((*blah),false,true))	//	only insert if in range
							{
								insert_type = navcapship;
								insert_size = navcapshipsize;
							}
						}
					}
					else	//	fighter
					{
						if(ThePlayer->InRange((*blah),false,true))	//	only insert if in range
						{
							insert_type = navfighter;
							insert_size = navfightersize;
						}
					}
				}
			}


			else if((*blah)->isUnit()==PLANETPTR  )	//	is it a PLANET?
			{
				if(UnitUtil::isSun(*blah))	//	is this a SUN?
				{
					insert_type = navsun;
					insert_size = navsunsize;
				}				
				

				else if (!((*blah)->GetDestinations().empty()))	//	is a jump point (has destinations)
				{
					insert_type = navjump;
					insert_size = navjumpsize;
				}


				else	//	its a planet
				{
					insert_type = navplanet;
					insert_size = navplanetsize;
				}
			}


			else if((*blah)->isUnit()==MISSILEPTR)
			{
				//	a missile
				insert_type = navmissile;
				insert_size = navmissilesize;
			}


			else if((*blah)->isUnit()==ASTEROIDPTR)
			{
				//	a missile
				insert_type = navasteroid;
				insert_size = navasteroidsize;
			}


			else if((*blah)->isUnit()==NEBULAPTR)
			{
				//	a missile
				insert_type = navnebula;
				insert_size = navnebulasize;
			}


			else	//	undefined non uni
				
#endif
					
			{
				insert_type = systemambiguous;
				insert_size = SYSTEM_DEFAULT_SIZE;
			}



  			if(system_item_scale_temp > (system_item_scale * 3))
			{
				system_item_scale_temp = (system_item_scale * 3);
			}

#if 0
			if (_Universe->AccessCockpit()->GetParent()->Target()==(*blah))
			{
				DrawTargetCorners(the_x, the_y, (insert_size*system_item_scale_temp), GFXColor(1, 0.3, 0.3, 0.8));
			}
#endif


			bool moused = false;
			if (TestIfInRangeRad(the_x, the_y, insert_size*system_item_scale_temp, (-1+float(mousex)/(.5*g_game.x_resolution)), (1+float(-1*mousey)/(.5*g_game.y_resolution))) ) {
				mouselist.push_back(systemdrawnode(insert_type, insert_size*system_item_scale_temp, the_x, the_y, (*blah),screenoccupation,false));
				moused=true;
			}
			
			mainlist.insert(systemdrawnode(insert_type, insert_size*system_item_scale_temp, the_x, the_y, (*blah),screenoccupation,moused));



		++blah;

	}

	//	**********************************	//	done enlisting items and attributes









	//	Adjust mouse list for 'n' kliks
	//	**********************************
	//	STANDARD	: (1 3 2) ~ [0] [2] [1]
	//	VS			: (1 2 3) ~ [0] [1] [2]	<-- use this
	if(mouselist.size() > 0)	//	mouse is over a target when this is > 0
	{
		if(mouse_wentdown[2]== 1)	//	mouse button went down for mouse button 2(standard)
			rotations += 1;
	}


	if(rotations >= mouselist.size())	//	dont rotate more than there is
		rotations = 0;


	systemdrawlist tmpv;
	int siz = mouselist.size();
	for (int l=0;l<siz;++l) {
		tmpv.push_back(mouselist[((unsigned int)(l+rotations))%((unsigned int)siz)]);
	}
	mouselist.swap(tmpv);
	//	**********************************






	//	Draw the damn shit
	//	**********************************
	for (systemdrawset::iterator it = mainlist.begin();it !=mainlist.end();++it) {
	   (*it).draw();
		string sys = (*it).source;
		int adj = UniverseUtil::GetNumAdjacentSystems(sys);
		GFXBegin(GFXLINE);	
	
		for (int i=0;i<adj;++i) {
			string oth = UniverseUtil::GetAdjacentSystem(sys,i);
			if (sys<oth) {
				systemdrawset::iterator j = mainlist.find (systemdrawnode(0,1,1,1,oth,screenoccupation,true));
				if (j!=mainlist.end()) {
					GFXColorf (GetColor (sys));
					GFXVertex3f((*it).x,(*it).y,0);
					GFXColorf (GetColor(oth));
					GFXVertex3f((*j).x,(*j).y,0);						  
				}
				
			}
		}
		GFXEnd();
	   
	}
	mainlist.clear();	//	whipe the list
	//	**********************************






	//	Check for selection query
	//	give back the selected tail IF there is one
	//	IF given back, undo the selection state
	//	**********************************
	if(1||checkbit(buttonstates, 1))	//	button #2 is down, wanting a (selection)
	{
		if(mouselist.size() > 0)	//	mouse is over a target when this is > 0
		{
			if(mouse_wentdown[0]== 1)	//	mouse button went down for mouse button 1
			{
				systemselection = mouselist.back().source;
				unsetbit(buttonstates, 1);

				// JUST FOR NOW, target == current selection. later it'll be used for other shit, that will then set target.
#if 0
				( UniverseUtil::getPlayerX( UniverseUtil::getCurrentPlayer() ) )->Target(systemselection);
				( UniverseUtil::getPlayerX( UniverseUtil::getCurrentPlayer() ) )->LockTarget(systemselection);
#else
				currentsystem=systemselection;
#endif
			}
		}
	}
	//	**********************************





	//	Clear the lists
	//	**********************************
	for (systemdrawlist::iterator it = mouselist.begin();it!=mouselist.end();++it) {
		(*it).draw();
		
	}
	
	mouselist.clear();	//	whipe mouse over'd list
	//	**********************************
}
//	**********************************









