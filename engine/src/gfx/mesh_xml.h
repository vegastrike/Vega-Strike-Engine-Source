/*
 * mesh_xml.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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


#ifndef __MESH_XML_H__INCLUDED_
#define __MESH_XML_H__INCLUDED_

struct MeshXML {
    enum Names {
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
        USENORMALS,
        USETANGENTS,
        REVERSE,
        POLYGONOFFSET,
        DETAILTEXTURE,
        DETAILPLANE,
        FRAMESPERSECOND,
        STARTFRAME,
        ALPHATEST
    };
    ///Saves which attributes of vertex have been set in XML file
    enum PointState {
        P_X = 0x1,
        P_Y = 0x2,
        P_Z = 0x4,
        P_I = 0x8,
        P_J = 0x10,
        P_K = 0x20
    };
    ///Saves which attributes of vertex have been set in Polygon for XML file
    enum VertexState {
        V_POINT = 0x1,
        V_S = 0x2,
        V_T = 0x4
    };
    ///Save if various logo values have been set
    enum LogoState {
        V_TYPE = 0x1,
        V_ROTATE = 0x2,
        V_SIZE = 0x4,
        V_OFFSET = 0x8,
        V_REF = 0x10
    };
    ///To save the constructing of a logo
    struct ZeLogo {
        ///Which type the logo is (0 = faction 1 = squad >2 = internal use
        unsigned int type;
        ///how many degrees logo is rotated
        float rotate;
        ///Size of the logo
        float size;
        ///offset of polygon of logo
        float offset;
        ///the reference points that the logo is weighted against
        vector<int> refpnt;
        ///the weight of the points in weighted average of refpnts
        vector<float> refweight;
    };
    struct ZeTexture {
        string decal_name;
        string alpha_name;
        string animated_name;
    };
    class Flightgroup *fg;
    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap element_map;
    static const EnumMap attribute_map;

    MeshXML()
            : sharevert(false),
            usenormals(false),
            usetangents(false),
            reverse(false),
            force_texture(false),
            load_stage(0),
            point_state(0),
            vertex_state(0),
            recalc_norm(false),
            num_vertices(0),
            tstrcnt(0),
            tfancnt(0),
            qstrcnt(0),
            lstrcnt(0),
            faction(0),
            mesh(0) {
    }

    ///All logos on this unit
    vega_types::ContiguousSequenceContainer<ZeLogo> logos;
    vega_types::ContiguousSequenceContainer<Names> state_stack;
    bool sharevert;
    bool usenormals;
    bool usetangents;
    bool reverse;
    bool force_texture;
    int load_stage;
    int point_state;
    int vertex_state;
    Vector scale;
    Vector lodscale;
    vega_types::ContiguousSequenceContainer<ZeTexture> decals;
    string technique;
    bool recalc_norm;
    int num_vertices;
    vega_types::ContiguousSequenceContainer<GFXVertex> vertices;
    ///keep count to make averaging easy
    vega_types::ContiguousSequenceContainer<int> vertexcount;
    vega_types::ContiguousSequenceContainer<GFXVertex> lines;
    vega_types::ContiguousSequenceContainer<GFXVertex> tris;
    vega_types::ContiguousSequenceContainer<GFXVertex> quads;
    vega_types::ContiguousSequenceContainer<vega_types::ContiguousSequenceContainer<GFXVertex> > linestrips;
    vega_types::ContiguousSequenceContainer<vega_types::ContiguousSequenceContainer<GFXVertex> > tristrips;
    vega_types::ContiguousSequenceContainer<vega_types::ContiguousSequenceContainer<GFXVertex> > trifans;
    vega_types::ContiguousSequenceContainer<vega_types::ContiguousSequenceContainer<GFXVertex> > quadstrips;
    int tstrcnt;
    int tfancnt;
    int qstrcnt;
    int lstrcnt;
    vega_types::ContiguousSequenceContainer<int> lineind;
    vega_types::ContiguousSequenceContainer<int> nrmllinstrip;
    vega_types::ContiguousSequenceContainer<int> linestripind;
    ///for possible normal computation
    vega_types::ContiguousSequenceContainer<int> triind;
    vega_types::ContiguousSequenceContainer<int> nrmltristrip;
    vega_types::ContiguousSequenceContainer<int> tristripind;
    vega_types::ContiguousSequenceContainer<int> nrmltrifan;
    vega_types::ContiguousSequenceContainer<int> trifanind;
    vega_types::ContiguousSequenceContainer<int> nrmlquadstrip;
    vega_types::ContiguousSequenceContainer<int> quadstripind;
    vega_types::ContiguousSequenceContainer<int> quadind;
    vega_types::ContiguousSequenceContainer<int> trishade;
    vega_types::ContiguousSequenceContainer<int> quadshade;
    vega_types::ContiguousSequenceContainer<int> * active_shade;
    vega_types::ContiguousSequenceContainer<GFXVertex> * active_list;
    vega_types::ContiguousSequenceContainer<int> * active_ind;
    vega_types::ContiguousSequenceContainer<vega_types::SharedPtr<Mesh> > lod;
    vega_types::ContiguousSequenceContainer<float> lodsize;
    GFXVertex vertex;
    GFXMaterial material;
    int faction;
    vega_types::SharedPtr<Mesh> mesh;
};

#endif


