/*
 * pk3.cpp
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
/*
 *  pk3.cpp
 *
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

/*
 * Modified by Stephane Vaxelaire on 2003/09/23
 * - Turned longs into ints in the headers (long are 8 bytes on some (all?) 64 bits archs)
 * - Added endianness support (was not working on big endian machine)
 */

/*
 * Modified by Stephen G. Tuggy 2020-10-28
 * - Use Boost logging instead of cerr output directly
 * - Use VSExit() rather than exit() directly
 */

/*
 * Further Modified by Stephen G. Tuggy 2021-09-06 and 2024-04-23
 * - Use VS_LOG, VS_LOG_AND_FLUSH, etc. instead of BOOST_LOG_TRIVIAL
 */

#include "pk3.h"
#include <cstdlib>
#include <iostream>
#include "posh.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "vs_exit.h"

using std::hex;

#pragma pack(2)

struct CPK3::TZipLocalHeader {
    enum {
        SIGNATURE = 0x04034b50,
        COMP_STORE = 0,
        COMP_DEFLAT = 8,
    };
    unsigned int sig;
    unsigned short version;
    unsigned short flag;
    unsigned short compression;     //COMP_xxxx
    unsigned short modTime;
    unsigned short modDate;
    unsigned int crc32;
    unsigned int cSize;
    unsigned int ucSize;
    unsigned short fnameLen;        //Filename string follows header.
    unsigned short xtraLen;         //Extra field follows filename.

    void correctByteOrder() {
        sig = POSH_LittleU32(sig);
        version = POSH_LittleU16(version);
        flag = POSH_LittleU16(flag);
        compression = POSH_LittleU16(compression);
        modTime = POSH_LittleU16(modTime);
        modDate = POSH_LittleU16(modDate);
        crc32 = POSH_LittleU32(crc32);
        cSize = POSH_LittleU32(cSize);
        ucSize = POSH_LittleU32(ucSize);
        fnameLen = POSH_LittleU16(fnameLen);
        xtraLen = POSH_LittleU16(xtraLen);
    }
};

struct CPK3::TZipDirHeader {
    enum {
        SIGNATURE = 0x06054b50,
    };
    unsigned int sig;
    unsigned short nDisk;
    unsigned short nStartDisk;
    unsigned short nDirEntries;
    unsigned short totalDirEntries;
    unsigned int dirSize;
    unsigned int dirOffset;
    unsigned short cmntLen;

    void correctByteOrder() {
        sig = POSH_LittleU32(sig);
        nDisk = POSH_LittleU16(nDisk);
        nStartDisk = POSH_LittleU16(nStartDisk);
        nDirEntries = POSH_LittleU16(nDirEntries);
        totalDirEntries = POSH_LittleU16(totalDirEntries);
        dirSize = POSH_LittleU32(dirSize);
        dirOffset = POSH_LittleU32(dirOffset);
        cmntLen = POSH_LittleU16(cmntLen);
    }
};

struct CPK3::TZipDirFileHeader {
    enum {
        SIGNATURE = 0x02014b50,
        COMP_STORE = 0,
        COMP_DEFLAT = 8,
    };
    unsigned int sig;
    unsigned short verMade;
    unsigned short verNeeded;
    unsigned short flag;
    unsigned short compression;     //COMP_xxxx
    unsigned short modTime;
    unsigned short modDate;
    unsigned int crc32;
    unsigned int cSize;           //Compressed size
    unsigned int ucSize;          //Uncompressed size
    unsigned short fnameLen;        //Filename string follows header.
    unsigned short xtraLen;         //Extra field follows filename.
    unsigned short cmntLen;         //Comment field follows extra field.
    unsigned short diskStart;
    unsigned short intAttr;
    unsigned int extAttr;
    unsigned int hdrOffset;

    char *GetName() const {
        return (char *) (this + 1);
    }

    char *GetExtra() const {
        return GetName() + fnameLen;
    }

    char *GetComment() const {
        return GetExtra() + xtraLen;
    }

    void correctByteOrder() {
        sig = POSH_LittleU32(sig);
        verMade = POSH_LittleU16(verMade);
        verNeeded = POSH_LittleU16(verNeeded);
        flag = POSH_LittleU16(flag);
        compression = POSH_LittleU16(compression);
        modTime = POSH_LittleU16(modTime);
        modDate = POSH_LittleU16(modDate);
        crc32 = POSH_LittleU32(crc32);
        cSize = POSH_LittleU32(cSize);
        ucSize = POSH_LittleU32(ucSize);
        fnameLen = POSH_LittleU16(fnameLen);
        xtraLen = POSH_LittleU16(xtraLen);
        cmntLen = POSH_LittleU16(cmntLen);
        diskStart = POSH_LittleU16(diskStart);
        intAttr = POSH_LittleU16(intAttr);
        extAttr = POSH_LittleU32(extAttr);
        hdrOffset = POSH_LittleU32(hdrOffset);
    }
};

#pragma pack()

CPK3::CPK3(FILE *n_f) {
    CheckPK3(n_f);
}

CPK3::CPK3(const char *filename) {
    Open(filename);
}

static size_t bogus_sizet; //added by chuck_starchaser to squash some warnings

bool CPK3::CheckPK3(FILE *f) {
    if (f == NULL) {
        return false;
    }
    TZipDirHeader dh;

    fseek(f, -(int) sizeof(dh), SEEK_END);

    long dhOffset = ftell(f);
    memset(&dh, 0, sizeof(dh));

    bogus_sizet = fread(&dh, sizeof(dh), 1, f);
    dh.correctByteOrder();
    //Check
    if (dh.sig != TZipDirHeader::SIGNATURE) {
        VS_LOG_FLUSH_EXIT(fatal, "PK3 -- BAD DIR HEADER SIGNATURE, NOT A PK3 FILE !", 1);
        return false;
    }
    //Go to the beginning of the directory.
    fseek(f, dhOffset - dh.dirSize, SEEK_SET);

    //Allocate the data buffer, and read the whole thing.
    m_pDirData = new char[dh.dirSize + dh.nDirEntries * sizeof(*m_papDir)];
    if (!m_pDirData) {
        VS_LOG_FLUSH_EXIT(fatal, "PK3 -- ERROR ALLOCATING DATA BUFFER !", 1);
        return false;
    }
    memset(m_pDirData, 0, dh.dirSize + dh.nDirEntries * sizeof(*m_papDir));
    bogus_sizet = fread(m_pDirData, dh.dirSize, 1, f);

    //Now process each entry.
    char *pfh = m_pDirData;
    m_papDir = (const TZipDirFileHeader **) (m_pDirData + dh.dirSize);

    bool ret = true;
    for (int i = 0; i < dh.nDirEntries && ret == true; i++) {
        TZipDirFileHeader &fh = *(TZipDirFileHeader *) pfh;
        fh.correctByteOrder();

        //Store the address of nth file for quicker access.
        m_papDir[i] = &fh;
        //Check the directory entry integrity.
        if (fh.sig != TZipDirFileHeader::SIGNATURE) {
            VS_LOG_FLUSH_EXIT(fatal, "PK3 -- ERROR BAD DIRECTORY SIGNATURE !", 1);
            ret = false;
        } else {
            pfh += sizeof(fh);
            //Convert UNIX slashes to DOS backlashes.
            for (int j = 0; j < fh.fnameLen; j++) {
                if (pfh[j] == '/') {
                    pfh[j] = '\\';
                }
            }
            //Skip name, extra and comment fields.
            pfh += fh.fnameLen + fh.xtraLen + fh.cmntLen;
        }
    }
    if (ret != true) {
        delete[] m_pDirData;
    } else {
        m_nEntries = dh.nDirEntries;
        this->f = f;
    }
    return ret;
}

bool CPK3::Open(const char *filename) {
    f = fopen(filename, "rb");
    if (f) {
        strcpy(pk3filename, filename);
        return CheckPK3(f);
    } else {
        return false;
    }
}

bool CPK3::ExtractFile(const char *lp_name) {
    return ExtractFile(lp_name, lp_name);
}

bool CPK3::ExtractFile(const char *lp_name, const char *new_filename) {
    //open file tp write data
    FILE *new_f = NULL;
    int size = -1;

    char *data_content = ExtractFile(lp_name, &size);
    if (data_content) {
        if (size != -1) {
            new_f = fopen(new_filename, "wb");
            fwrite(data_content, 1, size, new_f);
            fclose(new_f);
            delete data_content;
            return true;
        }
    }
    return false;     //probably file not found
}

//Compares 2 c-strings but do not take into account '/' or '\'
int vsstrcmp(const char *lp, const char *str) {
    unsigned int i, ok = 1;
    unsigned int len = strlen(lp);
    for (i = 0; ok && i < len; i++) {
        if (lp[i] != '/' && lp[i] != '\\' && lp[i] != str[i]) {
            ok = 0;
        }
    }
    return !ok;
}

int CPK3::FileExists(const char *lpname) {
    char str[PK3LENGTH];
    int idx = -1;

    memset(&str, 0, sizeof(str));
    for (int i = 0; idx == -1 && i < m_nEntries; i++) {
        GetFilename(i, str);
        int result = vsstrcmp(lpname, str);
        if (result == 0) {
            VS_LOG(info, (boost::format("FOUND IN PK3 FILE : %1% with index=%2%") % lpname % i));
            idx = i;
        }
    }
    //if the file isn't in the archive idx=-1
    return idx;
}

char *CPK3::ExtractFile(int index, int *file_size) {
    char *buffer;
    int flength = GetFileLen(index);

    buffer = new char[flength];
    if (!buffer) {
        VS_LOG(error, "Unable to allocate memory, probably memory too low !!!");
        return NULL;
    } else {
        if (true == ReadFile(index, buffer)) {
            //everything went well !!!
        } else {
            VS_LOG(error,
                    "\nThe file was found in the archive, but I was unable to extract it. Maybe the archive is broken.\n");
        }
    }
    *file_size = flength;
    return buffer;
}

char *CPK3::ExtractFile(const char *lpname, int *file_size) {
    char str[PK3LENGTH];
    int index = -1;
    char *buffer;

    memset(&str, 0, sizeof(str));
    for (int i = 0; index == -1 && i < m_nEntries; i++) {
        GetFilename(i, str);
        int result = vsstrcmp(lpname, str);
        if (result == 0) {
            index = i;
        }
    }
    //if the file isn't in the archive
    if (index == -1) {
        return (NULL);
    }
    int flength = GetFileLen(index);

    buffer = new char[flength];
    if (!buffer) {
        VS_LOG(error, "Unable to allocate memory, probably memory too low !!!");
        return NULL;
    } else {
        if (true == ReadFile(index, buffer)) {
            //everything went well !!!
        } else {
            VS_LOG(error,
                    "\nThe file was found in the archive, but I was unable to extract it. Maybe the archive is broken.\n");
        }
    }
    *file_size = flength;
    return buffer;
}

bool CPK3::Close() {
    fclose(f);
    delete[] m_pDirData;
    m_nEntries = 0;

    return true;
}

void CPK3::PrintFileContent() {
    VS_LOG(info, (boost::format("PK3 File: %1%\n") % pk3filename));
    VS_LOG(info, (boost::format("files count: %1%\n\n") % m_nEntries));
    for (int i = 0; i < m_nEntries; i++) {
    }
}

void CPK3::GetFilename(int i, char *pszDest) const {
    if (pszDest != NULL) {
        if (i < 0 || i >= m_nEntries) {
            *pszDest = '\0';
        } else {
            memcpy(pszDest, m_papDir[i]->GetName(), m_papDir[i]->fnameLen);
            pszDest[m_papDir[i]->fnameLen] = '\0';
        }
    }
}

int CPK3::GetFileLen(int i) const {
    if (i < 0 || i >= m_nEntries) {
        return -1;
    } else {
        return m_papDir[i]->ucSize;
    }
}

bool CPK3::ReadFile(int i, void *pBuf) {
    if (pBuf == nullptr) {
        VS_LOG(error, "PK3ERROR :  pBuf is NULL !!!");
        return false;
    } else if (i < 0) {
        VS_LOG(error, "PK3ERROR :  Bad index < 0 !!!");
        return false;
    } else if (i >= m_nEntries) {
        VS_LOG(error, "PK3ERROR :  Index TOO BIG !!!");
        return false;
    }

    //Quick'n dirty read, the whole file at once.
    //Ungood if the ZIP has huge files inside

    //Go to the actual file and read the local header.
    fseek(this->f, m_papDir[i]->hdrOffset, SEEK_SET);
    TZipLocalHeader h;

    memset(&h, 0, sizeof(h));
    bogus_sizet = fread(&h, sizeof(h), 1, this->f);
    h.correctByteOrder();
    if (h.sig != TZipLocalHeader::SIGNATURE) {
        VS_LOG(error, "PK3ERROR - BAD LOCAL HEADER SIGNATURE !!!");
        return false;
    }
    //Skip extra fields
    fseek(this->f, h.fnameLen + h.xtraLen, SEEK_CUR);
    if (h.compression == TZipLocalHeader::COMP_STORE) {
        //Simply read in raw stored data.
        bogus_sizet = fread(pBuf, h.cSize, 1, this->f);
        return true;
    } else if (h.compression != TZipLocalHeader::COMP_DEFLAT) {
        VS_LOG(error,
                (boost::format("BAD Compression level, found=%1% - expected=%2%") % h.compression
                        % TZipLocalHeader::COMP_DEFLAT));
        return false;
    }
    //Alloc compressed data buffer and read the whole stream
    char *pcData = new char[h.cSize];
    if (!pcData) {
        VS_LOG(error, "PK3ERROR : Could not allocate memory buffer for decompression");
        return false;
    }
    memset(pcData, 0, h.cSize);
    bogus_sizet = fread(pcData, h.cSize, 1, this->f);

    bool ret = true;

    //Setup the inflate stream.
    z_stream stream;
    int err, err2;

    stream.next_in = (Bytef *) pcData;
    stream.avail_in = (uInt) h.cSize;
    stream.next_out = (Bytef *) pBuf;
    stream.avail_out = h.ucSize;
    stream.zalloc = (alloc_func) 0;
    stream.zfree = (free_func) 0;

    //Perform inflation. wbits < 0 indicates no zlib header inside the data.
    err = inflateInit2(&stream, -MAX_WBITS);
    if (err == Z_OK) {
        err = inflate(&stream, Z_FINISH);
        if (err == Z_STREAM_END) {
            err = Z_OK;
        } else if (err == Z_NEED_DICT)
            VS_LOG(error, "PK3ERROR : Needed a dictionary");
        else if (err == Z_DATA_ERROR)
            VS_LOG(error, "PK3ERROR : Bad data buffer");
        else if (err == Z_STREAM_ERROR)
            VS_LOG(error, "PK3ERROR : Bad parameter, stream error");
        err2 = inflateEnd(&stream);
        if (err2 == Z_STREAM_ERROR)
            VS_LOG(error, "PK3ERROR : Bad parameter, stream error");
        err2 = inflateEnd(&stream);
        if (err2 == Z_STREAM_ERROR)
            VS_LOG(error, "PK3ERROR : Bad parameter, stream error");
    } else {
        if (err == Z_STREAM_ERROR)
            VS_LOG(error, "PK3ERROR : Bad parameter, stream error");
        else if (err == Z_MEM_ERROR)
            VS_LOG(error, "PK3ERROR : Memory error");
    }
    if (err != Z_OK) {
        VS_LOG(error, "PK3ERROR : Bad decompression return code");
        ret = false;
    }
    delete[] pcData;
    return ret;
}

