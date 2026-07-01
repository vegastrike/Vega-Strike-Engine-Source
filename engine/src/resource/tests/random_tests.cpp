/*
 * random_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include "resource/random_utils.h"
#include "common/vega_random.h"

constexpr int kNumRepetitions = 100;

TEST(random_utils, randomInt) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        int random_int = randomInt(10);
        EXPECT_GE(random_int, 0);
        EXPECT_LE(random_int, 10);
    }
}

TEST(random_utils, randomDouble) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        int random_double = randomDouble();
        EXPECT_GE(random_double, 0.0);
        EXPECT_LE(random_double, 1.0);
    }
}

TEST(VegaRandom, GenRandUInt32) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        uint_fast32_t random_uint32 = VegaRandom::Instance().GenRandUInt32();
        EXPECT_GT(random_uint32, 0);
        EXPECT_LT(random_uint32, kVegaUIntLeast32tMax);
    }
}

TEST(VegaRandom, RandomDouble) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().RandomDouble();
        EXPECT_GE(random_double, 0.0);
        EXPECT_LE(random_double, 1.0);
    }
}

TEST(VegaRandom, RandomDoubleUpTo) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().RandomDoubleUpTo(10.0);
        EXPECT_GE(random_double, 0.0);
        EXPECT_LE(random_double, 10.0);
    }
}

TEST(VegaRandom, RandomDoubleInRange) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().RandomDoubleInRange(-10.0, 10.0);
        EXPECT_GE(random_double, -10.0);
        EXPECT_LE(random_double, 10.0);
    }
}

TEST(VegaRandom, RandomRealInRange) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().RandomRealInRange(-10.0, 10.0);
        EXPECT_GE(random_double, -10.0);
        EXPECT_LE(random_double, 10.0);
    }
}

TEST(VegaRandom, RandomFloatInRange) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        float random_float = VegaRandom::Instance().RandomFloatInRange(-10.0F, 10.0F);
        EXPECT_GE(random_float, -10.0F);
        EXPECT_LE(random_float, 10.0F);
    }
}

TEST(VegaRandom, RandomFloatUpTo) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        float random_float = VegaRandom::Instance().RandomFloatUpTo(10.0F);
        EXPECT_GE(random_float, 0.0F);
        EXPECT_LE(random_float, 10.0F);
    }
}

TEST(VegaRandom, RandomFloat) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        float random_float = VegaRandom::Instance().RandomFloat();
        EXPECT_GE(random_float, 0.0F);
        EXPECT_LE(random_float, 1.0F);
    }
}

TEST(VegaRandom, RandomInt32InRange) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        int random_int32 = VegaRandom::Instance().RandomInt32InRange(-1000, 1000);
        EXPECT_GE(random_int32, -1000);
        EXPECT_LE(random_int32, 1000);
    }
}

TEST(VegaRandom, RandomInt32UpTo) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        int random_int32 = VegaRandom::Instance().RandomInt32UpTo(1000);
        EXPECT_GE(random_int32, 0);
        EXPECT_LE(random_int32, 1000);
    }
}

TEST(VegaRandom, RandomUInt32InRange) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        uint_fast32_t random_uint32 = VegaRandom::Instance().RandomUInt32InRange( 1000, 2000);
        EXPECT_GE(random_uint32, 1000);
        EXPECT_LE(random_uint32, 2000);
    }
}

TEST(VegaRandom, RandomUInt32UpTo) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        uint_fast32_t random_uint32 = VegaRandom::Instance().RandomUInt32UpTo(1000);
        EXPECT_LE(random_uint32, 1000);
    }
}

TEST(VegaRandom, GenRandRes53) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().GenRandRes53();
        EXPECT_GT(random_double, 0.0);
        EXPECT_LE(random_double, 1.0);
    }
}

TEST(VegaRandom, GenRandReal1) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().GenRandReal1();
        EXPECT_GT(random_double, 0.0);
        EXPECT_LT(random_double, 1.0);
    }
}

TEST(VegaRandom, GenRandReal2) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().GenRandReal2();
        EXPECT_GT(random_double, 0.0);
        EXPECT_LE(random_double, 1.0);
    }
}

TEST(VegaRandom, GenRandReal3) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().GenRandReal3();
        EXPECT_GE(random_double, 0.0);
        EXPECT_LE(random_double, 1.0);
    }
}

TEST(VegaRandom, UniformInclusive) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().UniformInclusive(-10.0, 10.0);
        EXPECT_GE(random_double, -10.0);
        EXPECT_LE(random_double, 10.0);
    }
}

TEST(VegaRandom, UniformExclusive) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().UniformExclusive(-10.0, 10.0);
        EXPECT_GT(random_double, -10.0);
        EXPECT_LT(random_double, 10.0);
    }
}

TEST(VegaRandom, GenRandInt31) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        int_fast32_t random_int32 = VegaRandom::Instance().GenRandInt31();
        EXPECT_GT(random_int32, 0);
        EXPECT_LT(random_int32, kVegaIntLeast32tMax);
    }
}

TEST(VegaRandom, RandomUCharInRange) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        unsigned char random_uchar = VegaRandom::Instance().RandomUCharInRange(10, 20);
        EXPECT_GE(random_uchar, 10);
        EXPECT_LE(random_uchar, 20);
    }
}

TEST(VegaRandom, RandomUCharUpTo) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        unsigned char random_uchar = VegaRandom::Instance().RandomUCharUpTo(30);
        EXPECT_GE(random_uchar, 0);
        EXPECT_LE(random_uchar, 30);
    }
}

TEST(VegaRandom, RandomUChar) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        unsigned char random_uchar = VegaRandom::Instance().RandomUChar();
        EXPECT_GE(random_uchar, 0);
        EXPECT_LE(random_uchar, 255);
    }
}

TEST(VegaRandom, RandomSizeTInRange) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        size_t random_size_t = VegaRandom::Instance().RandomSizeTInRange(50, 100);
        EXPECT_GE(random_size_t, 50);
        EXPECT_LE(random_size_t, 100);
    }
}

TEST(VegaRandom, RandomSizeTUpTo) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        size_t random_size_t = VegaRandom::Instance().RandomSizeTUpTo(150);
        EXPECT_GE(random_size_t, 0);
        EXPECT_LE(random_size_t, 150);
    }
}

TEST(VegaRandom, RandomSizeT) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        size_t random_size_t = VegaRandom::Instance().RandomSizeT();
        EXPECT_GE(random_size_t, 0);
        EXPECT_LE(random_size_t, std::numeric_limits<size_t>::max());
    }
}

TEST(VegaRandom, RandomSizeTLessThan) {
    // Initialize our test collection
    std::vector<int> test_vector{};
    const size_t test_vector_size = VegaRandom::Instance().RandomSizeTInRange(1, 200);
    for (size_t i = 0; i < test_vector_size; ++i) {
        test_vector.emplace_back(VegaRandom::Instance().GenRandInt31());
    }
    ASSERT_EQ(test_vector.size(), test_vector_size);

    // Repeatedly get random indexes into our test collection, and make sure that none of them are out of range
    for (int i = 0; i < kNumRepetitions; ++i) {
        size_t random_index = VegaRandom::Instance().RandomSizeTLessThan(test_vector_size);
        EXPECT_GE(random_index, 0);
        EXPECT_LT(random_index, test_vector_size);
        EXPECT_LT(random_index, test_vector.size());
        int _result{};
        ASSERT_NO_THROW(_result = test_vector.at(random_index));
        EXPECT_EQ(_result, test_vector.at(random_index));
        EXPECT_GE(_result, 0);
        EXPECT_LE(_result, std::numeric_limits<int_least32_t>::max());
    }
}

TEST(VegaRandom, NormalDistribution_Double) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().NormalDistribution(5.0, 2.0, 0.0, 10.0);
        EXPECT_GE(random_double, 0.0);
        EXPECT_LE(random_double, 10.0);
    }
}

TEST(VegaRandom, NormalDistribution_Float) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        float random_float = VegaRandom::Instance().NormalDistribution(5.0F, 2.0F, 0.0F, 10.0F);
        EXPECT_GE(random_float, 0.0F);
        EXPECT_LE(random_float, 10.0F);
    }
}

TEST(VegaRandom, NormalDistribution_DoubleWithinVeryCloseTolerances) {
    double random_double = VegaRandom::Instance().NormalDistribution(1.0, 2.0, 1.0, 1.0);
    ASSERT_DOUBLE_EQ(random_double, 1.0);

    random_double = VegaRandom::Instance().NormalDistribution(1.0, 0.0, 0.0, 10.0);
    ASSERT_DOUBLE_EQ(random_double, 1.0);

    random_double = VegaRandom::Instance().NormalDistribution(0.3, 2.0, 0.3, 0.1 + 0.2);
    ASSERT_DOUBLE_EQ(random_double, 0.3);
}

TEST(VegaRandom, NormalDistribution_FloatWithinVeryCloseTolerances) {
    float random_float = VegaRandom::Instance().NormalDistribution(1.0F, 2.0F, 1.0F, 1.0F);
    ASSERT_FLOAT_EQ(random_float, 1.0F);

    random_float = VegaRandom::Instance().NormalDistribution(1.0F, 0.0F, 0.0F, 10.0F);
    ASSERT_FLOAT_EQ(random_float, 1.0F);

    random_float = VegaRandom::Instance().NormalDistribution(0.3F, 2.0F, 0.3F, 0.1F + 0.2F);
    ASSERT_FLOAT_EQ(random_float, 0.3F);
}

TEST(VegaRandom, NormalDistribution_DoubleInvalidArguments) {
    ASSERT_THROW({ VegaRandom::Instance().NormalDistribution(1.0, -1.0, 0.0, 10.0); }, std::domain_error);
    ASSERT_THROW({ VegaRandom::Instance().NormalDistribution(1.0, 1.0, 2.0, 0.0); }, std::domain_error);
    ASSERT_THROW({ VegaRandom::Instance().NormalDistribution(-1.0, 1.0, 0.0, 10.0); }, std::domain_error);
}

TEST(VegaRandom, NormalDistribution_FloatInvalidArguments) {
    ASSERT_THROW({ VegaRandom::Instance().NormalDistribution(1.0F, -1.0F, 0.0F, 10.0F); }, std::domain_error);
    ASSERT_THROW({ VegaRandom::Instance().NormalDistribution(1.0F, 1.0F, 2.0F, 0.0F); }, std::domain_error);
    ASSERT_THROW({ VegaRandom::Instance().NormalDistribution(-1.0F, 1.0F, 0.0F, 10.0F); }, std::domain_error);
}

TEST(VegaRandom, NormalDistribution_DoubleMaxRetries) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        double random_double = VegaRandom::Instance().NormalDistribution(5.0, 200.0, 4.999999, 5.000001);
        EXPECT_GE(random_double, 4.999999);
        EXPECT_LE(random_double, 5.000001);
    }
}

TEST(VegaRandom, NormalDistribution_FloatMaxRetries) {
    for (int i = 0; i < kNumRepetitions; ++i) {
        float random_float = VegaRandom::Instance().NormalDistribution(5.0F, 200.0F, 4.999999F, 5.000001F);
        EXPECT_GE(random_float, 4.999999F);
        EXPECT_LE(random_float, 5.000001F);
    }
}
