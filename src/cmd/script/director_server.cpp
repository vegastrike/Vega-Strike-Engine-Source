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
  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#include <pwd.h>
#endif

#ifdef HAVE_PYTHON
#include "Python.h"
#endif
#include "python/python_class.h"
#ifndef USE_BOOST_128
#include <boost/python/class.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif

#include "mission.h"

void InitBase() {
	Python::reseterrors();
	//PYTHON_INIT_MODULE(Base);
	Python::reseterrors();
}
void InitBriefing() {
	Python::reseterrors();
	//PYTHON_INIT_MODULE(Briefing);
	Python::reseterrors();
}

void Mission::DirectorLoop(){}
void Mission::BriefingUpdate(){}
void Mission::DirectorBenchmark(){}
varInst * Mission::call_briefing (missionNode * node, int mode) { return NULL;}
