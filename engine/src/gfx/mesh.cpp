/*
 * mesh.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include "aux_logo.h"
#include "mesh.h"
#include "camera.h"
#include <math.h>
#include "cmd/nebula.h"
#include <list>
#include <string>
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "lin_time.h"
#include "gfxlib.h"
#include "vs_globals.h"
#include "configxml.h"
#include "hashtable.h"
#include "vegastrike.h"
#include <cfloat>
#include <boost/utility/string_view.hpp>
#include "preferred_types.h"
#include "shared_ptr_hashtable.h"

#define LOD_HYSTHERESIS_DIVIDER (20)
#define LOD_HYSTHERESIS_MAXENLARGEMENT_FACTOR (1.1)

using namespace vega_types;
using std::list;

SharedPtrHashtable<std::string, Mesh, 503> Mesh::meshHashTable;
SharedPtrHashtable<std::string, SequenceContainer<int>, 503> Mesh::animationSequences;
Vector mouseline;

int Mesh::getNumAnimationFrames(const std::string &which) const {
    if (which.empty()) {
        SharedPtr<SequenceContainer<int>> const animSeq = animationSequences.Get(hash_name);
        if (animSeq) {
            return animSeq->size();
        }
    } else {
        SharedPtr<SequenceContainer<int>> const animSeq = animationSequences.Get(hash_name + "*" + which);
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
    forcelogos.reset();
    squadlogos.reset();
    local_pos = Vector(0, 0, 0);
    blendSrc = ONE;
    blendDst = ZERO;
    vlist.reset();
    mn = Vector(0, 0, 0);
    mx = Vector(0, 0, 0);
    radialSize = 0;
    if (!Decal) {
        Decal = MakeShared<SequenceContainer<SharedPtr<Texture>>>();
    }
    if (Decal->empty()) {
        Decal->push_back(nullptr);
    }
    if (!LocalFX) {
        LocalFX = MakeShared<vega_types::SequenceContainer<vega_types::SharedPtr<MeshFX>>>();
    }
    if (LocalFX->empty()) {
        LocalFX->push_back(nullptr);
    }

    //texturename[0] = -1;
//    numforcelogo = numsquadlogo = 0;
    myMatNum = 0;     //default material!
    //scale = Vector(1.0,1.0,1.0);
//    refcount = 1;     //FIXME VEGASTRIKE  THIS _WAS_ zero...NOW ONE
    orig.reset();

    envMapAndLit = 0x3;
    setEnvMap(GFXTRUE);
    setLighting(GFXTRUE);
    detailTexture.reset();
    draw_queue.reset();
    will_be_drawn = GFXFALSE;
    draw_sequence = 0;

    initTechnique("fixed");
}

Mesh::Mesh() {
    InitUnit();
}

bool Mesh::LoadExistant(SharedPtr<Mesh> oldmesh) {
    *this = *oldmesh;
//    oldmesh->refcount++;
    if (!orig) {
        orig = MakeShared<SequenceContainer<SharedPtr<Mesh>>>();
    }
    orig->push_front(oldmesh);
    return true;
}

bool Mesh::LoadExistant(const std::string filehash, const Vector &scale, int faction) {
    SharedPtr<Mesh> old_mesh;

    hash_name = VSFileSystem::GetHashName(filehash, scale, faction);
    old_mesh = meshHashTable.Get(hash_name);
    if (!old_mesh) {
        hash_name = VSFileSystem::GetSharedMeshHashName(filehash, scale, faction);
        old_mesh = meshHashTable.Get(hash_name);
    }
    if (old_mesh) {
        return LoadExistant(old_mesh);
    }
    return false;
}

Mesh::Mesh(const Mesh &m) {
    VS_LOG(warning, "UNTESTED MESH COPY CONSTRUCTOR");
    this->orig.reset();
    this->hash_name = m.hash_name;
    InitUnit();
    SharedPtr<Mesh> oldmesh = meshHashTable.Get(hash_name);
    if (!oldmesh) {
        SharedPtr<SequenceContainer<SharedPtr<Mesh>>> const p_meshes = bfxmHashTable()->Get(hash_name);
        for (unsigned int i = 0; i < p_meshes->size(); ++i) {
            SharedPtr<Mesh> mush;
            if (p_meshes->at(i)->orig && !p_meshes->at(i)->orig->empty()) {
                mush = p_meshes->at(i)->orig->at(0);
            } else {
                mush = p_meshes->at(i);
            }
            if ((!m.orig->empty() && mush == m.orig->front()) || mush.get() == &m) {
                oldmesh = p_meshes->at(i);
            }
        }
        if (!oldmesh) {
            if (p_meshes->size() > 1) {
                VS_LOG(warning,
                        (boost::format("Copy constructor %1$s used in ambiguous Situation") % hash_name.c_str()));
            }
            if (!p_meshes->empty()) {
                oldmesh = p_meshes->front();
            }
        }
    }
    if (LoadExistant((oldmesh->orig && !oldmesh->orig->empty()) ? oldmesh->orig->front() : oldmesh)) {
        return;
    }
}

void Mesh::setConvex(bool b) {
    this->convex = b;
    if (orig && !orig->empty() && orig->front().get() != this) {
        orig->front()->setConvex(b);
    }
}

using namespace VSFileSystem;

Mesh::Mesh(std::string filename, const Vector &scale, int faction, Flightgroup *fg, bool is_original)
        : convex(false), hash_name(filename) {
    SharedPtr<Mesh> cpy = LoadMesh(filename.c_str(), scale, faction, fg, {});
    if (cpy->orig && !cpy->orig->empty()) {
        LoadExistant(cpy->orig->front());
        cpy.reset();         //wasteful, but hey
        if (is_original) {
            is_original = false;
            SharedPtr<SequenceContainer<SharedPtr<Mesh>>> tmp = bfxmHashTable()->Get(this->orig->front()->hash_name);
            if (tmp && !tmp->empty() && !this->orig->empty() && tmp->front() == this->orig->front()) {
                if (this->orig->front().use_count() == 1) {
                    bfxmHashTable()->Delete(this->orig->front()->hash_name);
                    tmp.reset();
                    is_original = true;
                }
            }
            if (meshHashTable.Get(this->orig->front()->hash_name) == this->orig->front()) {
                if (this->orig->front().use_count() == 1) {
                    meshHashTable.Delete(this->orig->front()->hash_name);
                    is_original = true;
                }
            }
            if (is_original) {
                SharedPtr<Mesh> tmp2 = this->orig->front();
                tmp2->orig->push_back(shared_from_this());
                this->orig.reset();
//                refcount = 2;
                tmp2.reset();
            }
        }
    } else {
        cpy.reset();
        VS_LOG(error, (boost::format("fallback, %1$s unable to be loaded as bfxm\n") % filename.c_str()));
    }
}

Mesh::Mesh(boost::string_view filename,
           const Vector &scale,
           int faction,
           Flightgroup *fg,
           bool is_original,
           const  SequenceContainer<string> &textureOverride) : orig(nullptr), convex(false), hash_name(filename) {
    InitUnit();
    if (LoadExistant(filename, scale, faction)) {
        return;
    }
}

float const ooPI = 1.00F / 3.1415926535F;

//#include "d3d_internal.h"
void Mesh::SetMaterial(const GFXMaterial &mat) {
    GFXSetMaterial(myMatNum, mat);
    if (orig) {
        for (int i = 0; i < numlods; ++i) {
            orig->at(i)->myMatNum = myMatNum;
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

SharedPtr<GFXVertexList> Mesh::getVertexList() const {
    return vlist;
}

void Mesh::setVertexList(SharedPtr<GFXVertexList> _vlist) {
    vlist = _vlist;
}

float Mesh::getFramesPerSecond() const {
    return (orig && !orig->empty()) ? orig->front()->framespersecond : framespersecond;
}

SharedPtr<Mesh> Mesh::getLOD(float lod, bool bBypassDamping) {
    if (!orig || orig->empty()) {
        return shared_from_this();
    }
    SharedPtr<Mesh> retval = orig->at(0);
    SharedPtr<SequenceContainer<int>> animFrames = animationSequences.Get(hash_name);
    if (getFramesPerSecond() > .0000001 && animFrames && !animFrames->empty()) {
        //return &orig[(int)floor(fmod (getNewTime()*getFramesPerSecond(),numlods))];
        unsigned int which = (int) float_to_int(floor(fmod(getCurrentFrame(),
                animFrames->size())));
        float adv = GetElapsedTime() * getFramesPerSecond();
        static float max_frames_skipped =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "mesh_animation_max_frames_skipped", "3"));
        if (adv > max_frames_skipped) {
            adv = max_frames_skipped;
        }
        setCurrentFrame(getCurrentFrame() + adv);
        return orig->at(animFrames->at(which % animFrames->size()) % getNumLOD());
    } else {
        float maxlodsize = retval ? retval->lodsize : 0.0f;
        for (int i = 1; i < numlods; ++i) {
            float lodoffs = 0;
            if (!bBypassDamping) {
                if (lod < orig->at(i)->lodsize) {
                    lodoffs = ((i < numlods - 1) ? (orig->at(i + 1)->lodsize - orig->at(i)->lodsize) / LOD_HYSTHERESIS_DIVIDER
                            : 0.0f);
                } else {
                    lodoffs = ((i > 0) ? (orig->at(i - 1)->lodsize - orig->at(i)->lodsize) / LOD_HYSTHERESIS_DIVIDER : 0.0f);
                }
                float const maxenlargement = ((orig->at(i)->lodsize * LOD_HYSTHERESIS_MAXENLARGEMENT_FACTOR) - orig->at(i)->lodsize);
                if ((lodoffs > 0) && (lodoffs > maxenlargement)) {
                    lodoffs =
                            maxenlargement;
                }                     //Avoid excessive enlargement of low-detail LOD levels, when LOD levels are far apart.
            }
            if ((lod < (orig->at(i)->lodsize + lodoffs)) && (lod < maxlodsize)) {
                maxlodsize = orig->at(i)->lodsize;
                retval = orig->at(i);
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
    if (orig && !orig->empty() && orig->front()) {
        SharedPtr<Mesh> orig0 = orig->front();
        orig0->draw_sequence = draw_sequence;
        orig0->blendSrc = src;
        orig0->blendDst = dst;
        if (lodcascade) {
            for (int i = 1; i < numlods; i++) {
                orig->at(i)->draw_sequence = draw_sequence;
                orig->at(i)->blendSrc = src;
                orig->at(i)->blendDst = dst;
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
