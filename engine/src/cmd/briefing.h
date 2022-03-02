/**
 * briefing.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include "gfx/matrix.h"
#include "gfx/camera.h"
#include "gfx/hud.h"
#include <list>
#include <vector>
using std::list;
using std::vector;
class BriefingOrder {
public:
    Vector vec;
    float speed;

    BriefingOrder(const Vector &v, const float s) : vec(v) {
        speed = s;
    }
};

class Briefing {
public:
//Very reduced unit class
    class Ship {
        std::vector<class Mesh *> meshdata;
        Vector pos;
        std::list<BriefingOrder> orders;
    public:
        ~Ship();
        void Destroy();

        bool LoadFailed() {
            return meshdata.empty();
        }

        Vector Position() {
            return pos;
        }                           //return Vector (mat[12],mat[13],mat[14]);}
        void SetPosition(const Vector &pos) {
            this->pos = pos;
        }                                                 //mat[12]=pos.i;mat[13]=pos.j;mat[14]=pos.k;}
        float cloak; //btw 0 and 1
        Ship(const char *filename, int faction, const Vector &position);
        void Render(const Matrix &cam, double interpol);
        void Update();
        void OverrideOrder(const Vector &destination, float time);
        void EnqueueOrder(const Vector &destination, float time);
    };
    Camera cam;
    TextPlane tp;
    vector<Ship *> starships;
    void Render();
    void Update();
//-1 returns file not found
    int AddStarship(const char *filename, int faction, const Vector &);
    void RemoveStarship(int);
    void EnqueueOrder(int, const Vector &destination, float time);
    void OverrideOrder(int, const Vector &destination, float time);
    void SetPosition(int, const Vector &Position);
    Vector GetPosition(int);
    void SetCloak(int, float);
    Briefing();
    ~Briefing();
};

