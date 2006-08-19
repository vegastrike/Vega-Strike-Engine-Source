#include "role_bitmask.h"
#include "xml_support.h"
#include <gnuhash.h>

#include "vs_globals.h"
#include "config_xml.h"
#include "vsfilesystem.h"
#include "csv.h"
using stdext::hash_map;
using std::string;
using std::pair;
using namespace VSFileSystem;

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
	stdext::hash_map<string,int> rolemap;
	unsigned char InternalGetRole (const std::string &s) {
		stdext::hash_map<string,int>::iterator i = rolemap.find (strtoupper (s));
		if (i!=rolemap.end()) {
			return (*i).second;
		}
		return 0;
	}
	std::string InternalGetStrRole (unsigned char c) {
	   stdext::hash_map<string,int>::iterator i = rolemap.begin();
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
                        size_t siz=getAllRolePriorities().size();

			vector <string> vec=readCSV(temp);
                        if (siz&&getAllRolePriorities()[0].size()!=vec.size()) {
                          fprintf (stderr, "FATAL error in hash map... column %d in ai/VegaEvents.csv does not line up with that item in ai/VegaPriorities.csv\n",vec.size());
                        }
			if (vec.size()) vec.erase (vec.begin());                        
                        for (unsigned int j=0;j<vec.size();j++) {
                          if (getRole(vec[j])!=j){
                            fprintf (stderr, "FATAL error in hash map... column %d in ai/VegaEvents.csv does not line up with that item in ai/VegaPriorities.csv\n",j);
                          }
                        }
			unsigned int i=0;
			for (i=0;i<siz;i++) {
			  scripts.push_back (vector<string>());
			  for (unsigned int j=0;j<vec.size();j++) {
			    scripts[i].push_back("default");
			  }
			}
  		    //VSFileSystem::vs_seek (fp,0,SEEK_SET);
			for (i=0;i<vec.size();i++) {
			  f.ReadLine(temp,len);
			  vector <string> strs=readCSV(temp);
			  if (strs.size()) {
			    string front = strs.front();
			    unsigned int scriptind = getRole(front);
                              while(scripts.size()<=scriptind)
                                scripts.push_back(vector<string>());
                              
			    for (unsigned int j=1;j<strs.size();j++) {
			      int index=  getRole(vec[j]);
                              while(scripts[scriptind].size()<=index)
                                scripts[scriptind].push_back("default");
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
			//VSFileSystem::vs_fprintf (stderr," SIZE %d\n",vec.size());
			unsigned int i;
			for (i=1;i<vec.size();i++) {
			  //VSFileSystem::vs_fprintf (stderr," %s AS %d\n",vec[i].c_str(),i);
				rolemap.insert (pair<string,int>(strtoupper(vec[i]),i-1));
			}
                        vector<vector<char> >tmprolepriorities;
                        vector<string> tmpnamelist;
                        while(f.ReadLine(temp,len)==Ok) {				
                          vector <string> priority = readCSV(temp);
                          if (priority.size()>0) {
                            tmpnamelist.push_back(strtoupper(priority[0]));
                            tmprolepriorities.push_back(vector<char>());
                            for (unsigned int j=1;j<priority.size();j++) {
                              tmprolepriorities.back().push_back(XMLSupport::parse_int(priority[j]));
                            }
                            while(tmprolepriorities.back().size()<vec.size()) {
                              tmprolepriorities.back().push_back(31);
                            }
                          }                                  
                        }
                        for (int k=0;k<2;++k) {
                          for (i=0;i<tmpnamelist.size();++i) {
                            stdext::hash_map<string,int>::iterator iter=rolemap.find(tmpnamelist[i]);
                            int j=-1;
                            if (iter!=rolemap.end()) {
                              if (k==0)
                                j=iter->second;
                            }else if (k==1){
                              for (j=0;j<(int)rolePriorities.size();++j){
                                if (rolePriorities[j].size()==0) {
                                  break;
                                }
                              }
                              rolemap[tmpnamelist[i]]=j;
                            }       
                            if (j!=-1) {
                              while(rolePriorities.size()<=j) {
                                rolePriorities.push_back(vector<char>());                              
                              }
                              rolePriorities[j].swap(tmprolepriorities[i]);                            
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
	unsigned int getCapitalRoles () {
          static string defaultcapshipvalues=vs_config->getVariable("data","capship_roles","ESCORTCAP CAPITAL CARRIER BASE TROOP");
          unsigned int retval=0;
          string inp=defaultcapshipvalues;
          string::size_type where;
          while((where=inp.find(" "))!=string::npos) {                     
            string tmp=inp.substr(0,where);
            unsigned char logrole=getRole(tmp);
            if (tmp==getRole(logrole)) {
              retval|=(1<<logrole);
            }
            inp=inp.substr(where+1);
          }
          if (inp.length()) {
            unsigned char logrole=getRole(inp);
            string tmp=getRole(logrole);
            if (tmp==inp) {
              retval|=(1<<logrole);
            }
          }
          return retval;
	}
}
