/**
* to_BFXM.h
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

#ifndef _TO_BFXM_H_
#define _TO_BFXM_H_
#include <string>
#include <vector>
///Stores all the load-time vertex info in the XML struct FIXME light calculations
#include <expat.h>
#include "endianness.h"
using std::vector;
using std::string;
#include "xml_support.h"
#include "hashtable.h"
#include "mesh_io.h"
using namespace XMLSupport;
struct GFXMaterial /// ambient rgba, if you don't like these things, ask me to rename them
{
    float ar;
    float ag;
    float ab;
    float aa;
    /// diffuse rgba
    float dr;
    float dg;
    float db;
    float da;
    /// specular rgba
    float sr;
    float sg;
    float sb;
    float sa;
    /// emissive rgba
    float er;
    float eg;
    float eb;
    float ea;
    /// specular power
    float power;
    GFXMaterial()
    {
        dr    = dg = db = da = sr = sg = sb = sa = ea = aa = 1.0f;
        er    = eg = eb = ar = ag = ab = 0.0f;
        power = 60.0f;
        //defaults for material struct
    }
};

struct GFXVertex
{
    float x, y, z;     //Location
    float i, j, k;     //Normal
    float s, t;     //U,V coords
    GFXVertex operator*( float s )
    {
        GFXVertex ret( *this );
        ret.   x *= s;
        ret.   y *= s;
        ret.   z *= s;
        return ret;
    }
};

struct line
{
    bool  flatshade;
    int   indexref[2];   //Index into Points
    float s[2];     //U
    float t[2];     //V
    line()
    {
        flatshade = 0;
    }
    line( int i1, int i2, float s1, float t1, float s2, float t2, bool fs = 0 )
    {
        indexref[0] = i1;
        indexref[1] = i2;
        s[0] = s1;
        s[1] = s2;
        t[0] = t1;
        t[1] = t2;
        flatshade = fs;
    }
    void clear()
    {
        flatshade = false;
        indexref[0] = {0};
        indexref[1] = {0};
        s[0] = {0.0};
        s[1] = {0.0};
        t[0] = {0.0};
        t[1] = {0.0};
    }
};

struct triangle
{
    bool  flatshade;
    int   indexref[3];   //Index into Points
    float s[3];     //U
    float t[3];     //V
    triangle()
    {
        flatshade = 0;
    }
    triangle( int i1, int i2, int i3, float s1, float t1, float s2, float t2, float s3, float t3, bool fs = 0 )
    {
        indexref[0] = i1;
        indexref[1] = i2;
        indexref[2] = i3;
        s[0] = s1;
        s[1] = s2;
        s[2] = s3;
        t[0] = t1;
        t[1] = t2;
        t[2] = t3;
        flatshade = fs;
    }
    void clear()
    {
        flatshade = false;
        indexref[0] = {0};
        indexref[1] = {0};
        indexref[2] = {0};
        s[0] = {0.0};
        s[1] = {0.0};
        s[2] = {0.0};
        t[0] = {0.0};
        t[1] = {0.0};
        t[2] = {0.0};
    }
};

struct quad
{
    bool  flatshade;
    int   indexref[4];   //Index into Points
    float s[4];     //U
    float t[4];     //V
    quad()
    {
        flatshade = 0;
    }
    quad( int i1,
          int i2,
          int i3,
          int i4,
          float s1,
          float t1,
          float s2,
          float t2,
          float s3,
          float t3,
          float s4,
          float t4,
          bool fs = 0 )
    {
        indexref[0] = i1;
        indexref[1] = i2;
        indexref[2] = i3;
        indexref[3] = i4;
        s[0] = s1;
        s[1] = s2;
        s[2] = s3;
        s[3] = s4;
        t[0] = t1;
        t[1] = t2;
        t[2] = t3;
        t[3] = t4;
        flatshade = fs;
    }
    void clear()
    {
        flatshade = false;
        indexref[0] = {0};
        indexref[1] = {0};
        indexref[2] = {0};
        indexref[3] = {0};
        s[0] = {0.0};
        s[1] = {0.0};
        s[2] = {0.0};
        s[3] = {0.0};
        t[0] = {0.0};
        t[1] = {0.0};
        t[2] = {0.0};
        t[3] = {0.0};
    }
};

struct stripelement
{
    int   indexref;
    float s;
    float t;
};

struct strip
{
    bool flatshade;
    vector< stripelement >points;
    strip()
    {
        flatshade = false;
        points    = vector< stripelement > ();
    }
};

struct LODholder   //Holds 1 LOD entry
{
    float size;
    vector< unsigned char >name;
    LODholder()
    {
        name = vector< unsigned char > ();
        size = 0;
    }
};

struct animframe   //Holds one animation frame
{
    vector< unsigned char >name;
    animframe()
    {
        name = vector< unsigned char > ();
    }
};

struct animdef   //Holds animation definition
{
    vector< unsigned char >name;
    float FPS;
    vector< int >meshoffsets;
    animdef()
    {
        name = vector< unsigned char > ();
        FPS  = 0;
        meshoffsets = vector< int > ();
    }
};

struct textureholder   //Holds 1 texture entry
{
    int type;
    int index;
    vector< unsigned char >name;
    textureholder() : type( -1 ), index( 0 ) {}
    textureholder( int _index, int _type = TEXTURE ) : type( _type ), index( _index ) {}
};

enum polytype
{
    LINE,
    TRIANGLE,
    QUAD,
    LINESTRIP,
    TRISTRIP,
    TRIFAN,
    QUADSTRIP,
    UNKNOWN
};

struct XML
{
    enum Names
    {
        //elements
        UNKNOWN,
        MATERIAL,
        AMBIENT,
        DIFFUSE,
        SPECULAR,
        EMISSIVE,
        MESH,
        POINTS,
        POINT,
        LOCATION,
        NORMAL,
        POLYGONS,
        LINE,
        LOD,
        TRI,
        QUAD,
        LODFILE,
        LINESTRIP,
        TRISTRIP,
        TRIFAN,
        QUADSTRIP,
        VERTEX,
        LOGO,
        REF,
        //new
        ANIMDEF,
        ANIMFRAME,
        //attributes
        POWER,
        REFLECT,
        CULLFACE,
        LIGHTINGON,
        FLATSHADE,
        TEXTURE,
        TECHNIQUE,
        FORCETEXTURE,
        ALPHAMAP,
        SHAREVERT,
        ALPHA,
        RED,
        GREEN,
        BLUE,
        X,
        Y,
        Z,
        I,
        J,
        K,
        S,
        T,
        SCALE,
        BLENDMODE,
        TYPE,
        ROTATE,
        WEIGHT,
        SIZE,
        OFFSET,
        ANIMATEDTEXTURE,
        ALPHATEST,
        USENORMALS,
        REVERSE,
        POLYGONOFFSET,
        DETAILTEXTURE,
        DETAILPLANE,
        FRAMESPERSECOND,
        STARTFRAME,
        FRAMEMESHNAME,
        ANIMATIONNAME,
        ANIMATIONFRAMEINDEX,
        ANIMATIONMESHINDEX
    };

    ///holds a logo
    struct ZeLogo
    {
        ///Which type the logo is (0 = faction 1 = squad >2 = internal use
        unsigned int type;
        ///how many degrees logo is rotated
        float rotate;
        ///Size of the logo
        float size;
        ///offset of polygon of logo
        float offset;
        ///the reference points that the logo is weighted against
        vector< int >  refpnt;
        ///the weight of the points in weighted average of refpnts
        vector< float >refweight;
        ZeLogo()
        {
            refpnt    = vector< int > ();
            refweight = vector< float > ();
            size = 0;
            offset    = 0;
            rotate    = 0;
            type = 0;
        }
    };
    struct ZeTexture
    {
        string decal_name;
        string alpha_name;
        string animated_name;
    };

    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap   element_map;
    static const EnumMap   attribute_map;
    vector< Names >        state_stack;
    vector< GFXVertex >    vertices;
    vector< int >          num_vertex_references;
    vector< line >         lines;
    vector< triangle >     tris;
    vector< quad >         quads;

    //FIXME - strips have yet to be verified to work
    vector< strip >        linestrips;
    vector< strip >        tristrips;
    vector< strip >        trifans;
    vector< strip >        quadstrips;
    //END FIXME

    vector< ZeLogo >       logos;

    Mesh_vec3f             detailplane;
    vector< Mesh_vec3f >   detailplanes;

    bool                   sharevert;
    bool                   usenormals;
    bool                   reverse;
    bool                   force_texture;

    bool                   reflect;
    bool                   lighting;
    bool                   cullface;
    float                  polygon_offset;
    int                    blend_src;
    int                    blend_dst;
    float                  alphatest;
    GFXVertex              vertex;
    textureholder          texturetemp;
    vector< textureholder >textures;
    textureholder          detailtexture;

    int                    curpolytype;
    int                    curpolyindex;

    line                   linetemp;
    triangle               triangletemp;
    quad                   quadtemp;
    strip                  striptemp;
    stripelement           stripelementtemp;
    LODholder              lodtemp;
    animframe              animframetemp;
    animdef                animdeftemp;
    vector< LODholder >    LODs;
    vector< animframe >    animframes;
    vector< animdef >      animdefs;

    GFXMaterial            material;
    float                  scale;
    XML()
    {
        //FIXME make defaults appear here.
        alphatest = 0.0f;
        scale = 1.0;
        sharevert = 0;
        usenormals     = 1;
        reverse        = 0;
        force_texture  = 0;
        reflect        = 1;
        lighting       = 1;
        cullface       = 1;
        polygon_offset = 0;
        blend_src      = ONE;
        blend_dst      = ZERO;
    }
};

XML LoadXML( const char *filename, float unitscale );
void xmeshToBFXM( XML memfile, FILE *Outputfile, char mode, bool forcenormals ); //converts input file to BFXM creates new, or appends record based on mode
unsigned int writesuperheader( XML memfile, FILE *Outputfile ); //Writes superheader to file Outputfile
unsigned int appendrecordfromxml( XML memfile, FILE *Outputfile, bool forcenormals ); //Append a record specified in memfile to the output file and return number of bytes written. Assumes Outputfile is appropriately positioned at the end of the file.
unsigned int appendmeshfromxml( XML memfile, FILE *Outputfile, bool forcenormals ); //Append a mesh specified in memfile to the output file and return number of bytes written. Assumes Outputfile is appropriately positioned at the end of the file.
void AddNormal( GFXVertex &outp, const GFXVertex &inp );
void SetNormal( GFXVertex &outp, const GFXVertex &a, const GFXVertex &b, const GFXVertex &c );

#endif

