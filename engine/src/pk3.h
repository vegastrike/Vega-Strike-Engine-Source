/*
 * pk3.h
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

#ifndef __PK3__H__
#define __PK3__H__

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#define PK3LENGTH 512

class CPK3 {
private:
    struct TZipDirHeader;
    struct TZipDirFileHeader;
    struct TZipLocalHeader;

    FILE *f;
    char pk3filename[PK3LENGTH];
    char *m_pDirData;     //Raw data buffer.
    int m_nEntries;     //Number of entries.

//Pointers to the dir entries in pDirData.
    const TZipDirFileHeader **m_papDir;
    void GetFilename(int i, char *pszDest) const;
    int GetFileLen(int i) const;
    bool ReadFile(int i, void *pBuf);

public:
    CPK3() : m_nEntries(0) {
    }

    CPK3(FILE *n_f);
    CPK3(const char *filename);

    ~CPK3() {
    }

    bool CheckPK3(FILE *f);
    bool Open(const char *filename);
    bool ExtractFile(const char *lp_name);
    bool ExtractFile(const char *lp_name, const char *new_filename);
    char *ExtractFile(int index, int *file_size);
    char *ExtractFile(const char *lpname, int *file_size);
    int FileExists(const char *lpname);                                       //Checks if a file exists and returns index or -1 if not found
    bool Close(void);

    void PrintFileContent();
};

#endif

