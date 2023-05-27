/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
// NO HEADER GUARD

enum POLYGON_RELATION {
    BACK = -1,
    UNKNOWN = 0,
    FRONT = 1
};

class Vector {
public:
    double x, y, z;

    Vector(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {
    }
};
class Index {
public:
    Vector V;
    int p, n, t, c;

    Index(Vector xyz, int p, int n, int t, int c) : V(xyz), p(p), n(n), t(t), c(c) {
    }

    void write(FILE *fp) const;
};

class Plane {
public:
    double a, b, c, d;

    Plane(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {
    }

    POLYGON_RELATION inFront(const Vector &) const;

    float frontBack(const Vector &v) const {
        return a * v.x + b * v.y + c * v.z + d;
    }
};
class Face {
    bool Cross(Plane &) const;
    POLYGON_RELATION inFront(const Plane &) const;
public:
    std::vector<Index> p;
    int id;
    Plane planeEqu() const;
    bool operator<(const class Face &) const;

    Face() {
        static int temp = 0;
        id = temp++;
    }
};
class Mesh {
    void processline(char *line);
    Index processfacevertex(char *chunk) const;
    Face processface(char *line) const;
    std::vector<Vector> p;
    std::vector<Vector> n;
    std::vector<Vector> t; //tex coords i=s j=t
    std::vector<Vector> c;
    std::vector<Face> f;
public:
    Mesh(const char *filename);
    void sort();
    void write(const char *filename) const;
    void writeIndex(FILE *fp, const Index &) const;
};

