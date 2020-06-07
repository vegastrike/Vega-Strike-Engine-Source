/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
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
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#include <boost/version.hpp>
#include <boost/python/class.hpp>
#include "python/python_class.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#include <pwd.h>
#endif

#include <expat.h>

#include <fstream>

#include "xml_support.h"

#include "vegastrike.h"
#include "vsfilesystem.h"
#include "lin_time.h"
#include "cmd/unit_generic.h"

#include "cmd/ai/order.h"
#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"
#include "savegame.h"
#include "msgcenter.h"
#include "cmd/briefing.h"
#include "pythonmission.h"
#ifdef HAVE_PYTHON
#include "cs_python.h"
#endif
#include "flightgroup.h"
#include "gldrv/winsys.h"
#include "gfx/cockpit_generic.h"

/* *********************************************************** */
//ADD_FROM_PYTHON_FUNCTION(pythonMission)
void Mission::DirectorLoop()
{
    double oldgametime = gametime;
    gametime += SIMULATION_ATOM;     //elapsed;
    if (getTimeCompression() >= .1)
        if (gametime <= oldgametime)
            gametime = SIMULATION_ATOM;
    try {
        BriefingLoop();
        if (runtime.pymissions)
            runtime.pymissions->Execute();
    }
    catch (...) {
        if ( PyErr_Occurred() ) {
            PyErr_Print();
            PyErr_Clear();
            fflush( stderr );
            fflush( stdout );
        } throw;
    }
}
void Mission::DirectorEnd()
{
    if (director == NULL)
        return;
    RunDirectorScript( "endgame" );
}

void Mission::DirectorShipDestroyed( Unit *unit )
{
    Flightgroup *fg = unit->getFlightgroup();
    if (fg == NULL) {
        printf( "ship destroyed-no flightgroup\n" );
        return;
    }
    if (fg->nr_ships_left <= 0 && fg->nr_waves_left > 0) {
        printf( "WARNING: nr_ships_left<=0\n" );
        return;
    }
    fg->nr_ships_left -= 1;

    char buf[512];

    if ( (fg->faction.length()+fg->type.length()+fg->name.length()+12+30) < sizeof (buf) )
        sprintf( buf, "Ship destroyed: %s:%s:%s-%d", fg->faction.c_str(), fg->type.c_str(),
                fg->name.c_str(), unit->getFgSubnumber() );
    else
        sprintf( buf, "Ship destroyed: (ERROR)-%d", unit->getFgSubnumber() );

    msgcenter->add( "game", "all", buf );

    if (fg->nr_ships_left == 0) {
        BOOST_LOG_TRIVIAL(debug) << boost::format("no ships left in fg %1%") % fg->name;
        if (fg->nr_waves_left > 0) {
            BOOST_LOG_TRIVIAL(info) << boost::format("Relaunching %1% wave") % fg->name;

            //launch new wave
            fg->nr_waves_left -= 1;
            fg->nr_ships_left  = fg->nr_ships;

            Order *order = NULL;
            order = unit->getAIState() ? unit->getAIState()->findOrderList() : NULL;
            fg->orderlist = NULL;
            if (order) {
                fg->orderlist = order->getOrderList();
            }
            CreateFlightgroup cf;
            cf.fg       = fg;
            cf.unittype = CreateFlightgroup::UNIT;
            cf.fg->pos  = unit->Position();
            cf.waves    = fg->nr_waves_left;
            cf.nr_ships = fg->nr_ships;

            call_unit_launch( &cf, UNITPTR, string( "" ) );
        } else {
            mission->msgcenter->add( "game", "all", "Flightgroup "+fg->name+" destroyed" );
        }
    }
}

bool Mission::BriefingInProgress()
{
    return briefing != NULL;
}
void Mission::BriefingStart()
{
    if (briefing)
        BriefingEnd();
    briefing = new Briefing();
    if (runtime.pymissions)
        runtime.pymissions->callFunction( "initbriefing" );
}
void Mission::BriefingUpdate()
{
    if (briefing)
        briefing->Update();
}

void Mission::BriefingLoop()
{
    if (briefing)
        if (runtime.pymissions)
            runtime.pymissions->callFunction( "loopbriefing" );
}
class TextPlane* Mission::BriefingRender()
{
    if (briefing) {
        vector< std::string >who;
        who.push_back( "briefing" );
        string str1;
        gameMessage g1, g2;
        if ( msgcenter->last( 0, g1, who ) )
            str1 = g1.message;
        if ( msgcenter->last( 1, g2, who ) )
            str1 = str1+string( "\n" )+g2.message;
        briefing->tp.SetText( str1 );
        briefing->Render();
        return &briefing->tp;
    }
    return NULL;
}

void Mission::BriefingEnd()
{
    if (briefing) {
        if (runtime.pymissions)
            runtime.pymissions->callFunction( "endbriefing" );
        delete briefing;
        briefing = NULL;
    }
}

void Mission::DirectorBenchmark()
{
    total_nr_frames++;
    if (benchmark > 0.0 && benchmark < gametime) {
        std::cout<<"Game was running for "<<gametime<<" secs,   av. framerate "<<( (double) total_nr_frames )/gametime
                 <<std::endl;
        winsys_exit( 0 );
    }
}
