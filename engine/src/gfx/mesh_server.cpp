/*
 * mesh_server.cpp
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


#include "gfx_generic/mesh.h"

#ifdef __cplusplus
extern "C"
{
void winsys_exit(int code) {
    exit(code);
}
}
#endif

int Mesh::getNumTextureFrames() const {
    return 1;
}

double Mesh::getTextureCumulativeTime() const {
    return 0;
}

float Mesh::getTextureFramesPerSecond() const {
    return 0;
}

void Mesh::setTextureCumulativeTime(double d) {
}

void Mesh::RestoreCullFace(int whichdrawqueue) {
}

void Mesh::SelectCullFace(int whichdrawqueue) {
}

void Mesh::CreateLogos(struct MeshXML *, int faction, Flightgroup *fg) {
}

Texture *Mesh::TempGetTexture(struct MeshXML *, std::string filename, std::string factionname, GFXBOOL detail) const {
    return NULL;
}

Texture *Mesh::TempGetTexture(struct MeshXML *, int index, std::string factionname) const {
    return NULL;
}

const GFXMaterial &Mesh::GetMaterial() const {
    static GFXMaterial tmp;
    return tmp;
}

Texture *createTexture(const char *filename,
        int stage = 0,
        enum FILTER f1 = MIPMAP,
        enum TEXTURE_TARGET t0 = TEXTURE2D,
        enum TEXTURE_IMAGE_TARGET t = TEXTURE_2D,
        unsigned char c = GFXFALSE,
        int i = 65536) {
    return NULL;
}

Texture *createTexture(char const *ccc,
        char const *cc,
        int k = 0,
        enum FILTER f1 = MIPMAP,
        enum TEXTURE_TARGET t0 = TEXTURE2D,
        enum TEXTURE_IMAGE_TARGET t = TEXTURE_2D,
        float f = 1,
        int j = 0,
        unsigned char c = GFXFALSE,
        int i = 65536) {
    return NULL;
}

AnimatedTexture *createAnimatedTexture(char const *c, int i, enum FILTER f) {
    return NULL;
}

Logo *createLogo(int numberlogos,
        Vector *center,
        Vector *normal,
        float *sizes,
        float *rotations,
        float offset,
        Texture *Dec,
        Vector *Ref) {
    return NULL;
}

extern Hashtable<std::string, std::vector<Mesh *>, MESH_HASTHABLE_SIZE> bfxmHashTable;

Mesh::~Mesh() {
    if (!orig || orig == this) {
        delete vlist;
        if (meshHashTable.Get(hash_name) == this) {
            meshHashTable.Delete(hash_name);
        }
        vector<Mesh *> *hashers = bfxmHashTable.Get(hash_name);
        vector<Mesh *>::iterator finder;
        if (hashers) {
            for (int i = hashers->size() - 1; i >= 0; --i) {
                if ((*hashers)[i] == this) {
                    hashers->erase(hashers->begin() + i);
                    if (hashers->empty()) {
                        bfxmHashTable.Delete(hash_name);
                        delete hashers;
                    }
                }
            }
        }
        orig->refcount--;
        //printf ("orig refcount: %d",refcount);
        if (orig->refcount == 0) {
            delete[] orig;
        }
    }
}

void Mesh::Draw(float lod,
        const Matrix &m,
        float toofar,
        int cloak,
        float nebdist,
        unsigned char damage,
        bool renormalize,
        const MeshFX *) //short fix
{
}

void Mesh::DrawNow(float lod, bool centered, const Matrix &m, int cloak, float nebdist) {
    //short fix
}

void Mesh::ProcessZFarMeshes(bool) {
}

void Mesh::ProcessUndrawnMeshes(bool pushSpecialEffects, bool) {
}

void Mesh::ProcessDrawQueue(size_t, int, bool, const QVector &) {
}

void Mesh::EnableSpecialFX() {
}

void Mesh::AddDamageFX(Vector const &, Vector const &, float, struct GFXColor const &) {
}

void Mesh::initTechnique(const std::string &) {
}

void AddWarp(Unit *un, QVector, float) {
}

void WarpTrailDraw() {
}
