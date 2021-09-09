/**
* dattoxml.cpp
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

#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <string.h>
#include <math.h>
#endif
using namespace std;
char FileName[256];
struct LVector
{
    float i;
    float j;
    float k;
};
enum GUNS {FLUX, LASER, MASSDRIVER, MESON, NEUTRON, ION, PARTICLE, TACH, PLASMA, REAPER, PHOTON, ANTIMATTER, STORMFIRE, LEECH};
enum MISS {DRAGONFLY, DUMBFIRE, HS, IR, FF, TORP, SWARM, TRACKER, LEECHMIS, MACE};

struct V
{
    float x, y, z, i, j, k, s, t;
};
short readf( FILE *fp )
{
    short int n;
    fread( &n, sizeof (n), 1, fp );
    return n;
}

int readi( FILE *fp )
{
    int n;
    fread( &n, sizeof (n), 1, fp );
    return n;
}

float rf( FILE *fp )
{
    float n;
    fread( &n, sizeof (n), 1, fp );
    return n;
}
struct shipstats
{
    LVector *GunOffset;
    LVector *MissileOffset;
    unsigned char *MissileType;
    unsigned char *GunType;
    unsigned char *GunRefire;
    float LastMissileFire;
    float *LastFire;
    unsigned char *MissileAmmo;
    char GunMode;
    char MissileMode;
    unsigned char  ShieldR, ShieldG, ShieldB;
    float  MaxSpeed, MaxAftSpd;
    float  Accel, ABAccel;
    short  ShieldSize;
    float *Shield;
    float *MaxShield;
    float  MaxHull;
    short  MaxArmor[4];
    float  ShieldRecharge;
    char   NumShieldFacing;
    int    WeapSpd;
    char   NumGuns, NumMissiles;
    short  Armor[4];
    float  Hull;

    float  Yaw, Pitch, Roll;
    float  HullResistance;
    float  Mass;
    float  ooMass;
};
const int labelinc = 5;
struct point
{
    float   x, y, z;
    V       Vertex;
    LVector Normal;
    void    init( float x, float y, float z )
    {
        this->x = this->Vertex.x = x;
        this->y = this->Vertex.y = y;
        this->z = this->Vertex.z = z;
    }
};
struct Glow
{
    short conerad;
    short maxradius;
    short minradius;
    float currentradius;
    float increasing;
    point V[9];      //center plus 12 points
};
struct drawdata
{
    short  NumPoints;
    point *FixP;
    point *VarP;
    unsigned char **TriTexture;
    unsigned char **QuadTexture;
    unsigned char **PentTexture;
    unsigned char **HexTexture;
    char   *DecalFileName;
    short   NumTris, NumQuads, NumPents, NumHexs;
    short **Tris;
    short **Quads;
    short **Pents;
    short **Hexs;
    short   NumRevTris, NumRevQuads, NumRevPents, NumRevHexs;
    short **RevTris;
    short **RevQuads;
    short **RevPents;
    short **RevHexs;
    short   NumLines;
    short **Lins;
    short   numengines;
    Glow   *Engine;
};

FILE *fp = NULL;
const bool writenormals = true;
void wrtc( char c )
{
    fwrite( &c, sizeof (char), 1, fp );
}
void wrts( short s )
{
    fwrite( &s, sizeof (short), 1, fp );
}
void wrti( int i )
{
    fwrite( &i, sizeof (int), 1, fp );
}
void wrtf( float f )
{
    fwrite( &f, sizeof (float), 1, fp );
}
void wrtd( double d )
{
    fwrite( &d, sizeof (double), 1, fp );
}
void Tab( int i = 1 )
{
    for (int j = 0; j < i; j++)
        wrtc( '\t' );
}
void StrWrite( char *x )
{
    fwrite( x, strlen( x ), 1, fp );
}

void TextI( int i )
{
    char intt[256];
    sprintf( intt, "\"%d\"", i );
    StrWrite( intt );
}
void TextF( float f )
{
    char flt[256];
    sprintf( flt, "\"%f\"", f );
    StrWrite( flt );
}
void Tag( char *t )
{
    wrtc( '<' );
    StrWrite( t );
    wrtc( '>' );
    wrtc( '\n' );
}
void ETag( char *t )
{
    wrtc( '<' );
    wrtc( '/' );
    StrWrite( t );
    wrtc( '>' );
    wrtc( '\n' );
}
void TextP( float x, float y, float z )
{
    StrWrite( "<Location x=" );
    TextF( x );
    StrWrite( " y=" );
    TextF( y );
    StrWrite( " z=" );
    TextF( z );
    StrWrite( "/>\n" );
}
void TextN( float x, float y, float z )
{
    StrWrite( "<Normal i=" );
    TextF( x );
    StrWrite( " j=" );
    TextF( y );
    StrWrite( " k=" );
    TextF( z );
    StrWrite( "/>\n" );
}
void Vert( int point, float s, float t )
{
    StrWrite( "<Vertex point=" );
    TextI( point );
    StrWrite( " s=" );
    TextF( s );
    StrWrite( " t=" );
    TextF( t );
    StrWrite( "/>\n" );
}

int main( int argc, char **argv )
{
    int       i;
    int      *revpnts = NULL;
    drawdata  DrawDat;
    memset( (void*) &DrawDat, 0, sizeof (drawdata) );
    shipstats Stat;
    memset( (void*) &Stat, 0, sizeof (shipstats) );
    float     scl = 1;
    char     *DecalFileName;
    char      tmpprint[1000];
    int       pointcount = 0;
    if (argc < 3)
        return 0;
    if (argc > 3)
        sscanf( argv[3], "%f", &scl );
    else
        scl = .1;
    bool vegaclassic = true;
    bool interactive = false;
    if (argc > 4) {
        vegaclassic = false;
        interactive = (argv[4][0] == 'i' || argv[4][1] == 'i');
    }
    FILE *shp = fopen( argv[1], "r+b" );

    char  meshname[256];
    strcpy( meshname, argv[2] );
    meshname[strlen( meshname )-6] = '\0';
    strcat( meshname, ".xmesh" );
    char  tmp[255];
    fp = fopen( meshname, "w+b" );
    short texfilenamesize = vegaclassic ? readf( shp ) : readi( shp );
    if (texfilenamesize > 0) {
        DrawDat.DecalFileName = DecalFileName = new char[texfilenamesize+5];          //.3df/0
        fread( DecalFileName, texfilenamesize, 1, shp );
        DecalFileName[texfilenamesize]   = '.';
        DecalFileName[texfilenamesize+1] = 'b';
        DecalFileName[texfilenamesize+2] = 'm';
        DecalFileName[texfilenamesize+3] = 'p';
        DecalFileName[texfilenamesize+4] = '\0';
    }
    StrWrite( "<Mesh texture=\"" );
    StrWrite( DecalFileName );
    StrWrite( "\" scale=" );
    TextF( scl );
    StrWrite( " >\n" );
    Tag( "Points" );
    if (vegaclassic) {
        DrawDat.NumPoints = readf( shp );
        DrawDat.VarP = new point[DrawDat.NumPoints];
        //Find how many points and what type;
        DrawDat.FixP = new point[DrawDat.NumPoints];
        for (i = 0; i < DrawDat.NumPoints; i++) {
            Tab( 1 );
            if (i%labelinc == 0) {
                sprintf( tmpprint, "Point> <!-- %d --", pointcount );
                pointcount += labelinc;
                Tag( tmpprint );
            } else {
                Tag( "Point" );
            }
            DrawDat.FixP[i].Vertex.x = -readf( shp );
            DrawDat.FixP[i].Vertex.y = -readf( shp );
            DrawDat.FixP[i].Vertex.z = readf( shp );
            Tab( 2 );
            TextP( DrawDat.FixP[i].Vertex.x, DrawDat.FixP[i].Vertex.y, DrawDat.FixP[i].Vertex.z );
            DrawDat.FixP[i].Normal.i = -( (double) readf( shp ) )/10000;
            DrawDat.FixP[i].Normal.j = -( (double) readf( shp ) )/10000;
            DrawDat.FixP[i].Normal.k = ( (double) readf( shp ) )/10000;
            if (writenormals) {
                Tab( 2 );
                TextN( DrawDat.FixP[i].Normal.i, DrawDat.FixP[i].Normal.j, DrawDat.FixP[i].Normal.k );
            }
            Tab( 1 );
            ETag( "Point" );
        }
        //find out the number of planes and which points make them up

        DrawDat.NumTris = readf( shp );
        DrawDat.Tris    = new short*[DrawDat.NumTris];
        for (i = 0; i < DrawDat.NumTris; i++)
            DrawDat.Tris[i] = new short[3];
        for (i = 0; i < DrawDat.NumTris; i++)
            for (int j = 0; j < 3; j++)
                DrawDat.Tris[i][j] = readf( shp );
        DrawDat.NumQuads = readf( shp );
        DrawDat.Quads    = new short*[DrawDat.NumQuads];
        for (i = 0; i < DrawDat.NumQuads; i++)
            DrawDat.Quads[i] = new short[4];
        for (i = 0; i < DrawDat.NumQuads; i++)
            for (int j = 0; j < 4; j++)
                DrawDat.Quads[i][j] = readf( shp );
        DrawDat.NumPents = readf( shp );
        DrawDat.Pents    = new short*[DrawDat.NumPents];
        for (i = 0; i < DrawDat.NumPents; i++)
            DrawDat.Pents[i] = new short[5];
        for (i = 0; i < DrawDat.NumPents; i++)
            for (int j = 0; j < 5; j++)
                DrawDat.Pents[i][j] = readf( shp );
        DrawDat.NumHexs = readf( shp );
        DrawDat.Hexs    = new short*[DrawDat.NumHexs];
        for (i = 0; i < DrawDat.NumHexs; i++)
            DrawDat.Hexs[i] = new short[6];
        for (i = 0; i < DrawDat.NumHexs; i++)
            for (int j = 0; j < 6; j++)
                DrawDat.Hexs[i][j] = readf( shp );
        revpnts = new int[DrawDat.NumPoints];
        for (i = 0; i < DrawDat.NumPoints; i++)
            revpnts[i] = -1;
        DrawDat.NumRevTris = readf( shp );
        DrawDat.RevTris    = new short*[DrawDat.NumRevTris];
        int i;
        for (i = 0; i < DrawDat.NumRevTris; i++)
            DrawDat.RevTris[i] = new short[3];
        for (i = 0; i < DrawDat.NumRevTris; i++)
            for (int j = 0; j < 3; j++) {
                DrawDat.RevTris[i][j] = readf( shp );
                revpnts[DrawDat.RevTris[i][j]] = 1;
            }
        DrawDat.NumRevQuads = readf( shp );
        DrawDat.RevQuads    = new short*[DrawDat.NumRevQuads];
        for (i = 0; i < DrawDat.NumRevQuads; i++)
            DrawDat.RevQuads[i] = new short[4];
        for (i = 0; i < DrawDat.NumRevQuads; i++)
            for (int j = 0; j < 4; j++) {
                DrawDat.RevQuads[i][j] = readf( shp );
                revpnts[DrawDat.RevQuads[i][j]] = 1;
            }
        DrawDat.NumRevPents = readf( shp );
        DrawDat.RevPents    = new short*[DrawDat.NumRevPents];
        for (i = 0; i < DrawDat.NumRevPents; i++)
            DrawDat.RevPents[i] = new short[5];
        for (i = 0; i < DrawDat.NumRevPents; i++)
            for (int j = 0; j < 5; j++) {
                DrawDat.RevPents[i][j] = readf( shp );
                revpnts[DrawDat.RevPents[i][j]] = 1;
            }
        DrawDat.NumRevHexs = readf( shp );
        DrawDat.RevHexs    = new short*[DrawDat.NumRevHexs];
        for (i = 0; i < DrawDat.NumRevHexs; i++)
            DrawDat.RevHexs[i] = new short[6];
        for (i = 0; i < DrawDat.NumRevHexs; i++)
            for (int j = 0; j < 6; j++) {
                DrawDat.RevHexs[i][j] = readf( shp );
                revpnts[DrawDat.RevHexs[i][j]] = 1;
            }
        int maxrev = 0;
        StrWrite( "<!-- BeginReversePoints -->\n" );
        for (i = 0; i < DrawDat.NumPoints; i++)
            if (revpnts[i] == 1) {
                revpnts[i] = maxrev;                 //so now we have a lookup into revpnts
                maxrev++;
                Tab( 1 );
                if (i%labelinc == 0) {
                    sprintf( tmpprint, "Point> <!-- %d --", pointcount );
                    pointcount += labelinc;
                    Tag( tmpprint );
                } else {
                    Tag( "Point" );
                }
                Tab( 2 );
                TextP( DrawDat.FixP[i].Vertex.x, DrawDat.FixP[i].Vertex.y, DrawDat.FixP[i].Vertex.z );
                if (writenormals) {
                    Tab( 2 );
                    TextN( DrawDat.FixP[i].Normal.i, DrawDat.FixP[i].Normal.j, DrawDat.FixP[i].Normal.k );
                }
                Tab( 1 );
                ETag( "Point" );
            }
        ETag( "Points" );
        DrawDat.NumLines = readf( shp );
        DrawDat.Lins     = new short*[DrawDat.NumLines];
        for (i = 0; i < DrawDat.NumLines; i++)
            DrawDat.Lins[i] = new short[2];
        for (i = 0; i < DrawDat.NumLines; i++) {
            DrawDat.Lins[i][0] = readf( shp );
            DrawDat.Lins[i][1] = readf( shp );
        }
        if (DrawDat.DecalFileName) {
            DrawDat.TriTexture  = new unsigned char*[DrawDat.NumTris+DrawDat.NumRevTris];
            for (i = 0; i < DrawDat.NumTris+DrawDat.NumRevTris; i++)
                DrawDat.TriTexture[i] = new unsigned char[6];
            DrawDat.QuadTexture = new unsigned char*[DrawDat.NumQuads+DrawDat.NumRevQuads];
            for (i = 0; i < DrawDat.NumQuads+DrawDat.NumRevQuads; i++)
                DrawDat.QuadTexture[i] = new unsigned char[8];
            DrawDat.PentTexture = new unsigned char*[DrawDat.NumPents+DrawDat.NumRevPents];
            for (i = 0; i < DrawDat.NumPents+DrawDat.NumRevPents; i++)
                DrawDat.PentTexture[i] = new unsigned char[10];
            DrawDat.HexTexture  = new unsigned char*[DrawDat.NumHexs+DrawDat.NumRevHexs];
            for (i = 0; i < DrawDat.NumHexs+DrawDat.NumRevHexs; i++)
                DrawDat.HexTexture[i] = new unsigned char[12];
            //each polygon has a texture with these coords
            for (i = 0; i < (DrawDat.NumTris+DrawDat.NumRevTris); i++)
                for (int j = 0; j < 6; j++)
                    fread( &DrawDat.TriTexture[i][j], sizeof (unsigned char), 1, shp );
            for (i = 0; i < (DrawDat.NumQuads+DrawDat.NumRevQuads); i++)
                for (int j = 0; j < 8; j++)
                    fread( &DrawDat.QuadTexture[i][j], sizeof (unsigned char), 1, shp );
            for (i = 0; i < (DrawDat.NumPents+DrawDat.NumRevPents); i++)
                for (int j = 0; j < 10; j++)
                    fread( &DrawDat.PentTexture[i][j], sizeof (unsigned char), 1, shp );
            for (i = 0; i < (DrawDat.NumHexs+DrawDat.NumRevHexs); i++)
                for (int j = 0; j < 12; j++)
                    fread( &DrawDat.HexTexture[i][j], sizeof (unsigned char), 1, shp );
        }
    } else {
        //vega advanced
        DrawDat.NumPoints = readi( shp );
        DrawDat.VarP = new point[DrawDat.NumPoints];
        DrawDat.FixP = new point[DrawDat.NumPoints];
        for (i = 0; i < DrawDat.NumPoints; i++) {
            Tab( 1 );
            Tag( "Point" );
            DrawDat.FixP[i].Vertex.x = rf( shp );
            DrawDat.FixP[i].Vertex.y = rf( shp );
            DrawDat.FixP[i].Vertex.z = rf( shp );
            Tab( 2 );
            TextP( DrawDat.FixP[i].Vertex.x, DrawDat.FixP[i].Vertex.y, DrawDat.FixP[i].Vertex.z );
            DrawDat.FixP[i].Normal.i = ( (double) rf( shp ) );
            DrawDat.FixP[i].Normal.j = ( (double) rf( shp ) );
            DrawDat.FixP[i].Normal.k = ( (double) rf( shp ) );
            if (writenormals) {
                Tab( 2 );
                TextN( DrawDat.FixP[i].Normal.i, DrawDat.FixP[i].Normal.j, DrawDat.FixP[i].Normal.k );
            }
            Tab( 1 );
            ETag( "Point" );
        }
        ETag( "Points" );
        DrawDat.NumTris = readi( shp );
        DrawDat.Tris    = new short*[DrawDat.NumTris];
        for (i = 0; i < DrawDat.NumTris; i++)
            DrawDat.Tris[i] = new short[3];
        for (i = 0; i < DrawDat.NumTris; i++)
            for (int j = 0; j < 3; j++)
                DrawDat.Tris[i][j] = readi( shp );
        DrawDat.NumQuads = readi( shp );
        DrawDat.Quads    = new short*[DrawDat.NumQuads];
        for (i = 0; i < DrawDat.NumQuads; i++)
            DrawDat.Quads[i] = new short[4];
        for (i = 0; i < DrawDat.NumQuads; i++)
            for (int j = 0; j < 4; j++)
                DrawDat.Quads[i][j] = readi( shp );
        /*
         *  DrawDat.NumPents = readi (shp);
         *  DrawDat.Pents = new short * [DrawDat.NumPents];
         *  for (i=0; i<DrawDat.NumPents; i++)
         *  DrawDat.Pents[i] = new short [5];
         *  for (i=0; i<DrawDat.NumPents; i++)
         *  for (int j=0; j<5; j++)
         *   DrawDat.Pents[i][j] = readi(shp);
         *
         *  DrawDat.NumHexs = readi (shp);
         *  DrawDat.Hexs = new short * [DrawDat.NumHexs];
         *  for (i=0; i<DrawDat.NumHexs; i++)
         *  DrawDat.Hexs[i] = new short [6];
         *  for (i=0; i<DrawDat.NumHexs; i++)
         *  for (int j=0; j<6; j++)
         *   DrawDat.Hexs[i][j] = readi(shp);
         *
         *  DrawDat.NumLines = readi (shp);
         *  DrawDat.Lins = new short * [DrawDat.NumLines];
         *  for (i=0; i<DrawDat.NumLines; i++)
         *  DrawDat.Lins[i] = new short [2];
         *  for (i=0; i<DrawDat.NumLines; i++) {
         *  DrawDat.Lins[i][0] = readi(shp);
         *  DrawDat.Lins[i][1] = readi(shp);
         *  }
         *  Stat.NumGuns = DrawDat.NumLines;
         *  Stat.GunType = new unsigned char [Stat.NumGuns];
         *  for (i=0; i<Stat.NumGuns; i++)
         *  Stat.GunType[i] = 14;//laser
         *  Stat.GunOffset = new LVector [Stat.NumGuns];
         */
        for (i = 0; i < Stat.NumGuns; i++) {
            if (DrawDat.FixP[DrawDat.Lins[i][0]].Vertex.z > DrawDat.FixP[DrawDat.Lins[i][1]].Vertex.z) {
                Stat.GunOffset[i].i = DrawDat.FixP[DrawDat.Lins[i][0]].Vertex.x;
                Stat.GunOffset[i].j = DrawDat.FixP[DrawDat.Lins[i][0]].Vertex.y;
                Stat.GunOffset[i].k = DrawDat.FixP[DrawDat.Lins[i][0]].Vertex.z;
            } else {
                Stat.GunOffset[i].i = DrawDat.FixP[DrawDat.Lins[i][1]].Vertex.x;
                Stat.GunOffset[i].j = DrawDat.FixP[DrawDat.Lins[i][1]].Vertex.y;
                Stat.GunOffset[i].k = DrawDat.FixP[DrawDat.Lins[i][1]].Vertex.z;
            }
        }
        DrawDat.NumRevHexs  = 0;
        DrawDat.NumRevQuads = 0;
        DrawDat.NumRevPents = 0;
        DrawDat.NumRevTris  = 0;
        //neew
        if (DrawDat.DecalFileName) {
            DrawDat.TriTexture  = new unsigned char*[DrawDat.NumTris+DrawDat.NumRevTris];
            for (i = 0; i < DrawDat.NumTris+DrawDat.NumRevTris; i++)
                DrawDat.TriTexture[i] = new unsigned char[6];
            DrawDat.QuadTexture = new unsigned char*[DrawDat.NumQuads+DrawDat.NumRevQuads];
            for (i = 0; i < DrawDat.NumQuads+DrawDat.NumRevQuads; i++)
                DrawDat.QuadTexture[i] = new unsigned char[8];
            DrawDat.PentTexture = new unsigned char*[DrawDat.NumPents+DrawDat.NumRevPents];
            for (i = 0; i < DrawDat.NumPents+DrawDat.NumRevPents; i++)
                DrawDat.PentTexture[i] = new unsigned char[10];
            DrawDat.HexTexture  = new unsigned char*[DrawDat.NumHexs+DrawDat.NumRevHexs];
            for (i = 0; i < DrawDat.NumHexs+DrawDat.NumRevHexs; i++)
                DrawDat.HexTexture[i] = new unsigned char[12];
            //each polygon has a texture with these coords
            float cachunk;
            for (i = 0; i < (DrawDat.NumTris+DrawDat.NumRevTris); i++)
                for (int j = 0; j < 6; j++) {
                    fread( &cachunk, sizeof (float), 1, shp );
                    DrawDat.TriTexture[i][j] = (unsigned char) (cachunk*256);
                }
            for (i = 0; i < (DrawDat.NumQuads+DrawDat.NumRevQuads); i++)
                for (int j = 0; j < 8; j++) {
                    fread( &cachunk, sizeof (float), 1, shp );
                    DrawDat.QuadTexture[i][j] = (unsigned char) (cachunk*256);
                }
        }
    }
    const float oo255 = 1./255;
    Tag( "Polygons" );
    for (i = 0; i < DrawDat.NumLines; i++) {
        Tag( "Line" );
        Vert( DrawDat.Lins[i][0], 0, 0 );
        Vert( DrawDat.Lins[i][1], 0, 0 );
        ETag( "Line" );
    }
    for (i = 0; i < DrawDat.NumTris; i++) {
        Tag( "Tri" );
        Vert( DrawDat.Tris[i][2],
              DrawDat.TriTexture[i][4]*oo255,
              DrawDat.TriTexture[i][5]*oo255 );
        Vert( DrawDat.Tris[i][1],
              DrawDat.TriTexture[i][2]*oo255,
              DrawDat.TriTexture[i][3]*oo255 );
        Vert( DrawDat.Tris[i][0],
              DrawDat.TriTexture[i][0]*oo255,
              DrawDat.TriTexture[i][1]*oo255 );

        ETag( "Tri" );
    }
    for (i = 0; i < DrawDat.NumRevTris; i++) {
        Tag( "Tri" );
        Vert( revpnts[DrawDat.RevTris[i][2]]+DrawDat.NumPoints,
              DrawDat.TriTexture[i+DrawDat.NumTris][4]*oo255,
              DrawDat.TriTexture[i+DrawDat.NumTris][5]*oo255 );
        Vert( revpnts[DrawDat.RevTris[i][1]]+DrawDat.NumPoints,
              DrawDat.TriTexture[i+DrawDat.NumTris][2]*oo255,
              DrawDat.TriTexture[i+DrawDat.NumTris][3]*oo255 );
        Vert( revpnts[DrawDat.RevTris[i][0]]+DrawDat.NumPoints,
              DrawDat.TriTexture[i+DrawDat.NumTris][0]*oo255,
              DrawDat.TriTexture[i+DrawDat.NumTris][1]*oo255 );

        ETag( "Tri" );
    }
    for (i = 0; i < DrawDat.NumPents; i++) {
        Tag( "Tri" );
        Vert( DrawDat.Pents[i][2],
              DrawDat.PentTexture[i][4]*oo255,
              DrawDat.PentTexture[i][5]*oo255 );
        Vert( DrawDat.Pents[i][1],
              DrawDat.PentTexture[i][2]*oo255,
              DrawDat.PentTexture[i][3]*oo255 );
        Vert( DrawDat.Pents[i][0],
              DrawDat.PentTexture[i][0]*oo255,
              DrawDat.PentTexture[i][1]*oo255 );

        ETag( "Tri" );
    }
    for (i = 0; i < DrawDat.NumRevPents; i++) {
        Tag( "Tri" );
        Vert( revpnts[DrawDat.RevPents[i][2]]+DrawDat.NumPoints,
              DrawDat.PentTexture[i+DrawDat.NumPents][4]*oo255,
              DrawDat.PentTexture[i+DrawDat.NumPents][5]*oo255 );
        Vert( revpnts[DrawDat.RevPents[i][1]]+DrawDat.NumPoints,
              DrawDat.PentTexture[i+DrawDat.NumPents][2]*oo255,
              DrawDat.PentTexture[i+DrawDat.NumPents][3]*oo255 );
        Vert( revpnts[DrawDat.RevPents[i][0]]+DrawDat.NumPoints,
              DrawDat.PentTexture[i+DrawDat.NumPents][0]*oo255,
              DrawDat.PentTexture[i+DrawDat.NumPents][1]*oo255 );

        ETag( "Tri" );
    }
    for (i = 0; i < DrawDat.NumQuads; i++) {
        Tag( "Quad" );
        Vert( DrawDat.Quads[i][3],
              DrawDat.QuadTexture[i][6]*oo255,
              DrawDat.QuadTexture[i][7]*oo255 );
        Vert( DrawDat.Quads[i][2],
              DrawDat.QuadTexture[i][4]*oo255,
              DrawDat.QuadTexture[i][5]*oo255 );
        Vert( DrawDat.Quads[i][1],
              DrawDat.QuadTexture[i][2]*oo255,
              DrawDat.QuadTexture[i][3]*oo255 );
        Vert( DrawDat.Quads[i][0],
              DrawDat.QuadTexture[i][0]*oo255,
              DrawDat.QuadTexture[i][1]*oo255 );

        ETag( "Quad" );
    }
    for (i = 0; i < DrawDat.NumRevQuads; i++) {
        Tag( "Quad" );
        Vert( revpnts[DrawDat.RevQuads[i][3]]+DrawDat.NumPoints,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][6]*oo255,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][7]*oo255 );
        Vert( revpnts[DrawDat.RevQuads[i][2]]+DrawDat.NumPoints,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][4]*oo255,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][5]*oo255 );
        Vert( revpnts[DrawDat.RevQuads[i][1]]+DrawDat.NumPoints,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][2]*oo255,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][3]*oo255 );
        Vert( revpnts[DrawDat.RevQuads[i][0]]+DrawDat.NumPoints,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][0]*oo255,
              DrawDat.QuadTexture[i+DrawDat.NumQuads][1]*oo255 );

        ETag( "Quad" );
    }
    for (i = 0; i < DrawDat.NumPents; i++) {
        Tag( "Quad" );
        Vert( DrawDat.Pents[i][4],
              DrawDat.PentTexture[i][8]*oo255,
              DrawDat.PentTexture[i][9]*oo255 );

        Vert( DrawDat.Pents[i][3],
              DrawDat.PentTexture[i][6]*oo255,
              DrawDat.PentTexture[i][7]*oo255 );

        Vert( DrawDat.Pents[i][2],
              DrawDat.PentTexture[i][4]*oo255,
              DrawDat.PentTexture[i][5]*oo255 );

        Vert( DrawDat.Pents[i][0],
              DrawDat.PentTexture[i][0]*oo255,
              DrawDat.PentTexture[i][1]*oo255 );

        ETag( "Quad" );
    }
    for (i = 0; i < DrawDat.NumRevPents; i++) {
        Tag( "Quad" );
        Vert( revpnts[DrawDat.RevPents[i][4]]+DrawDat.NumPoints,
              DrawDat.PentTexture[i+DrawDat.NumPents][8]*oo255,
              DrawDat.PentTexture[i+DrawDat.NumPents][9]*oo255 );
        Vert( revpnts[DrawDat.RevPents[i][3]]+DrawDat.NumPoints,
              DrawDat.PentTexture[i+DrawDat.NumPents][6]*oo255,
              DrawDat.PentTexture[i+DrawDat.NumPents][7]*oo255 );
        Vert( revpnts[DrawDat.RevPents[i][2]]+DrawDat.NumPoints,
              DrawDat.PentTexture[i+DrawDat.NumPents][4]*oo255,
              DrawDat.PentTexture[i+DrawDat.NumPents][5]*oo255 );
        Vert( revpnts[DrawDat.RevPents[i][0]]+DrawDat.NumPoints,
              DrawDat.PentTexture[i+DrawDat.NumPents][0]*oo255,
              DrawDat.PentTexture[i+DrawDat.NumPents][1]*oo255 );

        ETag( "Quad" );
    }
    for (i = 0; i < DrawDat.NumHexs; i++) {
        Tag( "Quad" );
        Vert( DrawDat.Hexs[i][3],
              DrawDat.HexTexture[i][6]*oo255,
              DrawDat.HexTexture[i][7]*oo255 );

        Vert( DrawDat.Hexs[i][2],
              DrawDat.HexTexture[i][4]*oo255,
              DrawDat.HexTexture[i][5]*oo255 );

        Vert( DrawDat.Hexs[i][1],
              DrawDat.HexTexture[i][2]*oo255,
              DrawDat.HexTexture[i][3]*oo255 );

        Vert( DrawDat.Hexs[i][0],
              DrawDat.HexTexture[i][0]*oo255,
              DrawDat.HexTexture[i][1]*oo255 );

        ETag( "Quad" );
        Tag( "Quad" );
        Vert( DrawDat.Hexs[i][5],
              DrawDat.HexTexture[i][10]*oo255,
              DrawDat.HexTexture[i][11]*oo255 );
        Vert( DrawDat.Hexs[i][4],
              DrawDat.HexTexture[i][8]*oo255,
              DrawDat.HexTexture[i][9]*oo255 );
        Vert( DrawDat.Hexs[i][3],
              DrawDat.HexTexture[i][6]*oo255,
              DrawDat.HexTexture[i][7]*oo255 );
        Vert( DrawDat.Hexs[i][0],
              DrawDat.HexTexture[i][0]*oo255,
              DrawDat.HexTexture[i][1]*oo255 );

        ETag( "Quad" );
    }
    for (i = 0; i < DrawDat.NumRevHexs; i++) {
        Tag( "Quad" );
        Vert( revpnts[DrawDat.RevHexs[i][3]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][6]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][7]*oo255 );
        Vert( revpnts[DrawDat.RevHexs[i][2]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][4]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][5]*oo255 );
        Vert( revpnts[DrawDat.RevHexs[i][1]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][2]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][3]*oo255 );
        Vert( revpnts[DrawDat.RevHexs[i][0]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][0]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][1]*oo255 );

        ETag( "Quad" );
        Tag( "Quad" );
        Vert( revpnts[DrawDat.RevHexs[i][5]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][10]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][11]*oo255 );
        Vert( revpnts[DrawDat.RevHexs[i][4]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][8]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][9]*oo255 );
        Vert( revpnts[DrawDat.RevHexs[i][3]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][6]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][7]*oo255 );
        Vert( revpnts[DrawDat.RevHexs[i][0]]+DrawDat.NumPoints,
              DrawDat.HexTexture[i+DrawDat.NumHexs][0]*oo255,
              DrawDat.HexTexture[i+DrawDat.NumHexs][1]*oo255 );

        ETag( "Quad" );
    }
    ETag( "Polygons" );
    ETag( "Mesh" );
error:
    if (vegaclassic) {
        const float r3o2 = .866025403784;
        const float r2o2 = .707106781187;
        DrawDat.numengines = readf( shp );
        DrawDat.Engine     = new Glow[DrawDat.numengines];
        for (i = 0; i < DrawDat.numengines; i++) {
            short tx = -readf( shp );
            short ty = -readf( shp );
            short tz = readf( shp );
            DrawDat.Engine[i].V[0].init( tx, ty, tz );
            DrawDat.Engine[i].minradius     = readf( shp );
            DrawDat.Engine[i].maxradius     = readf( shp );
            DrawDat.Engine[i].conerad       = readf( shp );
            short ttt = DrawDat.Engine[i].minradius;
            DrawDat.Engine[i].currentradius = DrawDat.Engine[i].minradius;
            DrawDat.Engine[i].increasing    = 1;
            if (DrawDat.Engine[i].minradius > DrawDat.Engine[i].maxradius) {
                DrawDat.Engine[i].minradius  = DrawDat.Engine[i].maxradius;
                DrawDat.Engine[i].maxradius  = ttt;
                ttt = DrawDat.Engine[i].minradius;
                DrawDat.Engine[i].increasing = -1;
            }
            DrawDat.Engine[i].V[1].init( tx-ttt, ty, tz );             //left
            DrawDat.Engine[i].V[2].init( tx-r2o2*ttt, ty+r2o2*ttt, tz );
            DrawDat.Engine[i].V[3].init( tx, ty+ttt, tz );
            DrawDat.Engine[i].V[4].init( tx+r2o2*ttt, ty+r2o2*ttt, tz );
            DrawDat.Engine[i].V[5].init( tx+ttt, ty, tz );
            DrawDat.Engine[i].V[6].init( tx+r2o2*ttt, ty-r2o2*ttt, tz );
            DrawDat.Engine[i].V[7].init( tx, ty-ttt, tz );
            DrawDat.Engine[i].V[8].init( tx-r2o2*ttt, ty-r2o2*ttt, tz );

            //DrawDat.Engine[i].V[10].init (tx, ty-ttt, tz);
            //DrawDat.Engine[i].V[11].init (tx-.5*ttt, ty-r3o2*ttt, tz);
            //DrawDat.Engine[i].V[12].init (tx-r3o2*ttt, ty-.5*ttt, tz);
        }
        Stat.ShieldSize = readf( shp );
        Stat.ShieldR    = readf( shp );
        Stat.ShieldG    = readf( shp );
        Stat.ShieldB    = readf( shp );
        Stat.NumGuns    = readf( shp );
        Stat.GunType    = new unsigned char[Stat.NumGuns];
        for (i = 0; i < Stat.NumGuns; i++)
            Stat.GunType[i] = readf( shp );
        Stat.GunOffset  = new LVector[Stat.NumGuns];
        for (i = 0; i < Stat.NumGuns; i++) {
            Stat.GunOffset[i].i = readf( shp );
            Stat.GunOffset[i].j = readf( shp );
            Stat.GunOffset[i].k = readf( shp );
        }
        Stat.NumMissiles   = readf( shp );
        Stat.MissileType   = new unsigned char[Stat.NumMissiles];
        for (i = 0; i < Stat.NumMissiles; i++)
            Stat.MissileType[i] = readf( shp );
        Stat.MissileOffset = new LVector[Stat.NumMissiles];
        for (i = 0; i < Stat.NumMissiles; i++) {
            Stat.MissileOffset[i].i = readf( shp );
            Stat.MissileOffset[i].j = readf( shp );
            Stat.MissileOffset[i].k = readf( shp );
        }
        Stat.MissileAmmo     = new unsigned char[Stat.NumMissiles];
        for (i = 0; i < Stat.NumMissiles; i++)
            Stat.MissileAmmo[i] = readf( shp );
        Stat.NumShieldFacing = readf( shp );
        Stat.Shield    = new float[Stat.NumShieldFacing];
        for (i = 0; i < Stat.NumShieldFacing; i++)
            Stat.Shield[i] = readf( shp );
        Stat.MaxShield = new float[Stat.NumShieldFacing];
        for (i = 0; i < Stat.NumShieldFacing; i++)
            Stat.MaxShield[i] = Stat.Shield[i];              //*.1
        Stat.ShieldRecharge = readf( shp );
        for (i = 0; i < 4; i++)
            Stat.MaxArmor[i] = Stat.Armor[i] = readf( shp );
        Stat.MaxHull   = Stat.Hull = readf( shp );
        Stat.HullResistance = readf( shp );
        Stat.MaxSpeed  = readf( shp )*.75;
        Stat.MaxAftSpd = readf( shp )*.75;
        Stat.Accel     = readf( shp )*15;
        Stat.ABAccel   = readf( shp )*20;         //*.002;
        Stat.Mass = readf( shp )/2;
#define PI 3.1415926536
        Stat.Yaw   = 1.2*readf( shp )*PI/360;
        Stat.Pitch = 1.2*readf( shp )*PI/360;
        Stat.Roll  = 1.2*readf( shp )*PI/360;         //make maneuverability less should be /180
    } else {
        Stat.NumShieldFacing = 4;
        Stat.MaxShield = new float[4];
        Stat.Shield = new float[4];
        Stat.MaxShield[0]    = 0;
        Stat.MaxShield[1]    = 0;
        Stat.MaxShield[2]    = 0;
        Stat.MaxShield[3]    = 0;
        Stat.Shield[0] = 0;
        Stat.Shield[1] = 0;
        Stat.Shield[2] = 0;
        Stat.Shield[3] = 0;
        if (interactive) {
            printf( "NumShieldFacings\n" );
            scanf( "%d", &Stat.NumShieldFacing );
            int i;
            for (i = 0; i < Stat.NumShieldFacing; i++) {
                printf( "Shield %d\n", i );
                scanf( "%f", &Stat.MaxShield[i] );
                Stat.Shield[i] = Stat.MaxShield[i];
            }
            printf( "Shield Recharge: \n" );

            scanf( "%f", &Stat.ShieldRecharge );
            for (i = 0; i < 4; i++) {
                int tmp;
                printf( "Max Armor %d:", i );
                scanf( "%d", &tmp );
                Stat.Armor[i]    = tmp;
                Stat.MaxArmor[i] = Stat.Armor[i];
            }
            float hll;
            float hr;
            printf( "Max Hull, Hull Reistance" );
            scanf( "%f %f", &hll, &hr );
            Stat.MaxHull = Stat.Hull = hll;
            Stat.HullResistance = hr;
            printf( "Speed, MaxAftSpeed" );
            scanf( "%f %f", &hll, &hr );
            Stat.MaxSpeed  = hll;
            Stat.MaxAftSpd = hr;

            printf( "Accel AbAccel" );
            scanf( "%f %f", &hll, &hr );
            Stat.Accel   = hll*15;
            Stat.ABAccel = hr*20;             //*.002;
            printf( "Mass" );
            scanf( "%f", &hll );
            Stat.Mass    = hll/2;
            float r;
            printf( "Yaw Pitch Roll" );
            scanf( "%f %f %f", &hll, &hr, &r );
            Stat.Yaw   = 1.2*hll*PI/360;
            Stat.Pitch = 1.2*hr*PI/360;
            Stat.Roll  = 1.2*r*PI/360;             //make maneuverability less should be /180
        }
    }
    fclose( shp );

    fp = fopen( argv[2], "w+b" );
    Tag( "Unit" );
    Tab();
    StrWrite( "<Meshfile file=\"" );
    StrWrite( meshname );
    StrWrite( "\" />\n" );
    for (i = 0; i < DrawDat.numengines; i++) {
        Tab();
        StrWrite( "<Light file=\"supernova.bmp\" x=" );
        TextF( DrawDat.Engine[i].V[0].x*scl );
        StrWrite( " y=" );
        TextF( DrawDat.Engine[i].V[0].y*scl );
        StrWrite( " z=" );
        TextF( DrawDat.Engine[i].V[0].z*scl );
        StrWrite( " size=" );
        TextF( (DrawDat.Engine[i].minradius+DrawDat.Engine[i].maxradius)*.5*scl );
        StrWrite( "/>\n" );
    }
    for (i = 0; i < Stat.NumGuns; i++)
        for (int j = (vegaclassic ? -1 : 1); j <= 1; j += 2) {
            Tab();
            StrWrite( "<Mount weapon=\"" );
            switch (Stat.GunType[i])
            {
            case FLUX:
                StrWrite( "Flux\" size=\"Light\"" );
                break;
            case MASSDRIVER:
                StrWrite( "MassDriver\" size=\"Medium\"" );
                break;
            case MESON:
                StrWrite( "Meson\" size=\"Medium\"" );
                break;
            case NEUTRON:
                StrWrite( "Neutron\" size=\"Medium\"" );
                break;
            case ION:
                StrWrite( "Ion\" size=\"Medium\"" );
                break;
            case PARTICLE:
                StrWrite( "Particle\" size=\"Heavy\"" );
                break;
            case TACH:
                StrWrite( "Tachyon\" size=\"Heavy\"" );
                break;
            case PLASMA:
                StrWrite( "Plasma\" size=\"heavy\"" );
                break;
            case REAPER:
                StrWrite( "Reaper\" size=\"heavy\"" );
                break;
            case PHOTON:
                StrWrite( "Photon\" size=\"heavy\"" );
                break;
            case ANTIMATTER:
                StrWrite( "AntiMatter\" size=\"heavy\"" );
                break;
            case STORMFIRE:
                StrWrite( "Light\" size=\"Special\"" );
                break;
            case LEECH:
                StrWrite( "Leech\" size=\"Special\"" );
                break;
            case LASER:
            default:
                StrWrite( "Laser\" size=\"Light\"" );
                break;
            }
            StrWrite( " x=" );
            TextF( Stat.GunOffset[i].i*scl*j );
            StrWrite( " y=" );
            TextF( Stat.GunOffset[i].j*scl );
            StrWrite( " z=" );
            TextF( Stat.GunOffset[i].k*scl );
            StrWrite( "/>\n" );
        }
    for (i = 0; i < Stat.NumMissiles; i++)
        for (int j = vegaclassic ? -1 : 1; j <= 1; j += 2) {
            int ammo = (int) ( (j == -1 ? ceil : floor)( ( (float) Stat.MissileAmmo[i] )/2 ) );
            if (!ammo)
                continue;
            Tab();
            StrWrite( "<Mount weapon=\"" );
            switch (Stat.MissileType[i])
            {
            case DRAGONFLY:
                StrWrite( "DragonFly\" size=\"Special-Missile\"" );
                break;
            case DUMBFIRE:
                StrWrite( "Dumbfire\" size=\"Light-Missile\"" );
                break;
            case HS:
                StrWrite( "HeatSeeker\" size=\"Medium-Missile\"" );
                break;
            case FF:
                StrWrite( "FriendOrFoe\" size=\"Medium-Missile\"" );
                break;
            case TORP:
                StrWrite( "Torpedo\" size=\"Heavy-Missile\"" );
                break;
            case SWARM:
                StrWrite( "Swarm\" size=\"Special-Missile\"" );
                break;
            case TRACKER:
                StrWrite( "Tracker\" size=\"Special-Missile\"" );
                break;
            case LEECHMIS:
                StrWrite( "LeechMis\" size=\"Medium-Missile\"" );
                break;
            case MACE:
                StrWrite( "Mace\" size=\"Special-Missile\"" );
                break;
            case IR:
            default:
                StrWrite( "ImageRecognition\" size=\"Medium-Missile\"" );
                break;
            }
            StrWrite( " x=" );
            TextF( j*Stat.MissileOffset[i].i*scl );
            StrWrite( " y=" );
            TextF( Stat.MissileOffset[i].j*scl );
            StrWrite( " z=" );
            TextF( Stat.MissileOffset[i].k*scl );
            StrWrite( " ammo=" );
            TextI( ammo );
            StrWrite( "/>\n" );
        }
    Tab();
    Tag( "Thrust" );
    //float abspeed =.066666666666666666667*((float)Stat.MaxAftSpd);
    //float nspeed = .0666666666666666666667*((float)Stat.MaxSpeed);
    float   abspeed = .13333333333333333333*( (float) Stat.MaxAftSpd );
    float   nspeed  = .13333333333333333333*( (float) Stat.MaxSpeed );
    LVector ypr;
    ypr.i = 2.3*Stat.Yaw*180/3.1415926536;
    ypr.j = 2.3*Stat.Pitch*180/3.1415926536;
    ypr.k = 2.3*Stat.Roll*180/3.1415926536;
    float shrch = Stat.ShieldRecharge;
    Tab( 2 );
    fprintf( fp,
             "<Engine Afterburner=\"%f\" Forward=\"%f\" Retro=\"%f\" Left=\"%f\" Right=\"%f\" Top=\"%f\" Bottom=\"%f\"/>\n",
             10*abspeed,
             8*nspeed,
             7*nspeed,
             4*nspeed,
             4*nspeed,
             4*nspeed,
             4*nspeed );
    Tab( 2 );
    fprintf( fp, "<Maneuver yaw=\"%f\" pitch=\"%f\" roll=\"%f\"/>\n", ypr.i*33, ypr.j*33, ypr.k*33 );
    Tab();
    ETag( "Thrust" );
    Tab();
    Tag( "Sound" );
    Tab();
    ETag( "Sound" );
    Tab();
    Tag( "Defense" );
    Tab( 2 );
    fprintf( fp,
             "<Armor front=\"%d\" right=\"%d\" left=\"%d\" back=\"%d\"/>\n",
             Stat.Armor[0],
             Stat.Armor[1],
             Stat.Armor[2],
             Stat.Armor[3] );
    if (Stat.NumShieldFacing == 2) {
        Tab( 2 );
        fprintf( fp, "<Shields front=\"%f\" back=\"%f\" recharge=\"%f\"/>\n", Stat.MaxShield[0], Stat.MaxShield[1], shrch );
    } else {
        Tab( 2 );
        fprintf( fp,
                 "<Shields front=\"%f\" right=\"%f\" left=\"%f\" back=\"%f\" recharge=\"%f\"/>\n",
                 Stat.MaxShield[0],
                 Stat.MaxShield[1],
                 Stat.MaxShield[2],
                 Stat.MaxShield[3],
                 shrch );
    }
    Tab( 2 );
    fprintf( fp, "<Hull strength=\"%f\"/>\n", Stat.Hull );
    Tab();
    ETag( "Defense" );
    Tab();
    fprintf( fp, "<Stats mass=\"%f\" momentofinertia=\"%f\" fuel=\"%d\"></Stats>\n", Stat.Mass, Stat.Mass, 20000 );
    Tab();
    fprintf( fp,
             "<Computer yaw=\"%f\" pitch=\"%f\" roll=\"%f\" afterburner=\"%f\" maxspeed=\"%f\"/>\n",
             ypr.i,
             ypr.j,
             ypr.k,
             abspeed,
             nspeed );
    Tab();
    Tag( "Energy" );
    Tab( 2 );
    fprintf( fp, "<Reactor recharge=\"%d\" limit=\"%d\"/>\n", 40, 100 );
    Tab();
    ETag( "Energy" );
    Tab();
    fprintf( fp, "<Jump delay=\"%d\" jumpenergy=\"%d\"/>\n", 1, 80 );
    Tab();
    fprintf( fp, "<Cockpit file=\"hornet-cockpit.cpt\" x=\"%d\" y=\"%d\" z=\"%d\"/>", 0, 0, 0 );
    ETag( "Unit" );
    fclose( fp );
    /*	yawing = pitching = rolling = 0;
     *  const float Root3 = 1.73205080757f;
     *  ShieldPnt[0].init (.5 , -Root3 * .5 , 2.5); // init the shield
     *  ShieldPnt[0].InitNorm(0,-Root3*.5,.5);
     *  ShieldPnt[1].init (1 , 0 , 2.5);
     *  ShieldPnt[1].InitNorm(0.65465366902942,0.37796446643580,0.65465364757260);
     *  ShieldPnt[2].init (.5 , Root3 * .5, 2.5);
     *  ShieldPnt[2].InitNorm(0.00000000000000,0.91766292416306, 0.39735969508626);
     *  ShieldPnt[3].init (-.5 , Root3 * .5 , 2.5);
     *  ShieldPnt[3].InitNorm(-0.62217101107471, 0.71842123388939, 0.31108551399975);
     *  ShieldPnt[4].init (-1 , 0 , 2.5);
     *  ShieldPnt[4].InitNorm(-0.93325652471705,0.17960530847235, 0.31108551399975);
     *  ShieldPnt[5].init (-.5 , -.5 * Root3 , 2.5);
     *  ShieldPnt[5].InitNorm(-0.59513969559913, -0.68720813768153, 0.41659778596652);
     *  ShieldPnt[6].init (1 , -Root3 , 1.5);
     *  ShieldPnt[6].InitNorm(0,-1, 0);
     *  ShieldPnt[7].init (2 , 0 , 1.5);
     *  ShieldPnt[7].InitNorm(Root3*.5, 0,0.5);
     *  ShieldPnt[8].init (1 , Root3 , 1.5);
     *  ShieldPnt[8].InitNorm(0.42874645259957,0.74261066823585, 0.51449578107554);
     *  ShieldPnt[9].init (-1 , Root3 , 1.5);
     *  ShieldPnt[9].InitNorm(-0.42874645560607,0.74261066823585, 0.51449576744606);
     *  ShieldPnt[10].init (-2 , 0 , 1.5);
     *  ShieldPnt[10].InitNorm(-Root3*.5,0, 0.51449579470503);
     *  ShieldPnt[11].init (-1 , -Root3 , 1.5);
     *  ShieldPnt[11].InitNorm(0,-1,0);
     *  ShieldPnt[12].init (1.2,-1.2*Root3,.5);
     *  ShieldPnt[12].InitNorm(0,-1,0);
     *  ShieldPnt[13].init (2.4 , 0 , .5);
     *  ShieldPnt[13].InitNorm(0.92847666798532,0,0.37139074342578);
     *  ShieldPnt[14].init (1.2 , 1.2 * Root3 , .5);
     *  ShieldPnt[14].InitNorm(0.46423833242208,0.80408441699318, 0.37139072128917);
     *  ShieldPnt[15].init (-1.2 , 1.2 * Root3 , .5);
     *  ShieldPnt[15].InitNorm(-0.46423833623874,0.80408441699318, 0.37139069915256);
     *  ShieldPnt[16].init (-2.4 , 0 , .5 );
     *  ShieldPnt[16].InitNorm(-0.92847666798532,0,0.37139074342578);
     *  ShieldPnt[17].init (-1.2 , -1.2 * Root3 , .5);
     *  ShieldPnt[17].InitNorm(0,-1,0);
     *  for (i=0; i<18; i++)
     *  {
     *       ShieldPnt[i].Vertex.x *=Stat.ShieldSize;
     *       ShieldPnt[i].Vertex.y *=Stat.ShieldSize;
     *       ShieldPnt[i].Vertex.z *=Stat.ShieldSize;
     *  }
     */
    if (Stat.NumGuns)
        Stat.WeapSpd /= Stat.NumGuns;
    Stat.ooMass = .8/(float) Stat.Mass;
    return 0;
}

