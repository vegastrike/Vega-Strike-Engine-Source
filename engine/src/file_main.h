/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_FILE_MAIN_H
#define VEGA_STRIKE_ENGINE_FILE_MAIN_H

#include <stdio.h>
#include <string.h>
//#include "gfxlib.h"
#include "endianness.h"
#include "vsfilesystem.h"
using VSFileSystem::VSFile;
//using namespace VSFileSystem;
extern VSFile fpread;

/*File utility functions*/
inline void LoadFile(const char *filename) {
    fpread.OpenReadOnly(filename);
}

inline void CloseFile() {
    fpread.Close();
}

inline float readf(VSFileSystem::VSFile &f) {
    union {
        float fval;
        unsigned int ival;
    }
            t;
    f.Read(&t.fval, sizeof t.fval);
    t.ival = le32_to_cpu(t.ival);
    return t.fval;
}

inline size_t readf(VSFileSystem::VSFile &f, float *b, int n) {
    int i;
    size_t rode = f.Read(b, sizeof(*b) * n);
#ifndef NATURAL_ENDIANNESS
    for (i = 0; i < n; i++) {
        ((unsigned int *) b)[i] = le32_to_cpu(((unsigned int *) b)[i]);
    }
#endif
    return (rode > 0) ? (rode / sizeof(*b)) : rode;
}

inline short reads(VSFileSystem::VSFile &f) {
    short temp;
    f.Read(&temp, sizeof(short));
    return le16_to_cpu(temp);
}

inline int readi(VSFileSystem::VSFile &f) {
    int i;
    f.Read(&i, sizeof(int));
    return le32_to_cpu(i);
}

inline size_t readi(VSFileSystem::VSFile &f, int *b, int n) {
    int i;
    size_t rode = f.Read(b, sizeof(*b) * n);
#ifndef NATURAL_ENDIANNESS
    for (i = 0; i < n; i++) {
        b[i] = le32_to_cpu(b[i]);
    }
#endif
    return (rode > 0) ? (rode / sizeof(*b)) : rode;
}

inline unsigned char readc(VSFileSystem::VSFile &f) {
    unsigned char temp;
    f.Read(&temp, sizeof(char));
    return temp;
}

/*Read simple data*/
inline void ReadInt(int &integer) {
    fpread.Read(&integer, sizeof(int));
    integer = le32_to_cpu(integer);
}

inline void ReadFloat(float &num) {
    fpread.Read(&num, sizeof(float));
    *((int *) &num) = le32_to_cpu(*((int *) &num));
}

inline void ReadString(char *string) {
    int length = strlen(string);

    ReadInt(length);
    fpread.Read(string, length);
    string[length] = '\0';
}

/*Read aggregated data*/
inline void ReadVector(float &x, float &y, float &z) {
    ReadFloat(x);
    ReadFloat(y);
    ReadFloat(z);
}

inline void ReadVector(Vector &v) {
    ReadVector(v.i, v.j, v.k);
}

inline void ReadGeneric(char *string, float &x, float &y, float &z) {
    ReadString(string);
    ReadVector(x, y, z);
}

/*The goods*/
inline void ReadUnit(char *filename, int &type, float &x, float &y, float &z) {
    ReadGeneric(filename, x, y, z);
}

inline void ReadMesh(char *filename, float &x, float &y, float &z) {
    ReadGeneric(filename, x, y, z);
}

inline void ReadWeapon(char *filename, float &x, float &y, float &z) {
    ReadGeneric(filename, x, y, z);
}

inline void ReadRestriction(int &isrestricted, float &start, float &end) {
    ReadInt(isrestricted);
    ReadFloat(start);
    ReadFloat(end);
}

inline long GetPosition() {
    return fpread.GetPosition();
}

inline void SetPosition(long position) {
    fpread.GoTo(position);
}

#endif //VEGA_STRIKE_ENGINE_FILE_MAIN_H
