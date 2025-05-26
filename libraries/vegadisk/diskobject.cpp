/*
 * diskobject.cpp
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

#include "vegadisk/diskobject.h"

#include "src/vs_logging.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>

#include <archive.h>
#include <archive_entry.h>

using namespace vega_disk;

bool DiskObject::record_archive_error(int archive_error, struct archive* archive_file) {
    if (archive_error != ARCHIVE_OK) {
        error_code = archive_errno(archive_file);
        error_message = archive_error_string(archive_file);
        return true;
    }

    return false;
}

int DiskObject::ListEntries(std::deque<std::string>& entries) {
    if (filename.empty()) {
        error_message = "No file to open";
        VS_LOG(error, (boost::format("Error opening file %1% - Message (%2%)") % filename % error_message ));
        return DiskObject::ERROR_NO_FILE;
    }

    FILE* input_file = fopen(filename.c_str(), "r+b");
    if (input_file == nullptr) {
        error_code = errno;
        error_message = strerror(errno);
        VS_LOG(error, (boost::format("Error opening file %1% - Message (%2%)") % filename % error_message ));
        return DiskObject::ERROR_OPEN_READ_FAILED;
    }

    struct archive* archive_file = archive_read_new();
    // allow the input to be any format and compression supported by libarchive
    archive_read_support_filter_all(archive_file);
    archive_read_support_format_all(archive_file);

    int archive_error = archive_read_open_FILE(archive_file, input_file);
    if (record_archive_error(archive_error, archive_file)) {
        return DiskObject::ERROR_ARCHIVE_READ_FAILED;
    }

    // reset the list
    entries.clear();

    struct archive_entry* current_entry = nullptr;
    while (archive_read_next_header(archive_file, &current_entry) == ARCHIVE_OK) {
        std::string path(archive_entry_pathname(current_entry));

        entries.push_back(path);
    }

    if (record_archive_error(archive_error, archive_file)) {
        // quick cleanup before returning
        // but don't check the error since its already in an error state
        archive_error = archive_read_free(archive_file);
        if (archive_error != ARCHIVE_OK) {
            VS_LOG(error, (boost::format("Error reading file %1% - Message (%2%)") % filename % archive_error_string(archive_file)));
        }
        if (fclose(input_file) != 0) {
            VS_LOG(error, (boost::format("Error closing file %1% - Message (%2%)") % filename % strerror(errno) ));
        }

        return DiskObject::ERROR_ARCHIVE_READ_FAILED;
    }

    archive_error = archive_read_free(archive_file);
    if (record_archive_error(archive_error, archive_file)) {
        VS_LOG(error, (boost::format("Error closing archive after reading %1% - Message (%2%)") % filename % archive_error_string(archive_file) ));
        if (fclose(input_file) != 0) {
            VS_LOG(error, (boost::format("Error closing file after reading %1% - Message (%2%)") % filename % strerror(errno) ));
        }
        return DiskObject::ERROR_ARCHIVE_CLOSE_FAILED;
    }

    if (fclose(input_file) != 0) {
        VS_LOG(error, (boost::format("Error closing file after reading %1% - Message (%2%)") % filename % strerror(errno) ));
    }
    return DiskObject::ERROR_OK;
}


// ReadEntry opens an archive file to extract a file by the name of `entry_name`
// parameter and return its data via the `data` parameter. If there's an error then
// it returns an integer to denote the issue and records the information to the VS Logs
int DiskObject::ReadEntry(const std::string& entry_name, std::string& data) {
    if (filename.empty()) {
        error_message = "No file to open";
        VS_LOG(error, (boost::format("Error opening file entry %1% from %2% - Message (%3%)") % entry_name % filename % error_message ));
        return DiskObject::ERROR_NO_FILE;
    }

    FILE* input_file = fopen(filename.c_str(), "r+b");
    if (input_file == nullptr) {
        error_code = errno;
        error_message = strerror(errno);
        VS_LOG(error, (boost::format("Error opening file entry %1% from %2% - Message (%3%)") % entry_name % filename % error_message ));
        return DiskObject::ERROR_OPEN_READ_FAILED;
    }

    struct archive* archive_file = archive_read_new();
    // allow the input to be any format and compression supported by libarchive
    archive_read_support_filter_all(archive_file);
    archive_read_support_format_all(archive_file);

    int archive_error = archive_read_open_FILE(archive_file, input_file);
    if (record_archive_error(archive_error, archive_file)) {
        return DiskObject::ERROR_ARCHIVE_READ_FAILED;
    }

    struct archive_entry* current_entry = nullptr;
    while (archive_read_next_header(archive_file, &current_entry) == ARCHIVE_OK) {
        std::string path(archive_entry_pathname(current_entry));

        if (path == entry_name) {
            data.clear();


            while (true) {
                const void* buffer = nullptr;
                size_t buffer_size = 0;
                la_int64_t archive_offset = 0;

                archive_error = archive_read_data_block(archive_file, &buffer, &buffer_size, &archive_offset);
                switch (archive_error) {
                    case ARCHIVE_WARN:
                        // warning occurred - but it is still okay to use the data
                        // okay, keep going
                        record_archive_error(archive_error, archive_file);
                        VS_LOG(warning, (boost::format("Warning while reading file entry %1% from %2% - Archive Warning (%3%)") % entry_name % filename % error_message ));
                        // fall thru to ARCHIVE_OK which saves the data
                    case ARCHIVE_OK:
                        data += std::string(static_cast<const char*>(buffer), buffer_size);
                        break;

                    case ARCHIVE_EOF:
                        // end of archive - no more data
                        // `goto` is used to short circuit the two while loops
                        goto end_archive_read;

                    case ARCHIVE_RETRY:
                        // read failed, but it can be tried again
                        // simply repeat the loop
                        continue;

                    case ARCHIVE_FATAL:
                        // archive fatally errored, close it and return
                        // don't try to do anything else with it

                        // capture error data
                        record_archive_error(archive_error, archive_file);
                        VS_LOG(error, (boost::format("Fatal error while reading file entry %1% from %2% - Message (%3%)") % entry_name % filename % error_message ));
                        archive_read_free(archive_file);
                        fclose(input_file);
                        return DiskObject::ERROR_ARCHIVE_FATAL_ERROR;
                };
            }
        }
    }
end_archive_read:

    if (record_archive_error(archive_error, archive_file)) {
        // quick cleanup before returning
        // but don't check the error since its already in an error state
        archive_error = archive_read_free(archive_file);
        if (archive_error != ARCHIVE_OK) {
            VS_LOG(error, (boost::format("Error reading file entry %1% from %2% - Message (%3%)") % entry_name % filename % archive_error_string(archive_file)));
        }
        if (fclose(input_file) != 0) {
            VS_LOG(error, (boost::format("Error closing file while reading entry %1% from %2% - Message (%3%)") % entry_name % filename % strerror(errno) ));
        }

        return DiskObject::ERROR_ARCHIVE_READ_FAILED;
    }

    archive_error = archive_read_free(archive_file);
    if (record_archive_error(archive_error, archive_file)) {
        VS_LOG(error, (boost::format("Error closing archive after reading entry %1% from %2% - Message (%3%)") % entry_name % filename % archive_error_string(archive_file) ));
        if (fclose(input_file) != 0) {
            VS_LOG(error, (boost::format("Error closing file after entry %1% from %2% - Message (%3%)") % entry_name % filename % strerror(errno) ));
        }
        return DiskObject::ERROR_ARCHIVE_CLOSE_FAILED;
    }

    if (fclose(input_file) != 0) {
        VS_LOG(error, (boost::format("Error closing file after entry %1% from %2% - Message (%3%)") % entry_name % filename % strerror(errno) ));
    }
    return DiskObject::ERROR_OK;
}

int DiskObject::WriteEntry(const std::string& entry_name, const std::string& data) {
    return -1;
}
