/*
 * force_feedback_server.cpp
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

/*
 *  Force Feedback support by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include "force_feedback.h"

#include "vegastrike.h"
#include "vsfilesystem.h"
#include "vs_globals.h"

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

