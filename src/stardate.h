#ifndef __STARDATE_H
#define __STARDATE_H

#include <string>
using std::string;

#define HOURS_DIV 8

/*
 * Stardate class
 * Full date format is days.hours.minutes:seconds
 * Short date format is days.hours.minutes
 * Compact date format is days.hours
 */

class StarDate
{
	private:
		double *		initial_star_time;
		double			initial_time;


		// TREK Date stuff
		string	ConvertTrekDate( double date);
		string	ConvertFullTrekDate( double date);
		double	ConvertTrekDate( string date);
		float	GetFloatFromTrekDate( int faction=0);
	public:
		StarDate();
		StarDate( double time);
		void	Init( double time);
		double	GetCurrentStarTime( int faction=0);

		// TREK Date stuff
		void	InitTrek( string date);
		string	GetTrekDate( int faction=0);
		string	GetFullTrekDate( int faction=0);

		// DAN.A StarDate
		void	InitSDate( string date);
		string	GetSDate( int faction=0);
		string	GetFullSDate( int faction=0);

		// Between date format conversion
		string	SDateFromTrekDate( string trekdate);
		string	TrekDateFromSDate( string sdate);
};

#endif

