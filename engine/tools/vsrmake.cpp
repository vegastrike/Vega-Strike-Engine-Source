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

/** @file vsrmake.cpp Main file for the vsrmake tool.
  This is the creator .vsr files.
  @todo This is highly POSIX specific. The code to descent directories only
  works on POSIX systems, and not even all of these I am afraid.

  @todo Implement proper error handling.

  @todo Allow packaging of files named to the standard input.
 */
#include "vsrtools.h"
#include "tools/common.h"
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

/** @addtogroup vsrtools Command line tools to handle the Vega Strike Resource
  Package files.
 */
using std::string;
using std::vector;

class PackageFileCreator {
public:
    PackageFileCreator(const string &file);

    ~PackageFileCreator(void) {
        finish();
    }

    void add(const string &anyname);
private:
    void finish(void);
    void addDirectory(const string &dirname);
    void addFile(const string &filename, uint32_t file_size);

    FILEHandle pkg;
    uint32_t offsetInPackage;
    vector<VSRMember> pkg_index;
};

static char usage[] =
        "Syntax:\tpkgcreate <pkgfile> <path1> [... <pathN>]\n"
        "\t<pkgfile> The package file to create or overwrite.\n"
        "\t<path1> The first file/directory to add to the package file.\n";

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s", usage);
        exit(0);
    }

    try {
        PackageFileCreator pkg(argv[1]);
        for (int i = 2; i < argc; ++i) {
            pkg.add(argv[i]);
        }
    } catch (int en) {
    }
    return 0;
}

PackageFileCreator::PackageFileCreator(const string &file)
        : pkg(file, "wb"), offsetInPackage(12) {
    offsetInPackage = 12;
}

void
PackageFileCreator::addFile(const string &fname, uint32_t file_size) {
    if (fseek(pkg, offsetInPackage, SEEK_SET)) {
        throw errno;
    }
    FILEHandle incoming(fname, "rb");
    char buf[1024];
    uint32_t size = file_size;

    while (size > 0) {
        int bi = fread(buf, 1, 1024, incoming);
        if (bi <= 0) {
            throw errno;
        }
        size -= bi;
        int bo = fwrite(buf, 1, bi, pkg);
        if (bi != bo) {
            throw errno;
        }
    }
    pkg_index.push_back(VSRMember(fname, file_size, offsetInPackage));
    offsetInPackage += file_size;
}

void
PackageFileCreator::addDirectory(const string &dname) {
    DIR *dd = opendir(dname.c_str());

    if (dd == 0) {
        throw errno;
    }
    for (struct dirent *dentry = readdir(dd); dentry != 0;
            dentry = readdir(dd)) {
        try {
            if (dentry->d_name[0] != '.') {
                add(dname + '/' + dentry->d_name);
            }
        } catch (int) {
        }
    }
    closedir(dd);
}

void
PackageFileCreator::add(const string &pname) {
    struct stat stats;

    stat(pname.c_str(), &stats);

    try {
        if (S_ISDIR(stats.st_mode)) {
            addDirectory(pname);
        } else if (S_ISREG(stats.st_mode)) {
            addFile(pname, stats.st_size);
        }
    } catch (int error) {
        fprintf(stderr, "Addition of %s failed:\n\t%s\n", pname.c_str(),
                strerror(error));
    }
}

void PackageFileCreator::finish(void) {
    //Sort the index
    sort(pkg_index.begin(), pkg_index.end());
    VSRHeader header;
    memcpy(header.magic, "VSR", 4);
    header.entryTableOffset = offsetInPackage;
    header.entries = pkg_index.size();
    fseek(pkg, 0, SEEK_SET);
    fwrite(&header, sizeof(VSRHeader), 1, pkg);
    //Now write entry table to file
    fseek(pkg, offsetInPackage, SEEK_SET);
    for (vector<VSRMember>::iterator ii = pkg_index.begin();
            ii != pkg_index.end(); ++ii) {
        VSRPEntry entry;
        entry.fileLength = ii->fileLength;
        entry.offset = ii->offset;
        memset(entry.filename, 0, 256);
        strcpy(entry.filename, ii->filename.c_str());
        fwrite(&entry, sizeof(VSRPEntry), 1, pkg);
    }
}
