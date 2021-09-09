/**
* unit_click.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
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

#ifndef __UNIT_CLICK_CPP__
#define __UNIT_CLICK_CPP__

#include "vegastrike.h"
#include "gfx/mesh.h"
#include "unit.h"

extern Vector mouseline;
extern vector< Vector >perplines;
Vector MouseCoordinate( int mouseX, int mouseY );


bool GameUnit::querySphereClickList( int mouseX, int mouseY, float err, Camera *activeCam ) const
{
    unsigned int    i;
    Matrix vw;
    _Universe->AccessCamera()->GetView( vw );
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
        TargetPoint = Transform( this->cumulative_transformation_matrix, this->meshdata[i]->Position() );

        mousePoint  = Transform( vw, TargetPoint );
        if (mousePoint.k > 0)          //z coordinate reversed  -  is in front of camera
            continue;
        mousePoint  = MouseCoordinate( mouseX, mouseY );

        activeCam->GetPQR( CamP, CamQ, CamR );
        mousePoint  = Transform( CamP, CamQ, CamR, mousePoint );
        CamP = activeCam->GetPosition().Cast();
        mousePoint += CamP;

        TargetPoint = TargetPoint-mousePoint;
        if (TargetPoint.Dot( TargetPoint )
            < err*err
            +this->meshdata[i]->rSize()*this->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
            *SizeScaleFactor*SizeScaleFactor
#endif
            +
#ifdef VARIABLE_LENGTH_PQR
            SizeScaleFactor*
#endif
            2*err*this->meshdata[i]->rSize()
           )
            return true;
    }
    const Unit *su;
    for (un_kiter ui = this->viewSubUnits(); (su = *ui); ++ui){
        if( su->querySphereClickList( mouseX, mouseY, err, activeCam ) )
            return true;
    }
    return false;
}

#endif

