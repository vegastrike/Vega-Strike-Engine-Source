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
		double			initial_star_time;
		double			initial_time;

	public:
		StarDate();
		StarDate( string date);
		void	Init( string date);

		string	ConvertStarDate( double date);
		string	ConvertFullStarDate( double date);
		double	ConvertStarDate( string date);

		double	GetCurrentStarTime();
		string	GetCurrentStarDate();
		string	GetFullCurrentStarDate();

		float	GetFloatFromDate();
};

#endif
