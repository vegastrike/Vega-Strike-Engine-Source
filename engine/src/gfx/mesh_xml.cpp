/*
 * mesh_xml.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, chuck_starchaser, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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


#include "mesh.h"
#include "mesh_xml.h"
#include "aux_texture.h"
#include "aux_logo.h"
#include "vegastrike.h"
#include <iostream>
//#include <fstream>
#include <expat.h>
#include <cfloat>
#include <cassert>
//#include "ani_texture.h"
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include <direct.h>
#include <process.h>
#endif
#if !defined (_WIN32) && !(defined (__APPLE__) || defined (MACOSX )) && !defined (BSD) && !defined(__HAIKU__)
#include <values.h>
#endif
#include "xml_support.h"
#include "vec.h"
//#include "config_xml.h"
#include "vs_globals.h"
//#include "cmd/script/mission.h"
#include "cmd/script/flightgroup.h"
#include "hashtable.h"
#include "vs_logging.h"
#include "vs_exit.h"
#include "preferred_types.h"

#ifdef max
#undef max
#endif

static inline float max(float x, float y) {
    if (x > y) {
        return x;
    } else {
        return y;
    }
}

#ifdef min
#undef min
#endif

static inline float min(float x, float y) {
    if (x < y) {
        return x;
    } else {
        return y;
    }
}

using namespace vega_types;

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
struct GFXMaterial;

const EnumMap::Pair MeshXML::element_names[] = {
        EnumMap::Pair("UNKNOWN", MeshXML::UNKNOWN),
        EnumMap::Pair("Material", MeshXML::MATERIAL),
        EnumMap::Pair("LOD", MeshXML::LOD),
        EnumMap::Pair("Ambient", MeshXML::AMBIENT),
        EnumMap::Pair("Diffuse", MeshXML::DIFFUSE),
        EnumMap::Pair("Specular", MeshXML::SPECULAR),
        EnumMap::Pair("Emissive", MeshXML::EMISSIVE),
        EnumMap::Pair("Mesh", MeshXML::MESH),
        EnumMap::Pair("Points", MeshXML::POINTS),
        EnumMap::Pair("Point", MeshXML::POINT),
        EnumMap::Pair("Location", MeshXML::LOCATION),
        EnumMap::Pair("Normal", MeshXML::NORMAL),
        EnumMap::Pair("Polygons", MeshXML::POLYGONS),
        EnumMap::Pair("Line", MeshXML::LINE),
        EnumMap::Pair("Tri", MeshXML::TRI),
        EnumMap::Pair("Quad", MeshXML::QUAD),
        EnumMap::Pair("Linestrip", MeshXML::LINESTRIP),
        EnumMap::Pair("Tristrip", MeshXML::TRISTRIP),
        EnumMap::Pair("Trifan", MeshXML::TRIFAN),
        EnumMap::Pair("Quadstrip", MeshXML::QUADSTRIP),
        EnumMap::Pair("Vertex", MeshXML::VERTEX),
        EnumMap::Pair("Logo", MeshXML::LOGO),
        EnumMap::Pair("Ref", MeshXML::REF),
        EnumMap::Pair("DetailPlane", MeshXML::DETAILPLANE)
};

const EnumMap::Pair MeshXML::attribute_names[] = {
        EnumMap::Pair("UNKNOWN", MeshXML::UNKNOWN),
        EnumMap::Pair("Scale", MeshXML::SCALE),
        EnumMap::Pair("Blend", MeshXML::BLENDMODE),
        EnumMap::Pair("alphatest", MeshXML::ALPHATEST),
        EnumMap::Pair("texture", MeshXML::TEXTURE),
        EnumMap::Pair("technique", MeshXML::TECHNIQUE),
        EnumMap::Pair("alphamap", MeshXML::ALPHAMAP),
        EnumMap::Pair("sharevertex", MeshXML::SHAREVERT),
        EnumMap::Pair("red", MeshXML::RED),
        EnumMap::Pair("green", MeshXML::GREEN),
        EnumMap::Pair("blue", MeshXML::BLUE),
        EnumMap::Pair("alpha", MeshXML::ALPHA),
        EnumMap::Pair("power", MeshXML::POWER),
        EnumMap::Pair("reflect", MeshXML::REFLECT),
        EnumMap::Pair("x", MeshXML::X),
        EnumMap::Pair("y", MeshXML::Y),
        EnumMap::Pair("z", MeshXML::Z),
        EnumMap::Pair("i", MeshXML::I),
        EnumMap::Pair("j", MeshXML::J),
        EnumMap::Pair("k", MeshXML::K),
        EnumMap::Pair("Shade", MeshXML::FLATSHADE),
        EnumMap::Pair("point", MeshXML::POINT),
        EnumMap::Pair("s", MeshXML::S),
        EnumMap::Pair("t", MeshXML::T),
        //Logo stuffs
        EnumMap::Pair("Type", MeshXML::TYPE),
        EnumMap::Pair("Rotate", MeshXML::ROTATE),
        EnumMap::Pair("Weight", MeshXML::WEIGHT),
        EnumMap::Pair("Size", MeshXML::SIZE),
        EnumMap::Pair("Offset", MeshXML::OFFSET),
        EnumMap::Pair("meshfile", MeshXML::LODFILE),
        EnumMap::Pair("Animation", MeshXML::ANIMATEDTEXTURE),
        EnumMap::Pair("Reverse", MeshXML::REVERSE),
        EnumMap::Pair("LightingOn", MeshXML::LIGHTINGON),
        EnumMap::Pair("CullFace", MeshXML::CULLFACE),
        EnumMap::Pair("ForceTexture", MeshXML::FORCETEXTURE),
        EnumMap::Pair("UseNormals", MeshXML::USENORMALS),
        EnumMap::Pair("UseTangents", MeshXML::USETANGENTS),
        EnumMap::Pair("PolygonOffset", MeshXML::POLYGONOFFSET),
        EnumMap::Pair("DetailTexture", MeshXML::DETAILTEXTURE),
        EnumMap::Pair("FramesPerSecond", MeshXML::FRAMESPERSECOND)
};

const EnumMap MeshXML::element_map
        (MeshXML::element_names,
                sizeof(MeshXML::element_names) / sizeof(MeshXML::element_names[0]));
const EnumMap MeshXML::attribute_map(MeshXML::attribute_names,
        sizeof(MeshXML::attribute_names) / sizeof(MeshXML::attribute_names[0]));

void Mesh::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
    MeshXML *xml = (MeshXML *) userData;
    xml->mesh->beginElement(xml, name, AttributeList(atts));
}

void Mesh::endElement(void *userData, const XML_Char *name) {
    MeshXML *xml = (MeshXML *) userData;
    xml->mesh->endElement(xml, std::string(name));
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

#if 0
std::string parse_alpha( enum BLENDMODE tmp )
{
    switch (tmp)
    {
    case ZERO:
        return "ZERO";

    case ONE:
        return "ONE";

    case SRCCOLOR:
        return "SRCCOLOR";

    case INVSRCCOLOR:
        return "INVSRCCOLOR";

    case SRCALPHA:
        return "SRCALPHA";

    case INVSRCALPHA:
        return "INVSRCALPHA";

    case DESTALPHA:
        return "DESTALPHA";

    case INVDESTALPHA:
        return "INVDESTALPHA";

    case DESTCOLOR:
        return "DESTCOLOR";

    case INVDESTCOLOR:
        return "INVDESTCOLOR";

    case SRCALPHASAT:
        return "SRCALPHASAT";

    case CONSTALPHA:
        return "CONSTALPHA";

    case INVCONSTALPHA:
        return "INVCONSTALPHA";

    case CONSTCOLOR:
        return "CONSTCOLOR";

    case INVCONSTCOLOR:
        return "INVCONSTCOLOR";
    }
    return ZERO;
}
#endif

/* Load stages:
 *
 *  0 - no tags seen
 *  1 - waiting for points
 *  2 - processing points
 *  3 - done processing points, waiting for face data
 *
 *  Note that this is only a debugging aid. Once DTD is written, there
 *  will be no need for this sort of checking
 */

bool shouldreflect(string r) {
    if (strtoupper(r) == "FALSE") {
        return false;
    }
    int i;
    for (i = 0; i < (int) r.length(); ++i) {
        if (r[i] != '0' && r[i] != '.' && r[i] != '+' && r[i] != 'e') {
            return true;
        }
    }
    return false;
}

void Mesh::beginElement(MeshXML *xml, const string &name, const AttributeList &attributes) {
    static bool use_detail_texture =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "use_detail_texture", "true"));
    //static bool flatshadeit=false;
    AttributeList::const_iterator iter;
    float flotsize = 1;
    MeshXML::Names elem = (MeshXML::Names) MeshXML::element_map.lookup(name);
    MeshXML::Names top = MeshXML::UNKNOWN;
    if (!xml->state_stack.empty()) {
        top = *xml->state_stack.rbegin();
    }
    xml->state_stack.push_back(elem);
    bool texture_found = false;
    switch (elem) {
        case MeshXML::DETAILPLANE:
            if (use_detail_texture) {
                Vector vec(detailPlanes.size() >= 2 ? 1 : 0,
                        detailPlanes.size() == 1 ? 1 : 0,
                        detailPlanes.size() == 0 ? 1 : 0);
                for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                    switch (MeshXML::attribute_map.lookup((*iter).name)) {
                        case MeshXML::X:
                            vec.i = XMLSupport::parse_float(iter->value);
                            break;
                        case MeshXML::Y:
                            vec.j = XMLSupport::parse_float(iter->value);
                            break;

                        case MeshXML::Z:
                            vec.k = XMLSupport::parse_float(iter->value);
                            break;
                    }
                }
                static float detailscale =
                        XMLSupport::parse_float(vs_config->getVariable("graphics", "detail_texture_scale", "1"));
                if (detailPlanes.size() < 6) {
                    detailPlanes.push_back(vec * detailscale);
                }
            }
            break;
        case MeshXML::MATERIAL:
            //assert(xml->load_stage==4);
            xml->load_stage = 7;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::USENORMALS:
                        xml->usenormals = XMLSupport::parse_bool(iter->value);
                        break;
                    case MeshXML::USETANGENTS:
                        xml->usetangents = XMLSupport::parse_bool(iter->value);
                        break;
                    case MeshXML::POWER:
                        xml->material.power = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::REFLECT:
                        setEnvMap(shouldreflect((*iter).value));
                        break;
                    case MeshXML::LIGHTINGON:
                        setLighting(XMLSupport::parse_bool(vs_config->getVariable("graphics",
                                "ForceLighting",
                                "true"))
                                || XMLSupport::parse_bool((*iter).value));
                        break;
                    case MeshXML::CULLFACE:
                        forceCullFace(XMLSupport::parse_bool((*iter).value));
                        break;
                }
            }
            break;
        case MeshXML::DIFFUSE:
            //assert(xml->load_stage==7);
            xml->load_stage = 8;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::RED:
                        xml->material.dr = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::BLUE:
                        xml->material.db = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::ALPHA:
                        xml->material.da = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::GREEN:
                        xml->material.dg = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            break;
        case MeshXML::EMISSIVE:
            //assert(xml->load_stage==7);
            xml->load_stage = 8;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::RED:
                        xml->material.er = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::BLUE:
                        xml->material.eb = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::ALPHA:
                        xml->material.ea = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::GREEN:
                        xml->material.eg = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            break;
        case MeshXML::SPECULAR:
            //assert(xml->load_stage==7);
            xml->load_stage = 8;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::RED:
                        xml->material.sr = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::BLUE:
                        xml->material.sb = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::ALPHA:
                        xml->material.sa = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::GREEN:
                        xml->material.sg = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            break;
        case MeshXML::AMBIENT:
            //assert(xml->load_stage==7);
            xml->load_stage = 8;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::RED:
                        xml->material.ar = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::BLUE:
                        xml->material.ab = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::ALPHA:
                        xml->material.aa = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::GREEN:
                        xml->material.ag = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            break;
        case MeshXML::UNKNOWN:
            VS_LOG(error, (boost::format("Unknown element start tag '%1%' detected") % name));
            break;
        case MeshXML::MESH:
            assert(xml->load_stage == 0);
            assert(xml->state_stack.size() == 1);
            xml->load_stage = 1;
            //Read in texture attribute
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::REVERSE:
                        xml->reverse = XMLSupport::parse_bool((*iter).value);
                        break;
                    case MeshXML::FORCETEXTURE:
                        xml->force_texture = XMLSupport::parse_bool((*iter).value);
                        break;
                    case MeshXML::SCALE:
                        xml->scale *= XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::SHAREVERT:
                        xml->sharevert =
                                (XMLSupport::parse_bool((*iter).value)
                                        && XMLSupport::parse_bool(vs_config->getVariable("graphics",
                                                "SharedVertexArrays",
                                                "true")));
                        break;
                    case MeshXML::POLYGONOFFSET:
                        this->polygon_offset = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::BLENDMODE: {
                        char *csrc = strdup((*iter).value.c_str());
                        char *cdst = strdup((*iter).value.c_str());
                        sscanf(((*iter).value).c_str(), "%s %s", csrc, cdst);
                        SetBlendMode(parse_alpha(csrc), parse_alpha(cdst));
                        free(csrc);
                        free(cdst);
                        break;
                    }
                    case MeshXML::ALPHATEST: {
                        float tmp = XMLSupport::parse_float((*iter).value);
                        if (tmp > 1) {
                            xml->mesh->alphatest = 255;
                        } else if (tmp > 0 && tmp <= 1) {
                            xml->mesh->alphatest = float_to_int(tmp * 255.0f);
                        } else {
                            xml->mesh->alphatest = 0;
                        }
                        break;
                    }
                    case MeshXML::TECHNIQUE:
                        xml->technique = iter->value;
                        break;
                    case MeshXML::DETAILTEXTURE:
                        if (use_detail_texture) {
                            detailTexture =
                                    TempGetTexture(xml, iter->value, FactionUtil::GetFaction(xml->faction), GFXTRUE);
                        }
                        break;
                    case MeshXML::TEXTURE:
                        //NO BREAK..goes to next statement
                    case MeshXML::ALPHAMAP:
                    case MeshXML::ANIMATEDTEXTURE:
                    case MeshXML::UNKNOWN: {
                        MeshXML::Names whichtype = MeshXML::UNKNOWN;
                        int strsize = 0;
                        if (strtoupper(iter->name).find("ANIMATION") == 0) {
                            whichtype = MeshXML::ANIMATEDTEXTURE;
                            strsize = strlen("ANIMATION");
                        }
                        if (strtoupper(iter->name).find("TEXTURE") == 0) {
                            whichtype = MeshXML::TEXTURE;
                            strsize = strlen("TEXTURE");
                        }
                        if (strtoupper(iter->name).find("ALPHAMAP") == 0) {
                            whichtype = MeshXML::ALPHAMAP;
                            strsize = strlen("ALPHAMAP");
                        }
                        if (whichtype != MeshXML::UNKNOWN) {
                            unsigned int texindex = 0;
                            string ind(iter->name.substr(strsize));
                            if (!ind.empty()) {
                                texindex = XMLSupport::parse_int(ind);
                            }
                            static bool per_pixel_lighting =
                                    XMLSupport::parse_bool(vs_config->getVariable("graphics",
                                            "per_pixel_lighting",
                                            "true"));
                            if ((texindex == 0) || per_pixel_lighting) {
                                while (xml->decals.size() <= texindex) {
                                    xml->decals.push_back(MeshXML::ZeTexture());
                                }
                            }
                            switch (whichtype) {
                                case MeshXML::ANIMATEDTEXTURE:
                                    xml->decals[texindex].animated_name = iter->value;
                                    break;
                                case MeshXML::ALPHAMAP:
                                    xml->decals[texindex].alpha_name = iter->value;
                                    break;
                                default:
                                    xml->decals[texindex].decal_name = iter->value;
                            }
                            if (texindex == 0) {
                                texture_found = true;
                            }
                        }
                        break;
                    }
                }
            }
            assert(texture_found);
            break;
        case MeshXML::POINTS:
            assert(top == MeshXML::MESH); //FIXME top was never initialized if state stack was empty
            //assert(xml->load_stage == 1);
            xml->load_stage = 2;
            break;
        case MeshXML::POINT:
            assert(top == MeshXML::POINTS); //FIXME top was never initialized if state stack was empty
            xml->vertex.s = 0.0;
            xml->vertex.t = 0.0;
            xml->vertex.i = 0.0;
            xml->vertex.j = 0.0;
            xml->vertex.k = 0.0;
            xml->vertex.x = 0.0;
            xml->vertex.y = 0.0;
            xml->vertex.z = 0.0;
            xml->vertex.tx = 0.0;
            xml->vertex.ty = 0.0;
            xml->vertex.tz = 0.0;
            xml->vertex.tw = 0.0;
            xml->point_state = 0;         //Point state is used to check that all necessary attributes are recorded
            break;
        case MeshXML::LOCATION:
            assert(top == MeshXML::POINT); //FIXME top was never initialized if state stack was empty
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        VS_LOG(error,
                                (boost::format("Unknown attribute '%1%' encountered in Location tag") % (*iter).name));
                        break;
                    case MeshXML::X:
                        assert(!(xml->point_state & MeshXML::P_X));
                        xml->vertex.x = XMLSupport::parse_float((*iter).value);
                        xml->vertex.i = 0;
                        xml->point_state |= MeshXML::P_X;
                        break;
                    case MeshXML::Y:
                        assert(!(xml->point_state & MeshXML::P_Y));
                        xml->vertex.y = XMLSupport::parse_float((*iter).value);
                        xml->vertex.j = 0;
                        xml->point_state |= MeshXML::P_Y;
                        break;
                    case MeshXML::Z:
                        assert(!(xml->point_state & MeshXML::P_Z));
                        xml->vertex.z = XMLSupport::parse_float((*iter).value);
                        xml->vertex.k = 0;
                        xml->point_state |= MeshXML::P_Z;
                        break;
                    case MeshXML::S:
                        xml->vertex.s = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::T:
                        xml->vertex.t = XMLSupport::parse_float((*iter).value);
                        break;
                    default:
                        assert(0);
                }
            }
            assert((xml->point_state & (MeshXML::P_X
                    | MeshXML::P_Y
                    | MeshXML::P_Z))
                    == (MeshXML::P_X
                            | MeshXML::P_Y
                            | MeshXML::P_Z));
            break;
        case MeshXML::NORMAL:
            assert(top == MeshXML::POINT);
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        VS_LOG(error,
                                (boost::format("Unknown attribute '%1%' encountered in Normal tag") % (*iter).name));
                        break;
                    case MeshXML::I:
                        assert(!(xml->point_state & MeshXML::P_I));
                        xml->vertex.i = XMLSupport::parse_float((*iter).value);
                        xml->point_state |= MeshXML::P_I;
                        break;
                    case MeshXML::J:
                        assert(!(xml->point_state & MeshXML::P_J));
                        xml->vertex.j = XMLSupport::parse_float((*iter).value);
                        xml->point_state |= MeshXML::P_J;
                        break;
                    case MeshXML::K:
                        assert(!(xml->point_state & MeshXML::P_K));
                        xml->vertex.k = XMLSupport::parse_float((*iter).value);
                        xml->point_state |= MeshXML::P_K;
                        break;
                    default:
                        assert(0);
                }
            }
            if ((xml->point_state & (MeshXML::P_I
                    | MeshXML::P_J
                    | MeshXML::P_K))
                    != (MeshXML::P_I
                            | MeshXML::P_J
                            | MeshXML::P_K)) {
                if (!xml->recalc_norm) {
                    xml->vertex.i = xml->vertex.j = xml->vertex.k = 0;
                    xml->recalc_norm = true;
                }
            }
            break;
        case MeshXML::POLYGONS:
            assert(top == MeshXML::MESH);
            //assert(xml->load_stage==3);
            xml->load_stage = 4;
            break;
        case MeshXML::LINE:
            assert(top == MeshXML::POLYGONS);
            //assert(xml->load_stage==4);
            xml->num_vertices = 2;
            xml->active_list = &xml->lines;
            xml->active_ind = &xml->lineind;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            VS_LOG(error, "Cannot Flatshade Lines");
                        } else if ((*iter).value == "Smooth") {
                            //ignored -- already done
                        }
                        break;
                    default:
                        assert(0);
                }
            }
            break;
        case MeshXML::TRI:
            assert(top == MeshXML::POLYGONS);
            //assert(xml->load_stage==4);
            xml->num_vertices = 3;
            xml->active_list = &xml->tris;
            xml->active_ind = &xml->triind;
            xml->trishade.push_back(0);
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            xml->trishade[xml->trishade.size() - 1] = 1;
                        } else if ((*iter).value == "Smooth") {
                            xml->trishade[xml->trishade.size() - 1] = 0;
                        }
                        break;
                    default:
                        assert(0);
                }
            }
            break;
        case MeshXML::LINESTRIP:
            assert(top == MeshXML::POLYGONS);
            //assert(xml->load_stage==4);
            xml->num_vertices = 2;
            xml->linestrips.push_back(SequenceContainer<GFXVertex>());
            xml->active_list = &(xml->linestrips[xml->linestrips.size() - 1]);
            xml->lstrcnt = xml->linestripind.size();
            xml->active_ind = &xml->linestripind;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            VS_LOG(error, "Cannot Flatshade Linestrips");
                        } else if ((*iter).value == "Smooth") {
                            //ignored -- already done
                        }
                        break;
                    default:
                        assert(0);
                }
            }
            break;

        case MeshXML::TRISTRIP:
            assert(top == MeshXML::POLYGONS);
            //assert(xml->load_stage==4);
            xml->num_vertices = 3;         //minimum number vertices
            xml->tristrips.push_back(SequenceContainer<GFXVertex>());
            xml->active_list = &(xml->tristrips[xml->tristrips.size() - 1]);
            xml->tstrcnt = xml->tristripind.size();
            xml->active_ind = &xml->tristripind;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            VS_LOG(error, "Cannot Flatshade Tristrips");
                        } else if ((*iter).value == "Smooth") {
                            //ignored -- already done
                        }
                        break;
                    default:
                        assert(0);
                }
            }
            break;

        case MeshXML::TRIFAN:
            assert(top == MeshXML::POLYGONS);
            //assert(xml->load_stage==4);
            xml->num_vertices = 3;         //minimum number vertices
            xml->trifans.push_back(SequenceContainer<GFXVertex>());
            xml->active_list = &(xml->trifans[xml->trifans.size() - 1]);
            xml->tfancnt = xml->trifanind.size();
            xml->active_ind = &xml->trifanind;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            VS_LOG(error, "Cannot Flatshade Trifans");
                        } else if ((*iter).value == "Smooth") {
                            //ignored -- already done
                        }
                        break;
                    default:
                        assert(0);
                }
            }
            break;

        case MeshXML::QUADSTRIP:
            assert(top == MeshXML::POLYGONS);
            //assert(xml->load_stage==4);
            xml->num_vertices = 4;         //minimum number vertices
            xml->quadstrips.push_back(SequenceContainer<GFXVertex>());
            xml->active_list = &(xml->quadstrips[xml->quadstrips.size() - 1]);
            xml->qstrcnt = xml->quadstripind.size();
            xml->active_ind = &xml->quadstripind;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            VS_LOG(error, "Cannot Flatshade Quadstrips");
                        } else if ((*iter).value == "Smooth") {
                            //ignored -- already done
                        }
                        break;
                    default:
                        assert(0);
                }
            }
            break;

        case MeshXML::QUAD:
            assert(top == MeshXML::POLYGONS);
            //assert(xml->load_stage==4);
            xml->num_vertices = 4;
            xml->active_list = &xml->quads;
            xml->active_ind = &xml->quadind;
            xml->quadshade.push_back(0);
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FLATSHADE:
                        if ((*iter).value == "Flat") {
                            xml->quadshade[xml->quadshade.size() - 1] = 1;
                        } else if ((*iter).value == "Smooth") {
                            xml->quadshade[xml->quadshade.size() - 1] = 0;
                        }
                        break;
                    default:
                        assert(0);
                }
            }
            break;
        case MeshXML::LOD:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::FRAMESPERSECOND:
                        framespersecond = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::LODFILE:
                        xml->lod
                                .push_back(new Mesh((*iter).value.c_str(),
                                        xml->lodscale,
                                        xml->faction,
                                        xml->fg,
                                        true));                   //make orig mesh
                        break;
                    case MeshXML::SIZE:
                        flotsize = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            if (xml->lodsize.size() != xml->lod.size()) {
                xml->lodsize.push_back(flotsize);
            }
            break;
        case MeshXML::VERTEX: {
            assert(
                    top == MeshXML::TRI || top == MeshXML::QUAD || top == MeshXML::LINE || top == MeshXML::TRISTRIP
                            || top
                                    == MeshXML::TRIFAN || top == MeshXML::QUADSTRIP || top == MeshXML::LINESTRIP);
            //assert(xml->load_stage==4);

            xml->vertex_state = 0;
            unsigned int index =
                    0; //FIXME not all switch cases initialize index --"=0" added temporarily by chuck_starchaser
            float s, t; //FIXME not all switch cases initialize s and t ...
            s = t = 0.0f; //This initialization line to "=0.0f" added temporarily by chuck_starchaser
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::POINT:
                        assert(!(xml->vertex_state & MeshXML::V_POINT));
                        xml->vertex_state |= MeshXML::V_POINT;
                        index = XMLSupport::parse_int((*iter).value);
                        break;
                    case MeshXML::S:
                        assert(!(xml->vertex_state & MeshXML::V_S));
                        xml->vertex_state |= MeshXML::V_S;
                        s = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::T:
                        assert(!(xml->vertex_state & MeshXML::V_T));
                        xml->vertex_state |= MeshXML::V_T;
                        t = XMLSupport::parse_float((*iter).value);
                        break;
                    default:
                        assert(0);
                }
            }
            assert((xml->vertex_state & (MeshXML::V_POINT
                    | MeshXML::V_S
                    | MeshXML::V_T))
                    == (MeshXML::V_POINT
                            | MeshXML::V_S
                            | MeshXML::V_T));
            assert(index < xml->vertices.size()); //FIXME not all switch cases initialize index
            xml->vertex.s = 0.0;
            xml->vertex.t = 0.0;
            xml->vertex.i = 0.0;
            xml->vertex.j = 0.0;
            xml->vertex.k = 0.0;
            xml->vertex.x = 0.0;
            xml->vertex.y = 0.0;
            xml->vertex.z = 0.0;
            xml->vertex.tx = 0.0;
            xml->vertex.ty = 0.0;
            xml->vertex.tz = 0.0;
            xml->vertex.tw = 0.0;
            xml->vertex = xml->vertices[index]; //FIXME not all switch cases initialize index
            xml->vertexcount[index] += 1; //FIXME not all switch cases initialize index
            if ((!xml->vertex.i) && (!xml->vertex.j) && (!xml->vertex.k)) {
                if (!xml->recalc_norm) {
                    xml->recalc_norm = true;
                }
            }
            //xml->vertex.x*=scale;
            //xml->vertex.y*=scale;
            //xml->vertex.z*=scale;//FIXME
            xml->vertex.s = s; //FIXME not all cases in switch above initialized s and t
            xml->vertex.t = t; //FIXME not all cases in switch above initialized s and t
            xml->active_list->push_back(xml->vertex);
            xml->active_ind->push_back(index);
            if (xml->reverse) {
                unsigned int i;
                for (i = xml->active_ind->size() - 1; i > 0; i--) {
                    (*xml->active_ind)[i] = (*xml->active_ind)[i - 1];
                }
                (*xml->active_ind)[0] = index;
                for (i = xml->active_list->size() - 1; i > 0; i--) {
                    (*xml->active_list)[i] = (*xml->active_list)[i - 1];
                }
                (*xml->active_list)[0] = xml->vertex;
            }
            xml->num_vertices--;
            break;
        }
        case MeshXML::LOGO: {
            assert(top == MeshXML::MESH);
            //assert (xml->load_stage==4);
            xml->load_stage = 5;
            xml->vertex_state = 0;
            unsigned int typ; //FIXME Not all cases below initialize typ
            float rot, siz, offset; //FIXME Not all cases below initialize rot, siz or offset
            typ = 0; //FIXME this line temporarily added by chuck_starchaser
            rot = siz = offset = 0.0f; //FIXME this line temporarily added by chuck_starchaser
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::TYPE:
                        assert(!(xml->vertex_state & MeshXML::V_TYPE));
                        xml->vertex_state |= MeshXML::V_TYPE;
                        typ = XMLSupport::parse_int((*iter).value);

                        break;
                    case MeshXML::ROTATE:
                        assert(!(xml->vertex_state & MeshXML::V_ROTATE));
                        xml->vertex_state |= MeshXML::V_ROTATE;
                        rot = XMLSupport::parse_float((*iter).value);

                        break;
                    case MeshXML::SIZE:
                        assert(!(xml->vertex_state & MeshXML::V_SIZE));
                        xml->vertex_state |= MeshXML::V_SIZE;
                        siz = XMLSupport::parse_float((*iter).value);
                        break;
                    case MeshXML::OFFSET:
                        assert(!(xml->vertex_state & MeshXML::V_OFFSET));
                        xml->vertex_state |= MeshXML::V_OFFSET;
                        offset = XMLSupport::parse_float((*iter).value);
                        break;
                    default:
                        assert(0);
                }
            }
            assert((xml->vertex_state & (MeshXML::V_TYPE
                    | MeshXML::V_ROTATE
                    | MeshXML::V_SIZE
                    | MeshXML::V_OFFSET))
                    == (MeshXML::V_TYPE
                            | MeshXML::V_ROTATE
                            | MeshXML::V_SIZE
                            | MeshXML::V_OFFSET));
            xml->logos.push_back(MeshXML::ZeLogo());
            xml->logos[xml->logos.size() - 1].type = typ;
            xml->logos[xml->logos.size() - 1].rotate = rot;
            xml->logos[xml->logos.size() - 1].size = siz;
            xml->logos[xml->logos.size() - 1].offset = offset;
            break;
        }
        case MeshXML::REF: {
            assert(top == MeshXML::LOGO);
            //assert (xml->load_stage==5);
            xml->load_stage = 6;
            unsigned int ind = 0;
            float indweight = 1;
            bool foundindex = false;
            int ttttttt;
            ttttttt = 0;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (MeshXML::attribute_map.lookup((*iter).name)) {
                    case MeshXML::UNKNOWN:
                        break;
                    case MeshXML::POINT:
                        assert(ttttttt < 2);
                        xml->vertex_state |= MeshXML::V_POINT;
                        ind = XMLSupport::parse_int((*iter).value);
                        foundindex = true;
                        ttttttt += 2;
                        break;
                    case MeshXML::WEIGHT:
                        assert((ttttttt & 1) == 0);
                        ttttttt += 1;
                        xml->vertex_state |= MeshXML::V_S;
                        indweight = XMLSupport::parse_float((*iter).value);
                        break;
                    default:
                        assert(0);
                }
            }
            assert(ttttttt == 3);
            if (!foundindex) {
                VS_LOG(error, "mesh with uninitalized logo");
            }
            xml->logos[xml->logos.size() - 1].refpnt.push_back(ind);
            xml->logos[xml->logos.size() - 1].refweight.push_back(indweight);
            xml->vertex_state += MeshXML::V_REF;
            break;
        }
        default:
            assert(0);
    }
}

void Mesh::endElement(MeshXML *xml, const string &name) {
    MeshXML::Names elem = (MeshXML::Names) MeshXML::element_map.lookup(name);
    assert(*xml->state_stack.rbegin() == elem);
    xml->state_stack.pop_back();
    unsigned int i;
    switch (elem) {
        case MeshXML::UNKNOWN:
            VS_LOG(error, (boost::format("Unknown element end tag '%1%' detected") % name));
            break;
        case MeshXML::POINT:
            assert((xml->point_state & (MeshXML::P_X
                    | MeshXML::P_Y
                    | MeshXML::P_Z
                    | MeshXML::P_I
                    | MeshXML::P_J
                    | MeshXML::P_K))
                    == (MeshXML::P_X
                            | MeshXML::P_Y
                            | MeshXML::P_Z
                            | MeshXML::P_I
                            | MeshXML::P_J
                            | MeshXML::P_K));
            xml->vertices.push_back(xml->vertex);
            xml->vertexcount.push_back(0);
            break;
        case MeshXML::POINTS:
            xml->load_stage = 3;
            break;
        case MeshXML::LINE:
            assert(xml->num_vertices == 0);
            break;
        case MeshXML::TRI:
            assert(xml->num_vertices == 0);
            break;
        case MeshXML::QUAD:
            assert(xml->num_vertices == 0);
            break;
        case MeshXML::LINESTRIP:
            assert(xml->num_vertices <= 0);
            for (i = xml->lstrcnt + 1; i < xml->linestripind.size(); i++) {
                xml->nrmllinstrip.push_back(xml->linestripind[i - 1]);
                xml->nrmllinstrip.push_back(xml->linestripind[i]);
            }
            break;
        case MeshXML::TRISTRIP:
            assert(xml->num_vertices <= 0);
            for (i = xml->tstrcnt + 2; i < xml->tristripind.size(); i++) {
                if ((i - xml->tstrcnt) % 2) {
                    //normal order
                    xml->nrmltristrip.push_back(xml->tristripind[i - 2]);
                    xml->nrmltristrip.push_back(xml->tristripind[i - 1]);
                    xml->nrmltristrip.push_back(xml->tristripind[i]);
                } else {
                    //reverse order
                    xml->nrmltristrip.push_back(xml->tristripind[i - 1]);
                    xml->nrmltristrip.push_back(xml->tristripind[i - 2]);
                    xml->nrmltristrip.push_back(xml->tristripind[i]);
                }
            }
            break;
        case MeshXML::TRIFAN:
            assert(xml->num_vertices <= 0);
            for (i = xml->tfancnt + 2; i < xml->trifanind.size(); i++) {
                xml->nrmltrifan.push_back(xml->trifanind[xml->tfancnt]);
                xml->nrmltrifan.push_back(xml->trifanind[i - 1]);
                xml->nrmltrifan.push_back(xml->trifanind[i]);
            }
            break;
        case MeshXML::QUADSTRIP:     //have to fix up nrmlquadstrip so that it 'looks' like a quad list for smooth shading
            assert(xml->num_vertices <= 0);
            for (i = xml->qstrcnt + 3; i < xml->quadstripind.size(); i += 2) {
                xml->nrmlquadstrip.push_back(xml->quadstripind[i - 3]);
                xml->nrmlquadstrip.push_back(xml->quadstripind[i - 2]);
                xml->nrmlquadstrip.push_back(xml->quadstripind[i]);
                xml->nrmlquadstrip.push_back(xml->quadstripind[i - 1]);
            }
            break;
        case MeshXML::POLYGONS:
            assert(xml->tris.size() % 3 == 0);
            assert(xml->quads.size() % 4 == 0);
            break;
        case MeshXML::REF:
            //assert (xml->load_stage==6);
            xml->load_stage = 5;
            break;
        case MeshXML::LOGO:
            //assert (xml->load_stage==5);
            assert(xml->vertex_state >= MeshXML::V_REF * 3);         //make sure there are at least 3 reference points
            xml->load_stage = 4;
            break;
        case MeshXML::MATERIAL:
            //assert(xml->load_stage==7);
            xml->load_stage = 4;
            break;
        case MeshXML::DETAILPLANE:

            break;
        case MeshXML::DIFFUSE:
            //assert(xml->load_stage==8);
            xml->load_stage = 7;
            break;
        case MeshXML::EMISSIVE:
            //assert(xml->load_stage==8);
            xml->load_stage = 7;
            break;
        case MeshXML::SPECULAR:
            //assert(xml->load_stage==8);
            xml->load_stage = 7;
            break;
        case MeshXML::AMBIENT:
            //assert(xml->load_stage==8);
            xml->load_stage = 7;
            break;
        case MeshXML::MESH:
            //assert(xml->load_stage==4);//4 is done with poly, 5 is done with Logos
            xml->load_stage = 5;
            break;
        default:
            break;
    }
}

void updateMax(Vector &mn, Vector &mx, const GFXVertex &ver) {
    mn.i = min(ver.x, mn.i);
    mx.i = max(ver.x, mx.i);
    mn.j = min(ver.y, mn.j);
    mx.j = max(ver.y, mx.j);
    mn.k = min(ver.z, mn.k);
    mx.k = max(ver.z, mx.k);
}

using namespace VSFileSystem;

void LaunchConverter(const char *input, const char *output, const char *args = "obc") {
    string intmp = string("\"") + input + string("\"");
    string outtmp = string("\"") + output + string("\"");
#ifndef _WIN32
    int pid = fork();
    if (!pid) {
        string soundserver_path = VSFileSystem::datadir + "/bin/mesher";
        string firstarg = string("\"") + soundserver_path + string("\"");
        pid = execlp(soundserver_path.c_str(), soundserver_path.c_str(), input, output, args, NULL);
        soundserver_path = VSFileSystem::datadir + "/mesher";
        firstarg = string("\"") + soundserver_path + string("\"");
        pid = execlp(soundserver_path.c_str(), soundserver_path.c_str(), input, output, args, NULL);
        VS_LOG_AND_FLUSH(fatal, "Unable to spawn converter");
        VSExit(-1);
    } else {
        if (pid == -1) {
            VS_LOG_AND_FLUSH(fatal, "Unable to spawn converter");
            VSExit(-1);
        }
        int mystat = 0;
        waitpid(pid, &mystat, 0);
    }
#else
    string ss_path  = VSFileSystem::datadir+"\\bin\\mesher.exe";
    string firstarg = string( "\"" )+ss_path+string( "\"" );
    int    pid = spawnl( P_WAIT, ss_path.c_str(), firstarg.c_str(), intmp.c_str(), outtmp.c_str(), args, NULL );
    if (pid == -1) {
        ss_path  = VSFileSystem::datadir+"\\mesher.exe";
        firstarg = string( "\"" )+ss_path+string( "\"" );
        int pid = spawnl( P_WAIT, ss_path.c_str(), firstarg.c_str(), intmp.c_str(), outtmp.c_str(), args, NULL );
        if (pid == -1) {
            VS_LOG(error, (boost::format("Unable to spawn obj converter Error (%1%)") % pid));
        }
    }
#endif
}

bool isBFXM(VSFile &f) {
    char bfxm[4];
    f.Read(&bfxm[0], 1);
    f.Read(&bfxm[1], 1);
    f.Read(&bfxm[2], 1);
    f.Read(&bfxm[3], 1);
    f.GoTo(0);
    return bfxm[0] == 'B' && bfxm[1] == 'F' && bfxm[2] == 'X' && bfxm[3] == 'M';
}

void CopyFile(VSFile &src, VSFile &dst) {
    size_t hm;
    size_t srcstruct;
    size_t *srcptr = &srcstruct;
    while ((hm = src.Read(srcptr, sizeof(srcstruct)))) {
        dst.Write(srcptr, hm);
    }
}

/*

bool loadObj( VSFile &f, std::string str )
{
    string fullpath = f.GetFullPath();
    VSFile output;
    output.OpenCreateWrite( "output.bfxm", BSPFile );
    output.Close();
    output.OpenReadOnly( "output.bfxm", BSPFile );
    string outputpath = output.GetFullPath();
    output.Close();
    LaunchConverter( fullpath.c_str(), outputpath.c_str() );
    output.OpenReadOnly( "output.bfxm", BSPFile );
    if ( isBFXM( output ) ) {
        output.Close();
        f.Close();
        f.OpenReadOnly( "output.bfxm", BSPFile );
        return true;
    } else {
        output.Close();
    }
    VSFile input;
    input.OpenCreateWrite( "input.obj", BSPFile );
    f.GoTo( 0 );
    CopyFile( f, input );
    input.Close();
    input.OpenReadOnly( "input.obj", BSPFile );
    string inputpath = input.GetFullPath();
    input.Close();

    f.Close();
    int    where = str.find_last_of( "." );
    str = str.substr( 0, where )+".mtl";
    f.OpenReadOnly( str, MeshFile );
    VSFile inputmtl;
    inputmtl.OpenCreateWrite( "input.mtl", BSPFile );
    CopyFile( f, inputmtl );
    f.Close();
    inputmtl.Close();
    LaunchConverter( inputpath.c_str(), outputpath.c_str() );

    output.OpenReadOnly( "output.bfxm", BSPFile );
    if ( isBFXM( output ) ) {
        output.Close();
        f.OpenReadOnly( "output.bfxm", BSPFile );
        return true;
    } else {
        output.Close();
    }
    return false;
}
*/
const bool USE_RECALC_NORM = true;
const bool FLAT_SHADE = true;

SharedPtr<Mesh> Mesh::LoadMesh(const char *filename,
        const Vector &scale,
        int faction,
        Flightgroup *fg,
        const SequenceContainer<std::string> &overridetextures) {
    SequenceContainer<SharedPtr<Mesh>> m = LoadMeshes(filename, scale, faction, fg, overridetextures);
    if (m.empty()) {
        return nullptr;
    }
    if (m.size() > 1) {
        VS_LOG(warning, (boost::format("Mesh %1% has %2% subcomponents. Only first used!") % filename % m.size()));
        for (unsigned int i = 1; i < m.size(); ++i) {
            delete m[i];
        }
    }
    return m[0];
}

/**
 * This is a Meyers singleton that returns a shared_ptr to a hash table whose keys are strings, and whose values are
 * shared_ptrs to deques of shared_ptrs to Mesh. Confused yet? Heh
 *
 * @return a shared_ptr to a hash table whose keys are strings, and whose values are
 * shared_ptrs to deques of shared_ptrs to Mesh
 */
SharedPtr<SharedPtrHashtable<std::string, SequenceContainer<SharedPtr<Mesh>>, MESH_HASTHABLE_SIZE>> bfxmHashTable() {
    static const SharedPtr<SharedPtrHashtable<std::string, SequenceContainer<SharedPtr<Mesh>>, MESH_HASTHABLE_SIZE>> kBfxmHashTable
            = MakeShared<SharedPtrHashtable<std::string, SequenceContainer<SharedPtr<Mesh>>, MESH_HASTHABLE_SIZE>>();
    return kBfxmHashTable;
}

SequenceContainer<SharedPtr<Mesh>> Mesh::LoadMeshes(const char *filename,
        const Vector &scale,
        int faction,
        Flightgroup *fg,
        const SequenceContainer<std::string> &overrideTextures) {
    /*
     *  if (strstr(filename,".xmesh")) {
     *  SharedPtr<Mesh> m = new Mesh (filename,scale,faction,fg);
     *  vector <SharedPtr<Mesh> > ret;
     *  ret.push_back(m);
     *  return ret;
     *  }*/
    string hash_name = VSFileSystem::GetHashName(filename, scale, faction);
    SharedPtr<SequenceContainer<SharedPtr<Mesh>>> oldmesh = bfxmHashTable()->Get(hash_name);
    if (oldmesh == 0) {
        hash_name = VSFileSystem::GetSharedMeshHashName(filename, scale, faction);
        oldmesh = bfxmHashTable()->Get(hash_name);
    }
    if (0 != oldmesh) {
        SequenceContainer<SharedPtr<Mesh> > ret;
        for (unsigned int i = 0; i < oldmesh->size(); ++i) {
            ret.push_back(new Mesh());
            SharedPtr<Mesh> const m = (*oldmesh)[i];
            ret.back()->LoadExistant(m->orig ? m->orig : m);
        }
        return ret;
    }
    VSFile f;
    VSError err = f.OpenReadOnly(filename, MeshFile);
    if (err > Ok) {
        VS_LOG(error, (boost::format("Cannot Open Mesh File %1%") % filename));
        return SequenceContainer<SharedPtr<Mesh> >();
    }
    char bfxm[4];
    f.Read(&bfxm[0], sizeof(bfxm[0]) * 4);
    bool isbfxm = (bfxm[0] == 'B' && bfxm[1] == 'F' && bfxm[2] == 'X' && bfxm[3] == 'M');
    if (isbfxm || strstr(filename, ".obj")) {
        if (!isbfxm) {
            // NOTE : Commented out following block, probably not needed anymore
/*            if ( !loadObj( f, filename ) ) {
                VS_LOG(error, (boost::format("Cannot Open Mesh File %1%") % filename));
*/
//cleanexit=1;
//winsys_exit(1);
            //    return vector< SharedPtr<Mesh> > ();
            // }
        }
        f.GoTo(0);
        hash_name =
                (err == VSFileSystem::Shared) ? VSFileSystem::GetSharedMeshHashName(filename, scale,
                        faction)
                        : VSFileSystem::GetHashName(
                        filename,
                        scale,
                        faction);
        SequenceContainer<SharedPtr<Mesh> > retval(LoadMeshes(f, scale, faction, fg, hash_name, overrideTextures));
        SequenceContainer<SharedPtr<Mesh> > *newvec = new SequenceContainer<SharedPtr<Mesh> >(retval);
        for (unsigned int i = 0; i < retval.size(); ++i) {
            retval[i]->hash_name = hash_name;
            if (retval[i]->orig) {
                retval[i]->orig->hash_name = hash_name;
            }
            (*newvec)[i] = retval[i]->orig ? retval[i]->orig : retval[i];
        }
        bfxmHashTable()->Put(hash_name, newvec);
        return retval;
    } else {
        f.Close();
        bool original = false;
        SharedPtr<Mesh> m = new Mesh(filename, scale, faction, fg, original);
        SequenceContainer<SharedPtr<Mesh> > ret;
        ret.push_back(m);
        return ret;
    }
}

void Mesh::LoadXML(const char *filename,
        const Vector &scale,
        int faction,
        Flightgroup *fg,
        bool origthis,
        const SequenceContainer<string> &textureOverride) {
    VSFile f;
    VSError err = f.OpenReadOnly(filename, MeshFile);
    if (err > Ok) {
        VS_LOG(error, (boost::format("Cannot Open Mesh File %1%") % filename));
//cleanexit=1;
//winsys_exit(1);
        return;
    }
    LoadXML(f, scale, faction, fg, origthis, textureOverride);
    f.Close();
}

void Mesh::LoadXML(VSFileSystem::VSFile &f,
        const Vector &scale,
        int faction,
        Flightgroup *fg,
        bool origthis,
        const SequenceContainer<string> &textureOverride) {
    SequenceContainer<unsigned int> ind;
    MeshXML *xml = new MeshXML;
    xml->mesh = this;
    xml->fg = fg;
    xml->usenormals = false;
    xml->usetangents = false;
    xml->force_texture = false;
    xml->reverse = false;
    xml->sharevert = false;
    xml->faction = faction;
    GFXGetMaterial(0, xml->material);     //by default it's the default material;
    xml->load_stage = 0;
    xml->recalc_norm = false;
    xml->scale = scale;
    xml->lodscale = scale;
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, xml);
    XML_SetElementHandler(parser, &Mesh::beginElement, &Mesh::endElement);
    XML_Parse(parser, (f.ReadFull()).c_str(), f.Size(), 1);
    XML_ParserFree(parser);
    //Now, copy everything into the mesh data structures
    if (xml->load_stage != 5) {
        VS_LOG_AND_FLUSH(fatal, "Warning: mesh load possibly failed");
        VSExit(-1);
    }
    PostProcessLoading(xml, textureOverride);
    numlods = xml->lod.size() + 1;
    if (origthis) {
        orig = NULL;
    } else {
        orig = new Mesh[numlods];
        unsigned int i;
        for (i = 0; i < xml->lod.size(); i++) {
            orig[i + 1] = *xml->lod[i];
            orig[i + 1].lodsize = xml->lodsize[i];
        }
    }
    delete xml;
}

static void SumNormal(SequenceContainer<GFXVertex> &vertices, int i1, int i2, int i3, SequenceContainer<float> &weights) {
    Vector v1(vertices[i2].x - vertices[i1].x,
            vertices[i2].y - vertices[i1].y,
            vertices[i2].z - vertices[i1].z);
    Vector v2(vertices[i3].x - vertices[i1].x,
            vertices[i3].y - vertices[i1].y,
            vertices[i3].z - vertices[i1].z);
    v1.Normalize();
    v2.Normalize();
    Vector N = v1.Cross(v2);
    float w = 1.f - 0.9 * fabsf(v1.Dot(v2));
    N *= w;

    vertices[i1].i += N.i;
    vertices[i1].j += N.j;
    vertices[i1].k += N.k;
    weights[i1] += w;

    vertices[i2].i += N.i;
    vertices[i2].j += N.j;
    vertices[i2].k += N.k;
    weights[i2] += w;

    vertices[i3].i += N.i;
    vertices[i3].j += N.j;
    vertices[i3].k += N.k;
    weights[i3] += w;
}

static void SumLineNormal(SequenceContainer<GFXVertex> &vertices, int i1, int i2, SequenceContainer<float> &weights) {
    Vector v1(vertices[i2].x - vertices[i1].x,
            vertices[i2].y - vertices[i1].y,
            vertices[i2].z - vertices[i1].z);
    static Vector v2(0.3408, 0.9401, 0.0005);
    v1.Normalize();
    Vector N = v1.Cross(v2);

    vertices[i1].i += N.i;
    vertices[i1].j += N.j;
    vertices[i1].k += N.k;
    weights[i1] += 1;

    vertices[i2].i += N.i;
    vertices[i2].j += N.j;
    vertices[i2].k += N.k;
    weights[i2] += 1;
}

static void SumNormals(SequenceContainer<GFXVertex> &vertices,
        SequenceContainer<int> &indices,
        size_t begin,
        size_t end,
        POLYTYPE polytype,
        SequenceContainer<float> &weights) {
    int flip = 0;
    size_t i;
    switch (polytype) {
        case GFXTRI:
            if (end <= 2) {
                break;
            }
            end -= 2;
            for (; begin < end; begin += 3) {
                SumNormal(vertices, indices[begin], indices[begin + 1], indices[begin + 2], weights);
            }
            break;
        case GFXQUAD:
            if (end <= 3) {
                break;
            }
            end -= 3;
            for (; begin < end; begin += 4) {
                SumNormal(vertices, indices[begin], indices[begin + 1], indices[begin + 2], weights);
                SumNormal(vertices, indices[begin], indices[begin + 2], indices[begin + 3], weights);
            }
            break;
        case GFXTRISTRIP:
        case GFXQUADSTRIP:
            if (end <= 2) {
                break;
            }
            end -= 2;
            for (; begin < end; ++begin, flip ^= 1) {
                SumNormal(vertices, indices[begin], indices[begin + 1 + flip], indices[begin + 2 - flip], weights);
            }
            break;
        case GFXTRIFAN:
            if (end <= 2) {
                break;
            }
            end -= 2;
            for (i = begin; begin < end; ++begin) {
                SumNormal(vertices, indices[i], indices[begin + 1], indices[begin + 2], weights);
            }
            break;
        case GFXLINESTRIP:
            if (end <= 1) {
                break;
            }
            end -= 1;
            for (i = begin; begin < end; ++begin) {
                SumLineNormal(vertices, indices[begin], indices[begin + 1], weights);
            }
            break;
        case GFXLINE:
            if (end <= 1) {
                break;
            }
            end -= 1;
            for (i = begin; begin < end; begin += 2) {
                SumLineNormal(vertices, indices[begin], indices[begin + 1], weights);
            }
            break;
        case GFXPOLY:
        case GFXPOINT:
            break;
    }
}

static void ClearTangents(SequenceContainer<GFXVertex> &vertices) {
    for (SequenceContainer<GFXVertex>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
        it->SetTangent(Vector(0, 0, 0), 0);
    }
}

static float faceTSPolarity(const Vector &T, const Vector &B, const Vector &N) {
    if (T.Cross(B).Dot(N) >= 0.f) {
        return -1.f;
    } else {
        return 1.f;
    }
}

static float faceTSWeight(SequenceContainer<GFXVertex> &vertices, int i1, int i2, int i3) {
    const GFXVertex &vtx1 = vertices[i1];
    const GFXVertex &vtx2 = vertices[i2];
    const GFXVertex &vtx3 = vertices[i3];

    Vector v1(vtx2.x - vtx1.x,
            vtx2.y - vtx1.y,
            vtx2.z - vtx1.z);
    Vector v2(vtx3.x - vtx1.x,
            vtx3.y - vtx1.y,
            vtx3.z - vtx1.z);
    v1.Normalize();
    v2.Normalize();
    return 1.f - fabsf(v1.Dot(v2));
}

static void computeTangentspace(SequenceContainer<GFXVertex> &vertices, int i1, int i2, int i3, Vector &T, Vector &B, Vector &N) {
    const GFXVertex &v1 = vertices[i1];
    const GFXVertex &v2 = vertices[i2];
    const GFXVertex &v3 = vertices[i3];

    //compute deltas. I think that the fact we use (*2-*1) and (*3-*1) is arbitrary, but I could be wrong
    Vector p0(v1.x, v1.y, v1.z);
    Vector p1(v2.x, v2.y, v2.z);
    Vector p2(v3.x, v3.y, v3.z);
    p1 -= p0;
    p2 -= p0;

    float s1, t1;
    float s2, t2;
    s1 = v2.s - v1.s;
    s2 = v3.s - v1.s;
    t1 = v2.t - v1.t;
    t2 = v3.t - v1.t;

    //and now a myracle happens...
    T = t2 * p1 - t1 * p2;
    B = s1 * p2 - s2 * p1;
    N = p1.Cross(p2);

    //normalize
    T.Normalize();
    B.Normalize();
    N.Normalize();
}

static void SumTangent(SequenceContainer<GFXVertex> &vertices, int i1, int i2, int i3, SequenceContainer<float> &weights) {
    float w = faceTSWeight(vertices, i1, i2, i3);
    Vector T, B, N;
    computeTangentspace(vertices, i1, i2, i3, T, B, N);

    float p = faceTSPolarity(T, B, N) * w;
    T *= w;

    GFXVertex &v1 = vertices[i1];
    GFXVertex &v2 = vertices[i2];
    GFXVertex &v3 = vertices[i3];

    v1.tx += T.x;
    v1.ty += T.y;
    v1.tz += T.z;
    v1.tw += p;
    weights[i1] += w;

    v2.tx += T.x;
    v2.ty += T.y;
    v2.tz += T.z;
    v2.tw += p;
    weights[i2] += w;

    v3.tx += T.x;
    v3.ty += T.y;
    v3.tz += T.z;
    v3.tw += p;
    weights[i3] += w;
}

static void SumTangents(SequenceContainer<GFXVertex> &vertices,
        SequenceContainer<int> &indices,
        size_t begin,
        size_t end,
        POLYTYPE polytype,
        SequenceContainer<float> &weights) {
    int flip = 0;
    size_t i;
    switch (polytype) {
        case GFXTRI:
            if (end <= 2) {
                break;
            }
            end -= 2;
            for (; begin < end; begin += 3) {
                SumTangent(vertices, indices[begin], indices[begin + 1], indices[begin + 2], weights);
            }
            break;
        case GFXQUAD:
            if (end <= 3) {
                break;
            }
            end -= 3;
            for (; begin < end; begin += 4) {
                SumTangent(vertices, indices[begin], indices[begin + 1], indices[begin + 2], weights);
                SumTangent(vertices, indices[begin], indices[begin + 2], indices[begin + 3], weights);
            }
            break;
        case GFXTRISTRIP:
        case GFXQUADSTRIP:
            if (end <= 2) {
                break;
            }
            end -= 2;
            for (; begin < end; ++begin, flip ^= 1) {
                SumTangent(vertices, indices[begin], indices[begin + 1 + flip], indices[begin + 2 - flip], weights);
            }
            break;
        case GFXTRIFAN:
            if (end <= 2) {
                break;
            }
            end -= 2;
            for (i = begin; begin < end; ++begin) {
                SumTangent(vertices, indices[i], indices[begin + 1], indices[begin + 2], weights);
            }
            break;
        case GFXLINE:
        case GFXLINESTRIP:
        case GFXPOLY:
        case GFXPOINT:
            break;
    }
}

static void NormalizeTangents(SequenceContainer<GFXVertex> &vertices, SequenceContainer<float> &weights) {
    for (size_t i = 0, n = vertices.size(); i < n; ++i) {
        GFXVertex &v = vertices[i];
        float w = weights[i];

        if (w > 0) {
            //Average (shader will normalize)
            float iw = (w < 0.001) ? 1.f : (1.f / w);
            v.tx *= iw;
            v.ty *= iw;
            v.tz *= iw;
            v.tw *= iw;
        }

        // Don't let null vectors around (they create NaNs within shaders when normalized)
        // Since they happen regularly on sphere polar caps, replace them with a suitable value there (+x)
        if (Vector(v.tx, v.ty, v.tz).MagnitudeSquared() < 0.00001) {
            v.tx = 0.001;
        }
    }
}

static void NormalizeNormals(SequenceContainer<GFXVertex> &vertices, SequenceContainer<float> &weights) {
    for (size_t i = 0, n = vertices.size(); i < n; ++i) {
        GFXVertex &v = vertices[i];
        float w = weights[i];

        if (w > 0) {
            //Renormalize
            float mag = v.GetNormal().MagnitudeSquared();
            if (mag < 0.00001) {
                mag = 1.f;
            } else {
                mag = 1.f / sqrt(mag);
            }
            v.i *= mag;
            v.j *= mag;
            v.k *= mag;
        }
    }
}

void Mesh::PostProcessLoading(SharedPtr<MeshXML> xml, const SequenceContainer<string> &overrideTexture) {
    unsigned int i;
    unsigned int a = 0;
    unsigned int j;
    //begin vertex normal calculations if necessary
    if (!xml->usenormals) {
        ClearTangents(xml->vertices);

        SequenceContainer<float> weights;
        weights.resize(xml->vertices.size(), 0.f);

        size_t i, j, n;

        SumNormals(xml->vertices, xml->triind, 0, xml->triind.size(), GFXTRI, weights);
        SumNormals(xml->vertices, xml->quadind, 0, xml->quadind.size(), GFXQUAD, weights);
        SumNormals(xml->vertices, xml->lineind, 0, xml->lineind.size(), GFXLINE, weights);
        for (i = j = 0, n = xml->tristrips.size(); i < n; j += xml->tristrips[i++].size()) {
            SumNormals(xml->vertices, xml->tristripind, j, j + xml->tristrips[i].size(), GFXTRISTRIP, weights);
        }
        for (i = j = 0, n = xml->quadstrips.size(); i < n; j += xml->quadstrips[i++].size()) {
            SumNormals(xml->vertices, xml->quadstripind, j, j + xml->quadstrips[i].size(), GFXQUADSTRIP, weights);
        }
        for (i = j = 0, n = xml->trifans.size(); i < n; j += xml->trifans[i++].size()) {
            SumNormals(xml->vertices, xml->trifanind, j, j + xml->trifans[i].size(), GFXTRIFAN, weights);
        }
        for (i = j = 0, n = xml->linestrips.size(); i < n; j += xml->linestrips[i++].size()) {
            SumNormals(xml->vertices, xml->linestripind, j, j + xml->linestrips[i].size(), GFXLINESTRIP, weights);
        }
        NormalizeNormals(xml->vertices, weights);
    } else {
        //Flip normals - someone thought VS should flips normals, ask him why.
        for (i = 0; i < xml->vertices.size(); ++i) {
            GFXVertex &v = xml->vertices[i];
            v.i *= -1;
            v.j *= -1;
            v.k *= -1;
        }
    }
    a = 0;
    SequenceContainer<unsigned int> ind;
    for (a = 0; a < xml->tris.size(); a += 3) {
        for (j = 0; j < 3; j++) {
            int ix = xml->triind[a + j];
            ind.push_back(ix);
            xml->tris[a + j].SetNormal(xml->vertices[ix].GetNormal());
            xml->tris[a + j].SetTangent(xml->vertices[ix].GetTangent(),
                    xml->vertices[ix].GetTangentParity());
        }
    }
    a = 0;
    for (a = 0; a < xml->quads.size(); a += 4) {
        for (j = 0; j < 4; j++) {
            int ix = xml->quadind[a + j];
            ind.push_back(ix);
            xml->quads[a + j].SetNormal(xml->vertices[ix].GetNormal());
            xml->quads[a + j].SetTangent(xml->vertices[ix].GetTangent(),
                    xml->vertices[ix].GetTangentParity());
        }
    }
    a = 0;
    for (a = 0; a < xml->lines.size(); a += 2) {
        for (j = 0; j < 2; j++) {
            int ix = xml->lineind[a + j];
            ind.push_back(ix);
            xml->lines[a + j].SetNormal(xml->vertices[ix].GetNormal());
        }
    }
    a = 0;
    unsigned int k = 0;
    unsigned int l = 0;
    for (l = a = 0; a < xml->tristrips.size(); a++) {
        for (k = 0; k < xml->tristrips[a].size(); k++, l++) {
            int ix = xml->tristripind[l];
            ind.push_back(ix);
            xml->tristrips[a][k].SetNormal(xml->vertices[ix].GetNormal());
            xml->tristrips[a][k].SetTangent(xml->vertices[ix].GetTangent(),
                    xml->vertices[ix].GetTangentParity());
        }
    }
    for (l = a = 0; a < xml->trifans.size(); a++) {
        for (k = 0; k < xml->trifans[a].size(); k++, l++) {
            int ix = xml->trifanind[l];
            ind.push_back(ix);
            xml->trifans[a][k].SetNormal(xml->vertices[ix].GetNormal());
            xml->trifans[a][k].SetTangent(xml->vertices[ix].GetTangent(),
                    xml->vertices[ix].GetTangentParity());
        }
    }
    for (l = a = 0; a < xml->quadstrips.size(); a++) {
        for (k = 0; k < xml->quadstrips[a].size(); k++, l++) {
            int ix = xml->quadstripind[l];
            ind.push_back(ix);
            xml->quadstrips[a][k].SetNormal(xml->vertices[ix].GetNormal());
            xml->quadstrips[a][k].SetTangent(xml->vertices[ix].GetTangent(),
                    xml->vertices[ix].GetTangentParity());
        }
    }
    for (l = a = 0; a < xml->linestrips.size(); a++) {
        for (k = 0; k < xml->linestrips[a].size(); k++, l++) {
            int ix = xml->linestripind[l];
            ind.push_back(ix);
            xml->linestrips[a][k].SetNormal(xml->vertices[ix].GetNormal());
        }
    }
    //TODO: add alpha handling

    //check for per-polygon flat shading
    unsigned int trimax = xml->tris.size() / 3;
    a = 0;
    i = 0;
    j = 0;
    if (!xml->usenormals) {
        for (i = 0; i < trimax; i++, a += 3) {
            if (FLAT_SHADE || xml->trishade[i] == 1) {
                for (j = 0; j < 3; j++) {
                    Vector Cur = xml->vertices[xml->triind[a + j]].GetPosition();
                    Cur = (xml->vertices[xml->triind[a + ((j + 1) % 3)]].GetPosition() - Cur)
                            .Cross(xml->vertices[xml->triind[a + ((j + 2) % 3)]].GetPosition() - Cur);
                    Normalize(Cur);
                    xml->tris[a + j].SetNormal(Cur);
                }
            }
        }
        a = 0;
        trimax = xml->quads.size() / 4;
        for (i = 0; i < trimax; i++, a += 4) {
            if (xml->quadshade[i] == 1 || (FLAT_SHADE)) {
                for (j = 0; j < 4; j++) {
                    Vector Cur = xml->vertices[xml->quadind[a + j]].GetPosition();
                    Cur = (xml->vertices[xml->quadind[a + ((j + 1) % 4)]].GetPosition() - Cur)
                            .Cross(xml->vertices[xml->quadind[a + ((j + 2) % 4)]].GetPosition() - Cur);
                    Normalize(Cur);
                    xml->quads[a + j].SetNormal(Cur);
                }
            }
        }
    }
    string factionname = FactionUtil::GetFaction(xml->faction);
    for (unsigned int LC = 0; LC < overrideTexture.size(); ++LC) {
        if (overrideTexture[LC] != "") {
            while (xml->decals.size() <= LC) {
                MeshXML::ZeTexture z;
                xml->decals.push_back(z);
            }
            if (overrideTexture[LC].find(".ani") != string::npos) {
                xml->decals[LC].decal_name = "";
                xml->decals[LC].animated_name = overrideTexture[LC];
                xml->decals[LC].alpha_name = "";
            } else {
                xml->decals[LC].animated_name = "";
                xml->decals[LC].alpha_name = "";
                xml->decals[LC].decal_name = overrideTexture[LC];
            }
        }
    }
    while (Decal.size() < xml->decals.size()) {
        Decal.push_back(NULL);
    }
    {
        for (unsigned int i = 0; i < xml->decals.size(); i++) {
            Decal[i] = (TempGetTexture(xml, i, factionname));
        }
    }
    while (Decal.back() == NULL && Decal.size() > 1) {
        Decal.pop_back();
    }
    initTechnique(xml->technique);

    unsigned int index = 0;

    unsigned int totalvertexsize = xml->tris.size() + xml->quads.size() + xml->lines.size();
    for (index = 0; index < xml->tristrips.size(); index++) {
        totalvertexsize += xml->tristrips[index].size();
    }
    for (index = 0; index < xml->trifans.size(); index++) {
        totalvertexsize += xml->trifans[index].size();
    }
    for (index = 0; index < xml->quadstrips.size(); index++) {
        totalvertexsize += xml->quadstrips[index].size();
    }
    for (index = 0; index < xml->linestrips.size(); index++) {
        totalvertexsize += xml->linestrips[index].size();
    }
    index = 0;
    SequenceContainer<GFXVertex> vertexlist(totalvertexsize);

    mn = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
    mx = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    radialSize = 0;
    SequenceContainer<enum POLYTYPE> polytypes;
    polytypes.insert(polytypes.begin(), totalvertexsize, GFXTRI);
    //enum POLYTYPE * polytypes= new enum POLYTYPE[totalvertexsize];//overkill but what the hell
    SequenceContainer<int> poly_offsets;
    poly_offsets.insert(poly_offsets.begin(), totalvertexsize, 0);
    int o_index = 0;
    if (xml->tris.size()) {
        polytypes[o_index] = GFXTRI;
        poly_offsets[o_index] = xml->tris.size();
        o_index++;
    }
    if (xml->quads.size()) {
        polytypes[o_index] = GFXQUAD;
        poly_offsets[o_index] = xml->quads.size();
        o_index++;
    }
    if (xml->lines.size()) {
        polytypes[o_index] = GFXLINE;
        poly_offsets[o_index] = xml->lines.size();
        o_index++;
    }
    for (a = 0; a < xml->tris.size(); a++, index++) {
        vertexlist[index] = xml->tris[a];
    }
    for (a = 0; a < xml->quads.size(); a++, index++) {
        vertexlist[index] = xml->quads[a];
    }
    for (a = 0; a < xml->lines.size(); a++, index++) {
        vertexlist[index] = xml->lines[a];
    }
    for (a = 0; a < xml->tristrips.size(); a++) {
        for (unsigned int m = 0; m < xml->tristrips[a].size(); m++, index++) {
            vertexlist[index] = xml->tristrips[a][m];
        }
        polytypes[o_index] = GFXTRISTRIP;
        poly_offsets[o_index] = xml->tristrips[a].size();
        o_index++;
    }
    for (a = 0; a < xml->trifans.size(); a++) {
        for (unsigned int m = 0; m < xml->trifans[a].size(); m++, index++) {
            vertexlist[index] = xml->trifans[a][m];
        }
        polytypes[o_index] = GFXTRIFAN;
        poly_offsets[o_index] = xml->trifans[a].size();
        o_index++;
    }
    for (a = 0; a < xml->quadstrips.size(); a++) {
        for (unsigned int m = 0; m < xml->quadstrips[a].size(); m++, index++) {
            vertexlist[index] = xml->quadstrips[a][m];
        }
        polytypes[o_index] = GFXQUADSTRIP;
        poly_offsets[o_index] = xml->quadstrips[a].size();
        o_index++;
    }
    for (a = 0; a < xml->linestrips.size(); a++) {
        for (unsigned int m = 0; m < xml->linestrips[a].size(); m++, index++) {
            vertexlist[index] = xml->linestrips[a][m];
        }
        polytypes[o_index] = GFXLINESTRIP;
        poly_offsets[o_index] = xml->linestrips[a].size();
        o_index++;
    }
    for (i = 0; i < index; ++i) {
        updateMax(mn, mx, vertexlist[i]);
    }
    //begin tangent calculations if necessary
    if (!xml->usetangents) {
        ClearTangents(vertexlist);

        SequenceContainer<float> weights;
        SequenceContainer<int> indices(vertexlist.size());         //Oops, someday we'll use real indices
        weights.resize(vertexlist.size(), 0.f);

        size_t i, j, n;
        for (i = 0, n = vertexlist.size(); i < n; ++i) {
            indices[i] = i;
        }
        for (i = j = 0, n = polytypes.size(); i < n; j += poly_offsets[i++]) {
            SumTangents(vertexlist, indices, j, j + poly_offsets[i], polytypes[i], weights);
        }
        NormalizeTangents(vertexlist, weights);
    }
    if (mn.i == FLT_MAX && mn.j == FLT_MAX && mn.k == FLT_MAX) {
        mx.i = mx.j = mx.k = mn.i = mn.j = mn.k = 0;
    }
    mn.i *= xml->scale.i;
    mn.j *= xml->scale.j;
    mn.k *= xml->scale.k;
    mx.i *= xml->scale.i;
    mx.j *= xml->scale.j;
    mx.k *= xml->scale.k;
    float x_center = (mn.i + mx.i) / 2.0,
            y_center = (mn.j + mx.j) / 2.0,
            z_center = (mn.k + mx.k) / 2.0;
    local_pos = Vector(x_center, y_center, z_center);
    for (a = 0; a < totalvertexsize; a++) {
        vertexlist[a].x *= xml->scale.i;         //FIXME
        vertexlist[a].y *= xml->scale.j;
        vertexlist[a].z *= xml->scale.k;
    }
    for (a = 0; a < xml->vertices.size(); a++) {
        xml->vertices[a].x *= xml->scale.i;         //FIXME
        xml->vertices[a].y *= xml->scale.j;
        xml->vertices[a].z *= xml->scale.k;
        xml->vertices[a].i *= -1;
        xml->vertices[a].k *= -1;
        xml->vertices[a].j *= -1;
    }
    if (o_index || index) {
        radialSize = .5 * (mx - mn).Magnitude();
    }
    if (xml->sharevert) {
        vlist = new GFXVertexList(
                (polytypes.size() ? &polytypes[0] : 0),
                xml->vertices.size(),
                (xml->vertices.size() ? &xml->vertices[0] : 0), o_index,
                (poly_offsets.size() ? &poly_offsets[0] : 0), false,
                (ind.size() ? &ind[0] : 0));
    } else {
        static bool usopttmp =
                (XMLSupport::parse_bool(vs_config->getVariable("graphics", "OptimizeVertexArrays", "false")));
        static float optvertexlimit =
                (XMLSupport::parse_float(vs_config->getVariable("graphics", "OptimizeVertexCondition", "1.0")));
        bool cachunk = false;
        if (usopttmp && (vertexlist.size() > 0)) {
            int numopt = totalvertexsize;
            GFXVertex *newv;
            unsigned int *ind;
            GFXOptimizeList(&vertexlist[0], totalvertexsize, &newv, &numopt, &ind);
            if (numopt < totalvertexsize * optvertexlimit) {
                vlist = new GFXVertexList(
                        (polytypes.size() ? &polytypes[0] : 0),
                        numopt, newv, o_index,
                        (poly_offsets.size() ? &poly_offsets[0] : 0), false,
                        ind);
                cachunk = true;
            }
            free(ind);
            free(newv);
        }
        if (!cachunk) {
            if (vertexlist.size() == 0) {
                vertexlist.resize(1);
            }
            vlist = new GFXVertexList(
                    (polytypes.size() ? &polytypes[0] : 0),
                    totalvertexsize, &vertexlist[0], o_index,
                    (poly_offsets.size() ? &poly_offsets[0] : 0));
        }
    }
    CreateLogos(xml, xml->faction, xml->fg);
    //Calculate bounding sphere
    if (mn.i == FLT_MAX) {
        mn = Vector(0, 0, 0);
        mx = Vector(0, 0, 0);
    }
    GFXSetMaterial(myMatNum, xml->material);
}

