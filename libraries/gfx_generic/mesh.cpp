/*
 * mesh.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020-2025 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <memory.h>
#include "gfx/animation.h"
#include "gfx/aux_logo.h"
#include "gfx_generic/mesh.h"
#include "gfx_generic/matrix.h"
#include "gfx/camera.h"
#include <assert.h>
#include <math.h>
#include "cmd/nebula.h"
#include <list>
#include <string>
#include <fstream>
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "root_generic/lin_time.h"
#include "src/gfxlib.h"
#include "root_generic/vs_globals.h"
#include "root_generic/configxml.h"
#include "src/hashtable.h"
#include "src/vegastrike.h"
#include "gfx_generic/sphere.h"
#include "root_generic/lin_time.h"
#include "gfx_generic/mesh_xml.h"
#include "gfx/technique.h"
#include <cfloat>
#include <algorithm>

#define LOD_HYSTHERESIS_DIVIDER (20)
#define LOD_HYSTHERESIS_MAXENLARGEMENT_FACTOR (1.1)

using std::list;
Hashtable<std::string, Mesh, 503>Mesh::meshHashTable;
Hashtable<std::string, std::vector<int>, 503>Mesh::animationSequences;
Vector mouseline;

int Mesh::getNumAnimationFrames(const string &which) const {
    if (which.empty()) {
        vector<int> *animSeq = animationSequences.Get(hash_name);
        if (animSeq) {
            return animSeq->size();
        }
    } else {
        vector<int> *animSeq = animationSequences.Get(hash_name + "*" + which);
        if (animSeq) {
            return animSeq->size();
        }
    }
    return 0;
}

void Mesh::InitUnit() {
    convex = false;
    polygon_offset = 0;
    framespersecond = 0;
    numlods = 1;
    alphatest = 0;
    lodsize = FLT_MAX;
    forcelogos = NULL;
    squadlogos = NULL;
    local_pos = Vector(0, 0, 0);
    blendSrc = ONE;
    blendDst = ZERO;
    vlist = NULL;
    mn = Vector(0, 0, 0);
    mx = Vector(0, 0, 0);
    radialSize = 0;
    if (Decal.empty()) {
        Decal.push_back(NULL);
    }

    //texturename[0] = -1;
    numforcelogo = numsquadlogo = 0;
    myMatNum = 0;     //default material!
    //scale = Vector(1.0,1.0,1.0);
    refcount = 1;     //FIXME VEGASTRIKE  THIS _WAS_ zero...NOW ONE
    orig = NULL;

    envMapAndLit = 0x3;
    setEnvMap(GFXTRUE);
    setLighting(GFXTRUE);
    detailTexture = NULL;
    draw_queue = NULL;
    will_be_drawn = GFXFALSE;
    draw_sequence = 0;

    initTechnique("fixed");
}

Mesh::Mesh() {
    InitUnit();
}

bool Mesh::LoadExistant(Mesh *oldmesh) {
    *this = *oldmesh;
    oldmesh->refcount++;
    orig = oldmesh;
    return true;
}

bool Mesh::LoadExistant(const string filehash, const Vector &scale, int faction) {
    Mesh *oldmesh;

    hash_name = VSFileSystem::GetHashName(filehash, scale, faction);
    oldmesh = meshHashTable.Get(hash_name);
    if (oldmesh == 0) {
        hash_name = VSFileSystem::GetSharedMeshHashName(filehash, scale, faction);
        oldmesh = meshHashTable.Get(hash_name);
    }
    if (0 != oldmesh) {
        return LoadExistant(oldmesh);
    }
    return false;
}

extern Hashtable<std::string, std::vector<Mesh *>, MESH_HASTHABLE_SIZE> bfxmHashTable;

Mesh::Mesh(const Mesh &m) {
    VS_LOG(warning, "UNTESTED MESH COPY CONSTRUCTOR");
    this->orig = NULL;
    this->hash_name = m.hash_name;
    InitUnit();
    Mesh *oldmesh = meshHashTable.Get(hash_name);
    if (0 == oldmesh) {
        vector<Mesh *> *vec = bfxmHashTable.Get(hash_name);
        for (unsigned int i = 0; i < vec->size(); ++i) {
            Mesh *mush = (*vec)[i]->orig ? (*vec)[i]->orig : (*vec)[i];
            if (mush == m.orig || mush == &m) {
                oldmesh = (*vec)[i];
            }
        }
        if (0 == oldmesh) {
            if (vec->size() > 1) {
                VS_LOG(warning,
                        (boost::format("Copy constructor %1$s used in ambiguous Situation") % hash_name.c_str()));
            }
            if (vec->size()) {
                oldmesh = (*vec)[0];
            }
        }
    }
    if (LoadExistant(oldmesh->orig != NULL ? oldmesh->orig : oldmesh)) {
        return;
    }
}

void Mesh::setConvex(bool b) {
    this->convex = b;
    if (orig && orig != this) {
        orig->setConvex(b);
    }
}

using namespace VSFileSystem;
extern Hashtable<std::string, std::vector<Mesh *>, MESH_HASTHABLE_SIZE> bfxmHashTable;

Mesh::Mesh(std::string filename, const Vector &scale, int faction, Flightgroup *fg, bool orig) : hash_name(filename) {
    this->convex = false;
    Mesh *cpy = LoadMesh(filename.c_str(), scale, faction, fg, vector<std::string>());
    if (cpy->orig) {
        LoadExistant(cpy->orig);
        delete cpy;         //wasteful, but hey
        if (orig != false) {
            orig = false;
            std::vector<Mesh *> *tmp = bfxmHashTable.Get(this->orig->hash_name);
            if (tmp && tmp->size() && (*tmp)[0] == this->orig) {
                if (this->orig->refcount == 1) {
                    bfxmHashTable.Delete(this->orig->hash_name);
                    delete tmp;
                    orig = true;
                }
            }
            if (meshHashTable.Get(this->orig->hash_name) == this->orig) {
                if (this->orig->refcount == 1) {
                    meshHashTable.Delete(this->orig->hash_name);
                    orig = true;
                }
            }
            if (orig) {
                Mesh *tmp = this->orig;
                tmp->orig = this;
                this->orig = NULL;
                refcount = 2;
                delete[] tmp;
            }
        }
    } else {
        delete cpy;
        VS_LOG(error, (boost::format("fallback, %1$s unable to be loaded as bfxm\n") % filename.c_str()));
    }
}

Mesh::Mesh(const char *filename,
        const Vector &scale,
        int faction,
        Flightgroup *fg,
        bool orig,
        const vector<string> &textureOverride) : hash_name(filename) {
    this->convex = false;
    this->orig = NULL;
    InitUnit();
    Mesh *oldmesh;
    if (LoadExistant(filename, scale, faction)) {
        return;
    }
    bool shared = false;
    VSFile f;
    VSError err = Unspecified;
    err = f.OpenReadOnly(filename, MeshFile);
    if (err > Ok) {
        VS_LOG(error, (boost::format("Cannot Open Mesh File %1$s\n") % filename));
//cleanexit=1;
//winsys_exit(1);
        return;
    }
    shared = (err == Shared);

    bool xml = true;
    if (xml) {
        //LoadXML(filename,scale,faction,fg,orig);
        LoadXML(f, scale, faction, fg, orig, textureOverride);
        oldmesh = this->orig;
    } else {
        //This must be changed someday
        LoadBinary(shared ? (VSFileSystem::shared_meshes + "/" + (filename)).c_str() : filename, faction);
        oldmesh = new Mesh[1];
    }
    if (err <= Ok) {
        f.Close();
    }
    draw_queue = new vector<MeshDrawContext>[NUM_ZBUF_SEQ + 1];
    if (!orig) {
        hash_name = shared ? VSFileSystem::GetSharedMeshHashName(filename, scale, faction) : VSFileSystem::GetHashName(
                filename,
                scale,
                faction);
        meshHashTable.Put(hash_name, oldmesh);
        //oldmesh[0]=*this;
        *oldmesh = *this;
        oldmesh->orig = NULL;
        oldmesh->refcount++;
    } else {
        this->orig = NULL;
    }
}

float const ooPI = 1.00F / 3.1415926535F;

//#include "d3d_internal.h"
void Mesh::SetMaterial(const GFXMaterial &mat) {
    GFXSetMaterial(myMatNum, mat);
    if (orig) {
        for (int i = 0; i < numlods; i++) {
            orig[i].myMatNum = myMatNum;
        }
    }
}

int Mesh::getNumLOD() const {
    return numlods;
}

void Mesh::setCurrentFrame(float which) {
    framespersecond = which;
}

float Mesh::getCurrentFrame() const {
    return framespersecond;
}

GFXVertexList *Mesh::getVertexList() const {
    return vlist;
}

void Mesh::setVertexList(GFXVertexList *_vlist) {
    vlist = _vlist;
}

float Mesh::getFramesPerSecond() const {
    return orig ? orig->framespersecond : framespersecond;
}

Mesh *Mesh::getLOD(float lod, bool bBypassDamping) {
    if (!orig) {
        return this;
    }
    Mesh *retval = &orig[0];
    vector<int> *animFrames = 0;
    if (getFramesPerSecond() > .0000001 && (animFrames = animationSequences.Get(hash_name))) {
        //return &orig[(int)floor(fmod (getNewTime()*getFramesPerSecond(),numlods))];
        unsigned int which = (int) float_to_int(floor(fmod(getCurrentFrame(),
                animFrames->size())));
        float adv = GetElapsedTime() * getFramesPerSecond();
        static float max_frames_skipped =
                vega_config::config->graphics.mesh_animation_max_frames_skipped
        if (adv > max_frames_skipped) {
            adv = max_frames_skipped;
        }
        setCurrentFrame(getCurrentFrame() + adv);
        return &orig[(*animFrames)[which % animFrames->size()] % getNumLOD()];
    } else {
        float maxlodsize = retval ? retval->lodsize : 0.0f;
        for (int i = 1; i < numlods; i++) {
            float lodoffs = 0;
            if (!bBypassDamping) {
                if (lod < orig[i].lodsize) {
                    lodoffs = ((i < numlods - 1) ? (orig[i + 1].lodsize - orig[i].lodsize) / LOD_HYSTHERESIS_DIVIDER
                            : 0.0f);
                } else {
                    lodoffs = ((i > 0) ? (orig[i - 1].lodsize - orig[i].lodsize) / LOD_HYSTHERESIS_DIVIDER : 0.0f);
                }
                float maxenlargement = ((orig[i].lodsize * LOD_HYSTHERESIS_MAXENLARGEMENT_FACTOR) - orig[i].lodsize);
                if ((lodoffs > 0) && (lodoffs > maxenlargement)) {
                    lodoffs =
                            maxenlargement;
                }                     //Avoid excessive enlargement of low-detail LOD levels, when LOD levels are far apart.
            }
            if ((lod < (orig[i].lodsize + lodoffs)) && (lod < maxlodsize)) {
                maxlodsize = orig[i].lodsize;
                retval = &orig[i];
            }
        }
    }
    return retval;
}

void Mesh::SetBlendMode(BLENDFUNC src, BLENDFUNC dst, bool lodcascade) {
    blendSrc = src;
    blendDst = dst;
    draw_sequence = 0;
    if (blendDst != ZERO) {
        draw_sequence++;
        if (blendDst != ONE) {
            draw_sequence++;
        }
    }
    if (orig) {
        orig->draw_sequence = draw_sequence;
        orig->blendSrc = src;
        orig->blendDst = dst;
        if (lodcascade) {
            for (int i = 1; i < numlods; i++) {
                orig[i].draw_sequence = draw_sequence;
                orig[i].blendSrc = src;
                orig[i].blendDst = dst;
            }
        }
    }
}

enum EX_EXCLUSION { EX_X, EX_Y, EX_Z };

inline bool OpenWithin(const QVector &query,
        const Vector &mn,
        const Vector &mx,
        const float err,
        enum EX_EXCLUSION excludeWhich) {
    switch (excludeWhich) {
        case EX_X:
            return (query.j >= mn.j - err) && (query.k >= mn.k - err) && (query.j <= mx.j + err)
                    && (query.k <= mx.k + err);

        case EX_Y:
            return (query.i >= mn.i - err) && (query.k >= mn.k - err) && (query.i <= mx.i + err)
                    && (query.k <= mx.k + err);

        case EX_Z:
        default:
            return (query.j >= mn.j - err) && (query.i >= mn.i - err) && (query.j <= mx.j + err)
                    && (query.i <= mx.i + err);
    }
}
/*
bool Mesh::queryBoundingBox( const QVector &eye, const QVector &end, const float err ) const
{
    QVector slope( end-eye );
    QVector IntersectXYZ;
    double  k = ( (mn.i-eye.i)/slope.i );
    IntersectXYZ = eye+k*slope;      //(Normal dot (mn-eye)/div)*slope
    if ( OpenWithin( IntersectXYZ, mn, mx, err, EX_X ) )
        return true;
    k = ( (mx.i-eye.i)/slope.i );
    if (k >= 0) {
        IntersectXYZ = eye+k*slope;
        if ( OpenWithin( IntersectXYZ, mn, mx, err, EX_X ) )
            return true;
    }
    k = ( (mn.j-eye.j)/slope.j );
    if (k >= 0) {
        IntersectXYZ = eye+k*slope;
        if ( OpenWithin( IntersectXYZ, mn, mx, err, EX_Y ) )
            return true;
    }
    k = ( (mx.j-eye.j)/slope.j );
    if (k >= 0) {
        IntersectXYZ = eye+k*slope;
        if ( OpenWithin( IntersectXYZ, mn, mx, err, EX_Y ) )
            return true;
    }
    k = ( (mn.k-eye.k)/slope.k );
    if (k >= 0) {
        IntersectXYZ = eye+k*slope;
        if ( OpenWithin( IntersectXYZ, mn, mx, err, EX_Z ) )
            return true;
    }
    k = ( (mx.k-eye.k)/slope.k );
    if (k >= 0) {
        IntersectXYZ = eye+k*slope;
        if ( OpenWithin( IntersectXYZ, mn, mx, err, EX_Z ) )
            return true;
    }
    return false;
}

bool Mesh::queryBoundingBox( const QVector &start, const float err ) const
{
    return start.i >= mn.i-err && start.j >= mn.j-err && start.k >= mn.k-err
           && start.i <= mx.i+err && start.j <= mx.j+err && start.k <= mx.k+err;
}

BoundingBox* Mesh::getBoundingBox()
{
    BoundingBox *tbox = new BoundingBox( QVector( mn.i, 0, 0 ), QVector( mx.i, 0, 0 ),
                                        QVector( 0, mn.j, 0 ), QVector( 0, mx.j, 0 ),
                                        QVector( 0, 0, mn.k ), QVector( 0, 0, mx.k ) );
    return tbox;
}
*/
