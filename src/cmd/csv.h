#include <string>
#include <vector>
#include <map>
#include "hashtable.h"
namespace VSFileSystem
{
	class VSFile;
}
std::vector <std::string> readCSV(std::string line, std::string delim=",");
std::string writeCSV(const std::vector<std::string> &key, const std::vector<std::string> &table);
class CSVTable {
  
 private:
   void Init (VSFileSystem::VSFile &f);
 public:
  std::string rootdir;
   std::map<std::string,int> columns;
   std::map<std::string,int> rows;
   std::vector<std::string> key;
   std::vector<std::string > table;
   CSVTable(std::string name);
   bool RowExists(std::string name, unsigned int&where);
  CSVTable(VSFileSystem::VSFile &f){
    Init(f);
  }
   
};

class CSVRow {
  std::string::size_type iter;
  CSVTable * parent;
public:
  std::string getRoot();
  size_t size() {return parent->key.size();}
  CSVRow(CSVTable * parent, std::string key);
  CSVRow(CSVTable * parent, unsigned int which);
  CSVRow(){parent=NULL;iter=string::npos;}
  std::string operator[](std::string);
  std::string operator[](unsigned int);
  std::vector<std::string>::iterator begin();
  std::vector<std::string>::iterator end();
  std::string getKey(unsigned int which);      
  bool success()const {
    return parent!=NULL;
  }
};

extern std::vector<CSVTable*> unitTables;
