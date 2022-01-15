/**
 * enhancement.h
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


#ifndef _ENHANCER_H_
#define _ENHANCER_H_
#include "savegame.h"
#include "universe.h"
#include "unit_generic.h"

class Enhancement : public Unit {
public:
    Enhancement(const char *filename,
                int faction,
                const string &modifications,
                Flightgroup *flightgrp = NULL,
                int fg_subnumber = 0) :
            Unit(filename, false, faction, modifications, flightgrp, fg_subnumber)
    {
        string file(filename);
        this->filename = filename;
    }

protected:
    std::string filename;

    enum _UnitType isUnit() const
    {
        return _UnitType::enhancement;
    }

protected:
/// default constructor forbidden
    Enhancement();
/// copy constructor forbidden
    Enhancement(const Enhancement &);
/// assignment operator forbidden
    Enhancement &operator=(const Enhancement &);
};

#endif

