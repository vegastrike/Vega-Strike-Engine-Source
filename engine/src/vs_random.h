/*
 * vs_random.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
// NO HEADER GUARD

/*
 * #define N 624
 * #define M 397
 * #define MATRIX_A 0x9908b0dfUL   // constant vector a
 * #define UPPER_MASK 0x80000000UL // most significant w-r bits
 * #define LOWER_MASK 0x7fffffffUL // least significant r bits
 */

#define VS_RAND_MAX 0x7fffffffUL

class VSRandom {
#define NN_CONSTANT 624

    static const unsigned int N() {
        return NN_CONSTANT;
    }

    static const unsigned int M() {
        return 397;
    }

    static const unsigned int MATRIX_A() {
        return 0x9908b0dfUL;
    }

    static const unsigned int UPPER_MASK() {
        return 0x80000000UL;
    }

    static const unsigned int LOWER_MASK() {
        return 0x7fffffffUL;
    }

    unsigned int mt[NN_CONSTANT]; /* the array for the state vector  */
#undef NN_CONSTANT
    unsigned int mti; /* mti==N+1 means mt[N] is not initialized */
/* initializes mt[N] with a seed */
public:
    VSRandom(unsigned int s) : mti(N() + 1) {
        init_genrand(s);
    }

    void init_genrand(unsigned int s) {
        mt[0] = s & 0xffffffffUL;
        for (mti = 1; mti < N(); mti++) {
            mt[mti] =
                    (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
            /*
             * See Knuth TAOCP Vol2. 3	rd Ed. P.106 for multiplier.
             * In the previous versions, MSB	s of the seed affect
             * only MSBs of the array mt[].
             * 2002/01/09 modified by Makoto Matsumoto
             */
            mt[mti] &= 0xffffffffUL;
            /* for >32 bit machines */
        }
    }

/*
 * initialize by an array with array-length
 * init_key is the array for initializing keys
 * key_length is its length
 */
    VSRandom(unsigned int init_key[], unsigned int key_length) : mti(N() + 1) {
        unsigned int i, j, k;
        init_genrand(19650218UL);
        i = 1;
        j = 0;
        k = (N() > key_length ? N() : key_length);
        for (; k; k--) {
            mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1664525UL))
                    + init_key[j] + j;       /* non linear */
            mt[i] &= 0xffffffffUL;     /* for WORDSIZE > 32 machines */
            i++;
            j++;
            if (i >= N()) {
                mt[0] = mt[N() - 1];
                i = 1;
            }
            if (j >= key_length) {
                j = 0;
            }
        }
        for (k = N() - 1; k; k--) {
            mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1566083941UL))
                    - i;     /* non linear */
            mt[i] &= 0xffffffffUL;     /* for WORDSIZE > 32 machines */
            i++;
            if (i >= N()) {
                mt[0] = mt[N() - 1];
                i = 1;
            }
        }
        mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
    }

/* generates a random number on [0,0xffffffff]-interval */
    unsigned int genrand_int32(void) {
        unsigned int y;
        static unsigned int mag01[2] = {0x0UL, MATRIX_A()};
        /* mag01[x] = x * MATRIX_A  for x=0,1 */
        if (mti >= N()) {
            /* generate N words at one time */
            unsigned int kk;
            if (mti == N() + 1) {      /* if init_genrand() has not been called, */
                init_genrand(5489UL);
            }          /* a default initial seed is used */
            for (kk = 0; kk < N() - M(); kk++) {
                y = (mt[kk] & UPPER_MASK()) | (mt[kk + 1] & LOWER_MASK());
                mt[kk] = mt[kk + M()] ^ (y >> 1) ^ mag01[y & 0x1UL];
            }
            for (; kk < N() - 1; kk++) {
                y = (mt[kk] & UPPER_MASK()) | (mt[kk + 1] & LOWER_MASK());
                mt[kk] = mt[kk + (M() - N())] ^ (y >> 1) ^ mag01[y & 0x1UL];
            }
            y = (mt[N() - 1] & UPPER_MASK()) | (mt[0] & LOWER_MASK());
            mt[N() - 1] = mt[M() - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];
            mti = 0;
        }
        y = mt[mti++];
        /* Tempering */
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680UL;
        y ^= (y << 15) & 0xefc60000UL;
        y ^= (y >> 18);
        return y;
    }

/* generates a random number on [0,0x7fffffff]-interval */
    int genrand_int31(void) {
        return (int) (genrand_int32() >> 1);
    }

    unsigned int rand() {
        return genrand_int31();
    }

/* generates a random number on [0,1]-real-interval */
    double genrand_real1(void) {
        return genrand_int32() * (1.0 / 4294967295.0);
        /* divided by 2^32-1 */
    }

/* generates a random number on [0,1)-real-interval */
    double genrand_real2(void) {
        return genrand_int32() * (1.0 / 4294967296.0);
        /* divided by 2^32 */
    }

    double uniformInc(double min, double max) {
        return genrand_real1() * (max - min) + min;
    }

    double uniformExc(double min, double max) {
        return genrand_real2() * (max - min) + min;
    }

/* generates a random number on (0,1)-real-interval */
    double genrand_real3(void) {
        return (((double) genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
        /* divided by 2^32 */
    }

/* generates a random number on [0,1) with 53-bit resolution*/
    double genrand_res53(void) {
        unsigned int a = genrand_int32() >> 5, b = genrand_int32() >> 6;
        return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
    }
/* These real versions are due to Isaku Wada, 2002/01/09 added */
};
extern VSRandom vsrandom;

