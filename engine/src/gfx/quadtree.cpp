/*
 * quadtree.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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


#include "quadtree.h"
#include "matrix.h"
#include "aux_texture.h"
#include "universe.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "vs_logging.h"

const GFXVertex InitialVertices[4] = {
        GFXVertex(Vector(0, 0, 0), Vector(0, 1, 0), 0, 0),
        GFXVertex(Vector(0, 0, 0), Vector(0, 1, 0), 0, 0),
        GFXVertex(Vector(0, 0, 0), Vector(0, 1, 0), 0, 0),
        GFXVertex(Vector(0, 0, 0), Vector(0, 1, 0), 0, 0)
};

QuadTree::QuadTree(const char *filename, const Vector &Scales, const float radius)
        : minX(0), minZ(0), maxX(0), maxZ(0), Scales(Scales), vertices(GFXTRI, 4, InitialVertices, 4, true) {
    neighbors[0] = NULL;
    neighbors[1] = NULL;
    neighbors[2] = NULL;
    neighbors[3] = NULL;
    detail = 128;
    Identity(transformation);
    transformation.r[0] = Scales.i;
    transformation.r[4] = Scales.j;
    transformation.r[8] = Scales.k;
    //nonlinear_transform = new IdentityTransform;
    RootCornerData.Parent = NULL;
    RootCornerData.Square = NULL;
    RootCornerData.ChildIndex = 0;
    RootCornerData.Level = 15;
    RootCornerData.xorg = 0;
    RootCornerData.zorg = 0;
    RootCornerData.Verts[0].Y = 0;
    RootCornerData.Verts[0].vertindex = 0;
    RootCornerData.Verts[1].Y = 0;
    RootCornerData.Verts[1].vertindex = 1;
    RootCornerData.Verts[2].Y = 0;
    RootCornerData.Verts[2].vertindex = 2;
    RootCornerData.Verts[3].Y = 0;
    RootCornerData.Verts[3].vertindex = 3;
    RootCornerData.Verts[0].SetTex(0);
    RootCornerData.Verts[1].SetTex(0);
    RootCornerData.Verts[2].SetTex(0);
    RootCornerData.Verts[3].SetTex(0);
    VertexAllocated = VertexCount = 4;
    /*
     *  textures.push_back (TerrainTexture());
     *  for (int i=1;i<10;i++) {
     *  char name[]="terrainX.bmp";
     *  name[7] = '0'+i;
     *  TerrainTexture tmp;
     *  tmp.tex.t = new Texture (name);
     *  textures.push_back (tmp);
     *  }
     */
    if (filename) {
        LoadXML(filename, Scales, radius);
        //adData();
    } else {
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
    //root->StaticCullData (RootCornerData,25);
    /*
     *     for (i = 0; i < 10; i++) {
     *             root->Update(RootCornerData, (const float*) ViewerLoc, Detail);
     *     }
     */
}

void QuadTree::StaticCullData(const float detail) {
    root->StaticCullData(RootCornerData, detail);
}

void QuadTree::SetNeighbors(quadsquare *east, quadsquare *north, quadsquare *west, quadsquare *south) {
    neighbors[0] = east;
    neighbors[1] = north;
    neighbors[2] = west;
    neighbors[3] = south;
}

void QuadTree::SetNeighbors(QuadTree *east, QuadTree *north, QuadTree *west, QuadTree *south) {
    SetNeighbors(east->root, north->root, west->root, south->root);
}

QuadTree::~QuadTree() {
    delete root;
    delete nonlinear_transform;
}

bool QuadTree::GetGroundPos(QVector &Location,
        Vector &norm,
        const Matrix &transf,
        float TotalTerrainSizeX,
        float TotalTerrainSizeZ) const {
    QVector Loc = nonlinear_transform->InvTransform(InvScaleTransform(transf, Location));
    if (TotalTerrainSizeX) {
        float tmpx = Loc.i;
        Loc.i = fmod((double) Loc.i, (double) TotalTerrainSizeX);
        if (Loc.i < 0) {
            Loc.i += TotalTerrainSizeX;
        }
        TotalTerrainSizeX = tmpx;
        tmpx = Loc.k;
        Loc.k = fmod((double) Loc.k, (double) TotalTerrainSizeZ);
        if (Loc.k < 0) {
            Loc.k += TotalTerrainSizeZ;
        }
        TotalTerrainSizeZ = tmpx;
    } else {
        TotalTerrainSizeX = Loc.i;
        TotalTerrainSizeZ = Loc.k;
    }
    float tmp = root->GetHeight(RootCornerData, Loc.i, Loc.k, norm);
    if (tmp > -FLT_MAX) {
        Location =
                Transform(transf,
                        nonlinear_transform->Transform(QVector(TotalTerrainSizeX, tmp, TotalTerrainSizeZ))).Cast();
        norm = TransformNormal(transf, nonlinear_transform->TransformNormal(Location, norm));
        norm.Normalize();

        return true;
    }
    return false;
}

float QuadTree::GetHeight(Vector Location,
        Vector &normal,
        const Matrix &transf,
        float TotalTerrainSizeX,
        float TotalTerrainSizeZ) const {
    Location = nonlinear_transform->InvTransform(InvScaleTransform(transf, Location));
    if (TotalTerrainSizeX) {
        //float t1=Location.i;
        //float t2=Location.k;
        Location.i = fmod(Location.i, TotalTerrainSizeX);
        Location.k = fmod(Location.k, TotalTerrainSizeZ);
        if (Location.i < 0) {
            Location.i += TotalTerrainSizeX;
        }
        if (Location.k < 0) {
            Location.k += TotalTerrainSizeZ;
        }
    }
    float tmp = Location.j - root->GetHeight(RootCornerData, Location.i, Location.k, normal);
    normal = TransformNormal(transf, nonlinear_transform->TransformNormal(Location, normal));
    normal.Normalize();
    return tmp;
}

void QuadTree::Update(unsigned short numstages, unsigned short whichstage, updateparity *uo) {
    //GetViewerPosition
    quadsquare::SetCurrentTerrain(&VertexAllocated,
            &VertexCount,
            &vertices,
            &unusedvertices,
            nonlinear_transform,
            &textures,
            Vector(1.0F / Scales.i, 1.0F / Scales.j, 1.0F / Scales.k),
            neighbors);
    root->Update(RootCornerData,
            nonlinear_transform->InvTransform(InvScaleTransform(transformation,
                    _Universe->AccessCamera()->GetPosition())), detail,
            numstages, whichstage, uo);
}

inline Vector calculatenormscale(const Matrix &trans) {
    return Vector(1.0F / (Vector(trans.getP()).Magnitude()), 1.0F / (Vector(trans.getQ()).Magnitude()), 1.0F
            / (Vector(trans.getR()).Magnitude()));
}

void QuadTree::SetTransformation(const Matrix &mat) {
    transformation = mat;
    Scales = (calculatenormscale(mat));
}

void QuadTree::Render() {
    GFXLoadMatrixModel(transformation);
    GFXBoxInFrustumModel(transformation);
    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXEnable(LIGHTING);
    GFXBlendMode(ONE, ZERO);
    quadsquare::SetCurrentTerrain(&VertexAllocated,
            &VertexCount,
            &vertices,
            &unusedvertices,
            nonlinear_transform,
            &textures,
            Scales,
            neighbors);
    root->Render(RootCornerData,
            nonlinear_transform->InvTransform(InvScaleTransform(transformation,
                    _Universe->AccessCamera()->GetPosition())));
}

void QuadTree::LoadData() {
//Load some data and put it into the quadtree.
    nonlinear_transform = new IdentityTransform();
    HeightMapInfo hm;
    hm.XOrigin = 0;
    hm.ZOrigin = 0;
    hm.XSize = 512;
    hm.ZSize = 512;
    hm.RowWidth = hm.XSize;
    hm.Scale = 7;
    hm.Data = new short[hm.XSize * hm.ZSize];
    hm.terrainmap = new unsigned char[hm.XSize * hm.ZSize];
    memset(hm.terrainmap, 0, sizeof(unsigned char) * hm.ZSize * hm.XSize);
    VS_LOG(info, "Loading height grids...");

    //Big coarse data, at 128 meter sample spacing.
    FILE *fp = VSFileSystem::vs_open("demdata/gc16at128.raw", "rb");
    VSFileSystem::vs_read(hm.Data, sizeof(unsigned short), hm.XSize * hm.ZSize, fp);
    VSFileSystem::vs_close(fp);
    VS_LOG(info, "Building quadtree data...");
    root->AddHeightMap(RootCornerData, hm);

    //More detailed data at 64 meter spacing, covering the middle of the terrain.
    hm.XOrigin = 16384;
    hm.ZOrigin = 16384;
    hm.Scale = 6;
    fp = VSFileSystem::vs_open("demdata/gc16at64.raw", "rb");
    VSFileSystem::vs_read(hm.Data, sizeof(unsigned short), hm.XSize * hm.ZSize, fp);
    VSFileSystem::vs_close(fp);
    VS_LOG(info, "Adding quadtree data...");
    root->AddHeightMap(RootCornerData, hm);

    //Even more detailed data, at 32 meter spacing, covering a smaller area near the middle.
    hm.XOrigin = 24576;
    hm.ZOrigin = 24576;
    hm.Scale = 5;
    fp = VSFileSystem::vs_open("demdata/gc16at32.raw", "rb");
    VSFileSystem::vs_read(hm.Data, sizeof(unsigned short), hm.XSize * hm.ZSize, fp);
    VSFileSystem::vs_close(fp);
    VS_LOG(info, "Adding quadtree data...");
    root->AddHeightMap(RootCornerData, hm);

    delete[] hm.Data;
}

Vector QuadTree::GetNormal(const Vector &position, const Vector &norm) const {
    return TransformNormal(transformation, nonlinear_transform->TransformNormal(position, norm));
}

