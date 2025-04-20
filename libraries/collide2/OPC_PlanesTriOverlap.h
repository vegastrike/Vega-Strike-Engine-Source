///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * OPC_PlanesTriOverlap.h
 *
 * Copyright (C) 2001 Pierre Terdiman
 * Copyright (C) 2021, 2022, 2025 Stephen G. Tuggy
 * Copyright (C) 2023 Benjamen R. Meyer
 *
 * This file is part of OPCODE - Optimized Collision Detection
 * (http://www.codercorner.com/Opcode.htm) and has been
 * incorporated into Vega Strike
 * (https://github.com/vegastrike/Vega-Strike-Engine-Source).
 *
 * Public Domain
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NO HEADER GUARD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Planes-triangle overlap test.
 *  \param		in_clip_mask	[in] bitmask for active planes
 *  \return		TRUE if triangle overlap planes
 *  \warning	THIS IS A CONSERVATIVE TEST !! Some triangles will be returned as intersecting, while they're not!
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
