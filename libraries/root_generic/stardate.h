/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_STARDATE_H
#define VEGA_STRIKE_ENGINE_STARDATE_H

/// Provides functions for stardate and startime manipulation and conversion
/// There are various time measurement systems in VS
/// The stardate is an absolute time measurement in the game calendar
/// Stardate is the in-game universe date in the format ddd.hhmm:sec
/// Startime is the internal numeric representation of the stardate as number of seconds
/// since the begin of initial universe time, which in Vega Strike UTCS universe is 3276800
/// The formatting of the startime into faction specific output is done by the stardate.py script

/*
 * Stardate class
 * Full date format is days.hours.minutes:seconds
 * Short date format is days.hours.minutes
 * Compact date format is days.hours
 */

#include <string>
using std::string;

#define HOURS_DIV 8

class StarDate {
private:
    double *initial_star_time;
    double initial_time;

//TREK Date stuff
    string ConvertTrekDate(double date);
    double ConvertTrekDate(string date);
    float GetFloatFromTrekDate(int faction = 0);

public:
    StarDate();
    StarDate(double time);
    void Init(double time);
    double GetCurrentStarTime(int faction = 0);
    double GetElapsedStarTime(int faction = 0);

//TREK Date stuff
    void InitTrek(string date);
    string GetTrekDate(int faction = 0);
    string GetFullTrekDate(int faction = 0);
    string ConvertFullTrekDate(double date);

//DAN.A StarDate
    void InitSDate(string date);
    string GetSDate(int faction = 0);
    string GetFullSDate(int faction = 0);

//Between date format conversion
    string SDateFromTrekDate(string trekdate);
    string TrekDateFromSDate(string sdate);
};

#endif //VEGA_STRIKE_ENGINE_STARDATE_H

