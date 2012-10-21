/*
 * Vega Strike
 * Copyright (C) 2012 Claudio Freire
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _VS_OCCLUSION_H_
#define _VS_OCCLUSION_H_

#include "vs_globals.h"

#include "gfxlib.h"
#include "vec.h"

namespace Occlusion {

    /// Initialize occlusion system for a new frame
    void /*GFXDRVAPI*/ start( );

    /// Finalize occlusion system for a frame, and free up resources
    void /*GFXDRVAPI*/ end( );

    /** 
     * Register an occluder
     * 
     * @param pos The occluder object's center
     * @param rSize The occluder's radius
     * @param significant If false, the occluder may be ignored
     *          if there are other, more significant occluders.
     *          If true, it will be forcibly considered when
     *          rendering all objects. Ie: for planets.
     */
    void /*GFXDRVAPI*/ addOccluder( const QVector &pos, float rSize, bool significant );

    /** 
     * Test occlusion between a light and an object
     * 
     * @param lightPos The light's center
     * @param lightSize The light's radius
     * @param pos The object's center
     * @param rSize The object's radius
     * 
     * @returns An occlusion factor, with 0 being fully occluded 
     *          and 1 being fully clear.
     */
    float /*GFXDRVAPI*/ testOcclusion( const QVector &lightPos, float lightSize, const QVector &pos, float rSize );
    
}

#endif//_VS_OCCLUSION_H_