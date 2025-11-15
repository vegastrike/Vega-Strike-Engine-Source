/*
 * vegastrike.h
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
#ifndef VEGA_STRIKE_ENGINE_VEGASTRIKE_H
#define VEGA_STRIKE_ENGINE_VEGASTRIKE_H

#ifdef __cplusplus
#include <assert.h>
#endif //__cplusplus

extern float simulation_atom_var;
extern float audio_atom_var;
//#define SIMULATION_ATOM (simulation_atom_var)
//#define AUDIO_ATOM (audio_atom_var)

// Why do we need two variables to reflect the same thing ?
extern float SIMULATION_ATOM;
extern float AUDIO_ATOM;

#include "src/vs_math.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "src/debug_vs.h"

#if !defined(_WIN32)
#   define GL_GLEXT_PROTOTYPES 1
#endif

#if defined (WIN32) || defined (__CYGWIN__)
/* Note that this will define WIN32 for us, if it isn't defined already
 */
#ifndef NOMINMAX
    #define NOMINMAX
#endif //tells VCC not to generate min/max macros
    #include <windows.h>
 //#include <wingdi.h>
    #include <time.h>
#else //defined (WIN32) || defined (__CYGWIN__)
    #include <sys/time.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif //defined (WIN32) || defined (__CYGWIN__)

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#if !defined (NO_GFX)
#   if defined(__APPLE__) && defined(__MACH__)
#       define GL_GLEXT_FUNCTION_POINTERS 1
#       include <GL/gl.h>
#       include <GL/glu.h>
#       include <GL/glut.h>
#   else //defined(__APPLE__) && defined(__MACH__)
#       define __glext_h_
#       include <GL/gl.h>
#       include <GL/glu.h>
#       include <GL/glut.h>
#       undef __glext_h_
#   endif //defined(__APPLE__) && defined(__MACH__)
#endif //NO_GFX

#define PROG_NAME "vegastrike"

#define BUFF_LEN 512

//#define RANDOMIZE_SIM_ATOMS 1

extern std::locale our_numeric_locale;

inline double locale_aware_stod(const std::string& s, const std::locale& loc = our_numeric_locale, const double default_value = 0.0) {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        double tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stod encountered an error");
        return default_value;
    }
}

inline float locale_aware_stof(const std::string& s, const std::locale& loc = our_numeric_locale, const float default_value = 0.0F) {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        float tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stof encountered an error");
        return default_value;
    }
}

inline int locale_aware_stoi(const std::string& s, const std::locale& loc = our_numeric_locale, const int default_value = 0) {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        int tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stoi encountered an error");
        return default_value;
    }
}

inline long locale_aware_stol(const std::string& s, const std::locale& loc = our_numeric_locale, const long default_value = 0L) {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        long tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stol encountered an error");
        return default_value;
    }
}

#ifdef __cplusplus

#endif //__cplusplus

#endif //VEGA_STRIKE_ENGINE_VEGASTRIKE_H
