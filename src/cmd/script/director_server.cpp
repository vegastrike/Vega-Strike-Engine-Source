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
#ifdef HAVE_PYTHON
#include <Python.h>
#endif
#include "python/python_class.h"
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/class.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#include <pwd.h>
#endif


#include "python/python_class.h"

#include "cmd/unit_generic.h"
#include "mission.h"

PYTHON_BEGIN_MODULE(Base)
// Nothing here, but keeps those files that do a "import Base" happy.
PYTHON_END_MODULE(Base)

PYTHON_BEGIN_MODULE(Briefing)
// Nothing here, but keeps those files that do a "import Briefing" happy.
PYTHON_END_MODULE(Briefing)

void InitBase() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Base);
	Python::reseterrors();
}
void InitBriefing() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Briefing);
	Python::reseterrors();
}

void Mission::DirectorLoop(){
   double oldgametime=gametime;
   gametime+=SIMULATION_ATOM;//elapsed;

   try {
      if (runtime.pymissions)
         runtime.pymissions->Execute();
   }catch (...) {
      if (PyErr_Occurred()) {
         PyErr_Print();
         PyErr_Clear();
         fflush(stderr);         
         fflush(stdout);
      }
      throw;
   }
}
void Mission::BriefingUpdate(){}
void Mission::DirectorBenchmark(){
}
varInst * Mission::call_briefing (missionNode * node, int mode) { return NULL;}
