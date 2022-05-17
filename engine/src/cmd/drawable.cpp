/*
 * drawable.cpp
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy and other
 * Vega Strike contributors
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

//#include <cassert>

#include "drawable.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "gfx/mesh.h"
#include "gfx/quaternion.h"
#include "gfx/lerp.h"
#include "gfx/occlusion.h"

#include "unit_generic.h"
#include "vs_globals.h"
#include "gfx/point_to_cam.h"
#include "gfx/halo_system.h"
#include "options.h"
#include "weapon_info.h"
#include "beam.h"
#include "csv.h"
#include "unit_csv.h"
#include "universe_util.h"

#include <boost/algorithm/string.hpp>
#include "vega_cast_utils.hpp"

// Dupe to same function in unit.cpp
// TODO: remove duplication
inline static float perspectiveFactor(float d) {
    if (d > 0) {
        return g_game.x_resolution * GFXGetZPerspective(d);
    } else {
        return 1.0f;
    }
}

Drawable::Drawable() :
        halos(new HaloSystem()),
        animatedMesh(true),
        activeAnimation(0),
        timeperframe(3.0),
        done(true),
        activeMesh(0),
        nextactiveMesh(1),
        infiniteLoop(true),
        loopCount(0),
        curtime(0.0) {
}

Drawable::~Drawable() {
    for (Mesh *mesh : meshdata) {
        if (mesh != nullptr) {
            delete mesh;
            mesh = nullptr;
        }
    }
    meshdata.clear();
    clear();
}

bool Drawable::DrawableInit(const char *filename, int faction,
        Flightgroup *flightgrp, const char *animationExt) {
    string fnam(filename);
    string::size_type pos = fnam.find('.');
    string anifilename = fnam.substr(0, pos);

    if (animationExt) {
        anifilename += string("_") + string(animationExt);
    }

    std::vector<Mesh *> *meshes = new vector<Mesh *>();
    int i = 1;
    char count[30] = "1";
    string dir = anifilename;
    while (true) {
        sprintf(count, "%d", i);
        string unit_name = anifilename;
        boost::algorithm::to_lower(unit_name); //toLowerCase(anifilename) + "_";

        if (i < 10) {
            unit_name += "0";
        }
        if (i < 100) {
            unit_name += "0";
        }
        if (i < 1000) {
            unit_name += "0";
        }
        if (i < 10000) {
            unit_name += "0";
        }
        if (i < 100000) {
            unit_name += "0";
        }

        unit_name += count;
        string path = dir + "/" + unit_name + ".bfxm";
        if (VSFileSystem::FileExistsData(path, VSFileSystem::MeshFile) != -1) {
            Mesh *m = Mesh::LoadMesh(path.c_str(), Vector(1, 1, 1), faction, flightgrp);
            meshes->push_back(m);
            #ifdef DEBUG_MESH_ANI
            VS_LOG(debug, (boost::format("Animated Mesh: %1% loaded - with: %2% vertices.") % path % m->getVertexList()->GetNumVertices()));
            #endif
        } else {
            break;
        }
        i++;
    }

    if (meshes->size() != 0) {
        //FIXME: an animation is created only for the first submesh
        string animationName;
        sprintf(count, "%lu", (unsigned long) meshes->size());
        if (!animationExt) {
            animationName =
                    string(count); //if there is no extension given, the animations are called by their load order, 1, 2 ,3 ....10..
        } else {
            animationName = animationExt;
        }
        addAnimation(meshes, animationName.c_str());

        int numFrames = meshes->size();
        ++Drawable::unitCount;
        sprintf(count, "%u", unitCount);
        uniqueUnitName = drawableGetName() + string(count);
        Units[uniqueUnitName] = vega_dynamic_cast_ptr<Unit>(this);
        VS_LOG(info,
                (boost::format("Animation data loaded for unit: %1%, named %2% - with: %3% frames.") % string(filename)
                        % uniqueUnitName % numFrames));
        return true;
    } else {
        delete meshes;
        return false;
    }
}

extern double interpolation_blend_factor;
extern double saved_interpolation_blend_factor;
extern bool cam_setup_phase;
extern int cloakVal(int cloakint, int cloakminint, int cloakrateint, bool cloakglass); //short fix?
extern double calc_blend_factor(double frac,
        unsigned int priority,
        unsigned int when_it_will_be_simulated,
        unsigned int cur_simulation_frame);

void Drawable::Draw(const Transformation &parent, const Matrix &parentMatrix) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    //Quick shortcut for camera setup phase
    bool myparent = (unit == _Universe->AccessCockpit()->GetParent());

    Matrix *ctm;
    Matrix invview;
    Transformation *ct;

    unit->cumulative_transformation = linear_interpolate(unit->prev_physical_state,
            unit->curr_physical_state,
            interpolation_blend_factor);
    unit->cumulative_transformation.Compose(parent, parentMatrix);
    unit->cumulative_transformation.to_matrix(unit->cumulative_transformation_matrix);

    ctm = GetCumulativeTransformationMatrix(unit, parentMatrix, invview);
    ct = &unit->cumulative_transformation;

#ifdef PERFRAMESOUND
    AUDAdjustSound( sound.engine, cumulative_transformation.position, GetVelocity() );
#endif

    unsigned int i, n;
    if ((unit->Destroyed()) && (!cam_setup_phase)) {
        unit->Explode(true, GetElapsedTime());
    }

    float damagelevel = 1.0f;
    unsigned char chardamage = 0;

    // We might need to scale rSize, this "average scale" takes the transform matrix into account
    float avgscale = 1.0f;

    bool On_Screen = false;
    bool Unit_On_Screen = false;
    float Apparent_Size = 0.0f;
    int cloak = unit->cloaking;
    Matrix wmat;

    if (!cam_setup_phase) {
        // Following stuff is only needed in actual drawing phase
        if (unit->cloaking > unit->cloakmin) {
            cloak = (int) (unit->cloaking - interpolation_blend_factor * unit->cloakrate * simulation_atom_var);
            cloak = cloakVal(cloak, unit->cloakmin, unit->cloakrate, unit->cloakglass);
        }
        if ((*unit->current_hull) < (*unit->max_hull)) {
            damagelevel = (*unit->current_hull) / (*unit->max_hull);
            chardamage = (255 - (unsigned char) (damagelevel * 255));
        }
        avgscale = sqrt((ctm->getP().MagnitudeSquared() + ctm->getR().MagnitudeSquared()) * 0.5);
        wmat = unit->WarpMatrix(*ctm);
    }

    if ((!(unit->invisible & unit->INVISUNIT)) && ((!(unit->invisible & unit->INVISCAMERA)) || (!myparent))) {
        if (!cam_setup_phase) {
            Camera *camera = _Universe->AccessCamera();
            QVector camerapos = camera->GetPosition();

            float minmeshradius =
                    (camera->GetVelocity().Magnitude() + unit->Velocity.Magnitude()) * simulation_atom_var;

            unsigned int numKeyFrames = unit->graphicOptions.NumAnimationPoints;
            for (i = 0, n = nummesh(); i <= n; i++) {
                //NOTE LESS THAN OR EQUALS...to cover shield mesh
                if (this->meshdata[i] == NULL) {
                    continue;
                }
                if (i == n && (this->meshdata[i]->numFX() == 0 || unit->Destroyed())) {
                    continue;
                }

                if (this->meshdata[i]->getBlendDst() == ONE) {
                    if ((unit->invisible & unit->INVISGLOW) != 0) {
                        continue;
                    }
                    if (damagelevel < .9) {
                        if (unit->flickerDamage()) {
                            continue;
                        }
                    }
                }
                QVector TransformedPosition = Transform(*ctm, meshdata[i]->Position().Cast());

                //d can be used for level of detail shit
                float mSize = meshdata[i]->rSize() * avgscale;
                double d = (TransformedPosition - camerapos).Magnitude();
                double rd = d - mSize;
                float pixradius = Apparent_Size = mSize * perspectiveFactor(
                        (rd < g_game.znear) ? g_game.znear : rd);
                float lod = pixradius * g_game.detaillevel;
                if (meshdata[i]->getBlendDst() == ZERO) {
                    if (unit->isUnit() == _UnitType::planet && pixradius > 10) {
                        Occlusion::addOccluder(TransformedPosition, mSize, true);
                    } else if (pixradius >= 10.0) {
                        Occlusion::addOccluder(TransformedPosition, mSize, false);
                    }
                }
                if (lod >= 0.5 && pixradius >= 2.5) {
                    double frustd = GFXSphereInFrustum(
                            TransformedPosition,
                            minmeshradius + mSize);
                    if (frustd) {
                        //if the radius is at least half a pixel at detail 1 (equivalent to pixradius >= 0.5 / detail)
                        float currentFrame = meshdata[i]->getCurrentFrame();
                        this->meshdata[i]->Draw(lod, wmat, d,
                                i == this->meshdata.size() - 1 ? -1 : cloak,
                                (camera->GetNebula() == unit->nebula && unit->nebula != NULL) ? -1 : 0,
                                chardamage);                                                                                                                                                            //cloakign and nebula
                        On_Screen = true;
                        unsigned int numAnimFrames = 0;
                        static const string default_animation;
                        if (this->meshdata[i]->getFramesPerSecond()
                                && (numAnimFrames = this->meshdata[i]->getNumAnimationFrames(default_animation))) {
                            float currentprogress = floor(
                                    this->meshdata[i]->getCurrentFrame() * numKeyFrames / (float) numAnimFrames);
                            if (numKeyFrames
                                    && floor(currentFrame * numKeyFrames / (float) numAnimFrames) != currentprogress) {
                                unit->graphicOptions.Animating = 0;
                                meshdata[i]->setCurrentFrame(
                                        .1 + currentprogress * numAnimFrames / (float) numKeyFrames);
                            } else if (!unit->graphicOptions.Animating) {
                                meshdata[i]->setCurrentFrame(currentFrame);                                 //dont' budge
                            }
                        }
                    }
                }
            }

            Unit_On_Screen = On_Screen || !!GFXSphereInFrustum(
                    ct->position,
                    minmeshradius + unit->rSize());
        } else {
            Unit_On_Screen = true;
        }
        if (Unit_On_Screen && unit->hasSubUnits()) {
            Unit *un;
            double backup = interpolation_blend_factor;
            int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
            for (un_iter iter = unit->getSubUnits(); (un = *iter); ++iter) {
                float sim_atom_backup = simulation_atom_var;
                //if (sim_atom_backup != SIMULATION_ATOM) {
                //    VS_LOG(debug, (boost::format("void GameUnit::Draw( const Transformation &parent, const Matrix &parentMatrix ): sim_atom as backed up != SIMULATION_ATOM: %1%") % sim_atom_backup));
                //}
                if (unit->sim_atom_multiplier && un->sim_atom_multiplier) {
                    //VS_LOG(trace, (boost::format("void GameUnit::Draw( const Transformation &parent, const Matrix &parentMatrix ): simulation_atom_var as backed up  = %1%") % simulation_atom_var));
                    simulation_atom_var = simulation_atom_var * un->sim_atom_multiplier / unit->sim_atom_multiplier;
                    //VS_LOG(trace, (boost::format("void GameUnit::Draw( const Transformation &parent, const Matrix &parentMatrix ): simulation_atom_var as multiplied = %1%") % simulation_atom_var));
                }
                interpolation_blend_factor = calc_blend_factor(saved_interpolation_blend_factor,
                        un->sim_atom_multiplier,
                        un->cur_sim_queue_slot,
                        cur_sim_frame);
                (un)->Draw(*ct, *ctm);

                simulation_atom_var = sim_atom_backup;
            }
            interpolation_blend_factor = backup;
        }
    } else {
        _Universe->AccessCockpit()->SetupViewPort();         ///this is the final, smoothly calculated cam
        //UpdateHudMatrix();
    }
    /***DEBUGGING cosAngleFromMountTo
     *  UnitCollection *dL = _Universe->activeStarSystem()->getUnitList();
     *  UnitCollection::UnitIterator *tmpiter = dL->createIterator();
     *  GameUnit<UnitType> * curun;
     *  while (curun = tmpiter->current()) {
     *  if (curun->selected) {
     *   float tmpdis;
     *   float tmpf = cosAngleFromMountTo (curun, tmpdis);
     *   VSFileSystem::vs_fprintf (stderr,"%s: <%f d: %f\n", curun->name.c_str(), tmpf, tmpdis);
     *
     *  }
     *  tmpiter->advance();
     *  }
     *  delete tmpiter;
     **/
    if (cam_setup_phase) {
        return;
    }

    DrawSubunits(On_Screen, wmat, cloak, avgscale, chardamage);
    DrawHalo(On_Screen, Apparent_Size, wmat, cloak);
    Sparkle(On_Screen, ctm);
}

void Drawable::AnimationStep() {
#ifdef DEBUG_MESH_ANI
    VS_LOG(debug, (boost::format("Starting animation step of Unit: %1%") % uniqueUnitName));
#endif
    if ((!this->isContinuousLoop()) && (loopCount == 0)) {
        return;
    }
    //copy reference to data
    meshdata.at(0) = vecAnimations.at(activeAnimation)->at(activeMesh);

    Draw();

#ifdef DEBUG_MESH_ANI
    VS_LOG(debug, (boost::format("Drawed mesh: %1%") % uniqueUnitName));
#endif

    activeMesh = nextactiveMesh;
    ++nextactiveMesh;
    if (nextactiveMesh >= vecAnimations.at(activeAnimation)->size()) {
        nextactiveMesh = 0;
    }

    if (loopCount > 0) {
        --loopCount;
    }
#ifdef DEBUG_MESH_ANI
    VS_LOG(debug, (boost::format("Ending animation step of Unit: %1%") % uniqueUnitName));
#endif
}

void Drawable::DrawNow(const Matrix &mato, float lod) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    static const void *rootunit = NULL;
    if (rootunit == NULL) {
        rootunit = (const void *) this;
    }
    float damagelevel = 1.0;
    unsigned char chardamage = 0;
    if (*unit->current_hull < *unit->max_hull) {
        damagelevel = (*unit->current_hull) / (*unit->max_hull);
        chardamage = (255 - (unsigned char) (damagelevel * 255));
    }
#ifdef VARIABLE_LENGTH_PQR
    const float  vlpqrScaleFactor = SizeScaleFactor;
#else
    const float vlpqrScaleFactor = 1.f;
#endif
    unsigned int i;
    Matrix mat(mato);
    if (unit->graphicOptions.FaceCamera) {
        Vector p, q, r;
        QVector pos(mato.p);
        float wid, hei;
        CalculateOrientation(pos, p, q, r, wid, hei, 0, false, &mat);
        pos = mato.p;
        VectorAndPositionToMatrix(mat, p, q, r, pos);
    }
    int cloak = unit->cloaking;
    if (unit->cloaking > unit->cloakmin) {
        cloak = cloakVal(cloak, unit->cloakmin, unit->cloakrate, unit->cloakglass);
    }
    for (i = 0; i <= this->nummesh(); i++) {
        //NOTE LESS THAN OR EQUALS...to cover shield mesh
        if (this->meshdata[i] == NULL) {
            continue;
        }
        QVector TransformedPosition = Transform(mat,
                this->meshdata[i]->Position().Cast());
        float d = GFXSphereInFrustum(TransformedPosition, this->meshdata[i]->clipRadialSize() * vlpqrScaleFactor);
        if (d) {          //d can be used for level of detail
            //this->meshdata[i]->DrawNow(lod,false,mat,cloak);//cloakign and nebula
            this->meshdata[i]->Draw(lod, mat, d, cloak);
        }
    }
    Unit *un;
    /*for (un_iter iter = this->getSubUnits(); (un = *iter); ++iter) {
        Matrix temp;
        un->curr_physical_state.to_matrix( temp );
        Matrix submat;
        MultMatrix( submat, mat, temp );
        (un)->DrawNow( submat, lod );*/
    if (unit->hasSubUnits()) {
        for (un_iter iter = unit->getSubUnits(); (un = *iter); ++iter) {
            Matrix temp;
            un->curr_physical_state.to_matrix(temp);
            Matrix submat;
            MultMatrix(submat, mat, temp);
            (un)->DrawNow(submat, lod);
        }
    }
    float cmas = unit->computer.max_ab_speed() * unit->computer.max_ab_speed();
    if (cmas == 0) {
        cmas = 1;
    }
    Vector Scale(1, 1, 1);         //Now, HaloSystem handles that
    int nummounts = unit->getNumMounts();
    Matrix wmat = WarpMatrix(mat);
    for (i = 0; (int) i < nummounts; i++) {
        Mount *mahnt = &unit->mounts[i];
        if (game_options()->draw_weapons) {
            if (mahnt->xyscale != 0 && mahnt->zscale != 0) {
                Mesh *gun = mahnt->type->gun;
                if (gun && mahnt->status != Mount::UNCHOSEN) {
                    Transformation mountLocation(mahnt->GetMountOrientation(), mahnt->GetMountLocation().Cast());
                    mountLocation.Compose(Transformation::from_matrix(mat), wmat);
                    Matrix mmat;
                    mountLocation.to_matrix(mmat);
                    if (GFXSphereInFrustum(mountLocation.position, gun->rSize() * vlpqrScaleFactor) > 0) {
                        float d = (mountLocation.position - _Universe->AccessCamera()->GetPosition()).Magnitude();
                        float lod = gun->rSize() * g_game.detaillevel * perspectiveFactor(
                                (d - gun->rSize() < g_game.znear) ? g_game.znear : d - gun->rSize());
                        ScaleMatrix(mmat, Vector(mahnt->xyscale, mahnt->xyscale, mahnt->zscale));
                        gun->setCurrentFrame(unit->mounts[i].ComputeAnimatedFrame(gun));
                        gun->Draw(lod, mmat, d, cloak,
                                (_Universe->AccessCamera()->GetNebula() == unit->nebula && unit->nebula != NULL) ? -1
                                        : 0,
                                chardamage,
                                true);                                                                                                                                       //cloakign and nebula
                        if (mahnt->type->gun1) {
                            gun = mahnt->type->gun1;
                            gun->setCurrentFrame(unit->mounts[i].ComputeAnimatedFrame(gun));
                            gun->Draw(lod,
                                    mmat,
                                    d,
                                    cloak,
                                    (_Universe->AccessCamera()->GetNebula() == unit->nebula && unit->nebula
                                            != NULL) ? -1 : 0,
                                    chardamage,
                                    true);                                                                                                                               //cloakign and nebula
                        }
                    }
                }
            }
        }
    }
    Vector linaccel = unit->GetAcceleration();
    Vector angaccel = unit->GetAngularAcceleration();
    float maxaccel = unit->GetMaxAccelerationInDirectionOf(mat.getR(), true);
    Vector velocity = unit->GetVelocity();
    if (!(unit->docked & (unit->DOCKED | unit->DOCKED_INSIDE))) {
        halos->Draw(mat,
                Scale,
                cloak,
                0,
                unit->GetHullPercent(),
                velocity,
                linaccel,
                angaccel,
                maxaccel,
                cmas,
                unit->faction);
    }
    if (rootunit == (const void *) this) {
        Mesh::ProcessZFarMeshes();
        Mesh::ProcessUndrawnMeshes();
        rootunit = NULL;
    }
}

void Drawable::UpdateFrames() {
    std::map<string, Unit *>::iterator pos;
    for (pos = Units.begin(); pos != Units.end(); ++pos) {
        pos->second->curtime += GetElapsedTime();
        if (pos->second->curtime >= pos->second->timePerFrame()) {
            pos->second->AnimationStep();
            pos->second->curtime = 0.0;
        }
    }
}

void Drawable::addAnimation(std::vector<Mesh *> *meshes, const char *name) {
    if ((meshes->size() > 0) && animatedMesh) {
        vecAnimations.push_back(meshes);
        vecAnimationNames.push_back(string(name));
    }
}

void Drawable::StartAnimation(unsigned int how_many_times, int numAnimation) {
    if (animationRuns()) {
        StopAnimation();
    }

    done = false;
}

void Drawable::StopAnimation() {
    done = true;
}

string Drawable::getAnimationName(unsigned int animationNumber) const {
    return vecAnimationNames.at(animationNumber);
}

unsigned int Drawable::getAnimationNumber(const char *name) const {
    string strname(name);
    for (unsigned int i = 0; i < vecAnimationNames.size(); i++) {
        if (strname == vecAnimationNames[i]) {
            return i;
        }
    }

    return 0; //NOT FOUND!
}

void Drawable::ChangeAnimation(const char *name) {
    unsigned int AnimNumber = getAnimationNumber(name);
    if ((AnimNumber < numAnimations()) && isAnimatedMesh()) {
        activeAnimation = AnimNumber;
    }
}

void Drawable::ChangeAnimation(unsigned int AnimNumber) {
    if ((AnimNumber < numAnimations()) && isAnimatedMesh()) {
        activeAnimation = AnimNumber;
    }
}

bool Drawable::isAnimatedMesh() const {
    return animatedMesh;
}

double Drawable::framesPerSecond() const {
    return 1 / timeperframe;
}

double Drawable::timePerFrame() const {
    return timeperframe;
}

unsigned int Drawable::numAnimations() {
    return vecAnimations.size();
}

void Drawable::ToggleAnimatedMesh(bool on) {
    animatedMesh = on;
}

bool Drawable::isContinuousLoop() const {
    return infiniteLoop;
}

void Drawable::SetAniSpeed(float speed) {
    timeperframe = speed;
}

void Drawable::clear() {
    StopAnimation();

    for (unsigned int i = 0; i < vecAnimations.size(); i++) {
        for (unsigned int j = 0; j < vecAnimations[i]->size(); j++) {
            delete vecAnimations[i]->at(j);
        }
        delete vecAnimations[i];
        vecAnimations[i]->clear();
    }
    vecAnimations.clear();
    vecAnimationNames.clear();

    Units.erase(uniqueUnitName);
}

bool Drawable::animationRuns() const {
    return !done;
}

Matrix *GetCumulativeTransformationMatrix(Unit *unit, const Matrix &parentMatrix, Matrix invview) {
    Matrix *ctm = &unit->cumulative_transformation_matrix;

    if (unit->graphicOptions.FaceCamera == 1) {
        Vector p, q, r;
        QVector pos(ctm->p);
        float wid, hei;
        float magr = parentMatrix.getR().Magnitude();
        float magp = parentMatrix.getP().Magnitude();
        float magq = parentMatrix.getQ().Magnitude();
        CalculateOrientation(pos, p, q, r, wid, hei, 0, false, ctm);
        VectorAndPositionToMatrix(invview, p * magp, q * magq, r * magr, ctm->p);
        ctm = &invview;
    }

    return ctm;
}

/**
 * @brief Drawable::Sparkle caused damaged units to emit sparks
 */
void Drawable::Sparkle(bool on_screen, Matrix *ctm) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    const Vector velocity = unit->GetVelocity();

    // Docked units don't sparkle
    // Move to a separate isDocked() function
    if (unit->docked & (unit->DOCKED | unit->DOCKED_INSIDE)) {
        return;
    }

    // Units not shown don't sparkle
    if (!on_screen) {
        return;
    }

    // Obviously, don't sparkle if the option isn't set
    if (unit->graphicOptions.NoDamageParticles) {
        return;
    }

    // Destroyed units (dying?) don't sparkle
    if (unit->GetHull() <= 0) {
        return;
    }

    // Units with no meshes, don't sparkle
    if (unit->nummesh() <= 0) {
        return;
    }

    // Undamaged units don't sparkle
    float damage_level = (*unit->current_hull) / (*unit->max_hull);
    if (damage_level >= .99) {
        return;
    }

    double sparkle_accum = GetElapsedTime() * game_options()->sparklerate;
    int spawn = (int) (sparkle_accum);
    sparkle_accum -= spawn;


    // Pretty sure the following is the equivalent of the commented code
    // unsigned int switcher = (damagelevel > .8) ? 1
    //: (damagelevel > .6) ? 2 : (damagelevel > .4) ? 3 : (damagelevel > .2) ? 4 : 5;
    unsigned int switcher = 5 * (1 - damage_level);

    long seed = (long) this;

    while (spawn-- > 0) {
        switch (switcher) {
            case 5:
                seed += 165;
            case 4:
                seed += 47;
            case 3:
                seed += 61;
            case 2:
                seed += 65537;
            default:
                seed += 257;
        }

        LaunchOneParticle(*ctm, velocity, seed, unit, damage_level, unit->faction);
    }
}

void Drawable::DrawHalo(bool on_screen, float apparent_size, Matrix wmat, int cloak) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    // Units not shown don't emit a halo
    if (!on_screen) {
        return;
    }

    // Units with no halo
    if (halos->NumHalos() == 0) {
        return;
    }

    // Docked units
    if (unit->docked & (unit->DOCKED | unit->DOCKED_INSIDE)) {
        return;
    }

    // Small units
    if (apparent_size <= 5.0f) {
        return;
    }

    Vector linaccel = unit->GetAcceleration();
    Vector angaccel = unit->GetAngularAcceleration();
    float maxaccel = unit->GetMaxAccelerationInDirectionOf(wmat.getR(), true);
    Vector velocity = unit->GetVelocity();

    float cmas = unit->computer.max_ab_speed() * unit->computer.max_ab_speed();
    if (cmas == 0) {
        cmas = 1;
    }

    Vector Scale(1, 1, 1);         //Now, HaloSystem handles that
    float damage_level = (*unit->current_hull) / (*unit->max_hull);
    //WARNING: cmas is not a valid maximum speed for the upcoming multi-direction thrusters,
    //nor is maxaccel. Instead, each halo should have its own limits specified in units.csv
    float nebd = (_Universe->AccessCamera()->GetNebula() == unit->nebula && unit->nebula != nullptr) ? -1 : 0;
    float hulld = unit->GetHull() > 0 ? damage_level : 1.0;
    halos->Draw(wmat, Scale, cloak, nebd, hulld, velocity,
            linaccel, angaccel, maxaccel, cmas, unit->faction);

}

void Drawable::DrawSubunits(bool on_screen, Matrix wmat, int cloak, float average_scale, unsigned char char_damage) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    Transformation *ct = &unit->cumulative_transformation;

    for (int i = 0; (int) i < unit->getNumMounts(); i++) {
        Mount *mount = &unit->mounts[i];
        Mesh *gun = mount->type->gun;

        // Has to come before check for on screen, as a fired beam can still be seen
        // even if the subunit firing it isn't on screen.
        if (unit->mounts[i].type->type == WEAPON_TYPE::BEAM) {
            // If gun is null (?)
            if (unit->mounts[i].ref.gun) {
                unit->mounts[i].ref.gun->Draw(*ct, wmat,
                        (isAutoTrackingMount(unit->mounts[i].size)
                                && (unit->mounts[i].time_to_lock <= 0)
                                && unit->TargetTracked()) ? unit->Target() : NULL,
                        unit->computer.radar.trackingcone);
            }
        }

        // Don't try to draw a nonexistent object -- stephengtuggy 2021-10-10
        if (gun == nullptr) {
            continue;
        }

        // Units not shown don't draw subunits
        if (!on_screen) {
            continue;
        }

        // Don't draw mounts if game is set not to...
        if (!game_options()->draw_weapons) {
            continue;
        }


        // Don't bother drawing small mounts ?!
        if (mount->xyscale == 0 || mount->zscale == 0) {
            continue;
        }

        // Don't draw unchosen GUN mounts, whatever that means
        // Does not cover beams for some reason.
        if (mount->status == Mount::UNCHOSEN) {
            continue;
        }

        Transformation mountLocation(mount->GetMountOrientation(), mount->GetMountLocation().Cast());
        mountLocation.Compose(*ct, wmat);
        Matrix mat;
        mountLocation.to_matrix(mat);
        if (GFXSphereInFrustum(mountLocation.position, gun->rSize() * average_scale) > 0) {
            float d = (mountLocation.position - _Universe->AccessCamera()->GetPosition()).Magnitude();
            float pixradius = gun->rSize() * perspectiveFactor(
                    (d - gun->rSize() < g_game.znear) ? g_game.znear : d - gun->rSize());
            float lod = pixradius * g_game.detaillevel;
            if (lod > 0.5 && pixradius > 2.5) {
                ScaleMatrix(mat, Vector(mount->xyscale, mount->xyscale, mount->zscale));
                gun->setCurrentFrame(unit->mounts[i].ComputeAnimatedFrame(gun));
                gun->Draw(lod, mat, d, cloak,
                        (_Universe->AccessCamera()->GetNebula() == unit->nebula && unit->nebula != NULL) ? -1 : 0,
                        char_damage,
                        true);                                                                                                                                      //cloakign and nebula
            }
            if (mount->type->gun1) {
                pixradius = gun->rSize() * perspectiveFactor(
                        (d - gun->rSize() < g_game.znear) ? g_game.znear : d - gun->rSize());
                lod = pixradius * g_game.detaillevel;
                if (lod > 0.5 && pixradius > 2.5) {
                    gun = mount->type->gun1;
                    gun->setCurrentFrame(unit->mounts[i].ComputeAnimatedFrame(gun));
                    gun->Draw(lod,
                            mat,
                            d,
                            cloak,
                            (_Universe->AccessCamera()->GetNebula() == unit->nebula && unit->nebula
                                    != NULL) ? -1 : 0,
                            char_damage,
                            true);                                                                                                                              //cloakign and nebula
                }
            }
        }
    }
}

void Drawable::Split(int level) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    if (game_options()->split_dead_subunits) {
        for (un_iter su = unit->getSubUnits(); *su; ++su) {
            (*su)->Split(level);
        }
    }
    Vector PlaneNorm;
    for (unsigned int i = 0; i < nummesh();) {
        if (this->meshdata[i]) {
            if (this->meshdata[i]->getBlendDst() == ONE) {
                delete this->meshdata[i];
                this->meshdata.erase(this->meshdata.begin() + i);
            } else {
                i++;
            }
        } else {
            this->meshdata.erase(this->meshdata.begin() + i);
        }
    }
    int nm = this->nummesh();
    string fac = FactionUtil::GetFaction(unit->faction);

    CSVRow unit_stats(LookupUnitRow(unit->name, fac));
    unsigned int num_chunks = unit_stats.success() ? atoi(unit_stats["Num_Chunks"].c_str()) : 0;
    if (nm <= 0 && num_chunks == 0) {
        return;
    }
    vector<Mesh *> old = this->meshdata;
    Mesh *shield = old.back();
    old.pop_back();

    vector<unsigned int> meshsizes;
    if (num_chunks && unit_stats.success()) {
        size_t i;
        vector<Mesh *> nw;
        unsigned int which_chunk = rand() % num_chunks;
        string chunkname = UniverseUtil::LookupUnitStat(unit->name, fac, "Chunk_" + XMLSupport::tostring(which_chunk));
        string dir = UniverseUtil::LookupUnitStat(unit->name, fac, "Directory");
        VSFileSystem::current_path.push_back(unit_stats.getRoot());
        VSFileSystem::current_subdirectory.push_back("/" + dir);
        VSFileSystem::current_type.push_back(VSFileSystem::UnitFile);
        float randomstartframe = 0;
        float randomstartseconds = 0;
        string scalestr = UniverseUtil::LookupUnitStat(unit->name, fac, "Unit_Scale");
        int scale = atoi(scalestr.c_str());
        if (scale == 0) {
            scale = 1;
        }
        AddMeshes(nw, randomstartframe, randomstartseconds, scale, chunkname, unit->faction,
                unit->getFlightgroup(), &meshsizes);
        VSFileSystem::current_type.pop_back();
        VSFileSystem::current_subdirectory.pop_back();
        VSFileSystem::current_path.pop_back();
        for (i = 0; i < old.size(); ++i) {
            delete old[i];
        }
        old.clear();
        old = nw;
    } else {
        for (int split = 0; split < level; split++) {
            vector<Mesh *> nw;
            size_t oldsize = old.size();
            for (size_t i = 0; i < oldsize; i++) {
                PlaneNorm.Set(rand() - RAND_MAX / 2, rand() - RAND_MAX / 2, rand() - RAND_MAX / 2 + .5);
                PlaneNorm.Normalize();
                nw.push_back(NULL);
                nw.push_back(NULL);
                old[i]->Fork(nw[nw.size() - 2], nw.back(), PlaneNorm.i, PlaneNorm.j, PlaneNorm.k,
                        -PlaneNorm.Dot(old[i]->Position()));                                                                              //splits somehow right down the middle.
                delete old[i];
                old[i] = NULL;
                if (nw[nw.size() - 2] == NULL) {
                    nw[nw.size() - 2] = nw.back();
                    nw.pop_back();
                }
                if (nw.back() == NULL) {
                    nw.pop_back();
                }
            }
            old = nw;
        }
        meshsizes.reserve(old.size());
        for (size_t i = 0; i < old.size(); ++i) {
            meshsizes.push_back(1);
        }
    }
    old.push_back(NULL);     //push back shield
    if (shield != nullptr) {
        delete shield;
        shield = nullptr;
    }
    nm = old.size() - 1;
    unsigned int k = 0;
    vector<Mesh *> tempmeshes;
    for (vector<Mesh *>::size_type i = 0; i < meshsizes.size(); i++) {
        Unit *splitsub;
        tempmeshes.clear();
        tempmeshes.reserve(meshsizes[i]);
        for (unsigned int j = 0; j < meshsizes[i] && k < old.size(); ++j, ++k) {
            tempmeshes.push_back(old[k]);
        }
        unit->SubUnits.prepend(splitsub = new Unit(tempmeshes, true, unit->faction));
        *splitsub->current_hull = 1000.0f;
        splitsub->name = "debris";
        splitsub->setMass(game_options()->debris_mass * splitsub->getMass() / level);
        splitsub->pImage->timeexplode = .1;
        if (splitsub->meshdata[0]) {
            Vector loc = splitsub->meshdata[0]->Position();
            float locm = loc.Magnitude();
            if (locm < .0001) {
                locm = 1;
            }
            splitsub->ApplyForce(
                    splitsub->meshdata[0]->rSize() * game_options()->explosionforce * 10 * splitsub->getMass() * loc
                            / locm);
            loc.Set(rand(), rand(), rand() + .1);
            loc.Normalize();
            splitsub->ApplyLocalTorque(loc * splitsub->GetMoment() * game_options()->explosiontorque
                    * (1 + rand() % (int) (1 + unit->rSize())));
        }
    }
    old.clear();
    this->meshdata.clear();
    this->meshdata.push_back(NULL);     //the shield
    unit->Mass *= game_options()->debris_mass;
}

void Drawable::LightShields(const Vector &pnt, const Vector &normal, float amt, const GFXColor &color) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    // Not sure about shield percentage - more variance for more damage?
    // TODO: figure out the above comment

    Mesh *mesh = meshdata.back();

    if (!mesh) {
        return;
    }

    if (!unit) {
        return;
    }

    mesh->AddDamageFX(pnt, unit->shieldtight ? unit->shieldtight * normal : Vector(0, 0, 0),
            std::min(1.0f, std::max(0.0f, amt)), color);
}

Matrix Drawable::WarpMatrix(const Matrix &ctm) const {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    if (unit->GetWarpVelocity().MagnitudeSquared()
            < (static_cast<double>(game_options()->warp_stretch_cutoff) * game_options()->warp_stretch_cutoff * game_options()->game_speed
                    * game_options()->game_speed)
            || (game_options()->only_stretch_in_warp && unit->graphicOptions.InWarp == 0)) {
        return ctm;
    } else {
        Matrix k(ctm);

        float speed = unit->GetWarpVelocity().Magnitude();
        float stretchregion0length = static_cast<float>(game_options()->warp_stretch_region0_max
                * (speed - (game_options()->warp_stretch_cutoff * game_options()->game_speed))
                / ((game_options()->warp_stretch_max_region0_speed * game_options()->game_speed)
                        - (game_options()->warp_stretch_cutoff * game_options()->game_speed)));
        float stretchlength =
                static_cast<float>((game_options()->warp_stretch_max
                        - game_options()->warp_stretch_region0_max)
                        * (speed - (game_options()->warp_stretch_max_region0_speed * game_options()->game_speed))
                        / ((game_options()->warp_stretch_max_speed * game_options()->game_speed)
                                - (game_options()->warp_stretch_max_region0_speed * game_options()->game_speed) + .06125f)
                        + game_options()->warp_stretch_region0_max);
        if (stretchlength > game_options()->warp_stretch_max) {
            stretchlength = game_options()->warp_stretch_max;
        }
        if (stretchregion0length > game_options()->warp_stretch_region0_max) {
            stretchregion0length = game_options()->warp_stretch_region0_max;
        }
        ScaleMatrix(k,
                Vector(1,
                        1,
                        1 + (speed > (game_options()->warp_stretch_max_region0_speed * game_options()->game_speed)
                                ? stretchlength : stretchregion0length)));
        return k;
    }
}

void Drawable::UpdateHudMatrix(int whichcam) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    Matrix m;
    Matrix ctm = unit->cumulative_transformation_matrix;
    Vector q(ctm.getQ());
    Vector r(ctm.getR());
    Vector tmp;
    CrossProduct(r, q, tmp);
    _Universe->AccessCamera(whichcam)->SetOrientation(tmp, q, r);

    QVector v = Transform(ctm, unit->pImage->CockpitCenter.Cast());
    _Universe->AccessCamera(whichcam)->SetPosition(v,
            unit->GetWarpVelocity(),
            unit->GetAngularVelocity(),
            unit->GetAcceleration());
}

VSSprite *Drawable::getHudImage() const {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);
    return unit->pImage->pHudImage;
}
