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
	string sector;
	string name;
	float distance;
	float ascension;
	float declination;
	vec3 xyz;
	float luminosity;//in sun
	int type; // 0 = 1O blue   B = 20 blue // A = 30 Bluish-white  F = 40 White G = 50 yellow K = 60 Orange M = Red 70 
	int  size;// 0 dwwarf 1 normal 2 giant
	bool operator < (const System & a) const{
		if (sector==a.sector)
			return name < a.name;
		return sector<a.sector;
	}
	void computeXYZ() {
		xyz.z = distance * sin (declination);
		float xy = distance * cos (declination);
		xyz.y = xy*sin(ascension);
		xyz.x = xy*cos(ascension);
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
			fprintf (fp, "\t\t\t<var name=\"%s\" value=\"%s\"/>\n");			
		}
		fprintf (fp,"\t\t\t<var name=\"xyz\" value=\"%lf %lf %lf\"/>\n",(*i).xyz.x,(*i).xyz.y,(*i).xyz.z);
		fprintf (fp,"\t\t\t<var name=\"faction\" value=\"confed\"/>\n");
		if (iter>4 && iter+4<s.size()) {
			fprintf (fp,"\t\t\t<var name=\"jumps\" value=\"nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s\"/>\n",s[iter-1].name.c_str(),s[iter-2].name.c_str(),s[iter-3].name.c_str(),s[iter-4].name.c_str(),s[iter+1].name.c_str(),s[iter+2].name.c_str(),s[iter+3].name.c_str());
		}
		fprintf(fp,"\t\t</sector>\n");
		iter++;
	}	
	fprintf(fp,"\t</sector>\n</systems></galaxy>\n");
	
}
void processsystems (vector <System> & s){

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
