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
}
vector <std::string> readCSV (std::string s,std::string delim) {
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
}
static std::string addQuote(std::string s, string delim=",") {
  if (s.find(delim)!=string::npos) {
    if (s.find("\"")!=string::npos) {
      return string("\"\"\"")+s+string("\"\"\"");
    }else {
      return string("\"")+s+"\"";
    }
  }else {
    return s;
  }
}
std::string writeCSV(const vector<string> &key, const vector<string> &table){
  unsigned int i;
  unsigned int wid = key.size();
  std::string ret;
  for (i=0;i<wid;++i) {
    ret+=addQuote(key[i]);
    if (i+1<wid)
      ret+=',';
  }
  ret+='\n';
  for (i=0;i<table.size();++i) {
    ret+=addQuote(table[i]);
    if (i+1%wid==0)
      ret+='\n';
    else
      ret+=',';
  }
  return ret;
}
void CSVTable::Init (std::string data) {
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
   map<string,int>::iterator i = parent->columns.find(col);
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
   map<string,int>::iterator i = rows.find(name);
   if (i==rows.end())
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
