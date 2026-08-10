/*
 * base_write_python.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "cmd/base.h"
#ifdef BASE_MAKER
//#ifndef BASE_XML
#include <cstdio>
#include <boost/format.hpp>
#include "vega_string_utils.h"
#include "common/vega_random.h"

void BaseInterface::Room::Link::EndXML( FILE *fp )
{
    VSFileSystem::vs_fprintf( fp, "room, '%s', %g, %g, %g, %g, '%s'", index.c_str(), x, y, wid, hei, text.c_str() );
}

void BaseInterface::Room::Goto::EndXML( FILE *fp )
{
    VSFileSystem::vs_fprintf( fp, "Base.Link (" );
    Link::EndXML( fp );
    VSFileSystem::vs_fprintf( fp, ", %d)\n", Goto::index );
}

void BaseInterface::Room::Python::EndXML( FILE *fp )
{
    VSFileSystem::vs_fprintf( fp, "Base.Python (" );
    Link::EndXML( fp );
    VSFileSystem::vs_fprintf( fp, ", '%s')\n", pythonfile.c_str() );
}

void BaseInterface::Room::Talk::EndXML( FILE *fp )
{
    const std::string python_filename = (boost::format("NEW_SCRIPT_%1%.py") % VegaRandom::Instance().GenRandUInt32()).str();
    char *randstr = vega_str_dup(python_filename.c_str());
    VSFileSystem::vs_fprintf( fp, "Base.Python (" );
    Link::EndXML( fp );
    VSFileSystem::vs_fprintf( fp, ", '%s')\n", randstr );
    FILE *py = VSFileSystem::vs_open( randstr, "wt" );
    VSFileSystem::vs_fprintf( py, "import Base\nimport VS\nimport random\n\nrandnum=random.randrange(0,%d)\n", static_cast<int>(say.size()) );
    for (int i = 0; i < say.size(); i++) {
        VSFileSystem::vs_fprintf( fp, "if (randnum==%d):\n", i );
        for (int j = 0; j < say[i].size(); j++)
            if (say[i][j] == '\n') {
                say[i][j] = '\\';
                static const char *ins = "n";
                say[i].insert( j, ins );
            }
        VSFileSystem::vs_fprintf( fp, "  Base.Message ('%s')\n", say[i].c_str() );
        if ( !( soundfiles[i].empty() ) )
            VSFileSystem::vs_fprintf( fp, "  VS.playSound ('%s', (0,0,0), (0,0,0))\n", soundfiles[i].c_str() );
    }
    //obsolete... creates a file that uses the Python function instead.
}

void BaseInterface::Room::Launch::EndXML( FILE *fp )
{
    VSFileSystem::vs_fprintf( fp, "Base.Launch (" );
    Link::EndXML( fp );
    VSFileSystem::vs_fprintf( fp, ")\n" );
}
void BaseInterface::Room::Eject::EndXML( FILE *fp )
{
    VSFileSystem::vs_fprintf( fp, "Base.Eject (" );
    Link::EndXML( fp );
    VSFileSystem::vs_fprintf( fp, ")\n" );
}

void BaseInterface::Room::Comp::EndXML( FILE *fp )
{
    VSFileSystem::vs_fprintf( fp, "Base.Comp (" );
    Link::EndXML( fp );
    VSFileSystem::vs_write( ", '", 3, 1, fp );
    for (const auto each_mode : modes) {
        std::string mode_string;
        switch (modes.at(RAW_DISPLAY_MODE(each_mode))) {
            case DisplayMode::CARGO:
                mode_string = "Cargo";
                break;
            case DisplayMode::UPGRADE:
                mode_string = "Upgrade";
                break;
            case DisplayMode::SHIP_DEALER:
                mode_string = "ShipDealer";
                break;
            case DisplayMode::MISSIONS:
                mode_string = "Missions";
                break;
            case DisplayMode::NEWS:
                mode_string = "News";
                break;
            case DisplayMode::INFO:
                mode_string = "Info";
                break;
            default:
                break;
        }
        if (!mode_string.empty()) {
            VSFileSystem::vs_fprintf( fp, "%s ", mode_string.c_str() );
        }
        if (each_mode == modes.at(modes.size() - 1)) {
            VSFileSystem::vs_fprintf( fp, "'" );
        }
    }
    VSFileSystem::vs_fprintf( fp, ")\n" );
}

void BaseInterface::Room::BaseObj::EndXML( FILE *fp )
{
//Do nothing
}

void BaseInterface::Room::BaseShip::EndXML( FILE *fp )
{
    VSFileSystem::vs_fprintf( fp, "Base.Ship (room, '%s', (%lg,%lg,%lg), (%g, %g, %g), (%g, %g, %g))\n", index.c_str(),
                              mat.p.i, mat.p.j, mat.p.k,
                              mat.getR().i, mat.getR().j, mat.getR().k,
                              mat.getQ().i, mat.getQ().j, mat.getQ().k );
}

void BaseInterface::Room::BaseVSSprite::EndXML( FILE *fp )
{
    float x, y;
    spr.GetPosition( x, y );
    VSFileSystem::vs_fprintf( fp, "Base.Texture (room, '%s', '%s', %g, %g)\n", index.c_str(), texfile.c_str(), x, y );
}

void BaseInterface::Room::EndXML(FILE *fp) const {
    int i = VSFileSystem::vs_fprintf(fp, "room = Base.Room ('%s')\n", deftext.c_str());
    for (i = 0; i < links.size(); i++) {
        if (links[i]) {
            links[i]->EndXML(fp);
        }
    }
    for (i = 0; i < objs.size(); i++) {
        if (objs[i]) {
            objs[i]->EndXML(fp);
        }
    }
    VSFileSystem::vs_fprintf(fp, "\n");
    fflush(fp);
}

void BaseInterface::EndXML(FILE *fp) const {
    VSFileSystem::vs_fprintf(fp, "import Base\n\n");
    for (int i = 0; i < rooms.size(); i++) {
        rooms[i]->EndXML(fp);
    }
}

//#endif
#endif

