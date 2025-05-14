/*
 * diskobject.h
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
#ifndef VEGA_STRIKE_ENGINE_DISK_OBJECT_H
#define VEGA_STRIKE_ENGINE_DISK_OBJECT_H

#include <deque>
#include <string>

#include <archive.h>

namespace vegastrike {
    namespace diskobject {

        class DiskObject {
            enum {
                ERROR_OK=0,
                ERROR_NO_FILE,
                ERROR_OPEN_READ_FAILED,
                ERROR_OPEN_WRITE_FAILED,
                ERROR_ARCHIVE_READ_FAILED,
                ERROR_ARCHIVE_CLOSE_FAILED,
                ERROR_ARCHIVE_FATAL_ERROR,
            };

            public:
                DiskObject() {}
                DiskObject(const std::string& _filename): filename(_filename) {}

                inline void SetFilename(const std::string& _filename) { filename = _filename; }
                int ReadEntry(const std::string& entry_name, std::string& data);
                int ListEntries(std::deque<std::string>& entries);
                int WriteEntry(const std::string& entry_name, const std::string& data);

                inline int GetError() const { return error_code; }
                inline void ClearError() { error_code = ERROR_OK; error_message.clear(); }

                inline const std::string GetErrorString() { return error_message; }
            protected:
                std::string filename{};
                int error_code{0};
                std::string error_message{};

                bool record_archive_error(int archive_error, struct archive* archive_file);
        };

    };
};

#endif //VEGA_STRIKE_ENGINE_DISK_OBJECT_H
