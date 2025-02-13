/*
 * al.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_AL_H
#define VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_AL_H

#if defined(__APPLE__) && defined(__MACH__)
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

typedef ALuint ALSourceHandle;
typedef ALuint ALBufferHandle;

#define AL_NULL_BUFFER (ALBufferHandle(0))

#ifndef AL_SEC_OFFSET
/* Supported on Windows, but the headers might be out of date. */
#define AL_SEC_OFFSET 0x1024
#endif

// Windows AL calls it "ILLEGAL" not "INVALID".....?
#if (!defined(ALC_INVALID_ENUM) && defined(ALC_ILLEGAL_ENUM))
#define ALC_INVALID_ENUM ALC_ILLEGAL_ENUM
#endif

// Both are major=0x1000 and minor=0x1001, but are completely different!?!?!?
#ifdef ALC_VERSION_0_1
#define ALCstring const ALCchar*
#else
#define ALCstring ALCubyte*
#endif

#endif //VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_AL_H
