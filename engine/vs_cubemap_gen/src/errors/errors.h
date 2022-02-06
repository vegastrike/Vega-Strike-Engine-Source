/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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

#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <ostream>
#include <fstream>
#include <exception>

class error {
public:
};

//copied from mesh_gfx for inspiration:
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

class MissingTexture : public Exception {
public:
    explicit MissingTexture(const string &msg) : Exception(msg)
    {
    }

    MissingTexture()
    {
    }
};

#endif


