/*
 * configuration_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, David Wales
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


#include <gtest/gtest.h>
#include "configuration/game_config.h"
#include "src/vs_logging.h"

#include <string>
#include <chrono>

#include "gfxlib_struct.h"
#include "vega_cast_utils.h"
#include "configuration/configuration.h"
#include "gfx/quadsquare.h"
#include "resource/random_utils.h"
#include "root_generic/lin_time.h"
// #include "src/resizable.h"
#include "gfx_generic/tvector.h"

constexpr int kIterations = 100000;

TEST(QVector, Cast_Performance) {
    VS_LOG_AND_FLUSH(important_info, "Starting QVector Cast() performance test");
    const double start_time = realTime();
    for (int i = 0; i < kIterations; ++i) {
        const QVector q_vector(randomDouble(), randomDouble(), randomDouble());
        // ReSharper disable once CppDFAUnreadVariable
        // ReSharper disable once CppDeclaratorNeverUsed
        // ReSharper disable once CppDFAUnusedValue
        const Vector vector = q_vector.Cast();
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished QVector Cast() performance test. Took %1% for %2% iterations") % duration % kIterations));
}

TEST(Vector, Cast_Performance) {
    VS_LOG_AND_FLUSH(important_info, "Starting Vector Cast() performance test");
    const double start_time = realTime();
    for (int i = 0; i < kIterations; ++i) {
        const Vector vector(randomDouble(), randomDouble(), randomDouble());
        // ReSharper disable once CppDeclaratorNeverUsed
        // ReSharper disable once CppDFAUnreadVariable
        // ReSharper disable once CppDFAUnusedValue
        const QVector q_vector = vector.Cast();
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished Vector Cast() performance test. Took %1% for %2% iterations") % duration % kIterations));
}

class B {
public:
    B() = default;
    virtual ~B() = default;
};

class C : public B {
public:
    C() = default;
    ~C() override = default;
};

TEST(vega_dynamic_cast_ptr, Performance) {
    VS_LOG_AND_FLUSH(important_info, "Starting vega_dynamic_cast_ptr performance test");
    const double start_time = realTime();
    for (int i = 0; i < kIterations; ++i) {
        C child_class_instance;
        // ReSharper disable once CppDFAUnusedValue
        // ReSharper disable once CppDeclaratorNeverUsed
        // ReSharper disable once CppDFAUnreadVariable
        const B * child_class_instance_as_parent_class = vega_dynamic_cast_ptr<B>(&child_class_instance);
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished vega_dynamic_cast_ptr performance test. Took %1% for %2% iterations") % duration % kIterations));
}

TEST(vega_dynamic_const_cast_ptr, Performance) {
    VS_LOG_AND_FLUSH(important_info, "Starting vega_dynamic_const_cast_ptr performance test");
    const double start_time = realTime();
    for (int i = 0; i < kIterations; ++i) {
        const C child_class_instance;
        // ReSharper disable once CppDFAUnusedValue
        // ReSharper disable once CppDeclaratorNeverUsed
        // ReSharper disable once CppDFAUnreadVariable
        const B * child_class_instance_as_parent_class = vega_dynamic_const_cast_ptr<const B>(&child_class_instance);
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished vega_dynamic_const_cast_ptr performance test. Took %1% for %2% iterations") % duration % kIterations));
}

TEST(configuration, const_Performance) {
    VS_LOG_AND_FLUSH(important_info, "Starting configuration()...._dbl test");
    const double start_time = realTime();
    for (int i = 0; i < kIterations; ++i) {
        // ReSharper disable once CppDFAUnreadVariable
        // ReSharper disable once CppDeclaratorNeverUsed
        // ReSharper disable once CppDFAUnusedValue
        const double asteroid_difficulty = configuration().physics.asteroid_difficulty_dbl;
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished configuration()...._dbl test. Took %1% for %2% iterations") % duration % kIterations));
}

TEST(configuration, static_optional_Performance) {
    VS_LOG_AND_FLUSH(important_info, "Starting test of static optional setting variable");
    const double start_time = realTime();
    static boost::optional<double> setting{};
    for (int i = 0; i < kIterations; ++i) {
        if (setting == boost::none) {
            setting = configuration().physics.asteroid_difficulty_dbl;
        }
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished test of static optional setting variable. Took %1% for %2% iterations") % duration % kIterations));
}

TEST(configuration, Performance_of_static_optional_with_get) {
    VS_LOG_AND_FLUSH(important_info, "Starting test of static optional setting variable with .get()");
    const double start_time = realTime();
    static boost::optional<double> setting{};
    for (int i = 0; i < kIterations; ++i) {
        if (setting == boost::none) {
            setting = configuration().physics.asteroid_difficulty_dbl;
        }
        setting.get();
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished test of static optional setting variable with .get(). Took %1% for %2% iterations") % duration % kIterations));
}

TEST(LoadConfig, Sanity) {
    // Test without configuration
    const bool default_bool = vega_config::GetGameConfig().GetBool("test.boolean_variable", false);
    const int32_t default_int_32_t = vega_config::GetGameConfig().GetInt32("test.int_variable", 1);
    const float default_float = vega_config::GetGameConfig().GetFloat("test.float_variable", 7.8F);
    const std::string default_string = vega_config::GetGameConfig().GetString("test.string_variable", "World");
    const std::string default_escaped_string = vega_config::GetGameConfig().GetEscapedString("test.default_escaped_string", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_FALSE(default_bool);
    EXPECT_EQ(default_int_32_t, 1);
    EXPECT_FLOAT_EQ(default_float, 7.8F);
    EXPECT_EQ(default_string, "World");
    EXPECT_EQ(default_escaped_string, "#00FF00Hi\r\nthere!#000000");

    // Read the configuration from the config file
    const std::string filename = "../test_assets/vegastrike.config";
    vega_config::GetGameConfig().LoadGameConfig(filename);

    // Test again
    const bool test_bool = vega_config::GetGameConfig().GetBool("test.boolean_variable", false);
    const int test_int32_t = vega_config::GetGameConfig().GetInt32("test.int_variable", 1);
    const float test_float = vega_config::GetGameConfig().GetFloat("test.float_variable", 7.8F);
    const std::string test_string = vega_config::GetGameConfig().GetString("test.string_variable", "World");
    const std::string escaped_string = vega_config::GetGameConfig().GetEscapedString("test.escaped_string_variable", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_TRUE(test_bool);
    EXPECT_EQ(test_int32_t, 15) << "Expected 15 but got " << test_int32_t;
    EXPECT_FLOAT_EQ(test_float, 4.2F) << "Expected 4.2 but got " << test_float;
    EXPECT_EQ(test_string, "hello");
    EXPECT_EQ(escaped_string, "#FF0000Hello\r\nthere!#000000");

    // Test subsection functionality
    const bool subsection_bool = vega_config::GetGameConfig().GetBool("test.subsection.subsection_boolean_variable", false);
    const int32_t subsection_int_32_t =
            vega_config::GetGameConfig().GetInt32("test.subsection.subsection_int_variable", 2);
    const float subsection_float = vega_config::GetGameConfig().GetFloat("test.subsection.subsection_float_variable", 8.9F);
    const std::string subsection_string = vega_config::GetGameConfig().GetString("test.subsection.subsection_string_variable", "World");
    const std::string subsection_escaped_string = vega_config::GetGameConfig().GetEscapedString("test.subsection.subsection_escaped_string_variable", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_TRUE(subsection_bool);
    EXPECT_EQ(subsection_int_32_t, 15) << "Expected 15 but got " << subsection_int_32_t;
    EXPECT_FLOAT_EQ(subsection_float, 4.2F) << "Expected 4.2 but got " << subsection_float;
    EXPECT_EQ(subsection_string, "hello");
    EXPECT_EQ(subsection_escaped_string, "#FF0000Hello\r\nthere!#000000");

    // Another main section
    const bool test_bool2 = vega_config::GetGameConfig().GetBool("test2.boolean_variable2", true);
    const int32_t test_int32_t_2 = vega_config::GetGameConfig().GetInt32("test2.int_variable2", 3);
    const float test_float2 = vega_config::GetGameConfig().GetFloat("test2.float_variable2", 10.0F);
    const std::string test_string2 = vega_config::GetGameConfig().GetString("test2.string_variable2", "World");
    const std::string escaped_string2 = vega_config::GetGameConfig().GetEscapedString("test2.escaped_string_variable2", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_TRUE(test_bool2);
    EXPECT_EQ(test_int32_t_2, 15) << "Expected 15 but got " << test_int32_t_2;
    EXPECT_FLOAT_EQ(test_float2, 4.2F) << "Expected 4.2 but got " << test_float2;
    EXPECT_EQ(test_string2, "hello");
    EXPECT_EQ(escaped_string2, "#FF0000Hello\r\nthere!#000000");
}

TEST(GameConfig, GetFloat_Performance) {
    VS_LOG_AND_FLUSH(important_info, "Starting GetFloat performance test");
    const double start_time = realTime();
    for (int i = 0; i < kIterations; ++i) {
        vega_config::GetGameConfig().GetFloat("test.subsection.subsection_float_variable", 11.1F);
    }
    const double end_time = realTime();
    const double duration = end_time - start_time;
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished GetFloat performance test. Took %1% for %2% iterations") % duration % kIterations));
}

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
