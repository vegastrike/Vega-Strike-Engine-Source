/* 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  Account Managment - Network Client Account Managment - written by Stephane Vaxelaire <svax@free.fr>
*/

#ifndef __ACCOUNTS_H
#define __ACCOUNTS_H

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include "cmd/unit_generic.h"
#include "const.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

class Account
{
		ObjSerial	serial;
		char		haschar;
		bool		connected;

	public:
		  enum account_type {
			UNKNOWN,
			PLAYER,
			ADMIN,
		  } type;
		FILE *		fp;
		char	name[NAMELEN];
		char	passwd[NAMELEN];
		Unit *		unit;

		// Assume player has created ship/char -> to remove in the future
		Account() { haschar=1; connected=false; };

		void	display();
		short	isNew() { return !haschar;}
		void	setIsNew( int new1) { haschar = new1;}

		ObjSerial	getSerial() { return this->serial;}
		void		setSerial( ObjSerial sernum) { this->serial = sernum;}

		int		compareName( char *str)
		{
			//cout<<"Name : "<<name<<" - compared to : "<<str<<endl;
			return strncmp( name, str, strlen( name));
		}
		int		comparePass( char *str)
		{
			//cout<<"Pass : "<<passwd<<" - compared to : "<<str<<endl;
			return strncmp( passwd, str, strlen( passwd));
		}
		int		isConnected()
		{
			return (connected==true);
		}
		void	setConnected( bool mode)
		{
			connected = mode;
		}
};

Account * getAcctTemplate(const string &key);
vector<Account *> getAllAccounts();
void LoadAccounts(const char *filename);

#endif
