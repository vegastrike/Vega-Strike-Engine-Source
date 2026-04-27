///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for random generators.
 *  \file		IceRandom.cpp
 *  \author		Pierre Terdiman
 *  \date		August, 9, 2001
 *
 *  Copyright (C) 1998-2026 Pierre Terdiman, Stephen G. Tuggy, Benjamen R. Meyer
 *  Public Domain
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "collide2/Opcode.h"
#include "root_generic/vega_random.h"

using namespace Opcode;

void SRand(const uint32_t seed) {
    VegaRandom::Instance().InitGenRand(seed);
}

uint32_t Rand() {
    return VegaRandom::Instance().GenRandUInt32();
}

uint32_t GetRandomIndex(const uint32_t max_index) {
    return VegaRandom::Instance().RandomUInt32UpTo(max_index - 1);
}
