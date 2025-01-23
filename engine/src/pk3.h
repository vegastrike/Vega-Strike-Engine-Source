/*
 * pk3.h
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
/*
 *  Copyright 2002-2003 LibPK3, Inc. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in
 *  the documentation and/or other materials provided with the
 *  distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE LIBPK3 PROJECT
 *  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 *  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE LIBPK3 PROJECT OR CONTRIBUTORS
 *  BE LIABLE FORANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 *  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 *
 *  The views and conclusions contained in the software and
 *  documentation are those of the authors and should not be
 *  interpreted as representing official policies, either expressed or
 *  implied, of the LibPK3 Project or Viktor Liehr alias picard.
 *
 *  $Id$
 *
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

