/*
 * quadtree_xml.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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


#include "gfx/quadtree.h"
#include "root_generic/xml_support.h"
#include "src/gfxlib.h"
#include "gfx/ani_texture.h"
#include <assert.h>
#include "vegadisk/vsfilesystem.h"
#include "gfx/vsimage.h"

using namespace VSFileSystem;

extern enum BLENDFUNC parse_alpha(const char *tmp);

struct TerrainData {
    int scale;
    float OriginX;
    float OriginY;
    std::string file;
    std::string terrainfile;
};

struct TerraXML {
    float scales;
    float scalet;
    float radius;
    float detail;
    float SphereSizeX;
    float SphereSizeZ;
    std::vector<std::string> animation;

    std::vector<std::string> alpha;
    std::vector<GFXMaterial> mat;
    std::vector<TerrainData> data;
};

void QuadTree::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
    ((QuadTree *) userData)->beginElement(name, XMLSupport::AttributeList(atts));
}

void QuadTree::endElement(void *userData, const XML_Char *name) {
    ((QuadTree *) userData)->endElement(name);
}

using XMLSupport::EnumMap;

namespace TerrainXML {
//

enum Names {
    UNKNOWN,
    TEXTURE,
    TERRAIN,
    MATERIAL,
    DATA,
    LEVEL,
    BLEND,
    FFILE,
    ALPHAFILE,
    TERRAINFILE,
    DETAIL,
    STATICDETAIL,
    REFLECT,
    COLOR,
    SCALE,
    ORIGINX,
    SCALES,
    ORIGINY,
    SCALET,
    SPHERESIZEX,
    SPHERESIZEZ,
    TERRAINAMBIENT,
    TERRAINDIFFUSE,
    TERRAINSPECULAR,
    TERRAINEMISSIVE,
    RED,
    GREEN,
    BLUE,
    ALPHA,
    POWER,
    RADIUS,
    ANIMATIONFILE
};

const EnumMap::Pair element_names[] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("Terrain", TERRAIN),
        EnumMap::Pair("Texture", TEXTURE),
        EnumMap::Pair("Material", MATERIAL),
        EnumMap::Pair("Data", DATA),
        EnumMap::Pair("Ambient", TERRAINAMBIENT),
        EnumMap::Pair("Diffuse", TERRAINDIFFUSE),
        EnumMap::Pair("Specular", TERRAINSPECULAR),
        EnumMap::Pair("Emissive", TERRAINEMISSIVE)
};

const EnumMap::Pair attribute_names[] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("Detail", DETAIL),
        EnumMap::Pair("StaticDetail", STATICDETAIL),
        EnumMap::Pair("Level", LEVEL),
        EnumMap::Pair("Blend", BLEND),
        EnumMap::Pair("File", FFILE),
        EnumMap::Pair("AlphaFile", ALPHAFILE),
        EnumMap::Pair("Animation", ANIMATIONFILE),
        EnumMap::Pair("TerrainFile", TERRAINFILE),
        EnumMap::Pair("Reflect", REFLECT),
        EnumMap::Pair("Color", COLOR),
        EnumMap::Pair("Scale", SCALE),
        EnumMap::Pair("ScaleS", SCALES),
        EnumMap::Pair("ScaleT", SCALET),
        EnumMap::Pair("OriginX", ORIGINX),
        EnumMap::Pair("OriginY", ORIGINY),
        EnumMap::Pair("red", RED),
        EnumMap::Pair("green", GREEN),
        EnumMap::Pair("blue", BLUE),
        EnumMap::Pair("alpha", ALPHA),
        EnumMap::Pair("power", POWER),
        EnumMap::Pair("radius", RADIUS),
        EnumMap::Pair("SphereWidth", SPHERESIZEX),
        EnumMap::Pair("SphereHeight", SPHERESIZEZ)
};

const EnumMap element_map(element_names, 9);
const EnumMap attribute_map(attribute_names, 24);

//end namespace
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
using XMLSupport::parse_float;
using namespace TerrainXML;

void QuadTree::beginElement(const std::string &name, const AttributeList &attributes) {
    Names elem = (Names) element_map.lookup(name);
    AttributeList::const_iterator iter;
    char csrc[128];
    char cdst[128];
    switch (elem) {
        case TERRAIN:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case DETAIL:
                        detail = parse_float((*iter).value);
                        break;
                    case STATICDETAIL:
                        xml->detail = parse_float((*iter).value);
                        break;
                    case LEVEL:
                        RootCornerData.Level = parse_int((*iter).value);
                        break;
                    case SCALES:
                        xml->scales = parse_float((*iter).value);
                        break;
                    case SCALET:
                        xml->scalet = parse_float((*iter).value);
                        break;
                    case RADIUS:
                        xml->radius = parse_float((*iter).value);
                        break;
                    case SPHERESIZEX:
                        xml->SphereSizeX = parse_float((*iter).value);
                        break;
                    case SPHERESIZEZ:
                        xml->SphereSizeZ = parse_float((*iter).value);
                        break;
                }
            }
            break;
        case TEXTURE:
            textures.push_back(TerrainTexture());
            textures.back().color = textures.size() - 1;
            xml->mat.push_back(GFXMaterial());
            xml->alpha.push_back(std::string());
            xml->animation.push_back(std::string());
            GFXGetMaterial(0, xml->mat.back());
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case FFILE:
                        textures.back().tex.filename = strdup((*iter).value.c_str());
                        break;
                    case ALPHAFILE:
                        xml->alpha.back() = (*iter).value.c_str();
                        break;
                    case ANIMATIONFILE:
                        xml->animation.back() = (*iter).value.c_str();
                        break;
                    case BLEND:
                        sscanf(((*iter).value).c_str(), "%s %s", csrc, cdst);
                        textures.back().blendSrc = parse_alpha(csrc);
                        textures.back().blendDst = parse_alpha(cdst);
                        break;
                    case COLOR:
                        textures.back().color = parse_int(((*iter).value));
                        break;
                }
            }
            break;
        case MATERIAL:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case REFLECT:
                        textures.back().reflect = parse_bool((*iter).value);
                        break;
                    case POWER:
                        xml->mat.back().power = parse_float((*iter).value);
                        break;
                }
            }
            break;
        case TERRAINDIFFUSE:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case RED:
                        xml->mat.back().dr = parse_float((*iter).value);
                        break;
                    case BLUE:
                        xml->mat.back().db = parse_float((*iter).value);
                        break;
                    case ALPHA:
                        xml->mat.back().da = parse_float((*iter).value);
                        break;
                    case GREEN:
                        xml->mat.back().dg = parse_float((*iter).value);
                        break;
                }
            }
            break;
        case TERRAINEMISSIVE:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case RED:
                        xml->mat.back().er = parse_float((*iter).value);
                        break;
                    case BLUE:
                        xml->mat.back().eb = parse_float((*iter).value);
                        break;
                    case ALPHA:
                        xml->mat.back().ea = parse_float((*iter).value);
                        break;
                    case GREEN:
                        xml->mat.back().eg = parse_float((*iter).value);
                        break;
                }
            }
            break;
        case TERRAINSPECULAR:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case RED:
                        xml->mat.back().sr = parse_float((*iter).value);
                        break;
                    case BLUE:
                        xml->mat.back().sb = parse_float((*iter).value);
                        break;
                    case ALPHA:
                        xml->mat.back().sa = parse_float((*iter).value);
                        break;
                    case GREEN:
                        xml->mat.back().sg = parse_float((*iter).value);
                        break;
                }
            }
            break;
        case TERRAINAMBIENT:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case RED:
                        xml->mat.back().ar = parse_float((*iter).value);
                        break;
                    case BLUE:
                        xml->mat.back().ab = parse_float((*iter).value);
                        break;
                    case ALPHA:
                        xml->mat.back().aa = parse_float((*iter).value);
                        break;
                    case GREEN:
                        xml->mat.back().ag = parse_float((*iter).value);
                        break;
                }
            }
            break;
        case DATA:
            xml->data.push_back(TerrainData());
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (attribute_map.lookup((*iter).name)) {
                    case SCALE:
                        xml->data.back().scale = parse_int((*iter).value);
                        break;
                    case ORIGINX:
                        xml->data.back().OriginX = parse_float((*iter).value);
                        break;
                    case ORIGINY:
                        xml->data.back().OriginY = parse_float((*iter).value);
                        break;
                    case FFILE:
                        xml->data.back().file = (*iter).value;
                        break;
                    case TERRAINFILE:
                        xml->data.back().terrainfile = (*iter).value;
                        break;
                }
            }
            break;
        case UNKNOWN:
        default:
            break;
    }
}

void QuadTree::endElement(const std::string &name) {
}

void QuadTree::SetXSizes(int mX, unsigned int maxX) {
    if (mX < minX) {
        minX = mX;
    }
    if (maxX > this->maxX) {
        this->maxX = maxX;
    }
}

void QuadTree::SetZSizes(int mZ, unsigned int maxZ) {
    if (mZ < minZ) {
        minZ = mZ;
    }
    if (maxZ > this->maxZ) {
        this->maxZ = maxZ;
    }
}

void QuadTree::LoadXML(const char *filename, const Vector &Scales, const float Radius) {
    std::vector<unsigned int> ind;
    //FILE* inFile = VSFileSystem::vs_open (filename, "r");
    VSFile f;
    VSError err = f.OpenReadOnly(filename, UnknownFile);
    if (err > Ok) {
        assert(0);
        return;
    }
    xml = new TerraXML;
    xml->SphereSizeX = -1;
    xml->SphereSizeZ = -1;
    xml->scales = .001;
    xml->scalet = .001;
    xml->radius = 10000;
    xml->detail = 20;
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, this);
    XML_SetElementHandler(parser, &QuadTree::beginElement, &QuadTree::endElement);
    XML_Parse(parser, (f.ReadFull()).c_str(), f.Size(), 1);
    f.Close();
    XML_ParserFree(parser);
    unsigned int i;
    for (i = 0; i < textures.size(); i++) {
        textures[i].scales = xml->scales;
        textures[i].scalet = xml->scalet;
        if (textures[i].tex.filename || xml->animation[i].length() > 0) {
            Texture *tex;
            if (xml->animation[i].length() > 0) {
                tex = new AnimatedTexture(xml->animation[i].c_str(), 0, MIPMAP);
            } else {
                if (xml->alpha[i].length() > 0) {
                    tex = new Texture(textures[i].tex.filename, xml->alpha[i].c_str());
                } else {
                    tex = new Texture(textures[i].tex.filename);
                }
                free(textures[i].tex.filename);
            }
            textures[i].tex.t = tex;
            GFXSetMaterial(textures[i].material, xml->mat[i]);
        } else {
            textures[i].tex.t = NULL;
        }
    }
    bool biggest = true;
    for (i = 0; i < xml->data.size(); i++) {
        HeightMapInfo hm;
        hm.XOrigin = (int) xml->data[i].OriginX;
        hm.ZOrigin = (int) xml->data[i].OriginY;
        hm.Scale = xml->data[i].scale;
        //FILE * fp;
        //fp = VSFileSystem::vs_open (xml->data[i].file.c_str(),"rb");
        Texture tex;
        err = f.OpenReadOnly(xml->data[i].file.c_str(), UnknownFile);
        if (err <= Ok) {
            //hm.Data = (short *) readImage (fp,bpp, format, hm.XSize,hm.ZSize, palette, &heightmapTransform,false);
            hm.Data = (short *) tex.ReadImage(&f, &heightmapTransform, false);
            hm.XSize = tex.sizeX;
            hm.ZSize = tex.sizeY;
            f.Close();
        }
        //LoadData();
        unsigned long xsize = 0;
        unsigned long zsize = 0;
        //fp = VSFileSystem::vs_open (xml->data[i].terrainfile.c_str(),"rb");
        f.OpenReadOnly(xml->data[i].terrainfile.c_str(), UnknownFile);
        if (err <= Ok) {
            //hm.terrainmap = (unsigned char *)readImage (fp, bpp, format, xsize, zsize, palette, &terrainTransform,true);
            hm.terrainmap = (unsigned char *) tex.ReadImage(&f, &terrainTransform, true);
            xsize = tex.sizeX;
            zsize = tex.sizeY;
            f.Close();
        }
        if (hm.Data && hm.terrainmap) {
            assert(xsize == hm.XSize && zsize == hm.ZSize);
            SetXSizes(hm.XOrigin, hm.XSize << hm.Scale);
            SetZSizes(hm.ZOrigin, hm.ZSize << hm.Scale);
            hm.RowWidth = hm.XSize;
            if (biggest) {
                biggest = false;
                if (Radius != 0 && xml->radius != 0) {
                    nonlinear_transform =
                            new SphericalTransform(xml->SphereSizeX < 0 ? hm.XSize << hm.Scale : xml->SphereSizeX,
                                    Radius < 0 ? xml->radius : Radius,
                                    xml->SphereSizeZ < 0 ? hm.ZSize << hm.Scale : xml->SphereSizeZ);
                } else {
                    nonlinear_transform = new IdentityTransform();
                    //only happens the first time!
                }
                GFXVertex *v = vertices.BeginMutate(0)->vertices;
                float xmax = (hm.XOrigin + (hm.XSize << hm.Scale));
                float zmax = (hm.ZOrigin + (hm.ZSize << hm.Scale));
                v[0].SetVertex(nonlinear_transform->Transform(Vector(xmax, 0, hm.ZOrigin)));
                v[0].SetTexCoord(nonlinear_transform->TransformS(xmax,
                                xml->scales),
                        nonlinear_transform->TransformT(hm.ZOrigin, xml->scalet));
                Vector Norm0(nonlinear_transform->TransformNormal(v[0].GetConstVertex(), Vector(0, 1, 0)));
                v[0].SetNormal(Vector(Norm0.i * Scales.i, Norm0.j * Scales.j, Norm0.k * Scales.k));
                v[1].SetVertex(nonlinear_transform->Transform(Vector(hm.XOrigin, 0, hm.ZOrigin)));
                v[1].SetTexCoord(nonlinear_transform->TransformS(hm.XOrigin,
                                xml->scales),
                        nonlinear_transform->TransformT(hm.ZOrigin, xml->scalet));
                Norm0 = (nonlinear_transform->TransformNormal(v[1].GetConstVertex(), Vector(0, 1, 0)));
                v[1].SetNormal(Vector(Norm0.i * Scales.i, Norm0.j * Scales.j, Norm0.k * Scales.k));

                v[2].SetVertex(nonlinear_transform->Transform(Vector(hm.XOrigin, 0, zmax)));
                v[2].SetTexCoord(nonlinear_transform->TransformS(hm.XOrigin,
                                xml->scales),
                        nonlinear_transform->TransformT(zmax, xml->scalet));
                Norm0 = (nonlinear_transform->TransformNormal(v[2].GetConstVertex(), Vector(0, 1, 0)));
                v[2].SetNormal(Vector(Norm0.i * Scales.i, Norm0.j * Scales.j, Norm0.k * Scales.k));

                v[3].SetVertex(nonlinear_transform->Transform(Vector(xmax, 0, zmax)));
                v[3].SetTexCoord(nonlinear_transform->TransformS(zmax,
                                xml->scales),
                        nonlinear_transform->TransformT(zmax, xml->scalet));
                Norm0 = (nonlinear_transform->TransformNormal(v[3].GetConstVertex(), Vector(0, 1, 0)));
                v[3].SetNormal(Vector(Norm0.i * Scales.i, Norm0.j * Scales.j, Norm0.k * Scales.k));
                vertices.EndMutate();
                quadsquare::SetCurrentTerrain(&VertexAllocated,
                        &VertexCount,
                        &vertices,
                        &unusedvertices,
                        nonlinear_transform,
                        &textures,
                        Vector(1.0F / Scales.i, 1.0F / Scales.j, 1.0F / Scales.k),
                        neighbors);
                root = new quadsquare(&RootCornerData);
            }
            root->AddHeightMap(RootCornerData, hm);
            free(hm.Data);
            free(hm.terrainmap);
        }
    }
    if (biggest) {
        quadsquare::SetCurrentTerrain(&VertexAllocated,
                &VertexCount,
                &vertices,
                &unusedvertices,
                nonlinear_transform,
                &textures,
                Vector(1.0F / Scales.i, 1.0F / Scales.j, 1.0F / Scales.k),
                neighbors);
        root = new quadsquare(&RootCornerData);
    }
    //root->StaticCullData (RootCornerData,xml->detail);
    delete xml;
}

