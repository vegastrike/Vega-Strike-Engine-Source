#include "config.h"
#include <string>
#include <vector>
#include <gnuhash.h>

#include "hashtable.h"
namespace VSFileSystem
{
	class VSFile;
}
//std::vector <std::string> readCSV(std::string line, std::string delim=",;");
std::vector <std::string> readCSV(std::string line, std::string delim=",;");
std::string writeCSV(const std::vector<std::string> &key, const std::vector<std::string> &table, std::string delim=",;");
class CSVTable {
 
private:
    void Init (std::string data);
public:
    std::string rootdir;
    vsUMap<std::string,int> columns;
    vsUMap<std::string,int> rows;
    std::vector<std::string> key;
    std::vector<std::string> table;

    CSVTable(std::string name,std::string saveroot);
    CSVTable(VSFileSystem::VSFile &f,  std::string saveroot);

    bool RowExists(std::string name, unsigned int&where);
    bool ColumnExists(std::string name, unsigned int&where);

public:
    //Optimizer toolbox
    enum optimizer_enum { optimizer_undefined=0x7fffffff };
    void SetupOptimizer(std::vector<std::string> keys, unsigned int type);

    //Opaque Optimizers - use the optimizer toolbox to set them up
    bool optimizer_setup;
    unsigned int optimizer_type;
    std::vector<std::string> optimizer_keys;
    std::vector<unsigned int> optimizer_indexes;
};

class CSVRow {
  std::string::size_type iter;
  CSVTable * parent;
public:
  std::string getRoot();
  size_t size() {return parent->key.size();}
  CSVRow(CSVTable * parent, std::string key);
  CSVRow(CSVTable * parent, unsigned int which);
  CSVRow(){parent=NULL;iter=std::string::npos;}
  const std::string& operator[](const std::string&) const;
  const std::string& operator[](unsigned int) const;
  const std::string& getKey(unsigned int which) const;
  std::vector<std::string>::iterator begin();
  std::vector<std::string>::iterator end();
  bool success()const {
    return parent!=NULL;
  }
  CSVTable* getParent() { return parent; };
};

extern std::vector<CSVTable*> unitTables;
