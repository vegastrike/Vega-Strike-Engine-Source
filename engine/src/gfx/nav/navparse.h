/**
* navparse.h
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

#include "vsfilesystem.h"

//using namespace VSFileSystem;
using VSFileSystem::CockpitFile;
using VSFileSystem::vs_fprintf;
bool NavigationSystem::ParseFile( string filename )
{
    string expression = "";
    string tag  = "";
    string data = "";
    char   next = ' ';
    int    totalitems = 0;

    //ifstream dataset_file;
    //dataset_file.open("navdata.xml");
    VSFile  f;
    VSError err = f.OpenReadOnly( "nav/navdata.xml", CockpitFile );
    //if (dataset_file.fail())
    if (err > Ok)
        return 0;
    while ( !f.Eof() ) {
        if (next == '<') {
            //trap <*>
            //- know what type it is
            f.Read( &next, 1 );                                         //
            while (next != '>') {
                //
                //
                expression = expression+next;                   //
                f.Read( &next, 1 );                                             //
            }                                                                                   //
            if (expression[0] == '!') {
                //doesnt catch comment in comment
                while ( (expression[expression.size()-1] != '>') || (expression[expression.size()-2] != '-') ) {
                    expression = expression+next;
                    f.Read( &next, 1 );
                }
                expression = "";
                continue;
            }
            if ( (expression[0] != '/') && (expression[expression.size()-1] != '/') ) {
                //starter
                tag = expression;
                expression = "";
            } else if (expression[0] == '/') {
                //terminator = no more to be done
                data = "";
                tag  = "";
                expression = "";
            } else if (expression[expression.size()-1] == '/') {
                //data
                data = expression;
                if (tag == "console") {
                    string mesh_   = retrievedata( data, "file" );
                    float  x_small = atof( ( retrievedata( data, "x_small" ) ).c_str() );
                    float  x_large = atof( ( retrievedata( data, "x_large" ) ).c_str() );
                    float  y_small = atof( ( retrievedata( data, "y_small" ) ).c_str() );
                    float  y_large = atof( ( retrievedata( data, "y_large" ) ).c_str() );
                    float  scale_  = atof( ( retrievedata( data, "scale" ) ).c_str() );
                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    meshcoordinate_x[0] = x_mesh_coord;
                    meshcoordinate_y[0] = y_mesh_coord;
                    meshcoordinate_z[0] = z_mesh_coord;
                    screenskipby4[0]    = x_small;
                    screenskipby4[1]    = x_large;
                    screenskipby4[2]    = y_small;
                    screenskipby4[3]    = y_large;
                    mesh[0] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "button1") {
                    string mesh_  = retrievedata( data, "file" );
                    float  x_     = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float  y_     = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float  dx_    = atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float  dy_    = atof( ( retrievedata( data, "dy" ) ).c_str() );
                    float  scale_ = atof( ( retrievedata( data, "scale" ) ).c_str() );
                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord_delta = atof( ( retrievedata( data, "z_mesh_coord_delta" ) ).c_str() );

                    meshcoordinate_x[1] = x_mesh_coord;
                    meshcoordinate_y[1] = y_mesh_coord;
                    meshcoordinate_z[1] = z_mesh_coord;
                    meshcoordinate_z_delta[1] = z_mesh_coord_delta;
                    buttonskipby4_1[0]  = x_;
                    buttonskipby4_1[1]  = x_+dx_;
                    buttonskipby4_1[2]  = y_;
                    buttonskipby4_1[3]  = y_+dy_;

                    mesh[1] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "button2") {
                    string mesh_  = retrievedata( data, "file" );
                    float  x_     = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float  y_     = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float  dx_    = atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float  dy_    = atof( ( retrievedata( data, "dy" ) ).c_str() );
                    float  scale_ = atof( ( retrievedata( data, "scale" ) ).c_str() );

                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord_delta = atof( ( retrievedata( data, "z_mesh_coord_delta" ) ).c_str() );

                    meshcoordinate_x[2] = x_mesh_coord;
                    meshcoordinate_y[2] = y_mesh_coord;
                    meshcoordinate_z[2] = z_mesh_coord;
                    meshcoordinate_z_delta[2] = z_mesh_coord_delta;

                    buttonskipby4_2[0]  = x_;
                    buttonskipby4_2[1]  = x_+dx_;
                    buttonskipby4_2[2]  = y_;
                    buttonskipby4_2[3]  = y_+dy_;

                    mesh[2] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "button3") {
                    string mesh_  = retrievedata( data, "file" );
                    float  x_     = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float  y_     = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float  dx_    = atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float  dy_    = atof( ( retrievedata( data, "dy" ) ).c_str() );
                    float  scale_ = atof( ( retrievedata( data, "scale" ) ).c_str() );

                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord_delta = atof( ( retrievedata( data, "z_mesh_coord_delta" ) ).c_str() );

                    meshcoordinate_x[3] = x_mesh_coord;
                    meshcoordinate_y[3] = y_mesh_coord;
                    meshcoordinate_z[3] = z_mesh_coord;
                    meshcoordinate_z_delta[3] = z_mesh_coord_delta;

                    buttonskipby4_3[0]  = x_;
                    buttonskipby4_3[1]  = x_+dx_;
                    buttonskipby4_3[2]  = y_;
                    buttonskipby4_3[3]  = y_+dy_;

                    mesh[3] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "button4") {
                    string mesh_  = retrievedata( data, "file" );
                    float  x_     = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float  y_     = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float  dx_    = atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float  dy_    = atof( ( retrievedata( data, "dy" ) ).c_str() );
                    float  scale_ = atof( ( retrievedata( data, "scale" ) ).c_str() );

                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord_delta = atof( ( retrievedata( data, "z_mesh_coord_delta" ) ).c_str() );

                    meshcoordinate_x[4] = x_mesh_coord;
                    meshcoordinate_y[4] = y_mesh_coord;
                    meshcoordinate_z[4] = z_mesh_coord;
                    meshcoordinate_z_delta[4] = z_mesh_coord_delta;

                    buttonskipby4_4[0]  = x_;
                    buttonskipby4_4[1]  = x_+dx_;
                    buttonskipby4_4[2]  = y_;
                    buttonskipby4_4[3]  = y_+dy_;

                    mesh[4] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "button5") {
                    string mesh_  = retrievedata( data, "file" );
                    float  x_     = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float  y_     = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float  dx_    = atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float  dy_    = atof( ( retrievedata( data, "dy" ) ).c_str() );
                    float  scale_ = atof( ( retrievedata( data, "scale" ) ).c_str() );

                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord_delta = atof( ( retrievedata( data, "z_mesh_coord_delta" ) ).c_str() );

                    meshcoordinate_x[5] = x_mesh_coord;
                    meshcoordinate_y[5] = y_mesh_coord;
                    meshcoordinate_z[5] = z_mesh_coord;
                    meshcoordinate_z_delta[5] = z_mesh_coord_delta;

                    buttonskipby4_5[0]  = x_;
                    buttonskipby4_5[1]  = x_+dx_;
                    buttonskipby4_5[2]  = y_;
                    buttonskipby4_5[3]  = y_+dy_;

                    mesh[5] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "button6") {
                    string mesh_  = retrievedata( data, "file" );
                    float  x_     = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float  y_     = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float  dx_    = atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float  dy_    = atof( ( retrievedata( data, "dy" ) ).c_str() );
                    float  scale_ = atof( ( retrievedata( data, "scale" ) ).c_str() );

                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord_delta = atof( ( retrievedata( data, "z_mesh_coord_delta" ) ).c_str() );

                    meshcoordinate_x[6] = x_mesh_coord;
                    meshcoordinate_y[6] = y_mesh_coord;
                    meshcoordinate_z[6] = z_mesh_coord;
                    meshcoordinate_z_delta[6] = z_mesh_coord_delta;

                    buttonskipby4_6[0]  = x_;
                    buttonskipby4_6[1]  = x_+dx_;
                    buttonskipby4_6[2]  = y_;
                    buttonskipby4_6[3]  = y_+dy_;

                    mesh[6] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "button7") {
                    string mesh_  = retrievedata( data, "file" );
                    float  x_     = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float  y_     = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float  dx_    = atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float  dy_    = atof( ( retrievedata( data, "dy" ) ).c_str() );
                    float  scale_ = atof( ( retrievedata( data, "scale" ) ).c_str() );

                    float  x_mesh_coord = atof( ( retrievedata( data, "x_mesh_coord" ) ).c_str() );
                    float  y_mesh_coord = atof( ( retrievedata( data, "y_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord = atof( ( retrievedata( data, "z_mesh_coord" ) ).c_str() );
                    float  z_mesh_coord_delta = atof( ( retrievedata( data, "z_mesh_coord_delta" ) ).c_str() );

                    meshcoordinate_x[7] = x_mesh_coord;
                    meshcoordinate_y[7] = y_mesh_coord;
                    meshcoordinate_z[7] = z_mesh_coord;
                    meshcoordinate_z_delta[7] = z_mesh_coord_delta;

                    buttonskipby4_7[0]  = x_;
                    buttonskipby4_7[1]  = x_+dx_;
                    buttonskipby4_7[2]  = y_;
                    buttonskipby4_7[3]  = y_+dy_;

                    mesh[7] = Mesh::LoadMesh( mesh_.c_str(), Vector( scale_, scale_, scale_ ), 0, NULL );
                } else if (tag == "objectives") {
                    float x_  = atof( ( retrievedata( data, "x" ) ).c_str() );
                    float y_  = atof( ( retrievedata( data, "y" ) ).c_str() );
                    float dx_ = x_+atof( ( retrievedata( data, "dx" ) ).c_str() );
                    float dy_ = y_+atof( ( retrievedata( data, "dy" ) ).c_str() );
                    ScreenToCoord( x_ );
                    ScreenToCoord( y_ );
                    ScreenToCoord( dx_ );
                    ScreenToCoord( dy_ );

                    screen_objectives.SetPos( x_, dy_ );
                    screen_objectives.SetSize( dx_-x_, y_-dy_ );
                    dosetbit( whattodraw, 4 );
                } else if (tag == "systemiteminfo") {
                    float scale_   = atof( ( retrievedata( data, "itemscale" ) ).c_str() );
                    float zmult_   = atof( ( retrievedata( data, "zshiftmultiplier" ) ).c_str() );
                    float zfactor_ = atof( ( retrievedata( data, "itemzscalefactor" ) ).c_str() );
                    float _unselectedalpha      = atof( ( retrievedata( data, "unselectedalpha" ) ).c_str() );
                    float _minimumitemscaledown = atof( ( retrievedata( data, "minimumitemscaledown" ) ).c_str() );
                    float _maximumitemscaleup   = atof( ( retrievedata( data, "maximumitemscaleup" ) ).c_str() );

                    unselectedalpha      = _unselectedalpha;
                    minimumitemscaledown = _minimumitemscaledown;
                    maximumitemscaleup   = _maximumitemscaleup;
                    if (scale_ < 0.5)
                        system_item_scale = 0.5;
                    else if (scale_ > 4)
                        system_item_scale = 4;
                    else
                        system_item_scale = scale_;
                    if (zmult_ < 0.5)
                        zshiftmultiplier = 0.5;
                    else if (zmult_ > 6)
                        zshiftmultiplier = 6;
                    else
                        zshiftmultiplier = zmult_;
                    if (zfactor_ < 1.0)
                        item_zscalefactor = 1.0;
                    else if (zfactor_ > 8)
                        item_zscalefactor = 8;
                    else
                        item_zscalefactor = zfactor_;
                } else if (tag == "dimensions") {
                    int    how_many_sys = atoi( ( retrievedata( data, "system" ) ).c_str() );
                    int    how_many_gal = atoi( ( retrievedata( data, "galaxy" ) ).c_str() );
                    string multi_sys    = retrievedata( data, "systemmultidimensional" );
                    string multi_gal    = retrievedata( data, "galaxymultidimensional" );
                    if (how_many_sys == 3)
                        system_view = VIEW_3D;
                    else
                        system_view = VIEW_2D;
                    if (how_many_gal == 3)
                        galaxy_view = VIEW_3D;
                    else
                        galaxy_view = VIEW_2D;
                    if (multi_sys == "yes")
                        system_multi_dimensional = 1;
                    else
                        system_multi_dimensional = 0;
                    if (multi_gal == "yes")
                        galaxy_multi_dimensional = 1;
                    else
                        galaxy_multi_dimensional = 0;
                } else if (tag == "factioncolours") {
                    string factionname = retrievedata( data, "faction" );
                    float  r_ = atof( ( retrievedata( data, "r" ) ).c_str() );
                    float  g_ = atof( ( retrievedata( data, "g" ) ).c_str() );
                    float  b_ = atof( ( retrievedata( data, "b" ) ).c_str() );
                    float  a_ = atof( ( retrievedata( data, "a" ) ).c_str() );
                    for (unsigned int k = 0; k < FactionUtil::GetNumFactions(); k++)
                        if ( ( FactionUtil::GetFactionName( k ) ) == factionname ) {
                            factioncolours[k].r = r_;
                            factioncolours[k].g = g_;
                            factioncolours[k].b = b_;
                            factioncolours[k].a = a_;
                        }
                } else if (tag == "configmode") {
                    int configmode_ = atoi( ( retrievedata( data, "configmode" ) ).c_str() );
                    configmode = configmode_;
                }
                data = "";
                //tag = "";
                expression = "";
                continue;
            } else {
                continue;
            }
        }
        f.Read( &next, 1 );
        //do something
    }
    f.Close();
    if (totalitems == 0)
        return 1;
    else
        return 0;
}

