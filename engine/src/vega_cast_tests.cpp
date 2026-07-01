/*
 * vega_cast_tests.cpp
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
#include "gfx_generic/tvector.h"

constexpr int kIterations = 1000000;

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
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished QVector Cast() performance test. Took %1% second(s) for %2% iterations") % duration % kIterations));
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
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished Vector Cast() performance test. Took %1% second(s) for %2% iterations") % duration % kIterations));
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
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished vega_dynamic_cast_ptr performance test. Took %1% second(s) for %2% iterations") % duration % kIterations));
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
    VS_LOG_AND_FLUSH(important_info, (boost::format("Finished vega_dynamic_const_cast_ptr performance test. Took %1% second(s) for %2% iterations") % duration % kIterations));
}
