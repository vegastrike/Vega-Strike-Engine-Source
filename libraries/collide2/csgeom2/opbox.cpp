/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
    Copyright (C) 2022 Stephen G. Tuggy
    Copyright (C) 2025 Benjamen R. Meyer
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))
#include <cmath>
#include "opbox.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

// We have a coordinate system around our box which is
// divided into 27 regions. The center region at coordinate (1,1,1)
// is the node itself. Every one of the 26 remaining regions
// defines an number of vertices which are the convex outline
// as seen from a camera view point in that region.
// The numbers inside the outlines table are indices from 0 to
// 7 which describe the 8 vertices outlining the node:
//	0: left/down/front vertex
//	1: left/down/back
//	2: left/up/front
//	3: left/up/back
//	4: right/down/front
//	5: right/down/back
//	6: right/up/front
//	7: right/up/back
// This table also contains an array of sides visible from that region.
struct Outline {
    int num;
    int vertices[7];
    int num_sides;
    int sides[3];
};
/// Outline lookup table.
static Outline outlines[27] =
        {
                {7, {3, 2, 6, 4, 5, 1, 0}, 3, {BOX_SIDE_x, BOX_SIDE_y, BOX_SIDE_z}},    // 0,0,0
                {6, {3, 2, 0, 4, 5, 1, -1}, 2, {BOX_SIDE_x, BOX_SIDE_y, -1}},        // 0,0,1
                {7, {7, 3, 2, 0, 4, 5, 1}, 3, {BOX_SIDE_x, BOX_SIDE_y, BOX_SIDE_Z}},    // 0,0,2
                {6, {3, 2, 6, 4, 0, 1, -1}, 2, {BOX_SIDE_x, BOX_SIDE_z, -1}},        // 0,1,0
                {4, {3, 2, 0, 1, -1, -1, -1}, 1, {BOX_SIDE_x, -1, -1}},            // 0,1,1
                {6, {7, 3, 2, 0, 1, 5, -1}, 2, {BOX_SIDE_x, BOX_SIDE_Z, -1}},        // 0,1,2
                {7, {3, 7, 6, 4, 0, 1, 2}, 3, {BOX_SIDE_x, BOX_SIDE_Y, BOX_SIDE_z}},    // 0,2,0
                {6, {3, 7, 6, 2, 0, 1, -1}, 2, {BOX_SIDE_x, BOX_SIDE_Y, -1}},        // 0,2,1
                {7, {7, 6, 2, 0, 1, 5, 3}, 3, {BOX_SIDE_x, BOX_SIDE_Y, BOX_SIDE_Z}},    // 0,2,2
                {6, {2, 6, 4, 5, 1, 0, -1}, 2, {BOX_SIDE_y, BOX_SIDE_z, -1}},        // 1,0,0
                {4, {0, 4, 5, 1, -1, -1, -1}, 1, {BOX_SIDE_y, -1, -1}},            // 1,0,1
                {6, {3, 1, 0, 4, 5, 7, -1}, 2, {BOX_SIDE_y, BOX_SIDE_Z, -1}},        // 1,0,2
                {4, {2, 6, 4, 0, -1, -1, -1}, 1, {BOX_SIDE_z, -1, -1}},            // 1,1,0
                {0, {-1, -1, -1, -1, -1, -1, -1}, 0, {-1, -1, -1}},                // 1,1,1
                {4, {7, 3, 1, 5, -1, -1, -1}, 1, {BOX_SIDE_Z, -1, -1}},            // 1,1,2
                {6, {3, 7, 6, 4, 0, 2, -1}, 2, {BOX_SIDE_Y, BOX_SIDE_z, -1}},        // 1,2,0
                {4, {3, 7, 6, 2, -1, -1, -1}, 1, {BOX_SIDE_Y, -1, -1}},            // 1,2,1
                {6, {2, 3, 1, 5, 7, 6, -1}, 2, {BOX_SIDE_Y, BOX_SIDE_Z, -1}},        // 1,2,2
                {7, {2, 6, 7, 5, 1, 0, 4}, 3, {BOX_SIDE_X, BOX_SIDE_y, BOX_SIDE_z}},    // 2,0,0
                {6, {6, 7, 5, 1, 0, 4, -1}, 2, {BOX_SIDE_X, BOX_SIDE_y, -1}},        // 2,0,1
                {7, {6, 7, 3, 1, 0, 4, 5}, 3, {BOX_SIDE_X, BOX_SIDE_y, BOX_SIDE_Z}},    // 2,0,2
                {6, {2, 6, 7, 5, 4, 0, -1}, 2, {BOX_SIDE_X, BOX_SIDE_z, -1}},        // 2,1,0
                {4, {6, 7, 5, 4, -1, -1, -1}, 1, {BOX_SIDE_X, -1, -1}},            // 2,1,1
                {6, {6, 7, 3, 1, 5, 4, -1}, 2, {BOX_SIDE_X, BOX_SIDE_Z, -1}},        // 2,1,2
                {7, {2, 3, 7, 5, 4, 0, 6}, 3, {BOX_SIDE_X, BOX_SIDE_Y, BOX_SIDE_z}},    // 2,2,0
                {6, {2, 3, 7, 5, 4, 6, -1}, 2, {BOX_SIDE_X, BOX_SIDE_Y, -1}},        // 2,2,1
                {7, {6, 2, 3, 1, 5, 4, 7}, 3, {BOX_SIDE_X, BOX_SIDE_Y, BOX_SIDE_Z}}    // 2,2,2
        };

csBox3::bEdge csBox3::edges[24] =
        {
                {BOX_CORNER_Xyz, BOX_CORNER_xyz, BOX_SIDE_y, BOX_SIDE_z},
                {BOX_CORNER_xyz, BOX_CORNER_Xyz, BOX_SIDE_z, BOX_SIDE_y},
                {BOX_CORNER_xyz, BOX_CORNER_xYz, BOX_SIDE_x, BOX_SIDE_z},
                {BOX_CORNER_xYz, BOX_CORNER_xyz, BOX_SIDE_z, BOX_SIDE_x},
                {BOX_CORNER_xYz, BOX_CORNER_XYz, BOX_SIDE_Y, BOX_SIDE_z},
                {BOX_CORNER_XYz, BOX_CORNER_xYz, BOX_SIDE_z, BOX_SIDE_Y},
                {BOX_CORNER_XYz, BOX_CORNER_Xyz, BOX_SIDE_X, BOX_SIDE_z},
                {BOX_CORNER_Xyz, BOX_CORNER_XYz, BOX_SIDE_z, BOX_SIDE_X},

                {BOX_CORNER_Xyz, BOX_CORNER_XyZ, BOX_SIDE_X, BOX_SIDE_y},
                {BOX_CORNER_XyZ, BOX_CORNER_Xyz, BOX_SIDE_y, BOX_SIDE_X},
                {BOX_CORNER_XyZ, BOX_CORNER_XYZ, BOX_SIDE_X, BOX_SIDE_Z},
                {BOX_CORNER_XYZ, BOX_CORNER_XyZ, BOX_SIDE_Z, BOX_SIDE_X},
                {BOX_CORNER_XYZ, BOX_CORNER_XYz, BOX_SIDE_X, BOX_SIDE_Y},
                {BOX_CORNER_XYz, BOX_CORNER_XYZ, BOX_SIDE_Y, BOX_SIDE_X},
                {BOX_CORNER_XYZ, BOX_CORNER_xYZ, BOX_SIDE_Y, BOX_SIDE_Z},
                {BOX_CORNER_xYZ, BOX_CORNER_XYZ, BOX_SIDE_Z, BOX_SIDE_Y},

                {BOX_CORNER_xYZ, BOX_CORNER_xYz, BOX_SIDE_Y, BOX_SIDE_x},
                {BOX_CORNER_xYz, BOX_CORNER_xYZ, BOX_SIDE_x, BOX_SIDE_Y},
                {BOX_CORNER_xYZ, BOX_CORNER_xyZ, BOX_SIDE_x, BOX_SIDE_Z},
                {BOX_CORNER_xyZ, BOX_CORNER_xYZ, BOX_SIDE_Z, BOX_SIDE_x},
                {BOX_CORNER_xyZ, BOX_CORNER_xyz, BOX_SIDE_x, BOX_SIDE_y},
                {BOX_CORNER_xyz, BOX_CORNER_xyZ, BOX_SIDE_y, BOX_SIDE_x},
                {BOX_CORNER_xyZ, BOX_CORNER_XyZ, BOX_SIDE_y, BOX_SIDE_Z},
                {BOX_CORNER_XyZ, BOX_CORNER_xyZ, BOX_SIDE_Z, BOX_SIDE_y}
        };
// Index by BOX_SIDE_? number.
csBox3::bFace csBox3::faces[6] =
        {
                {BOX_EDGE_xyz_xyZ, BOX_EDGE_xyZ_xYZ, BOX_EDGE_xYZ_xYz, BOX_EDGE_xYz_xyz},
                {BOX_EDGE_XYz_XYZ, BOX_EDGE_XYZ_XyZ, BOX_EDGE_XyZ_Xyz, BOX_EDGE_Xyz_XYz},
                {BOX_EDGE_xyz_Xyz, BOX_EDGE_Xyz_XyZ, BOX_EDGE_XyZ_xyZ, BOX_EDGE_xyZ_xyz},
                {BOX_EDGE_xYZ_XYZ, BOX_EDGE_XYZ_XYz, BOX_EDGE_XYz_xYz, BOX_EDGE_xYz_xYZ},
                {BOX_EDGE_xYz_XYz, BOX_EDGE_XYz_Xyz, BOX_EDGE_Xyz_xyz, BOX_EDGE_xyz_xYz},
                {BOX_EDGE_XYZ_xYZ, BOX_EDGE_xYZ_xyZ, BOX_EDGE_xyZ_XyZ, BOX_EDGE_XyZ_XYZ}
        };

csVector3 csBox3::GetCorner(int corner) const {
    switch (corner) {
        case BOX_CORNER_xyz:
            return Min();
        case BOX_CORNER_xyZ:
            return csVector3(MinX(), MinY(), MaxZ());
        case BOX_CORNER_xYz:
            return csVector3(MinX(), MaxY(), MinZ());
        case BOX_CORNER_xYZ:
            return csVector3(MinX(), MaxY(), MaxZ());
        case BOX_CORNER_Xyz:
            return csVector3(MaxX(), MinY(), MinZ());
        case BOX_CORNER_XyZ:
            return csVector3(MaxX(), MinY(), MaxZ());
        case BOX_CORNER_XYz:
            return csVector3(MaxX(), MaxY(), MinZ());
        case BOX_CORNER_XYZ:
            return Max();
    }
    return csVector3(0, 0, 0);
}

void csBox3::SetCenter(const csVector3 &c) {
    csVector3 move = c - GetCenter();
    minbox += move;
    maxbox += move;
}

void csBox3::SetSize(const csVector3 &s) {
    csVector3 center = GetCenter();
    minbox = center - s * (float) .5;
    maxbox = center + s * (float) .5;
}

bool csBox3::AdjacentX(const csBox3 &other) const {
    if (ABS (other.MinX() - MaxX()) < SMALL_EPSILON ||
            ABS (other.MaxX() - MinX()) < SMALL_EPSILON) {
        if (MaxY() < other.MinY() || MinY() > other.MaxY()) {
            return false;
        }
        if (MaxZ() < other.MinZ() || MinZ() > other.MaxZ()) {
            return false;
        }
        return true;
    }
    return false;
}

bool csBox3::AdjacentY(const csBox3 &other) const {
    if (ABS (other.MinY() - MaxY()) < SMALL_EPSILON ||
            ABS (other.MaxY() - MinY()) < SMALL_EPSILON) {
        if (MaxX() < other.MinX() || MinX() > other.MaxX()) {
            return false;
        }
        if (MaxZ() < other.MinZ() || MinZ() > other.MaxZ()) {
            return false;
        }
        return true;
    }
    return false;
}

bool csBox3::AdjacentZ(const csBox3 &other) const {
    if (ABS (other.MinZ() - MaxZ()) < SMALL_EPSILON ||
            ABS (other.MaxZ() - MinZ()) < SMALL_EPSILON) {
        if (MaxX() < other.MinX() || MinX() > other.MaxX()) {
            return false;
        }
        if (MaxY() < other.MinY() || MinY() > other.MaxY()) {
            return false;
        }
        return true;
    }
    return false;
}

int csBox3::Adjacent(const csBox3 &other) const {
    if (AdjacentX(other)) {
        if (other.MaxX() > MaxX()) {
            return BOX_SIDE_X;
        } else {
            return BOX_SIDE_x;
        }
    }
    if (AdjacentY(other)) {
        if (other.MaxY() > MaxY()) {
            return BOX_SIDE_Y;
        } else {
            return BOX_SIDE_y;
        }
    }
    if (AdjacentZ(other)) {
        if (other.MaxZ() > MaxZ()) {
            return BOX_SIDE_Z;
        } else {
            return BOX_SIDE_z;
        }
    }
    return -1;
}

int csBox3::GetVisibleSides(const csVector3 &pos, int *visible_sides) const {
    const csVector3 &bmin = Min();
    const csVector3 &bmax = Max();
    int idx;
    // First select x part of coordinate.
    if (pos.x < bmin.x) {
        idx = 0 * 9;
    } else if (pos.x > bmax.x) {
        idx = 2 * 9;
    } else {
        idx = 1 * 9;
    }
    // Then y part.
    if (pos.y < bmin.y) {
        idx += 0 * 3;
    } else if (pos.y > bmax.y) {
        idx += 2 * 3;
    } else {
        idx += 1 * 3;
    }
    // Then z part.
    if (pos.z < bmin.z) {
        idx += 0;
    } else if (pos.z > bmax.z) {
        idx += 2;
    } else {
        idx += 1;
    }
    const Outline &ol = outlines[idx];
    int num_array = ol.num_sides;
    int i;
    for (i = 0; i < num_array; i++) {
        visible_sides[i] = ol.sides[i];
    }
    return num_array;
}

void csBox3::GetConvexOutline(const csVector3 &pos,
        csVector3 *ar, int &num_array, bool bVisible) const {
    const csVector3 &bmin = Min();
    const csVector3 &bmax = Max();
    int idx;
    // First select x part of coordinate.
    if (pos.x < bmin.x) {
        idx = 0 * 9;
    } else if (pos.x > bmax.x) {
        idx = 2 * 9;
    } else {
        idx = 1 * 9;
    }
    // Then y part.
    if (pos.y < bmin.y) {
        idx += 0 * 3;
    } else if (pos.y > bmax.y) {
        idx += 2 * 3;
    } else {
        idx += 1 * 3;
    }
    // Then z part.
    if (pos.z < bmin.z) {
        idx += 0;
    } else if (pos.z > bmax.z) {
        idx += 2;
    } else {
        idx += 1;
    }

    const Outline &ol = outlines[idx];
    num_array = (bVisible ? ol.num : MIN (ol.num, 6));
    int i;
    for (i = 0; i < num_array; i++) {
        switch (ol.vertices[i]) {
            case 0:
                ar[i].x = bmin.x;
                ar[i].y = bmin.y;
                ar[i].z = bmin.z;
                break;
            case 1:
                ar[i].x = bmin.x;
                ar[i].y = bmin.y;
                ar[i].z = bmax.z;
                break;
            case 2:
                ar[i].x = bmin.x;
                ar[i].y = bmax.y;
                ar[i].z = bmin.z;
                break;
            case 3:
                ar[i].x = bmin.x;
                ar[i].y = bmax.y;
                ar[i].z = bmax.z;
                break;
            case 4:
                ar[i].x = bmax.x;
                ar[i].y = bmin.y;
                ar[i].z = bmin.z;
                break;
            case 5:
                ar[i].x = bmax.x;
                ar[i].y = bmin.y;
                ar[i].z = bmax.z;
                break;
            case 6:
                ar[i].x = bmax.x;
                ar[i].y = bmax.y;
                ar[i].z = bmin.z;
                break;
            case 7:
                ar[i].x = bmax.x;
                ar[i].y = bmax.y;
                ar[i].z = bmax.z;
                break;
        }
    }
}

bool csBox3::Between(const csBox3 &box1, const csBox3 &box2) const {
    // First the trival test to see if the coordinates are
    // at least within the right intervals.
    if (((maxbox.x >= box1.minbox.x && minbox.x <= box2.maxbox.x) ||
            (maxbox.x >= box2.minbox.x && minbox.x <= box1.maxbox.x)) &&
            ((maxbox.y >= box1.minbox.y && minbox.y <= box2.maxbox.y) ||
                    (maxbox.y >= box2.minbox.y && minbox.y <= box1.maxbox.y)) &&
            ((maxbox.z >= box1.minbox.z && minbox.z <= box2.maxbox.z) ||
                    (maxbox.z >= box2.minbox.z && minbox.z <= box1.maxbox.z))) {
        // @@@ Ok, let's just return true here. Maybe this test is already
        // enough? We could have used the planes as well.
        return true;
    }
    return false;
}

void csBox3::ManhattanDistance(const csBox3 &other, csVector3 &dist) const {
    if (other.MinX() >= MaxX()) {
        dist.x = other.MinX() - MaxX();
    } else if (MinX() >= other.MaxX()) {
        dist.x = MinX() - other.MaxX();
    } else {
        dist.x = 0;
    }
    if (other.MinY() >= MaxY()) {
        dist.y = other.MinY() - MaxY();
    } else if (MinY() >= other.MaxY()) {
        dist.y = MinY() - other.MaxY();
    } else {
        dist.y = 0;
    }
    if (other.MinZ() >= MaxZ()) {
        dist.z = other.MinZ() - MaxZ();
    } else if (MinZ() >= other.MaxZ()) {
        dist.z = MinZ() - other.MaxZ();
    } else {
        dist.z = 0;
    }
}

float csBox3::SquaredOriginDist() const {
    // Thanks to Ivan Avramovic for the original.
    // Adapted by Norman Kramer, Jorrit Tyberghein and Wouter Wijngaards.
    float res = 0;
    if (minbox.x > 0) {
        res = minbox.x * minbox.x;
    } else if (maxbox.x < 0) {
        res = maxbox.x * maxbox.x;
    }
    if (minbox.y > 0) {
        res += minbox.y * minbox.y;
    } else if (maxbox.y < 0) {
        res += maxbox.y * maxbox.y;
    }
    if (minbox.z > 0) {
        res += minbox.z * minbox.z;
    } else if (maxbox.z < 0) {
        res += maxbox.z * maxbox.z;
    };
    return res;
}

float csBox3::SquaredOriginMaxDist() const {
    // Thanks to Ivan Avramovic for the original.
    // Adapted by Norman Kramer, Jorrit Tyberghein and Wouter Wijngaards.
    float res;
    if (minbox.x > 0) {
        res = maxbox.x * maxbox.x;
    } else if (maxbox.x < 0) {
        res = minbox.x * minbox.x;
    } else {
        res = MAX (maxbox.x * maxbox.x, minbox.x * minbox.x);
    }
    if (minbox.y > 0) {
        res += maxbox.y * maxbox.y;
    } else if (maxbox.y < 0) {
        res += minbox.y * minbox.y;
    } else {
        res += MAX (maxbox.y * maxbox.y, minbox.y * minbox.y);
    }
    if (minbox.z > 0) {
        res += maxbox.z * maxbox.z;
    } else if (maxbox.z < 0) {
        res += minbox.z * minbox.z;
    } else {
        res += MAX (maxbox.z * maxbox.z, minbox.z * minbox.z);
    }
    return res;
}

csBox3 &csBox3::operator+=(const csBox3 &box) {
    if (box.minbox.x < minbox.x) {
        minbox.x = box.minbox.x;
    }
    if (box.minbox.y < minbox.y) {
        minbox.y = box.minbox.y;
    }
    if (box.minbox.z < minbox.z) {
        minbox.z = box.minbox.z;
    }
    if (box.maxbox.x > maxbox.x) {
        maxbox.x = box.maxbox.x;
    }
    if (box.maxbox.y > maxbox.y) {
        maxbox.y = box.maxbox.y;
    }
    if (box.maxbox.z > maxbox.z) {
        maxbox.z = box.maxbox.z;
    }
    return *this;
}

csBox3 &csBox3::operator+=(const csVector3 &point) {
    if (point.x < minbox.x) {
        minbox.x = point.x;
    }
    if (point.x > maxbox.x) {
        maxbox.x = point.x;
    }
    if (point.y < minbox.y) {
        minbox.y = point.y;
    }
    if (point.y > maxbox.y) {
        maxbox.y = point.y;
    }
    if (point.z < minbox.z) {
        minbox.z = point.z;
    }
    if (point.z > maxbox.z) {
        maxbox.z = point.z;
    }
    return *this;
}

csBox3 &csBox3::operator*=(const csBox3 &box) {
    if (box.minbox.x > minbox.x) {
        minbox.x = box.minbox.x;
    }
    if (box.minbox.y > minbox.y) {
        minbox.y = box.minbox.y;
    }
    if (box.minbox.z > minbox.z) {
        minbox.z = box.minbox.z;
    }
    if (box.maxbox.x < maxbox.x) {
        maxbox.x = box.maxbox.x;
    }
    if (box.maxbox.y < maxbox.y) {
        maxbox.y = box.maxbox.y;
    }
    if (box.maxbox.z < maxbox.z) {
        maxbox.z = box.maxbox.z;
    }
    return *this;
}

csBox3 operator+(const csBox3 &box1, const csBox3 &box2) {
    return csBox3(
            MIN(box1.minbox.x, box2.minbox.x),
            MIN(box1.minbox.y, box2.minbox.y),
            MIN(box1.minbox.z, box2.minbox.z),
            MAX(box1.maxbox.x, box2.maxbox.x),
            MAX(box1.maxbox.y, box2.maxbox.y),
            MAX(box1.maxbox.z, box2.maxbox.z));
}

csBox3 operator+(const csBox3 &box, const csVector3 &point) {
    return csBox3(
            MIN(box.minbox.x, point.x),
            MIN(box.minbox.y, point.y),
            MIN(box.minbox.z, point.z),
            MAX(box.maxbox.x, point.x),
            MAX(box.maxbox.y, point.y),
            MAX(box.maxbox.z, point.z));
}

csBox3 operator*(const csBox3 &box1, const csBox3 &box2) {
    return csBox3(
            MAX(box1.minbox.x, box2.minbox.x),
            MAX(box1.minbox.y, box2.minbox.y),
            MAX(box1.minbox.z, box2.minbox.z),
            MIN(box1.maxbox.x, box2.maxbox.x),
            MIN(box1.maxbox.y, box2.maxbox.y),
            MIN(box1.maxbox.z, box2.maxbox.z));
}

bool operator==(const csBox3 &box1, const csBox3 &box2) {
    return ((box1.minbox.x == box2.minbox.x)
            && (box1.minbox.y == box2.minbox.y)
            && (box1.minbox.z == box2.minbox.z)
            && (box1.maxbox.x == box2.maxbox.x)
            && (box1.maxbox.y == box2.maxbox.y)
            && (box1.maxbox.z == box2.maxbox.z));
}

bool operator!=(const csBox3 &box1, const csBox3 &box2) {
    return ((box1.minbox.x != box2.minbox.x)
            || (box1.minbox.y != box2.minbox.y)
            || (box1.minbox.z != box2.minbox.z)
            || (box1.maxbox.x != box2.maxbox.x)
            || (box1.maxbox.y != box2.maxbox.y)
            || (box1.maxbox.z != box2.maxbox.z));
}

bool operator<(const csBox3 &box1, const csBox3 &box2) {
    return ((box1.minbox.x >= box2.minbox.x)
            && (box1.minbox.y >= box2.minbox.y)
            && (box1.minbox.z >= box2.minbox.z)
            && (box1.maxbox.x <= box2.maxbox.x)
            && (box1.maxbox.y <= box2.maxbox.y)
            && (box1.maxbox.z <= box2.maxbox.z));
}

bool operator>(const csBox3 &box1, const csBox3 &box2) {
    return ((box2.minbox.x >= box1.minbox.x)
            && (box2.minbox.y >= box1.minbox.y)
            && (box2.minbox.z >= box1.minbox.z)
            && (box2.maxbox.x <= box1.maxbox.x)
            && (box2.maxbox.y <= box1.maxbox.y)
            && (box2.maxbox.z <= box1.maxbox.z));
}

bool operator<(const csVector3 &point, const csBox3 &box) {
    return ((point.x >= box.minbox.x)
            && (point.x <= box.maxbox.x)
            && (point.y >= box.minbox.y)
            && (point.y <= box.maxbox.y)
            && (point.z >= box.minbox.z)
            && (point.z <= box.maxbox.z));
}

//---------------------------------------------------------------------------
