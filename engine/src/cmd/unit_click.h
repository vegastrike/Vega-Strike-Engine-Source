/*
 * unit_click.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_UNIT_CLICK_H
#define VEGA_STRIKE_ENGINE_CMD_UNIT_CLICK_H

#include "vegastrike.h"
#include "gfx/mesh.h"

extern Vector mouseline;
extern vector<Vector> perplines;
Vector MouseCoordinate(int mouseX, int mouseY);

bool Unit::querySphereClickList(int mouseX, int mouseY, float err, Camera *activeCam) const {
    unsigned int i;
    Matrix vw;
    _Universe->AccessCamera()->GetView(vw);
    Vector mousePoint;
#ifdef VARIABLE_LENGTH_PQR
    Vector TargetPoint( cumulative_transformation_matrix[0],
                        cumulative_transformation_matrix[1],
                        cumulative_transformation_matrix[2] );
    float  SizeScaleFactor = sqrtf( TargetPoint.Dot( TargetPoint ) );
#else
    Vector TargetPoint;
#endif

    Vector CamP, CamQ, CamR;
    for (i = 0; i < nummesh(); i++) {
        TargetPoint = Transform(this->cumulative_transformation_matrix, this->meshdata[i]->Position());

        mousePoint = Transform(vw, TargetPoint);
        if (mousePoint.k > 0) {          //z coordinate reversed  -  is in front of camera
            continue;
        }
        mousePoint = MouseCoordinate(mouseX, mouseY);

        activeCam->GetPQR(CamP, CamQ, CamR);
        mousePoint = Transform(CamP, CamQ, CamR, mousePoint);
        CamP = activeCam->GetPosition().Cast();
        mousePoint += CamP;

        TargetPoint = TargetPoint - mousePoint;
        if (TargetPoint.Dot(TargetPoint)
                < err * err
                        + this->meshdata[i]->rSize() * this->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
                        *SizeScaleFactor*SizeScaleFactor
#endif
                        +
#ifdef VARIABLE_LENGTH_PQR
                                SizeScaleFactor*
#endif
                                2 * err * this->meshdata[i]->rSize()
                ) {
            return true;
        }
    }
    const Unit *su;
    for (un_kiter ui = this->viewSubUnits(); (su = *ui); ++ui) {
        if (su->querySphereClickList(mouseX, mouseY, err, activeCam)) {
            return true;
        }
    }
    return false;
}

#endif //VEGA_STRIKE_ENGINE_CMD_UNIT_CLICK_H
