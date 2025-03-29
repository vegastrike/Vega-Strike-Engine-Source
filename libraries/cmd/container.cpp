/**
 * container.cpp
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


#include <stdlib.h>
#include "cmd/container.h"
#include "cmd/unit_generic.h"

UnitContainer::UnitContainer() : unit(nullptr) {
    VSCONSTRUCT1('U')
}

UnitContainer::UnitContainer(Unit *un) : unit(nullptr) {
    SetUnit(un);
    VSCONSTRUCT1('U');
}

UnitContainer::~UnitContainer() {
    VSDESTRUCT1
    if (unit) {
        unit->UnRef();
    }
    //bad idea...arrgh!
}

void UnitContainer::SetUnit(Unit *un) {
    //if the unit is null then go here otherwise if the unit is killed then go here
    if (un != NULL ? un->Killed() == true : true) {
        if (unit) {
            unit->UnRef();
        }
        unit = NULL;
        return;
    } else {
        if (unit) {
            unit->UnRef();
        }
        unit = un;
        unit->Ref();
    }
}

