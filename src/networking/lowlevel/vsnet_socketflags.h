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

#ifndef VSNET_SOCKETFLAGS_H
#define VSNET_SOCKETFLAGS_H

#include <config.h>
#include <iostream>

enum PCKTFLAGS
{
    NONE          = 0,
    STDPRI        = 0,      // normal priority
    LOPRI         = 0x0010, // lower than normal
    HIPRI         = 0x0020, // higher than normal
    SENDRELIABLE  = 0,      // retransmit if lost
    SENDANDFORGET = 0x0100, // do not retransmit if lost
    SENT          = 0x0001,
    RESENT        = 0x0002,
    ACKED         = 0x0004,
    COMPRESSED    = 0x1000,
    FRAGMENT      = 0x2000, // low pri packet that is split up
    LASTFRAGMENT  = 0x4000, // low pri packet that is split up
};

std::ostream& operator<<( std::ostream& ostr, PCKTFLAGS flags );

#endif /* VSNET_SOCKETFLAGS_H */

