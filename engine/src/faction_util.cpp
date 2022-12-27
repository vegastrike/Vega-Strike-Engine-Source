/*
 * faction_util.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include <assert.h>

#include "vs_globals.h"
#include "vs_logging.h"
#include "cmd/unit_generic.h"
#include "faction_generic.h"
#include "gfx/aux_texture.h"
#include "cmd/unit_util.h"
#include "cmd/unit_generic.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "cmd/music.h"

// DO NOT PUT INCLUDES AFTER using namespace

using namespace FactionUtil;

Faction::~Faction() {
    delete[] factionname;
    if (contraband) {
        contraband->Kill();
    }
    logo.reset();           // I don't think this is strictly necessary anymore -- Stephen G. Tuggy 2022-12-27
}

vega_types::SharedPtr<Texture> FactionUtil::getForceLogo(int faction) {
    vega_types::SharedPtr<Faction> fac = factions.at(faction);
    if (!fac->logo) {
        if (!fac->logoName.empty()) {
            if (!fac->logoAlphaName.empty()) {
                fac->logo = FactionUtil::createTexture(fac->logoName.c_str(), fac->logoAlphaName.c_str(), true);
            } else {
                fac->logo = FactionUtil::createTexture(fac->logoName.c_str(), true);
            }
        } else {
            fac->logo = FactionUtil::createTexture("white.png", true);
        }
    }
    return factions.at(faction)->logo;
}

//fixme--add squads in here
vega_types::SharedPtr<Texture> FactionUtil::getSquadLogo(int faction) {
    vega_types::SharedPtr<Faction> fac = factions.at(faction);
    if (!fac->secondaryLogo) {
        if (!fac->secondaryLogoName.empty()) {
            if (!fac->secondaryLogoAlphaName.empty()) {
                fac->secondaryLogo = FactionUtil::createTexture(
                        fac->secondaryLogoName.c_str(), fac->secondaryLogoAlphaName.c_str(), true);
            } else {
                fac->secondaryLogo = FactionUtil::createTexture(fac->secondaryLogoName.c_str(), true);
            }
        } else {
            return getForceLogo(faction);
        }
    }
    return factions.at(faction)->secondaryLogo;
}

int FactionUtil::GetNumAnimation(int faction) {
    return factions.at(faction)->comm_faces.size();
}

//COMES FROM FACTION_XML.CPP

std::vector<Animation *> *FactionUtil::GetAnimation(int faction, int n, unsigned char &sex) {
    sex = factions.at(faction)->comm_face_sex[n];
    return &factions.at(faction)->comm_faces[n].animations;
}

std::vector<Animation *> *FactionUtil::GetRandCommAnimation(int faction, Unit *un, unsigned char &sex) {
    bool dockable = UnitUtil::isDockableUnit(un);
    bool base = UnitUtil::getFlightgroupName(un) == "Base";
    int siz = factions.at(faction)->comm_faces.size();
    if (siz > 0) {
        for (int i = 0; i < 8 + siz; ++i) {
            int ind = i < 8 ? rand() % siz : i - 8;
            Faction::comm_face_t *tmp = &factions.at(faction)->comm_faces[ind];
            if (tmp->dockable == Faction::comm_face_t::CEITHER
                    || (tmp->dockable == Faction::comm_face_t::CYES && dockable)
                    || (tmp->dockable == Faction::comm_face_t::CNO && !dockable)) {
                if (tmp->base == Faction::comm_face_t::CEITHER
                        || (tmp->base == Faction::comm_face_t::CYES && base)
                        || (tmp->base == Faction::comm_face_t::CNO && !base)) {
                    return GetAnimation(faction, ind, sex);
                }
            }
            if (tmp->base == Faction::comm_face_t::CYES && base) {
                return GetAnimation(faction,
                        ind,
                        sex);                  //bases may be dockable but we have set dockable_only to no
            }
        }
        VS_LOG(error,
                (boost::format("Error picking comm animation for %1$d faction with base:%2$d dock:%3$d\n") % faction
                        % ((int) base) % ((int) dockable)));
        return GetAnimation(faction, rand() % siz, sex);
    } else {
        sex = 0;
        return NULL;
    }
}

Animation *FactionUtil::GetRandExplosionAnimation(int whichfaction, std::string &which) {
    if (whichfaction < (int) factions.size()) {
        if (factions[whichfaction]->explosion_name.size()) {
            int whichexp = rand() % factions[whichfaction]->explosion_name.size();
            which = factions[whichfaction]->explosion_name[whichexp];
            return factions[whichfaction]->explosion[whichexp].get();
        }
    }
    return NULL;
}

void FactionUtil::LoadFactionPlaylists() {
    for (unsigned int i = 0; i < factions.size(); i++) {
        string fac = FactionUtil::GetFaction(i);
        fac += ".m3u";
        factions[i]->playlist = muzak->Addlist(fac.c_str());
    }
}

Animation *FactionUtil::createAnimation(const char *anim) {
    return new Animation(anim);
}

vega_types::SharedPtr<Texture> FactionUtil::createTexture(const char *tex, bool force) {
    if (force) {
        return vega_types::MakeShared<Texture>(tex, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE);
    } else {
        return vega_types::MakeShared<Texture>(tex, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXFALSE);
    }
}

vega_types::SharedPtr<Texture> FactionUtil::createTexture(const char *tex, const char *tmp, bool force) {
    if (force) {
        return vega_types::MakeShared<Texture>(tex, tmp, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1, 0, GFXTRUE);
    } else {
        return vega_types::MakeShared<Texture>(tex, tmp, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1, 0, GFXFALSE);
    }
}

