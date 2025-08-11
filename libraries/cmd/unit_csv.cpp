/*
 * unit_csv.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "cmd/unit_generic.h"
#include "cmd/csv.h"
#include "root_generic/savegame.h"
#include "root_generic/xml_serializer.h"
#include "gfx_generic/sphere.h"
#include "cmd/unit_collide.h"
#include "collide2/Opcode.h"
#include "collide2/CSopcodecollider.h"
#include "src/audiolib.h"
#include "cmd/unit_xml.h"
#include "gfx_generic/quaternion.h"
#include "cmd/role_bitmask.h"
#include "cmd/unit_csv.h"
#include <algorithm>
#include <boost/json/array.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/serialize.hpp>

#include "root_generic/lin_time.h"
#include "cmd/unit_const_cache.h"
#include "src/vs_math.h"
#include "src/universe.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "cmd/mount_size.h"
#include "cmd/weapon_info.h"
#include "resource/resource.h"
#include "cmd/unit_csv_factory.h"
#include "cmd/upgradeable_unit.h"
#include "resource/manifest.h"
#include "components/component_utils.h"

extern int GetModeFromName(const char *input_buffer);
extern void pushMesh(std::vector<Mesh *> &mesh,
        float &randomstartframe,
        float &randomstartseconds,
        const char *filename,
        const float scale,
        int faction,
        class Flightgroup *fg,
        int startframe,
        double texturestarttime);
void addShieldMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg);
void addRapidMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg);

void AddMeshes(std::vector<Mesh *> &xmeshes,
               float &randomstartframe,
               float &randomstartseconds,
               float unitscale,
               const std::string &meshes,
               int faction,
               Flightgroup *fg,
               vector<unsigned int> *counts) {
    string::size_type where, when, wheresf, wherest, ofs = 0;

    // Clear counts vector
    if (counts) {
        counts->clear();
    }

    // Reserve space in counts and xmeshes vectors based on number of opening braces
    {
        // This code is proven to work for one mesh.
        std::string::difference_type number_of_elements = std::count(meshes.begin(), meshes.end(), '{');

        if (counts) {
            counts->reserve(number_of_elements);
        }
        xmeshes.reserve(number_of_elements);
    }
    while ((where = meshes.find('{', ofs)) != string::npos) {
        when = meshes.find('}', where + 1);         //matching closing brace
        string mesh = meshes.substr(where + 1, ((when == string::npos) ? string::npos : when - where - 1));
        ofs = ((when == string::npos) ? string::npos : when + 1);

        wheresf = mesh.find(';');
        string startf = "0";
        string startt = "0";
        if (wheresf != string::npos) {
            startf = mesh.substr(wheresf + 1);
            mesh = mesh.substr(0, wheresf);
            wherest = startf.find(';');
            if (wherest != string::npos) {
                startt = startf.substr(wherest + 1);
                startf = startf.substr(0, wherest);
            }
        }
        int startframe = startf == "RANDOM" ? -1 : (startf == "ASYNC" ? -1 : atoi(startf.c_str()));
        float starttime = startt == "RANDOM" ? -1.0f : atof(startt.c_str());
        unsigned int s = xmeshes.size();
        pushMesh(xmeshes,
                randomstartframe,
                randomstartseconds,
                mesh.c_str(),
                unitscale,
                faction,
                fg,
                startframe,
                starttime);
        if (counts) {
            counts->push_back(xmeshes.size() - s);
        }
    }
}

static std::pair<string::size_type, string::size_type> nextElementRange(const string &inp,
        string::size_type &start,
        string::size_type end) {
    string::size_type ostart = start;
    start = inp.find(';', start);
    if (start != string::npos && (start != end && (end == string::npos || start < end))) {
        ++start;
        return std::pair<string::size_type, string::size_type>(ostart, start - 1);
    } else {
        start = end;
        return std::pair<string::size_type, string::size_type>(ostart, end);
    }
}

static string nextElementString(const string &inp, string::size_type &start, string::size_type end) {
    std::pair<string::size_type, string::size_type> rng = nextElementRange(inp, start, end);
    if (rng.second == string::npos) {
        return inp.substr(rng.first);
    } else {
        return inp.substr(rng.first, rng.second - rng.first);
    }
}

static int nextElementInt(const string &inp, string::size_type &start, string::size_type end, int def = 0) {
    std::pair<string::size_type, string::size_type> rng = nextElementRange(inp, start, end);
    if (rng.second == string::npos && rng.first >= inp.length()) {
        return def;
    } else if (rng.first == rng.second) {
        return def;
    } else {
        return atoi(inp.c_str() + rng.first);
    }
}

static double nextElementFloat(const string &inp, string::size_type &start, string::size_type end, double def = 0) {
    std::pair<string::size_type, string::size_type> rng = nextElementRange(inp, start, end);
    if (rng.second == string::npos && rng.first >= inp.length()) {
        return def;
    } else if (rng.first == rng.second) {
        return def;
    } else {
        return atof(inp.c_str() + rng.first);
    }
}

static double nextElementBool(const string &inp, string::size_type &start, string::size_type end, bool def = false) {
    std::pair<string::size_type, string::size_type> rng = nextElementRange(inp, start, end);
    if (rng.second == string::npos && rng.first >= inp.length()) {
        return def;
    } else if (rng.first == rng.second) {
        return def;
    } else if (rng.second == string::npos) {
        return XMLSupport::parse_bool(inp.substr(rng.first));
    } else {
        return XMLSupport::parse_bool(inp.substr(rng.first, rng.second - rng.first));
    }
}

static string nextElement(string &inp) {
    string::size_type start = 0;
    std::pair<string::size_type, string::size_type> rng = nextElementRange(inp, start, string::npos);
    string ret = inp.substr(rng.first, ((rng.second == string::npos) ? string::npos : (rng.second - rng.first)));
    inp.erase(0, ((rng.second == string::npos) ? string::npos : (rng.second + 1)));
    return ret;
}



extern bool CheckAccessory(Unit *);

extern int parseMountSizes(const char *str);

static void AddMounts(Unit *thus, Unit::XML &xml, const std::string &mounts) {
    string::size_type where, when, ofs = 0;
    unsigned int first_new_mount = thus->mounts.size();
    {
        int nmountz = 0;
        while ((ofs = mounts.find('{', ofs)) != string::npos) {
            nmountz++, ofs++;
        }
        thus->mounts.reserve(nmountz + thus->mounts.size());
        ofs = 0;
    }
    while ((where = mounts.find('{', ofs)) != string::npos) {
        if ((when = mounts.find('}', where + 1)) != string::npos) {
            string::size_type elemstart = where + 1, elemend = when;
            ofs = when + 1;

            QVector P;
            QVector Q = QVector(0, 1, 0);
            QVector R = QVector(0, 0, 1);
            QVector pos = QVector(0, 0, 0);

            string filename = nextElementString(mounts, elemstart, elemend);
            int ammo = nextElementInt(mounts, elemstart, elemend, -1);
            int volume = nextElementInt(mounts, elemstart, elemend);
            string mountsize = nextElementString(mounts, elemstart, elemend);
            pos.i = nextElementFloat(mounts, elemstart, elemend);
            pos.j = nextElementFloat(mounts, elemstart, elemend);
            pos.k = nextElementFloat(mounts, elemstart, elemend);
            double xyscale = nextElementFloat(mounts, elemstart, elemend);
            double zscale = nextElementFloat(mounts, elemstart, elemend);
            R.i = nextElementFloat(mounts, elemstart, elemend);
            R.j = nextElementFloat(mounts, elemstart, elemend);
            R.k = nextElementFloat(mounts, elemstart, elemend, 1);
            Q.i = nextElementFloat(mounts, elemstart, elemend);
            Q.j = nextElementFloat(mounts, elemstart, elemend, 1);
            Q.k = nextElementFloat(mounts, elemstart, elemend);
            float func = nextElementFloat(mounts, elemstart, elemend, 1);
            float maxfunc = nextElementFloat(mounts, elemstart, elemend, 1);
            bool banked = nextElementBool(mounts, elemstart, elemend, false);
            Q.Normalize();
            if (fabs(Q.i) == fabs(R.i) && fabs(Q.j) == fabs(R.j) && fabs(Q.k) == fabs(R.k)) {
                Q.i = -1;
                Q.j = 0;
                Q.k = 0;
            }
            R.Normalize();

            CrossProduct(Q, R, P);
            CrossProduct(R, P, Q);
            Q.Normalize();
            Mount mnt(filename, ammo, volume, xml.unitscale * xyscale, xml.unitscale * zscale, func, maxfunc, banked);
            mnt.SetMountOrientation(Quaternion::from_vectors(P.Cast(), Q.Cast(), R.Cast()));
            mnt.SetMountPosition(xml.unitscale * pos.Cast());
            int mntsiz = as_integer(MOUNT_SIZE::NOWEAP);
            if (mountsize.length()) {
                mntsiz = parseMountSizes(mountsize.c_str());
                mnt.size = mntsiz;
            } else {
                mnt.size = as_integer(mnt.type->size);
            }
            thus->mounts.push_back(mnt);
        } else {
            ofs = string::npos;
        }
    }
    unsigned char parity = 0;
    bool half_sounds = configuration()->audio.every_other_mount;
    for (unsigned int a = first_new_mount; a < thus->mounts.size(); ++a) {
        if ((a & 1) == parity) {
            int b = a;
            if ((a & 3) == 2 && (int) a < (thus->getNumMounts() - 1)) {
                if (thus->mounts[a].type->type != WEAPON_TYPE::PROJECTILE
                        && thus->mounts[a + 1].type->type != WEAPON_TYPE::PROJECTILE) {
                    b = a + 1;
                }
            }
            thus->mounts[b].sound = AUDCreateSound(thus->mounts[b].type->sound, false);
        } else if ((!half_sounds) || thus->mounts[a].type->type == WEAPON_TYPE::PROJECTILE) {
            thus->mounts[a].sound = AUDCreateSound(thus->mounts[a].type->sound, false);
        }
        if (a > 0) {
            if (thus->mounts[a].sound == thus->mounts[a - 1].sound && thus->mounts[a].sound != -1) {
                VS_LOG(error, "Sound error");
            }
        }
    }
}

struct SubUnitStruct {
    string filename;
    QVector pos;
    QVector Q;
    QVector R;
    double restricted;

    SubUnitStruct(string fn, QVector POS, QVector q, QVector r, double res) {
        filename = fn;
        pos = POS;
        Q = q;
        R = r;
        restricted = res;
    }
};

static vector<SubUnitStruct> GetSubUnits(const std::string &subunits) {
    string::size_type where, when, ofs = 0;
    vector<SubUnitStruct> ret;
    {
        int nelem = 0;
        while ((ofs = subunits.find('{', ofs)) != string::npos) {
            nelem++, ofs++;
        }
        ret.reserve(nelem);
        ofs = 0;
    }
    while ((where = subunits.find('{', ofs)) != string::npos) {
        if ((when = subunits.find('}', ofs)) != string::npos) {
            string::size_type elemstart = where + 1, elemend = when;
            ofs = when + 1;

            QVector pos, Q, R;

            string filename = nextElementString(subunits, elemstart, elemend);
            pos.i = nextElementFloat(subunits, elemstart, elemend);
            pos.j = nextElementFloat(subunits, elemstart, elemend);
            pos.k = nextElementFloat(subunits, elemstart, elemend);
            R.i = nextElementFloat(subunits, elemstart, elemend);
            R.j = nextElementFloat(subunits, elemstart, elemend);
            R.k = nextElementFloat(subunits, elemstart, elemend);
            Q.i = nextElementFloat(subunits, elemstart, elemend);
            Q.j = nextElementFloat(subunits, elemstart, elemend);
            Q.k = nextElementFloat(subunits, elemstart, elemend);
            double restricted = cos(nextElementFloat(subunits, elemstart, elemend, 180) * M_PI / 180.0);

            ret.push_back(SubUnitStruct(filename, pos, Q, R, restricted));
        } else {
            ofs = string::npos;
        }
    }
    return ret;
}

static void AddSubUnits(Unit *thus,
        Unit::XML &xml,
        const std::string &subunits,
        int faction,
        const std::string &modification) {
    vector<SubUnitStruct> su = GetSubUnits(subunits);
    xml.units.reserve(subunits.size() + xml.units.size());
    for (vector<SubUnitStruct>::iterator i = su.begin(); i != su.end(); ++i) {
        string filename = (*i).filename;
        QVector pos = (*i).pos;
        QVector Q = (*i).Q;
        QVector R = (*i).R;
        double restricted = (*i).restricted;
        xml.units
                .push_back(new Unit(filename.c_str(),
                        true,
                        faction,
                        modification,
                        NULL));         //I set here the fg arg to NULL

        if (!thus->isSubUnit()) {         //Useless to set recursive owner in subunits - as parent will do the same
            xml.units.back()->SetRecursiveOwner(thus);
        }
        xml.units.back()->SetOrientation(Q, R);
        R.Normalize();
        xml.units.back()->prev_physical_state = xml.units.back()->curr_physical_state;
        xml.units.back()->SetPosition(pos * xml.unitscale);

        // Subunit movement restrictions
        xml.units.back()->structure_limits = R.Cast();
        xml.units.back()->limit_min = restricted;

        xml.units.back()->name = filename;
        if (xml.units.back()->pImage->unitwriter != NULL) {
            xml.units.back()->pImage->unitwriter->setName(filename);
        }
        CheckAccessory(xml.units.back());         //turns on the ceerazy rotation for the turr
    }
    for (int a = xml.units.size() - 1; a >= 0; a--) {
        bool randomspawn = xml.units[a]->name.get().find("randomspawn") != string::npos;
        if (randomspawn) {
            int chancetospawn = float_to_int(xml.units[a]->ftl_energy.MaxLevel());
            if (chancetospawn > rand() % 100) {
                thus->SubUnits.prepend(xml.units[a]);
            } else {
                xml.units[a]->Kill();
            }
        } else {
            thus->SubUnits.prepend(xml.units[a]);
        }
    }
}

void AddDocks(Unit *thus, Unit::XML &xml, const string &docks) {
    string::size_type where, when;
    string::size_type ofs = 0;
    int overlap = 1;

    // TODO: Someone should check blame here. It doesn't look right.
    {
        int nelem = 0;
        while ((ofs = docks.find('{', ofs)) != string::npos) {
            nelem++, ofs++;
        }
        thus->pImage->dockingports.reserve(nelem * overlap + thus->pImage->dockingports.size());
        ofs = 0;
    }
    while ((where = docks.find('{', ofs)) != string::npos) {
        if ((when = docks.find('}', where + 1)) != string::npos) {
            string::size_type elemstart = where + 1, elemend = when;
            ofs = when + 1;

            QVector pos = QVector(0, 0, 0);
            int type = nextElementInt(docks, elemstart, elemend);
            pos.i = nextElementFloat(docks, elemstart, elemend);
            pos.j = nextElementFloat(docks, elemstart, elemend);
            pos.k = nextElementFloat(docks, elemstart, elemend);
            double size = nextElementFloat(docks, elemstart, elemend);
            double minsize = nextElementFloat(docks, elemstart, elemend);
            for (int i = 0; i < overlap; i++) {
                thus->pImage
                        ->dockingports
                        .emplace_back(pos.Cast() * xml.unitscale, size * xml.unitscale, minsize
                                * xml.unitscale, DockingPorts::Type::Value(type));
            }
        } else {
            ofs = string::npos;
        }
    }
}

void AddLights(Unit *thus, Unit::XML &xml, const string &lights) {
    const float default_halo_activation = configuration()->graphics.default_engine_activation;
    string::size_type where, when;
    string::size_type ofs = 0;
    while ((where = lights.find('{', ofs)) != string::npos) {
        if ((when = lights.find('}', where + 1)) != string::npos) {
            string::size_type elemstart = where + 1, elemend = when;
            ofs = when + 1;

            string filename = nextElementString(lights, elemstart, elemend);
            QVector pos, scale;
            GFXColor halocolor;
            QVector P(1, 0, 0), Q(0, 1, 0), R(0, 0, 1);
            pos.i = nextElementFloat(lights, elemstart, elemend);
            pos.j = nextElementFloat(lights, elemstart, elemend);
            pos.k = nextElementFloat(lights, elemstart, elemend);
            scale.i = xml.unitscale * nextElementFloat(lights, elemstart, elemend, 1);
            scale.j = scale.k = scale.i;
            halocolor.r = nextElementFloat(lights, elemstart, elemend, 1);
            halocolor.g = nextElementFloat(lights, elemstart, elemend, 1);
            halocolor.b = nextElementFloat(lights, elemstart, elemend, 1);
            halocolor.a = nextElementFloat(lights, elemstart, elemend, 1);
            double act_speed = nextElementFloat(lights, elemstart, elemend, default_halo_activation);
            R.i = nextElementFloat(lights, elemstart, elemend);
            R.j = nextElementFloat(lights, elemstart, elemend);
            R.k = nextElementFloat(lights, elemstart, elemend, 1);
            Q.i = nextElementFloat(lights, elemstart, elemend);
            Q.j = nextElementFloat(lights, elemstart, elemend, 1);
            Q.k = nextElementFloat(lights, elemstart, elemend);

            Q.Normalize();
            if (fabs(Q.i) == fabs(R.i) && fabs(Q.j) == fabs(R.j) && fabs(Q.k) == fabs(R.k)) {
                Q = QVector(-1, 0, 0);
            }
            R.Normalize();
            CrossProduct(Q, R, P);
            CrossProduct(R, P, Q);
            Q.Normalize();
            Matrix trans(P.Cast(), Q.Cast(), R.Cast(), pos * xml.unitscale);

            thus->addHalo(filename.c_str(), trans, scale.Cast(), halocolor, "", act_speed);
        } else {
            ofs = string::npos;
        }
    }
}

static void ImportCargo(Unit *thus, const string &imports) {
    string::size_type where, when, ofs = 0;
    {
        int nelem = 0;
        while ((ofs = imports.find('{', ofs)) != string::npos) {
            nelem++, ofs++;
        }
        thus->cargo.reserve(nelem + thus->cargo.size());
        ofs = 0;
    }
    while ((where = imports.find('{', ofs)) != string::npos) {
        if ((when = imports.find('}', where + 1)) != string::npos) {
            string::size_type elemstart = where + 1, elemend = when;
            ofs = when + 1;

            string filename = nextElementString(imports, elemstart, elemend);
            double price = nextElementFloat(imports, elemstart, elemend, 1);
            double pricestddev = nextElementFloat(imports, elemstart, elemend);
            double quant = nextElementFloat(imports, elemstart, elemend, 1);
            double quantstddev = nextElementFloat(imports, elemstart, elemend);

            thus->ImportPartList(filename, price, pricestddev, quant, quantstddev);
        } else {
            ofs = string::npos;
        }
    }
}

static void AddCarg(Unit *thus, const string &cargo_text) {
    // TODO: better error handling here and below
    if(cargo_text.size() < 10) {
        return;
    }

    std::string trimmed_cargo_text = cargo_text.substr(1, cargo_text.size() - 2);
    std::vector<std::string> cargo_text_elements;
    boost::split(cargo_text_elements,trimmed_cargo_text,boost::is_any_of("}{"));

    for(std::string& cargo_text_element : cargo_text_elements) {
        if(cargo_text_element.size()<10) {
            // There's probably a better minimum size, but it should be good enough
            // This is used to identify text that created a dummy cargo with defaults
            // probably at the beginning or end of the cargo_text_elements vector.
            continue;
        }
        try {
            Cargo c(cargo_text_element);
            thus->AddCargo(c);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing cargo: " << e.what() << std::endl;
        }
    }
}

void HudDamage(float *dam, const string &damages) {
    if (dam) {
        string::size_type elemstart = 0, elemend = string::npos;
        for (int i = 0; i < 1 + MAXVDUS + UnitImages<void>::NUMGAUGES; ++i) {
            dam[i] = nextElementFloat(damages, elemstart, elemend, 1);
        }
    }
}


void LoadCockpit(Unit *thus, const string &cockpit) {
    string::size_type elemstart = 0, elemend = string::npos;
    thus->pImage->cockpitImage = nextElementString(cockpit, elemstart, elemend);
    thus->pImage->CockpitCenter.i = nextElementFloat(cockpit, elemstart, elemend);
    thus->pImage->CockpitCenter.j = nextElementFloat(cockpit, elemstart, elemend);
    thus->pImage->CockpitCenter.k = nextElementFloat(cockpit, elemstart, elemend);
}


const std::string EMPTY_STRING("");

extern std::string getDamageColor(double);

void Unit::LoadRow(std::string unit_identifier, string modification, bool saved_game) {
    Unit::XML xml;
    xml.unitModifications = modification.c_str();
    xml.randomstartframe = ((float) rand()) / RAND_MAX;
    xml.randomstartseconds = 0;
    xml.calculated_role = false;
    xml.damageiterator = 0;
    xml.shieldmesh = NULL;
    xml.rapidmesh = NULL;
    xml.hasColTree = true;
    xml.unitlevel = 0;
    xml.unitscale = 1;
    xml.data = xml.shieldmesh = xml.rapidmesh = NULL;     //was uninitialized memory
    string tmpstr;
    csvRow = unit_identifier;

    // Textual Descriptions
    this->setUnitKey(unit_identifier);
    this->unit_name = UnitCSVFactory::GetVariable(unit_key, "Name", std::string());
    this->unit_description = Manifest::MPL().GetShipDescription(unit_identifier);

    // This shadows the unit variable. It also doesn't support more than one ship.
    // TODO: figure this out.
    std::string unit_key = (saved_game ? "player_ship" : unit_identifier);

    fullname = UnitCSVFactory::GetVariable(unit_key, "Name", std::string());

    tmpstr = UnitCSVFactory::GetVariable(unit_key, "Hud_image", std::string());

    if (!tmpstr.empty()) {
        std::string fac = FactionUtil::GetFaction(faction);
        fac += "_";
        fac += tmpstr;
        pImage->pHudImage = createVSSprite(fac.c_str());
        if (!isVSSpriteLoaded(pImage->pHudImage)) {
            deleteVSSprite(pImage->pHudImage);
            pImage->pHudImage = createVSSprite(tmpstr.c_str());
        }
    }

    tmpstr = UnitCSVFactory::GetVariable(unit_key, "FaceCamera", std::string());

    if (!tmpstr.empty()) {
        graphicOptions.FaceCamera = XMLSupport::parse_bool(tmpstr) ? 1 : 0;
    }

    std::string llegacy_combat_role(UnitCSVFactory::GetVariable(unit_key, "Combat_Role", std::string()));
    std::string lunit_role(UnitCSVFactory::GetVariable(unit_key, "Unit_Role", std::string()));
    std::string lattack_preference(UnitCSVFactory::GetVariable(unit_key, "Attack_Preference", std::string()));

    if (lunit_role.empty()) {
        this->setUnitRole(llegacy_combat_role);
    } else {
        this->setUnitRole(lunit_role);
    }

    if (lattack_preference.empty()) {
        this->setAttackPreference(llegacy_combat_role);
    } else {
        this->setAttackPreference(lattack_preference);
    }

    graphicOptions.NumAnimationPoints = UnitCSVFactory::GetVariable(unit_key, "Num_Animation_Stages", 0);
    graphicOptions.NoDamageParticles = UnitCSVFactory::GetVariable(unit_key, "NoDamageParticles", 0);

    if (graphicOptions.NumAnimationPoints > 0) {
        graphicOptions.Animating = 0;
    }
    xml.unitscale = UnitCSVFactory::GetVariable(unit_key, "Unit_Scale", 1.0f);
    if (!xml.unitscale) {
        xml.unitscale = 1;
    }
    pImage->unitscale = xml.unitscale;

    std::string mesh_string = UnitCSVFactory::GetVariable(unit_key, "Mesh", std::string());
    AddMeshes(xml.meshes, xml.randomstartframe, xml.randomstartseconds, xml.unitscale,
            mesh_string, faction,
            getFlightgroup());

    std::string dock_string = UnitCSVFactory::GetVariable(unit_key, "Dock", std::string());
    AddDocks(this, xml, UnitCSVFactory::GetVariable(unit_key, "Dock", std::string()));

    std::string sub_string = UnitCSVFactory::GetVariable(unit_key, "Sub_Units", std::string());
    AddSubUnits(this, xml, UnitCSVFactory::GetVariable(unit_key, "Sub_Units", std::string()), faction, modification);

    meshdata = xml.meshes;
    meshdata.push_back(NULL);
    corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
    corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    calculate_extent(false);
    AddMounts(this, xml, UnitCSVFactory::GetVariable(unit_key, "Mounts", std::string()));
    this->CargoVolume = UnitCSVFactory::GetVariable(unit_key, "Hold_Volume", 0.0f);
    this->HiddenCargoVolume = UnitCSVFactory::GetVariable(unit_key, "Hidden_Hold_Volume", 0.0f);
    this->UpgradeVolume = UnitCSVFactory::GetVariable(unit_key, "Upgrade_Storage_Volume", 0.0f);

    std::string cargo_import_string = UnitCSVFactory::GetVariable(unit_key, "Cargo_Import", std::string());
    ImportCargo(this, cargo_import_string);     //if this changes change planet_generic.cpp

    std::string cargo_string = UnitCSVFactory::GetVariable(unit_key, "Cargo", std::string());
    AddCarg(this, cargo_string);

    // Replaced by below: AddSounds( this, OPTIM_GET( row, table, Sounds ) );
    this->addSounds(&nextElement, UnitCSVFactory::GetVariable(unit_key, "Sounds", std::string()));

    LoadCockpit(this, UnitCSVFactory::GetVariable(unit_key, "Cockpit", std::string()));
    pImage->CockpitCenter.i = UnitCSVFactory::GetVariable(unit_key, "CockpitX", 0.0f) * xml.unitscale;
    pImage->CockpitCenter.j = UnitCSVFactory::GetVariable(unit_key, "CockpitY", 0.0f) * xml.unitscale;
    pImage->CockpitCenter.k = UnitCSVFactory::GetVariable(unit_key, "CockpitZ", 0.0f) * xml.unitscale;
    Load(unit_key); // ComponentsManager
    Momentofinertia = GetMass();


    // Hull
    hull.Load(unit_key);
    armor.Load(unit_key);
    shield.Load(unit_key);


    // Energy
    // TODO: The following code has a bug.
    // It will set the max of the component as the current value loaded from the
    // CSV. If the component is damaged, this will be lower than the original value.
    fuel.Load(unit_key);
    energy.Load(unit_key);
    ftl_energy.Load(unit_key);
    reactor.Load(unit_key);

    // End Energy

    graphicOptions.MinWarpMultiplier = UnitCSVFactory::GetVariable(unit_key, "Warp_Min_Multiplier", 1.0f);
    graphicOptions.MaxWarpMultiplier = UnitCSVFactory::GetVariable(unit_key, "Warp_Max_Multiplier", 1.0f);

    // Begin Drive Section
    // Afterburner
    afterburner = Afterburner(GetSource(ComponentType::Afterburner, &fuel, &energy, &ftl_energy));
    afterburner.Load(unit_key);

    drive = Drive(GetSource(ComponentType::Drive, &fuel, &energy, &ftl_energy));
    drive.Load(unit_key);

    ftl_drive.Load(unit_key);
    jump_drive.Load(unit_key);


    forcejump = UnitCSVFactory::GetVariable(unit_key, "Wormhole", false);
    graphicOptions.RecurseIntoSubUnitsOnCollision = UnitCSVFactory::GetVariable(unit_key,
            "Collide_Subunits",
            graphicOptions.RecurseIntoSubUnitsOnCollision
                    ? true : false) ? 1 : 0;

    // End Drive Section

    computer.itts = UnitCSVFactory::GetVariable(unit_key, "ITTS", true);

    radar.Load(unit_key);

    const static bool warp_energy_for_cloak = configuration()->warp.use_warp_energy_for_cloak;
    cloak.SetSource((warp_energy_for_cloak ? &ftl_energy : &energy));
    cloak.Load(unit_key);

    ecm.Load(unit_key);
    repair_bot.Load(unit_key);
    ship_functions.Load(unit_key);

    computer.slide_start = UnitCSVFactory::GetVariable(unit_key, "Slide_Start", 0);
    computer.slide_end = UnitCSVFactory::GetVariable(unit_key, "Slide_End", 0);

    UpgradeUnit(UnitCSVFactory::GetVariable(unit_key, "Cargo", std::string()));


    {
        std::string tractorability = UnitCSVFactory::GetVariable(unit_key, "Tractorability", std::string());
        unsigned char tflags;
        if (!tractorability.empty()) {
            tflags = tractorImmune;
            if (tractorability.find_first_of("pP") != string::npos) {
                tflags |= tractorPush;
            }
            if (tractorability.find_first_of("iI") != string::npos) {
                tflags |= tractorIn;
            }
        } else {
            tflags = tractorPush;
        }
        setTractorability((enum tractorHow) tflags);
    }
    this->pImage->explosion_type = UnitCSVFactory::GetVariable(unit_key, "Explosion", std::string());
    if (pImage->explosion_type.get().length()) {
        cache_ani(pImage->explosion_type);
    } else {
        const std::string expani = configuration()->graphics.explosion_animation;
        cache_ani(expani);
    }
    AddLights(this, xml, UnitCSVFactory::GetVariable(unit_key, "Light", std::string()));
    xml.shieldmesh_str = UnitCSVFactory::GetVariable(unit_key, "Shield_Mesh", std::string());
    if (xml.shieldmesh_str.length()) {
        addShieldMesh(&xml, xml.shieldmesh_str.c_str(), xml.unitscale, faction, getFlightgroup());
        meshdata.back() = xml.shieldmesh;
    } else {
        const int shieldstacks = configuration()->graphics.shield_detail_level;
        const std::string& shieldtex = configuration()->graphics.shield_texture;
        const std::string& shieldtechnique = configuration()->graphics.shield_technique;
        meshdata.back() = new SphereMesh(rSize(),
                shieldstacks,
                shieldstacks,
                shieldtex.c_str(),
                shieldtechnique,
                NULL,
                false,
                ONE,
                ONE);
    }
    meshdata.back()->EnableSpecialFX();
    //Begin the Pow-w-w-war Zone Collide Tree Generation
    {
        xml.rapidmesh_str = UnitCSVFactory::GetVariable(unit_key, "Rapid_Mesh", std::string());
        vector<mesh_polygon> polies;

        std::string collideTreeHash = VSFileSystem::GetHashName(modification + "#" + unit_identifier);
        this->colTrees = collideTrees::Get(collideTreeHash);
        if (this->colTrees) {
            this->colTrees->Inc();
        }
        csOPCODECollider *colShield = NULL;
        string tmpname = unit_identifier;       //key
        if (!this->colTrees) {
            string val;
            xml.hasColTree = 1;
            if ((val = UnitCSVFactory::GetVariable(unit_key, "Use_Rapid", std::string())).length()) {
                xml.hasColTree = XMLSupport::parse_bool(val);
            }
            if (xml.shieldmesh) {
                if (meshdata.back()) {
                    meshdata.back()->GetPolys(polies);
                    colShield = new csOPCODECollider(polies);
                }
            }
            if (xml.rapidmesh_str.length()) {
                addRapidMesh(&xml, xml.rapidmesh_str.c_str(), xml.unitscale, faction, getFlightgroup());
            } else {
                xml.rapidmesh = NULL;
            }
            polies.clear();
            if (xml.rapidmesh) {
                xml.rapidmesh->GetPolys(polies);
            }
            csOPCODECollider *csrc = NULL;
            if (xml.hasColTree) {
                csrc = getCollideTree(Vector(1, 1, 1),
                        xml.rapidmesh
                                ? &polies : NULL);
            }
            this->colTrees = new collideTrees(collideTreeHash,
                    csrc,
                    colShield);
            if (xml.rapidmesh && xml.hasColTree) {
                //if we have a special rapid mesh we need to generate things now
                for (unsigned int i = 1; i < collideTreesMaxTrees; ++i) {
                    if (!this->colTrees->rapidColliders[i]) {
                        unsigned int which = 1 << i;
                        this->colTrees->rapidColliders[i] =
                                getCollideTree(Vector(1, 1, which),
                                        &polies);
                    }
                }
            }
            if (xml.rapidmesh != nullptr) {
                delete xml.rapidmesh;
                xml.rapidmesh = nullptr;
            }
        }
    }
    CheckAccessory(this);     //turns on the ceerazy rotation for any accessories
    this->setAverageGunSpeed();

    // From drawable
    this->num_chunks = UnitCSVFactory::GetVariable(unit_key, "Num_Chunks", 0);

    // Add integral components
    // Make this a factor of unit price
    if(saved_game) {
        // If we are loading a saved game, we assume the integral components are already installed.
        // However, we still need to set them as integral and ensure quantity is 1 for each.
        // Note: In the future, we may want to support multiple integral components.
        for(Cargo& cargo : this->cargo) {
            if(cargo.GetCategory().find("upgrades/integral") == 0) {
                cargo.SetQuantity(1);
                cargo.SetInstalled(true);
                cargo.SetIntegral(true);
            }
        }
    } else {
        const std::string integral_components =
        "{hull;upgrades/integral;12000;1;0.1;0.1;1;1;@cargo/hull_patches.image@The ship's hull.;0;1}\
        {afterburner;upgrades/integral;2000;1;0.1;0.1;1;1;@upgrades/afterburner_generic.image@Engine overdrive. Increases thrust at the expense of decreased fuel efficiency.;0;1}\
        {drive;upgrades/integral;6000;1;0.1;0.1;1;1;@upgrades/afterburner_generic.image@The ship's engine.;0;1}\
        {ftl_drive;upgrades/integral;4500;1;0.1;0.1;1;1;@upgrades/jump_drive.image@The ship's faster than light engine.;0;1}";

        AddCarg(this, integral_components);
    }

    GenerateHudText(getDamageColor);
}

void Unit::WriteUnit(const char *modifications) {
    bool bad = false;
    if (!modifications) {
        bad = true;
    }
    if (!bad) {
        if (!strlen(modifications)) {
            bad = true;
        }
    }
    if (bad) {
        VS_LOG(error,
                (boost::format("Cannot Write out unit file %1% %2% that has no filename") % name.get().c_str()
                        % csvRow.get().c_str()));
        return;
    }
    std::string savedir = modifications;
    VSFileSystem::CreateDirectoryHome(VSFileSystem::savedunitpath + "/" + savedir);
    VSFileSystem::VSFile f;
    VSFileSystem::VSError err = f.OpenCreateWrite(savedir + "/" + name + ".json", VSFileSystem::UnitFile);
    if (err > VSFileSystem::Ok) {
        VS_LOG(error, (boost::format("!!! ERROR : Writing saved unit file : %1%") % f.GetFullPath().c_str()));
        return;
    }

    std::map<std::string, std::string> map = UnitToMap();
    boost::json::array json_root_array;
    json_root_array.emplace_back(boost::json::value_from(map));
    std::string towrite = boost::json::serialize(json_root_array);
    f.Write(towrite.c_str(), towrite.length());
    f.Close();
}

using XMLSupport::tostring;

static void mapToStringVec(vsUMap<string, string> a, vector<string> &key, vector<string> &value) {
    for (vsUMap<string, string>::iterator i = a.begin(); i != a.end(); ++i) {
        key.push_back(i->first);
        value.push_back(i->second);
    }
}

static string tos(double val) {
    return XMLSupport::tostring((float) val);
}

static string tos(bool val) {
    return XMLSupport::tostring((int) val);
}

static string tos(int val) {
    return XMLSupport::tostring(val);
}

const std::map<std::string, std::string> Unit::UnitToMap() {
    std::map<std::string, std::string> unit = std::map<std::string, std::string>();
    string val;

    // Textual Descriptions
    unit["Key"] = unit_key;
    unit["Name"] = unit_name;
    unit["Textual_Description"] = unit_description; // Used in ship view

    // Take some immutable stats directly from the original unit
    const std::string immutable_stats[] = {"Directory", "STATUS", "Combat_Role", "Hud_image",
                                           "Unit_Scale", "CockpitZ", "Mesh", "Prohibited_Upgrades",
                                           "Light"};

    for(const auto& stat : immutable_stats) {
        unit[stat] = UnitCSVFactory::GetVariable(unit_key, stat, std::string());
    }

    //mutable things
    unit["Hold_Volume"] = XMLSupport::tostring(CargoVolume);
    unit["Hidden_Hold_Volume"] = XMLSupport::tostring(HiddenCargoVolume);
    unit["Upgrade_Storage_Volume"] = XMLSupport::tostring(UpgradeVolume);
    string mountstr;
    double unitScale = UnitCSVFactory::GetVariable(unit_key, "Unit_Scale", 1.0f);
    {
        //mounts
        for (unsigned int j = 0; j < mounts.size(); ++j) {
            char mnt[1024];
            Matrix m;
            Transformation tr(mounts[j].GetMountOrientation(),
                              mounts[j].GetMountLocation().Cast());
            tr.to_matrix(m);
            string printedname = mounts[j].type->name;
            if (mounts[j].status == Mount::DESTROYED || mounts[j].status == Mount::UNCHOSEN) {
                printedname = "";
            }
            mountstr += "{" + printedname + ";" + XMLSupport::tostring(mounts[j].ammo) + ";"
                    + XMLSupport::tostring(
                        mounts[j].volume) + ";" + getMountSizeString(mounts[j].size);
            sprintf(mnt, ";%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf}",
                    m.p.i / unitScale,
                    m.p.j / unitScale,
                    m.p.k / unitScale,
                    (double) mounts[j].xyscale / unitScale,
                    (double) mounts[j].zscale / unitScale,
                    (double) m.getR().i,
                    (double) m.getR().j,
                    (double) m.getR().k,
                    (double) m.getQ().i,
                    (double) m.getQ().j,
                    (double) m.getQ().k,
                    (double) mounts[j].functionality,
                    (double) mounts[j].maxfunctionality
                    );
            mountstr += mnt;
        }
        unit["Mounts"] = mountstr;
    }
    {
        //subunits
        vector<SubUnitStruct> subunits = GetSubUnits(unit["Sub_Units"]);
        if (subunits.size()) {
            unsigned int k = 0;
            Unit *subun;
            for (; k < subunits.size(); ++k) {
                subunits[k].filename = "destroyed_blank";
            }
            k = 0;
            for (un_iter su = this->getSubUnits(); (subun = (*su)) != NULL; ++su, ++k) {
                unsigned int j = k;
                for (; j < subunits.size(); ++j) {
                    if ((subun->Position() - subunits[j].pos).MagnitudeSquared() < .00000001) {
                        //we've got a hit
                        break;
                    }
                }
                if (j >= subunits.size()) {
                    j = k;
                }
                if (j < subunits.size()) {
                    subunits[j].filename = subun->name;
                }
            }
            string str;
            for (k = 0; k < subunits.size(); ++k) {
                char tmp[1024];
                sprintf(tmp, ";%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf}",
                        subunits[k].pos.i,
                        subunits[k].pos.j,
                        subunits[k].pos.k,
                        subunits[k].R.i,
                        subunits[k].R.j,
                        subunits[k].R.k,
                        subunits[k].Q.i,
                        subunits[k].Q.j,
                        subunits[k].Q.k,
                        ((double) acos(subunits[k].restricted) * 180. / M_PI));
                str += "{" + subunits[k].filename + tmp;
            }
            unit["Sub_Units"] = str;
        }
    }
    {
        string carg ;
        for (const Cargo& c : cargo) {
            carg += c.Serialize();
        }
        unit["Cargo"] = carg;
    }
    Serialize(unit); // ComponentsManager

    hull.SaveToCSV(unit);
    armor.SaveToCSV(unit);
    shield.SaveToCSV(unit);

    unit["Warp_Min_Multiplier"] = tos(graphicOptions.MinWarpMultiplier);
    unit["Warp_Max_Multiplier"] = tos(graphicOptions.MaxWarpMultiplier);


    reactor.SaveToCSV(unit);
    fuel.SaveToCSV(unit);
    energy.SaveToCSV(unit);
    ftl_energy.SaveToCSV(unit);

    afterburner.SaveToCSV(unit);
    drive.SaveToCSV(unit);
    jump_drive.SaveToCSV(unit);
    ftl_drive.SaveToCSV(unit);

    radar.SaveToCSV(unit);
    cloak.SaveToCSV(unit);

    unit["Wormhole"] = tos(forcejump != 0);


    unit["ITTS"] = tos(computer.itts);

    ecm.SaveToCSV(unit);
    repair_bot.SaveToCSV(unit);
    ship_functions.SaveToCSV(unit);

    unit["Slide_Start"] = tos(computer.slide_start);
    unit["Slide_End"] = tos(computer.slide_end);
    unit["Cargo_Import"] = unit["Upgrades"] = "";                 //make sure those are empty
    {
        std::string trac;
        if (isTractorable(tractorPush)) {
            trac += "p";
        }
        if (isTractorable(tractorIn)) {
            trac += "i";
        }
        if (trac.empty()) {
            trac = "-";
        }
        unit["Tractorability"] = trac;
    }

    return unit;
}


string Unit::WriteUnitString() {
    std::map<std::string, std::string> unit = UnitToMap();
    return writeCSV(unit);
}
