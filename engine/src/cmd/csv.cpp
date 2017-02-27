#include "csv.h"
#include "vsfilesystem.h"

using std::string;

vector< string > readCSV( string s, string delim )
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

void CSVTable::Init( string data )
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

CSVTable::CSVTable( string data, string root )
{
    this->rootdir = root;
    Init( data );
}

CSVTable::CSVTable( VSFileSystem::VSFile &f, string root )
{
    this->rootdir = root;
    Init( f.ReadFull() );
}

CSVRow::CSVRow( CSVTable *parent, string key )
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

bool CSVTable::RowExists( string name, unsigned int &where )
{
    vsUMap< string, int >::iterator i = rows.find( name );
    if ( i == rows.end() )
        return false;
    where = (*i).second;
    return true;
}

bool CSVTable::ColumnExists( string name, unsigned int &where )
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

void CSVTable::SetupOptimizer( vector< string > keys, unsigned int type )
{
    optimizer_setup = true;
    optimizer_type  = type;
    optimizer_keys  = keys;
    optimizer_indexes.resize( keys.size(), CSVTable::optimizer_undefined );
    for (unsigned int i = 0; i < keys.size(); ++i)
        ColumnExists( keys[i], optimizer_indexes[i] );
}

