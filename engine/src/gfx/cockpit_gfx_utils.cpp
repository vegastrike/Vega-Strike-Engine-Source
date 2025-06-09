// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/*
 * cockpit_gfx_utils.cpp
 *
 * Copyright (C) 2020-2025 Daniel Horn, Roy Falk, Stephen G. Tuggy, and
 * other Vega Strike contributors
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


#include "gfx/cockpit_gfx_utils.h"

#include "configuration/game_config.h"
#include "configuration/configuration.h"
#include "src/gfxlib.h"
#include "src/vs_math.h"

VertexBuilder<> GetCross(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& size) {
    static VertexBuilder<> verts;
    verts.clear();
    verts.insert( location+cam_p*size );
    verts.insert( location+.125*cam_p*size );
    verts.insert( location-cam_p*size );
    verts.insert( location-.125*cam_p*size );
    verts.insert( location+cam_q*size );
    verts.insert( location+.125*cam_q*size );
    verts.insert( location-cam_q*size );
    verts.insert( location-.125*cam_q*size );
    verts.insert( location+.0625*cam_q*size );
    verts.insert( location+.0625*cam_p*size );
    verts.insert( location-.0625*cam_q*size );
    verts.insert( location-.0625*cam_p*size );
    verts.insert( location+.9*cam_p*size+.125*cam_q*size );
    verts.insert( location+.9*cam_p*size-.125*cam_q*size );
    verts.insert( location-.9*cam_p*size+.125*cam_q*size );
    verts.insert( location-.9*cam_p*size-.125*cam_q*size );
    verts.insert( location+.9*cam_q*size+.125*cam_p*size );
    verts.insert( location+.9*cam_q*size-.125*cam_p*size );
    verts.insert( location-.9*cam_q*size+.125*cam_p*size );
    verts.insert( location-.9*cam_q*size-.125*cam_p*size );

    return verts;
}


VertexBuilder<> GetDiamond(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size) {
    static VertexBuilder<> verts;
    verts.clear();
    float size = r_size/1.41;
    verts.insert( location+(.75*cam_p+cam_q).Cast()*size );
    verts.insert( location+(cam_p+.75*cam_q).Cast()*size );
    verts.insert( location+(cam_p-.75*cam_q).Cast()*size );
    verts.insert( location+(.75*cam_p-cam_q).Cast()*size );
    verts.insert( location+(-.75*cam_p-cam_q).Cast()*size );
    verts.insert( location+(-cam_p-.75*cam_q).Cast()*size );
    verts.insert( location+(.75*cam_q-cam_p).Cast()*size );
    verts.insert( location+(cam_q-.75*cam_p).Cast()*size );
    verts.insert( location+(.75*cam_p+cam_q).Cast()*size );

    return verts;
}


VertexBuilder<> GetRectangle(const QVector &location, const Vector& cam_p,
                            const Vector& cam_q, const float& r_size) {
    static VertexBuilder<> verts;
    verts.clear();
    verts.insert( location+(cam_p+cam_q).Cast()*r_size );
    verts.insert( location+(cam_p-cam_q).Cast()*r_size );
    verts.insert( location+(-cam_p-cam_q).Cast()*r_size );
    verts.insert( location+(cam_q-cam_p).Cast()*r_size );
    verts.insert( location+(cam_p+cam_q).Cast()*r_size );
    return verts;
}

/*  ┌   ┐

    └   ┘
*/
VertexBuilder<> GetOpenRectangle(const QVector &location, const Vector& cam_p,
                            const Vector& cam_q, const float& r_size) {
    static VertexBuilder<> verts;
    verts.clear();

    verts.insert( location+(cam_p+cam_q).Cast()*r_size );
    verts.insert( location+(cam_p+.66*cam_q).Cast()*r_size );

    verts.insert( location+(cam_p-cam_q).Cast()*r_size );
    verts.insert( location+(cam_p-.66*cam_q).Cast()*r_size );

    verts.insert( location+(-cam_p-cam_q).Cast()*r_size );
    verts.insert( location+(-cam_p-.66*cam_q).Cast()*r_size );

    verts.insert( location+(cam_q-cam_p).Cast()*r_size );
    verts.insert( location+(cam_q-.66*cam_p).Cast()*r_size );

    verts.insert( location+(cam_p+cam_q).Cast()*r_size );
    verts.insert( location+(cam_p+.66*cam_q).Cast()*r_size );

    verts.insert( location+(cam_p+cam_q).Cast()*r_size );
    verts.insert( location+(.66*cam_p+cam_q).Cast()*r_size );

    verts.insert( location+(cam_p-cam_q).Cast()*r_size );
    verts.insert( location+(.66*cam_p-cam_q).Cast()*r_size );

    verts.insert( location+(-cam_p-cam_q).Cast()*r_size );
    verts.insert( location+(-.66*cam_p-cam_q).Cast()*r_size );

    verts.insert( location+(cam_q-cam_p).Cast()*r_size );
    verts.insert( location+(.66*cam_q-cam_p).Cast()*r_size );

    verts.insert( location+(cam_p+cam_q).Cast()*r_size );
    verts.insert( location+(.66*cam_p+cam_q).Cast()*r_size );

    return verts;
}


/*
 * The locking icon will grow and shrink as lock is achieved.
 */
VertexBuilder<> GetLockingIcon(const QVector &location, const Vector& cam_p,
                            const Vector& cam_q, const float& r_size,
                            const float& lock_percent) {
    const bool bracket_inner_or_outer = configuration()->graphics.hud.rotating_bracket_inner;
    const float bracket_width = configuration()->graphics.hud.rotating_bracket_width;

    const float bounded_r_size = std::max(r_size, static_cast<float>(configuration()->graphics.hud.min_lock_box_size));
    const float inner_distance = configuration()->graphics.hud.min_missile_bracket_size + (configuration()->graphics.hud.max_missile_bracket_size - configuration()->graphics.hud.min_missile_bracket_size) * lock_percent;
    const float outer_distance = inner_distance + configuration()->graphics.hud.rotating_bracket_size;
    const float bracket_distance = (bracket_inner_or_outer ? inner_distance : outer_distance);

    const float adjusted_inner_distance = inner_distance * bounded_r_size;
    const float adjusted_outer_distance = outer_distance * bounded_r_size;
    const float adjusted_bracket_distance = bracket_distance * bounded_r_size;
    const float adjusted_bracket_width = bracket_width * bounded_r_size;

    static VertexBuilder<> verts;
    verts.clear();

    verts.insert( location + cam_p * adjusted_inner_distance );
    verts.insert( location + cam_p * adjusted_outer_distance );

    verts.insert( location + cam_p * adjusted_bracket_distance + cam_q * adjusted_bracket_width );
    verts.insert( location + cam_p * adjusted_bracket_distance - cam_q * adjusted_bracket_width );

    verts.insert( location - cam_p * adjusted_inner_distance );
    verts.insert( location - cam_p * adjusted_outer_distance );

    verts.insert( location - cam_p * adjusted_bracket_distance + cam_q * adjusted_bracket_width );
    verts.insert( location - cam_p * adjusted_bracket_distance-cam_q * adjusted_bracket_width );

    verts.insert( location + cam_q * adjusted_inner_distance );
    verts.insert( location + cam_q * adjusted_outer_distance );

    verts.insert( location + cam_q * adjusted_bracket_distance + cam_p * adjusted_bracket_width );
    verts.insert( location + cam_q * adjusted_bracket_distance - cam_p * adjusted_bracket_width );

    verts.insert( location - cam_q * adjusted_inner_distance );
    verts.insert( location - cam_q * adjusted_outer_distance );

    verts.insert( location - cam_q * adjusted_bracket_distance + cam_p * adjusted_bracket_width );
    verts.insert( location-cam_q * adjusted_bracket_distance - cam_p * adjusted_bracket_width );

    return verts;
}

/*
 * The locking icon will grow and shrink as lock as achieved.
 * The animated locking icon will also move from the reticle to the target and apparently rotate.
 */
VertexBuilder<> GetAnimatedLockingIcon(const QVector &location, const Vector& cam_p,
                            const Vector& cam_q, const Vector& cam_r,
                            const float& r_size, const float& lock_percent) {
    const float lock_line = configuration()->graphics.hud.lock_confirm_line_length;
    const double diamond_size = configuration()->graphics.hud.diamond_size;
    const float theta_speed = configuration()->graphics.hud.diamond_rotation_speed;

    const float max = diamond_size * r_size * 0.75 * configuration()->graphics.hud.min_missile_bracket_size;
    const float coord = configuration()->graphics.hud.min_missile_bracket_size + (configuration()->graphics.hud.max_missile_bracket_size - configuration()->graphics.hud.min_missile_bracket_size) * lock_percent;
    const double rtot = 1.0 / sqrtf(2.0);

    //this causes the rotation!
    const float theta = 4.0 * M_PI * lock_percent * theta_speed;
    const Vector lock_box( -cos( theta )*rtot, -rtot, sin( theta )*rtot );

    QVector t_lock_box( rtot*lock_box.i+rtot*lock_box.j, rtot*lock_box.j-rtot*lock_box.i, lock_box.k );
    // FIXME: Are i and j reversed here from what they should be? -- SGT 2022-05-08
    QVector s_lock_box( t_lock_box.j, t_lock_box.i, t_lock_box.k );
    t_lock_box = (t_lock_box.i*cam_p+t_lock_box.j*cam_q+t_lock_box.k*cam_r).Cast();
    s_lock_box = (s_lock_box.i*cam_p+s_lock_box.j*cam_q+s_lock_box.k*cam_r).Cast();

    const double r_1_size = std::max(r_size * configuration()->graphics.hud.rotating_bracket_size, configuration()->graphics.hud.min_lock_box_size);

    t_lock_box *= r_1_size;
    s_lock_box *= r_1_size;

    const QVector q_cam_p(cam_p.Cast());
    const QVector q_cam_q(cam_q.Cast());

    QVector origin = (cam_p+cam_q).Cast()*(r_size*coord);
    QVector origin1 = (cam_p-cam_q).Cast()*(r_size*coord);

    SetThickness( ShapeType::Diamond );

    static VertexBuilder<> verts;
    verts.clear();

    if (lock_percent == 0) {
        verts.insert( location+q_cam_q*max*lock_line );
        verts.insert( location+q_cam_q*max );
        verts.insert( location+origin+t_lock_box );
        verts.insert( location+origin );
        verts.insert( location+origin+s_lock_box );
        verts.insert( location+q_cam_p*max );
        verts.insert( location+q_cam_p*max*lock_line );
        verts.insert( location-q_cam_p*max );
        verts.insert( location-origin-s_lock_box );
        verts.insert( location-origin );
        verts.insert( location-origin-t_lock_box );
        verts.insert( location-q_cam_q*max );
        verts.insert( location-q_cam_q*max*lock_line );
        verts.insert( location+origin1+t_lock_box );
        verts.insert( location+origin1 );
        verts.insert( location+origin1-s_lock_box );
        verts.insert( location-q_cam_p*max*lock_line );
        verts.insert( location-origin1+s_lock_box );
        verts.insert( location-origin1 );
        verts.insert( location-origin1-t_lock_box );
    } else {
        verts.insert( location+origin+t_lock_box );
        verts.insert( location+origin );
        verts.insert( location+origin+s_lock_box );
        verts.insert( location-origin-s_lock_box );
        verts.insert( location-origin );
        verts.insert( location-origin-t_lock_box );
        verts.insert( location+origin1+t_lock_box );
        verts.insert( location+origin1 );
        verts.insert( location+origin1-s_lock_box );
        verts.insert( location-origin1+s_lock_box );
        verts.insert( location-origin1 );
        verts.insert( location-origin1-t_lock_box );
    }

    return verts;
}


void SetThickness(ShapeType type) {
    switch(type) {
        case ShapeType::Box:
            GFXLineWidth(configuration()->graphics.hud.box_line_thickness);
            break;
        case ShapeType::Diamond:
            GFXLineWidth(configuration()->graphics.hud.diamond_line_thickness);
            break;
        case ShapeType::Cross:
            GFXLineWidth(configuration()->graphics.hud.nav_cross_line_thickness);
            break;
        default:
            GFXLineWidth(1.0F);
            break;
    }
}

