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
#ifndef COMMON_CODE_FOR_VSRP_TOOLS_
#define COMMON_CODE_FOR_VSRP_TOOLS_
#include <stdio.h>
#include <stdint.h>
#include <string>
#include "vsrtools.h"

class FILEHandle {
public:
    FILEHandle(const std::string &name, const char perms[]);

    ~FILEHandle()
    {
        if (file) {
            fclose(file);
        }
    }

    operator FILE *const(void) const
    {
        return file;
    }

private:
    FILE *file;
};

struct VSRMember {
    std::string filename;
    uint32_t fileLength;
    uint32_t offset;

    friend bool operator<(const VSRMember &a, const VSRMember &b)
    {
        return a.filename < b.filename;
    }

    VSRMember()
    {
    }

    VSRMember &operator=(const VSRPEntry &entry)
    {
        filename = entry.filename;
        fileLength = entry.fileLength;
        offset = entry.offset;
        return *this;
    }

    VSRMember(const VSRPEntry &entry)
            : filename(entry.filename), fileLength(entry.fileLength),
              offset(entry.offset)
    {
    }

    VSRMember(std::string nam, uint32_t len, uint32_t offs)
            : filename(nam), fileLength(len), offset(offs)
    {
    }
};

#endif //COMMON_CODE_FOR_VSRP_TOOLS_
