/*
    segmentcell.h

    Copyright (C) 2006 by Kapoulkine Arseny
    Copyright (C) 2022-2025 by Stephen G. Tuggy, Benjamen R. Meyer

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

// NO HEADER GUARD

class csTerrainSegmentCellCollider {
private:
    int sign(float val) {
        return val > 0 ? 1 : val < 0 ? -1 : 0;
    }

    iTerrainCell *cell;
    const csVector3 &start;
    const csVector3 &end;

    const csVector2 &pos;
    const csVector3 &size;

    float scale_u, scale_v;

    float u0, v0, h0;
    float u1, v1, h1;
    float du, dv, dh;
    float oneOverdu, oneOverdv;
    float eu, ev;
    float dp, oneOverdp, ep;

    float t, h, cell_height;

    unsigned int width, height;

    bool firsttime, vertical, verticalhit;

public:
    csTerrainSegmentCellCollider(iTerrainCell *cell, const csVector3 &
    start, const csVector3 &end)
            : start(start), end(end), pos(cell->GetPosition()), size(cell->GetSize()),
            vertical(false), verticalhit(false) {
        // Constants
        const float rootOf2 = 1.414213f;
        const float halfRoot2 = rootOf2 / 2;

        this->cell = cell;

        width = cell->GetGridWidth();
        height = cell->GetGridHeight();

        scale_u = size.x / (width - 1);
        scale_v = size.z / (height - 1);

        // Offset from grid 0,0
        const csVector2 gridOffsetStart = csVector2(start.x - pos.x,
                -(start.z - (pos.y + size.z)));
        const csVector2 gridOffsetEnd = csVector2(end.x - pos.x,
                -(end.z - (pos.y + size.z)));

        // U, V and height of segment start in cell space
        u0 = (gridOffsetStart.x) / scale_u;
        v0 = (gridOffsetStart.y) / scale_v;
        h0 = start.y;

        // U, V and height of segment end in cell space
        u1 = (gridOffsetEnd.x) / scale_u;
        v1 = (gridOffsetEnd.y) / scale_v;
        h1 = end.y;

        // Compute differences for ray (lengths along axes) and their inverse
        du = u1 - u0;
        dv = v1 - v0;
        dh = h1 - h0;

        // vertical case
        if (du == 0 && dv == 0) {
            vertical = true;

            float height = cell->GetHeight(gridOffsetStart);
            if (csMin(h0, h1) <= height &&
                    csMax(h0, h1) >= height) {
                verticalhit = true;
            }
        }

        if (fabs(du) < SMALL_EPSILON) {
            du = SMALL_EPSILON;
        }
        if (fabs(dv) < SMALL_EPSILON) {
            dv = SMALL_EPSILON;
        }

        oneOverdu = 1 / du;
        oneOverdv = 1 / dv;

        // Distance to intersection with u/v axes
        eu = u0 - floor(u0);
        ev = v0 - floor(v0);

        // Differences and distance to intersection with diagonal
        dp = (du + dv) / rootOf2;

        if (fabs(dp) < SMALL_EPSILON) {
            dp = SMALL_EPSILON;
        }

        oneOverdp = 1 / dp;
        ep = fabs(dp) * (1 - eu - ev) / (dv + du); // line-line intersection

        // Fixup for positive directions
        if (du > 0) {
            eu = 1 - eu;
        }
        if (dv > 0) {
            ev = 1 - ev;
        }
        if (ep < 0) {
            ep += halfRoot2;
        }

        // Stepping variables
        t = 0;
        h = h0;
        cell_height = cell->GetHeight(gridOffsetStart);

        firsttime = true;
    }

    // 0 - nothing happened
    // 1 - collision
    // -1 - loop stop
    int GetIntersection(csVector3 &result, csVector2 &cell_result) {
        // Constants
        const float rootOf2 = 1.414213f;
        const float halfRoot2 = rootOf2 / 2;

        if (t < 0 || t >= 1) {
            return -1;
        }
        if (!(t < 0) && !(t >= 0)) {
            return -1;
        }

        float r_h0 = h;
        float tstep = 0;

        if (vertical) {
            if (verticalhit && firsttime) {
                firsttime = false;

                cell_result.x = u0;
                cell_result.y = v0;

                result.x = pos.x + cell_result.x * scale_u;
                result.y = cell_height;
                result.z = pos.y + height - cell_result.y * scale_v;

                return 1;
            } else {
                return -1;
            }
        }

        if (!firsttime) {
            float tToU = eu * fabs(oneOverdu); // Time to reach U intersection
            float tToV = ev * fabs(oneOverdv); // Time to reach V intersection
            float tToP = ep * fabs(oneOverdp); // Time to reach P intersection

            if (tToU <= tToV && tToU <= tToP) {
                // U intersection first
                if (t + tToU > 1) {
                    tToU = 1 - t;
                }

                tstep = tToU;
                t += tToU;

                // Update distances
                eu = 0;
                ev -= fabs(dv) * tToU;
                ep -= fabs(dp) * tToU;

                // Update height
                h += dh * tToU;
            } else if (tToV <= tToU && tToV <= tToP) {
                // V intersection first
                if (t + tToV > 1) {
                    tToV = 1 - t;
                }

                tstep = tToV;
                t += tToV;

                // Update distances
                eu -= fabs(du) * tToV;
                ev = 0;
                ep -= fabs(dp) * tToV;

                // Update height
                h += dh * tToV;
            } else {
                // P intersection first
                if (t + tToP > 1) {
                    tToP = 1 - t;
                }

                tstep = tToP;
                t += tToP;

                // Update distances
                eu -= fabs(du) * tToP;
                ev -= fabs(dv) * tToP;
                ep = 0;

                // Update height
                h += dh * tToP;
            }

            // Wrap around
            if (eu <= 0) {
                eu = 1;
            }
            if (ev <= 0) {
                ev = 1;
            }
            if (ep <= 0) {
                ep = halfRoot2;
            }
        }

        firsttime = false;

        // Check for intersection
        float r_h1 = h;
        float h_h0 = cell_height;

        csVector2 uv = csVector2((u0 + du * t) * scale_u,
                (v0 + dv * t) * scale_v);
        float h_h1 = cell->GetHeight(uv);

        // Remember height value
        cell_height = h_h1;

        // Check intersection
        int cmp_h0 = sign(r_h0 - h_h0);
        int cmp_h1 = sign(r_h1 - h_h1);

        if (cmp_h0 * cmp_h1 == -1 || fabs(r_h1 - h_h1) < EPSILON) {
            float correct_t = t;

            if (cmp_h0 * cmp_h1 == -1) {
                //    A                 B                   C
                // (vertex) ------(intersection)------- (vertex)
                // AC = tstep (in terms of t), we have to subtract
                // BC from correct_t. AB / BC equals the height ratio
                float coeff = fabs(r_h0 - h_h0) / fabs(r_h1 - h_h1);

                // AB / BC = coeff
                // AB + BC = tstep
                // AB = coeff * BC
                // BC * (coeff + 1) = tstep

                correct_t -= tstep / (coeff + 1);
            }

            cell_result.x = u0 + du * correct_t;
            cell_result.y = v0 + dv * correct_t;

            result.x = pos.x + cell_result.x * scale_u;
            result.y = h0 + dh * correct_t;
            result.z = pos.y + height - cell_result.y * scale_v;

            return (cell_result.x >= 0 && cell_result.x <= width - 1 &&
                    cell_result.y >= 0 && cell_result.y <= height - 1);
        }

        return 0;
    }
};
