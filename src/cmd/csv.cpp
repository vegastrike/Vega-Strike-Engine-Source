#include "csv.h"
#include "vsfilesystem.h"
using namespace std;


static string::size_type findQuot(string s,string chr, int offset=0){
	if (offset>=s.length())
          return string::npos;
	string::size_type quot=s.substr(offset).find(chr);
	if (quot!=string::npos)
          return quot+offset;
	return quot;//string::npos
}
static string elimiQuote(string s, string delim="\"\"\""){
  string ret;
  int sl=s.length();
  ret.reserve(sl);
  for (int i=0; i<sl; i++) {
      if ((i+1<sl)&&(s[i]=='\"')&&(s[i+1]=='\"')) 
          ret+=s[i++]; else if (s[i]!='\"')
          ret+=s[i];
  }
  /*
  string ret;
  string::size_type where=findQuot(s,delim);
  bool even = true;
  if (where==string::npos && delim!="\""){
    s=elimiQuote(s,"\"");
  }
  while(where!=string::npos){
    string tmp=s.substr(0,where);
    if (even && delim!="\""){
      tmp=elimiQuote(tmp,"\"");
    }
    ret=ret+tmp;
    even = !even;
    s=s.substr(where+delim.length());
    where=findQuot(s,delim);
  }
  ret=ret+s;
  return ret;
  */
}
vector <std::string> readCSV (std::string s,std::string delim) {
    // Proposed readCSV code -- begin
    //   Working this way should be much more efficient, and
    //   also allows flexibility in separators (if you specify
    //   delim=",;", the first , or ; it finds specifies that that
    //   is a delimiter. Doing so and using ; as delimiter provides
    //   both excel and current file compatibility.
    //   Sadly, addQuote has to change in an incompatible way...
	vector <std::string> l;
    std::string as;
    int spos=0,epos=0,i=0;
    int sl=s.length();
    bool insert;
    bool quote=false;
    char ddelim=0;
    while ((epos<sl)&&!((s[epos]=='\r')||(s[epos]=='\n'))) {
        insert=true;
        if (quote) {
            if (s[epos]=='\"') {
                if ((epos+1<sl)&&(s[epos+1]=='\"')) 
                    epos++; else 
                    quote=insert=false;
            };
        } else {
            bool ep;
            if (!ddelim) {
                int dp=delim.find(s[epos]);
                if (dp != std::string::npos) ddelim=delim[dp];
                ep = (dp != std::string::npos);
            } else ep = s[epos]==ddelim;
            if (ep) {
                insert=false;
                l.push_back(as);
                as.erase();
            } else {
                if (s[epos]=='\"') {
                    if ((epos+1<sl)&&(s[epos+1]=='\"')) 
                        epos++; else 
                        insert=!(quote=true);
                };
            };
        };
        if (insert&&(epos<sl)) as += s[epos];
        epos++;
    };
    if (!as.empty()) l.push_back(as);
	return l;
    // Proposed readCSV code -- end
    /*
    // Original readCSV code -- begin
	vector <std::string> l;
	std::string::size_type loc;
	int sub1=s.find ("\r");
	s=s.substr(0,sub1);
	int sub2=s.find("\n");
	s=s.substr(0,sub2);
	int trip=1;
        string::size_type quot3 = findQuot(s,"\"\"\"");
        string::size_type quot1 = findQuot(s,"\"");
        string::size_type quot;
        string::size_type sem=string::npos;
        if (quot3==string::npos || (quot1!=string::npos && quot1<quot3)){
          trip=0;
            quot=quot1;
        }else{
          quot=quot3;
        }
	sem = s.find(delim);	
	while (sem!=string::npos){
          string::size_type equot=0;
          while (quot<sem && quot!=string::npos){
            if (trip){
              equot=findQuot(s,"\"\"\"",quot+1);
            }else{
              equot=findQuot(s,"\"",quot+1);
            }
            quot3=findQuot(s,"\"\"\"",equot+1);
            quot1 = findQuot(s,"\"",equot+1);
            trip=1;
            quot=quot3;
            if (quot3==string::npos || (quot1!=string::npos && quot1<quot3)){
              quot=quot1;
              trip=0;
              if (equot==string::npos)
                break;
            }
          }
          sem = s.substr(equot).find(delim);
          if (sem!=string::npos){
            sem=sem+equot;
          }else{
            break;
          }
          l.push_back(elimiQuote(s.substr(0,sem)));
          s=s.substr(sem+delim.length());
          sem = s.find(delim);
          quot3=findQuot(s,"\"\"\"");
          quot1 = findQuot(s,"\"");
          quot=quot3;
          trip=1;
          if (quot3==-1 || (quot1!=-1 && quot1<quot3)){
            quot=quot1;
            trip=0;
          }
        }
        s=elimiQuote(s);
	if (s.length()){
          l.push_back(s);
        }
	return l;
    // Original readCSV code -- end
    */
}
static std::string addQuote(std::string s, string delim=",;") {
  // Proposed addQuote code -- begin
  if (s.find_first_of(delim+"\"")!=string::npos) {
      if (s.find('\"')!=string::npos) {
          //Replace " by ""
          std::string as;
          int sl = s.length();
          as.reserve(2*sl);
          for (int i=0; i<sl; i++) if (s[i]!='\"') as += s[i]; else as += "\"\"";
          s.swap(as);
      }
      //Add single quotes to the sides
      s.insert(s.begin(),1,'\"');
      s.insert(s.end(),1,'\"');
  }
  return s;
  // Proposed addQuote code -- end

  /*
  // Original addQuote code -- begin
  if (s.find(delim)!=string::npos) {
    if (s.find("\"")!=string::npos) {
      return string("\"\"\"")+s+string("\"\"\"");
    }else {
      return string("\"")+s+"\"";
    }
  }else {
    return s;
  }
  // Original addQuote code -- end
  */
}
std::string writeCSV(const vector<string> &key, const vector<string> &table, std::string delim){
  unsigned int i;
  unsigned int wid = key.size();
  std::string ret;
  for (i=0;i<wid;++i) {
    ret+=addQuote(key[i],delim);
    if (i+1<wid)
      ret+=delim[0];
  }
  ret+='\n';
  for (i=0;i<table.size();++i) {
    ret+=addQuote(table[i],delim);
    if (i+1%wid==0)
      ret+='\n';
    else
      ret+=delim[0];
  }
  return ret;
}
void CSVTable::Init (std::string data) {
   // Clear optimizer
   optimizer_setup = false;
   optimizer_keys.clear();
   optimizer_indexes.clear();
   optimizer_type = ~0;
   // Proposed Init code -- begin
   const std::string delim(",;");
   const char *cdata = data.c_str();
   const char *csep = strchr(cdata,'\n');
   if (csep==NULL) return;
   std::string buffer(cdata,csep-cdata);
   cdata = csep+1;
   key = readCSV(buffer,delim);
   for (unsigned int i=0;i<key.size();i++) columns[key[i]]=i;
   while (cdata&&*cdata) {
       csep = strchr(cdata,'\n');
       if (csep==NULL)
           buffer.assign(cdata); else
           buffer.assign(cdata,csep-cdata);
       if (csep==NULL)
           cdata = NULL; else
           cdata = csep+1;
       vector<string> strs = readCSV(buffer);
       unsigned int row = table.size()/key.size();
       while (strs.size()>key.size()) {
           fprintf (stderr,"error in csv, line %d: %s has no key",row+1,strs.back().c_str());
           strs.pop_back();
       }
       while(strs.size()<key.size()) strs.push_back("");
       assert(strs.size()==key.size());
       table.insert(table.end(),strs.begin(),strs.end());
       if (strs.size())
           rows[strs[0]] = row; else
           table.pop_back();
   }
   // Proposed Init code -- end
   /*
   // Original Init code -- begin
   std::string::size_type where=data.find('\n');
   if (where==string::npos) return;
   std::string buffer = data.substr(0,where);
   data=data.substr(where+1);
   key = readCSV(buffer.c_str());
   for( unsigned int i=0;i<key.size();++i) {
      columns[key[i] ]=i;
   }
   while (data.length()) {
      where=data.find("\n");
      if (where!=std::string::npos) {
        buffer = data.substr(0,where);
        data=data.substr(where+1);
      }else {
        buffer=data;
        data="";
      }
      vector<string> strs = readCSV(buffer.c_str());
      unsigned int row = table.size()/key.size();
      while (strs.size()>key.size()) {
         fprintf (stderr,"error in csv, line %d: %s has no key",row+1,strs.back().c_str());
         strs.pop_back();
      }
      while(strs.size()<key.size()) {
         strs.push_back("");
      }
      assert(strs.size()==key.size());
      table.insert(table.end(),strs.begin(),strs.end());
      if (strs.size()) {
         rows[strs[0] ] = row;
      }else {
         table.pop_back();
      }      
   }
   // Original Init code -- end
   */
}
CSVTable::CSVTable(std::string data,std::string root) {
	this->rootdir=root;
	Init(data);
//   VSFileSystem::VSFile f;
//   VSFileSystem::VSError err = f.OpenReadOnly(filename,VSFileSystem::UnknownFile);
//   if (err<=VSFileSystem::Ok) {
//      Init(f);
//      f.Close();
//   }
}

CSVTable::CSVTable(VSFileSystem::VSFile & f,std::string root) {
	this->rootdir = root;
	Init(f.ReadFull());
}
CSVRow::CSVRow(CSVTable * parent, std::string key) {
   this->parent=parent;
   iter=parent->rows[key]*parent->key.size();
}
CSVRow::CSVRow(CSVTable * parent, unsigned int i) {
   this->parent=parent;
   iter=i*parent->key.size();
}
string CSVRow::operator [] (string col) {
	stdext::hash_map<string,int>::iterator i = parent->columns.find(col);
   if (i==parent->columns.end()) return std::string("");
   return parent->table[iter+(*i).second];
}
string CSVRow::operator [] (unsigned int col) {
   return parent->table[iter+col];
}
string CSVRow::getKey(unsigned int which ) {
   return parent->key[which];
}
bool CSVTable::RowExists(string name, unsigned int &where) {
	stdext::hash_map<string,int>::iterator i = rows.find(name);
   if (i==rows.end())
      return false;
   where = (*i).second;
   return true;
}
bool CSVTable::ColumnExists(string name, unsigned int &where) {
	stdext::hash_map<string,int>::iterator i = columns.find(name);
   if (i==columns.end())
      return false;
   where = (*i).second;
   return true;
}
vector<CSVTable*> unitTables;
string CSVRow::getRoot()  {
  if (parent)
    return parent->rootdir;
  fprintf (stderr,"Error getting root for unit\n");
  return "";
}
void CSVTable::SetupOptimizer(std::vector<std::string> keys, unsigned int type) 
{
    optimizer_setup = true;
    optimizer_type = type;
    optimizer_keys = keys;
    optimizer_indexes.resize(keys.size(),CSVTable::optimizer_undefined);
    for (int i=0; i<keys.size(); i++) ColumnExists(keys[i],optimizer_indexes[i]);
}