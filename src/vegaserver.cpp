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
  Vegastrike Network Server Main - written by Stephane Vaxelaire <svax@free.fr>
*/

#include <iostream>
#include <stdlib.h>
#include "vs_globals.h"
#include "networking/netserver.h"
#include "cmd/script/mission.h"
#include "force_feedback.h"

char SERVER = 1;
Universe * _Universe;
LeakVector<Mission *> active_missions;

int main( int argc, char **argv)
{
	Server = new NetServer;
	// Fake forcefeedback
    forcefeedback=new ForceFeedback();

	Server->start( argc, argv);

	delete Server;
	return 0;
}
