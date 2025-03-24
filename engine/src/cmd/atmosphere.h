/**
 * atmosphere.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
// NO HEADER GUARD

#include "gfx_generic/vec.h"
#include "src/gfxlib_struct.h"
#include "gfx_generic/matrix.h"
#include "gfx_generic/mesh.h"
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

