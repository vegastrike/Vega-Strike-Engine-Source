/**
 * turret.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "turret.h"

#include "unit_generic.h"

enum class TURRET_SIZE {
    SMALL = 0x0,
    MEDIUM = 0x1,
    LARGE = 0x2,
    CAPSHIP_SMALL = 0x4,
    CAPSHIP_LARGE = 0x8
};

void Tokenize(const string &str, vector<string> &tokens, const string &delimiters = " ") {
    //Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    //Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        //Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        //Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        //Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

std::string CheckBasicSizes(const std::string tokens) {
    if (tokens.find("small") != string::npos) {
        return "small";
    }
    if (tokens.find("medium") != string::npos) {
        return "medium";
    }
    if (tokens.find("large") != string::npos) {
        return "large";
    }
    if (tokens.find("cargo") != string::npos) {
        return "cargo";
    }
    if (tokens.find("LR") != string::npos || tokens.find("massive") != string::npos) {
        return "massive";
    }
    return "";
}

Turret::Turret() {

}

std::map<std::string, std::string> parseTurretSizes() {
    using namespace VSFileSystem;
    std::map<std::string, std::string> t;
    VSFile f;
    VSError err = f.OpenReadOnly("units/subunits/size.txt", UnknownFile);
    if (err <= Ok) {
        int siz = f.Size();
        char *filedata = (char *) malloc(siz + 1);
        filedata[siz] = 0;
        while (f.ReadLine(filedata, siz) == Ok) {
            std::string x(filedata);
            string::size_type len = x.find(",");
            if (len != std::string::npos) {
                std::string y = x.substr(len + 1);
                x = x.substr(0, len);
                len = y.find(",");
                y = y.substr(0, len);
                sscanf(y.c_str(), "%s", filedata);
                y = filedata;
                std::string key(x);
                std::string value(y);
                t[key] = value;
            }
        }
        free(filedata);
        f.Close();
    }
    return t;
}

std::string getTurretSize(const std::string &size) {
    static std::map<std::string, std::string> turretmap = parseTurretSizes();
    std::map<std::string, std::string>::iterator h = turretmap.find(size);
    if (h != turretmap.end()) {
        return (*h).second;
    }
    vector<string> tokens;
    Tokenize(size, tokens, "_");
    for (unsigned int i = 0; i < tokens.size(); ++i) {
        if (tokens[i].find("turret") != string::npos) {
            string temp = CheckBasicSizes(tokens[i]);
            if (!temp.empty()) {
                return temp;
            }
        } else {
            return tokens[i];
        }
    }
    return "capital";
}

Unit *CreateGenericTurret(std::string tur, int faction) {
    return new Unit(tur.c_str(), true, faction, "", 0, 0);
}

/*class VCString : public std::string
{
public: VCString() {}
    VCString( const string &s ) : string( s ) {}
};
std::map< VCString, VCString >parseTurretSizes()
{
    using namespace VSFileSystem;
    std::map< VCString, VCString >t;
    VSFile  f;
    VSError err = f.OpenReadOnly( "units/subunits/size.txt", UnknownFile );
    if (err <= Ok) {
        int   siz = f.Size();
        char *filedata = (char*) malloc( siz+1 );
        filedata[siz] = 0;
        while (f.ReadLine( filedata, siz ) == Ok) {
            std::string x( filedata );
            string::size_type len = x.find( "," );
            if (len != std::string::npos) {
                std::string y = x.substr( len+1 );
                x = x.substr( 0, len );
                len    = y.find( "," );
                y      = y.substr( 0, len );
                sscanf( y.c_str(), "%s", filedata );
                y      = filedata;
                VCString key( x );
                VCString value( y );
                t[key] = value;
            }
        }
        free( filedata );
        f.Close();
    }
    return t;
}

std::string getTurretSize( const std::string &size )
{
    static std::map< VCString, VCString >    turretmap = parseTurretSizes();
    std::map< VCString, VCString >::iterator h = turretmap.find( size );
    if ( h != turretmap.end() )
        return (*h).second;
    vector< string >tokens;
    Tokenize( size, tokens, "_" );
    for (unsigned int i = 0; i < tokens.size(); ++i) {
        if (tokens[i].find( "turret" ) != string::npos) {
            string temp = CheckBasicSizes( tokens[i] );
            if ( !temp.empty() )
                return temp;
        } else {
            return tokens[i];
        }
    }
    return "capital";
}*/

