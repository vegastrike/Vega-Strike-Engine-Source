/*
 * gfx_quad_list_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, David Wales
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


#include <gtest/gtest.h>

#include <string>

#include "resource/random_utils.h"
#include "root_generic/lin_time.h"
#include "src/vega_cast_utils.h"
#include "gfxlib_struct.h"

constexpr int kIterations = 1000000;

TEST(GFXQuadList, GFXVertex) {
    constexpr int kNumberOfVertices = 4;
    GFXVertex vertices_original[kNumberOfVertices]{};
    GFXVertex vertices_memcpy[kNumberOfVertices]{};
    GFXVertex vertices_no_memcpy[kNumberOfVertices]{};
    GFXVertex vertices_copy_constructor[kNumberOfVertices]{};

    for (auto & vertex : vertices_original) {
        vertex.s = randomDouble();
        vertex.t = randomDouble();
        vertex.i = randomDouble();
        vertex.j = randomDouble();
        vertex.k = randomDouble();
        vertex.x = randomDouble();
        vertex.y = randomDouble();
        vertex.z = randomDouble();
        vertex.tx = randomDouble();
        vertex.ty = randomDouble();
        vertex.tz = randomDouble();
        vertex.tw = randomDouble();
    }

    for (int i = 0; i < kNumberOfVertices; ++i) {
        vertices_no_memcpy[i].s = vertices_original[i].s;
        vertices_no_memcpy[i].t = vertices_original[i].t;
        vertices_no_memcpy[i].i = vertices_original[i].i;
        vertices_no_memcpy[i].j = vertices_original[i].j;
        vertices_no_memcpy[i].k = vertices_original[i].k;
        vertices_no_memcpy[i].x = vertices_original[i].x;
        vertices_no_memcpy[i].y = vertices_original[i].y;
        vertices_no_memcpy[i].z = vertices_original[i].z;
        vertices_no_memcpy[i].tx = vertices_original[i].tx;
        vertices_no_memcpy[i].ty = vertices_original[i].ty;
        vertices_no_memcpy[i].tz = vertices_original[i].tz;
        vertices_no_memcpy[i].tw = vertices_original[i].tw;

        vertices_copy_constructor[i] = vertices_original[i];
    }

    memcpy(vertices_memcpy, vertices_original, kNumberOfVertices * sizeof(GFXVertex));

    for (int i = 0; i < kNumberOfVertices; ++i) {
        EXPECT_EQ(vertices_no_memcpy[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_no_memcpy[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_no_memcpy[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_no_memcpy[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_no_memcpy[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_no_memcpy[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_no_memcpy[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_no_memcpy[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_no_memcpy[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_no_memcpy[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_no_memcpy[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_no_memcpy[i].tw, vertices_original[i].tw);

        EXPECT_EQ(vertices_memcpy[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_memcpy[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_memcpy[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_memcpy[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_memcpy[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_memcpy[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_memcpy[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_memcpy[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_memcpy[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_memcpy[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_memcpy[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_memcpy[i].tw, vertices_original[i].tw);

        EXPECT_EQ(vertices_copy_constructor[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_copy_constructor[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_copy_constructor[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_copy_constructor[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_copy_constructor[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_copy_constructor[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_copy_constructor[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_copy_constructor[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_copy_constructor[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_copy_constructor[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_copy_constructor[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_copy_constructor[i].tw, vertices_original[i].tw);
    }
}

TEST(GFXQuadList, GFXColorVertex) {
    constexpr int kNumberOfVertices = 4;
    GFXColorVertex vertices_original[kNumberOfVertices]{};
    GFXColorVertex vertices_memcpy[kNumberOfVertices]{};
    GFXColorVertex vertices_no_memcpy[kNumberOfVertices]{};
    GFXColorVertex vertices_copy_constructor[kNumberOfVertices]{};

    for (auto & vertex : vertices_original) {
        vertex.s = randomDouble();
        vertex.t = randomDouble();
        vertex.r = randomDouble();
        vertex.g = randomDouble();
        vertex.b = randomDouble();
        vertex.a = randomDouble();
        vertex.i = randomDouble();
        vertex.j = randomDouble();
        vertex.k = randomDouble();
        vertex.x = randomDouble();
        vertex.y = randomDouble();
        vertex.z = randomDouble();
        vertex.tx = randomDouble();
        vertex.ty = randomDouble();
        vertex.tz = randomDouble();
        vertex.tw = randomDouble();
    }

    for (int i = 0; i < kNumberOfVertices; ++i) {
        vertices_no_memcpy[i].s = vertices_original[i].s;
        vertices_no_memcpy[i].t = vertices_original[i].t;
        vertices_no_memcpy[i].r = vertices_original[i].r;
        vertices_no_memcpy[i].g = vertices_original[i].g;
        vertices_no_memcpy[i].b = vertices_original[i].b;
        vertices_no_memcpy[i].a = vertices_original[i].a;
        vertices_no_memcpy[i].i = vertices_original[i].i;
        vertices_no_memcpy[i].j = vertices_original[i].j;
        vertices_no_memcpy[i].k = vertices_original[i].k;
        vertices_no_memcpy[i].x = vertices_original[i].x;
        vertices_no_memcpy[i].y = vertices_original[i].y;
        vertices_no_memcpy[i].z = vertices_original[i].z;
        vertices_no_memcpy[i].tx = vertices_original[i].tx;
        vertices_no_memcpy[i].ty = vertices_original[i].ty;
        vertices_no_memcpy[i].tz = vertices_original[i].tz;
        vertices_no_memcpy[i].tw = vertices_original[i].tw;

        vertices_copy_constructor[i] = vertices_original[i];
    }

    memcpy(vertices_memcpy, vertices_original, kNumberOfVertices * sizeof(GFXColorVertex));


    for (int i = 0; i < kNumberOfVertices; ++i) {
        EXPECT_EQ(vertices_no_memcpy[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_no_memcpy[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_no_memcpy[i].r, vertices_original[i].r);
        EXPECT_EQ(vertices_no_memcpy[i].g, vertices_original[i].g);
        EXPECT_EQ(vertices_no_memcpy[i].b, vertices_original[i].b);
        EXPECT_EQ(vertices_no_memcpy[i].a, vertices_original[i].a);
        EXPECT_EQ(vertices_no_memcpy[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_no_memcpy[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_no_memcpy[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_no_memcpy[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_no_memcpy[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_no_memcpy[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_no_memcpy[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_no_memcpy[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_no_memcpy[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_no_memcpy[i].tw, vertices_original[i].tw);

        EXPECT_EQ(vertices_memcpy[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_memcpy[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_memcpy[i].r, vertices_original[i].r);
        EXPECT_EQ(vertices_memcpy[i].g, vertices_original[i].g);
        EXPECT_EQ(vertices_memcpy[i].b, vertices_original[i].b);
        EXPECT_EQ(vertices_memcpy[i].a, vertices_original[i].a);
        EXPECT_EQ(vertices_memcpy[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_memcpy[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_memcpy[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_memcpy[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_memcpy[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_memcpy[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_memcpy[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_memcpy[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_memcpy[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_memcpy[i].tw, vertices_original[i].tw);

        EXPECT_EQ(vertices_copy_constructor[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_copy_constructor[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_copy_constructor[i].r, vertices_original[i].r);
        EXPECT_EQ(vertices_copy_constructor[i].g, vertices_original[i].g);
        EXPECT_EQ(vertices_copy_constructor[i].b, vertices_original[i].b);
        EXPECT_EQ(vertices_copy_constructor[i].a, vertices_original[i].a);
        EXPECT_EQ(vertices_copy_constructor[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_copy_constructor[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_copy_constructor[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_copy_constructor[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_copy_constructor[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_copy_constructor[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_copy_constructor[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_copy_constructor[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_copy_constructor[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_copy_constructor[i].tw, vertices_original[i].tw);
    }
}

TEST(GFXColorVertex, SetVtx) {
    constexpr int kNumberOfVertices = 4;
    GFXVertex vertices_original[kNumberOfVertices]{};
    GFXColorVertex vertices_copied[kNumberOfVertices]{};

    for (auto & vertex : vertices_original) {
        vertex.s = randomDouble();
        vertex.t = randomDouble();
        vertex.i = randomDouble();
        vertex.j = randomDouble();
        vertex.k = randomDouble();
        vertex.x = randomDouble();
        vertex.y = randomDouble();
        vertex.z = randomDouble();
        vertex.tx = randomDouble();
        vertex.ty = randomDouble();
        vertex.tz = randomDouble();
        vertex.tw = randomDouble();
    }

    for (int i = 0; i < kNumberOfVertices; ++i) {
        vertices_copied[i].SetVtx(vertices_original[i]);

        vertices_copied[i].tx = vertices_original[i].tx;
        vertices_copied[i].ty = vertices_original[i].ty;
        vertices_copied[i].tz = vertices_original[i].tz;
        vertices_copied[i].tw = vertices_original[i].tw;
    }

    for (int i = 0; i < kNumberOfVertices; ++i) {
        EXPECT_EQ(vertices_copied[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_copied[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_copied[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_copied[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_copied[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_copied[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_copied[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_copied[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_copied[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_copied[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_copied[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_copied[i].tw, vertices_original[i].tw);

        EXPECT_EQ(vertices_copied[i].r, 0.0F);
        EXPECT_EQ(vertices_copied[i].g, 0.0F);
        EXPECT_EQ(vertices_copied[i].b, 0.0F);
        EXPECT_EQ(vertices_copied[i].a, 0.0F);
    }
}

TEST(GFXColorVertex, SetVtx2) {
    constexpr int kNumberOfVertices = 4;
    GFXVertex vertices_original[kNumberOfVertices]{};
    GFXColorVertex vertices_copied[kNumberOfVertices]{};

    for (auto & vertex : vertices_original) {
        vertex.s = randomDouble();
        vertex.t = randomDouble();
        vertex.i = randomDouble();
        vertex.j = randomDouble();
        vertex.k = randomDouble();
        vertex.x = randomDouble();
        vertex.y = randomDouble();
        vertex.z = randomDouble();
        vertex.tx = randomDouble();
        vertex.ty = randomDouble();
        vertex.tz = randomDouble();
        vertex.tw = randomDouble();
    }

    for (int i = 0; i < kNumberOfVertices; ++i) {
        vertices_copied[i].SetVtx2(vertices_original[i]);
    }

    for (int i = 0; i < kNumberOfVertices; ++i) {
        EXPECT_EQ(vertices_copied[i].s, vertices_original[i].s);
        EXPECT_EQ(vertices_copied[i].t, vertices_original[i].t);
        EXPECT_EQ(vertices_copied[i].i, vertices_original[i].i);
        EXPECT_EQ(vertices_copied[i].j, vertices_original[i].j);
        EXPECT_EQ(vertices_copied[i].k, vertices_original[i].k);
        EXPECT_EQ(vertices_copied[i].x, vertices_original[i].x);
        EXPECT_EQ(vertices_copied[i].y, vertices_original[i].y);
        EXPECT_EQ(vertices_copied[i].z, vertices_original[i].z);
        EXPECT_EQ(vertices_copied[i].tx, vertices_original[i].tx);
        EXPECT_EQ(vertices_copied[i].ty, vertices_original[i].ty);
        EXPECT_EQ(vertices_copied[i].tz, vertices_original[i].tz);
        EXPECT_EQ(vertices_copied[i].tw, vertices_original[i].tw);

        EXPECT_EQ(vertices_copied[i].r, 0.0F);
        EXPECT_EQ(vertices_copied[i].g, 0.0F);
        EXPECT_EQ(vertices_copied[i].b, 0.0F);
        EXPECT_EQ(vertices_copied[i].a, 0.0F);
    }
}

// TEST(Resizable, Sanity) {
//     Resizable<GFXColorVertex> resizable{};
//     EXPECT_EQ(resizable.size(), 0);
//     std::vector<GFXColorVertex *> original{};
//     for (size_t i = 0; i < 50; ++i) {
//         GFXColorVertex gfx_color_vertex{};
//         gfx_color_vertex.s = randomDouble();
//         gfx_color_vertex.t = randomDouble();
//         gfx_color_vertex.i = randomDouble();
//         gfx_color_vertex.j = randomDouble();
//         gfx_color_vertex.k = randomDouble();
//         gfx_color_vertex.x = randomDouble();
//         gfx_color_vertex.y = randomDouble();
//         gfx_color_vertex.z = randomDouble();
//         gfx_color_vertex.tx = randomDouble();
//         gfx_color_vertex.ty = randomDouble();
//         gfx_color_vertex.tz = randomDouble();
//         gfx_color_vertex.tw = randomDouble();
//         gfx_color_vertex.r = randomDouble();
//         gfx_color_vertex.g = randomDouble();
//         gfx_color_vertex.b = randomDouble();
//         gfx_color_vertex.a = randomDouble();
//         original.emplace_back(&gfx_color_vertex);
//         resizable.push_back(gfx_color_vertex);
//         EXPECT_EQ(resizable.size(), i);
//         for (unsigned int j = 0; j < i; ++j) {
//             EXPECT_EQ(resizable.at(j).s, original.at(j)->s);
//             EXPECT_EQ(resizable.at(j).t, original.at(j)->t);
//             EXPECT_EQ(resizable.at(j).i, original.at(j)->i);
//             EXPECT_EQ(resizable.at(j).j, original.at(j)->j);
//             EXPECT_EQ(resizable.at(j).k, original.at(j)->k);
//             EXPECT_EQ(resizable.at(j).x, original.at(j)->x);
//             EXPECT_EQ(resizable.at(j).y, original.at(j)->y);
//             EXPECT_EQ(resizable.at(j).z, original.at(j)->z);
//             EXPECT_EQ(resizable.at(j).tx, original.at(j)->tx);
//             EXPECT_EQ(resizable.at(j).ty, original.at(j)->ty);
//             EXPECT_EQ(resizable.at(j).tz, original.at(j)->tz);
//             EXPECT_EQ(resizable.at(j).tw, original.at(j)->tw);
//             EXPECT_EQ(resizable.at(j).r, original.at(j)->r);
//             EXPECT_EQ(resizable.at(j).g, original.at(j)->g);
//             EXPECT_EQ(resizable.at(j).b, original.at(j)->b);
//             EXPECT_EQ(resizable.at(j).a, original.at(j)->a);
//         }
//     }
//
//     resizable.clear();
//     EXPECT_EQ(resizable.size(), 0);
// }
