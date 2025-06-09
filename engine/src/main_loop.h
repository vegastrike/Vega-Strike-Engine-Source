/*
 * main_loop.h
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
#ifndef VEGA_STRIKE_ENGINE_MAIN_LOOP_H
#define VEGA_STRIKE_ENGINE_MAIN_LOOP_H

#include "src/in.h"
#include "cmd/unit_generic.h"
void bootstrap_draw(const std::string &message, Animation *SplashScreen = NULL);

namespace CockpitKeys {
void SkipMusicTrack(const KBData &, KBSTATE newState);
void PitchDown(const KBData &, KBSTATE newState);
void PitchUp(const KBData &, KBSTATE newState);
void YawLeft(const KBData &, KBSTATE newState);
void YawRight(const KBData &, KBSTATE newState);
void LookDown(const KBData &, KBSTATE newState);
void LookUp(const KBData &, KBSTATE newState);
void LookLeft(const KBData &, KBSTATE newState);
void LookRight(const KBData &, KBSTATE newState);
void Inside(const KBData &, KBSTATE newState);
void ZoomOut(const KBData &, KBSTATE newState);
void ZoomIn(const KBData &, KBSTATE newState);
void ZoomReset(const KBData &, KBSTATE newState);
void InsideLeft(const KBData &, KBSTATE newState);
void InsideRight(const KBData &, KBSTATE newState);
void InsideBack(const KBData &, KBSTATE newState);
void SwitchLVDU(const KBData &, KBSTATE newState);
void SwitchRVDU(const KBData &, KBSTATE newState);
void SwitchMVDU(const KBData &, KBSTATE newState);
void SwitchULVDU(const KBData &, KBSTATE newState);
void SwitchURVDU(const KBData &, KBSTATE newState);
void SwitchUMVDU(const KBData &, KBSTATE newState);
void CommModeVDU(const KBData &, KBSTATE);
void ObjectiveModeVDU(const KBData &, KBSTATE);
void TargetModeVDU(const KBData &, KBSTATE);
void ManifestModeVDU(const KBData &, KBSTATE);
void DamageModeVDU(const KBData &, KBSTATE);
void GunModeVDU(const KBData &, KBSTATE);
void MissileModeVDU(const KBData &, KBSTATE);
void ScanningModeVDU(const KBData &, KBSTATE);
void ReverseGunModeVDU(const KBData &, KBSTATE);
void ReverseMissileModeVDU(const KBData &, KBSTATE);
void ViewModeVDU(const KBData &, KBSTATE);
void ScrollUp(const KBData &, KBSTATE newState);
void ScrollDown(const KBData &, KBSTATE newState);
void Behind(const KBData &, KBSTATE newState);
void Pan(const KBData &, KBSTATE newState);
void PanTarget(const KBData &, KBSTATE newState);
void ViewTarget(const KBData &, KBSTATE newState);
void OutsideTarget(const KBData &, KBSTATE newState);
void Quit(const KBData &, KBSTATE newState);
void TextMessageKey(const KBData &, KBSTATE newState);
void QuitNow();
}

struct SavedUnits;
void AddUnitToSystem(const SavedUnits *su);
void createObjects(std::vector<std::string> &playersaveunit,
        std::vector<StarSystem *> &ssys,
        std::vector<QVector> &savedloc,
        vector<vector<string> > &savefiles);
void destroyObjects();
void InitializeInput();
void main_loop();
void restore_main_loop();

#endif //VEGA_STRIKE_ENGINE_MAIN_LOOP_H
