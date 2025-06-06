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

/*
 *  Force Feedback support by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include "src/force_feedback.h"

#include "src/vegastrike.h"
#include "vegadisk/vsfilesystem.h"
#include "root_generic/vs_globals.h"

#define FF_DOIT 1

ForceFeedback::ForceFeedback() {
}

ForceFeedback::~ForceFeedback() {
}

bool ForceFeedback::haveFF() {
    return false;
}

void ForceFeedback::updateForce(float angle, float strength) {
}

void ForceFeedback::updateSpeedEffect(float strength) {
}

void ForceFeedback::playHit(float angle, float strength) {
}

void ForceFeedback::playAfterburner(bool activate) {
}

void ForceFeedback::playLaser() {
}

void ForceFeedback::playDurationEffect(unsigned int eff_nr, bool activate) {
}

void ForceFeedback::playShortEffect(unsigned int eff_nr) {
}

#if HAVE_FORCE_FEEDBACK

void ForceFeedback::playEffect( unsigned int eff_nr ) {}
void ForceFeedback::stopEffect( unsigned int eff_nr ) {}
void ForceFeedback::init() {}

#endif

