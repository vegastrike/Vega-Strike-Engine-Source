#ifndef VSNET_ADDRESS_H
#define VSNET_ADDRESS_H

#include <config.h>

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
  netUI - Network Interface - written by Stephane Vaxelaire <svax@free.fr>
*/

#include <iostream>
#include "networking/const.h"
#include "vsnet_headers.h"

struct AddressIP : public sockaddr_in
{
    AddressIP( ) {
        memset( this, 0, sizeof(AddressIP) );
    }
    AddressIP( const AddressIP& orig ) {
        memcpy( this, &orig, sizeof(AddressIP) );
    }
    AddressIP( const sockaddr_in& orig ) {
        memcpy( this, &orig, sizeof(AddressIP) );
    }

	AddressIP( const AddressIP& orig, unsigned short port );
	AddressIP (const char *address, unsigned short port);
	
    AddressIP& operator=( const AddressIP& orig ) {
        memcpy( this, &orig, sizeof(AddressIP) );
        return *this;
    }
    AddressIP& operator=( const sockaddr_in& orig ) {
        memcpy( this, &orig, sizeof(AddressIP) );
        return *this;
    }
    /// print ip:port to stdout
    void dump( );

    /** note that this string is temporary and should not be used for
     *  anything but debug output.
     */
    const char*   ipadr( ) const;
    unsigned long inaddr( ) const;

    friend std::ostream& operator<<( std::ostream& ostr, const AddressIP& adr );
	
    friend bool operator==( const AddressIP& l, const AddressIP& r );
    friend bool operator!=( const AddressIP& l, const AddressIP& r );
    friend bool operator<( const AddressIP& l, const AddressIP& r );
};

std::ostream& operator<<( std::ostream& ostr, const AddressIP& adr );

bool operator==( const AddressIP& l, const AddressIP& r );
bool operator!=( const AddressIP& l, const AddressIP& r );
bool operator<( const AddressIP& l, const AddressIP& r );
#endif /* VSNET_ADDRESS_H */
