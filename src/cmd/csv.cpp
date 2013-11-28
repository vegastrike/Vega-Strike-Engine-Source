#include "csv.h"
#include "vsfilesystem.h"

using std::string;

vector< string > readCSV( const string &s, string delim )
{
    vector< string >l;
    string as;
    unsigned int    epos = 0;
    unsigned int    sl   = s.length();
    bool insert;
    bool quote  = false;
    char ddelim = 0;
    while ( (epos < sl) && !( (s[epos] == '\r') || (s[epos] == '\n') ) ) {
        insert = true;
        if (quote) {
            if (s[epos] == '\"') {
                if ( (epos+1 < sl) && (s[epos+1] == '\"') )
                    epos++;

                else
                    quote = insert = false;
            }
        } else {
            bool ep;
            if (!ddelim) {
                size_t dp = delim.find( s[epos] );
                if (dp != string::npos) ddelim = delim[dp];
                ep = (dp != string::npos);
            } else {
                ep = s[epos] == ddelim; 
            } 
            
            if (ep) {
                insert = false;
                l.push_back( as );
                as.erase();
            } else if (s[epos] == '\"') {
                if ( (epos+1 < sl) && (s[epos+1] == '\"') )
                    epos++;
                else
                    insert = !(quote = true);
            }
        }
        if ( insert && (epos < sl) ) as += s[epos];
        epos++;
    }
    if ( !as.empty() ) l.push_back( as );
    return l;
}

static string addQuote( string s, string delim = ",;" )
{
    if (s.find_first_of( delim+"\"" ) != string::npos) {
        if (s.find( '\"' ) != string::npos) {
            //Replace " by ""
            string as;
            int    sl = s.length();
            as.reserve( 2*sl );
            for (int i = 0; i < sl; i++)
                if (s[i] != '\"') as += s[i];

                else as += "\"\"";
            s.swap( as );
        }
        //Add single quotes to the sides
        s.insert( s.begin(), 1, '\"' );
        s.insert( s.end(), 1, '\"' );
    }
    return s;
}

string writeCSV( const vector< string > &key, const vector< string > &table, string delim )
{
    unsigned int i;
    unsigned int wid = key.size();
    string ret;
    for (i = 0; i < wid; ++i) {
        ret += addQuote( key[i], delim );
        if (i+1 < wid)
            ret += delim[0];
    }
    ret += '\n';
    for (i = 0; i < table.size(); ++i) {
        ret += addQuote( table[i], delim );
        if (i+1%wid == 0)
            ret += '\n';
        else
            ret += delim[0];
    }
    return ret;
}

void CSVTable::Init( const string &data )
{
    //Clear optimizer
    optimizer_setup = false;
    optimizer_keys.clear();
    optimizer_indexes.clear();
    optimizer_type  = ~0;
    const string delim( ",;" );
    const char  *cdata = data.c_str();
    const char  *csep  = strchr( cdata, '\n' );
    if (csep == NULL) return;
    string buffer( cdata, csep-cdata );
    cdata = csep+1;
    key   = readCSV( buffer, delim );
    for (unsigned int i = 0; i < key.size(); i++)
        columns[key[i]] = i;
    while (cdata && *cdata) {
        csep = strchr( cdata, '\n' );
        if (csep == NULL)
            buffer.assign( cdata );

        else
            buffer.assign( cdata, csep-cdata );
        if (csep == NULL)
            cdata = NULL;

        else
            cdata = csep+1;
        vector< string >strs = readCSV( buffer );
        unsigned int    row  = table.size()/key.size();
        while ( strs.size() > key.size() ) {
            fprintf( stderr, "error in csv, line %d: %s has no key", row+1, strs.back().c_str() );
            strs.pop_back();
        }
        while ( strs.size() < key.size() )
            strs.push_back( "" );
        assert( strs.size() == key.size() );
        table.insert( table.end(), strs.begin(), strs.end() );
        if ( strs.size() )
            rows[strs[0]] = row;

        else
            table.pop_back();
    }
}

CSVTable::CSVTable( const string &data, const string &root )
{
    this->rootdir = root;
    Init( data );
}

CSVTable::CSVTable( VSFileSystem::VSFile &f, const string &root )
{
    this->rootdir = root;
    Init( f.ReadFull() );
}

static string strip_white( const string &s )
{
    string::size_type start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    
    string::size_type end = s.find_last_not_of(" \t\r\n");
    if (end == string::npos)
        return s.substr(start);
    else
        return s.substr(start, end+1-start);
}

void 
CSVTable::Merge( const CSVTable &other )
{
    // Remember in preparation to reshape
    size_t orig_cols = key.size();
    
    // Merge columns
    std::vector<int> colmap;
    colmap.resize(other.key.size());
    
    for (vsUMap<std::string, int>::const_iterator it = other.columns.begin(); it != other.columns.end(); ++it) {
        vsUMap<std::string, int>::const_iterator local = columns.find(it->first);
        if (local == columns.end()) {
            VSFileSystem::vs_dprintf(2, "New column %s\n", it->first.c_str());
            key.push_back(it->first);
            local = columns.insert(std::pair<string, int>(it->first, key.size()-1)).first;
        }
        if (it->second >= int(colmap.size())) {
            std::cerr << "WTF column " << it->second << "?" << std::endl;
            abort();
        }
        VSFileSystem::vs_dprintf(2, "  %s (%d) -> %d\n", it->first.c_str(), it->second, local->second);
        colmap[it->second] = local->second;
    }
    
    // Reshape if necessary
    if (orig_cols != key.size()) {
        std::vector<std::string> orig_table;
        orig_table.swap(table);
        std::vector<std::string>::const_iterator orig_it = orig_table.begin();
        const std::string empty;
        
        VSFileSystem::vs_dprintf(1, "Reshaping %d columns into %d\n", orig_cols, columns.size());
        table.reserve(rows.size() * key.size());
        while (orig_it != orig_table.end()) {
            size_t i,n;
            for (i = 0; orig_it != orig_table.end() && i < orig_cols; ++i)
                table.push_back(*orig_it++);
            for (n = key.size(); i < n; ++i)
                table.push_back(empty);
        }
    }
    VSFileSystem::vs_dprintf(2, "Reshaped table holds %d cells\n", table.size());
    
    // Merge rows
    VSFileSystem::vs_dprintf(1, "Merging rows...\n");
    size_t merged_rows = 0, new_rows = 0;
    for (vsUMap<std::string, int>::const_iterator it = other.rows.begin(); it != other.rows.end(); ++it) {
        vsUMap<std::string, int>::const_iterator local = rows.find(it->first);
        if (local == rows.end()) {
            table.resize(table.size() + key.size());
            local = rows.insert(std::pair<string, int>(it->first, (table.size() - 1) / key.size())).first;
            ++new_rows;
        } else {
            ++merged_rows;
        }
        std::vector<std::string>::iterator table_it = table.begin() + local->second * key.size();
        std::vector<std::string>::const_iterator other_it = other.table.begin() + it->second * other.key.size();
        for (size_t i = 0; i < colmap.size(); ++i)
            if (!strip_white(*(other_it + i)).empty())
                *(table_it + colmap[i]) = *(other_it + i);
    }
    VSFileSystem::vs_dprintf(1, "Rows Merged: %d, Rows Added: %d\n", merged_rows, new_rows);
    VSFileSystem::vs_dprintf(2, "Merged table holds %d cells\n", table.size());
}

CSVRow::CSVRow( CSVTable *parent, const string &key )
{
    this->parent = parent;
    iter = parent->rows[key]*parent->key.size();
}

CSVRow::CSVRow( CSVTable *parent, unsigned int i )
{
    this->parent = parent;
    iter = i*parent->key.size();
}

const string& CSVRow::operator[]( const string &col ) const
{
    static string empty_string;
    vsUMap< string, int >::iterator i = parent->columns.find( col );
    if ( i == parent->columns.end() )
        return empty_string;

    else
        return parent->table[iter+(*i).second];
}

const string& CSVRow::operator[]( unsigned int col ) const
{
    return parent->table[iter+col];
}

const string& CSVRow::getKey( unsigned int which ) const
{
    return parent->key[which];
}

bool CSVTable::RowExists( const string &name, unsigned int &where )
{
    vsUMap< string, int >::iterator i = rows.find( name );
    if ( i == rows.end() )
        return false;
    where = (*i).second;
    return true;
}

bool CSVTable::ColumnExists( const string &name, unsigned int &where )
{
    vsUMap< string, int >::iterator i = columns.find( name );
    if ( i == columns.end() )
        return false;
    where = (*i).second;
    return true;
}

vector< CSVTable* > unitTables;

string CSVRow::getRoot()
{
    if (parent)
        return parent->rootdir;
    fprintf( stderr, "Error getting root for unit\n" );
    return "";
}

void CSVTable::SetupOptimizer( const vector< string > &keys, unsigned int type )
{
    optimizer_setup = true;
    optimizer_type  = type;
    optimizer_keys  = keys;
    optimizer_indexes.resize( keys.size(), CSVTable::optimizer_undefined );
    for (unsigned int i = 0; i < keys.size(); ++i)
        ColumnExists( keys[i], optimizer_indexes[i] );
}

CSVTable* loadCSVTableList(const string& csvfiles, VSFileSystem::VSFileType fileType, bool critical)
{
    string::size_type pwhere = 0, where, where2 = 0;
    CSVTable *table = NULL;
    while (where2 != string::npos) {
        where = where2 = csvfiles.find_first_of( " \t\r\n", pwhere );
        if (where == string::npos)
            where = csvfiles.length();
        string tmp = csvfiles.substr( pwhere, where-pwhere );
        
        if (!tmp.empty()) {
            VSFileSystem::vs_dprintf(3, "Opening CSV database from '%s'\n", tmp.c_str());
            
            VSFileSystem::VSFile thisFile;
            VSFileSystem::VSError err = thisFile.OpenReadOnly( tmp, fileType );
            if (err <= VSFileSystem::Ok) {
                VSFileSystem::vs_dprintf(1, "Loading CSV database from '%s'\n", tmp.c_str());
                if (table == NULL)
                    table = new CSVTable( thisFile, thisFile.GetRoot() );
                else
                    table->Merge(CSVTable( thisFile, thisFile.GetRoot() ));
                thisFile.Close();
            } else if (critical) {
                std::cerr << "Could not load CSV database at " << tmp << std::endl;
                exit(2);
            }
        }
        if (where2 == string::npos) 
            break;
        pwhere = where+1;
    }
    return table;
}
