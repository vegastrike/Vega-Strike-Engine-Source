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

#ifndef VSNET_CMD_H
#define VSNET_CMD_H

#include <iostream>

/// Commands that are sent through the network
enum Cmd
{
    CMD_LOGIN        = 0x01,
    CMD_LOGOUT       = 0xA1,
    CMD_INITIATE     = 0x02,
    CMD_POSUPDATE    = 0x03,
    CMD_NEWCHAR      = 0x04,
    CMD_LETSGO       = 0x05,
    CMD_UPDATECLT    = 0x06,
    CMD_ADDCLIENT    = 0x07,
    CMD_FULLUPDATE   = 0x08,
    CMD_PING         = 0x09,
    CMD_SNAPSHOT     = 0xA0,

    CMD_CREATECHAR   = 0x50,
    CMD_LOCATIONS    = 0x51,
    LOGIN_ERROR      = 0xF1,
    LOGIN_ACCEPT     = 0xF0,
    LOGIN_ALREADY    = 0xF6,
    LOGIN_NEW        = 0xF7,

    CMD_NEWSUBSCRIBE =  0xF8,

    CMD_ENTERCLIENT  = 0xF2,
    CMD_EXITCLIENT   = 0xF3,
    CMD_ADDEDYOU     = 0xF5,
    CMD_DISCONNECT   = 0xF4,

    CMD_ACK          = 0xFF
};

std::ostream& operator<<( std::ostream& ostr, Cmd cmd );

const char* displayCmd( Cmd cmd );

#endif

