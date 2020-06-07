#include <string>
#include <vector>
#include <gnuhash.h>

#include "hashtable.h"
#include "vsfilesystem.h"

// delim should be read as separator and not to be confused with text delimiter see http://creativyst.com/Doc/Articles/CSV/CSV01.htm
// separator values , and ; while delimiter is listed as quote or "
std::vector< std::string >readCSV( const std::string &line, std::string delim = ",;" );  
std::string writeCSV( const std::vector< std::string > &key, const std::vector< std::string > &table, std::string delim = ",;" );

class CSVTable
{
private:
    void Init( const std::string &data );
    
public:
    std::string rootdir;
    vsUMap< std::string, int >columns;
    vsUMap< std::string, int >rows;
    std::vector< std::string >key;
    std::vector< std::string >table;

    CSVTable( const std::string &name, const std::string &saveroot );
    CSVTable( VSFileSystem::VSFile &f, const std::string &saveroot );

    bool RowExists( const std::string &name, unsigned int &where );
    bool ColumnExists( const std::string &name, unsigned int &where );
    void Merge(const CSVTable &other);

public:
    //Optimizer toolbox
    enum optimizer_enum {optimizer_undefined=0x7fffffff};
    void SetupOptimizer( const std::vector< std::string > &keys, unsigned int type );

    //Opaque Optimizers - use the optimizer toolbox to set them up
    bool optimizer_setup;
    unsigned int optimizer_type;
    std::vector< std::string >optimizer_keys;
    std::vector< unsigned int >optimizer_indexes;
};

class CSVRow
{
    std::string::size_type iter;
    CSVTable *parent;
public:
    std::string getRoot();
    size_t size() const
    {
        return parent->key.size();
    }
    
    CSVRow( CSVTable *parent, const std::string &key );
    CSVRow( CSVTable *parent, unsigned int which );
    CSVRow()
    {
        parent = NULL;
        iter   = std::string::npos;
    }
    const std::string& operator[]( const std::string& ) const;
    const std::string& operator[]( unsigned int ) const;
    const std::string& getKey( unsigned int which ) const;
    std::vector< std::string >::iterator begin();
    std::vector< std::string >::iterator end();
    bool success() const
    {
        return parent != NULL;
    }
    CSVTable * getParent()
    {
        return parent;
    }
};

/**
 * Load a space-separated list of CSV files and return a merged
 * representation of it.
 * 
 * @param csvfiles Space-separated list of CSV files
 * @param critical If true, any error reading any file will result
 *      in a fatal error and an exit() call.
 */
CSVTable * loadCSVTableList(const std::string &csvfiles, VSFileSystem::VSFileType fileType, bool critical);

extern std::vector< CSVTable* >unitTables;

