/*
 * coord_select.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "vec.h"
#include "animation.h"
#include "in_mouse.h"
#include "preferred_types.h"

class CoordinateSelect {
protected:
    float CrosshairSize;
    vega_types::SharedPtr<Animation> LocSelAni;
    QVector LocalPosition;
    void UpdateMouse();
public:
    static void MouseMoveHandle(KBSTATE, int, int, int, int, int);
    explicit CoordinateSelect(QVector); //wish I could warp mouse pointer there
    ~CoordinateSelect() = default;

    inline QVector GetVector() {
        return LocalPosition;
    }

    void Draw();
};

