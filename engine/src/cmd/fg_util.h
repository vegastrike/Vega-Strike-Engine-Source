/**
 * fg_util.h
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
// NO HEADER GUARD

namespace fg_util {
std::string MakeFactionKey(int faction);
std::string MakeFGKey(const std::string &fgname, int faction);
std::string MakeStarSystemFGKey(const std::string &starsystem);
unsigned int ShipListOffset();
unsigned int PerShipDataSize();
bool IsFGKey(const std::string &fgcandidate);
bool CheckFG(std::vector<std::string> &data);
void PurgeZeroShips(SaveGame *sg, unsigned int faction);
void PurgeZeroShips(SaveGame *sg);
}

