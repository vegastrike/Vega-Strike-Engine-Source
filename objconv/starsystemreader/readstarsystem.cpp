#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <algorithm>
using std::vector;
using std::map;
using std::string;
using std::pair;
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

class vec3 {
public:
	double x,y,z;
	vec3 () {}
	vec3 (double x, double  y, double z):x(x),y(y),z(z){}
	vec3 max (const vec3 &in) {
		return vec3(in.x>x?in.x:x,
					in.y>y?in.y:y,
					in.z>z?in.z:z);
	}
	vec3 min (const vec3 &in) {
		return vec3(in.x<x?in.x:x,
					in.y<y?in.y:y,
					in.z<z?in.z:z);
	}
};

class System: public map <string,string> {
public:
	System () {}
	bool habitable;
	string sector;
	string name;
	float distance;
	float ascension;
	float declination;
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
	void computeProperties() {
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
		(*this)["sun_radius"]=ftostr(rad);
		(*this)["data"]=itostr(rand());
		lifeprob*=1;
		habitable=false;
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

vector<System> readfile (const char * name) {
	vector<System>systems;
	FILE * fp = fopen (name,"r");
	if (!fp) {
		printf("unable to open %s\n",name);
		return systems;
	}
	fseek (fp,0,SEEK_END);
	int len = ftell (fp);
	fseek (fp,0,SEEK_SET);
	char * line = (char *)malloc (len+1);
	line[len]=0;
	fgets(line,len,fp);
	vector<string> keys = readCSV(line);
	for (vector<string>::iterator i =keys.begin();i!=keys.end();++i) {
		*i = strtoupper(*i);
	}
	while(fgets (line,len,fp)) {
		System in;
		vector <string> content = readCSV(line);
		for (int i = 0;i<content.size();++i) {
			if (keys[i].find("DISTANCE")!=string::npos) {
				in.distance = atof (content[i].c_str());
			}else if (keys[i].find("ASCENSION")!=string::npos) {
				in.ascension = atof(content[i].c_str());
			}else if (keys[i].find("DECLINATION")!=string::npos) { 
				in.declination = atof (content[i].c_str());
			}else if (keys[i].find("LUMIN")!=string::npos) {
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
			if (in.name=="") {
				static int num=0;
				num++;
				in.name=string("DAN")+itostr(num);
				
			}
			in.sector="nowhereland";
		}
		in.computeXYZ();
		in.computeProperties();
		systems.push_back(in);
	}
	
	free(line);
	return systems;
}


void writesystems(FILE * fp, vector<System> s) {
	std::sort(s.begin(),s.end());//sort into sector categories
	string cursector;
	fprintf(fp,"<galaxy><systems>\n");
	int iter=0;
	for (vector<System>::iterator i = s.begin();i!=s.end();++i) {
		if ((*i).sector != cursector) {
 			//start sectortag;
			if (cursector!="")
					fprintf(fp,"\t</sector>\n");
			fprintf (fp, "\t<sector name=\"%s\">\n",(*i).sector.c_str());
			cursector = (*i).sector;
		}
		fprintf(fp,"\t\t<sector name=\"%s\">\n",(*i).name.c_str());
		for (map<string,string>::iterator j = (*i).begin();j!=(*i).end();++j) {
			fprintf (fp, "\t\t\t<var name=\"%s\" value=\"%s\"/>\n",(*j).first.c_str(),(*j).second.c_str());			
		}
		fprintf (fp,"\t\t\t<var name=\"faction\" value=\"confed\"/>\n");
/*		if (iter>4 && iter+4<s.size()) {
			fprintf (fp,"\t\t\t<var name=\"jumps\" value=\"nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s\"/>\n",s[iter-1].name.c_str(),s[iter-2].name.c_str(),s[iter-3].name.c_str(),s[iter-4].name.c_str(),s[iter+1].name.c_str(),s[iter+2].name.c_str(),s[iter+3].name.c_str());
			}*/
		fprintf(fp,"\t\t</sector>\n");
		iter++;
	}	
	fprintf(fp,"\t</sector>\n</systems></galaxy>\n");
	
}
string getSector(const System &s, vec3 min, vec3 max) {
	return "nowhereland";
}
double sqr (double x){
	return x*x;
}
void processsystems (vector <System> & s){
	vec3 min,max;
	computeminmax(s,min,max);
	for (unsigned int i=0;i<s.size();++i) {
		s[i].sector=getSector(s[i],min,max);
		map <double,string> jumps;
		if (s[i].habitable)
		for (unsigned int j=0;j<s.size();++j) {
			if (j!=i && (s[j].habitable||rand()<RAND_MAX*.001)){
				float dissqr = sqr(s[i].xyz.x-s[j].xyz.x)+sqr(s[i].xyz.y-s[j].xyz.y)+sqr(s[i].xyz.z-s[j].xyz.z);
				int desired_size = rand()%7+1;
				if (jumps.size()>=desired_size) {
					if (jumps.upper_bound(dissqr)!=jumps.end() && rand()<RAND_MAX*.995) {
						jumps[dissqr]=s[j].fullName();
						map<double,string>::iterator k = jumps.end();
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
			map<double,string>::iterator k= jumps.begin();
			j=(*k).second;			
			++k;
			for (;k!=jumps.end();++k) {
				j+=string(" ")+(*k).second;
			}
		}
		s[i]["jumps"]=j;
	}
}
int main (int argc, char ** argv) {
	if (argc<3) {
		printf ("not enough args. Usage ./a.out <input> <output>\n");
		return 1;
	}

	vector <System> s=readfile(argv[1]);
	processsystems(s);
	FILE * fp = fopen (argv[2],"w");
	if (fp){
		writesystems(fp,s);
		fclose(fp);
	}else
		printf ("could not open %s for writing\n",argv[2]);
	return 0;
}
