/*
 * click_list.cpp
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


#include "src/gfxlib.h"
#include "click_list.h"
#include "cmd/unit_generic.h"
#include "src/vegastrike.h"
#include "root_generic/vs_globals.h"
#include "gfx/camera.h"
#include "src/universe.h"
// #include "src/vs_logging.h"

extern Vector mouseline;
extern vector<Vector> perplines;

Vector MouseCoordinate(int mouseX, int mouseY) {
    return GFXDeviceToEye(mouseX, mouseY);
}

bool ClickList::queryShip(int mouseX, int mouseY, Unit *ship) {
    Vector mousePoint = MouseCoordinate(mouseX, mouseY);

    //mousePoint.k= -mousePoint.k;
    Vector CamP, CamQ, CamR;
    QVector CamPos;
    _Universe->AccessCamera()->GetPQR(CamP, CamQ, CamR);
    mousePoint = Transform(CamP, CamQ, CamR, mousePoint);
    _Universe->AccessCamera()->GetPosition(CamPos);

    mousePoint.Normalize();
    mouseline = mousePoint + CamPos.Cast();
    int tmp = ship->querySphere(CamP, mousePoint, 0);  //FIXME  bounding spheres seem to be broken
    /* if (tmp) {
        if ( ship->querySphereClickList( CamPos, mousePoint.Cast(), 0 ) ) {
            //camera position is not actually the center of the camera
            // VS_LOG(info, "bounding sphere hit\n");
            return true;
        }
    } POSSIBLE DELETE */
    return false;
}

ClickList::ClickList(StarSystem *parSystem, UnitCollection *parIter) {
    lastSelected = NULL;
    lastCollection = NULL;
    parentSystem = parSystem;
    parentIter = parIter;
}

UnitCollection *ClickList::requestIterator(int minX, int minY, int maxX, int maxY) {
    UnitCollection *uc = new UnitCollection();      ///arrgh dumb last collection thing to cycel through ships
    if (minX == maxX || minY == maxY) {
        return uc;
    }          //nothing in it

    Matrix view;
    float frustmat[16];
    float l, r, b, t, n, f;
    float drivel[16];
    GFXGetFrustumVars(true, &l, &r, &b, &t, &n, &f);
    GFXFrustum(frustmat,
            drivel,
            l
                    * (-2. * minX / configuration().graphics.resolution_x + 1) /*  *g_game.MouseSensitivityX*/,
            r * (2. * maxX / configuration().graphics.resolution_x - 1) /*  *g_game.MouseSensitivityX*/,
            t * (-2. * minY / configuration().graphics.resolution_y + 1) /*  *g_game.MouseSensitivityY*/,
            b * (2. * maxY / configuration().graphics.resolution_y - 1) /*  *g_game.MouseSensitivityY*/,
            n,
            f);
    _Universe->AccessCamera()->GetView(view);
    double frustum[6][4];
    GFXCalculateFrustum(frustum, view, frustmat);
    Unit *un;
    for (un_iter myParent = parentIter->createIterator(); (un = *myParent) != NULL; ++myParent) {
        if ((un)->queryFrustum(frustum)) {
            uc->prepend(un);
        }
    }
    return uc;
}

UnitCollection *ClickList::requestIterator(int mouseX, int mouseY) {
    perplines = vector<Vector>();
    UnitCollection *uc = new UnitCollection();
    Unit *un;
    for (un_iter myParent = parentIter->createIterator(), UAye = uc->createIterator(); (un = *myParent) != NULL;
            ++myParent) {
        if (queryShip(mouseX, mouseY, un)) {
            UAye.preinsert(un);
        }
    }
    return uc;
}

Unit *ClickList::requestShip(int mouseX, int mouseY) {
    bool equalCheck = false;
    UnitCollection *uc = requestIterator(mouseX, mouseY);
    if (lastCollection != NULL) {
        equalCheck = true;
        Unit *lastun;
        Unit *un;
        for (un_iter lastiter = lastCollection->createIterator(), UAye = uc->createIterator();
                (lastun = *lastiter) && (un = *UAye) && equalCheck;
                ++lastiter, ++UAye) {
            if (un != lastun) {
                equalCheck = false;
            }
        }
        delete lastCollection;
    }
    float minDistance = 1e+10;
    float tmpdis;
    Unit *targetUnit = NULL;
    if (equalCheck && lastSelected) {
        //the person clicked the same place and wishes to cycle through units from front to back
        float morethan = lastSelected->getMinDis(_Universe->AccessCamera()
                ->GetPosition());         //parent system for access cam
        Unit *un;
        for (un_iter UAye = uc->createIterator(); (un = *UAye) != NULL; ++UAye) {
            tmpdis = un->getMinDis(_Universe->AccessCamera()
                    ->GetPosition());             //parent_system? FIXME (for access cam
            if (tmpdis > morethan && tmpdis < minDistance) {
                minDistance = tmpdis;
                targetUnit = un;
            }
        }
    }
    if (targetUnit == NULL) {
        //ok the click location is either different, or
        //he clicked on the back of the list and wishes to start over
        Unit *un;
        for (un_iter UAye = uc->createIterator(); (un = *UAye) != NULL; ++UAye) {
            tmpdis = un->getMinDis(_Universe->AccessCamera()->GetPosition());             //parent_system FIXME
            if (tmpdis < minDistance) {
                minDistance = tmpdis;
                targetUnit = un;
            }
        }
    }
    lastCollection = uc;
    lastSelected = targetUnit;
    return targetUnit;
}

