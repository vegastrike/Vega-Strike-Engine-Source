/*
 * atmosphere.h
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
// NO HEADER GUARD

#include "gfx/vec.h"
#include "gfxlib_struct.h"
#include "gfx/matrix.h"
#include "gfx/mesh.h"
#include <vector>

class Atmosphere {
public:
    struct Parameters {
        /* High level parameters */
        /* coefficients for capturing scattering effects of the
         *  atmospheric composition/density */
        float radius;
        /* low angle */
        GFXColor low_color[2];
        /* low angle */
        GFXColor low_ambient_color[2];
        /* high angle */
        GFXColor high_color[2];
        /* high angle */
        GFXColor high_ambient_color[2];
        float scattering;
        /* "scattering" coefficient, basically
         *                                        how much the low_color spreads around */
    };

private:
    Parameters user_params;

    class SunBox {
    public:
        SunBox(Mesh *m) : sunbox(m) {
        }

        ~SunBox();
        Mesh *sunbox;
    };
    void setArray(float *, const GFXColor &);
    void setArray1(float *, const GFXColor &);
/* Internal state information */
    Mesh *dome;                                        /* entire sky; for texture mapping */
/* 3 rendering passes, to preserve framerate */
    Mesh *cap;                                         /* mid to zenith */
    Mesh *stack;                               /* horizon to zenith */
/* box around the sun (single star for now) */
    std::vector<SunBox *> sunboxes;
    Matrix tmatrix;
    QVector position;
/* lighting contexts for the above 3 */
    int l0, l1, l2;
    int divisions;

public:
    Atmosphere(const Parameters &params);
    ~Atmosphere();
    const Parameters &parameters();
    void SetParameters(const Parameters &params);

/* Update the precomputed mesh to the new parameters. This is
 *  expensive, so let the upper layers decide when to do this
 */

/* ugh, make update just change the lighting state */
    void Update(const QVector &position, const Matrix &tmatrix);
    void Draw();
    static void ProcessDrawQueue();
    void SetMatricesAndDraw(const QVector &position, const Matrix tmatrix);
/* inherit the orientation of
 *                                                                                the transformation matrix  */
    static void DrawAtmospheres();
};

