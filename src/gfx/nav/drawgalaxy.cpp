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
	char color;
	navscreenoccupied * screenoccupation;
	systemdrawnode(int type,float size,float x, float y, std::string source,navscreenoccupied * so, bool moused):type(type),size(size),x(x),y(y),source(source) {
		screenoccupation = so;
		this->moused=moused;
		color = 'v';
		vector<string> *v = &_Universe->AccessCockpit()->savegame->getMissionStringData ("visited_"+source);
		if (v->size()){
			string k = (*v)[0];
			if (k.length())
				color=k[0];
			
		}
		
		
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
		if (color=='m') {
			race.r=.5;
			race.g=.5;
			race.b=.5;
			race.a=.5;
				
		}
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

NavigationSystem::SystemIterator::SystemIterator (string current_system, unsigned int max){
	count=0;
	maxcount=max;
	vstack.push_back(current_system);
	visited[current_system]=true;
	which=0;
	
}
bool NavigationSystem::SystemIterator::done ()const {
	return which>=vstack.size();
}
QVector NavigationSystem::SystemIterator::Position () {
	
	if (done())
		return QVector(0,0,0);
	string currentsystem = (**this);
	string xyz=_Universe->getGalaxyProperty(currentsystem,"xyz");
	QVector pos;
	if (xyz.size() && (sscanf(xyz.c_str(), "%lf %lf %lf", &pos.i, &pos.j, &pos.k)>=3)) {
		return pos;
	} else {
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
}
string NavigationSystem::SystemIterator::operator * () {
	if (which<vstack.size())
		return vstack[which];
	return "-";
}
NavigationSystem::SystemIterator & NavigationSystem::SystemIterator::next () {
	return ++(*this);
}
NavigationSystem::SystemIterator & NavigationSystem::SystemIterator::operator ++ () {
	which +=1;
	if (which>=vstack.size()) {
		vector <string> newsys;
		for (unsigned int i=0;i<vstack.size();++i) {
			int nas = UniverseUtil::GetNumAdjacentSystems(vstack[i]);
			for (int j=0;j<nas;++j) {
				string n = UniverseUtil::GetAdjacentSystem(vstack[i],j);
				if (!testandset(visited[n],true)) {
					string key (string("visited_")+n);
					static bool dontbothervisiting = !XMLSupport::parse_bool (vs_config->getVariable ("graphics","explore_for_map","true"));
					vector <string> * v = &_Universe->AccessCockpit()->savegame->getMissionStringData(key);
					if (dontbothervisiting||v->size()>0) {
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




void NavigationSystem::CachedSystemIterator::init (string current_system, unsigned max_systems) {
	systems.clear();
	NavigationSystem::SystemIterator iter (current_system, max_systems);
	for (;!iter.done();++iter) {
		systems.push_back(std::pair<string, QVector> (*iter, iter.Position()));
	}
}

NavigationSystem::CachedSystemIterator::CachedSystemIterator () {}

NavigationSystem::CachedSystemIterator::CachedSystemIterator (string current_system, unsigned max_systems) {
	init(current_system, max_systems);
}
NavigationSystem::CachedSystemIterator::CachedSystemIterator(const CachedSystemIterator &other)
	: systems(other.systems),
	  currentPosition(other.currentPosition) {
}

bool NavigationSystem::CachedSystemIterator::seek(unsigned position) {
	if (position<systems.size()) {
		currentPosition=position;
		return true;
	} else {
		return false;
	}
}
unsigned NavigationSystem::CachedSystemIterator::getIndex() {
	return currentPosition;
}
bool NavigationSystem::NavigationSystem::CachedSystemIterator::done ()const {
	return currentPosition>=systems.size();
}
static std::pair <string , QVector > nullPair ("-", QVector(0,0,0));
std::pair<string , QVector >& NavigationSystem::CachedSystemIterator::operator[] (unsigned pos) {
	if (done())
		return nullPair;
	return systems[pos];
}
string &NavigationSystem::CachedSystemIterator::operator* () {
	if (done())
		return nullPair.first;
	return systems[currentPosition].first;
}
QVector NavigationSystem::CachedSystemIterator::Position () {
	if (done())
		return nullPair.second;
	return systems[currentPosition].second;
}
NavigationSystem::CachedSystemIterator & NavigationSystem::CachedSystemIterator::next () {
	return ++(*this);
}
NavigationSystem::CachedSystemIterator& NavigationSystem::CachedSystemIterator::operator ++ () {
	++currentPosition;
	return *this;
}
NavigationSystem::CachedSystemIterator NavigationSystem::CachedSystemIterator::operator ++ (int) {
	NavigationSystem::CachedSystemIterator iter(*this);
	++currentPosition;
	return iter;
}






void NavigationSystem::DrawGalaxy()
{
	systemdrawset mainlist;//(0, screenoccupation, factioncolours);		//	lists of items to draw
	
	systemdrawlist mouselist;//(1, screenoccupation, factioncolours);	//	lists of items to draw that are in mouse range

	if (currentsystem.empty()) {
		setCurrentSystem(UniverseUtil::getSystemFile());
	} else {
		systemIter.seek();
	}
	
	string csector,csystem;

	Beautify (currentsystem,csector,csystem);
	//what's my name
	//***************************
	TextPlane systemname;	//	will be used to display shits names
	string systemnamestring = "Current System : " + csystem + " in the " + csector;

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

	float zdistance = 0.0;
	float zscale = 0.0;

	Adjust3dTransformation();

	pos = systemIter.Position();
	ReplaceAxes(pos);

	//Modify by old rotation amount
	//*************************
	pos = dxyz(pos, 0, ry, 0);
	pos = dxyz(pos, rx, 0, 0);
	//*************************

	float max_x = (float)pos.i;
	float min_x = (float)pos.i;
	float max_y = (float)pos.j;
	float min_y = (float)pos.j;
	float max_z = (float)pos.k;
	float min_z = (float)pos.k;
	float themaxvalue = fabs(pos.i);

	float center_nav_x = ((screenskipby4[0] + screenskipby4[1]) / 2);
	float center_nav_y = ((screenskipby4[2] + screenskipby4[3]) / 2);
	//	**********************************

	//Retrieve system data min/max
	//**********************************
	while (!systemIter.done())	//	this goes through one time to get the major components locations, and scales its output appropriately
	{
		string temp = (*systemIter);
		
		pos = systemIter.Position();
		ReplaceAxes(pos);


		//Modify by old rotation amount
		//*************************
		pos = dxyz(pos, 0, ry, 0);
		pos = dxyz(pos, rx, 0, 0);
		//*************************
		//*************************


		RecordMinAndMax(pos,min_x,max_x,min_y,max_y,min_z,max_z,themaxvalue);
			
		++systemIter;
	} 
	//**********************************


	//Find Centers
	//**********************************
	center_x = (min_x + max_x)/2;
	center_y = (min_y + max_y)/2;
	center_z = (min_z + max_z)/2;
	//**********************************

#define SQRT3 1.7320508
//	themaxvalue = sqrt(themaxvalue*themaxvalue + themaxvalue*themaxvalue + themaxvalue*themaxvalue);
	themaxvalue = SQRT3*themaxvalue;


	//Set Camera Distance
	//**********************************
	camera_z = zoom * sqrt( 
				   ( (0.5*(max_x - min_x)) * (0.5*(max_x - min_x)) ) 
				+  ( (0.5*(max_y - min_y)) * (0.5*(max_y - min_y)) )
				+  ( (0.5*(max_z - min_z)) * (0.5*(max_z - min_z)) )
				);
	//**********************************

	DrawOriginOrientationTri(center_nav_x, center_nav_y);


	//	Enlist the items and attributes
	//	**********************************
	systemIter.seek();
	while (!systemIter.done())	//	this draws the points
	{

		//	Retrieve unit data
		//	**********************************
		string temp = (*systemIter);
			


		pos = systemIter.Position();

		float the_x, the_y, system_item_scale_temp;
		TranslateAndDisplay(pos, pos_flat, center_nav_x, center_nav_y, themaxvalue, zscale, zdistance, the_x, the_y,system_item_scale_temp);


		//IGNORE OFF SCREEN
		//**********************************
		if(	!TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], the_x, the_y))
		{
			++systemIter;
			continue;
		}
		//**********************************



		int insert_type = systemambiguous;
		float insert_size = SYSTEM_DEFAULT_SIZE;
		
		if(system_item_scale_temp > (system_item_scale * 3))
		{
			system_item_scale_temp = (system_item_scale * 3);
		}


		bool moused = false;
		if (TestIfInRangeRad(the_x, the_y, insert_size*system_item_scale_temp, (-1+float(mousex)/(.5*g_game.x_resolution)), (1+float(-1*mousey)/(.5*g_game.y_resolution))) ) {
			mouselist.push_back(systemdrawnode(insert_type, insert_size*system_item_scale_temp, the_x, the_y, (*systemIter),screenoccupation,false));
			moused=true;
		}
			
		mainlist.insert(systemdrawnode(insert_type, insert_size*system_item_scale_temp, the_x, the_y, (*systemIter),screenoccupation,moused));



		++systemIter;

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
				setCurrentSystem(systemselection);
			}
		}
	}
	//	**********************************



	//	Clear the lists
	//	**********************************
	{
	for (systemdrawlist::iterator it = mouselist.begin();it!=mouselist.end();++it) {
		(*it).draw();
		
	}
	}
	
	mouselist.clear();	//	whipe mouse over'd list
	//	**********************************
}
//	**********************************









