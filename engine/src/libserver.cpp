/*
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

#include "root_generic/vs_globals.h"
#include "root_generic/configxml.h"
#include "cmd/unit_generic.h"

VegaConfig *createVegaConfig(const char *file) {
    return new VegaConfig(file);
}

class Music;
class Unit;
class Animation;

void UpdateAnimatedTexture() {
}

void TerrainCollide() {
}

void UpdateTerrain() {
}

void UpdateCameraSnds() {
}

void NebulaUpdate(StarSystem *ss) {
}

void SwitchUnits2(Unit *nw) {
}

void DoCockpitKeys() {
}

void bootstrap_draw(const std::string &message, Animation *SplashScreen) {
}

void createObjects(std::vector<std::string> &playersaveunit,
        std::vector<StarSystem *> &ssys,
        std::vector<QVector> &savedloc,
        vector<vector<string> > &savefiles) {
}

void disableTerrainDraw(ContinuousTerrain *ct) {
}

void /*GFXDRVAPI*/ GFXLight::SetProperties(enum LIGHT_TARGET lighttarg, const GFXColor &color) {
}

