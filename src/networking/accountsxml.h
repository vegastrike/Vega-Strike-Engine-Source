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
#include "const.h"
#include <assert.h>
#include "netui.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

class Account
{
		ObjSerial	serial;
		char		haschar;
		bool		connected;
		SOCKETALT	server_sock;

	public:
		  enum account_type {
			UNKNOWN,
			PLAYER,
			ADMIN,
		  } type;
		FILE *		fp;
		string callsign;
		string passwd;
		//char	name[NAMELEN];
		//char	passwd[NAMELEN];

		// Assume player has created ship/char -> to remove in the future
		Account() { haschar=1; connected=false; };
		Account( char * scallsign, char * spasswd)
		{
			callsign = string(scallsign);
			passwd = string( spasswd);
		}
		Account( string scallsign, string spasswd)
		{
			callsign = scallsign;
			passwd = spasswd;
		}

		void	display();
		short	isNew() { return !haschar;}
		void	setIsNew( int new1) { haschar = new1;}

		ObjSerial	getSerial() { return this->serial;}
		void		setSerial( ObjSerial sernum) { this->serial = sernum;}
		SOCKETALT	getSocket() { return this->server_sock;}
		void		setSocket( SOCKETALT sck) { this->server_sock = sck;}
		void		set( char * scallsign, char * spasswd)
		{
			callsign = string( scallsign);
			passwd = string( spasswd);
		}
		void		set( string scallsign, string spasswd)
		{
			callsign = scallsign;
			passwd = spasswd;
		}

		int		compareName( char *str)
		{ return callsign==string( str); }
		int		comparePass( char *str)
		{ return passwd==string( str); }

		int		compareName( string str)
		{ return callsign==str; }
		int		comparePass( string str)
		{ return passwd==str; }

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
