/*
 * unit_csv.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Roy Falk, and other Vega Strike contributors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "unit_generic.h"
#include "csv.h"
#include "savegame.h"
#include "xml_serializer.h"
#include "gfx/sphere.h"
#include "unit_collide.h"
#include "collide2/Stdafx.h"
#include "collide2/CSopcodecollider.h"
#include "audiolib.h"
#include "unit_xml.h"
#include "gfx/quaternion.h"
#include "role_bitmask.h"
#include "unit_csv.h"
#include <algorithm>
#include "lin_time.h"
#include "unit_const_cache.h"
#include "vs_math.h"
#include "universe.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "mount_size.h"
#include "weapon_info.h"
#include "resource/resource.h"
#include "unit_csv_factory.h"
#include "upgradeable_unit.h"
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

static bool stob(const string &inp, bool defaul) {
    if (inp.length() != 0) {
        return XMLSupport::parse_bool(inp);
    }
    return defaul;
}

static double stof(const string &inp, double def = 0) {
    if (inp.length() != 0) {
        return XMLSupport::parse_float(inp);
    }
    return def;
}

static int stoi(const string &inp, int def = 0) {
    if (inp.length() != 0) {
        return XMLSupport::parse_int(inp);
    }
    return def;
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
    bool half_sounds = configuration()->audio_config.every_other_mount;
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
    const float default_halo_activation = configuration()->graphics_config.default_engine_activation;
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

static void AddCarg(Unit *thus, const string &cargos) {
    string::size_type where, when, ofs = 0;
    {
        int nelem = 0;
        while ((ofs = cargos.find('{', ofs)) != string::npos) {
            nelem++, ofs++;
        }
        thus->cargo.reserve(nelem + thus->cargo.size());
        ofs = 0;
    }
    while ((where = cargos.find('{', ofs)) != string::npos) {
        if ((when = cargos.find('}', where + 1)) != string::npos) {
            string::size_type elemstart = where + 1, elemend = when;
            ofs = when + 1;

            std::string name = nextElementString(cargos, elemstart, elemend);
            std::string category = nextElementString(cargos, elemstart, elemend);
            float price = nextElementFloat(cargos, elemstart, elemend);
            int quantity = nextElementInt(cargos, elemstart, elemend);
            float mass = nextElementFloat(cargos, elemstart, elemend);
            float volume = nextElementFloat(cargos, elemstart, elemend);
            float functionality = nextElementFloat(cargos, elemstart, elemend, 1.f);
            float max_functionality = nextElementFloat(cargos, elemstart, elemend, 1.f);
            std::string description = nextElementString(cargos, elemstart, elemend);
            bool mission = nextElementBool(cargos, elemstart, elemend, false);
            bool installed = nextElementBool(cargos, elemstart, elemend,
                    category.find("upgrades/") == 0);

            Cargo carg(name, category, price, quantity, mass, volume, functionality,
                       max_functionality, mission, installed);




            thus->AddCargo(carg, false);
        } else {
            ofs = string::npos;
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

string WriteHudDamage(Unit *un) {
    string ret;
    const string semi = ";";
    if (un->pImage->cockpit_damage) {
        for (int i = 0; i < 1 + MAXVDUS + UnitImages<void>::NUMGAUGES; ++i) {
            ret += XMLSupport::tostring(un->pImage->cockpit_damage[i]);
            ret += semi;
        }
    }
    return ret;
}

string WriteHudDamageFunc(Unit *un) {
    string ret;
    const string semi = ";";
    if (un->pImage->cockpit_damage) {
        int numg = 1 + MAXVDUS + UnitImages<void>::NUMGAUGES;
        for (int i = numg; i < 2 * numg; ++i) {
            ret += XMLSupport::tostring(un->pImage->cockpit_damage[i]);
            ret += semi;
        }
    }
    return ret;
}

void LoadCockpit(Unit *thus, const string &cockpit) {
    string::size_type elemstart = 0, elemend = string::npos;
    thus->pImage->cockpitImage = nextElementString(cockpit, elemstart, elemend);
    thus->pImage->CockpitCenter.i = nextElementFloat(cockpit, elemstart, elemend);
    thus->pImage->CockpitCenter.j = nextElementFloat(cockpit, elemstart, elemend);
    thus->pImage->CockpitCenter.k = nextElementFloat(cockpit, elemstart, elemend);
}


const std::string EMPTY_STRING("");



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
    this->unit_key = unit_identifier;            
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
    this->equipment_volume = UnitCSVFactory::GetVariable(unit_key, "Equipment_Space", 0.0f);

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
    Mass = UnitCSVFactory::GetVariable(unit_key, "Mass", 1.0f);
    Momentofinertia = Mass;
    

    // Hull
    float temp_hull = UnitCSVFactory::GetVariable(unit_key, "Hull", 0.0f);
    float hull_values[1] = {temp_hull};
    hull->UpdateFacets(1, hull_values);

    specInterdiction = UnitCSVFactory::GetVariable(unit_key, "Spec_Interdiction", 0.0f);


    // Armor
    // We support 3 options:
    // 1. Minimized armor = x (single value). 
    // 2. New detailed armor (Front, back, left, right).
    // 3. Old detailed (Front-left-top, ...). 8 facets converted to 4. 
    const std::string armor_single_value_string = UnitCSVFactory::GetVariable(unit_key, "armor", std::string());

    if(armor_single_value_string != "") {
        // Minimized
        const double armor_single_value = std::stod(armor_single_value_string, 0);
        armor->UpdateFacets(armor_single_value);
    } else {
        // Try new
        std::string armor_keys[] = {"armor_front", "armor_back",
            "armor_left", "armor_right"};
        bool new_form = true;
        for (int i = 0; i < 4; i++) {
            const std::string armor_string_value = UnitCSVFactory::GetVariable(unit_key, armor_keys[i], std::string());
            if(armor_string_value.empty()) {
                new_form = false;
                break;
            }

            double armor_value = std::stod(armor_string_value);
            armor->facets[i].health = armor_value;
            armor->facets[i].max_health = armor_value;
            armor->facets[i].adjusted_health = armor_value;
        }

        // Fallback to old
        if(!new_form) {
            const std::string armor_keys[] = {"Armor_Front_Top_Left", 
                                              "Armor_Front_Top_Right",
                                              "Armor_Front_Bottom_Left", 
                                              "Armor_Front_Bottom_Right",
                                              "Armor_Back_Top_Left", 
                                              "Armor_Back_Top_Right",
                                              "Armor_Back_Bottom_Left", 
                                              "Armor_Back_Bottom_Right"};

            double old_armor_values[8];
            float new_armor_values[4];
            for (int i = 0; i < 8; i++) {
                old_armor_values[i] = UnitCSVFactory::GetVariable(unit_key, armor_keys[i], 0.0f);
            }

            // Conversion is tricky because new values are a square and old values are
            // a two layered diamond.
            new_armor_values[0] = (old_armor_values[0] + old_armor_values[1] + old_armor_values[2] + old_armor_values[3])/2;
            new_armor_values[1] = (old_armor_values[4] + old_armor_values[5] + old_armor_values[5] + old_armor_values[5])/2;
            new_armor_values[2] = (old_armor_values[0] + old_armor_values[2] + old_armor_values[4] + old_armor_values[6])/2;
            new_armor_values[3] = (old_armor_values[1] + old_armor_values[3] + old_armor_values[5] + old_armor_values[7])/2;
            armor->UpdateFacets(4,new_armor_values);
        }
    }
       
    // shield

    // We support 3 options:
    // 1. Minimized shield_strength = x (single value). 
    // 2. New detailed shield (Front, back, left, right).
    // 3. Old detailed (Front-left-top, ...). 4/2 facets converted to 4/2. 
    
    // Some basic shield variables
    // TODO: lib_damage figure out how leak and efficiency work
    //char leak = static_cast<char>(UnitCSVFactory::GetVariable(unit_key, "Shield_Leak", 0.0f) * 100);

    float regeneration = UnitCSVFactory::GetVariable(unit_key, "Shield_Recharge", 0.0f);

    // This is necessary for upgrading shields, as it's done with an ugly macro in
    // unit_generic STDUPGRADE
    shield_regeneration = regeneration;
    shield->UpdateRegeneration(regeneration);
    //float efficiency = UnitCSVFactory::GetVariable(unit_key, "Shield_Efficiency", 1.0f );

    // Get shield count
    std::map<std::string, float> shield_sections{};
    std::vector<std::string> shield_string_values{};

    const std::string shield_strength_string = UnitCSVFactory::GetVariable(unit_key, "shield_strength", std::string());
    const std::string shield_facets_string = UnitCSVFactory::GetVariable(unit_key, "shield_facets", std::string());
    
    if(!shield_facets_string.empty()) {
        try {
            int shield_facets = std::stoi(shield_facets_string);
            shield->number_of_facets = shield_facets;
        } catch (std::invalid_argument const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_facets_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_facets_string));
            shield->number_of_facets = 1;
        } catch (std::out_of_range const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_facets_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_facets_string));
            shield->number_of_facets = 1;
        }
    }

    if(!shield_strength_string.empty()) {
        try {
            int shield_strength = std::stoi(shield_strength_string);
            shield->UpdateFacets(static_cast<float>(shield_strength));
        } catch (std::invalid_argument const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_strength_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_strength_string));
            shield->UpdateFacets(0.0F);
        } catch (std::out_of_range const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_strength_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_strength_string));
            shield->UpdateFacets(0.0F);
        }
    } else if(!shield_facets_string.empty()) {
        // Try new longform
        std::vector<float> shield_values{};
        std::string shield_keys[] = {"shield_front", "shield_back",
            "shield_left", "shield_right"};
        
        for (int i = 0; i < shield->number_of_facets; ++i) {
            const std::string shield_string_value = UnitCSVFactory::GetVariable(unit_key, shield_keys[i], std::string());
            if (shield_string_value.empty()) {
                shield_sections[shield_keys[i]] = 0.0F;
                shield_values.emplace_back(0.0F);
            } else {
                try {
                    float tmp = static_cast<float>(std::stoi(shield_string_value));
                    shield_sections[shield_keys[i]] = tmp;
                    shield_values.emplace_back(tmp);
                } catch (const std::invalid_argument& ex) {
                    VS_LOG(error, (boost::format("%1%: Unable to convert shield value '%2%' to a number: %3%") % __FUNCTION__ % shield_string_value % ex.what()));
                    shield_sections[shield_keys[i]] = 0.0F;
                    shield_values.emplace_back(0.0F);
                } catch (std::out_of_range const& ex) {
                    VS_LOG(error, (boost::format("%1%: Unable to convert shield value '%2%' to a number: %3%") % __FUNCTION__ % shield_string_value % ex.what()));
                    shield_sections[shield_keys[i]] = 0.0F;
                    shield_values.emplace_back(0.0F);
                }
            }
        }

        if (shield->number_of_facets == 4 || shield->number_of_facets == 2) {
            shield->UpdateFacets(shield->number_of_facets, shield_values.data());
        }
    } else {
        // Fallback to old shield_keys
        int shield_count = 0;
        std::vector<std::string> tmp_keys{"Shield_Front_Top_Right", "Shield_Back_Top_Left", "Shield_Front_Bottom_Right", "Shield_Front_Bottom_Left"};
        std::vector<float> shield_values{};

        try {
            for (auto &key : tmp_keys) {
                std::string tmp_string_value = UnitCSVFactory::GetVariable(unit_key, key, std::string());
                float tmp_float_value = std::stof(tmp_string_value);
                shield_sections[key] = tmp_float_value;
                shield_values.emplace_back(tmp_float_value);
                shield_string_values.emplace_back(tmp_string_value);
                ++shield_count;
            }
        } catch (std::exception const& ex) {
            shield_count = 0;
            shield_string_values.clear();
            shield_values.clear();
            shield_sections.clear();
            tmp_keys.clear();
            tmp_keys.emplace_back("shield_front");
            tmp_keys.emplace_back("shield_back");
            tmp_keys.emplace_back("shield_left");
            tmp_keys.emplace_back("shield_right");
            try {
                for (auto &key : tmp_keys) {
                    std::string tmp_string_value = UnitCSVFactory::GetVariable(unit_key, key, std::string());
                    float tmp_float_value = std::stof(tmp_string_value);
                    shield_sections[key] = tmp_float_value;
                    shield_values.emplace_back(tmp_float_value);
                    shield_string_values.emplace_back(tmp_string_value);
                    ++shield_count;
                }
            } catch (std::exception const& ex) {
                VS_LOG(error, (boost::format("%1%: %2% trying to parse shield facets") % __FUNCTION__ % ex.what()));
            }
        }

        if (shield_count == 4 || shield_count == 2) {
            shield->number_of_facets = shield_count;
            shield->UpdateFacets(shield_count, shield_values.data());
        }
    }

    // Energy 
    // TODO: The following code has a bug.
    // It will set the max of the component as the current value loaded from the 
    // CSV. If the component is damaged, this will be lower than the original value.
    fuel.Load("", unit_key);
    energy.Load("", unit_key);
    ftl_energy.Load("", unit_key);
    reactor.Load("", unit_key);

    // End Energy

    graphicOptions.MinWarpMultiplier = UnitCSVFactory::GetVariable(unit_key, "Warp_Min_Multiplier", 1.0f);
    graphicOptions.MaxWarpMultiplier = UnitCSVFactory::GetVariable(unit_key, "Warp_Max_Multiplier", 1.0f);

    // Begin Drive Section
    // Afterburner
    afterburner = Afterburner(GetSource(ComponentType::Afterburner, &fuel, &energy, &ftl_energy));
    afterburner.Load("", unit_key);

    drive = Drive(GetSource(ComponentType::Drive, &fuel, &energy, &ftl_energy));
    drive.Load("", unit_key);

    ftl_drive.Load("", unit_key);
    jump_drive.Load("", unit_key);
    
    
    forcejump = UnitCSVFactory::GetVariable(unit_key, "Wormhole", false);
    graphicOptions.RecurseIntoSubUnitsOnCollision = UnitCSVFactory::GetVariable(unit_key,
            "Collide_Subunits",
            graphicOptions.RecurseIntoSubUnitsOnCollision
                    ? true : false) ? 1 : 0;
            
    // End Drive Section

    computer.itts = UnitCSVFactory::GetVariable(unit_key, "ITTS", true);
    computer.radar.canlock = UnitCSVFactory::GetVariable(unit_key, "Can_Lock", true);


    // The Radar_Color column in the units.csv has been changed from a
    // boolean value to a string. The boolean values are supported for
    // backwardscompatibility.
    // When we save this setting, it is simply converted from an integer
    // number to a string, and we need to support this as well.
    std::string iffval = UnitCSVFactory::GetVariable(unit_key, "Radar_Color", std::string());

    if ((iffval.empty()) || (iffval == "FALSE") || (iffval == "0")) {
        computer.radar.capability = Computer::RADARLIM::Capability::IFF_NONE;
    } else if ((iffval == "TRUE") || (iffval == "1")) {
        computer.radar.capability = Computer::RADARLIM::Capability::IFF_SPHERE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE;
    } else if (iffval == "THREAT") {
        computer.radar.capability = Computer::RADARLIM::Capability::IFF_SPHERE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE
                | Computer::RADARLIM::Capability::IFF_THREAT_ASSESSMENT;
    } else if (iffval == "BUBBLE_THREAT") {
        computer.radar.capability = Computer::RADARLIM::Capability::IFF_BUBBLE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE
                | Computer::RADARLIM::Capability::IFF_OBJECT_RECOGNITION
                | Computer::RADARLIM::Capability::IFF_THREAT_ASSESSMENT;
    } else if (iffval == "PLANE") {
        computer.radar.capability = Computer::RADARLIM::Capability::IFF_PLANE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE;
    } else if (iffval == "PLANE_THREAT") {
        computer.radar.capability
                = Computer::RADARLIM::Capability::IFF_PLANE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE
                | Computer::RADARLIM::Capability::IFF_OBJECT_RECOGNITION
                | Computer::RADARLIM::Capability::IFF_THREAT_ASSESSMENT;
    } else {
        unsigned int value = stoi(iffval, 0);
        if (value == 0) {
            // Unknown value
            computer.radar.capability = Computer::RADARLIM::Capability::IFF_NONE;
        } else {
            computer.radar.capability = value;
        }
    }

    computer.radar.maxrange = UnitCSVFactory::GetVariable(unit_key, "Radar_Range", FLT_MAX);
    computer.radar.maxcone = cos(UnitCSVFactory::GetVariable(unit_key, "Max_Cone", 180.0f) * M_PI / 180);
    computer.radar.trackingcone = cos(UnitCSVFactory::GetVariable(unit_key, "Tracking_Cone", 180.0f) * M_PI / 180);
    computer.radar.lockcone = cos(UnitCSVFactory::GetVariable(unit_key, "Lock_Cone", 180.0f) * M_PI / 180);

    const static bool warp_energy_for_cloak = configuration()->warp_config.use_warp_energy_for_cloak;
    cloak = Cloak(unit_key, (warp_energy_for_cloak ? &ftl_energy : &energy));

    repair_droid = UnitCSVFactory::GetVariable(unit_key, "Repair_Droid", 0);
    ecm = UnitCSVFactory::GetVariable(unit_key, "ECM_Rating", 0);

    this->HeatSink = UnitCSVFactory::GetVariable(unit_key, "Heat_Sink_Rating", 0.0f);
    if (ecm < 0) {
        ecm *= -1;
    }
    if (pImage->cockpit_damage) {
        std::string hud_functionality = UnitCSVFactory::GetVariable(unit_key, "Hud_Functionality", std::string());
        std::string
                max_hud_functionality = UnitCSVFactory::GetVariable(unit_key, "Max_Hud_Functionality", std::string());

        HudDamage(pImage->cockpit_damage, hud_functionality);
        HudDamage(pImage->cockpit_damage + 1 + MAXVDUS + UnitImages<void>::NUMGAUGES, max_hud_functionality);
    }
    LifeSupportFunctionality = UnitCSVFactory::GetVariable(unit_key, "Lifesupport_Functionality", 1.0f);
    LifeSupportFunctionalityMax = UnitCSVFactory::GetVariable(unit_key, "Max_Lifesupport_Functionality", 1.0f);
    CommFunctionality = UnitCSVFactory::GetVariable(unit_key, "Comm_Functionality", 1.0f);
    CommFunctionalityMax = UnitCSVFactory::GetVariable(unit_key, "Max_Comm_Functionality", 1.0f);
    fireControlFunctionality = UnitCSVFactory::GetVariable(unit_key, "FireControl_Functionality", 1.0f);
    fireControlFunctionalityMax = UnitCSVFactory::GetVariable(unit_key, "Max_FireControl_Functionality", 1.0f);
    SPECDriveFunctionality = UnitCSVFactory::GetVariable(unit_key, "SPECDrive_Functionality", 1.0f);
    SPECDriveFunctionalityMax = UnitCSVFactory::GetVariable(unit_key, "Max_SPECDrive_Functionality", 1.0f);
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
        const std::string expani = configuration()->graphics_config.explosion_animation;
        cache_ani(expani);
    }
    AddLights(this, xml, UnitCSVFactory::GetVariable(unit_key, "Light", std::string()));
    xml.shieldmesh_str = UnitCSVFactory::GetVariable(unit_key, "Shield_Mesh", std::string());
    if (xml.shieldmesh_str.length()) {
        addShieldMesh(&xml, xml.shieldmesh_str.c_str(), xml.unitscale, faction, getFlightgroup());
        meshdata.back() = xml.shieldmesh;
    } else {
        const int shieldstacks = configuration()->graphics_config.shield_detail;
        const std::string& shieldtex = configuration()->graphics_config.shield_texture;
        const std::string& shieldtechnique = configuration()->graphics_config.shield_technique;
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
    std::map<std::string, std::string> unit = UnitCSVFactory::GetUnit(name);
    string val;

    // Textual Descriptions
    unit["Key"] = unit_key;            
    unit["Name"] = unit_name;
    unit["Textual_Description"] = unit_description; // Used in ship view

    //mutable things
    unit["Equipment_Space"] = XMLSupport::tostring(equipment_volume);
    unit["Hold_Volume"] = XMLSupport::tostring(CargoVolume);
    unit["Hidden_Hold_Volume"] = XMLSupport::tostring(HiddenCargoVolume);
    unit["Upgrade_Storage_Volume"] = XMLSupport::tostring(UpgradeVolume);
    string mountstr;
    double unitScale = stof(unit["Unit_Scale"], 1);
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
        string carg;
        for (unsigned int i = 0; i < numCargo(); ++i) {
            Cargo *c = &GetCargo(i);
            char tmp[2048];
            sprintf(tmp, ";%f;%d;%f;%f;%f;%f;;%s;%s}",
                    c->GetPrice(),
                    c->GetQuantity(),
                    c->GetMass(),
                    c->GetVolume(),
                    c->GetFunctionality(),
                    c->GetMaxFunctionality(),
                    c->GetMissionFlag() ? "true" : "false",
                    c->GetInstalled() ? "true" : "false"
            );
            carg += "{" + c->GetName() + ";" + c->GetCategory() + tmp;
        }
        unit["Cargo"] = carg;
    }
    unit["Mass"] = tos(Mass);
    unit["Hull"] = tos(GetHullLayer().facets[0].health);
    unit["Spec_Interdiction"] = tos(specInterdiction);

    // TODO: lib_damage figure out if this is correctly assigned
    unit["armor_front"] = tos(GetArmorLayer().facets[0].health);
    unit["armor_back"] = tos(GetArmorLayer().facets[1].health);
    unit["armor_left"] = tos(GetArmorLayer().facets[2].health);
    unit["armor_right"] = tos(GetArmorLayer().facets[3].health);
    
    unit["shield_facets"] = std::to_string(shield->number_of_facets);
    switch (shield->number_of_facets) {
        case 4:
            unit["shield_left"] = tos(GetShieldLayer().facets[2].max_health);
            unit["shield_right"] = tos(GetShieldLayer().facets[3].max_health);
            // Fallthrough     
        case 2:
            unit["shield_front"] = tos(GetShieldLayer().facets[0].max_health);
            unit["shield_back"] = tos(GetShieldLayer().facets[1].max_health);
    }


    //TODO: lib_damage shield leak and efficiency
    unit["Shield_Leak"] = tos(0); //tos( shield.leak/100.0 );
    unit["Shield_Efficiency"] = tos(1); //tos( shield.efficiency );
    unit["Shield_Recharge"] = tos(shield->GetRegeneration()); //tos( shield.recharge );
    
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


    unit["Wormhole"] = tos(forcejump != 0);
    
    
    unit["ITTS"] = tos(computer.itts);
    unit["Can_Lock"] = tos(computer.radar.canlock);
    unit["Radar_Color"] = std::to_string(computer.radar.capability);
    unit["Radar_Range"] = tos(computer.radar.maxrange);
    unit["Tracking_Cone"] = tos(acos(computer.radar.trackingcone) * 180. / M_PI);
    unit["Max_Cone"] = tos(acos(computer.radar.maxcone) * 180. / M_PI);
    unit["Lock_Cone"] = tos(acos(computer.radar.lockcone) * 180. / M_PI);

    cloak.SaveToCSV(unit);
    unit["Repair_Droid"] = tos(repair_droid);
    unit["ECM_Rating"] = tos(ecm > 0 ? ecm : -ecm);
    unit["Hud_Functionality"] = WriteHudDamage(this);
    unit["Max_Hud_Functionality"] = WriteHudDamageFunc(this);
    unit["Heat_Sink_Rating"] = tos(this->HeatSink);
    unit["Lifesupport_Functionality"] = tos(LifeSupportFunctionality);
    unit["Max_Lifesupport_Functionality"] = tos(LifeSupportFunctionalityMax);
    unit["Comm_Functionality"] = tos(CommFunctionality);
    unit["Max_Comm_Functionality"] = tos(CommFunctionalityMax);
    unit["Comm_Functionality"] = tos(CommFunctionality);
    unit["Max_Comm_Functionality"] = tos(CommFunctionalityMax);
    unit["FireControl_Functionality"] = tos(fireControlFunctionality);
    unit["Max_FireControl_Functionality"] = tos(fireControlFunctionalityMax);
    unit["SPECDrive_Functionality"] = tos(SPECDriveFunctionality);
    unit["Max_SPECDrive_Functionality"] = tos(SPECDriveFunctionalityMax);
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
