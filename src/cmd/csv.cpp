#include "csv.h"
#include "vsfilesystem.h"
using namespace std;
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

void CSVTable::Init (VSFileSystem::VSFile&f) {
   int len = f.Size();
   char * buffer = (char*)malloc(sizeof(char)*(len+1));
   buffer[len]=0;
   f.ReadLine(buffer,len);
   key = readCSV(buffer);
   for( unsigned int i=0;i<key.size();++i) {
      columns[key[i] ]=i;
   }
   while (f.ReadLine(buffer,len)<=VSFileSystem::Ok) {
      vector<string> strs = readCSV(buffer);
      unsigned int row = table.size()/key.size();
      while (strs.size()>key.size()) {
         strs.pop_back();
         fprintf (stderr,"error in csv, line %d: %s has no key",row+1,strs.back().c_str());
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
   free(buffer);
}
CSVTable::CSVTable(std::string filename) {
   VSFileSystem::VSFile f;
   VSFileSystem::VSError err = f.OpenReadOnly(filename,VSFileSystem::UnknownFile);
   if (err<=VSFileSystem::Ok) {
      Init(f);
      f.Close();
   }
}

CSVTable::Row::Row(CSVTable * parent, std::string key) {
   this->parent=parent;
   iter=parent->rows[key]*parent->key.size();
}
CSVTable::Row::Row(CSVTable * parent, unsigned int i) {
   this->parent=parent;
   iter=i*parent->key.size();
}
string CSVTable::Row::operator [] (string col) {
   map<string,int>::iterator i = parent->columns.find(col);
   if (i==parent->columns.end()) return std::string("");
   return parent->table[iter+(*i).second];
}
string CSVTable::Row::operator [] (unsigned int col) {
   return parent->table[iter+col];
}
string CSVTable::Row::getKey(unsigned int which ) {
   return parent->key[which];
}
bool CSVTable::RowExists(string name, unsigned int &where) {
   map<string,int>::iterator i = rows.find(name);
   if (i==rows.end())
      return false;
   where = (*i).second;
   return true;
}
