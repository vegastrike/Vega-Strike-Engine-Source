#ifndef __STARDATE_H
#define __STARDATE_H

#include <string>
using std::string;

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

		string	ConvertDate( double date);
		string	ConvertFullDate( double date);
		double	ConvertDate( string date);

		double	GetCurrentTime();
		string	GetCurrentDate();
		string	GetFullCurrentDate();
};

#endif
