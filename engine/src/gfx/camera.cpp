/*
 * camera.cpp
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


#include "camera.h"
#include "cmd/unit_generic.h" ///for GetUnit ();
#include "matrix.h"

//Remove GL specific stuff here

#include "vs_globals.h"
#include "audiolib.h"
#include "lin_time.h"

#include <assert.h>     //needed for assert() calls
//#include "planetary_transform.h"  commented out by chuck_starchaser; --never used

#include "vs_logging.h"
#include "vs_exit.h"

//const float PI=3.1415926536;

Camera::Camera(ProjectionType proj) : projectionType(proj), myPhysics(0.1, 0.075, &Coord, &P, &Q, &R) {
    ResetVectors(P, Q, R);
    R = -R;
    Coord.i = 0;
    Coord.j = 0;
    Coord.k = -1;
    velocity = angular_velocity = Vector(0, 0, 0);
    lastpos.Set(0, 0, 0);
    cockpit_offset = 0;
    //////////////////////////////////////////SetPlanetaryTransform( NULL );
    changed = GFXTRUE;
    //SetPosition();
    //SetOrientation();
    Yaw(PI);
    x = y = 0;
    xsize = ysize = 1.0;
    zoom = 1.0;
    fov = g_game.fov;

    lastGFXUpdate.clip = GFXTRUE;
    lastGFXUpdate.updateFrustum = GFXTRUE;
    lastGFXUpdate.centerCamera = GFXFALSE;
    lastGFXUpdate.overrideZFrustum = GFXFALSE;
    lastGFXUpdate.overrideZNear = 0;
    lastGFXUpdate.overrideZFar = 1000000;
}

void Camera::GetPQR(Vector &p1, Vector &q1, Vector &r1) const {
    p1.i = P.i;
    p1.j = P.j;
    p1.k = P.k;
    q1.i = Q.i;
    q1.j = Q.j;
    q1.k = Q.k;
    r1.i = R.i;
    r1.j = R.j;
    r1.k = R.k;
}

void Camera::UpdateGFX(GFXBOOL clip,
        GFXBOOL updateFrustum,
        GFXBOOL centerCamera,
        GFXBOOL overrideZFrustum,
        float overrideZNear,
        float overrideZFar) {
    lastGFXUpdate.clip = clip;
    lastGFXUpdate.updateFrustum = updateFrustum;
    lastGFXUpdate.centerCamera = centerCamera;
    lastGFXUpdate.overrideZFrustum = overrideZFrustum;
    lastGFXUpdate.overrideZNear = overrideZNear;
    lastGFXUpdate.overrideZFar = overrideZFar;

    const float ZFARCONST = 1000000;
    float znear, zfar;
    if (changed) {
        myPhysics.Update();
        changed = GFXFALSE;
    }
    GFXLoadIdentity(PROJECTION);
    //FIXMEGFXLoadIdentity(VIEW);
    switch (projectionType) {
        case Camera::PERSPECTIVE:
            znear = (overrideZFrustum ? overrideZNear : g_game.znear);
            zfar = (overrideZFrustum ? overrideZFar : g_game.zfar * (clip ? 1 : ZFARCONST));

            GFXPerspective(zoom * fov,
                    g_game.aspect,
                    znear,
                    zfar,
                    cockpit_offset);             //set perspective to 78 degree FOV
            break;
        case Camera::PARALLEL:

            znear = (overrideZFrustum ? overrideZNear : -g_game.zfar * (clip ? 1 : ZFARCONST));
            zfar = (overrideZFrustum ? overrideZFar : g_game.zfar * (clip ? 1 : ZFARCONST));

            //GFXParallel(xmin,xmax,ymin,ymax,-znear,zfar);
            GFXParallel(g_game.aspect * -zoom, g_game.aspect * zoom, -zoom, zoom, znear, zfar);
            break;
    }
    GFXLookAt(-R, centerCamera ? QVector(0, 0, 0) : Coord, Q);
    if (updateFrustum) {
        GFXCalculateFrustum();
    }
#ifdef PERFRAMESOUND
    Vector lastpos( view[12], view[13], view[14] );
    AUDListener( Coord, (Coord-lastpos)/GetElapsedTime() );           //this pos-last pos / elapsed time
#endif
    //GFXGetMatrix(VIEW,view);
    GFXSubwindow(x, y, xsize, ysize);
#ifdef PERFRAMESOUND
    AUDListenerOrientation( P, Q, R );
#endif

    if (centerCamera) {
        GFXSetLightOffset(-Coord.Cast());
    } else {
        GFXSetLightOffset(Vector(0, 0, 0));
    }

}

void Camera::UpdateCameraSounds() {
#ifndef PERFRAMESOUND
    AUDListener(Coord, GetVelocity());
    AUDListenerOrientation(P, Q, R);
#endif
}

void Camera::GetView(Matrix &vw) {
    GFXGetMatrixView(vw);
}

void Camera::SetNebula(Nebula *neb) {
    nebula.SetUnit((Unit *) neb);
}

Nebula *Camera::GetNebula() //this function can't be const, as it must return a non-const ptr
{
    return reinterpret_cast<Nebula *>( nebula.GetUnit()); //changed by chuck from (Nebula*) cast
}

void Camera::UpdatePlanetGFX() {
    Identity(planetview);
}

void Camera::RestoreViewPort(float xoffset, float yoffset) {
    GFXSubwindow(x + xoffset, y + yoffset, xsize, ysize);
}

void Camera::UpdateGLCenter() {
#define ITISDEPRECATED 0
    assert(ITISDEPRECATED);
#undef ITISDEPRECATED
//static float rotfactor = 0;
    //glMatrixMode(GL_PROJECTION);
    if (changed) {
        GFXLoadIdentity(PROJECTION);
        GFXLoadIdentity(VIEW);
        //updating the center should always use a perspective
        switch (Camera::PERSPECTIVE) {
            case Camera::PERSPECTIVE:
                GFXPerspective(zoom * fov,
                        g_game.aspect,
                        g_game.znear,
                        g_game.zfar,
                        cockpit_offset);             //set perspective to 78 degree FOV
                break;
            case Camera::PARALLEL:

                //GFXParallel(xmin,xmax,ymin,ymax,-znear,zfar);
                GFXParallel(g_game.aspect * -zoom, g_game.aspect * zoom, -zoom, zoom, -g_game.znear, g_game.zfar);
                break;
        }
        RestoreViewPort(0, 0);

        GFXLookAt(-R, QVector(0, 0, 0), Q);
        changed = GFXFALSE;
    }
    //glMultMatrixf(view);
}

void Camera::SetPosition(const QVector &origin, const Vector &vel, const Vector &angvel, const Vector &acceleration) {
    if (FINITE(origin.i) && FINITE(origin.j) && FINITE(origin.k)) {
        velocity = vel;
        angular_velocity = angvel;
        accel = acceleration;
        Coord = origin;
        changed = GFXTRUE;
    } else {
        // What value should this return? -- stephengtuggy 2021-09-06
        VS_LOG_FLUSH_EXIT(fatal, "fatal error in camera", -3);
    }
}

/** GetView (Matrix x)
 *  returns the view matrix (inverse matrix based on camera pqr)
 */
Matrix *Camera::GetPlanetGFX() {
    return &planetview;
    //CopyMatrix (x,view);
}

void Camera::LookDirection(const Vector &myR, const Vector &up) {
    P = (myR);
    P.Normalize();
    Q = up;
    Q.Normalize();
    CrossProduct(P, Q, R);
    changed = GFXTRUE;
}

void Camera::SetOrientation(const Vector &p, const Vector &q, const Vector &r) {
    P = p;
    Q = q;
    R = r;
    myPhysics.SetAngularVelocity(Vector(0, 0, 0));
    changed = GFXTRUE;
}

void Camera::SetSubwindow(float x, float y, float xsize, float ysize) {
    this->x = x;
    this->y = y;
    this->xsize = xsize;
    this->ysize = ysize;
    changed = GFXTRUE;
}

void Camera::SetProjectionType(ProjectionType t) {
    projectionType = t;
}

void Camera::SetZoom(float z) {
    zoom = z;
}

float Camera::GetZoom() const {
    return zoom;
}

void Camera::SetFov(float f) {
    fov = f;
}

float Camera::GetFov() const {
    return fov;
}

void Camera::Yaw(float rad) {
    ::Yaw(rad, P, Q, R);
    changed = GFXTRUE;
}

void Camera::Pitch(float rad) {
    ::Pitch(rad, P, Q, R);
    changed = GFXTRUE;
}

void Camera::Roll(float rad) {
    ::Roll(rad, P, Q, R);
    changed = GFXTRUE;
}

void Camera::XSlide(float factor) {
    Coord += (P * factor).Cast();
    changed = GFXTRUE;
}

void Camera::YSlide(float factor) {
    Coord += (Q * factor).Cast();
    changed = GFXTRUE;
}

void Camera::ZSlide(float factor) {
    Coord += (R * factor).Cast();
    changed = GFXTRUE;
}

