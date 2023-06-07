/**
 * quadtree.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 *  contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_GFX_QUADTREE_H
#define VEGA_STRIKE_ENGINE_GFX_QUADTREE_H

#include "quadsquare.h"
#include "xml_support.h"
#include "matrix.h"

class Texture;
struct TerraXML;

/**
 * This class is a wrapper class for quadsquare
 * It takes care of activating textures, having possession of the vlist
 * and having posession of the static vars that get set
 */
class QuadTree {
    int minX;
    int minZ;
    unsigned int maxX;
    unsigned int maxZ;
    void SetXSizes(int mX, unsigned int maxX);
    void SetZSizes(int mZ, unsigned int maxZ);
    Vector Scales;
    float detail;
    IdentityTransform *nonlinear_transform;
    quadcornerdata RootCornerData;
    quadsquare *root;
    quadsquare *neighbors[4];
    std::vector<TerrainTexture> textures;
    std::vector<unsigned int> unusedvertices;
    GFXVertexList vertices;
    unsigned int VertexAllocated;
    unsigned int VertexCount;
    void LoadData();
    void LoadXML(const char *filename, const Vector &scales, const float radius);
    TerraXML *xml;
    void SetNeighbors(quadsquare *east, quadsquare *north, quadsquare *west, quadsquare *south);
protected:
    Matrix transformation;
public:
    QuadTree(const char *filename, const Vector &scales, const float Radius);
    ~QuadTree();
    void Render();
    void SetNeighbors(QuadTree *east, QuadTree *north, QuadTree *west, QuadTree *south);
    void Update(unsigned short numstages, unsigned short whichstage, updateparity *updateorder = identityparity);
    void SetTransformation(const Matrix &transformation);
    float GetHeight(Vector Location,
            Vector &normal,
            const Matrix &transform,
            float TotalTerrainSizeX = 0,
            float TotalTerrainSizeZ = 0) const;
    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
    static void endElement(void *userData, const XML_Char *name);
    void beginElement(const std::string &name, const XMLSupport::AttributeList &attributes);
    void endElement(const std::string &name);

    float getminX() const {
        return 0;
    }

    float getminZ() const {
        return 0;
    }

    float getmaxX() const {
        return minX + (float) maxX;
    }

    float getmaxZ() const {
        return minZ + (float) maxZ;
    }

    float getSizeX() const {
        return maxX;
    }

    float getSizeZ() const {
        return maxZ;
    }

    void StaticCullData(const float detail);

    bool GetGroundPos(QVector &Location, Vector &norm, float TTSX = 0, float TTSZ = 0) const {
        return GetGroundPos(Location, norm, transformation, TTSX, TTSZ);
    }

    bool GetGroundPos(QVector &Location,
            Vector &norm,
            const Matrix &trans,
            float TotalTerrainSizeX = 0,
            float TotalTerrainSizeZ = 0) const;
    Vector GetNormal(const Vector &position, const Vector &requestednorm) const;
};

#endif //VEGA_STRIKE_ENGINE_GFX_QUADTREE_H
