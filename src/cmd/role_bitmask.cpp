#include "role_bitmask.h"
#include "xml_support.h"
#include <map>
using std::map;
using std::string;
using std::pair;
namespace ROLES {
	int discreteLog (int bitmask) {
		for (unsigned char i=0;i<sizeof(int)*8;i++) {
			if (bitmask& (1<<i)) {
				return i;
			}
		}
		fprintf (stderr,"undefined discrete log.");
		return 0;
	}
	vector < vector <char > > buildroles ();

	vector < vector <char > > &getAllRolePriorities () {
		static vector <vector <char> > allrolepriority = buildroles();
		return allrolepriority;
	}
	vector <char>& getPriority(unsigned char rolerow) {
		if (rolerow>getAllRolePriorities().size()) {
			fprintf (stderr,"FATAL ERROR ROLE OUT OF RANGE");
			exit(1);
		}
		return getAllRolePriorities()[rolerow];
	}
	std::map<string,int> rolemap;
	vector <std::string> readCSV (std::string s) {
		vector <std::string> v;int loc;
		int sub1=s.find ("\r");
		s=s.substr(0,sub1);
		int sub2=s.find("\n");
		s=s.substr(0,sub2);
			do {
				loc=s.find (",");
				string t =s.substr (0,loc);
					if (v.size() >=32) {
						fprintf (stderr,"ERROR with bitmasking. Only 32 ship types allowed");
					}
					v.push_back(t);
				if (loc!=string::npos)
					s = s.substr (loc+1);
			} while (loc!=string::npos);
		return v;
	}
	unsigned char InternalGetRole (const std::string &s) {
		map<string,int>::iterator i = rolemap.find (strtoupper (s));
		if (i!=rolemap.end()) {
			return (*i).second;
		}
		return 0;
	}
	vector < vector <string > > buildscripts() {
	  vector<vector <string> > scripts;
	  getAllRolePriorities ();
	  
	  FILE * fp = fopen("ai/VegaEvents.csv","rb");
	  if (fp) {
			fseek (fp,0,SEEK_END);
			int len = ftell (fp);
			char *temp = (char *)malloc (len+1);
			memset (temp,0,len+1);
			fseek (fp,0,SEEK_SET);
			fgets(temp,len,fp);
			vector <string> vec=readCSV(temp);
			if (vec.size()) vec.erase (vec.begin());
			unsigned int i=0;
			for (i=0;i<maxRoleValue();i++) {
			  scripts.push_back (vector<string>());
			  for (int j=0;j<maxRoleValue();j++) {
			    scripts[i].push_back("default");
			  }
			}
  		    //fseek (fp,0,SEEK_SET);
			for (i=0;i<vec.size();i++) {
			  fgets(temp,len,fp);
			  vector <string> strs=readCSV(temp);
			  if (strs.size()) {
			    string front = strs.front();
			    strs.erase(strs.begin());
			    unsigned int scriptind = getRole(front);
			    for (unsigned int j=0;j<strs.size();j++) {
			      int index=  getRole(vec[j]);
			      scripts[scriptind][index]=strs[j]; 
			    }
			  }
			 
			}
			free (temp);
			fclose (fp);
	  }
	  return scripts;
	}
        const std::string &getRoleEvents (unsigned char ourrole, unsigned char theirs) {
	  static vector < vector <string> > script = buildscripts();
	  const static string def="default";
	  if (ourrole>=script.size()) {
	    fprintf (stderr,"bad error with getRoleEvetnts (no event specified)");
	    return def;
	  }
	  if (theirs>=script[ourrole].size()) {
	    fprintf (stderr,"bad error || with getRoleEvetnts (no event specified)");
	    return def;
	  }
	  return script[ourrole][theirs];
	}
	vector < vector <char > > buildroles() {
		vector <vector <char> >rolePriorities;
		int count=0;
		FILE * fp = fopen("ai/VegaPriorities.csv","rb");
		if (fp) {
			fseek (fp,0,SEEK_END);
			int len = ftell (fp);
			char *temp = (char *)malloc (len+1);
			memset (temp,0,len+1);
			fseek (fp,0,SEEK_SET);
			fgets(temp,len,fp);
			vector <string> vec=readCSV(temp);
			if (vec.size()) vec.erase (vec.begin());
			//fprintf (stderr," SIZE %d\n",vec.size());
			int loc;
			unsigned int i;
			for (i=0;i<vec.size();i++) {
			  //fprintf (stderr," %s AS %d\n",vec[i].c_str(),i);
				rolemap.insert (pair<string,int>(strtoupper(vec[i]),i));
			}

			for (i=0;i<vec.size();i++) {
				rolePriorities.push_back (vector<char>());
			}
			for (i=0;i<vec.size();i++) {
				temp[0]=0;
				fgets (temp,len,fp);
				vector <string> priority = readCSV(temp);
				int i=InternalGetRole (priority[0]);
				//fprintf (stderr, "role of %s is %d\n",priority[0].c_str(),i);
				if (i<rolePriorities.size()) {
					for (unsigned int j=0;j<rolePriorities.size();j++) {
						if (rolePriorities[i].size()<rolePriorities.size())
							rolePriorities[i].push_back(31);
						if (j+1<priority.size()) {
							rolePriorities[i][j]=XMLSupport::parse_int(priority[j+1]);
						}
					}
				}
			}
			free( temp);
			fclose (fp);
		}else {
		  rolePriorities.push_back(vector <char>());
		  rolePriorities[0].push_back(0);
		}
		return rolePriorities;
	}
	unsigned int maxRoleValue() {
		static int i=getAllRolePriorities().size();
		return i;
	}
	unsigned char getRole (const std::string &s) {
		int temp = maxRoleValue();
		return InternalGetRole(s);
	}
	unsigned int readBitmask (const std::string &ss){
		string s= ss;
		int loc=string::npos;
		int ans =0;
		do{
			loc=s.find (" ");
			ans |= (1<<getRole (s.substr (0,loc)));
			if (loc!=string::npos)
				s = s.substr (loc+1);
		}while (loc!=string::npos);
		return ans;
	}
}
