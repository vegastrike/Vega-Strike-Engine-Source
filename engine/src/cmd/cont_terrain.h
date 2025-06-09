/*
 * cont_terrain.h
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
// NO HEADER GUARD

#include "collide2/Opcode.h"
#include "collide2/CSopcodecollider.h"
#include "cmd/terrain.h"

class StarSystem;
const int numcontterr = 4;
struct MeshDat {
    class Mesh *mesh;
    class csOPCODECollider *collider;
    Matrix mat;
};
class ContinuousTerrain {
public:
    Vector Scales;
    float sizeX;
    float sizeZ;
    int width;
    int numcontterr;
    Terrain **data;
    MeshDat *md;
    Matrix transformation;
    QVector *location;
    bool *dirty;
    bool checkInvScale(double &pos, double campos, float size);
public:
    ContinuousTerrain(const char *filenameUL, const Vector &Scales = Vector(0, 0, 0), const float mass = 0);
    void SetTransformation(const Matrix &transformation);
/**
 *  Centers terrain around camera
 */
    void AdjustTerrain(StarSystem *);

    void AdjustTerrain(Matrix &transform, const Matrix &transformation, const QVector &unitpos, int i);
    ~ContinuousTerrain();
    void DisableDraw();
    void EnableDraw();
    void DisableUpdate();
    void EnableUpdate();
    void Draw();
    QVector GetGroundPosIdentTrans(QVector ShipPos, Vector &norm);
    QVector GetGroundPos(QVector ShipPos, Vector &norm);
    Vector GetUpVector(const Vector &pos);
    void Collide(Unit *un, Matrix trans);
    void Collide(Unit *un);
    void Collide();

    void GetTotalSize(float &X, float &Z) {
        X = sizeX;
        Z = sizeZ;
    }
};

