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

#ifndef __STARDATE_H
#define __STARDATE_H

#include <string>
using std::string;

#define HOURS_DIV 8

class StarDate
{
private:
    double *initial_star_time;
    double  initial_time;

//TREK Date stuff
    string ConvertTrekDate( double date );
    double ConvertTrekDate( string date );
    float GetFloatFromTrekDate( int faction = 0 );

public: StarDate();
    StarDate( double time );
    void Init( double time );
    double GetCurrentStarTime( int faction = 0 );
    double GetElapsedStarTime(int faction = 0);

//TREK Date stuff
    void InitTrek( string date );
    string GetTrekDate( int faction = 0 );
    string GetFullTrekDate( int faction = 0 );
    string ConvertFullTrekDate( double date );

//DAN.A StarDate
    void InitSDate( string date );
    string GetSDate( int faction = 0 );
    string GetFullSDate( int faction = 0 );

//Between date format conversion
    string SDateFromTrekDate( string trekdate );
    string TrekDateFromSDate( string sdate );
};

#endif

