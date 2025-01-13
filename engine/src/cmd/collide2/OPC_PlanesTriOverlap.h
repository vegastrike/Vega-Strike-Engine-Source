/*
 * OPC_PlanesTriOverlap.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NO HEADER GUARD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Planes-triangle overlap test.
 *	\param		in_clip_mask	[in] bitmask for active planes
 *	\return		TRUE if triangle overlap planes
 *	\warning	THIS IS A CONSERVATIVE TEST !! Some triangles will be returned as intersecting, while they're not!
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool PlanesCollider::PlanesTriOverlap(uint32_t in_clip_mask) {
    // Stats
    mNbVolumePrimTests++;

    const Plane *p = mPlanes;
    uint32_t Mask = 1;

    while (Mask <= in_clip_mask) {
        if (in_clip_mask & Mask) {
            float d0 = p->Distance(*mVP.Vertex[0]);
            float d1 = p->Distance(*mVP.Vertex[1]);
            float d2 = p->Distance(*mVP.Vertex[2]);
            if (d0 > 0.0f && d1 > 0.0f && d2 > 0.0f) {
                return FALSE;
            }
//			if(!(IR(d0)&SIGN_BITMASK) && !(IR(d1)&SIGN_BITMASK) && !(IR(d2)&SIGN_BITMASK))	return FALSE;
        }
        Mask += Mask;
        p++;
    }
/*
	for(uint32_t i=0;i<6;i++)
	{
		float d0 = p[i].Distance(mLeafVerts[0]);
		float d1 = p[i].Distance(mLeafVerts[1]);
		float d2 = p[i].Distance(mLeafVerts[2]);
		if(d0>0.0f && d1>0.0f && d2>0.0f)	return false;
	}
*/
    return TRUE;
}
