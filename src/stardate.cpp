#include <iostream>
#include "stardate.h"
#include "lin_time.h"

using std::cerr;
using std::endl;

StarDate::StarDate()
{
	// InitTime should have been called in the beginning of the main server loop
	initial_time = getNewTime();
	initial_star_time = 0;
}

StarDate::StarDate( string date)
{
	initial_time = getNewTime();
	initial_star_time = this->ConvertDate( date);
}

// Convert a StarDate time into a Stardate string
string	StarDate::ConvertFullDate( double date)
{
	unsigned int days, hours, minutes, seconds;

	days = ((unsigned int)date / (60*60*24));
	date = (unsigned int)date % (60*60*24);
	hours = (unsigned int)date / (60*60);
	date = (unsigned int)date % (60*60);
	minutes = (unsigned int)date / 60;
	seconds = (unsigned int)date % 60;

	char cdate[32];
	unsigned int hrs = (hours*100+minutes)/24;
	unsigned int secs = seconds + (hours*100+minutes)%24;

	sprintf( cdate, "%d.%d:%d", days, hrs, secs);
	return string( cdate);
}

string	StarDate::ConvertDate( double date)
{
	unsigned int days, hours, minutes, seconds;

	days = ((unsigned int)date / (60*60*24));
	date = (unsigned int)date % (60*60*24);
	hours = (unsigned int)date / (60*60);
	date = (unsigned int)date % (60*60);
	minutes = (unsigned int)date / 60;
	seconds = (unsigned int)date % 60;

	char cdate[32];
	unsigned int hrs = (hours*100+minutes)/24;

	sprintf( cdate, "%d.%d", days, hrs);
	return string( cdate);
}

// Convert a StarDate into a number of seconds
double	StarDate::ConvertDate( string date)
{
	unsigned int days, hours, minutes, seconds;
	if( sscanf( date.c_str(), "%d.%d.%d:%d", &days, &hours, &minutes, &seconds)!=4)
		cerr<<"!!! ERROR reading date"<<endl;

	return days*(24*60*60)*hours*(60*60)*minutes*(60)*seconds;
}

// Get the current StarDate time in seconds
double	StarDate::GetCurrentTime()
{
	// Get the number of seconds elapsed since the server start
	double time_since_server_started = getNewTime() - initial_time;
	// Add them to the current date
	return (initial_star_time + time_since_server_started);
}

// Get the current StarDate in a string
string	StarDate::GetFullCurrentDate()
{
	return ConvertFullDate( this->GetCurrentTime());
}

// Get the current StarDate in a string - short format
string	StarDate::GetCurrentDate()
{
	return ConvertDate( this->GetCurrentTime());
}

//Convert the string xxxx.y date format into a float representing the same data xxxx.y
float	StarDate::GetFloatFromDate()
{
	float float_date;
	string cur_date = this->GetFullCurrentDate();
	sscanf( cur_date.c_str(), "%f", &float_date);

	return float_date;
}

