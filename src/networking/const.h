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
  Constants - Network Interface Constants - written by Stephane Vaxelaire <svax@free.fr>
*/

#ifndef __CONST_H
#define __CONST_H

#define SERVERCONFIGFILE	"vegaserver.config"
#define ACCTCONFIGFILE		"accountserver.config"

#define MAXCLIENTS	512
#define SERVER_PORT 6777
#define CLIENT_PORT	6778
#define ACCT_PORT	6779
#define NAMELEN		32
#define MAXBUFFER	2048

// Commands that are sent through the network
#define CMD_LOGIN		0x01
#define CMD_LOGOUT		0xA1
#define CMD_INITIATE	0x02
#define CMD_POSUPDATE	0x03
#define CMD_NEWCHAR		0x04
#define CMD_LETSGO		0x05
#define CMD_UPDATECLT	0x06
#define CMD_ADDCLIENT	0x07
#define CMD_FULLUPDATE	0x08
#define CMD_PING		0x09
#define CMD_SNAPSHOT	0xA0

#define CMD_CREATECHAR	0x50
#define CMD_LOCATIONS	0x51
#define LOGIN_ERROR		0xF1
#define LOGIN_ACCEPT	0xF0
#define LOGIN_ALREADY	0xF6
#define LOGIN_NEW		0xF7

#define CMD_NEWSUBSCRIBE 0xF8

#define CMD_ENTERCLIENT	0xF2
#define CMD_EXITCLIENT	0xF3
#define CMD_ADDEDYOU	0xF5
#define CMD_DISCONNECT	0xF4

typedef unsigned short ObjSerial;
#define MAXSERIAL 0xFFFF
#define TONET() htons()
typedef unsigned int InstSerial;

extern double NETWORK_ATOM;

#endif
