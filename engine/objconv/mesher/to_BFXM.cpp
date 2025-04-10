/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeaders/Converter.h"
#include "objconv/mesher/mesh_io.h"
#include "objconv/mesher/to_BFXM.h"
#include <cstring>  //We are using C style string functions here

//#define fprintf aprintf
uint32bit aprintf(...) {
    return 0;
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;

const EnumMap::Pair XML::element_names[] = {
        EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
        EnumMap::Pair("Material", XML::MATERIAL),
        EnumMap::Pair("LOD", XML::LOD),
        EnumMap::Pair("Ambient", XML::AMBIENT),
        EnumMap::Pair("Diffuse", XML::DIFFUSE),
        EnumMap::Pair("Specular", XML::SPECULAR),
        EnumMap::Pair("Emissive", XML::EMISSIVE),
        EnumMap::Pair("Mesh", XML::MESH),
        EnumMap::Pair("Points", XML::POINTS),
        EnumMap::Pair("Point", XML::POINT),
        EnumMap::Pair("Location", XML::LOCATION),
        EnumMap::Pair("Normal", XML::NORMAL),
        EnumMap::Pair("Polygons", XML::POLYGONS),
        EnumMap::Pair("Line", XML::LINE),
        EnumMap::Pair("Tri", XML::TRI),
        EnumMap::Pair("Quad", XML::QUAD),
        EnumMap::Pair("Linestrip", XML::LINESTRIP),
        EnumMap::Pair("Tristrip", XML::TRISTRIP),
        EnumMap::Pair("Trifan", XML::TRIFAN),
        EnumMap::Pair("Quadstrip", XML::QUADSTRIP),
        EnumMap::Pair("Vertex", XML::VERTEX),
        EnumMap::Pair("Logo", XML::LOGO),
        EnumMap::Pair("Ref", XML::REF),
        EnumMap::Pair("DetailPlane", XML::DETAILPLANE),
        EnumMap::Pair("AnimationDefinition", XML::ANIMDEF),
        EnumMap::Pair("Frame", XML::ANIMFRAME),
        EnumMap::Pair("AnimationFrameIndex", XML::ANIMATIONFRAMEINDEX)
};

const EnumMap::Pair XML::attribute_names[] = {
        EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
        EnumMap::Pair("Scale", XML::SCALE),
        EnumMap::Pair("Blend", XML::BLENDMODE),
        EnumMap::Pair("texture", XML::TEXTURE),
        EnumMap::Pair("technique", XML::TECHNIQUE),
        EnumMap::Pair("alphamap", XML::ALPHAMAP),
        EnumMap::Pair("sharevertex", XML::SHAREVERT),
        EnumMap::Pair("red", XML::RED),
        EnumMap::Pair("green", XML::GREEN),
        EnumMap::Pair("blue", XML::BLUE),
        EnumMap::Pair("alpha", XML::ALPHA),
        EnumMap::Pair("power", XML::POWER),
        EnumMap::Pair("reflect", XML::REFLECT),
        EnumMap::Pair("x", XML::X),
        EnumMap::Pair("y", XML::Y),
        EnumMap::Pair("z", XML::Z),
        EnumMap::Pair("i", XML::I),
        EnumMap::Pair("j", XML::J),
        EnumMap::Pair("k", XML::K),
        EnumMap::Pair("Shade", XML::FLATSHADE),
        EnumMap::Pair("point", XML::POINT),
        EnumMap::Pair("s", XML::S),
        EnumMap::Pair("t", XML::T),
        //Logo stuffs
        EnumMap::Pair("Type", XML::TYPE),
        EnumMap::Pair("Rotate", XML::ROTATE),
        EnumMap::Pair("Weight", XML::WEIGHT),
        EnumMap::Pair("Size", XML::SIZE),
        EnumMap::Pair("Offset", XML::OFFSET),
        EnumMap::Pair("meshfile", XML::LODFILE),
        EnumMap::Pair("Animation", XML::ANIMATEDTEXTURE),
        EnumMap::Pair("Reverse", XML::REVERSE),
        EnumMap::Pair("LightingOn", XML::LIGHTINGON),
        EnumMap::Pair("CullFace", XML::CULLFACE),
        EnumMap::Pair("ForceTexture", XML::FORCETEXTURE),
        EnumMap::Pair("UseNormals", XML::USENORMALS),
        EnumMap::Pair("PolygonOffset", XML::POLYGONOFFSET),
        EnumMap::Pair("DetailTexture", XML::DETAILTEXTURE),
        EnumMap::Pair("FramesPerSecond", XML::FRAMESPERSECOND),
        EnumMap::Pair("FPS", XML::FRAMESPERSECOND),
        EnumMap::Pair("FrameMeshName", XML::FRAMEMESHNAME),
        EnumMap::Pair("AnimationName", XML::ANIMATIONNAME),
        EnumMap::Pair("AnimationMeshIndex", XML::ANIMATIONMESHINDEX),
        EnumMap::Pair("alphatest", XML::ALPHATEST)
};

const EnumMap XML::element_map(XML::element_names, sizeof(element_names) / sizeof(*element_names));
const EnumMap XML::attribute_map(XML::attribute_names, sizeof(attribute_names) / sizeof(*attribute_names));

void CopyNormal(GFXVertex &outp, const GFXVertex &inp) {
    outp.i = inp.i;
    outp.j = inp.j;
    outp.k = inp.k;
}

void AddNormal(GFXVertex &outp, const GFXVertex &inp) {
    outp.i += inp.i;
    outp.j += inp.j;
    outp.k += inp.k;
}

void SetNormal(GFXVertex &outp, const GFXVertex &a, const GFXVertex &b, const GFXVertex &c) {
    GFXVertex left, right;
    left.i = b.x - a.x;
    left.j = b.y - a.y;
    left.k = b.z - a.z;
    right.i = c.x - a.x;
    right.j = c.y - a.y;
    right.k = c.z - a.z;
    outp.i = left.j * right.k - left.k * right.j;     //xpd
    outp.j = left.k * right.i - left.i * right.k;
    outp.k = left.i * right.j - left.j * right.i;
    float len = (float) sqrt(outp.i * outp.i + outp.j * outp.j + outp.k * outp.k);
    if (len > .00001) {
        outp.i /= len;
        outp.j /= len;
        outp.k /= len;
    }
}

void boundscheck(XML xml) {
    float maxa = 1.0;
    float maxd = 1.0;
    float maxe = 1.0;
    float maxs = 1.0;
    maxa = (maxa < xml.material.aa) ? xml.material.aa : maxa;
    maxa = (maxa < xml.material.ar) ? xml.material.ar : maxa;
    maxa = (maxa < xml.material.ag) ? xml.material.ag : maxa;
    maxa = (maxa < xml.material.ab) ? xml.material.ab : maxa;
    maxd = (maxd < xml.material.da) ? xml.material.da : maxd;
    maxd = (maxd < xml.material.dr) ? xml.material.dr : maxd;
    maxd = (maxd < xml.material.dg) ? xml.material.dg : maxd;
    maxd = (maxd < xml.material.db) ? xml.material.db : maxd;
    maxe = (maxe < xml.material.ea) ? xml.material.ea : maxe;
    maxe = (maxe < xml.material.er) ? xml.material.er : maxe;
    maxe = (maxe < xml.material.eg) ? xml.material.eg : maxe;
    maxe = (maxe < xml.material.eb) ? xml.material.eb : maxe;
    maxs = (maxs < xml.material.sa) ? xml.material.sa : maxs;
    maxs = (maxs < xml.material.sr) ? xml.material.sr : maxs;
    maxs = (maxs < xml.material.sg) ? xml.material.sg : maxs;
    maxs = (maxs < xml.material.sb) ? xml.material.sb : maxs;
    xml.material.aa /= maxa;
    xml.material.ar /= maxa;
    xml.material.ag /= maxa;
    xml.material.ab /= maxa;
    xml.material.da /= maxd;
    xml.material.dr /= maxd;
    xml.material.dg /= maxd;
    xml.material.db /= maxd;
    xml.material.ea /= maxe;
    xml.material.er /= maxe;
    xml.material.eg /= maxe;
    xml.material.eb /= maxe;
    xml.material.sa /= maxs;
    xml.material.sr /= maxs;
    xml.material.sg /= maxs;
    xml.material.sb /= maxs;
}

enum BLENDFUNC parse_alpha(const char *tmp) {
    if (strcmp(tmp, "ZERO") == 0) {
        return ZERO;
    }
    if (strcmp(tmp, "ONE") == 0) {
        return ONE;
    }
    if (strcmp(tmp, "SRCCOLOR") == 0) {
        return SRCCOLOR;
    }
    if (strcmp(tmp, "INVSRCCOLOR") == 0) {
        return INVSRCCOLOR;
    }
    if (strcmp(tmp, "SRCALPHA") == 0) {
        return SRCALPHA;
    }
    if (strcmp(tmp, "INVSRCALPHA") == 0) {
        return INVSRCALPHA;
    }
    if (strcmp(tmp, "DESTALPHA") == 0) {
        return DESTALPHA;
    }
    if (strcmp(tmp, "INVDESTALPHA") == 0) {
        return INVDESTALPHA;
    }
    if (strcmp(tmp, "DESTCOLOR") == 0) {
        return DESTCOLOR;
    }
    if (strcmp(tmp, "INVDESTCOLOR") == 0) {
        return INVDESTCOLOR;
    }
    if (strcmp(tmp, "SRCALPHASAT") == 0) {
        return SRCALPHASAT;
    }
    if (strcmp(tmp, "CONSTALPHA") == 0) {
        return CONSTALPHA;
    }
    if (strcmp(tmp, "INVCONSTALPHA") == 0) {
        return INVCONSTALPHA;
    }
    if (strcmp(tmp, "CONSTCOLOR") == 0) {
        return CONSTCOLOR;
    }
    if (strcmp(tmp, "INVCONSTCOLOR") == 0) {
        return INVCONSTCOLOR;
    }
    return ZERO;
}

bool shouldreflect(string r) {
    if (strtoupper(r) == "FALSE") {
        return false;
    }
    uint32bit i;
    for (i = 0; i < (uint32bit) r.length(); ++i) {
        if (r[i] != '0' && r[i] != '.' && r[i] != '+' && r[i] != 'e') {
            return true;
        }
    }
    //Just about anything other than "FALSE" or 0.00 etc.

    return false;
}

void beginElement(const string &name, const AttributeList &attributes, XML *xml) {
    bool flips = atoi(Converter::getNamedOption("flips").c_str()) != 0;
    bool flipt = atoi(Converter::getNamedOption("flipt").c_str()) != 0;
    bool flipn = atoi(Converter::getNamedOption("flipn").c_str()) != 0;

    AttributeList::const_iterator iter;
    XML::Names elem = (XML::Names) XML::element_map.lookup(name);
    xml->state_stack.push_back(elem);
    switch (elem) {
        case XML::DETAILPLANE:
            memset(&xml->detailplane, 0, sizeof(xml->detailplane));
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::X:
                        xml->detailplane.x = float(XMLSupport::parse_float(iter->value));
                        break;
                    case XML::Y:
                        xml->detailplane.y = float(XMLSupport::parse_float(iter->value));
                        break;
                    case XML::Z:
                        xml->detailplane.z = float(XMLSupport::parse_float(iter->value));
                        break;
                }
            }
            xml->detailplanes.push_back(xml->detailplane);
            break;
        case XML::MATERIAL:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::USENORMALS:
                        xml->usenormals = XMLSupport::parse_bool(iter->value);
                        break;
                    case XML::POWER:
                        xml->material.power = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::REFLECT:
                        xml->reflect = (shouldreflect((*iter).value));
                        break;
                    case XML::LIGHTINGON:
                        xml->lighting = (XMLSupport::parse_bool((*iter).value));
                        break;
                    case XML::CULLFACE:
                        xml->cullface = (XMLSupport::parse_bool((*iter).value));
                        break;
                }
            }
            break;
        case XML::DIFFUSE:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::RED:
                        xml->material.dr = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::BLUE:
                        xml->material.db = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::ALPHA:
                        xml->material.da = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::GREEN:
                        xml->material.dg = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            break;
        case XML::EMISSIVE:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::RED:
                        xml->material.er = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::BLUE:
                        xml->material.eb = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::ALPHA:
                        xml->material.ea = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::GREEN:
                        xml->material.eg = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            break;
        case XML::SPECULAR:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::RED:
                        xml->material.sr = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::BLUE:
                        xml->material.sb = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::ALPHA:
                        xml->material.sa = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::GREEN:
                        xml->material.sg = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            break;
        case XML::AMBIENT:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::RED:
                        xml->material.ar = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::BLUE:
                        xml->material.ab = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::ALPHA:
                        xml->material.aa = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::GREEN:
                        xml->material.ag = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            break;
        case XML::UNKNOWN:
            fprintf(stderr, "Unknown element start tag '%s' detected\n", name.c_str());
            break;
        case XML::MESH:
            //memset(&xml->material, 0, sizeof(xml->material));
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::REVERSE:
                        xml->reverse = XMLSupport::parse_bool((*iter).value);
                        break;
                    case XML::FORCETEXTURE:
                        xml->force_texture = XMLSupport::parse_bool((*iter).value);
                        break;
                    case XML::SCALE:
                        xml->scale = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::ALPHATEST:
                        xml->alphatest = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::SHAREVERT:
                        xml->sharevert = XMLSupport::parse_bool((*iter).value);
                        break;
                    case XML::POLYGONOFFSET:
                        xml->polygon_offset = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::BLENDMODE: {
                        std::string::size_type sep = (*iter).value.find(' ');
                        if (sep != std::string::npos) {
                            xml->blend_src = parse_alpha((*iter).value.substr(0, sep).c_str());
                            xml->blend_dst = parse_alpha((*iter).value.substr(sep + 1).c_str());
                        }
                        break;
                    }
                    case XML::DETAILTEXTURE: {
                        string detnametmp = (*iter).value.c_str();
                        xml->detailtexture.type = TEXTURE;
                        xml->detailtexture.index = 0;
                        xml->detailtexture.name = vector<uchar8bit>();
                        for (size_t detnamelen = 0; detnamelen < detnametmp.size(); detnamelen++) {
                            xml->detailtexture.name.push_back(detnametmp[detnamelen]);
                        }
                        break;
                    }
                    case XML::TECHNIQUE:
                    case XML::TEXTURE:
                    case XML::ALPHAMAP:
                    case XML::ANIMATEDTEXTURE:
                    case XML::UNKNOWN:             //FIXME?
                    {
                        XML::Names whichtype = XML::UNKNOWN;
                        uint32bit strsize = 0;
                        if (strtoupper(iter->name).find("ANIMATION") == 0) {
                            xml->texturetemp.type = ANIMATION;
                            whichtype = XML::ANIMATEDTEXTURE;
                            strsize = (uint32bit) strlen("ANIMATION");
                        }
                        if (strtoupper(iter->name).find("TEXTURE") == 0) {
                            xml->texturetemp.type = TEXTURE;
                            whichtype = XML::TEXTURE;
                            strsize = (uint32bit) strlen("TEXTURE");
                        }
                        if (strtoupper(iter->name).find("ALPHAMAP") == 0) {
                            xml->texturetemp.type = ALPHAMAP;
                            whichtype = XML::ALPHAMAP;
                            strsize = (uint32bit) strlen("ALPHAMAP");
                        }
                        if (strtoupper(iter->name).find("TECHNIQUE") == 0) {
                            xml->texturetemp.type = TECHNIQUE;
                            whichtype = XML::TECHNIQUE;
                            strsize = (uint32bit) strlen("TECHNIQUE");
                        }
                        if (whichtype != XML::UNKNOWN) {
                            uint32bit texindex = 0;
                            string ind(iter->name.substr(strsize));
                            if (!ind.empty()) {
                                texindex = atoi(ind.c_str());
                            }
                            xml->texturetemp.index = texindex;
                            xml->texturetemp.name = vector<uchar8bit>();
                            string nomdujour = iter->value.c_str();
                            for (size_t tni = 0; tni < nomdujour.size(); tni++) {
                                xml->texturetemp.name.push_back(nomdujour[tni]);
                            }
                            xml->textures.push_back(xml->texturetemp);
                        }
                        break;
                    }
                }
            }
        case XML::POINTS:
            break;
        case XML::POINT:
            memset(&xml->vertex, 0, sizeof(xml->vertex));
            break;
        case XML::LOCATION:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::X:
                        xml->vertex.x = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::Y:
                        xml->vertex.y = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::Z:
                        xml->vertex.z = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::S:
                        xml->vertex.s = float(XMLSupport::parse_float((*iter).value)) * (flips ? -1 : +1);
                        break;
                    case XML::T:
                        xml->vertex.t = float(XMLSupport::parse_float((*iter).value)) * (flipt ? -1 : +1);
                        break;
                }
            }
            break;
        case XML::NORMAL:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::I:
                        xml->vertex.i = float(XMLSupport::parse_float((*iter).value)) * (flipn ? -1 : +1);
                        break;
                    case XML::J:
                        xml->vertex.j = float(XMLSupport::parse_float((*iter).value)) * (flipn ? -1 : +1);
                        break;
                    case XML::K:
                        xml->vertex.k = float(XMLSupport::parse_float((*iter).value)) * (flipn ? -1 : +1);
                        break;
                }
            }
            break;
        case XML::POLYGONS:
            break;
        case XML::LINE:
            xml->linetemp.clear();
            xml->curpolytype = LINE;
            xml->curpolyindex = 0;
            break;
        case XML::TRI:
            xml->triangletemp.clear();
            xml->curpolytype = TRIANGLE;
            xml->curpolyindex = 0;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            xml->triangletemp.flatshade = 1;
                        } else {
                            xml->triangletemp.flatshade = 0;
                        }
                        break;
                }
            }
            break;
        case XML::QUAD:
            xml->quadtemp.clear();
            xml->curpolytype = QUAD;
            xml->curpolyindex = 0;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            xml->quadtemp.flatshade = 1;
                        } else {
                            xml->quadtemp.flatshade = 0;
                        }
                        break;
                }
            }
            break;
        case XML::LINESTRIP:     //FIXME?
            xml->striptemp = strip();
            xml->curpolytype = LINESTRIP;
            xml->striptemp.flatshade = 0;
            break;
        case XML::TRISTRIP:     //FIXME?
            xml->striptemp = strip();
            xml->curpolytype = TRISTRIP;
            xml->striptemp.flatshade = 0;
            break;
        case XML::TRIFAN:     //FIXME?
            xml->striptemp = strip();
            xml->curpolytype = TRIFAN;
            xml->striptemp.flatshade = 0;
            break;
        case XML::QUADSTRIP:     //FIXME?
            xml->striptemp = strip();
            xml->curpolytype = QUADSTRIP;
            xml->striptemp.flatshade = 0;
            break;
        case XML::VERTEX: {
            uint32bit
                    index = 0; //FIXME Not all cases below initialized index; "=0" added temporarily by chuck_starchaser
            float32bit s, t;
            s = 0.0f;
            t = 0.0f;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::POINT:
                        index = XMLSupport::parse_int((*iter).value);
                        break;
                    case XML::S:
                        s = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::T:
                        t = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            if (index < xml->num_vertex_references.size()) {
                if (xml->num_vertex_references[index] == 0) {
                    xml->vertices[index].i = xml->vertices[index].j = xml->vertices[index].k = 0;
                    if (xml->vertices[index].s == 0) {
                        xml->vertices[index].s = s;
                    }
                    if (xml->vertices[index].t == 0) {
                        xml->vertices[index].t = t;
                    }
                }
                xml->num_vertex_references[index]++;
            }
            switch (xml->curpolytype) {
                case LINE:
                    xml->linetemp.indexref[xml->curpolyindex] = index;
                    xml->linetemp.s[xml->curpolyindex] = s;
                    xml->linetemp.t[xml->curpolyindex] = t;
                    if (xml->curpolyindex == 1) {
                        float x =
                                xml->vertices[xml->linetemp.indexref[1]].x
                                        - xml->vertices[xml->linetemp.indexref[0]].x;
                        float y =
                                xml->vertices[xml->linetemp.indexref[1]].y
                                        - xml->vertices[xml->linetemp.indexref[0]].y;
                        float z =
                                xml->vertices[xml->linetemp.indexref[1]].z
                                        - xml->vertices[xml->linetemp.indexref[0]].z;
                        float len = sqrt(x * x + y * y + z * z);
                        if (len > .0001) {
                            x /= len;
                            y /= len;
                            z /= len;
                        }
                        xml->vertices[xml->linetemp.indexref[0]].i = -x;
                        xml->vertices[xml->linetemp.indexref[0]].j = -y;
                        xml->vertices[xml->linetemp.indexref[0]].k = -z;
                        xml->vertices[xml->linetemp.indexref[1]].i = x;
                        xml->vertices[xml->linetemp.indexref[1]].j = y;
                        xml->vertices[xml->linetemp.indexref[1]].k = z;
                    }
                    break;
                case TRIANGLE:
                    xml->triangletemp.indexref[xml->curpolyindex] = index;
                    xml->triangletemp.s[xml->curpolyindex] = s;
                    xml->triangletemp.t[xml->curpolyindex] = t;
                    if (xml->curpolyindex == 2) {
                        GFXVertex temp;
                        SetNormal(temp,
                                xml->vertices[xml->triangletemp.indexref[2]],
                                xml->vertices[xml->triangletemp.indexref[1]],
                                xml->vertices[xml->triangletemp.indexref[0]]);
                        AddNormal(xml->vertices[xml->triangletemp.indexref[0]],
                                temp);
                        AddNormal(xml->vertices[xml->triangletemp.indexref[1]],
                                temp);
                        AddNormal(xml->vertices[xml->triangletemp.indexref[2]],
                                temp);
                    }
                    break;
                case QUAD:
                    xml->quadtemp.indexref[xml->curpolyindex] = index;
                    xml->quadtemp.s[xml->curpolyindex] = s;
                    xml->quadtemp.t[xml->curpolyindex] = t;
                    if (xml->curpolyindex == 3) {
                        GFXVertex temp;
                        SetNormal(temp,
                                xml->vertices[xml->quadtemp.indexref[2]],
                                xml->vertices[xml->quadtemp.indexref[1]],
                                xml->vertices[xml->quadtemp.indexref[0]]);
                        AddNormal(xml->vertices[xml->quadtemp.indexref[0]],
                                temp);
                        AddNormal(xml->vertices[xml->quadtemp.indexref[1]],
                                temp);
                        AddNormal(xml->vertices[xml->quadtemp.indexref[2]],
                                temp);
                        AddNormal(xml->vertices[xml->quadtemp.indexref[3]],
                                temp);
                    }
                    break;
                case LINESTRIP:
                case TRISTRIP:
                case TRIFAN:
                case QUADSTRIP:
                    xml->stripelementtemp = stripelement();
                    xml->stripelementtemp.indexref = index;
                    xml->stripelementtemp.s = s;
                    xml->stripelementtemp.t = t;
                    xml->striptemp.points.push_back(xml->stripelementtemp);
                    if (xml->striptemp.points.size() > 2
                            && (xml->curpolytype != QUADSTRIP
                                    || xml->striptemp.points.size() % 2 == 0)) {
                        GFXVertex temp;
                        bool rev = ((xml->striptemp.points.size() % 2 == 0)
                                && xml->curpolytype == TRISTRIP)
                                || xml->curpolytype == QUADSTRIP;
                        SetNormal(temp,
                                xml->vertices[xml->striptemp.points[xml->striptemp.points.size()
                                        - (rev ? 3 : 2)].indexref],
                                xml->vertices[xml->striptemp.points[xml->striptemp.points.size()
                                        - (rev ? 2 : 3)].indexref],
                                xml->vertices[xml->striptemp.points.back().indexref]);
                        AddNormal(xml->vertices[xml->striptemp.points.back().indexref],
                                temp);
                        AddNormal(xml->vertices[xml->striptemp.points[xml->striptemp.points.size() - 2].indexref],
                                temp);
                        AddNormal(xml->vertices[xml->striptemp.points[xml->striptemp.points.size() - 3].indexref],
                                temp);
                        if (xml->curpolytype == QUADSTRIP) {
                            AddNormal(xml->vertices[xml->striptemp.points[xml->striptemp.points.size() - 4].indexref],
                                    temp);
                        }
                    }
                    break;
            }
            xml->curpolyindex += 1;
            break;
        }
        case XML::LOD:     //FIXME?
        {
            xml->lodtemp = LODholder();
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::UNKNOWN:
                        break;
                    case XML::FRAMESPERSECOND:
                        continue;
                        break;
                    case XML::SIZE:
                        xml->lodtemp.size = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::LODFILE:
                        string lodname = (*iter).value.c_str();
                        xml->lodtemp.name = vector<uchar8bit>();
                        for (size_t index = 0; index < lodname.size(); index++) {
                            xml->lodtemp.name.push_back(lodname[index]);
                        }
                        break;
                }
            }
            xml->LODs.push_back(xml->lodtemp);
            break;
        }
        case XML::LOGO: {
            uint32bit typ = 0; //FIXME Not all cases below initialized! "=0" added temporarily by chuck_starchaser
            float32bit rot, siz, offset;
            rot = siz = offset =
                    0.0f; //FIXME Not all cases below initialized! This line added temporarily by chuck_starchaser
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::UNKNOWN:
                        fprintf(stderr, "Unknown attribute '%s' encountered in Vertex tag\n", (*iter).name.c_str());
                        break;
                    case XML::TYPE:
                        typ = XMLSupport::parse_int((*iter).value);
                        break;
                    case XML::ROTATE:
                        rot = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::SIZE:
                        siz = float(XMLSupport::parse_float((*iter).value));
                        break;
                    case XML::OFFSET:
                        offset = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            xml->logos.push_back(XML::ZeLogo());
            xml->logos[xml->logos.size() - 1].type = typ;
            xml->logos[xml->logos.size() - 1].rotate = rot;
            xml->logos[xml->logos.size() - 1].size = siz;
            xml->logos[xml->logos.size() - 1].offset = offset;
            break;
        }
        case XML::REF:     //FIXME
        {
            uint32bit ind = 0;
            float32bit indweight = 1;
            bool foundindex = false;
            for (iter = attributes.begin(); iter != attributes.end() && !foundindex; iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::UNKNOWN:
                        fprintf(stderr, "Unknown attribute '%s' encountered in Vertex tag\n", (*iter).name.c_str());
                        break;
                    case XML::POINT:
                        ind = XMLSupport::parse_int((*iter).value);
                        foundindex = true;
                        break;
                    case XML::WEIGHT:
                        indweight = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            xml->logos[xml->logos.size() - 1].refpnt.push_back(ind);
            xml->logos[xml->logos.size() - 1].refweight.push_back(indweight);
            break;
        }
        case XML::ANIMDEF:
            xml->animdeftemp = animdef();
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::ANIMATIONNAME: {
                        string animname = (*iter).value.c_str();
                        xml->animdeftemp.name = vector<uchar8bit>();
                        for (size_t index = 0; index < animname.size(); index++) {
                            xml->animdeftemp.name.push_back(animname[index]);
                        }
                        break;
                    }
                    case XML::FRAMESPERSECOND:
                        xml->animdeftemp.FPS = float(XMLSupport::parse_float((*iter).value));
                        break;
                }
            }
            break;
        case XML::ANIMATIONFRAMEINDEX:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::ANIMATIONMESHINDEX:
                        xml->animdeftemp.meshoffsets.push_back(XMLSupport::parse_int((*iter).value));
                        break;
                }
            }
            break;
        case XML::ANIMFRAME:
            xml->animframetemp = animframe();
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::FRAMEMESHNAME:
                        string framename = (*iter).value.c_str();
                        xml->animframetemp.name = vector<uchar8bit>();
                        for (size_t index = 0; index < framename.size(); index++) {
                            xml->animframetemp.name.push_back(framename[index]);
                        }
                        break;
                }
            }
            xml->animframes.push_back(xml->animframetemp);
            break;
        default:
            break;
    }
}

void endElement(const string &name, XML *xml) {
    xml->state_stack.pop_back();
    XML::Names elem = (XML::Names) XML::element_map.lookup(name);
    switch (elem) {
        case XML::UNKNOWN:
            fprintf(stderr, "Unknown element end tag '%s' detected\n", name.c_str());
            break;
        case XML::POINT:
            xml->vertices.push_back(xml->vertex);
            xml->num_vertex_references.push_back(0);
            break;
        case XML::VERTEX:
            break;
        case XML::POINTS:
            break;
        case XML::LINE:
            xml->lines.push_back(xml->linetemp);
            break;
        case XML::TRI:
            xml->tris.push_back(xml->triangletemp);
            break;
        case XML::QUAD:
            xml->quads.push_back(xml->quadtemp);
            break;
        case XML::LINESTRIP:     //FIXE?
            xml->linestrips.push_back(xml->striptemp);
            break;
        case XML::TRISTRIP:     //FIXME?
            xml->tristrips.push_back(xml->striptemp);
            break;
        case XML::TRIFAN:     //FIXME?
            xml->trifans.push_back(xml->striptemp);
            break;
        case XML::QUADSTRIP:     //FIXME?
            xml->quadstrips.push_back(xml->striptemp);
            break;
        case XML::POLYGONS: {
            for (size_t i = 0; (i < xml->vertices.size()) && (i < xml->num_vertex_references.size()); i++) {
                float f = ((xml->num_vertex_references[i] > 0) ? 1.f / xml->num_vertex_references[i] : 1.f);
                xml->vertices[i].i *= f;
                xml->vertices[i].j *= f;
                xml->vertices[i].k *= f;
            }
            break;
        }
        case XML::REF:
            break;
        case XML::LOGO:
            break;
        case XML::MATERIAL:
            break;
        case XML::DIFFUSE:
            break;
        case XML::EMISSIVE:
            break;
        case XML::SPECULAR:
            break;
        case XML::AMBIENT:
            break;
        case XML::MESH:
            break;
        case XML::ANIMDEF:
            xml->animdefs.push_back(xml->animdeftemp);
            break;
        default:;
    }
}

void beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
    beginElement(name, AttributeList(atts), (XML *) userData);
}

void endElement(void *userData, const XML_Char *name) {
    endElement(name, (XML *) userData);
}

XML LoadXML(const char *filename, float32bit unitscale) {
    const uint32bit chunk_size = 16384;
    FILE *inFile = fopen(filename, "r");
    if (!inFile) {
        fprintf(stderr, "Cannot Open Mesh File %s\n", filename);
        exit(0);
        return XML();
    }
    XML xml;
    xml.scale = unitscale;
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &xml);
    XML_SetElementHandler(parser, &beginElement, &endElement);
    do {
        char buf[chunk_size];
        uint32bit length;

        length = (int) fread(buf, 1, chunk_size, inFile);
        XML_Parse(parser, buf, length, feof(inFile));
    } while (!feof(inFile));
    fclose(inFile);
    XML_ParserFree(parser);
    //Now, copy everything into the mesh data structures
//boundscheck(xml);
    return xml;
}

static size_t bogus_return; //added by chuck_starchaser, to get rid of ignored return warnings

void xmeshToBFXM(XML memfile, FILE *Outputfile, char mode, bool forcenormals) {
    //converts input file to BFXM creates new, or appends record based on mode
    uint32bit intbuf;

    bool append = (mode == 'a');

    uint32bit runningbytenum = 0;
    if (!append) {
        runningbytenum += writesuperheader(memfile, Outputfile);
    }          //Write superheader
    fseek(Outputfile, 0, SEEK_END);
    runningbytenum += appendrecordfromxml(memfile, Outputfile, forcenormals);     //Append one record

    rewind(Outputfile);
    fseek(Outputfile, 4 + 7 * sizeof(uint32bit), SEEK_SET);
    bogus_return = fread(&intbuf, sizeof(uint32bit), 1, Outputfile);      //Current number of records
    intbuf = VSSwapHostIntToLittle(intbuf);
    ++intbuf;
    intbuf = VSSwapHostIntToLittle(intbuf);
    fseek(Outputfile, 4 + 7 * sizeof(uint32bit), SEEK_SET);
    fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);      //number of records++

    fseek(Outputfile, 4 + sizeof(uint32bit), SEEK_SET);
    bogus_return = fread(&intbuf, sizeof(uint32bit), 1, Outputfile);      //Current length of file
    intbuf = VSSwapHostIntToLittle(intbuf);
    intbuf += runningbytenum;
    intbuf = VSSwapHostIntToLittle(intbuf);
    fseek(Outputfile, 4 + sizeof(uint32bit), SEEK_SET);
    fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);      //Correct number of bytes for total file
}

uint32bit writesuperheader(XML memfile, FILE *Outputfile) {
    uint32bit intbuf;
    uint32bit versionnumber = VSSwapHostIntToLittle(20);
    uchar8bit bytebuf;
    uint32bit runningbytenum = 0;
    //SUPER HEADER
    bytebuf = 'B';     //"Magic Word"
    runningbytenum += (uint32bit) fwrite(&bytebuf, 1, 1, Outputfile);
    bytebuf = 'F';
    runningbytenum += (uint32bit) fwrite(&bytebuf, 1, 1, Outputfile);
    bytebuf = 'X';
    runningbytenum += (uint32bit) fwrite(&bytebuf, 1, 1, Outputfile);
    bytebuf = 'M';
    runningbytenum += (uint32bit) fwrite(&bytebuf, 1, 1, Outputfile);
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&versionnumber,
            sizeof(uint32bit),
            1,
            Outputfile);       //VERSION number for BinaryFormattedXMesh
    intbuf = VSSwapHostIntToLittle(0);     //Length of File Placeholder
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of bytes in file
    //Super-Header Meaty part
    intbuf = VSSwapHostIntToLittle(4 + (9 * sizeof(uint32bit)));      //Super-Header length in Bytes for version 0.10
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of bytes in Superheader
    intbuf = VSSwapHostIntToLittle(8);     //Number of fields per vertex
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //
    intbuf = VSSwapHostIntToLittle(1);     //Number of fields per polygon structure
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //
    intbuf = VSSwapHostIntToLittle(3);     //Number of fields per referenced vertex
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //
    intbuf = VSSwapHostIntToLittle(1);     //Number of fields per referenced animation
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //
    intbuf = VSSwapHostIntToLittle(0);     //Number of records - initially 0
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of records
    intbuf = VSSwapHostIntToLittle(1);     //Number of fields per animation def
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //
    return runningbytenum;
}

uint32bit appendrecordfromxml(XML memfile, FILE *Outputfile, bool forcenormals) {
    uint32bit intbuf;
    uint32bit runningbytenum = 0;
    //Record Header
    intbuf = VSSwapHostIntToLittle(12);     //Size of Record Header in bytes
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of bytes in record header
    intbuf = VSSwapHostIntToLittle(0);     //Size of Record in bytes
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of bytes in record
    intbuf = VSSwapHostIntToLittle((uint32bit) (1 + memfile.LODs.size()
            + memfile.animframes.size()));     //Number of meshes = 1 + numLODs + numAnims.
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of meshes
    runningbytenum += appendmeshfromxml(memfile, Outputfile, forcenormals);     //write top level mesh
    size_t mesh;
    for (mesh = 0; mesh < memfile.LODs.size(); mesh++) {
        //write all LOD meshes
        string LODname = "";
        for (size_t i = 0; i < memfile.LODs[mesh].name.size(); i++) {
            LODname += memfile.LODs[mesh].name[i];
        }
        XML submesh = LoadXML(LODname.c_str(), 1);
        runningbytenum += appendmeshfromxml(submesh, Outputfile, forcenormals);
    }
    for (mesh = 0; mesh < memfile.animframes.size(); mesh++) {
        //write all Animation Frames
        string animname = "";
        for (size_t i = 0; i < memfile.animframes[mesh].name.size(); i++) {
            animname += memfile.animframes[mesh].name[i];
        }
        XML submesh = LoadXML(animname.c_str(), 1);
        runningbytenum += appendmeshfromxml(submesh, Outputfile, forcenormals);
    }
    fseek(Outputfile, (-1 * ((long) runningbytenum)) + 4, SEEK_CUR);
    intbuf = runningbytenum;
    intbuf = VSSwapHostIntToLittle(intbuf);
    fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);      //Correct number of bytes for total record
    fseek(Outputfile, 0, SEEK_END);
    return runningbytenum;
}

static float mymax(float a, float b) {
    return a > b ? a : b;
}

void NormalizeProperty(float &r, float &g, float &b, float &a) {
    if (r > 1 || g > 1 || b > 1 || a > 1) {
        float mx = mymax(mymax(mymax(r, g), b), a);
        r /= mx;
        g /= mx;
        b /= mx;
        if (a > 1) {          //sometimes they get the alpha right
            a /= mx;
        }
    }
}

void NormalizeMaterial(GFXMaterial &m) {
    NormalizeProperty(m.ar, m.ag, m.ab, m.aa);
    NormalizeProperty(m.dr, m.dg, m.db, m.da);
    NormalizeProperty(m.sr, m.sg, m.sb, m.sa);
    NormalizeProperty(m.er, m.eg, m.eb, m.ea);
}

uint32bit appendmeshfromxml(XML memfile, FILE *Outputfile, bool forcenormals) {
    float transx = float(atof(Converter::getNamedOption("addx").c_str()));
    float transy = float(atof(Converter::getNamedOption("addy").c_str()));
    float transz = float(atof(Converter::getNamedOption("addz").c_str()));
    long meshlengthpos, vsalengthpos, startpos;
    uint32bit intbuf;
    float32bit floatbuf;
    uchar8bit bytebuf;
    uint32bit runningbytenum = 0;
    //Mesh Header
    startpos = ftell(Outputfile);
    intbuf = VSSwapHostIntToLittle(11 * sizeof(uint32bit) + 20 * sizeof(float32bit));
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Size of Mesh header in Bytes
    intbuf = VSSwapHostIntToLittle(0);     //Temp - rewind and fix.
    meshlengthpos = ftell(Outputfile);
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Size of this Mesh in Bytes
    floatbuf = VSSwapHostFloatToLittle(memfile.scale);
    runningbytenum +=
            sizeof(float32bit) * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Mesh Scale
    intbuf = VSSwapHostIntToLittle(memfile.reverse);
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //reverse flag
    intbuf = VSSwapHostIntToLittle(memfile.force_texture);
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Force texture flag
    intbuf = VSSwapHostIntToLittle(memfile.sharevert);
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Share vertex flag
    floatbuf = VSSwapHostFloatToLittle(memfile.polygon_offset);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Polygon offset
    intbuf = VSSwapHostIntToLittle(memfile.blend_src);
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Blend Source
    intbuf = VSSwapHostIntToLittle(memfile.blend_dst);
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Blend Destination
    NormalizeMaterial(memfile.material);
    floatbuf = VSSwapHostFloatToLittle(memfile.material.power);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Specular:Power
    floatbuf = VSSwapHostFloatToLittle(memfile.material.ar);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Ambient:Red
    floatbuf = VSSwapHostFloatToLittle(memfile.material.ag);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Ambient:Green
    floatbuf = VSSwapHostFloatToLittle(memfile.material.ab);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Ambient:Blue
    floatbuf = VSSwapHostFloatToLittle(memfile.material.aa);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Ambient:Alpha
    floatbuf = VSSwapHostFloatToLittle(memfile.material.dr);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Diffuse:Red
    floatbuf = VSSwapHostFloatToLittle(memfile.material.dg);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Diffuse:Green
    floatbuf = VSSwapHostFloatToLittle(memfile.material.db);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Diffuse:Blue
    floatbuf = VSSwapHostFloatToLittle(memfile.material.da);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Diffuse:Alpha
    floatbuf = VSSwapHostFloatToLittle(memfile.material.er);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Emissive:Red
    floatbuf = VSSwapHostFloatToLittle(memfile.material.eg);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Emissive:Green
    floatbuf = VSSwapHostFloatToLittle(memfile.material.eb);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Emissive:Blue
    floatbuf = VSSwapHostFloatToLittle(memfile.material.ea);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Emissive:Alpha
    floatbuf = VSSwapHostFloatToLittle(memfile.material.sr);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Specular:Red
    floatbuf = VSSwapHostFloatToLittle(memfile.material.sg);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Specular:Green
    floatbuf = VSSwapHostFloatToLittle(memfile.material.sb);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Specular:Blue
    floatbuf = VSSwapHostFloatToLittle(memfile.material.sa);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //Material:Specular:Alpha
    intbuf = VSSwapHostIntToLittle(memfile.cullface);
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Cullface
    intbuf = VSSwapHostIntToLittle(memfile.lighting);
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //lighting
    intbuf = VSSwapHostIntToLittle(memfile.reflect);
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //reflect
    //Usenormals default value fix.
    if (forcenormals) {
        memfile.usenormals = true;
    }
    intbuf = VSSwapHostIntToLittle(memfile.usenormals);
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //usenormals
    //added by hellcatv
    floatbuf = VSSwapHostFloatToLittle(memfile.alphatest);
    runningbytenum += sizeof(float32bit)
            * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);       //alpha test value
    //END HEADER
    //Begin Variable sized Attributes
    uint32bit VSAstart = ftell(Outputfile);
    vsalengthpos = ftell(Outputfile);
    intbuf = VSSwapHostIntToLittle(0);     //Temp value will overwrite later
    runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
            sizeof(uint32bit),
            1,
            Outputfile);       //Length of Variable sized attribute section in bytes
    //Detail texture
    {
        uint32bit namelen = (uint32bit) memfile.detailtexture.name.size();
        intbuf = VSSwapHostIntToLittle(namelen);
        runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                sizeof(uint32bit),
                1,
                Outputfile);           //Length of name of detail texture
        uint32bit nametmp;
        for (nametmp = 0; nametmp < namelen; nametmp++) {
            bytebuf = memfile.detailtexture.name[nametmp];
            runningbytenum += (uint32bit) fwrite(&bytebuf,
                    sizeof(uchar8bit),
                    1,
                    Outputfile);              //char by char name of detail texture
        }
        uint32bit padlength = (sizeof(uint32bit) - (namelen % sizeof(uint32bit))) % sizeof(uint32bit);
        for (nametmp = 0; nametmp < padlength; nametmp++) {
            bytebuf = 0;
            runningbytenum += (uint32bit) fwrite(&bytebuf,
                    sizeof(uchar8bit),
                    1,
                    Outputfile);              //Padded so that next field is word aligned
        }
    }
    //Detail Planes
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.detailplanes.size());
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of detail planes
    for (size_t plane = 0; plane < memfile.detailplanes.size(); plane++) {
        floatbuf = VSSwapHostFloatToLittle(memfile.detailplanes[plane].x);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //Detail Plane:X
        floatbuf = VSSwapHostFloatToLittle(memfile.detailplanes[plane].y);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //Detail Plane:Y
        floatbuf = VSSwapHostFloatToLittle(memfile.detailplanes[plane].z);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //Detail Plane:Z
    }
    //Textures
    {
        size_t texnum;
        intbuf = memfile.textures.size();
        intbuf = VSSwapHostIntToLittle(intbuf);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Number of textures
        for (texnum = 0; texnum < memfile.textures.size(); texnum++) {
            intbuf = VSSwapHostIntToLittle(memfile.textures[texnum].type);
            runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                    sizeof(uint32bit),
                    1,
                    Outputfile);               //texture # texnum: type
            intbuf = VSSwapHostIntToLittle(memfile.textures[texnum].index);
            runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                    sizeof(uint32bit),
                    1,
                    Outputfile);               //texture # texnum: index
            uint32bit namelen = (uint32bit) memfile.textures[texnum].name.size();
            intbuf = VSSwapHostIntToLittle(namelen);
            runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                    sizeof(uint32bit),
                    1,
                    Outputfile);               //Length of name of texture # texnum
            uint32bit nametmp;
            for (nametmp = 0; nametmp < namelen; nametmp++) {
                bytebuf = memfile.textures[texnum].name[nametmp];
                runningbytenum += (uint32bit) fwrite(&bytebuf,
                        sizeof(uchar8bit),
                        1,
                        Outputfile);                  //Name of texture # texnum
            }
            uint32bit padlength = (sizeof(uint32bit) - (namelen % sizeof(uint32bit))) % sizeof(uint32bit);
            for (nametmp = 0; nametmp < padlength; nametmp++) {
                bytebuf = 0;
                runningbytenum += (uint32bit) fwrite(&bytebuf,
                        sizeof(uchar8bit),
                        1,
                        Outputfile);                  //Padded so that next field is word aligned
            }
        }
    }
    //Logos
    //FIXME?
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.logos.size());
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of logos
    for (size_t logonum = 0; logonum < memfile.logos.size(); logonum++) {
        floatbuf = VSSwapHostFloatToLittle(memfile.logos[logonum].size);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //logo # logonum: size
        floatbuf = VSSwapHostFloatToLittle(memfile.logos[logonum].offset);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //logo # logonum: offset
        floatbuf = VSSwapHostFloatToLittle(memfile.logos[logonum].rotate);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //logo # logonum: rotation
        intbuf = VSSwapHostIntToLittle(memfile.logos[logonum].type);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //logo # logonum: type
        uint32bit numrefs = (uint32bit) memfile.logos[logonum].refpnt.size();
        intbuf = VSSwapHostIntToLittle(numrefs);
        runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                sizeof(uint32bit),
                1,
                Outputfile);           //logo # logonum: number of references
        for (uint32bit ref = 0; ref < numrefs; ref++) {
            intbuf = VSSwapHostIntToLittle(memfile.logos[logonum].refpnt[ref]);
            runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                    sizeof(uint32bit),
                    1,
                    Outputfile);               //logo # logonum: reference # ref
            floatbuf = VSSwapHostFloatToLittle(memfile.logos[logonum].refweight[ref]);
            runningbytenum += sizeof(float32bit) * (uint32bit) fwrite(&floatbuf,
                    sizeof(float32bit),
                    1,
                    Outputfile);               //logo # logonum: reference # ref weight
        }
    }
    //LODs + Animations
    //LODs
    uint32bit submeshref = 1;
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.LODs.size());
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of LODs
    for (size_t lod = 0; lod < memfile.LODs.size(); lod++) {
        floatbuf = VSSwapHostFloatToLittle(memfile.LODs[lod].size);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //LOD # lod: size
        intbuf = submeshref;
        intbuf = VSSwapHostIntToLittle(intbuf);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //LOD mesh offset
        submeshref++;
    }
    //Current VS File format is not compatible with new animation specification - can't test until I fix old files (only 1 at present uses animations)
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.animdefs.size());
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of animdefs
    for (size_t anim = 0; anim < memfile.animdefs.size(); anim++) {
        uint32bit namelen = (uint32bit) memfile.animdefs[anim].name.size();
        intbuf = VSSwapHostIntToLittle(namelen);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Length of name animation
        uint32bit nametmp;
        for (nametmp = 0; nametmp < namelen; nametmp++) {
            bytebuf = memfile.animdefs[anim].name[nametmp];
            runningbytenum +=
                    (uint32bit) fwrite(&bytebuf, sizeof(uchar8bit), 1, Outputfile);              //char by char of above
        }
        uint32bit padlength = (sizeof(uint32bit) - (namelen % sizeof(uint32bit))) % sizeof(uint32bit);
        for (nametmp = 0; nametmp < padlength; nametmp++) {
            bytebuf = 0;
            runningbytenum += (uint32bit) fwrite(&bytebuf,
                    sizeof(uchar8bit),
                    1,
                    Outputfile);              //Padded so that next field is word aligned
        }
        floatbuf = VSSwapHostFloatToLittle(memfile.animdefs[anim].FPS);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //Animdef # anim: FPS
        intbuf = VSSwapHostIntToLittle((uint32bit) memfile.animdefs[anim].meshoffsets.size());
        runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                sizeof(uint32bit),
                1,
                Outputfile);           //number of animation mesh offsets
        for (size_t offset = 0; offset < memfile.animdefs[anim].meshoffsets.size(); offset++) {
            intbuf = submeshref + memfile.animdefs[anim].meshoffsets[offset];
            intbuf = VSSwapHostIntToLittle(intbuf);
            runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                    sizeof(uint32bit),
                    1,
                    Outputfile);               //animation mesh offset
        }
    }
    //End Variable sized Attributes
    uint32bit VSAend = ftell(Outputfile);
    //GEOMETRY
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.vertices.size());
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of vertices
    for (size_t verts = 0; verts < memfile.vertices.size(); verts++) {
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].x + transx);
        float normallen = sqrt(memfile.vertices[verts].i * memfile.vertices[verts].i
                + memfile.vertices[verts].j * memfile.vertices[verts].j
                + memfile.vertices[verts].k * memfile.vertices[verts].k);
        if (normallen > .0001) {
            memfile.vertices[verts].i /= normallen;
            memfile.vertices[verts].j /= normallen;
            memfile.vertices[verts].k /= normallen;
        }
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:x
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].y + transy);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:y
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].z + transz);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:z
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].i);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:i
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].j);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:j
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].k);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:k
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].s);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:s
        floatbuf = VSSwapHostFloatToLittle(memfile.vertices[verts].t);
        runningbytenum += sizeof(float32bit)
                * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);           //vertex #vert:t
    }
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.lines.size());
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of lines
    for (size_t lines = 0; lines < memfile.lines.size(); lines++) {
        intbuf = VSSwapHostIntToLittle(memfile.lines[lines].flatshade);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Flatshade flag
        for (uint32bit tmpcounter = 0; tmpcounter < 2; tmpcounter++) {
            intbuf = VSSwapHostIntToLittle(memfile.lines[lines].indexref[tmpcounter]);
            runningbytenum += sizeof(uint32bit)
                    * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);               //point index
            floatbuf = VSSwapHostFloatToLittle(memfile.lines[lines].s[tmpcounter]);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //s coord
            floatbuf = VSSwapHostFloatToLittle(memfile.lines[lines].t[tmpcounter]);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //t coord
        }
    }
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.tris.size());
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of triangles
    for (size_t tris = 0; tris < memfile.tris.size(); tris++) {
        intbuf = VSSwapHostIntToLittle(memfile.tris[tris].flatshade);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Flatshade flag
        for (uint32bit tmpcounter = 0; tmpcounter < 3; tmpcounter++) {
            intbuf = VSSwapHostIntToLittle(memfile.tris[tris].indexref[tmpcounter]);
            runningbytenum += sizeof(uint32bit)
                    * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);               //point index
            floatbuf = VSSwapHostFloatToLittle(memfile.tris[tris].s[tmpcounter]);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //s coord
            floatbuf = VSSwapHostFloatToLittle(memfile.tris[tris].t[tmpcounter]);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //t coord
        }
    }
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.quads.size());
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of Quads
    for (size_t quads = 0; quads < memfile.quads.size(); quads++) {
        intbuf = VSSwapHostIntToLittle(memfile.quads[quads].flatshade);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Flatshade flag
        for (uint32bit tmpcounter = 0; tmpcounter < 4; tmpcounter++) {
            intbuf = VSSwapHostIntToLittle(memfile.quads[quads].indexref[tmpcounter]);
            runningbytenum += sizeof(uint32bit)
                    * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);               //point index
            floatbuf = VSSwapHostFloatToLittle(memfile.quads[quads].s[tmpcounter]);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //s coord
            floatbuf = VSSwapHostFloatToLittle(memfile.quads[quads].t[tmpcounter]);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //t coord
        }
    }
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.linestrips.size());
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of linestrips
    for (size_t ls = 0; ls < memfile.linestrips.size(); ls++) {
        intbuf = VSSwapHostIntToLittle((uint32bit) memfile.linestrips[ls].points.size());
        runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                sizeof(uint32bit),
                1,
                Outputfile);           //Number of elements in current linestrip
        intbuf = VSSwapHostIntToLittle(memfile.linestrips[ls].flatshade);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Flatshade flag
        for (size_t tmpcounter = 0; tmpcounter < memfile.linestrips[ls].points.size(); tmpcounter++) {
            intbuf = VSSwapHostIntToLittle(memfile.linestrips[ls].points[tmpcounter].indexref);
            runningbytenum += sizeof(uint32bit)
                    * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);               //point index
            floatbuf = VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].s);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //s coord
            floatbuf = VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].t);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //t coord
        }
    }
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.tristrips.size());
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of tristrips
    for (size_t ts = 0; ts < memfile.tristrips.size(); ts++) {
        intbuf = VSSwapHostIntToLittle((uint32bit) memfile.tristrips[ts].points.size());
        runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                sizeof(uint32bit),
                1,
                Outputfile);           //Number of elements in current tristrip
        intbuf = VSSwapHostIntToLittle(memfile.tristrips[ts].flatshade);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Flatshade flag
        for (size_t tmpcounter = 0; tmpcounter < memfile.tristrips[ts].points.size(); tmpcounter++) {
            intbuf = VSSwapHostIntToLittle(memfile.tristrips[ts].points[tmpcounter].indexref);
            runningbytenum += sizeof(uint32bit)
                    * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);               //point index
            floatbuf = VSSwapHostFloatToLittle(memfile.tristrips[ts].points[tmpcounter].s);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //s coord
            floatbuf = VSSwapHostFloatToLittle(memfile.tristrips[ts].points[tmpcounter].t);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //t coord
        }
    }
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.trifans.size());
    runningbytenum +=
            sizeof(uint32bit) * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of trifans
    for (size_t tf = 0; tf < memfile.trifans.size(); tf++) {
        intbuf = VSSwapHostIntToLittle((uint32bit) memfile.trifans[tf].points.size());
        runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                sizeof(uint32bit),
                1,
                Outputfile);           //Number of elements in current trifan
        intbuf = VSSwapHostIntToLittle(memfile.trifans[tf].flatshade);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Flatshade flag
        for (size_t tmpcounter = 0; tmpcounter < memfile.trifans[tf].points.size(); tmpcounter++) {
            intbuf = VSSwapHostIntToLittle(memfile.trifans[tf].points[tmpcounter].indexref);
            runningbytenum += sizeof(uint32bit)
                    * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);               //point index
            floatbuf = VSSwapHostFloatToLittle(memfile.trifans[tf].points[tmpcounter].s);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //s coord
            floatbuf = VSSwapHostFloatToLittle(memfile.trifans[tf].points[tmpcounter].t);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //t coord
        }
    }
    intbuf = VSSwapHostIntToLittle((uint32bit) memfile.quadstrips.size());
    runningbytenum += sizeof(uint32bit)
            * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);       //Number of quadstrips
    for (size_t qs = 0; qs < memfile.quadstrips.size(); qs++) {
        intbuf = VSSwapHostIntToLittle((uint32bit) memfile.quadstrips[qs].points.size());
        runningbytenum += sizeof(uint32bit) * (uint32bit) fwrite(&intbuf,
                sizeof(uint32bit),
                1,
                Outputfile);           //Number of elements in current quadstrip
        intbuf = VSSwapHostIntToLittle(memfile.quadstrips[qs].flatshade);
        runningbytenum += sizeof(uint32bit)
                * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);           //Flatshade flag
        for (size_t tmpcounter = 0; tmpcounter < memfile.quadstrips[qs].points.size(); tmpcounter++) {
            intbuf = VSSwapHostIntToLittle(memfile.quadstrips[qs].points[tmpcounter].indexref);
            runningbytenum += sizeof(uint32bit)
                    * (uint32bit) fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);               //point index
            floatbuf = VSSwapHostFloatToLittle(memfile.quadstrips[qs].points[tmpcounter].s);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //s coord
            floatbuf = VSSwapHostFloatToLittle(memfile.quadstrips[qs].points[tmpcounter].t);
            runningbytenum += sizeof(float32bit)
                    * (uint32bit) fwrite(&floatbuf, sizeof(float32bit), 1, Outputfile);               //t coord
        }
    }
    //END GEOMETRY
    long endpos = ftell(Outputfile);
    fseek(Outputfile, meshlengthpos, SEEK_SET);
    intbuf = endpos - startpos;
    intbuf = VSSwapHostIntToLittle(intbuf);
    fwrite(&intbuf, sizeof(uint32bit), 1, Outputfile);      // Correct number of bytes for total mesh
    fseek(Outputfile, vsalengthpos, SEEK_SET);
    intbuf = VSAend - VSAstart;
    intbuf = VSSwapHostIntToLittle(intbuf);
    fwrite(&intbuf,
            sizeof(uint32bit),
            1,
            Outputfile);      // Correct number of bytes for Variable Sized Attribute section
    fseek(Outputfile, endpos, SEEK_SET);
    return runningbytenum;
}

