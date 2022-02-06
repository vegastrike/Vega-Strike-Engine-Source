/**
 * vid_file.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


//
//C++ Interface: vid_file
//
#ifndef __VID_FILE_H__INCLUDED__
#define __VID_FILE_H__INCLUDED__

#include <string>
#include <vsfilesystem.h>
#include <exception>

class VidFileImpl;

class VidFile {
public:
    class Exception : public std::exception {
    private:
        std::string _message;

    public:
        Exception()
        {
        }

        Exception(const Exception &other) : _message(other._message)
        {
        }

        explicit Exception(const std::string &message) : _message(message)
        {
        }

        virtual ~Exception()
        {
        }

        virtual const char *what() const noexcept
        {
            return _message.c_str();
        }
    };

    class UnsupportedCodecException : public Exception {
    public:
        UnsupportedCodecException()
        {
        }

        UnsupportedCodecException(const UnsupportedCodecException &other) : Exception(other)
        {
        }

        explicit UnsupportedCodecException(const std::string &message) : Exception(message)
        {
        }
    };

    class FileOpenException : public Exception {
    public:
        FileOpenException()
        {
        }

        FileOpenException(const FileOpenException &other) : Exception(other)
        {
        }

        explicit FileOpenException(const std::string &message) : Exception(message)
        {
        }
    };

    class FrameDecodeException : public Exception {
    public:
        FrameDecodeException()
        {
        }

        FrameDecodeException(const FrameDecodeException &other) : Exception(other)
        {
        }

        explicit FrameDecodeException(const std::string &message) : Exception(message)
        {
        }
    };

    class EndOfStreamException : public Exception {
    public:
        EndOfStreamException()
        {
        }

        EndOfStreamException(const EndOfStreamException &other) : Exception(other)
        {
        }

        explicit EndOfStreamException(const std::string &message) : Exception(message)
        {
        }
    };

public:
    VidFile();
    ~VidFile();

    bool isOpen() const;

    void open(const std::string &path, size_t maxDimension = 65535, bool forcePOT = false);
    void close();

/** Seeks to the specified time
 * @Returns true if frame changed, false otherwise.
 * @Throws EndOfStreamException when time lays past the end.
 * @Throws FrameDecodeException when an error occurs during frame decode.
 */
    bool seek(float time);

    float getFrameRate() const;
    float getDuration() const;
    int getWidth() const;
    int getHeight() const;
    void *getFrameBuffer() const;
    int getFrameBufferStride() const;

private:
    VidFileImpl *impl;
};

#endif //__VID_FILE_H__INCLUDED__

