/*
 * vega_random.h
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

#ifndef VEGA_STRIKE_VEGA_RANDOM_H
#define VEGA_STRIKE_VEGA_RANDOM_H

#include <random>
#include <cstdint>
#include <limits>

constexpr int_fast32_t kVegaIntFast32tMax = std::numeric_limits<int_fast32_t>::max();
constexpr int_least32_t kVegaIntLeast32tMax = std::numeric_limits<int_least32_t>::max();
constexpr unsigned long kVegaIntLeast32tMaxAsULong = std::numeric_limits<int_least32_t>::max();
constexpr uint_fast32_t kVegaUIntFast32tMax = std::numeric_limits<uint_fast32_t>::max();
constexpr uint_least32_t kVegaUIntLeast32tMax = std::numeric_limits<uint_least32_t>::max();
constexpr double kVegaUInt32tMaxAsDouble = std::numeric_limits<uint_least32_t>::max();
constexpr double kVegaUInt32tMaxAsDoublePlus1 = std::numeric_limits<uint_least32_t>::max() + 1.0;


class VegaRandom {
    std::mt19937 gen;

public:

    /// Initializes random number generator with system random_device, where available
    explicit VegaRandom() {
        std::random_device rd;
        gen.seed(rd());
    }

    /// Initializes random number generator with a specific seed
    explicit VegaRandom(const uint_fast32_t seed) {
        gen.seed(seed);
    }

    /**
     * initializes by an array with array-length
     * init_key is the array for initializing keys
     * key_length is its length
     */
    VegaRandom(uint_fast32_t init_key[], const size_t key_length) {
        std::seed_seq seq(init_key, init_key + key_length);
        gen.seed(seq);
    }

    ~VegaRandom() = default;

    /// Initializes random number generator with a specific seed
    void InitGenRand(const uint_fast32_t seed) {
        gen.seed(seed);
    }

    static VegaRandom& Instance() {
        static VegaRandom instance;
        return instance;
    }

    /** generates a random uint_fast32_t on [0,0xffffffff]-interval */
    uint_fast32_t GenRandUInt32() {
        return gen();
    }

    /** generates a random int_fast32_t on [0,0x7fffffff]-interval */
    int_fast32_t GenRandInt31() {
        return static_cast<int_fast32_t>(GenRandUInt32() >> 1);
    }

    /// generates a random uint_fast32_t on [0,0xffffffff]-interval
    uint_fast32_t rand() {
        return GenRandUInt32();
    }

    /** generates a random double on [0,1]-real-interval */
    double GenRandReal1() {
        return GenRandUInt32() * (1.0 / kVegaUInt32tMaxAsDouble);
        /* divided by 2^32-1 */
    }

    /** generates a random double on [0,1)-real-interval */
    double GenRandReal2() {
        return GenRandUInt32() * (1.0 / kVegaUInt32tMaxAsDoublePlus1);
        /* divided by 2^32 */
    }

    /// generates a random double between min and max inclusive
    double UniformInclusive(const double min, const double max) {
        std::uniform_real_distribution<double> real_dist(min, max);
        return real_dist(gen);
    }

    /// generates a random double between min and max exclusive
    double UniformExclusive(const double min, const double max) {
        std::uniform_real_distribution<double> real_dist(min, max - 1.0 / kVegaUInt32tMaxAsDoublePlus1);
        return real_dist(gen);
    }

    /** generates a random double on (0,1)-real-interval */
    double GenRandReal3() {
        return (static_cast<double>(GenRandUInt32()) + 0.5) * (1.0 / kVegaUInt32tMaxAsDoublePlus1);
        /* divided by 2^32 */
    }

    /** generates a random double on [0,1) with 53-bit resolution */
    double GenRandRes53() {
        const uint_fast32_t a = GenRandUInt32() >> 5;
        const uint_fast32_t b = GenRandUInt32() >> 6;
        return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
    }

    /// generates a random int_fast32_t between min and max, inclusive
    int_fast32_t RandomInt32InRange(const int_fast32_t min, const int_fast32_t max) {
        std::uniform_int_distribution<int_fast32_t> int_dist(min, max);
        return int_dist(gen);
    }

    /// generates a random int_fast32_t between 0 and max, inclusive
    int_fast32_t RandomInt32UpTo(const int_fast32_t max) {
        return RandomInt32InRange(0, max);
    }

    /// generates a random uint_fast32_t between min and max, inclusive
    uint_fast32_t RandomUInt32InRange(const uint_fast32_t min, const uint_fast32_t max) {
        std::uniform_int_distribution<uint_fast32_t> uint_dist(min, max);
        return uint_dist(gen);
    }

    /// generates a random uint_fast32_t between 0 and max, inclusive
    uint_fast32_t RandomUInt32UpTo(const uint_fast32_t max) {
        return RandomUInt32InRange(0, max);
    }

    /// generates a random unsigned char between min and max, inclusive
    unsigned char RandomUCharInRange(const unsigned char min, const unsigned char max) {
        std::uniform_int_distribution<unsigned char> dist(min, max);
        return dist(gen);
    }

    /// generates a random unsigned char between 0 and max, inclusive
    unsigned char RandomUCharUpTo(const unsigned char max) {
        return RandomUCharInRange(0, max);
    }

    /// generates a random unsigned char between 0 and 255, inclusive
    unsigned char RandomUChar() {
        return RandomUCharInRange(0, 255);
    }

    /// generates a random double between min and max, inclusive
    double RandomRealInRange(const double min, const double max) {
        return UniformInclusive(min, max);
    }

    /// generates a random double between min and max, inclusive
    double RandomDoubleInRange(const double min, const double max) {
        return UniformInclusive(min, max);
    }

    /// generates a random double between 0.0 and max, inclusive
    double RandomDoubleUpTo(const double max) {
        return RandomDoubleInRange(0.0, max);
    }

    /// generates a random double between 0.0 and 1.0, at 1/10,000th increments
    double RandomDouble() {
        constexpr double kPrecision = 10000.0;
        std::uniform_int_distribution<uint_fast32_t> int_dist(0,kPrecision);
        const uint_fast32_t random_int = int_dist(gen);
        return static_cast<double>(random_int) / kPrecision;
    }

    /// generates a random float between min and max, inclusive
    float RandomFloatInRange(const float min, const float max) {
        return UniformInclusive(min, max);
    }

    /// generates a random float between 0.0F and max, inclusive
    float RandomFloatUpTo(const float max) {
        return RandomFloatInRange(0.0F, max);
    }

    /// generates a random float on [0.0F, 1.0F)
    float RandomFloat() {
        std::uniform_real_distribution<float> real_dist(0.0F, 1.0F);
        return real_dist(gen);
    }
};

#endif //VEGA_STRIKE_VEGA_RANDOM_H
