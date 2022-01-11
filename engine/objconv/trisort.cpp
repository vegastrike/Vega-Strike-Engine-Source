/**
* trisort.cpp
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <ctype.h>
#include <math.h>
#include "trisort.h"

#ifdef __APPLE__
#define sqrtf sqrt
#endif
bool nearzero( double fb )
{
    const float eps = .0001;
    return fabs( fb ) < eps || fb == 0;
}
POLYGON_RELATION Plane::inFront( const Vector &v ) const
{
    double fb = frontBack( v );
    if ( nearzero( fb ) || nearzero( c ) )
        return UNKNOWN;
    return ( (fb > 0) == (c > 0) ) ? FRONT : BACK;
}

const int RIGHT_HANDED = 1;
bool Face::Cross( Plane &result ) const
{
    double size = 0;
    for (unsigned int i = 2; (!size) && i < this->p.size(); i++) {
        Vector v1( 0, 0, 0 );
        v1.x     = this->p[i].V.x-this->p[0].V.x;
        v1.y     = this->p[i].V.y-this->p[0].V.y;
        v1.z     = this->p[i].V.z-this->p[0].V.z;
        Vector v2( 0, 0, 0 );
        v2.x     = this->p[1].V.x-this->p[0].V.x;
        v2.y     = this->p[1].V.y-this->p[0].V.y;
        v2.z     = this->p[1].V.z-this->p[0].V.z;
        result.a = v1.y*v2.z-v1.z*v2.y;
        result.b = v1.z*v2.x-v1.x*v2.z;
        result.c = v1.x*v2.y-v1.y*v2.x;
        size     = result.a*result.a+result.b*result.b+result.c*result.c;
    }
    if (size)
        size = ( (double) 1 )/sqrtf( size );
    else
        return false;
    result.a *= RIGHT_HANDED*(size);
    result.b *= RIGHT_HANDED*(size);
    result.c *= RIGHT_HANDED*(size);
    return true;
}

POLYGON_RELATION Face::inFront( const Plane &p ) const
{
    POLYGON_RELATION retval = UNKNOWN;
    for (unsigned int i = 0; i < this->p.size(); i++) {
        POLYGON_RELATION pr = p.inFront( this->p[i].V );
        if (pr == FRONT) {
            if (retval == UNKNOWN || retval == FRONT)
                retval = FRONT;
            else return UNKNOWN;
        } else if (pr == BACK) {
            if (retval == UNKNOWN || retval == BACK)
                retval = BACK;
            else return UNKNOWN;
        }
    }
    return retval;
}
Plane Face::planeEqu() const
{
    if ( p.empty() )
        return Plane( 1, 0, 0, 0 );
    Plane rez( 1, 1, 1, 0 );
    if ( !Cross( rez ) )
        rez = Plane( 1, 1, 1, 0 );
    rez.d = ( (rez.a*p[0].V.x)+(rez.b*p[0].V.y)+(rez.c*p[0].V.z) );
    rez.d = -rez.d;
    return rez;
}
bool Face::operator<( const Face &b ) const
{
    //printf ("comparing %d %d\n",id,b.id);
    Plane bpe( b.planeEqu() );
    Plane ape( planeEqu() );
    POLYGON_RELATION bfronta = inFront( bpe );
    POLYGON_RELATION afrontb = b.inFront( ape );
    if (bfronta == FRONT || afrontb == BACK) {
        return false;
    } else if ( (bfronta == BACK || afrontb == FRONT) ) {
        return true;
    } else if (afrontb == UNKNOWN && bfronta == UNKNOWN) {
        static int errcount = 0;
        errcount++;
        if (errcount < 10 || errcount%2 == 1)
            fprintf( stderr, "polygon intersection detected %d\n", errcount );
        return false;
    }
    return false;
}

void Mesh::sort()
{
    std::sort( f.begin(), f.end() );
}
void Index::write( FILE *fp ) const
{
    printf( "G: %lf\n", V.z );
    fprintf( fp, "%d/", p );
    if (n != -1)
        fprintf( fp, "%d", n );
    fprintf( fp, "/" );
    if (t != -1)
        fprintf( fp, "%d", t );
    if (c != -1)
        fprintf( fp, "/%d", c );
}
void Mesh::write( const char *filename ) const
{
    FILE *fp = fopen( filename, "w" );
    if (fp) {
        unsigned int i;
        for (i = 0; i < p.size(); i++)
            fprintf( fp, "v %lf %lf %lf\n", p[i].x, p[i].y, p[i].z );
        for (i = 0; i < n.size(); i++)
            fprintf( fp, "n %lf %lf %lf\n", n[i].x, n[i].y, n[i].z );
        for (i = 0; i < t.size(); i++)
            fprintf( fp, "p %lf %lf\n", t[i].x, t[i].y );
        for (i = 0; i < c.size(); i++)
            fprintf( fp, "c %lf %lf %lf\n", c[i].x, c[i].y, c[i].z );
        for (i = 0; i < f.size(); i++) {
            fprintf( fp, "f" );
            for (unsigned j = 0; j < f[i].p.size(); j++) {
                fprintf( fp, " " );
                f[i].p[j].write( fp );
            }
            fprintf( fp, "\n" );
        }
        fclose( fp );
    }
}
static int iswhitespace( int c )
{
    return isspace( c ) || c == '\n' || c == '\r';
}

char * findspace( char *line )
{
    while (iswhitespace( line[0] ) && line[0])
        line++;
    if (!line[0])
        return NULL;
    while (!iswhitespace( line[0] ) && line[0])
        line++;
    return line;
}
Index Mesh::processfacevertex( char *vertex ) const
{
    int a, b, c, d;
    a = 0;
    b = c = d = -1;
    sscanf( vertex, "%d", &a );
    while (vertex[0] && (*vertex) != '/')
        vertex++;
    if (vertex[0] == '/') {
        vertex++;
        if (vertex[0] != '/')
            sscanf( vertex, "%d", &b );
        while (vertex[0] && (*vertex) != '/')
            vertex++;
        if (vertex[0] == '/') {
            vertex++;
            if (vertex[0] != '/')
                sscanf( vertex, "%d", &c );
            while (vertex[0] && (*vertex) != '/')
                vertex++;
            if (vertex[0] == '/') {
                vertex++;
                sscanf( vertex, "%d", &d );
            }
        }
    }
    Vector v( 0, 0, 0 );
    if (a <= (int) p.size() && a > 0)
        v = p[a-1];
    return Index( v, a, b, c, d );
}
Face Mesh::processface( char *line ) const
{
    Face  f;
    char *ln = line;
    while (line[0]) {
        char *lastspace = findspace( line );
        if (lastspace) {
            bool done = (lastspace[0] == 0);
            (*lastspace) = 0;
            f.p.push_back( processfacevertex( line ) );
            line = lastspace+1;
            if (done)
                break;
        } else {
            break;
        }
    }
    if (f.p.size() < 3)
        printf( "bleh!!||| %s\n", ln );
    return f;
}
void Mesh::processline( char *line )
{
    double a = 0, b = 0, c = 0, d = 0;
    switch (line[0])
    {
    case 'v':
        sscanf( line, "v %lf %lf %lf", &a, &b, &c );
        p.push_back( Vector( a, b, c ) );
        break;
    case 'n':
        sscanf( line, "n %lf %lf %lf", &a, &b, &c );
        n.push_back( Vector( a, b, c ) );
        break;
    case 't':
        sscanf( line, "t %lf %lf", &a, &b );
        t.push_back( Vector( a, b, 0 ) );
        break;
    case 'c':
        sscanf( line, "c %lf %lf %lf %lf", &a, &b, &c, &d );
        this->c.push_back( Vector( a, b, c ) );
        break;
    case 'f':
        f.push_back( processface( line+1 ) );
        break;
    default:
        break;
    }
}

Mesh::Mesh( const char *filename )
{
    FILE *fp = fopen( filename, "r" );
    char  line[65536];
    line[65535] = 0;
    while ( fgets( line, 65535, fp ) )
        processline( line );
}

int main( int argc, char **argv )
{
    for (int i = 1; i < argc; i++) {
        Mesh m( argv[i] );
        m.sort();
        std::string rez( argv[i] );
        rez += ".out";
        m.write( rez.c_str() );
    }
    return 0;
}

