/*
 * Copyright (C) 2001-2022 Daniel Horn, Konstantinos Arvanitis,
 * pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors.
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

/** @file vsrextract.cpp Main file for the vsrextract tool.
  This extracts all the files from a VSR package and places them in the current
  directory and below, creating directories as needed.

  @todo Proper documentation.

  @todo This is highly POSIX specific. The code to descend and create
  directories only works on POSIX systems, and not even all of these I am
  afraid.

  @todo Implement proper error handling.

  @todo Allow files to be extracted in a directory other than the current.
  
  @todo Allow only specified files to be extracted. (Low priority)
 */
#include "vsrtools.h"
#include "common.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>

using std::string;
using std::vector;
using std::min;

class PackageFileExtractor {
public:
    PackageFileExtractor(const string &fname);
    void extract(const string &topdir) const;
    void extract(const string &topdir, const string &entry) const;
private:
    void makePathExist(const string &path) const;
    void extractFile(const VSRMember &member) const;
    void extract(const VSRMember &member) const;
    FILEHandle pkg;
    vector<VSRMember> pkg_index;
};

static char usage[] =
        "Syntax:\tpkgopen <pkgfile>\n"
        "\t<pkgfile> The package file to open.\n";

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "%s", usage);
        exit(0);
    }

    try {
        PackageFileExtractor extractor(argv[1]);
        extractor.extract(".");
    } catch (int error) {
        fprintf(stderr, "Error extracting files from '%s'.\n%s\n", argv[1],
                strerror(error));
        exit(1);
    }

    return 0;
}

PackageFileExtractor::PackageFileExtractor(const string &fname)
        : pkg(fname, "rb") {
    VSRHeader hdr;

    if ((fread(&hdr, sizeof(VSRHeader), 1, pkg) != 1)
            || (strcmp(hdr.magic, "VSR") != 0)) {
        int tmp = errno;
        if (tmp == 0) {
            tmp = EINVAL;
        }
        throw tmp;
    }

    pkg_index.assign(hdr.entries, VSRMember());
    if (fseek(pkg, hdr.entryTableOffset, SEEK_SET)) {
        fprintf(stdout, "%s:%d!\n", __FILE__, __LINE__);
        throw errno;
    }

    for (unsigned ii = 0; ii < hdr.entries; ++ii) {
        VSRPEntry entry;
        if (fread(&entry, sizeof(VSRPEntry), 1, pkg) != 1) {
            int tmp = errno;
            if (tmp == 0) {
                tmp = EINVAL;
            }
            fprintf(stdout, "%s:%d!\n", __FILE__, __LINE__);
            throw tmp;
        }
        pkg_index[ii] = entry;
    }
}

void
PackageFileExtractor::extractFile(const VSRMember &file) const {
    if (fseek(pkg, file.offset, SEEK_SET)) {
        throw errno;
    }

    FILEHandle mfile(file.filename, "wb");
    uint32_t bytes_left = file.fileLength;
    uint32_t chunk, actual = 0;
    char buffer[1024];

    chunk = min(1024u, bytes_left);
    while (bytes_left && (actual = fread(buffer, 1, chunk, pkg)) > 0) {
        bytes_left -= actual;
        uint32_t written = fwrite(buffer, 1, actual, mfile);
        if (written != actual) {
            int tmp = errno;
            unlink(file.filename.c_str());
            fprintf(stdout, "%s:%d Error extracting '%s'!\n",
                    __FILE__, __LINE__, file.filename.c_str());
            throw tmp;
        }
        chunk = min(1024u, bytes_left);
    }
    if (bytes_left) {
        int tmp = errno;
        unlink(file.filename.c_str());
        throw tmp;
    }
}

void
PackageFileExtractor::extract(const VSRMember &member) const {
    size_t last_sep = member.filename.rfind('/');
    if (last_sep != string::npos) {
        makePathExist(string(member.filename.begin(),
                member.filename.begin() + last_sep));
    }
    extractFile(member);
}

void
PackageFileExtractor::extract(const string &targetpath) const {
    if (chdir(targetpath.c_str())) {
        int tmp = errno;
        fprintf(stdout, "%s:%d Unable to change directory to '%s'.\n",
                __FILE__, __LINE__, targetpath.c_str());
        throw tmp;
    }
    for (vector<VSRMember>::const_iterator ii = pkg_index.begin();
            ii != pkg_index.end(); ++ii) {
        extract(*ii);
    }
}

void
PackageFileExtractor::makePathExist(const string &path) const {
    fprintf(stdout, "Trying to make '%s'\n", path.c_str());
    size_t last_sep = path.rfind('/');
    if (last_sep != string::npos) {
        makePathExist(string(path.begin(),
                path.begin() + last_sep));
    }
    mkdir(path.c_str()
#ifndef WIN32
            , 0755
#endif
    );
}
