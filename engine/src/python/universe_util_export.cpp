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

#include <boost/version.hpp>
#include <boost/python.hpp>
#include "python_class.h"
#include "cmd/container.h"
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"
#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"

void StarSystemExports() {
#define EXPORT_UTIL(name, aff) PYTHON_DEFINE_GLOBAL( VS, &UniverseUtil::name, #name );
#define voidEXPORT_UTIL(name) EXPORT_UTIL( name, 0 )
#undef EXPORT_FACTION
#undef voidEXPORT_FACTION
#define EXPORT_FACTION(name, aff) PYTHON_DEFINE_GLOBAL( VS, &FactionUtil::name, #name );
#define voidEXPORT_FACTION(name) EXPORT_FACTION( name, 0 )

    #include "star_system_exports.h"
}

#include <string>


