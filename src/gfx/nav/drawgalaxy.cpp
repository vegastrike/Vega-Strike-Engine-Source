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
#include "hashtable.h"


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
	if (sector.size())
		sector[0]=toupper(sector[0]);
	if (system.size())
		system[0]=toupper(system[0]);
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
	std::vector<std::string> *dest; //let's just hope that the iterator doesn't get killed during the frame, which shouldn't happen.
	bool moused;
	char color;
	navscreenoccupied * screenoccupation;
	systemdrawnode(int type,float size,float x, float y, std::string source,navscreenoccupied * so, bool moused, std::vector<std::string> *dest):type(type),size(size),x(x),y(y),source(source),dest(dest) {
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
	systemdrawnode () : dest(NULL){
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

	void draw(bool mouseover=false, bool willclick=false)const {
		if (moused)
			return;
		if (willclick==true&&mouseover==false) {
			// Perhaps some key binding or mouseclick will be set in the future to do this.
			mouseover=true;
		}
		GFXColor race(GetColor (source));


		static bool inited=false;
		static GFXColor highlighted_tail_col;
		static GFXColor highlighted_tail_text;
		if (!inited) {
			float col1[4]={1,.3,.3,.8};
			vs_config->getColor("nav", "highlighted_unit_on_tail", col1, true);
			highlighted_tail_col=GFXColor(col1[0],col1[1],col1[2],col1[3]);

			float col2[4]={1,1,.7,1};
			vs_config->getColor("nav", "highlighted_text_on_tail", col2, true);
			highlighted_tail_text=GFXColor(col2[0],col2[1],col2[2],col2[3]);
			inited=true;
		}


		if (color=='m') {
			race.r=.5;
			race.g=.5;
			race.b=.5;
			race.a=.5;
				
		}
		if (mouseover) {
			if (willclick) {
				race=highlighted_tail_col;
			} else {
				// Leave just a faint resemblence of the original color,
				// but also make it look whiteish.
				race.r+=.75;
				race.g+=.75;
				race.b+=.75;
			}
		}
//		race=GFXColor (1,1,1,1);
		NavigationSystem::DrawCircle(x, y, size, race);
		if ((!mouseover)||(willclick)) {
			if (willclick) {
				race=highlighted_tail_text;
			}
			string blah,nam;
			Beautify (source,blah,nam);
			
			drawdescription (nam,x,y,1.0,1.0,0,race);
		}
	}
};

typedef set <systemdrawnode> systemdrawset;
typedef vector <systemdrawnode> systemdrawlist;
typedef Hashtable <std::string, const systemdrawnode, char [127]> systemdrawhashtable;

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
		systems.push_back(SystemInfo(*iter, iter.Position(), _Universe->getAdjacentStarSystems(*iter)));
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
unsigned NavigationSystem::CachedSystemIterator::getIndex() const{
	return currentPosition;
}
bool NavigationSystem::CachedSystemIterator::done ()const {
	return currentPosition>=systems.size();
}
static NavigationSystem::CachedSystemIterator::SystemInfo nullPair ("-", QVector(0,0,0), std::vector<std::string> ());
NavigationSystem::CachedSystemIterator::SystemInfo& NavigationSystem::CachedSystemIterator::operator[] (unsigned pos) {
	if (done())
		return nullPair;
	return systems[pos];
}
const NavigationSystem::CachedSystemIterator::SystemInfo& NavigationSystem::CachedSystemIterator::operator[] (unsigned pos) const{
	if (done())
		return nullPair;
	return systems[pos];
}
string &NavigationSystem::CachedSystemIterator::operator* () {
	if (done())
		return nullPair.name;
	return systems[currentPosition].name;
}
const string &NavigationSystem::CachedSystemIterator::operator* () const{
	if (done())
		return nullPair.name;
	return systems[currentPosition].name;
}
QVector NavigationSystem::CachedSystemIterator::Position () const{
	if (done())
		return nullPair.position;
	return systems[currentPosition].position;
}

std::vector<std::string> &NavigationSystem::CachedSystemIterator::Destinations () {
	if (done())
		return nullPair.destinations;
	return systems[currentPosition].destinations;
}

const std::vector<std::string> &NavigationSystem::CachedSystemIterator::Destinations () const {
	if (done())
		return nullPair.destinations;
	return systems[currentPosition].destinations;
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




static systemdrawhashtable jumptable;

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

//	int length = systemnamestring.size();
//	float offset = (float(length)*0.005);
	systemname.col = GFXColor(1, 1, .7, 1);
	systemname.SetPos(screenskipby4[0] , screenskipby4[3]); // Looks ugly when name is too long and goes off the edge.
//	systemname.SetPos( (((screenskipby4[0]+screenskipby4[1])/2)-offset) , screenskipby4[3]);
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
/*	min_x = (min_x+center_x)/2;
	min_y = (min_y+center_y)/2;
	min_z = (min_z+center_z)/2;
	max_x = (max_x+center_x)/2;
	max_y = (max_y+center_y)/2;
	max_z = (max_z+center_z)/2;*/
	//Set Camera Distance
	//**********************************

	{
		float half_x=(0.5*(max_x - min_x));
		float half_y=(0.5*(max_y - min_y));
		float half_z=(0.5*(max_z - min_z));
	
		camera_z = sqrt( ( half_x * half_x ) + ( half_y * half_y ) + 0*( half_z * half_z ));
	
	}

//	camera_z=  zoom * themaxvalue;

	//**********************************

	DrawOriginOrientationTri(center_nav_x, center_nav_y);

#define SQRT3 1.7320508
//	themaxvalue = sqrt(themaxvalue*themaxvalue + themaxvalue*themaxvalue + themaxvalue*themaxvalue);
	themaxvalue = SQRT3*themaxvalue;


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

		insert_size *= system_item_scale_temp;

		if (currentsystem==temp)
		{
			// Get a color from the config
			static float col[4]={1, 0.3, 0.3, 0.8};
			static bool init = false;
			if (!init) {
				vs_config->getColor("nav", "current_system", col, true);
				init=true;
			}

			DrawTargetCorners(the_x, the_y, (insert_size), GFXColor(col[0],col[1],col[2],col[3]));
		}

		bool moused = false;
		if (TestIfInRangeRad(the_x, the_y, insert_size, (-1+float(mousex)/(.5*g_game.x_resolution)), (1+float(-1*mousey)/(.5*g_game.y_resolution))) ) {
			mouselist.push_back(systemdrawnode(insert_type, insert_size, the_x, the_y, (*systemIter),screenoccupation,false,&systemIter.Destinations()));
			moused=true;
		}
			
		
		jumptable.Put(temp,&(*mainlist.insert(systemdrawnode(insert_type, insert_size, the_x, the_y, (*systemIter),screenoccupation,moused,&systemIter.Destinations())).first));



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

		std::vector <std::string>* dest=(*it).dest;
		if (dest!=NULL) {
			GFXBegin(GFXLINE);	
		
			for (int i=0;i<dest->size();++i) {
				std::string oth=(*dest)[i];
				if (sys<oth) {
					const systemdrawnode *j = jumptable.Get (oth);
					if (j!=NULL) {
						GFXColorf (GetColor (sys));
						GFXVertex3f((*it).x,(*it).y,0);
						GFXColorf (GetColor(oth));
						GFXVertex3f((*j).x,(*j).y,0);
//					} else {
//						// Happens whenever there's a system not drawn, which is every frame, so printing this is useless.
//						printf("Warning: System %s in %s's jump list not found.\n",oth,sys);
					}
					
				}
			}
			GFXEnd();
//			(*it).dest=NULL; // just in case, since it will die later on.
			// GCC won't let me set it to NULL for some silly reason... so I'll just leave it as is...
		}
		jumptable.Delete(sys); // Won't ever reference this again, since it checks for less than.
	   
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
		(*it).draw(true, &(*it)==&mouselist.back());
		
	}
	}
	
	mouselist.clear();	//	whipe mouse over'd list
	//	**********************************
}
//	**********************************









