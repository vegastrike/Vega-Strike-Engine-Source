#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <set>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <algorithm>
#ifndef _WIN32
#include <sys/stat.h>
#endif
#include <sys/stat.h>
using std::vector;
using std::map;
using std::string;
using std::pair;

std::string readfiledata(const char * name) {
	FILE * fp = fopen (name,"r");
	if (!fp) {
		printf("unable to open %s\n",name);
		return "";
	}
	int len;
	struct stat st;
	if (fstat(fileno(fp), &st)==0) {
		len=st.st_size;
	} else {
		// fstat B0rken.
		fseek (fp,0,SEEK_END);
		len = ftell (fp);
		fseek (fp,0,SEEK_SET);
	}
	char * line = (char *)malloc (len+1);
	line[len]=0;
	fread(line,sizeof(char),len,fp);
	return std::string(line,line+len);
}

double sqr (double x){
	return x*x;
}

std::string itostr(int i) {
	char test[256];
	sprintf (test,"%d",i);
	return test;
}
std::string ftostr(double i) {
	char test[256];
	sprintf (test,"%lf",i);
	return test;
}
char * milky_way="../../../data/universe/milky_way.xml";
class vec3 {
public:
	double x,y,z;
	vec3 () {}
	vec3 (double x, double  y, double z):x(x),y(y),z(z){}
	vec3 max (const vec3 &in) const{
		return vec3(in.x>x?in.x:x,
					in.y>y?in.y:y,
					in.z>z?in.z:z);
	}
	vec3 min (const vec3 &in) const{
		return vec3(in.x<x?in.x:x,
					in.y<y?in.y:y,
					in.z<z?in.z:z);
	}
	bool operator< (const vec3 &oth) const {
		return x<oth.x&&y<oth.y&&z<oth.z;
	}
	bool operator> (const vec3 &oth) const {
		return x>oth.x&&y>oth.y&&z>oth.z;
	}
	bool operator<= (const vec3 &oth) const {
		return x<=oth.x&&y<=oth.y&&z<=oth.z;
	}
	bool operator>= (const vec3 &oth) const {
		return x>=oth.x&&y>=oth.y&&z>=oth.z;
	}
	bool operator== (const vec3 &oth) const {
		return x==oth.x&&y==oth.y&&z==oth.z;
	}
	bool operator!= (const vec3 &oth) const {
		return x!=oth.x||y!=oth.y||z!=oth.z;
	}
};

class System: public map <string,string> {
public:
	static System* findSystem(vector<System> &s, string outgoing) {
		int slash = outgoing.find("/");
		if (slash==std::string::npos) {
			std::vector<System*> habitableSystems;
			for (unsigned int j=0;j<s.size();++j) {
				if (s[j].sector==outgoing) {
					if (s[j].habitable) {
						habitableSystems.push_back(&s[j]);
					}
				}
			}
			if (habitableSystems.empty()) {
				fprintf(stderr, "Fatal Error: No habitable systems in the %s sector!!!\n",outgoing.c_str());
				return NULL;
			} else {
				return habitableSystems[(int)((((float)rand())/RAND_MAX)*habitableSystems.size())];
			}
		} else {
			string sys=outgoing;
			sys = outgoing.substr(slash);
			System *bestChoice=NULL;
			if (sys.length())
				if (sys[0]=='/')
					sys = sys.substr(1);
			outgoing = outgoing.substr(0,slash);
			for (unsigned int j=0;j<s.size();++j) {
				if (s[j].name==sys) {
					if (s[j].sector!=outgoing) {
						fprintf (stderr,"Error: System %s not in %s but in %s\n",sys.c_str(),outgoing.c_str(),s[j].sector.c_str());
						if (bestChoice==NULL) {
							bestChoice=&s[j];
						} else if (bestChoice!=&s[j]) {
							fprintf(stderr," -- Duplicate ambiguous system name... Some systems may have invalid jump points!!!!\n");
						}
					}else {
						return &s[j];
					}
				}
			}
			return bestChoice;
		}
	}
	System () {}
	bool habitable;
	bool interesting;
	string sector;
	string name;
	string alphaonlyname;
	float distance;
	float ascension;
	float declination;
	vector<string> jumps;
	vec3 xyz;
	float luminosity;//in sun
	int type; // 0 = 1O blue (-2000)  B = 20 blue(2000-4000) // A = 30 Bluish-white (4000-8000) F = 40 White G = 50 (13500-15000) yellow  (15000-43000) K = 60 Orange (36500-80000) M = Red 70 (giant 80,000 dwarf 8,000-13500 )
	int  size;// 0 dwwarf 1 normal 2 giant
	bool operator < (const System & a) const{
		if (sector==a.sector)
			return name < a.name;
		return sector<a.sector;
	}
	string fullName () {
		return sector+"/"+name;
	}
	void computeProperties(bool interestingname,const vector<string> homeworlds) {
		this->interesting=interestingname;
		double rad=16000;
		double lifeprob= .25;
		if (type<30) {
			rad=type*4000./30;
			lifeprob=.01;
		}else if (type < 40) {
			rad = 6000;
			lifeprob=.02;
			if (size==0)
				rad = 4200;
			else if (size==2)
				rad = 7500;
		}else if (type < 50) {
			lifeprob=.05;
			rad = 14250;
			if (size==0){
				rad= 13600;
				lifeprob=.08;
			}else if (size==2)
				rad= 14750;
		}else if (type < 60) {
			lifeprob = .125;
			rad = 25000;
			if (size==0){
				rad= 16600;
				lifeprob = .25;
			}
			else if (size==2) {
				rad= 36500;
				lifeprob = .0625;
			}
		}else if (type < 70) {
			rad = 50000;
			lifeprob = .02;
			if (size==0) {
				lifeprob = .125;
				rad = 37000;
			}else if (size==2)
				rad = 75000;			
		}else if (type < 80) {
			rad = 85000;
			lifeprob = .005;
			if (size==0){
				rad =10000;
				lifeprob = .125;
			}else if (size==2)
				rad = 150000;
		}
		if (interesting) {
			lifeprob *=1;//the answer to the question...
		}else {
			lifeprob/=1;
		}
		(*this)["sun_radius"]=ftostr(rad);
		(*this)["data"]=itostr(rand());
		(*this)["faction"]="unknown";
		lifeprob*=1;
		habitable=false;
		for (int i=0;i<homeworlds.size();i++) {
			if (homeworlds[i]==name) {
				//All homeworlds have life!
				//(Or else I would not exist or be able to write this program)
				lifeprob=1;
				break;
			}
		}
		if (rand()<RAND_MAX*lifeprob) {
			habitable=true;
			//living
			if (rand()<RAND_MAX*.995) {
				(*this)["num_gas_giants"]= itostr(rand()%3);
			}else {
				(*this)["num_gas_giants"]= itostr(rand()%6);
			}
			if (rand()<RAND_MAX*.995) {
				(*this)["num_planets"]= itostr(rand()%3);
			}else {
				(*this)["num_planets"]= itostr(rand()%9);
			}
			if (rand()<RAND_MAX*.995) {
				(*this)["num_moons"]= itostr(rand()%3);
			}else {
				(*this)["num_moons"]= itostr(rand()%18);
			}
			if (rand()<RAND_MAX*.025) {
				(*this)["num_natural_phenomena"]= "2";
			}else if (rand()<RAND_MAX*.1) {
				(*this)["num_natural_phenomena"]= "1";
			}else {
				(*this)["num_natural_phenomena"]= "0";
			}
		}else {
//dead(
			if (rand()<RAND_MAX*.1)
				(*this)["num_natural_phenomena"]= "1";
			else
				(*this)["num_natural_phenomena"]= "0";
			(*this)["num_moons"]="0";
			if (rand()<RAND_MAX*.85) {
				(*this)["planetlist"]="planets.desolate.txt";
				(*this)["num_planets"]=itostr(rand()%2+1);
				(*this)["num_gas_giants"]="0";
			}else {
				(*this)["num_planets"]=itostr(0);
				(*this)["num_gas_giants"]=itostr(rand()%2+1);
				if (rand()>RAND_MAX*.99)
					(*this)["num_moons"]=itostr(rand()%5);
				
			}
			
		}
	}
	void computeXYZ() {
		xyz.z = distance * sin (declination);
		float xy = distance * cos (declination);
		xyz.y = xy*sin(ascension);
		xyz.x = xy*cos(ascension);
		char str[16384];
		sprintf (str,"%lf %lf %lf",xyz.x,xyz.y,xyz.z);
		(*this)["xyz"] = str;
	}
	
};
vector <std::string> readCSV (std::string s) {
	vector <std::string> v;int loc;
	int sub1=s.find ("\r");
	s=s.substr(0,sub1);
	int sub2=s.find("\n");
	s=s.substr(0,sub2);
	do {
		loc=s.find (",");
		string t =s.substr (0,loc);
		v.push_back(t);
		if (loc!=string::npos)
			s = s.substr (loc+1);
	} while (loc!=string::npos);
	return v;
}
void computeminmax(vector<System> sys ,vec3 & min, vec3 & max) {
	min = vec3(DBL_MAX,DBL_MAX,DBL_MAX);
	max = vec3(-DBL_MAX,-DBL_MAX,-DBL_MAX);
	for (unsigned int i=0;i<sys.size();++i) {
		min = min.min(sys[i].xyz);
		max = max.max(sys[i].xyz);
	}
}
std::string strtoupper (std::string s) {
	for (string::iterator i = s.begin() ; i!=s.end();++i ) {
		*i = toupper(*i);
	}
	return s;
}
std::string unpretty (std::string s) {
	for (string::iterator i = s.begin() ; i!=s.end();++i ) {
		if (isspace(*i)) {
			*i = '_';
		}
		if (*i=='\"' || *i ==  '\'')
			*i = '_';
	}
	return s;
	
}
std::string alphaOnly (std::string s){
	std::string out=s;
	int count=0;
	for (string::iterator i = s.begin();i!=s.end();++i) {
		if ((*i>'a'&&*i<'z')||(*i>'A'&&*i<'Z')) {
			out[count]=toupper(*i);
			++count;
		}
	}
	return out.substr(0,count);
}
class AlphaOnlySort{
public:
	bool operator() (const System &a, const System & b) {
		return a.alphaonlyname<b.alphaonlyname;
	}
};

std::vector<string> readMilkyWayNames( ) {
	string s=readfiledata(milky_way);
	vector<string> retval;
	unsigned int where=string::npos;
	do{
		where = s.find("system name=\"");
		if (where!=string::npos) {
			s= s.substr(where);
			where = s.find("\"");
			s= s.substr(where+1);//gotta be there cus we found it earlier
			unsigned int quote=  s.find("\"");
			if (quote!=string::npos) {
				string newname=s.substr(0,quote);
				if (newname!="max"&&newname!="min"&&newname!="maxlimit"&&newname!="minlimit"&&newname!="hardwicke"&&newname!="reid"&&newname!="lesnick"&&newname!="midgard"&&newname.find("blockade")==string::npos&& newname!="wolf359"&&newname.find("wolf")==string::npos)
					retval.push_back(newname);
			}
		}
	}while (where!=string::npos);
	return retval;
}

class FactionInfo {
	unsigned maxsystems;
	float takeoverprob; // The chace that an enemy system will be taken over.
	float takeneutralprob; // The chance that a neutral system will be taken over.
	string name;
	System *homeworld;
	
	unsigned turn;
	unsigned numsystems;
	std::vector<System*> borderSystems;
	std::vector<System*> developingSystems;
	std::set<System*> systems; // for quick access.
	
public:
	System *getHomeworld() {
		return homeworld;
	}
	const System *getHomeworld() const{
		return homeworld;
	}
	void developBorderSystems() {
		// reserve memory to increse speed.
		developingSystems.reserve(developingSystems.size()+borderSystems.size());
		for (int i=borderSystems.size()-1;i>=0;i--) {
			developingSystems.push_back(borderSystems[i]);
//			borderSystems.erase(borderSystems.begin()+i);
		}
		borderSystems.clear();
	}
	void addNewSystems(const vector<System*> &newSystems) {
		developBorderSystems();
		borderSystems=newSystems;
		for (int i=0;i<borderSystems.size();i++) {
			systems.insert(borderSystems[i]);
		}
		numsystems+=newSystems.size();
	}
	/*
	FactionInfo(const FactionInfo &other)
			: turn(other.turn), numsystems(other.numsystems), name(other.name), takeoverprob(other.takeoverprob),
			  takeneutralprob(other.takeneutralprob), maxsystems(other.maxsystems),
			  homeworld(other.homeworld), systems(other.systems), developingSystems(other.developingSystems),
			  borderSystems(other.borderSystems) {
	}
	*/
	FactionInfo(vector<string> stuff, vector<System> &s)
			: turn(0), numsystems(1), name(stuff[0]), takeoverprob(atof(stuff[1].c_str())),
			  takeneutralprob(1-takeoverprob), maxsystems(atoi(stuff[2].c_str())),
			  homeworld(System::findSystem(s,stuff[3])) {
		if (!homeworld) {
			fprintf(stderr,"Fatal error: homeworld \"%s\" not found!!!\n",stuff[3].c_str());
		} else if (homeworld->jumps.empty()) {
			fprintf(stderr,"Fatal error: homeworld \"%s\" has no jump points!!!\nThis means that the %s faction will wait forever for a jump point\nto come into existance.  The application will probably get stuck in an endless loop somewhere!",stuff[3].c_str(),name.c_str());
		}
		std::vector<System*> newsys;
		newsys.push_back(homeworld);
		addNewSystems(newsys);
	}
	FactionInfo(string nam, float prob, int max, System *homeworld)
			: turn(0), numsystems(1), name(nam), takeoverprob(prob),
			  takeneutralprob(1-takeoverprob), maxsystems(max), homeworld(homeworld) {
	}
	/*
	FactionInfo &operator= (const FactionInfo &other) {
		turn=(other.turn);
		numsystems=(other.numsystems);
		name=(other.name);
		takeoverprob=(other.takeoverprob);
		takeneutralprob=(other.takeneutralprob);
		maxsystems=(other.maxsystems);
		homeworld=(other.homeworld);
		systems=(other.systems);
		developingSystems=(other.developingSystems);
		borderSystems=(other.borderSystems);
		return *this;
	}
	*/
	void simulateTurn (unsigned int totalturn, bool allowTakeover, vector<System> &s) {
		++turn;
		vector<System*> systemsToAdd;
		if (borderSystems.empty()) {
			numsystems=maxsystems;
		}
		for (int i=0;i<borderSystems.size();i++) {
			std::vector<std::string>::const_iterator end=borderSystems[i]->jumps.end();
			for (std::vector<std::string>::const_iterator iter=borderSystems[i]->jumps.begin();iter!=end;++iter) {
				System *jump=System::findSystem(s,*iter);
				if (jump!=NULL&&systems.find(jump)==systems.end()) {
					// not in our territory! and it is valid.
					if (((*jump)["faction"]=="unknown"
							&& (((float)rand())/RAND_MAX)<takeneutralprob)
							|| ((((float)rand())/RAND_MAX)<takeoverprob&&allowTakeover)) {
						(*jump)["faction"]=name;
						systemsToAdd.push_back(jump);
					}
				}
			}
		}
		addNewSystems(systemsToAdd);
	}
	bool active() {
		return (numsystems<maxsystems);
	}
};

std::vector<FactionInfo> readFactions(vector<System> &s) {
	std::vector<FactionInfo> ret;
	std::string file = readfiledata("factions.csv");
	while (true) {
		vector<string> temp=readCSV(file);
		int r=file.find("\r");
		int n=file.find("\n");
		if (r==std::string::npos&&n==std::string::npos) {
			break;
		}
		file=file.substr(r>n?r+1:n+1);
		if (temp.size()) {
			if (temp.size()<4) {
				if (temp.size()<3) {
					if (temp.size()<2) {
						temp.push_back(".1");
					}
					temp.push_back("10");
				}
				temp.push_back("You are getting this error due to lack of required columns.");
			}
			ret.push_back(FactionInfo(temp, s));
		}
	}
	return ret;
}

void simulateFactionTurns (vector<System> &s) {
	std::vector<FactionInfo> factions=readFactions(s);
	unsigned turn;
	for (turn=0;;turn++) {
		int num_inactive=0;
		for (unsigned i=0;i<factions.size();i++) {
			if (factions[i].active()) {
				factions[i].simulateTurn(turn, false, s);
			} else {
				num_inactive++;
			}
		}
		if (num_inactive>=factions.size())
			break;
	}
	for (unsigned i=0;i<factions.size();i++) {
		factions[i].simulateTurn(turn, true, s);
	}
}

class SectorInfo {
private:
	std::string nam;
	vec3 min;
	vec3 max;
public:
	SectorInfo (std::string nam, vec3 min, vec3 max)
			: nam(nam), min(min), max(max) {
	}
	vec3 &minimum () {
		return min;
	}
	vec3 &maximum () {
		return max;
	}
	std::string &name () {
		return nam;
	}
};

void stripwhitespace(std::string &str) {
	unsigned int i;
	// Strip whitespace from a string.
	for (i=str.size()-1;i>=0;i--) {
		if (isspace(str[i])) {
			str.erase(i);
		} else {
			break;
		}
	}
	for (i=0;i<str.size();i++) {
		if (isspace(str[i])) {
			str.erase(i);
		} else {
			break;
		}
	}
}

void computeCoord (vec3 &lo, vec3 &hi, int wid, int hei, int x, int y) {
	vec3 min (lo);
	vec3 max (hi);
//	lo.z=min.z; // Already equal!
//	hi.z=max.z;
	lo.x=(((max.x-min.x)/wid)*x)+min.x;
	lo.y=(((max.y-min.y)/hei)*y)+min.y;
	hi.x=(((max.x-min.x)/wid)*(x+1))+min.x;
	hi.y=(((max.y-min.y)/hei)*(y+1))+min.y;
}

std::vector<SectorInfo> readSectors(vec3 min, vec3 max) {
	// Uncomment if you have problems.
//	printf("\n\n--MINIMUM-- <%f,%f,%f>\n--MAXIMUM-- <%f,%f,%f>\n\n",min.x,min.y,min.z,max.x,max.y,max.z);
	std::vector<SectorInfo> ret;
	std::string file = readfiledata("sectors.txt");
	std::vector<std::vector<std::string> >vec;
	int width=0;
	while (true) {
		vec.push_back(readCSV(file));
		int r=file.find("\r");
		int n=file.find("\n");
		if (r==std::string::npos&&n==std::string::npos) {
			break;
		}
		file=file.substr(r>n?r+1:n+1);
		width=width>vec.back().size()?width:vec.back().size();
	}
	for (int i=0;i<vec.size();++i) {
		for (int j=0;j<vec[i].size();++j) {
			stripwhitespace(vec[i][j]);
			vec3 lo (min);
			vec3 hi (max);
			computeCoord(lo,hi,width,vec.size(),j,i);
			ret.push_back(SectorInfo(vec[i][j], lo, hi));
		}
	}
	return ret;
}

std::string getSector(const System &s, vec3 min, vec3 max) {
	static std::vector<SectorInfo> sectors (readSectors(min,max));
	for (int i=0;i<sectors.size();i++) {
		if (s.xyz>=sectors[i].minimum()&&s.xyz<=sectors[i].maximum()) {
			return sectors[i].name();
		}
	}
	{
		char error[65535];
		sprintf(error,"<error:_system_<%f,%f,%f>_(%s)_not_found>",s.xyz.x,s.xyz.y,s.xyz.z,s.name.c_str());
		fputs(error,stderr);
		fputc('\n',stderr);
		return error;
	}
}

string dodad (int which) {
	if (which==0)
		return "";
	return "_"+itostr(which);
}
vector<string> shuffle (const vector<string> & inp) {
	vector<string> retval;
	for (unsigned int i=0;i<inp.size();++i) {
		int index = rand()%(retval.size()+1);
		if (std::find(retval.begin(),retval.end(),inp[i])==retval.end()) {
			retval.insert(retval.begin()+index,inp[i]);
		}
	}
	return retval;
}
string recomputeName(){
	static int which=-1;
	static std::vector<string> genericnames=shuffle(readMilkyWayNames());
	which++;
	return genericnames[(which)%genericnames.size()]+dodad(which/genericnames.size());
}

vector<System> readfile (const char * name) {
	vector<System>systems;
	std::string line=readfiledata(name);
	if (line.empty()){
		return systems;
	}
	int len=line.size();
	std::vector<string> keys = readCSV(line);
	for (std::vector<string>::iterator i =keys.begin();i!=keys.end();++i) {
		*i = strtoupper(*i);
	}
	while(true) {
		int n=line.find("\n");
		if (n==std::string::npos) {
			break;
		}
		line=line.substr(n+1);
		
		System in;
		std::vector <string> content = readCSV(line);
		for (int i = 0;i<content.size();++i) {
			if (keys[i].find("DISTANCE")!=string::npos) {
				in.distance = atof (content[i].c_str());
			}else if (keys[i].find("ASCENSION")!=string::npos) {
				in.ascension = atof(content[i].c_str());
			}else if (keys[i].find("DECLINATION")!=string::npos) { 
				in.declination = atof (content[i].c_str());
			}else if (keys[i].find("LUMIN")!=string::npos) {
				in["luminosity"]=content[i];  				
				in.luminosity = atof(content[i].c_str());
			}else if (keys[i].find("SPECTRUM")!=string::npos||keys[i].find("TYPE")!=string::npos) {
				in.type = atoi(content[i].c_str());
			}else if (keys[i].find("SIZE")!=string::npos) {
				in.size = atoi(content[i].c_str());
			}else {
				if (keys[i].find("NAME")!=string::npos) {
					in.name = unpretty(content[i]);
				}else {
					printf ( "error key %s not found\n",keys[i].c_str());
					in[keys[i]] = content[i];
				}
			   
			}
/*			
			if (in.name=="") {
				static int num=0;
				num++;
				in.name=string("Daniel")+itostr(num);
			}
*/
			in.alphaonlyname = alphaOnly(in.name);
			in.sector="nowhereland";

		}

		in.computeXYZ();
		systems.push_back(in);
		
	}
	std::sort (systems.begin(),systems.end(),AlphaOnlySort());
	vec3 min,max;
	computeminmax(systems,min,max);
	for (unsigned index=0;index<systems.size();++index) {
		systems[index].sector=getSector(systems[index],min,max);
	}
	int hc=0,bc=0;
	int ic=0,uc=0;
	int ec=0;
	if (1) {
		std::vector<string> homeworlds;
		{
			std::string factionsdata = readfiledata("factions.csv");
			while (true) {
				std::vector<std::string> vec=(readCSV(factionsdata));
				int n=factionsdata.find("\n");
				if (n==std::string::npos) {
					break;
				}
				factionsdata=factionsdata.substr(n+1);
				if (vec.size()>=4) {
					int slash=vec[3].find("/");
					if (slash!=std::string::npos) {
						homeworlds.push_back(vec[3].substr(slash+1));
					}
				}
			}
		}
		for (unsigned int i= 0;i<systems.size();++i) {
			if (systems[i].name.length()==0){
				systems[i].interesting=false;
				static int num=0;
				num++;
				systems[i].name=string("Daniel")+itostr(num);
				ec++;
			}else if (i<4) {
				systems[i].interesting=true;
			}else if (i>systems.size()-4) {
				systems[i].interesting=true;
			}else {
				//now we have a buffer zone of 4!
				//engage!
				if (systems[i].alphaonlyname==systems[i-1].alphaonlyname&&
					systems[i].alphaonlyname==systems[i+1].alphaonlyname&&
					systems[i].alphaonlyname==systems[i+2].alphaonlyname&&
					systems[i].alphaonlyname==systems[i-2].alphaonlyname) {
					systems[i].interesting=false;
					uc++;
				}else {
					systems[i].interesting=true;
				}
			}
			
			if (systems[i].interesting) {
				ic++;
				//printf("%s interesting\n",systems[i].name.c_str());
			}else {
				//printf("%s no\n",systems[i].name.c_str());
			}
			systems[i].computeProperties(systems[i].interesting, homeworlds);
			if (systems[i].habitable) {
				hc +=1;
				if (systems[i].interesting==false) {
					systems[i].name = recomputeName();
				}
			}else {
				bc+=1;
			}
		}
	}
	fprintf (stderr,"\n\nParsing has been completed.\nHabitable count: %d\nBarren count: %d\n\nInteresting count: %d\nUninteresting count: %d\nEmpty count: %d\n\nGenerating data...\n\n",hc,bc,ic,uc,ec);
	return systems;
}
	

void writesystems(FILE * fp, std::vector<System> s) {
	std::sort(s.begin(),s.end());//sort into sector categories
	string cursector;
	fprintf(fp,"<galaxy><systems>\n");
	int iter=0;
	for (std::vector<System>::iterator i = s.begin();i!=s.end();++i) {
		if ((*i).sector != cursector) {
 			//start sectortag;
			if (cursector!="")
					fprintf(fp,"\t</sector>\n");
			fprintf (fp, "\t<sector name=\"%s\">\n",(*i).sector.c_str());
			cursector = (*i).sector;
		}
		fprintf(fp,"\t\t<sector name=\"%s\">\n",(*i).name.c_str());
		for (std::map<string,string>::iterator j = (*i).begin();j!=(*i).end();++j) {
			fprintf (fp, "\t\t\t<var name=\"%s\" value=\"%s\"/>\n",(*j).first.c_str(),(*j).second.c_str());			
		}
		fprintf (fp,"\t\t\t<var name=\"jumps\" value=\"");
		if ((*i).jumps.size()) {
			fprintf(fp,"%s",(*i).jumps[0].c_str());
			for (unsigned int k=1;k<(*i).jumps.size();++k) {
				fprintf (fp," %s",(*i).jumps[k].c_str());
			}
		}
		fprintf (fp,"\"/>\n");
/*		if (iter>4 && iter+4<s.size()) {
			fprintf (fp,"\t\t\t<var name=\"jumps\" value=\"nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s\"/>\n",s[iter-1].name.c_str(),s[iter-2].name.c_str(),s[iter-3].name.c_str(),s[iter-4].name.c_str(),s[iter+1].name.c_str(),s[iter+2].name.c_str(),s[iter+3].name.c_str());
			}*/
		fprintf(fp,"\t\t</sector>\n");
		iter++;
	}	
	fprintf(fp,"\t</sector>\n</systems></galaxy>\n");
	
}
void processsystems (std::vector <System> & s){
	vec3 min,max;
	computeminmax(s,min,max);
	unsigned int i;
	for (i=0;i<s.size();++i) {
		std::map <double,string> jumps;
		if (s[i].habitable)
		for (unsigned int j=0;j<s.size();++j) {
			if (j!=i && (s[j].habitable||rand()<RAND_MAX*.001)){
				float dissqr = sqr(s[i].xyz.x-s[j].xyz.x)+sqr(s[i].xyz.y-s[j].xyz.y)+sqr(s[i].xyz.z-s[j].xyz.z);
				int desired_size = rand()%5+1;
				if (jumps.size()>=desired_size) {
					if (jumps.upper_bound(dissqr)!=jumps.end() && rand()<RAND_MAX*.995) {
						jumps[dissqr]=s[j].fullName();
						std::map<double,string>::iterator k = jumps.end();
						k--;
						jumps.erase(k);//erase last one
					}
				}else {
					jumps[dissqr]=s[j].fullName();
				}
			}
		}
		string j;
		if (jumps.size()) {
			std::map<double,string>::iterator k= jumps.begin();
			j=(*k).second;			
			++k;
			s[i].jumps.push_back(j);
			for (;k!=jumps.end();++k) {
				j+=string(" ")+(*k).second;
				s[i].jumps.push_back((*k).second);
			}
		}
//		s[i]["jumps"]=j;
	}
	for (i=0;i<s.size();++i) {
		if (s[i].habitable){
			unsigned int jsize = s[i].jumps.size();
			for (unsigned int k=0;k<jsize;++k) {
				string outgoing = s[i].jumps[k];
				System *foundsys=System::findSystem(s, outgoing);
				if (foundsys!=NULL) {
					string fullname = s[i].sector+"/"+s[i].name;
					unsigned int jjsize = foundsys->jumps.size();
					bool found=false;
					if (!foundsys->habitable){
						//fprintf (stderr,"looking for %s in %s\n",fullname.c_str(),s[j].name.c_str());
					}
					for (unsigned int l=0;l<jjsize;++l) {
						if (fullname==foundsys->jumps[l]) {
							found=true;
							break;
						}
					}
					if (!found){
						/*
						  if (s[j].jumps.empty())
						    s[j]["jumps"]=fullname;
						  else
						    s[j]["jumps"]=s[j]["jumps"]+" "+fullname;
						*/
						foundsys->jumps.push_back(fullname);
					}
				}
			}
		}
	}
	simulateFactionTurns(s); // Simulates the factions starting with one homeworld, and expands their territories.x
}
int main (int argc, char ** argv) {
	if (argc<3) {
		printf ("not enough args. Usage %s <input> <output>\n",argv[0]);
		return 1;
	}
	if (argc>3) {
		milky_way = argv[3];
	}
	srand(109427);
	std::vector <System> s=readfile(argv[1]);
	processsystems(s);
	FILE * fp = fopen (argv[2],"w");
	if (fp){
		writesystems(fp,s);
		fclose(fp);
	}else
		printf ("could not open %s for writing\n",argv[2]);
	return 0;
}
