#include "role_bitmask.h"
#include "xml_support.h"
#include <map>
#include "vsfilesystem.h"
#include "csv.h"
using std::map;
using std::string;
using std::pair;
using namespace VSFileSystem;
vector <std::string> readCSV (std::string s) {
	vector <std::string> v;
	std::string::size_type loc;
	int sub1=s.find ("\r");
	s=s.substr(0,sub1);
	int sub2=s.find("\n");
	s=s.substr(0,sub2);
		do {
			loc=s.find (",");
			string t =s.substr (0,loc);
				if (v.size() >=32) {
					VSFileSystem::vs_fprintf (stderr,"ERROR with bitmasking. Only 32 ship types allowed");
				}
				v.push_back(t);
			if (loc!=string::npos)
				s = s.substr (loc+1);
		} while (loc!=string::npos);
	return v;
}

namespace ROLES {
	int discreteLog (int bitmask) {
		for (unsigned char i=0;i<sizeof(int)*8;i++) {
			if (bitmask& (1<<i)) {
				return i;
			}
		}
		VSFileSystem::vs_fprintf (stderr,"undefined discrete log.");
		return 0;
	}
	vector < vector <char > > buildroles ();

	vector < vector <char > > &getAllRolePriorities () {
		static vector <vector <char> > allrolepriority = buildroles();
		return allrolepriority;
	}
	vector <char>& getPriority(unsigned char rolerow) {
		if (rolerow>getAllRolePriorities().size()) {
			VSFileSystem::vs_fprintf (stderr,"FATAL ERROR ROLE OUT OF RANGE");
			exit(1);
		}
		return getAllRolePriorities()[rolerow];
	}
	std::map<string,int> rolemap;
	unsigned char InternalGetRole (const std::string &s) {
		map<string,int>::iterator i = rolemap.find (strtoupper (s));
		if (i!=rolemap.end()) {
			return (*i).second;
		}
		return 0;
	}
	std::string InternalGetStrRole (unsigned char c) {
	   std::map<string,int>::iterator i = rolemap.begin();
	   for (;i!=rolemap.end();++i) {
              if ((*i).second==c)
                 return (*i).first;
           }
           return rolemap.size()?(*rolemap.begin()).first:std::string("");
	}
	vector < vector <string > > buildscripts() {
	  vector<vector <string> > scripts;
	  getAllRolePriorities ();
	  
	  VSFile f;
	  VSError err = f.OpenReadOnly( "VegaEvents.csv", AiFile);
	  if (err<=Ok) {
			int len = f.Size();
			char *temp = (char *)malloc (len+1);
			memset (temp,0,len+1);
			f.ReadLine(temp,len);
			vector <string> vec=readCSV(temp);
			if (vec.size()) vec.erase (vec.begin());
			unsigned int i=0;
			for (i=0;i<maxRoleValue();i++) {
			  scripts.push_back (vector<string>());
			  for (unsigned int j=0;j<maxRoleValue();j++) {
			    scripts[i].push_back("default");
			  }
			}
  		    //VSFileSystem::vs_seek (fp,0,SEEK_SET);
			for (i=0;i<vec.size();i++) {
			  f.ReadLine(temp,len);
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
			f.Close();
	  }
	  return scripts;
	}
        const std::string &getRoleEvents (unsigned char ourrole, unsigned char theirs) {
	  static vector < vector <string> > script = buildscripts();
	  const static string def="default";
	  if (ourrole>=script.size()) {
	    VSFileSystem::vs_fprintf (stderr,"bad error with getRoleEvetnts (no event specified)");
	    return def;
	  }
	  if (theirs>=script[ourrole].size()) {
	    VSFileSystem::vs_fprintf (stderr,"bad error || with getRoleEvetnts (no event specified)");
	    return def;
	  }
	  return script[ourrole][theirs];
	}
	vector < vector <char > > buildroles() {
		vector <vector <char> >rolePriorities;
		VSFile f;
		VSError err = f.OpenReadOnly( "VegaPriorities.csv", AiFile);
		if (err<=Ok) {
			int len = f.Size();
			char *temp = (char *)malloc (len+1);
			memset (temp,0,len+1);
			f.ReadLine(temp,len);
			vector <string> vec=readCSV(temp);
			if (vec.size()) vec.erase (vec.begin());
			//VSFileSystem::vs_fprintf (stderr," SIZE %d\n",vec.size());
			unsigned int i;
			for (i=0;i<vec.size();i++) {
			  //VSFileSystem::vs_fprintf (stderr," %s AS %d\n",vec[i].c_str(),i);
				rolemap.insert (pair<string,int>(strtoupper(vec[i]),i));
			}

			for (i=0;i<vec.size();i++) {
				rolePriorities.push_back (vector<char>());
			}
			for (i=0;i<vec.size();i++) {
				temp[0]=0;
				f.ReadLine(temp,len);
				vector <string> priority = readCSV(temp);
				unsigned int i=InternalGetRole (priority[0]);
				//VSFileSystem::vs_fprintf (stderr, "role of %s is %d\n",priority[0].c_str(),i);
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
			f.Close();
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
		//int temp = maxRoleValue();
		return InternalGetRole(s);
	}
	std::string getRole (unsigned char c) {
		//int temp = maxRoleValue();
		return InternalGetStrRole(c);
	}
	unsigned int readBitmask (const std::string &ss){
		string s= ss;
		std::string::size_type loc=string::npos;
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
