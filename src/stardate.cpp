#include <assert.h>
#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "stardate.h"
#include "lin_time.h"
#include "vsfilesystem.h"
#include "cmd/script/mission.h"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;

class Faction;
extern vector< boost::shared_ptr<Faction> >factions;

StarDate::StarDate()
{
    initial_time = mission->getGametime();
    initial_star_time = NULL;
}

void StarDate::Init( double time )
{
    if (initial_star_time != NULL)
        delete[] initial_star_time;
    initial_time = mission->getGametime();
    initial_star_time = new double[factions.size()];
    for (unsigned int i = 0; i < factions.size(); i++)
        initial_star_time[i] = time;
}

//Get the current StarDate time in seconds
double StarDate::GetCurrentStarTime( int faction )
{
    //Get the number of seconds elapsed since the server start
    double time_since_server_started = mission->getGametime()-initial_time;
    //Add them to the current date
    if (initial_star_time == NULL)
        return time_since_server_started;
    else
        return initial_star_time[faction]+time_since_server_started;
}

//Needed to calculate relative message and mission times
//into stardate
double StarDate::GetElapsedStarTime(int faction)
{
    if (initial_star_time == NULL)
        return initial_time;
    else
        return initial_star_time[faction] - initial_time;
}

/*
 *********************************************************************************
 **** Trek Stardate stuff                                                      ***
 *********************************************************************************
 */

void StarDate::InitTrek( string date )
{
    if (initial_star_time != NULL)
        //we must be reinitializing;
        delete[] initial_star_time;
    initial_star_time = 0;
    initial_time = mission->getGametime();
    initial_star_time = new double[factions.size()];
    double init_time = this->ConvertTrekDate( date );
    VSFileSystem::vs_dprintf( 3, "Initializing stardate from a Trek date for %d factions", factions.size() );
    for (unsigned int i = 0; i < factions.size(); i++)
        initial_star_time[i] = init_time;
}

/// The stardate format is ddd.hhmm:sec with
/// ddd, hh, mm, sec = star day, hour, minute, seconds
/// The second equals our terran second. The minute however has 480 seconds (HOURS_DIV=8)
/// The hour has 60 minutes and the day has 100 hours

//Convert a StarDate time into a Stardate string
string StarDate::ConvertFullTrekDate( double date )
{
    unsigned int days, hours, minutes, seconds;
    char cdate[32];

    // Get the number of days dividing seconds number by the number of seconds in a day: 100*60*60*8 = 2880000
    days    = (unsigned int) date/2880000;
    // Modulo gives us the number of stardate seconds elapsed in the current day
    date    = (unsigned int) date%2880000;
    // Get the hours elapsed in the day by dividing by number of seconds in a stardate hour: 60*60*8 = 28800
    hours   = (unsigned int) date/28800; 
    // Modulo gives us the number of seconds elapsed in that hour
    date    = (unsigned int) date%28800;
    //Get the number of minutes elapsed in that hour by dividing by the number of seconds in a minute: 60*8 = 480
    minutes = (unsigned int) date/480;
    //The remaining seconds in the date
    //The seconds are counted from 0 to 480 before the minute effectively is incremented
    seconds = (unsigned int) date%480;

    //The hour/minute part is displayed like HHMM divided by HOURS_DIV which is 8 for now
    unsigned int hhmm  = (hours*100+minutes);

    sprintf( cdate, "%d.%.4d:%.3d", days, hhmm, seconds );
    return string( cdate );
}

string StarDate::ConvertTrekDate( double date )
{
    unsigned int days, hours, minutes;
    char cdate[32];

    days    = (unsigned int) date/2880000;
    date    = (unsigned int) date%2880000;
    hours   = (unsigned int) date/28800;
    date    = (unsigned int) date%28800;
    minutes = (unsigned int) date/480;

    unsigned int hhmm = (hours*100+minutes);

    sprintf( cdate, "%d.%.4d", days, hhmm );
    return string( cdate );
}

//Convert a StarDate into a number of seconds
double StarDate::ConvertTrekDate( string date )
{
    unsigned int days, hours, minutes, tmphrs, seconds, nb, pos;
    double res;
    //Replace the dot with 'a' so sscanf won't take it for a decimal symbol
    pos = date.find( "." );
    date.replace( pos, 1, "a" );
    if ( ( nb = sscanf( date.c_str(), "%da%4d:%3d", &days, &tmphrs, &seconds ) ) != 3 )
        VSFileSystem::vs_dprintf( 3, "!!! ERROR reading date\n");

    //Extract number of hours
    hours   = tmphrs/100;
    //Extract number of minutes
    minutes = tmphrs%100;

    res     = days*2880000+hours*28800+minutes*480+seconds;
    std::string formatted = ConvertFullTrekDate(res);
    VSFileSystem::vs_dprintf( 3, "Converted date to %ld, which stardate is %s\n", long(res), formatted.c_str() );
    return res;
}

//Get the current StarDate in a string
string StarDate::GetFullTrekDate( int faction )
{
    return ConvertFullTrekDate( this->GetCurrentStarTime( faction ) );
}

//Get the current StarDate in a string - short format
string StarDate::GetTrekDate( int faction )
{
    return ConvertTrekDate( this->GetCurrentStarTime( faction ) );
}

//Convert the string xxxx.y date format into a float representing the same data xxxx.y
float StarDate::GetFloatFromTrekDate( int faction )
{
    float  float_date;
    string cur_date = this->GetFullTrekDate( faction );
    sscanf( cur_date.c_str(), "%f", &float_date );

    return float_date;
}

/*
 *********************************************************************************
 **** DAN.A Stardate stuff                                                     ***
 *********************************************************************************
 */

void InitSDate( string date )
{}

string GetSDate( int faction = 0 )
{
    return string( "" );
}

string GetFullSDate( int faction = 0 )
{
    return string( "" );
}

/*
 *********************************************************************************
 **** String stardate formats conversions                                      ***
 *********************************************************************************
 */

string SDateFromTrekDate( string trekdate )
{
    return string( "" );
}

string TrekDateFromSDate( string sdate )
{
    return string( "" );
}

