/**
* base_write_xml.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "base.h"
#ifdef BASE_MAKER
#ifdef BASE_XML
#include <stdio.h>

void begintag( FILE *Fp, const char *tag, int indent )
{
    int i;
    for (i = 0; i < indent; i++)
        VSFileSystem::Write( "\t", sizeof (char), 1, Fp );
    VSFileSystem::Write( "<", sizeof (char), 1, Fp );
    VSFileSystem::Write( tag, sizeof (char)*strlen( tag ), 1, Fp );
}
void midxmltag( FILE *Fp, const char *tag, const float value )
{
    char value2[100] = {0};       //new char [100];
    sprintf( value2, "%g", value );
    VSFileSystem::Write( " ", sizeof (char), 1, Fp );
    VSFileSystem::Write( tag, sizeof (char)*strlen( tag ), 1, Fp );
    VSFileSystem::Write( "=\"", sizeof (char)*2, 1, Fp );
    VSFileSystem::Write( value2, sizeof (char)*strlen( value2 ), 1, Fp );
    VSFileSystem::Write( "\"", sizeof (char)*1, 1, Fp );
//delete value2;
}
void midxmlchar( FILE *Fp, const char *tag, const char *value )
{
    VSFileSystem::Write( " ", sizeof (char), 1, Fp );
    VSFileSystem::Write( tag, sizeof (char)*strlen( tag ), 1, Fp );
    VSFileSystem::Write( "=\"", sizeof (char)*2, 1, Fp );
    VSFileSystem::Write( value, sizeof (char)*strlen( value ), 1, Fp );
    VSFileSystem::Write( "\"", sizeof (char)*1, 1, Fp );
}
void midxmlint( FILE *Fp, const char *tag, const int value )
{
    char value2[100] = {0};
    sprintf( value2, "%d", value );
    VSFileSystem::Write( " ", sizeof (char), 1, Fp );
    VSFileSystem::Write( tag, sizeof (char)*strlen( tag ), 1, Fp );
    VSFileSystem::Write( "=\"", sizeof (char)*2, 1, Fp );
    VSFileSystem::Write( value2, sizeof (char)*strlen( value2 ), 1, Fp );
    VSFileSystem::Write( "\"", sizeof (char)*1, 1, Fp );
}
void midxmlbool( FILE *Fp, const char *tag, const bool value )
{
    char value2[6];
    strcpy( value2, value == true ? "true" : "false" );
    VSFileSystem::Write( " ", sizeof (char), 1, Fp );
    VSFileSystem::Write( tag, sizeof (char)*strlen( tag ), 1, Fp );
    VSFileSystem::Write( "=\"", sizeof (char)*2, 1, Fp );
    VSFileSystem::Write( value2, sizeof (char)*strlen( value2 ), 1, Fp );
    VSFileSystem::Write( "\"", sizeof (char)*1, 1, Fp );
}
void endtag( FILE *Fp, bool end = false )
{
    if (end == true)
        VSFileSystem::Write( "/", sizeof (char), 1, Fp );
    VSFileSystem::Write( ">\n", sizeof (char)*2, 1, Fp );
}

void BaseInterface::Room::Link::EndXML( FILE *fp )
{
    midxmlchar( fp, "Text", text.c_str() );
    midxmltag( fp, "x", x );
    midxmltag( fp, "y", y );
    midxmltag( fp, "wid", wid );
    midxmltag( fp, "hei", hei );
}

void BaseInterface::Room::Python::EndXML( FILE *fp )
{
    begintag( fp, "Python", 2 );
    Link::EndXML( fp );
    midxmlchar( fp, "pythonfile", file.c_str() );
    endtag( fp, true );
}

void BaseInterface::Room::Goto::EndXML( FILE *fp )
{
    begintag( fp, "Link", 2 );
    Link::EndXML( fp );
    midxmlint( fp, "index", index );
    endtag( fp, true );
}

void BaseInterface::Room::Talk::EndXML( FILE *fp )
{
    begintag( fp, "Talk", 2 );
    Link::EndXML( fp );
    endtag( fp, false );
    for (int i = 0; i < say.size(); i++) {
        begintag( fp, "say", 3 );
        for (int j = 0; j < say[i].size(); j++)
            if (say[i][j] == '\n')
                say[i][j] = '\\';
        midxmlchar( fp, "text", say[i].c_str() );
        midxmlchar( fp, "soundfile", soundfiles[i].c_str() );
        endtag( fp, true );
    }
    begintag( fp, "/Talk", 2 );
    endtag( fp, false );
}

void BaseInterface::Room::Launch::EndXML( FILE *fp )
{
    begintag( fp, "Launch", 2 );
    Link::EndXML( fp );
    endtag( fp, true );
}

void BaseInterface::Room::Comp::EndXML( FILE *fp )
{
    begintag( fp, "Comp", 2 );
    Link::EndXML( fp );
    for (int i = 0; i < modes.size(); i++) {
        char *mode = NULL;
        switch (modes[i])
        {
        case BaseComputer::CARGO:
            mode = "Cargo";
            break;
        case BaseComputer::UPGRADE:
            mode = "Upgrade";
            break;
        case BaseComputer::SHIP_DEALER:
            mode = "ShipDealer";
            break;
        case BaseComputer::MISSIONS:
            mode = "Missions";
            break;
        case BaseComputer::NEWS:
            mode = "News";
            break;
        case BaseComputer::INFO:
            mode = "Info";
            break;
        }
        if (mode)
            midxmlchar( fp, mode, "" );
    }
    endtag( fp, true );
}

void BaseInterface::Room::BaseObj::EndXML( FILE *fp )
{
//Do nothing
}

void BaseInterface::Room::BaseShip::EndXML( FILE *fp )
{
    begintag( fp, "Ship", 2 );
    midxmltag( fp, "x", mat.p.i );
    midxmltag( fp, "y", mat.p.j );
    midxmltag( fp, "z", mat.p.k );
    midxmltag( fp, "ri", mat.getR().i );
    midxmltag( fp, "rj", mat.getR().j );
    midxmltag( fp, "rk", mat.getR().k );
    midxmltag( fp, "qi", mat.getQ().i );
    midxmltag( fp, "qj", mat.getQ().j );
    midxmltag( fp, "qk", mat.getQ().k );
    endtag( fp, true );
}

void BaseInterface::Room::BaseVSSprite::EndXML( FILE *fp )
{
    float x, y;
    begintag( fp, "Texture", 2 );
    spr.GetPosition( x, y );
    midxmlchar( fp, "File", texfile.c_str() );
    midxmltag( fp, "x", x );
    midxmltag( fp, "y", y );
    endtag( fp, true );
}

void BaseInterface::Room::EndXML( FILE *fp )
{
    begintag( fp, "Room", 1 );
    midxmlchar( fp, "Text", deftext.c_str() );
    endtag( fp, false );
    int i;
    for (i = 0; i < links.size(); i++)
        if (links[i])
            links[i]->EndXML( fp );
    for (i = 0; i < objs.size(); i++)
        if (objs[i])
            objs[i]->EndXML( fp );
    begintag( fp, "/Room", 1 );
    endtag( fp, false );
}

void BaseInterface::EndXML( FILE *fp )
{
    begintag( fp, "Base", 0 );
    endtag( fp, false );
    for (int i = 0; i < rooms.size(); i++)
        rooms[i]->EndXML( fp );
    begintag( fp, "/Base", 0 );
    endtag( fp, false );
}

#endif
#endif

