#include <assert.h>
#include <iostream>
#include <vector>
#include "stardate.h"
#include "lin_time.h"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;

class Faction;
extern vector <Faction *> factions;

StarDate::StarDate()
{
	initial_time = getNewTime();
	initial_star_time = NULL;
}

void	StarDate::Init( double time)
{
	assert( initial_star_time == NULL );

	initial_time = getNewTime();
	initial_star_time = new double[factions.size()];
	for( unsigned int i=0; i<factions.size(); i++)
		initial_star_time[i] = time;
}

// Get the current StarDate time in seconds
double	StarDate::GetCurrentStarTime( int faction)
{
    // Get the number of seconds elapsed since the server start
    double time_since_server_started = getNewTime() - initial_time;

    // Add them to the current date
    if( initial_star_time == NULL )
        return time_since_server_started;
    else
        return (initial_star_time[faction] + time_since_server_started);
}

/**********************************************************************************/
/**** Trek Stardate stuff                                                      ****/
/**********************************************************************************/

void	StarDate::InitTrek( string date)
{
	assert( initial_star_time == NULL );

	initial_star_time = 0;
	initial_time = getNewTime();
	initial_star_time = new double[factions.size()];
	double init_time = this->ConvertTrekDate( date);
	cerr<<"Initializing stardate from a Trek date for "<<factions.size()<<" factions"<<endl;
	for( unsigned int i=0; i<factions.size(); i++)
		initial_star_time[i] = init_time;
}

// Convert a StarDate time into a Stardate string
string	StarDate::ConvertFullTrekDate( double date)
{
	unsigned int days, hours, minutes, seconds;

	// Get the number of days dividing seconds number by the number of seconds in a day
	days = ((unsigned int)date / (60*60*24));
	// Modulo gives us the number of seconds elapsed in the current day
	date = (unsigned int)date % (60*60*24);
	// Get the hours elapsed in the day by dividing by number of seconds in an hour
	hours = (unsigned int)date / (60*60);
	// Modulo gives us the number of seconds elapsed in that hour
	date = (unsigned int)date % (60*60);
	// Get the number of minutes elapsed in that hour by dividing by the number of seconds in a minute
	minutes = (unsigned int)date / (60);
	// Modulo gives us the elapsed seconds in the current minute
	seconds = (unsigned int)date % (60);

	char cdate[32];
	// The hour/minute part is displayed like HHMM divided by HOURS_DIV which is 8 for now
	unsigned int hrs = (hours*100+minutes)/HOURS_DIV;
	// Then seconds must be counted from 0 to 480 (HOURS_DIV=8 minutes) before the minute effectively is incremented
	unsigned int secs = seconds + (hours*100+minutes)%(HOURS_DIV*60);

	sprintf( cdate, "%d.%.4d:%.2d", days, hrs, secs);
	return string( cdate);
}

string	StarDate::ConvertTrekDate( double date)
{
	unsigned int days, hours, minutes, seconds;

	days = ((unsigned int)date / (60*60*24));
	date = (unsigned int)date % (60*60*24);
	hours = (unsigned int)date / (60*60);
	date = (unsigned int)date % (60*60);
	minutes = (unsigned int)date / 60;
	seconds = (unsigned int)date % 60;

	char cdate[32];
	unsigned int hrs = (hours*100+minutes)/HOURS_DIV;

	sprintf( cdate, "%d.%.4d", days, hrs);
	return string( cdate);
}

// Convert a StarDate into a number of seconds
double	StarDate::ConvertTrekDate( string date)
{
	unsigned int days, hours, minutes, seconds, nb, pos;
	double res;
	// Replace the dot with 'a' so sscanf won't take it for a decimal symbol
	pos = date.find( ".");
	date.replace( pos, 1, "a");
	if( (nb=sscanf( date.c_str(), "%da%4d:%2d", &days, &minutes, &seconds))!=3)
	{
		cerr<<"!!! ERROR reading date"<<endl;
	}
	/*
	cerr<<"!!! Read "<<nb<<" arguments"<<endl;
	cerr<<"!!! Dot position : "<<pos<<endl;
	cerr<<"!!! String = "<<date<<" read = "<<days<<"."<<minutes<<":"<<seconds<<endl;
	*/

	// Get the number of hours and minutes back to the form HHMM
	int temphours = minutes*HOURS_DIV;
	// Extract number of hours
	hours = temphours/100;
	// Extract number of minutes
	minutes = temphours%100;

	res = days*(24*60*60)+hours*(60*60)+minutes*(60)+seconds;
	cerr<<"!!! Converted date to = "<<res<<" which stardate is "<<ConvertTrekDate(res)<<endl;
	return res;
}

// Get the current StarDate in a string
string	StarDate::GetFullTrekDate( int faction)
{
	return ConvertFullTrekDate( this->GetCurrentStarTime( faction));
}

// Get the current StarDate in a string - short format
string	StarDate::GetTrekDate( int faction)
{
	return ConvertTrekDate( this->GetCurrentStarTime( faction));
}

//Convert the string xxxx.y date format into a float representing the same data xxxx.y
float	StarDate::GetFloatFromTrekDate( int faction)
{
	float float_date;
	string cur_date = this->GetFullTrekDate( faction);
	//cout<<"------------------------- STARDATE "<<cur_date<<" --------------------------"<<endl;
	sscanf( cur_date.c_str(), "%f", &float_date);

	return float_date;
}

/**********************************************************************************/
/**** DAN.A Stardate stuff                                                     ****/
/**********************************************************************************/

void	InitSDate( string date)
{
}

string	GetSDate( int faction=0)
{
	return string( "");
}

string	GetFullSDate( int faction=0)
{
	return string( "");
}

/**********************************************************************************/
/**** String stardate formats conversions                                      ****/
/**********************************************************************************/

string	SDateFromTrekDate( string trekdate)
{
	return string( "");
}

string	TrekDateFromSDate( string sdate)
{
	return string( "");
}

