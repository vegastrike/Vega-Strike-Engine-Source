#include <string>
#include <vector>
#include <map>
#include "hashtable.h"
namespace VSFileSystem
{
	class VSFile;
}
std::vector <std::string> readCSV(std::string line);
class CSVTable {
 private:

   void Init (VSFileSystem::VSFile &f);
 public:
   std::map<std::string,int> columns;
   std::map<std::string,int> rows;
   std::vector<std::string> key;
   std::vector<std::string > table;
   class Row {
      unsigned int iter;
      CSVTable * parent;
   public:
      Row(CSVTable * parent, std::string key);
      Row(CSVTable * parent, unsigned int which);
      std::string operator[](std::string);
      std::string operator[](unsigned int);
      std::vector<std::string>::iterator begin();
      std::vector<std::string>::iterator end();
      std::string getKey(unsigned int which);      
   };
   CSVTable(VSFileSystem::VSFile &f){
      Init(f);
   }
   CSVTable(std::string name);
   bool RowExists(std::string name, unsigned int&where);
   
};
