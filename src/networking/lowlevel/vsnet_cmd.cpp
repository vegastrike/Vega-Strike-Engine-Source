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

#include "vsnet_cmd.h"

std::ostream& operator<<( std::ostream& ostr, Cmd cmd )
{
    ostr << displayCmd( cmd );
    return ostr;
};

#define CASE(a) case a: return #a; break;

const char* displayCmd( Cmd cmd )
{
    switch( cmd )
    {
    CASE( CMD_LOGIN        )
    CASE( CMD_LOGOUT       )
    CASE( CMD_POSUPDATE    )
    CASE( CMD_ADDCLIENT    )
    CASE( CMD_PING         )
    CASE( CMD_SNAPSHOT     )
    CASE( CMD_SERVERTIME   )
    
    CASE( CMD_CREATECHAR   )
    CASE( CMD_LOCATIONS    )
    CASE( LOGIN_ERROR      )
    CASE( LOGIN_ACCEPT     )
    CASE( LOGIN_DATA       )
    CASE( LOGIN_ALREADY    )
    CASE( LOGIN_NEW        )
    CASE( CMD_RESPAWN      )
    CASE( CMD_RESYNCACCOUNTS)
    CASE( CMD_SAVEACCOUNTS )

    CASE( CMD_SNAPCARGO    )
    CASE( CMD_CARGOUPGRADE )
    CASE( CMD_CREDITS      )
    CASE( CMD_FIREREQUEST  )
    CASE( CMD_UNFIREREQUEST)
    CASE( CMD_TARGET       )
    CASE( CMD_SCAN         )
    CASE( CMD_DAMAGE       )
    CASE( CMD_SNAPDAMAGE   )
    CASE( CMD_KILL         )
    CASE( CMD_JUMP         )
    CASE( CMD_ASKFILE      )
    CASE( CMD_DOWNLOAD     )
    CASE( CMD_CAMSHOT      )
    CASE( CMD_SOUNDSAMPLE  )
    CASE( CMD_TXTMESSAGE   )
    CASE( CMD_SECSNDSAMPLE )
    CASE( CMD_SECMESSAGE   )
    CASE( CMD_STARTNETCOMM )
    CASE( CMD_STOPNETCOMM  )
    CASE( CMD_DOCK         )
    CASE( CMD_UNDOCK       )

    CASE( CMD_NEWSUBSCRIBE )

    CASE( CMD_ENTERCLIENT  )
    CASE( CMD_EXITCLIENT   )
    CASE( CMD_ADDEDYOU     )
    CASE( CMD_DISCONNECT   )
    CASE( CMD_CONNECT      )
    CASE( CMD_CHOOSESHIP   )

	CASE( CMD_SAVEDATA     )
	CASE( CMD_MISSION      )
    // CASE( CMD_ACK          )
    default :
        return "CMD_UNKNOWN"; break;
    }
}

#undef CASE

