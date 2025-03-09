/*
 * mesh_gfx.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically:
 * surfdargent, hellcatv, ace123, klaussfreire, dan_w, pyramid3d
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

//====================================
// @file   : mesh_gfx.cpp
// @brief  : draws meshes
//====================================

#include <algorithm>
#include "mesh.h"
#include "aux_texture.h"
#include "aux_logo.h"
#include "lin_time.h"
#include "configxml.h"
#include "vs_globals.h"
#include "cmd/nebula.h"
#include "gfx/camera.h"
#include "gfx/animation.h"
#include "gfx/technique.h"
#include "mesh_xml.h"
#include "gldrv/gl_globals.h"
#include "gldrv/gl_light.h"
#if defined (CG_SUPPORT)
#include "cg_global.h"
#endif
#include "universe_util.h"
#include "universe.h"
#include <utility>
#ifdef _WIN32
// What do we need from unistd? - let's find out :)
#else
#include <unistd.h>
#endif
#include <signal.h>
#include <sys/types.h>

extern vector<Logo *> undrawn_logos;

#include <exception>
#include <vega_collection_utils.h>

#ifdef _MSC_VER
//Undefine those nasty min/max macros
//MS must hate the STL
#undef min
#undef max
#endif

#include "vs_logging.h"

class Exception : public std::exception {
private:
    std::string _message;

public:
    Exception() {
    }

    Exception(const Exception &other) : _message(other._message) {
    }

    explicit Exception(const std::string &message) : _message(message) {
    }

    virtual ~Exception() {
    }

    virtual const char *what() const noexcept {
        return _message.c_str();
    }
};

class MissingTexture : public Exception {
public:
    explicit MissingTexture(const string &msg) : Exception(msg) {
    }

    MissingTexture() {
    }
};

class OrigMeshContainer {
public:
    float d{};
    Mesh *orig;
    int program{};

    unsigned int transparent{1};
    unsigned int zsort{1};
    unsigned int passno{14};
    int sequence{16};

    OrigMeshContainer() {
        orig = nullptr;
    }

    OrigMeshContainer(Mesh *orig, float d, int passno) {
        assert(passno < orig->technique->getNumPasses());

        const Pass &pass = orig->technique->getPass(passno);
        this->orig = orig;
        this->d = -d;
        this->passno = passno;
        this->sequence = pass.sequence;
        this->program = pass.getCompiledProgram();
        this->transparent =
                ((pass.blendMode == Pass::Decal)
                        || ((pass.blendMode == Pass::Default)
                                && (orig->blendDst == ZERO)
                                && (orig->blendSrc != DESTCOLOR))
                ) ? 0 : 1;
        this->zsort = (transparent
                && ((pass.blendMode != Pass::Default) || (orig->blendDst != ONE) || (orig->blendSrc != ONE))) ? 1 : 0;

        assert(this->passno == passno);
        assert(this->sequence == pass.sequence);
    }

    #define SLESSX(a, b)                            \
    do {if ( !( (a) == (b) ) ) return ( (a) < (b) );  \
    }                                                 \
    while (0)

    #define PLESSX(a, b)                                            \
    do {SLESSX( bool(a), bool(b) ); if ( bool(a) ) SLESSX( *a, *b );  \
    }                                                                 \
    while (0)

    #define SLESS(f) SLESSX( f, b.f )

    #define PLESS(f) PLESSX( f, b.f )

    bool operator<(const OrigMeshContainer &b) const {
        SLESS(sequence); //explicit sequence takes precedence
        SLESS(transparent); //then render opaques first
        SLESS(zsort);   //And the ones that need z-sort last
        if (zsort)
            SLESS(d);
        SLESS(passno);  //Make sure lesser passes render first
        SLESS(program); //group same program together (with fixed-fn at the beginning)
        //Fixed-fn passes have program == 0
        if (program == 0) {
            //Fixed-fn passes sort by texture
            SLESS(orig->Decal.size());
            if (!orig->Decal.empty())
                PLESS(orig->Decal[0]);
        } else {
            //Shader passes sort by effective texture
            const Pass &apass = orig->technique->getPass(passno);
            const Pass &bpass = b.orig->technique->getPass(b.passno);

            SLESSX(apass.getNumTextureUnits(), bpass.getNumTextureUnits());
            for (size_t i = 0, n = apass.getNumTextureUnits(); i < n; ++i) {
                const Pass::TextureUnit atu = apass.getTextureUnit(i);
                const Pass::TextureUnit btu = bpass.getTextureUnit(i);
                if (atu.sourceType == btu.sourceType) {
                    if (atu.sourceType == Pass::TextureUnit::File) {
                        //Compare preloaded textures
                        SLESSX(*atu.texture, *btu.texture);
                    } else if (atu.sourceType == Pass::TextureUnit::Decal) {
                        //Compare decal textures
                        const Texture *ta =
                                (atu.sourceIndex < static_cast<int>(orig->Decal.size())) ? orig->Decal[atu.sourceIndex]
                                        : NULL;
                        const Texture *tb = (btu.sourceIndex < static_cast<int>(b.orig->Decal.size())) ? b.orig
                                ->Decal[btu.sourceIndex]
                                : NULL;
                        PLESSX(ta, tb);
                    }
                } else {
                    //Render file-type ones first
                    return atu.sourceType == Pass::TextureUnit::File;
                }
            }
        }
        //They're equivalent!
        return false;
    }

    #undef PLESS
    #undef SLESS
    #undef PLESSX
    #undef SLESSX

    bool operator==(const OrigMeshContainer &b) const {
        //TODO: Specialize operator==
        return !(*this < b) && !(b < *this);
    }
};

struct OrigMeshPainterSort {
    bool operator()(const OrigMeshContainer &a, const OrigMeshContainer &b) const {
        return a.d > b.d;
    }
};

struct MeshDrawContextPainterSort {
    Vector ctr;
    const std::vector<MeshDrawContext> &queue;

    MeshDrawContextPainterSort(const Vector &center, const std::vector<MeshDrawContext> &q) : ctr(center), queue(q) {
    }

    bool operator()(int a, int b) const {
        return (queue[a].mat.p - ctr).MagnitudeSquared() > (queue[b].mat.p - ctr).MagnitudeSquared();
    }
};

typedef std::vector<OrigMeshContainer> OrigMeshVector;

#define NUM_PASSES (4)
#define BASE_PASS (0)
#define ENVSPEC_PASS (1)
#define DAMAGE_PASS (2)
#define GLOW_PASS (3)
#define BASE_TEX (0)
#define ENVSPEC_TEX (1)
#define DAMAGE_TEX (2)
#define GLOW_TEX (3)
#define NORMAL_TEX (4)
#define NUM_TEX (5)

const int UNDRAWN_MESHES_SIZE = NUM_MESH_SEQUENCE;

OrigMeshVector undrawn_meshes[NUM_MESH_SEQUENCE];

Texture *Mesh::TempGetTexture(MeshXML *xml, std::string filename, std::string factionname, GFXBOOL detail) const {
    static FILTER fil =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "detail_texture_trilinear", "true")) ? TRILINEAR
                    : MIPMAP;
    static bool factionalize_textures =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "faction_dependant_textures", "true"));
    string faction_prefix = (factionalize_textures ? (factionname + "_") : string());
    Texture *ret = NULL;
    string facplus = faction_prefix + filename;
    if (filename.find(".ani") != string::npos) {
        ret = new AnimatedTexture(facplus.c_str(), 1, fil, detail);
        if (!ret->LoadSuccess()) {
            delete ret;
            ret = new AnimatedTexture(filename.c_str(), 1, fil, detail);
            if (!ret->LoadSuccess()) {
                delete ret;
                ret = NULL;
            } else {
                return ret;
            }
        } else {
            return ret;
        }
    }
    ret = new Texture(facplus.c_str(), 1, fil, TEXTURE2D, TEXTURE_2D, GFXFALSE, 65536, detail);
    if (!ret->LoadSuccess()) {
        delete ret;
        ret = new Texture(filename.c_str(), 1, fil, TEXTURE2D, TEXTURE_2D, GFXFALSE, 65536, detail);
    }
    return ret;
}

int Mesh::getNumTextureFrames() const {
    if (!Decal.empty()) {
        if (Decal[0]) {
            return Decal[0]->numFrames();
        }
    }
    return 1;
}

double Mesh::getTextureCumulativeTime() const {
    if (!Decal.empty()) {
        if (Decal[0]) {
            return Decal[0]->curTime();
        }
    }
    return 0;
}

float Mesh::getTextureFramesPerSecond() const {
    if (!Decal.empty()) {
        if (Decal[0]) {
            return Decal[0]->framesPerSecond();
        }
    }
    return 0;
}

void Mesh::setTextureCumulativeTime(double d) {
    for (unsigned int i = 0; i < Decal.size(); ++i) {
        if (Decal[i]) {
            Decal[i]->setTime(d);
        }
    }
}

Texture *Mesh::TempGetTexture(MeshXML *xml, int index, std::string factionname) const {
    static bool factionalize_textures =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "faction_dependant_textures", "true"));
    string faction_prefix = (factionalize_textures ? (factionname + "_") : string());
    Texture *tex = NULL;
    assert(index < (int) xml->decals.size());
    MeshXML::ZeTexture *zt = &(xml->decals[index]);
    if (zt->animated_name.length()) {
        string tempani = faction_prefix + zt->animated_name;
        tex = new AnimatedTexture(tempani.c_str(), 0, BILINEAR);
        if (!tex->LoadSuccess()) {
            delete tex;
            tex = new AnimatedTexture(zt->animated_name.c_str(), 0, BILINEAR);
        }
    } else if (zt->decal_name.length() == 0) {
        tex = NULL;
    } else {
        if (zt->alpha_name.length() == 0) {
            string temptex = faction_prefix + zt->decal_name;
            tex =
                    new Texture(
                            temptex.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D,
                            (g_game.use_ship_textures || xml->force_texture) ? GFXTRUE : GFXFALSE);
            if (!tex->LoadSuccess()) {
                delete tex;
                tex =
                        new Texture(
                                zt->decal_name.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D,
                                (g_game.use_ship_textures || xml->force_texture) ? GFXTRUE : GFXFALSE);
            }
        } else {
            string temptex = faction_prefix + zt->decal_name;
            string tempalp = faction_prefix + zt->alpha_name;
            tex =
                    new Texture(temptex.c_str(), tempalp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1, 0,
                            (g_game.use_ship_textures || xml->force_texture) ? GFXTRUE : GFXFALSE);
            if (!tex->LoadSuccess()) {
                delete tex;
                tex =
                        new Texture(zt->decal_name.c_str(),
                                zt->alpha_name.c_str(),
                                0,
                                MIPMAP,
                                TEXTURE2D,
                                TEXTURE_2D,
                                1,
                                0,
                                (g_game.use_ship_textures || xml->force_texture) ? GFXTRUE : GFXFALSE);
            }
        }
    }
    return tex;
}

Texture *createTexture(const char *filename,
        int stage = 0,
        enum FILTER f1 = MIPMAP,
        enum TEXTURE_TARGET t0 = TEXTURE2D,
        enum TEXTURE_IMAGE_TARGET t = TEXTURE_2D,
        unsigned char c = GFXFALSE,
        int i = 65536) {
    return new Texture(filename, stage, f1, t0, t, c, i);
}

Logo *createLogo(int numberlogos,
        Vector *center,
        Vector *normal,
        float *sizes,
        float *rotations,
        float offset,
        Texture *Dec,
        Vector *Ref) {
    return new Logo(numberlogos, center, normal, sizes, rotations, offset, Dec, Ref);
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
    return new Texture(ccc, cc, k, f1, t0, t, f, j, c, i);
}

AnimatedTexture *createAnimatedTexture(char const *c, int i, enum FILTER f) {
    return new AnimatedTexture(c, i, f);
}

extern Hashtable<std::string, std::vector<Mesh *>, MESH_HASTHABLE_SIZE> bfxmHashTable;

Mesh::~Mesh() {
    if (!orig || orig == this) {
        for (auto & undrawn_mesh_vec : undrawn_meshes) {
            if (!undrawn_mesh_vec.empty()) {
                auto first_to_remove = std::stable_partition(undrawn_mesh_vec.begin(),
                                                                  undrawn_mesh_vec.end(),
                                                             [this](const auto &pi) { return pi.orig != this; });
                undrawn_mesh_vec.erase(first_to_remove, undrawn_mesh_vec.end());
            }
        }
        if (vlist != nullptr) {
            delete vlist;
            vlist = nullptr;
        }
        for (auto & i : Decal) {
            if (i != nullptr) {
                delete i;
                i = nullptr;
            }
        }
        if (squadlogos != nullptr) {
            delete squadlogos;
            squadlogos = nullptr;
        }
        if (forcelogos != nullptr) {
            delete forcelogos;
            forcelogos = nullptr;
        }
        if (meshHashTable.Get(hash_name) == this) {
            meshHashTable.Delete(hash_name);
        }
        vector<Mesh *> *hashers = bfxmHashTable.Get(hash_name);
        bool hashers_was_deleted = false;
        remove_all_references_to(this, hashers, true, hashers_was_deleted);
        if (hashers_was_deleted) {
            VS_LOG_AND_FLUSH(debug, "hashers was deleted");
        }
        if (draw_queue != nullptr) {
            delete[] draw_queue;
            draw_queue = nullptr;
        }
    } else {
        orig->refcount--;
        VS_LOG(debug, (boost::format("orig refcount: %1%") % refcount));
    }
}

void Mesh::Draw(float lod,
                const Matrix &m,
                float toofar,
                Cloak cloak,
                float nebdist,
                unsigned char hulldamage,
                bool renormalize,
                const MeshFX *mfx) //short fix
{
    Mesh *origmesh = getLOD(lod);
    if (origmesh->rSize() > 0) {
        //Vector pos (local_pos.Transform(m));
        MeshDrawContext c(m);
        if (mfx) {
            c.xtraFX = *mfx;
            c.useXtraFX = true;
        }
        UpdateFX(GetElapsedTime());
        c.SpecialFX = &LocalFX;
        c.damage = hulldamage;

        c.mesh_seq = ((toofar + rSize()) > g_game.zfar) ? NUM_ZBUF_SEQ : draw_sequence;

        // Cloaking and Nebula
        c.cloaked = MeshDrawContext::NONE;
        if (nebdist < 0) {
            c.cloaked |= MeshDrawContext::FOG;
        }
        if (renormalize) {
            c.cloaked |= MeshDrawContext::RENORMALIZE;
        }

        // This should have gradually made the ship cloak go transparent
        // for cloak.Glass but it does the same thing as the ordinary cloak.
        // TODO: revisit this.
        if(cloak.Active()) {
            c.cloaked |= MeshDrawContext::CLOAK;
            c.cloaked |= MeshDrawContext::GLASSCLOAK;

            if (cloak.Glass()) {
                c.CloakFX.a = cloak.Current();
                c.mesh_seq = 2; //MESH_SPECIAL_FX_ONLY;
            } else {
                c.mesh_seq = 2;
                c.CloakFX.r = cloak.Visibility();
                c.CloakFX.g = cloak.Visibility();
                c.CloakFX.b = cloak.Visibility();
                c.CloakFX.a = cloak.Current();
            }
        }

        if (cloak.Cloaking()) {
            c.cloaked |= MeshDrawContext::NEARINVIS;
        }

        //c.mat[12]=pos.i;
        //c.mat[13]=pos.j;
        //c.mat[14]=pos.k;//to translate to local_pos which is now obsolete!

        origmesh->draw_queue[static_cast<size_t>(c.mesh_seq)].push_back(c);
        if (!(origmesh->will_be_drawn & (1 << c.mesh_seq))) {
            origmesh->will_be_drawn |= (1 << c.mesh_seq);
            for (int passno = 0, npasses = origmesh->technique->getNumPasses(); passno < npasses; ++passno) {
                undrawn_meshes[static_cast<size_t>(c.mesh_seq)].push_back(OrigMeshContainer(origmesh,
                        toofar - rSize(),
                        passno));
            }
        }
        will_be_drawn |= (1 << c.mesh_seq);
    }
}

void Mesh::DrawNow(float lod, bool centered, const Matrix &m, int cloak, float nebdist) {
    //short fix
    Mesh *o = getLOD(lod);
    //fixme: cloaking not delt with.... not needed for backgroudn anyway
    if (nebdist < 0) {
        Nebula *t = _Universe->AccessCamera()->GetNebula();
        if (t) {
            t->SetFogState();
        }
    } else {
        GFXFogMode(FOG_OFF);
    }
    if (centered) {
        //Matrix m1 (m);
        //Vector pos(_Universe->AccessCamera()->GetPosition().Transform(m1));
        //m1[12]=pos.i;
        //m1[13]=pos.j;
        //m1[14]=pos.k;
        GFXCenterCamera(true);
        GFXLoadMatrixModel(m);
    } else {
        if (o->draw_sequence != MESH_SPECIAL_FX_ONLY) {
            GFXLoadIdentity(MODEL);
            GFXPickLights(Vector(m.p.i, m.p.j, m.p.k), rSize());
        }
        GFXLoadMatrixModel(m);
    }
    //Making it static avoids frequent reallocations - although may be troublesome for thread safety
    //but... WTH... nothing is thread safe in VS.
    //Also: Be careful with reentrancy... right now, this section is not reentrant.
    static vector<int> specialfxlight;

    unsigned int i;
    for (i = 0; i < LocalFX.size(); i++) {
        int ligh;
        GFXCreateLight(ligh, (LocalFX)[i], true);
        specialfxlight.push_back(ligh);
    }
    GFXSelectMaterial(o->myMatNum);
    if (blendSrc != SRCALPHA && blendDst != ZERO) {
        GFXDisable(DEPTHWRITE);
    }
    GFXBlendMode(blendSrc, blendDst);
    if (!o->Decal.empty() && o->Decal[0]) {
        o->Decal[0]->MakeActive();
    }
    GFXTextureEnv(0, GFXMODULATETEXTURE);     //Default diffuse mode
    GFXTextureEnv(1, GFXADDTEXTURE);     //Default envmap mode
    GFXToggleTexture(bool(!o->Decal.empty() && o->Decal[0]), 0);
    o->vlist->DrawOnce();
    if (centered) {
        GFXCenterCamera(false);
    }
    for (i = 0; i < specialfxlight.size(); i++) {
        GFXDeleteLight(specialfxlight[i]);
    }
    if (cloak >= 0 && cloak < CLKSCALE) {
        GFXEnable(TEXTURE1);
    }
}

void Mesh::ProcessZFarMeshes(bool nocamerasetup) {
    int a = NUM_ZBUF_SEQ;

    if (!undrawn_meshes[a].empty()) {
        //clear Z buffer
        GFXClear(GFXFALSE, GFXTRUE, GFXFALSE);

        static float far_margin = XMLSupport::parse_float(vs_config->getVariable("graphics", "mesh_far_percent", ".8"));
        if (!nocamerasetup) {
            _Universe->AccessCamera()->UpdateGFXFrustum(GFXTRUE, g_game.zfar * far_margin, 0);
        }

        std::sort(undrawn_meshes[a].begin(), undrawn_meshes[a].end());
        for (OrigMeshVector::iterator it = undrawn_meshes[a].begin(); it < undrawn_meshes[a].end(); ++it) {
            Mesh *m = it->orig;
            m->ProcessDrawQueue(it->passno, a, it->zsort, _Universe->AccessCamera()->GetPosition());
            m->will_be_drawn &= (~(1 << a));           //not accurate any more
        }
        for (OrigMeshVector::iterator it = undrawn_meshes[a].begin(); it < undrawn_meshes[a].end(); ++it) {
            Mesh *m = it->orig;
            m->draw_queue[a].clear();
        }
        undrawn_meshes[a].clear();
    }

    GFXDeactivateShader();
    if (gl_options.ext_srgb_framebuffer) {
        glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    }
    Animation::ProcessFarDrawQueue(-FLT_MAX);
}

const GFXMaterial &Mesh::GetMaterial() const {
    return GFXGetMaterial(myMatNum);
}

template<typename T>
inline bool rangesOverlap(T min1, T max1, T min2, T max2) {
    return !(((min1 < min2) == (max1 < min2))
            && ((min1 < max2) == (max1 < max2))
            && ((min1 < min2) == (min1 < max2)));
}

void Mesh::ProcessUndrawnMeshes(bool pushSpecialEffects, bool nocamerasetup) {
    bool zcleared = false;

    for (int a = 0; a < NUM_ZBUF_SEQ; a++) {
        if (undrawn_meshes[a].empty() && undrawn_logos.empty()) {
            continue;
        }
        if (!zcleared) {
            //clear Z buffer
            GFXClear(GFXFALSE, GFXTRUE, GFXFALSE);
            zcleared = true;
        }
        if (a == MESH_SPECIAL_FX_ONLY) {
            GFXPushGlobalEffects();
            GFXDisable(DEPTHWRITE);
            // This is the more correct way to do it (svn r13722).
            // Unfortunately it breaks the background of the nav computer (Shift+M).
            // Therefore "else if (!nocamerasetup)" is replaced with
            // "} else {" until fixed.
            // The bug was introduced in (svn r13722)
            //} else if (!nocamerasetup) {
        } else { // less correct (svn r13721) but working on nav computer
            _Universe->AccessCamera()->UpdateGFXFrustum(GFXTRUE, g_game.znear, g_game.zfar);
        }
        std::sort(undrawn_meshes[a].begin(), undrawn_meshes[a].end());
        for (OrigMeshVector::iterator it = undrawn_meshes[a].begin(); it < undrawn_meshes[a].end(); ++it) {
            Mesh *m = it->orig;
            m->ProcessDrawQueue(it->passno, a, it->zsort, _Universe->AccessCamera()->GetPosition());
            m->will_be_drawn &= (~(1 << a));               //not accurate any more
        }
        for (OrigMeshVector::iterator it = undrawn_meshes[a].begin(); it < undrawn_meshes[a].end(); ++it) {
            Mesh *m = it->orig;
            m->draw_queue[a].clear();
        }
        undrawn_meshes[a].clear();
        if (a == MESH_SPECIAL_FX_ONLY) {
            if (!pushSpecialEffects) {
                GFXPopGlobalEffects();
            }
            GFXEnable(DEPTHWRITE);
        }
        for (vector<Logo *>::iterator it = undrawn_logos.begin(); it < undrawn_logos.end(); it++) {
            Logo *l = *it;
            l->ProcessDrawQueue();
            l->will_be_drawn = false;
        }
        undrawn_logos.clear();
    }

    // Restore state
    GFXDeactivateShader();
    if (gl_options.ext_srgb_framebuffer) {
        glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    }
}

void Mesh::RestoreCullFace(int whichdrawqueue) {
    if (blendDst != ZERO || getCullFaceForcedOff()) {
        if (blendSrc != SRCALPHA) {
            GFXEnable(CULLFACE);
        }
    }
}

void Mesh::SelectCullFace(int whichdrawqueue) {
    if (getCullFaceForcedOn()) {
        GFXEnable(CULLFACE);
    } else if (getCullFaceForcedOff()) {
        GFXDisable(CULLFACE);
    }
    if (blendDst != ZERO) {
        if (blendSrc != SRCALPHA || getCullFaceForcedOff()) {
            GFXDisable(CULLFACE);
        }
    }
}

void SetupCloakState(char cloaked,
        const GFXColor &CloakFX,
        vector<int> &specialfxlight,
        unsigned char hulldamage,
        unsigned int matnum) {
    if (cloaked & MeshDrawContext::CLOAK) {
        GFXPushBlendMode();
        GFXDisable(CULLFACE);
        /**/
        GFXEnable(LIGHTING);
        GFXEnable(TEXTURE0);
        GFXEnable(TEXTURE1);
        /**/
        if (cloaked & MeshDrawContext::GLASSCLOAK) {
            GFXDisable(TEXTURE1);
            int ligh;
            GFXCreateLight(ligh,
                    GFXLight(true,
                            GFXColor(0, 0, 0,
                                    1), GFXColor(0, 0, 0, 1), GFXColor(0, 0, 0, 1), CloakFX, GFXColor(1,
                                    0,
                                    0)),
                    true);
            specialfxlight.push_back(ligh);
            GFXBlendMode(ONE, ONE);
            GFXSelectMaterialHighlights(matnum,
                    GFXColor(1, 1, 1, 1),
                    GFXColor(1, 1, 1, 1),
                    GFXColor(1, 1, 1, 1),
                    CloakFX);
        } else {
            GFXEnable(TEXTURE1);
            if (cloaked & MeshDrawContext::NEARINVIS) {
                //NOT sure I like teh jump this produces	GFXDisable (TEXTURE1);
            }
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
            GFXColorMaterial(AMBIENT | DIFFUSE);
            if (hulldamage) {
                GFXColor4f(CloakFX.r, CloakFX.g, CloakFX.b, CloakFX.a * hulldamage / 255);
            } else {
                GFXColorf(CloakFX);
            }
        }
    } else if (hulldamage) {
        //ok now we go in and do the dirtying
        GFXColorMaterial(AMBIENT | DIFFUSE);
        GFXColor4f(1, 1, 1, hulldamage / 255.);
    }
}

static void RestoreCloakState(char cloaked, bool envMap, unsigned char damage) {
    if (cloaked & MeshDrawContext::CLOAK) {
        GFXColorMaterial(0);
        if (envMap) {
            GFXEnable(TEXTURE1);
        }
        GFXPopBlendMode();
#if defined (CG_SUPPORT)
        cgGLDisableProfile( cloak_cg->vertexProfile );
#endif
    }
    if (damage) {
        GFXColorMaterial(0);
    }
}

static void SetupFogState(char cloaked) {
    if (cloaked & MeshDrawContext::FOG) {
        Nebula *t = _Universe->AccessCamera()->GetNebula();
        if (t) {
            t->SetFogState();
        }
    } else {
        GFXFogMode(FOG_OFF);
    }
}

bool SetupSpecMapFirstPass(Texture **decal,
        size_t decalSize,
        unsigned int mat,
        bool envMap,
        float polygon_offset,
        Texture *detailTexture,
        const vector<Vector> &detailPlanes,
        bool &skip_glowpass,
        bool nomultienv) {
    GFXPushBlendMode();

    static bool separatespec =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "separatespecularcolor", "false")) ? GFXTRUE
                    : GFXFALSE;
    GFXSetSeparateSpecularColor(separatespec);

    static bool multitex_glowpass =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "multitexture_glowmaps", "true"));
    if (polygon_offset) {
        float a, b;
        GFXGetPolygonOffset(&a, &b);
        GFXPolygonOffset(a, b - polygon_offset);
    }
    if (nomultienv) {
        GFXSelectMaterialHighlights(mat,
                GFXColor(1, 1, 1, 1),
                GFXColor(1, 1, 1, 1),
                GFXColor(0, 0, 0, 0),
                GFXColor(0, 0, 0, 0));
        GFXBlendMode(ONE, ONE);
        GFXDepthFunc(LEQUAL);
    }
    bool retval = false;
    skip_glowpass = false;
    size_t detailoffset = 2;
    if (!nomultienv && (decalSize > 1) && (decal[1])) {
        detailoffset = 1;
        GFXSelectMaterialHighlights(mat,
                GFXColor(1, 1, 1, 1),
                GFXColor(1, 1, 1, 1),
                GFXColor(0, 0, 0, 0),
                GFXColor(0, 0, 0, 0));
        retval = true;
        if (envMap && detailTexture == NULL) {
            if ((decalSize > GLOW_PASS) && decal[GLOW_PASS] && gl_options.Multitexture && multitex_glowpass) {
                decal[GLOW_PASS]->MakeActive(1);
                GFXTextureEnv(1, GFXADDTEXTURE);
                GFXToggleTexture(true, 1);
                GFXTextureCoordGenMode(1, NO_GEN, NULL, NULL);
                skip_glowpass = true;
            } else {
                GFXDisable(TEXTURE1);
            }
        }
        if (decal[0]) {
            decal[0]->MakeActive(0);
            GFXToggleTexture(true, 0);
        } else {
            GFXToggleTexture(false, 0);
        }
    } else if (decalSize && decal[0]) {
        GFXToggleTexture(true, 0);
        decal[0]->MakeActive(0);
    } else {
        GFXToggleTexture(false, 0);
    }
    if (detailTexture && (gl_options.Multitexture > detailoffset)) {
        for (unsigned int i = 1; i < detailPlanes.size(); i += 2) {
            int stage = (i / 2) + detailoffset;
            const float params[4] = {detailPlanes[i - 1].i, detailPlanes[i - 1].j, detailPlanes[i - 1].k, 0};
            const float paramt[4] = {detailPlanes[i].i, detailPlanes[i].j, detailPlanes[i].k, 0};
            GFXTextureCoordGenMode(stage, OBJECT_LINEAR_GEN, params, paramt);
            detailTexture->MakeActive(stage);
            GFXTextureEnv(stage, GFXDETAILTEXTURE);
            GFXToggleTexture(true, stage);
        }
    }
    return retval;
}

void RestoreFirstPassState(Texture *detailTexture,
        const vector<Vector> &detailPlanes,
        bool skipped_glowpass,
        bool nomultienv) {
    GFXPopBlendMode();
    if (!nomultienv) {
        GFXDepthFunc(LEQUAL);
    }
    if (detailTexture || skipped_glowpass) {
        _Universe->activeStarSystem()->activateLightMap();
    }
    if (detailTexture) {
        unsigned int sizeplus1 = detailPlanes.size() / 2 + 1;
        for (unsigned int i = 1; i < sizeplus1; i++) {
            GFXToggleTexture(false, i + 1);
        }              //turn off high detial tex

        GFXTextureCoordGenMode(1, NO_GEN, NULL, NULL);
    }
}

void SetupEnvmapPass(Texture *decal, unsigned int mat, int passno) {
    assert(passno >= 0 && passno <= 2);

    //This is only used when there's no multitexturing... so don't use multitexturing
    GFXPushBlendMode();
    GFXSelectMaterialHighlights(mat,
            GFXColor(0, 0, 0, 0),
            GFXColor(0, 0, 0, 0),
            (passno == 2) ? GFXColor(1, 1, 1, 1) : GFXColor(0, 0, 0, 0),
            (passno == 2) ? GFXColor(0, 0, 0, 0) : GFXColor(1, 1, 1, 1));
    if (passno == 2) {
        GFXSetSeparateSpecularColor(GFXFALSE);
    }
    if (passno != 2) {
        GFXDisable(LIGHTING);
    }
    if (decal) {
        GFXBlendMode((passno != 1) ? ONE : ZERO, (passno == 1) ? SRCCOLOR : (passno ? ONE : ZERO));
    } else {
        GFXBlendMode(ONE, ONE);
    }
    if (decal && ((passno == 0) || (passno == 2))) {
        decal->MakeActive(0);
        GFXToggleTexture(true, 0);
    } else if (passno == 1) {
        _Universe->activateLightMap(0);
    } else {
        GFXToggleTexture(false, 0);
    }
    GFXTextureEnv(0, ((passno != 2) ? GFXREPLACETEXTURE : GFXMODULATETEXTURE));
    GFXDepthFunc(LEQUAL);
}

void RestoreEnvmapState() {
    static bool separatespec =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "separatespecularcolor", "false")) ? GFXTRUE
                    : GFXFALSE;
    GFXSetSeparateSpecularColor(separatespec);
    GFXEnable(LIGHTING);
    GFXTextureCoordGenMode(0, NO_GEN, NULL, NULL);
    GFXTextureEnv(0, GFXMODULATETEXTURE);
    GFXDepthFunc(LEQUAL);
    GFXPopBlendMode();
    GFXToggleTexture(false, 0);
}

void SetupSpecMapSecondPass(Texture *decal,
        unsigned int mat,
        BLENDFUNC blendsrc,
        bool envMap,
        const GFXColor &cloakFX,
        float polygon_offset) {
    GFXPushBlendMode();
    GFXSelectMaterialHighlights(mat,
            GFXColor(0, 0, 0, 0),
            GFXColor(0, 0, 0, 0),
            cloakFX,
            (envMap ? GFXColor(1, 1, 1, 1) : GFXColor(0, 0, 0, 0)));
    GFXBlendMode(ONE, ONE);
    if (!envMap || gl_options.Multitexture) {
        if (decal) {
            decal->MakeActive();
        }
    }
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b-1-polygon_offset); //Not needed, since we use GL_EQUAL and appeal to invariance
    GFXDepthFunc(LEQUAL);     //By Klauss - this, with invariance, assures correct rendering (and avoids z-buffer artifacts at low res)
    GFXDisable(DEPTHWRITE);
    if (envMap) {
        int stage = gl_options.Multitexture ? 1 : 0;
        if (!gl_options.Multitexture) {
            _Universe->activateLightMap(0);
        }
        GFXEnable(TEXTURE0);
        GFXActiveTexture(stage);
        GFXTextureEnv(stage, GFXMODULATETEXTURE);
        if (gl_options.Multitexture) {
            GFXEnable(TEXTURE1);
        }
    } else {
        GFXSetSeparateSpecularColor(GFXFALSE);
        GFXTextureEnv(0, GFXMODULATETEXTURE);
        GFXEnable(TEXTURE0);
        GFXActiveTexture(1);
        GFXDisable(TEXTURE1);
    }
}

void SetupGlowMapFourthPass(Texture *decal,
        unsigned int mat,
        BLENDFUNC blendsrc,
        const GFXColor &cloakFX,
        float polygon_offset) {
    GFXPushBlendMode();
    GFXSelectMaterialHighlights(mat,
            GFXColor(0, 0, 0, 0),
            GFXColor(0, 0, 0, 0),
            GFXColor(0, 0, 0, 0),
            cloakFX);
    GFXBlendMode(ONE, ONE);
    if (decal) {
        decal->MakeActive();
    }
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b-2-polygon_offset);
    GFXDepthFunc(LEQUAL);     //By Klauss - this, with invariance, assures correct rendering (and avoids z-buffer artifacts at low res)
    GFXDisable(DEPTHWRITE);
    GFXDisable(TEXTURE1);
}

void SetupDamageMapThirdPass(Texture *decal, unsigned int mat, float polygon_offset) {
    GFXPushBlendMode();
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    if (decal) {
        decal->MakeActive();
    }
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b-DAMAGE_PASS-polygon_offset);
    GFXDepthFunc(LEQUAL);     //By Klauss - this, with invariance, assures correct rendering (and avoids z-buffer artifacts at low res)
    GFXDisable(DEPTHWRITE);
    GFXDisable(TEXTURE1);
}

void RestoreGlowMapState(bool write_to_depthmap, float polygonoffset, float NOT_USED_BUT_BY_HELPER = 3) {
    GFXDepthFunc(LEQUAL);     //By Klauss - restore original depth function
    static bool force_write_to_depthmap =
            XMLSupport::parse_bool(vs_config->getVariable("graphics",
                    "force_glowmap_restore_write_to_depthmap",
                    "true"));
    if (force_write_to_depthmap || write_to_depthmap) {
        GFXEnable(DEPTHWRITE);
    }
    GFXEnable(TEXTURE1);
    GFXPopBlendMode();
}

void RestoreDamageMapState(bool write_to_depthmap, float polygonoffset) {
    RestoreGlowMapState(write_to_depthmap, polygonoffset, DAMAGE_PASS);
}

void RestoreSpecMapState(bool envMap, bool detailMap, bool write_to_depthmap, float polygonoffset) {
    //float a,b;
    //GFXGetPolygonOffset(&a,&b);
    //GFXPolygonOffset (a, b+1+polygonoffset); //Not needed anymore, since InitSpecMapSecondPass() no longer messes with polygon offsets
    GFXDepthFunc(LEQUAL);     //By Klauss - restore original depth function
    if (envMap) {
        if (gl_options.Multitexture) {
            GFXActiveTexture(1);
            GFXTextureEnv(1, GFXADDTEXTURE);             //restore modulate
        } else {
            GFXTextureCoordGenMode(0, NO_GEN, NULL, NULL);
        }
    } else {
        static bool separatespec =
                XMLSupport::parse_bool(vs_config->getVariable("graphics", "separatespecularcolor",
                        "false")) ? GFXTRUE : GFXFALSE;
        GFXSetSeparateSpecularColor(separatespec);
    }
    if (detailMap) {
        GFXTextureCoordGenMode(1, NO_GEN, NULL, NULL);
    }
    if (write_to_depthmap) {
        GFXEnable(DEPTHWRITE);
    }
    GFXEnable(TEXTURE0);
    GFXPopBlendMode();
}

void Mesh::ProcessDrawQueue(size_t whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    //Process the pass for all queued instances
    const Pass &pass = technique->getPass(whichpass);
    if (pass.type == Pass::ShaderPass) {
        ProcessShaderDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    } else {
        ProcessFixedDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    }
    //Restore texture units
    for (unsigned int i = 0; i < gl_options.Multitexture; ++i) {
        GFXToggleTexture(i < 2, i);
    }
}

void Mesh::activateTextureUnit(const Pass::TextureUnit &tu, bool deflt) {
    //I'm leaving target and source index as int's because I tried to change technique.h's target and source to size_t's
    //and had problems. So I'll just add static casts to int to the unsigned/size_t other sides in comparisons --chuck_starchaser
    int targetIndex = tu.targetIndex;
    int sourceIndex = deflt ? tu.defaultIndex : tu.sourceIndex;
    Pass::TextureUnit::SourceType sourceType = deflt ? tu.defaultType : tu.sourceType;
    switch (sourceType) {
        case Pass::TextureUnit::File:
            //Direct file sources go in tu.texture
            switch (tu.texKind) {
                case Pass::TextureUnit::TexDefault:
                case Pass::TextureUnit::Tex2D:
                case Pass::TextureUnit::Tex3D:
                case Pass::TextureUnit::TexCube:
                    tu.texture->MakeActive(targetIndex);
                    break;
                case Pass::TextureUnit::TexSepCube:
                default:
                    throw Exception("Texture Unit for technique of unhandled kind");
            }
            switch (tu.texKind) {
                case Pass::TextureUnit::TexDefault:
                case Pass::TextureUnit::Tex2D:
                    GFXToggleTexture(true, targetIndex, TEXTURE2D);
                    break;
                case Pass::TextureUnit::Tex3D:
                    GFXToggleTexture(true, targetIndex, TEXTURE3D);
                    break;
                case Pass::TextureUnit::TexCube:
                case Pass::TextureUnit::TexSepCube:
                    GFXToggleTexture(true, targetIndex, CUBEMAP);
                    break;
                default:
                    throw Exception("Texture Unit for technique of unhandled kind");
            }
            break;
        case Pass::TextureUnit::Environment:
            _Universe->activateLightMap(targetIndex);
            #ifdef NV_CUBE_MAP
            if (tu.texKind != Pass::TextureUnit::TexDefault
                    && tu.texKind != Pass::TextureUnit::TexCube
                    && tu.texKind != Pass::TextureUnit::TexSepCube) {
                throw Exception(
                        "Environment Texture Unit for technique must be a cube map");
            }
            GFXToggleTexture(true, targetIndex, CUBEMAP);
            #else
            if (tu.texKind != Pass::TextureUnit::TexDefault
                && tu.texKind != Pass::TextureUnit::Tex2D) throw Exception(
                    "Environment Texture Unit for technique must be a 2D spheremap" );
            GFXToggleTexture( true, targetIndex, TEXTURE2D );
            #endif
            break;
        case Pass::TextureUnit::Detail:
            if (tu.texKind != Pass::TextureUnit::TexDefault
                    && tu.texKind != Pass::TextureUnit::Tex2D) {
                throw Exception(
                        "Detail Texture Unit for technique must be 2D");
            }
            if (detailTexture) {
                detailTexture->MakeActive(targetIndex);
            } else if (!deflt) {
                activateTextureUnit(tu, true);
            } else {
                throw MissingTexture(
                        "Texture Unit for technique requested a missing texture (detail default given that cannot be found)");
            }
            break;
        case Pass::TextureUnit::Decal:
            if ((sourceIndex < static_cast<int>(Decal.size())) && Decal[sourceIndex]) {
                //Mesh has the referenced decal
                Decal[sourceIndex]->MakeActive(targetIndex);
            } else if (!deflt) {
                activateTextureUnit(tu, true);
            } else {
                throw MissingTexture(
                        "Texture Unit for technique requested a missing texture (decal default given that cannot be found)");
            }
            switch (tu.texKind) {
                case Pass::TextureUnit::TexDefault:
                case Pass::TextureUnit::Tex2D:
                    GFXToggleTexture(true, targetIndex, TEXTURE2D);
                    break;
                case Pass::TextureUnit::Tex3D:
                    GFXToggleTexture(true, targetIndex, TEXTURE3D);
                    break;
                case Pass::TextureUnit::TexCube:
                case Pass::TextureUnit::TexSepCube:
                    GFXToggleTexture(true, targetIndex, CUBEMAP);
                    break;
                default:
                    throw Exception("Texture Unit for technique of unhandled kind");
            }
            break;
        case Pass::TextureUnit::None: //chuck_starchaser
        default:
            break;
    }
}

static void setupGLState(const Pass &pass,
        bool zwrite,
        BLENDFUNC blendSrc,
        BLENDFUNC blendDst,
        int material,
        unsigned char alphatest,
        int whichdrawqueue) {
    //Setup color/z writes, culling, etc...
    if (pass.colorWrite) {
        GFXEnable(COLORWRITE);
    } else {
        GFXDisable(COLORWRITE);
    }
    {
        DEPTHFUNC func;
        switch (pass.depthFunction) {
            case Pass::Always:
                func = ALWAYS;
                break;
            case Pass::Never:
                func = NEVER;
                break;
            case Pass::Less:
                func = LESS;
                break;
            case Pass::Greater:
                func = GREATER;
                break;
            case Pass::GEqual:
                func = GEQUAL;
                break;
            case Pass::Equal:
                func = EQUAL;
                break;
            default:
            case Pass::LEqual:
                func = LEQUAL;
                break;
        }
        GFXDepthFunc(func);
    }
    {
        POLYMODE mode;
        switch (pass.polyMode) {
            case Pass::Point:
                mode = GFXPOINTMODE;
                break;
            case Pass::Line:
                mode = GFXLINEMODE;
                break;
            default:
            case Pass::Fill:
                mode = GFXFILLMODE;
                break;
        }
        GFXPolygonMode(mode);
    }
    if (pass.polyMode == Pass::Line) {
        GFXLineWidth(pass.lineWidth);
    }
    if (pass.cullMode == Pass::None) {
        GFXDisable(CULLFACE);
    } else if (pass.cullMode == Pass::DefaultFace) {
        //Not handled by this helper function, since it depends on mesh data
        //SelectCullFace( whichdrawqueue );
    } else {
        POLYFACE face;
        GFXEnable(CULLFACE);
        switch (pass.cullMode) {
            case Pass::Front:
                face = GFXFRONT;
                break;
            case Pass::FrontAndBack:
                face = GFXFRONTANDBACK;
                break;
            default:
            case Pass::Back:
                face = GFXBACK;
                break;
        }
        GFXCullFace(face);
    }
    GFXPolygonOffset(pass.offsetFactor, pass.offsetUnits);

    if (zwrite) {
        GFXEnable(DEPTHWRITE);
    } else {
        GFXDisable(DEPTHWRITE);
    }

    //Setup blend mode
    switch (pass.blendMode) {
        case Pass::Add:
            GFXBlendMode(ONE, ONE);
            break;
        case Pass::AlphaBlend:
        case Pass::MultiAlphaBlend:
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
            break;
        case Pass::Decal:
            GFXBlendMode(ONE, ZERO);
            break;
        case Pass::Multiply:
            GFXBlendMode(DESTCOLOR, ZERO);
            break;
        case Pass::PremultAlphaBlend:
            GFXBlendMode(ONE, INVSRCALPHA);
            break;
        case Pass::Default:
        default:
            GFXBlendMode(blendSrc, blendDst);
            break;
    }

    GFXEnable(LIGHTING);
    GFXSelectMaterial(material);

    //If we're doing zwrite, alpha test is more or less mandatory for correct results
    if (alphatest) {
        GFXAlphaTest(GEQUAL, alphatest / 255.0);
    } else if (zwrite) {
        GFXAlphaTest(GREATER, 0);
    }

    if (gl_options.ext_srgb_framebuffer) {
        if (pass.sRGBAware) {
            glEnable(GL_FRAMEBUFFER_SRGB_EXT);
        } else {
            glDisable(GL_FRAMEBUFFER_SRGB_EXT);
        }
    }
}

void Mesh::ProcessShaderDrawQueue(size_t whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    if (!technique->isCompiled(GFXGetProgramVersion())) {
        try {
            technique->compile();
        }
        catch (const Exception &e) {
            VS_LOG(info, (boost::format("Technique recompilation failed: %1%") % e.what()));
        }
    }

    const Pass &pass = technique->getPass(whichpass);

    //First of all, decide zwrite, so we can skip the pass if !zwrite && !cwrite
    bool zwrite;
    if (whichdrawqueue == MESH_SPECIAL_FX_ONLY) {
        //Special effects pass - no zwrites... no zwrites...
        zwrite = false;
    } else {
        //Near draw queue - write to z-buffer if in auto mode and not translucent
        zwrite = (blendDst == ZERO);
        switch (pass.zWrite) {
            case Pass::Auto:
                break;
            case Pass::True:
                zwrite = true;
                break;
            case Pass::False:
                zwrite = false;
                break;
        }
    }
    //If we're not writing anything... why go on?
    if (!pass.colorWrite && !zwrite) {
        return;
    }
    float envmaprgba[4] = {1, 1, 1, 0};
    float noenvmaprgba[4] = {0.5, 0.5, 0.5, 1.0};

    vector<MeshDrawContext> &cur_draw_queue = draw_queue[whichdrawqueue];

    GFXPushBlendMode();
    setupGLState(pass, zwrite, blendSrc, blendDst, myMatNum, alphatest, whichdrawqueue);
    if (pass.cullMode == Pass::DefaultFace) {
        SelectCullFace(whichdrawqueue);
    } // Default not handled by setupGLState, it depends on mesh data

    //Activate shader
    GFXActivateShader(pass.getCompiledProgram());

    //Set shader parameters (instance-independent only)
    int activeLightsArrayParam = -1;
    int apparentLightSizeArrayParam = -1;
    int numLightsParam = -1;
    for (size_t spi = 0; spi < pass.getNumShaderParams(); ++spi) {
        const Pass::ShaderParam &sp = pass.getShaderParam(spi);
        if (sp.id >= 0) {
            switch (sp.semantic) {
                case Pass::ShaderParam::Constant:
                    GFXShaderConstant(sp.id, sp.value);
                    break;
                case Pass::ShaderParam::EnvColor:
                    GFXShaderConstant(sp.id, getEnvMap() ? envmaprgba : noenvmaprgba);
                    break;
                case Pass::ShaderParam::DetailPlane0:
                    GFXShaderConstant(sp.id, detailPlanes[0]);
                    break;
                case Pass::ShaderParam::DetailPlane1:
                    GFXShaderConstant(sp.id, detailPlanes[1]);
                    break;
                case Pass::ShaderParam::GameTime:
                    GFXShaderConstant(sp.id, UniverseUtil::GetGameTime());
                    break;
                case Pass::ShaderParam::NumLights:
                    numLightsParam = sp.id;
                    break;
                case Pass::ShaderParam::ActiveLightsArray:
                    activeLightsArrayParam = sp.id;
                    break;
                case Pass::ShaderParam::ApparentLightSizeArray:
                    apparentLightSizeArrayParam = sp.id;
                    break;
                case Pass::ShaderParam::CloakingPhase: //chuck_starchaser
                default:
                    break;
            }
        }
    }
    //Activate texture units
    size_t tui;
    size_t tuimask = 0;
    for (tui = 0; tui < pass.getNumTextureUnits(); ++tui) {
        const Pass::TextureUnit &tu = pass.getTextureUnit(tui);
        if (tu.targetIndex < 0) {
            continue;
        }
        if (tu.targetParamId >= 0) {
            GFXShaderConstanti(tu.targetParamId, tu.targetIndex);
        } else {
            continue;
        }
        try {
            activateTextureUnit(tu);
            tuimask |= (1 << tu.targetIndex);
        }
        catch (const MissingTexture &e) {
            if (tu.defaultType == Pass::TextureUnit::Decal && tu.defaultIndex == 0) {
                //Global default for decal 0 is white, this allows textureless objects
                //that would otherwise not be possible
                static Texture *white = new Texture("white.png");
                white->MakeActive(tu.targetIndex);
                tuimask |= (1 << tu.targetIndex);
            } else {
                //Ok, this is an error - die horribly
                throw e;
            }
        }
    }
    for (tui = 0; tui < gl_options.Multitexture; ++tui) {
        GFXToggleTexture(((tuimask & (1 << tui)) != 0), tui, TEXTURE2D);
    }
    //Render all instances, no specific order if not necessary
    //TODO: right now only meshes of same kind get drawn in correct order - should fix this.
    static std::vector<int> indices;
    if (zsort) {
        indices.resize(cur_draw_queue.size());
        for (int i = 0, n = cur_draw_queue.size(); i < n; ++i) {
            indices[i] = i;
        }
        std::sort(indices.begin(), indices.end(),
                MeshDrawContextPainterSort(sortctr, cur_draw_queue));
    }
    vlist->BeginDrawState();
    for (int i = 0, n = cur_draw_queue.size(); i < n; ++i) {
        //Making it static avoids frequent reallocations - although may be troublesome for thread safety
        //but... WTH... nothing is thread safe in VS.
        //Also: Be careful with reentrancy... right now, this section is not reentrant.
        static vector<int> lights;
        MeshDrawContext &c = cur_draw_queue[zsort ? indices[i] : i];
        if (c.mesh_seq == whichdrawqueue) {
            lights.clear();
            //Dynamic lights
            if (whichdrawqueue != MESH_SPECIAL_FX_ONLY) {
                int maxPerPass = pass.perLightIteration ? pass.perLightIteration : 1;
                int maxPassCount = pass.perLightIteration ? (pass.maxIterations ? pass.maxIterations : 32) : 1;
                GFXPickLights(Vector(c.mat.p.i, c.mat.p.j, c.mat.p.k),
                        rSize(),
                        lights,
                        maxPerPass * maxPassCount,
                        true);
            }
            //FX lights
            size_t fxLightsBase = lights.size();
            for (size_t i = 0; i < c.SpecialFX->size(); i++) {
                int light;
                GFXLoadMatrixModel(c.mat);
                GFXCreateLight(light, (*c.SpecialFX)[i], true);
                lights.push_back(light);
            }
            if (c.useXtraFX) {
                int light;
                GFXLoadMatrixModel(c.mat);
                GFXCreateLight(light, c.xtraFX, true);
                lights.push_back(light);
            }
            //Fog
            SetupFogState(c.cloaked);

            //Render, iterating per light if/as requested
            bool popGlobals = false;
            size_t maxiter = 1;
            int maxlights = lights.size();
            if (pass.perLightIteration) {
                maxlights = pass.perLightIteration;
                maxiter = pass.maxIterations;
            }

            int npasslights = 0;
            for (size_t iter = 0, lightnum = 0, nlights = lights.size();
                    (iter < maxiter) && ((pass.perLightIteration == 0 && lightnum == 0) || (lightnum < nlights));
                    ++iter, lightnum += npasslights) {
                //Setup transform and lights
                npasslights = std::max(0, std::min(int(nlights) - int(lightnum), int(maxlights)));

                //MultiAlphaBlend stuff
                if (iter > 0) {
                    switch (pass.blendMode) {
                        case Pass::MultiAlphaBlend:
                            GFXBlendMode(SRCALPHA, ONE);
                            break;
                        case Pass::Default:
                            GFXBlendMode(blendSrc, ONE);
                            break;
                        default:
                            break;
                    }
                }

                GFXLoadMatrixModel(c.mat);
                if (lightnum == 0) {
                    GFXPushGlobalEffects();
                    popGlobals = true;
                }

                //Set shader parameters (instance-specific only)
                // NOTE: keep after GFXLoadMatrixModel
                GFXUploadLightState(
                        numLightsParam,
                        activeLightsArrayParam,
                        apparentLightSizeArrayParam,
                        true,
                        lights.begin() + lightnum,
                        lights.begin() + lightnum + npasslights);

                for (unsigned int spi = 0; spi < pass.getNumShaderParams(); ++spi) {
                    const Pass::ShaderParam &sp = pass.getShaderParam(spi);
                    if (sp.id >= 0) {
                        switch (sp.semantic) {
                            case Pass::ShaderParam::CloakingPhase:
                                GFXShaderConstant(sp.id,
                                        c.CloakFX.r,
                                        c.CloakFX.a,
                                        ((c.cloaked & MeshDrawContext::CLOAK) ? 1.f : 0.f),
                                        ((c.cloaked & MeshDrawContext::GLASSCLOAK) ? 1.f : 0.f));
                                break;
                            case Pass::ShaderParam::Damage:
                                GFXShaderConstant(sp.id, c.damage / 255.f);
                                break;
                            case Pass::ShaderParam::Damage4:
                                GFXShaderConstant(sp.id,
                                        c.damage / 255.f,
                                        c.damage / 255.f,
                                        c.damage / 255.f,
                                        c.damage / 255.f);
                                break;
                            case Pass::ShaderParam::EnvColor: //chuck_starchaser
                            case Pass::ShaderParam::DetailPlane0:
                            case Pass::ShaderParam::DetailPlane1:
                            case Pass::ShaderParam::NumLights:
                            case Pass::ShaderParam::ActiveLightsArray:
                            case Pass::ShaderParam::GameTime:
                            case Pass::ShaderParam::Constant:
                            default:
                                break;
                        }
                    }
                }
                vlist->Draw();
            }
            if (popGlobals) {
                GFXPopGlobalEffects();
            }
            for (; fxLightsBase < lights.size(); ++fxLightsBase) {
                GFXDeleteLight(lights[fxLightsBase]);
            }
            size_t lastPass = technique->getNumPasses();
            if (0 != forcelogos && whichpass == lastPass && !(c.cloaked & MeshDrawContext::NEARINVIS)) {
                forcelogos->Draw(c.mat);
            }
            if (0 != squadlogos && whichpass == lastPass && !(c.cloaked & MeshDrawContext::NEARINVIS)) {
                squadlogos->Draw(c.mat);
            }
        }
    }
    vlist->EndDrawState();

    //Restore state
    GFXEnable(CULLFACE);
    GFXEnable(COLORWRITE);
    GFXEnable(DEPTHWRITE);
    GFXCullFace(GFXBACK);
    GFXPolygonMode(GFXFILLMODE);
    GFXPolygonOffset(0, 0);
    GFXDepthFunc(LEQUAL);
    if (pass.polyMode == Pass::Line) {
        GFXLineWidth(1);
    }
    //Restore blend mode
    GFXPopBlendMode();
}

#define GETDECAL(pass) ( (Decal[pass]) )
#define HASDECAL(pass) ( ( (NUM_PASSES > pass) && Decal[pass] ) )
#define SAFEDECAL(pass) ( (HASDECAL( pass ) ? Decal[pass] : black) )

void Mesh::ProcessFixedDrawQueue(size_t techpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    const Pass &pass = technique->getPass(techpass);

    //First of all, decide zwrite, so we can skip the pass if !zwrite && !cwrite
    bool zwrite;
    if (whichdrawqueue == MESH_SPECIAL_FX_ONLY) {
        //Special effects pass - no zwrites... no zwrites...
        zwrite = false;
    } else {
        //Near draw queue - write to z-buffer if in auto mode and not translucent
        zwrite = (blendDst == ZERO);
        switch (pass.zWrite) {
            case Pass::Auto:
                break;
            case Pass::True:
                zwrite = true;
                break;
            case Pass::False:
                zwrite = false;
                break;
        }
    }
    //If we're not writing anything... why go on?
    if (!pass.colorWrite && !zwrite) {
        return;
    }

    //Map texture units
    Texture *Decal[NUM_PASSES];
    memset(Decal, 0, sizeof(Decal));
    for (unsigned int tui = 0; tui < pass.getNumTextureUnits(); ++tui) {
        const Pass::TextureUnit &tu = pass.getTextureUnit(tui);
        switch (tu.sourceType) {
            case Pass::TextureUnit::File:
                //Direct file sources go in tu.texture
                Decal[tu.targetIndex] = tu.texture.get();
                break;
            case Pass::TextureUnit::Decal:
                if ((tu.sourceIndex < static_cast<int>(this->Decal.size())) && this->Decal[tu.sourceIndex]) {
                    //Mesh has the referenced decal
                    Decal[tu.targetIndex] = this->Decal[tu.sourceIndex];
                } else {
                    //Mesh does not have the referenced decal, activate the default
                    switch (tu.defaultType) {
                        case Pass::TextureUnit::File:
                            //Direct file defaults go in tu.texture (direct file sources preclude defaults)
                            Decal[tu.targetIndex] = tu.texture.get();
                            break;
                        case Pass::TextureUnit::Decal:
                            //Decal reference as default - risky, but may be cool
                            if ((tu.defaultIndex < static_cast<int>(this->Decal.size()))
                                    && this->Decal[tu.defaultIndex]) {
                                //Valid reference, activate
                                Decal[tu.targetIndex] = this->Decal[tu.defaultIndex];
                            } else {
                                //Invalid reference, activate global default (null)
                                Decal[tu.targetIndex] = NULL;
                            }
                            break;
                        case Pass::TextureUnit::None: //chuck_starchaser
                        case Pass::TextureUnit::Environment:
                        case Pass::TextureUnit::Detail:
                        default:
                            break;
                    }
                }
                break;
            case Pass::TextureUnit::None: //chuck_starchaser
            case Pass::TextureUnit::Environment:
            case Pass::TextureUnit::Detail:
            default:
                break;
        }
    }
    std::vector<MeshDrawContext> &cur_draw_queue = draw_queue[whichdrawqueue];
    if (cur_draw_queue.empty()) {
        static bool thiserrdone =
                false;         //Avoid filling up logs with this thing (it would be output at least once per frame)
        if (!thiserrdone) {
            VS_LOG(error,
                    (boost::format(
                            "cloaking queues issue! Please report at https://github.com/vegastrike/Vega-Strike-Engine-Source\nn%1$d\n%2$s")
                            % whichdrawqueue
                            % hash_name.c_str()));
        }
        thiserrdone = true;
        return;
    }
    GFXDeactivateShader();

    size_t DecalSize = NUM_PASSES;
    while ((DecalSize > 0) && HASDECAL(DecalSize - 1)) {
        --DecalSize;
    }

    //Restore texture units
    for (unsigned int i = 2; i < gl_options.Multitexture; ++i) {
        GFXToggleTexture(false, i);
    }

    // Set up GL state from technique-specified values
    setupGLState(pass, zwrite, blendSrc, blendDst, myMatNum, alphatest, whichdrawqueue);
    if (pass.cullMode == Pass::DefaultFace) {
        SelectCullFace(whichdrawqueue);
    } // Default not handled by setupGLState, it depends on mesh data
    if (!getLighting()) {
        GFXDisable(LIGHTING);
        GFXColor4f(1, 1, 1, 1);
    }

    GFXEnable(TEXTURE0);
    if (alphatest) {
        GFXAlphaTest(GEQUAL, alphatest / 255.0);
    }
    static Texture *black = new Texture("blackclear.png");
    if (HASDECAL(BASE_TEX))
        GETDECAL(BASE_TEX)->MakeActive();
    GFXTextureEnv(0, GFXMODULATETEXTURE);     //Default diffuse mode
    GFXTextureEnv(1, GFXADDTEXTURE);     //Default envmap mode

    GFXToggleTexture((DecalSize > 0), 0);
    if (getEnvMap()) {
        GFXEnable(TEXTURE1);
        _Universe->activateLightMap();
    } else {
        GFXDisable(TEXTURE1);
    }
    const GFXMaterial &mat = GFXGetMaterial(myMatNum);
    static bool wantsplitpass1 =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "specmap_with_reflection", "false"));
    bool splitpass1 =
            (wantsplitpass1 && getEnvMap()
                    && ((mat.sr != 0) || (mat.sg != 0)
                            || (mat.sb != 0)))
                    || (getEnvMap() && !gl_options.Multitexture && (ENVSPEC_PASS < DecalSize)
                            && (!HASDECAL(ENVSPEC_TEX)));                                                                            //For now, no PPL reflections with no multitexturing - There is a way, however, doing it before the diffuse texture (odd, I know). If you see this, remind me to do it (Klauss).
    bool skipglowpass = false;
    bool nomultienv = false;
    int nomultienv_passno = 0;

    size_t whichpass = BASE_PASS;

    if (!gl_options.Multitexture && getEnvMap()) {
        if (HASDECAL(ENVSPEC_TEX)) {
            whichpass = ENVSPEC_PASS;
            nomultienv_passno = 0;
        } else {
            nomultienv_passno = 1;
        }
        nomultienv = true;
    }
    while ((nomultienv && (whichpass == ENVSPEC_PASS)) || !whichpass || (whichpass < DecalSize)) {
        if (!(nomultienv_passno >= 0 && nomultienv_passno <= 2)) {
            static int errcount = 0;
            errcount++;
            if (errcount < 100) {
                VS_LOG(error, (boost::format("Nomultienvpassno failure %1$s!\n") % hash_name.c_str()));
            }
            return;
        }
        if ((whichpass == GLOW_PASS) && skipglowpass) {
            whichpass++;
            continue;
        }
        if ((nomultienv && whichpass == ENVSPEC_PASS) || !whichpass || HASDECAL(whichpass)) {
            switch (whichpass) {
                case BASE_PASS:
                    SetupSpecMapFirstPass(Decal,
                            DecalSize,
                            myMatNum,
                            getEnvMap(),
                            polygon_offset,
                            detailTexture,
                            detailPlanes,
                            skipglowpass,
                            nomultienv
                                    && HASDECAL(ENVSPEC_TEX));
                    break;
                case ENVSPEC_PASS:
                    if (!nomultienv) {
                        SetupSpecMapSecondPass(SAFEDECAL(ENVSPEC_TEX),
                                myMatNum,
                                blendSrc,
                                (splitpass1 ? false : getEnvMap()),
                                (splitpass1 ? GFXColor(1, 1, 1, 1) : GFXColor(0, 0, 0, 0)),
                                polygon_offset);
                    } else {
                        SetupEnvmapPass(SAFEDECAL(ENVSPEC_TEX), myMatNum, nomultienv_passno);
                    }
                    break;
                case DAMAGE_PASS:
                    SetupDamageMapThirdPass(SAFEDECAL(DAMAGE_TEX), myMatNum, polygon_offset);
                    break;
                case GLOW_PASS:
                    SetupGlowMapFourthPass(SAFEDECAL(GLOW_TEX), myMatNum, ONE, GFXColor(1, 1, 1, 1), polygon_offset);
                    break;
            }
            //Render all instances, no specific order if not necessary
            //TODO: right now only meshes of same kind get drawn in correct order - should fix this.
            static std::vector<int> indices;
            if (zsort) {
                indices.resize(cur_draw_queue.size());
                for (int i = 0, n = cur_draw_queue.size(); i < n; ++i) {
                    indices[i] = i;
                }
                std::sort(indices.begin(), indices.end(),
                        MeshDrawContextPainterSort(sortctr, cur_draw_queue));
            }
            vlist->BeginDrawState();
            for (int i = 0, n = cur_draw_queue.size(); i < n; ++i) {
                //Making it static avoids frequent reallocations - although may be troublesome for thread safety
                //but... WTH... nothing is thread safe in VS.
                //Also: Be careful with reentrancy... right now, this section is not reentrant.
                static vector<int> specialfxlight;

                MeshDrawContext &c = cur_draw_queue[zsort ? indices[i] : i];
                if (c.mesh_seq != whichdrawqueue) {
                    continue;
                }
                if (c.damage == 0 && whichpass == DAMAGE_PASS) {
                    continue;
                }                      //No damage, so why draw it...
                if ((c.cloaked & MeshDrawContext::CLOAK) && whichpass != 0) {
                    continue;
                }                      //Cloaking, there are no multiple passes...
                if (whichdrawqueue != MESH_SPECIAL_FX_ONLY) {
                    GFXLoadIdentity(MODEL);
                    GFXPickLights(Vector(c.mat.p.i, c.mat.p.j, c.mat.p.k), rSize());
                }
                specialfxlight.clear();
                GFXLoadMatrixModel(c.mat);
                unsigned char damaged = ((whichpass == DAMAGE_PASS) ? c.damage : 0);
                SetupCloakState(c.cloaked, c.CloakFX, specialfxlight, damaged, myMatNum);
                for (unsigned int j = 0; j < c.SpecialFX->size(); j++) {
                    int ligh;
                    GFXCreateLight(ligh, (*c.SpecialFX)[j], true);
                    specialfxlight.push_back(ligh);
                }
                if (c.useXtraFX) {
                    int ligh;
                    GFXCreateLight(ligh, c.xtraFX, true);
                    specialfxlight.push_back(ligh);
                }
                SetupFogState(c.cloaked);
                if (c.cloaked & MeshDrawContext::RENORMALIZE) {
                    glEnable(GL_NORMALIZE);
                }
                vlist->Draw();
                if (c.cloaked & MeshDrawContext::RENORMALIZE) {
                    glDisable(GL_NORMALIZE);
                }
                for (unsigned int j = 0; j < specialfxlight.size(); j++) {
                    GFXDeleteLight(specialfxlight[j]);
                }
                RestoreCloakState(c.cloaked, getEnvMap(), damaged);
                if (0 != forcelogos && whichpass == BASE_PASS && !(c.cloaked & MeshDrawContext::NEARINVIS)) {
                    forcelogos->Draw(c.mat);
                }
                if (0 != squadlogos && whichpass == BASE_PASS && !(c.cloaked & MeshDrawContext::NEARINVIS)) {
                    squadlogos->Draw(c.mat);
                }
            }
            vlist->EndDrawState();
            switch (whichpass) {
                case BASE_PASS:
                    RestoreFirstPassState(detailTexture, detailPlanes, skipglowpass, nomultienv);
                    break;
                case ENVSPEC_PASS:
                    if (!nomultienv) {
                        RestoreSpecMapState((splitpass1 ? false : getEnvMap()),
                                detailTexture != NULL,
                                zwrite,
                                polygon_offset);
                    } else {
                        RestoreEnvmapState();
                    }
                    break;
                case DAMAGE_PASS:
                    RestoreDamageMapState(zwrite, polygon_offset);                 //nothin
                    break;
                case GLOW_PASS:
                    RestoreGlowMapState(zwrite, polygon_offset);
                    break;
            }
        }
        switch (whichpass) {
            case BASE_PASS:
                if (DecalSize > (whichpass = ((nomultienv && HASDECAL(ENVSPEC_TEX)) ? DAMAGE_PASS : ENVSPEC_PASS))) {
                    if ((nomultienv && whichpass == ENVSPEC_PASS) || HASDECAL(whichpass)) {
                        break;
                    }
                }
                break; //FIXME Nothing is done here! --chuck_starchaser
            case ENVSPEC_PASS:
                if (whichpass == ENVSPEC_PASS) {
                    //Might come from BASE_PASS and want to go to DAMAGE_PASS
                    if (!nomultienv) {
                        if (splitpass1) {
                            splitpass1 = false;
                            break;
                        } else if (DecalSize > (whichpass = DAMAGE_PASS)) {
                            if (HASDECAL(whichpass)) {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        nomultienv_passno++;
                        if (nomultienv_passno > (2 - ((splitpass1 || !HASDECAL(ENVSPEC_TEX)) ? 0 : 1))) {
                            whichpass = HASDECAL(ENVSPEC_TEX) ? BASE_PASS : GLOW_PASS;
                        }
                        break;
                    }
                }
            case DAMAGE_PASS:
                if (DecalSize > (whichpass = GLOW_PASS)) {
                    if (HASDECAL(whichpass)) {
                        break;
                    }
                }
                break; //FIXME Nothing is done here! --chuck_starchaser
            default:
                whichpass++;             //always increment pass number, otherwise infinite loop espresso
        }
    }
    if (alphatest) {
        GFXAlphaTest(ALWAYS,
                0);
    }       //Are you sure it was supposed to be after vlist->EndDrawState()? It makes more sense to put it here...
    if (!getLighting()) {
        GFXEnable(LIGHTING);
    }
    if (!zwrite) {
        GFXEnable(DEPTHWRITE);
    }       //risky--for instance logos might be fubar!
    RestoreCullFace(whichdrawqueue);
}

#undef SAFEDECAL
#undef GETDECAL
#undef HASDECAL

void Mesh::CreateLogos(MeshXML *xml, int faction, Flightgroup *fg) {
    numforcelogo = numsquadlogo = 0;
    unsigned int index;
    for (index = 0; index < xml->logos.size(); index++) {
        if (xml->logos[index].type == 0) {
            numforcelogo++;
        }
        if (xml->logos[index].type == 1) {
            numsquadlogo++;
        }
    }
    unsigned int nfl = numforcelogo;
    Logo **tmplogo = NULL;
    Texture *Dec = NULL;
    for (index = 0, nfl = numforcelogo, tmplogo = &forcelogos, Dec = FactionUtil::getForceLogo(faction);
            index < 2;
            index++, nfl = numsquadlogo, tmplogo = &squadlogos, Dec =
                    (fg == NULL ? FactionUtil::getSquadLogo(faction) : fg->squadLogo)) {
        if (Dec == NULL) {
            Dec = FactionUtil::getSquadLogo(faction);
        }
        if (nfl == 0) {
            continue;
        }
        Vector *PolyNormal = new Vector[nfl];
        Vector *center = new Vector[nfl];
        float *sizes = new float[nfl];
        float *rotations = new float[nfl];
        float *offset = new float[nfl];
        Vector *Ref = new Vector[nfl];
        Vector norm1, norm2, norm;
        int ri = 0;
        float totoffset = 0;
        for (unsigned int ind = 0; ind < xml->logos.size(); ind++) {
            if (xml->logos[ind].type == index) {
                float weight = 0;
                norm1.Set(0, 1, 0);
                norm2.Set(1, 0, 0);
                if (xml->logos[ind].refpnt.size() > 2) {
                    if (xml->logos[ind].refpnt[0] < static_cast<int>(xml->vertices.size())
                            && xml->logos[ind].refpnt[0] >= 0
                            && xml->logos[ind].refpnt[1] < static_cast<int>(xml->vertices.size())
                            && xml->logos[ind].refpnt[1] >= 0
                            && xml->logos[ind].refpnt[2] < static_cast<int>(xml->vertices.size())
                            && xml->logos[ind].refpnt[2] >= 0) {
                        norm2 = Vector(xml->vertices[xml->logos[ind].refpnt[1]].x
                                        - xml->vertices[xml->logos[ind].refpnt[0]].x,
                                xml->vertices[xml->logos[ind].refpnt[1]].y
                                        - xml->vertices[xml->logos[ind].refpnt[0]].y,
                                xml->vertices[xml->logos[ind].refpnt[1]].z
                                        - xml->vertices[xml->logos[ind].refpnt[0]].z);
                        norm1 = Vector(xml->vertices[xml->logos[ind].refpnt[2]].x
                                        - xml->vertices[xml->logos[ind].refpnt[0]].x,
                                xml->vertices[xml->logos[ind].refpnt[2]].y
                                        - xml->vertices[xml->logos[ind].refpnt[0]].y,
                                xml->vertices[xml->logos[ind].refpnt[2]].z
                                        - xml->vertices[xml->logos[ind].refpnt[0]].z);
                    }
                }
                CrossProduct(norm2, norm1, norm);

                Normalize(norm);                 //norm is our normal vect, norm1 is our reference vect
                Vector Cent(0, 0, 0);
                for (unsigned int rj = 0; rj < xml->logos[ind].refpnt.size(); rj++) {
                    weight += xml->logos[ind].refweight[rj];
                    Cent += Vector(xml->vertices[xml->logos[ind].refpnt[rj]].x * xml->logos[ind].refweight[rj],
                            xml->vertices[xml->logos[ind].refpnt[rj]].y * xml->logos[ind].refweight[rj],
                            xml->vertices[xml->logos[ind].refpnt[rj]].z * xml->logos[ind].refweight[rj]);
                }
                if (weight != 0) {
                    Cent.i /= weight;
                    Cent.j /= weight;
                    Cent.k /= weight;
                }
                //Cent.i-=x_center;
                //Cent.j-=y_center;
                //Cent.k-=z_center;
                Ref[ri] = norm2;
                PolyNormal[ri] = norm;
                center[ri] = Cent;
                sizes[ri] = xml->logos[ind].size * xml->scale.k;
                rotations[ri] = xml->logos[ind].rotate;
                offset[ri] = xml->logos[ind].offset;
                totoffset += offset[ri];
                ri++;
            }
        }
        totoffset /= nfl;
        *tmplogo = new Logo(nfl, center, PolyNormal, sizes, rotations, totoffset, Dec, Ref);
        delete[] Ref;
        delete[] PolyNormal;
        delete[] center;
        delete[] sizes;
        delete[] rotations;
        delete[] offset;
    }
}

void Mesh::initTechnique(const std::string &xmltechnique) {
    if (xmltechnique.empty()) {
        //Load default technique, which depends:
        string effective;
        if (Decal.size() > 1 || getEnvMap()) {
            //Use shader-ified technique for multitexture or environment-mapped meshes
#if defined(__APPLE__) && defined(__MACH__)
            static string shader_technique = vs_config->getVariable( "graphics", "default_full_technique", "mac" );
#else
            static string shader_technique = vs_config->getVariable("graphics", "default_full_technique", "default");
#endif
            effective = shader_technique;
        } else {
            static string
                    fixed_technique = vs_config->getVariable("graphics", "default_simple_technique", "fixed_simple");
            effective = fixed_technique;
        }
        technique = Technique::getTechnique(effective);
    } else {
        technique = Technique::getTechnique(xmltechnique);
    }
}
