/**
 * Exceptions.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_EXCEPTIONS_H__INCLUDED__
#define __AUDIO_EXCEPTIONS_H__INCLUDED__

#include <string>
#include <exception>
#include "Format.h"

namespace Audio {

/**
 * Base exception class
 *
 * @see std::exception
 *
 */
class Exception : public std::exception {
private:
    std::string _message;

public:
    Exception()
    {
    };
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

/**
 * File Open exception
 * @remarks thrown when an unrecoverable attempt to open a file fails.
 */
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

/**
 * Codec not found exception
 * @remarks thrown when an attempt to open a file with a certain codec fails,
 *      or when such codec cannot be found.
 */
class CodecNotFoundException : public Exception {
public:
    CodecNotFoundException()
    {
    }
    CodecNotFoundException(const CodecNotFoundException &other) : Exception(other)
    {
    }
    explicit CodecNotFoundException(const std::string &message) : Exception(message)
    {
    }
};

/**
 * File Format exception
 * @remarks thrown when an unrecoverable attempt to parse a file fails.
 */
class FileFormatException : public Exception {
public:
    FileFormatException()
    {
    }
    FileFormatException(const FileFormatException &other) : Exception(other)
    {
    }
    explicit FileFormatException(const std::string &message) : Exception(message)
    {
    }
};

/**
 * End-Of-Stream exception
 * @remarks thrown when an attempt to seek past the end of a stream is made.
 */
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

/**
 * Corrupt-Stream exception
 * @remarks thrown when a corruption in an already open and seemingly correct stream is found.
 *      @par It is possible that implementations may recover from this. If so, a second attempt
 *      at reading may or may not fail. Recoverability is reported by isFatal(). Notice that
 *      recoverable corruption means only that an attempt to recover is possible, not that the
 *      attempt will succeed.
 */
class CorruptStreamException : public Exception {
    bool fatal;
public:
    CorruptStreamException(const CorruptStreamException &other) : Exception(other)
    {
    }
    explicit CorruptStreamException(bool _fatal)
            : Exception(_fatal ? "Fatal corruption on stream" : "Recoverable corruption on stream"),
              fatal(_fatal)
    {
    }

    bool isFatal() const
    {
        return fatal;
    }
};

/**
 * Resource not loaded exception
 * @remarks thrown when an attempt to access a resource that hasn't been loaded yet is made,
 *      within a context where automatic loading is not possible.
 */
class ResourceNotLoadedException : public Exception {
public:
    ResourceNotLoadedException()
    {
    }
    ResourceNotLoadedException(const ResourceNotLoadedException &other) : Exception(other)
    {
    }
    explicit ResourceNotLoadedException(const std::string &message) : Exception(message)
    {
    }
};

/**
 * Resource already loaded exception
 * @remarks thrown when an attempt to load a resource that has already been loaded is made,
 *      within a context where transparent failure is not desirable, or such decision left
 *      to the caller (such cases should be explicitly documented).
 */
class ResourceAlreadyLoadedException : public Exception {
public:
    ResourceAlreadyLoadedException()
    {
    }
    ResourceAlreadyLoadedException(const ResourceAlreadyLoadedException &other) : Exception(other)
    {
    }
    explicit ResourceAlreadyLoadedException(const std::string &message) : Exception(message)
    {
    }
};

/**
 * Invalid parameters exception
 * @remarks thrown when a call to a method with invalid parameters is made.
 */
class InvalidParametersException : public Exception {
public:
    InvalidParametersException()
    {
    }
    InvalidParametersException(const InvalidParametersException &other) : Exception(other)
    {
    }
    explicit InvalidParametersException(const std::string &message) : Exception(message)
    {
    }
};

/**
 * Attempted to create an object that already existed
 */
class DuplicateObjectException : public Exception {
public:
    explicit DuplicateObjectException(const std::string &name) :
            Exception(std::string("Object with name \"") + name + "\" already existed")
    {
    }
};

/**
 * Seeked object did not exist
 */
class NotFoundException : public Exception {
public:
    explicit NotFoundException(const std::string &name) :
            Exception(std::string("Object with name \"") + name + "\" does not exist")
    {
    }
};

/**
 * Request for unimplemented features
 */
class NotImplementedException : public Exception {
public:
    explicit NotImplementedException(const std::string &name) :
            Exception(name + " has not been implemented yet")
    {
    }
};

/**
 * Unsupported format requested
 */
class UnsupportedFormatException : public Exception {
    Format format;
public:
    explicit UnsupportedFormatException(const std::string &where, const Format &fmt) :
            Exception(std::string("Unsupported format (" + where + ")")),
            format(fmt)
    {
    }

    const Format &getFormat() const
    {
        return format;
    }
};

/**
 * Ran out of memory while performing some operation
 */
class OutOfMemoryException : public Exception {
public:
    OutOfMemoryException()
    {
    }
};

};

#endif//__AUDIO_EXCEPTIONS_H__INCLUDED__
