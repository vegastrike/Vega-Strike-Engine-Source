/*
 * Copyright (C) 2002 Carsten Griwodz
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
#ifndef _UNIT_FACTORY_GENERIC_H_
#define _UNIT_FACTORY_GENERIC_H_
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string>
using std::string;

class Unit;
class Flightgroup;

class UnitGenericFactory
{
public:
    static Unit* createGenericUnit( const char *filename,
                             bool        SubUnit,
                             int         faction,
                             string customizedUnit=string(""),
                             Flightgroup *flightgroup=NULL,
                             int         fg_subnumber=0 );
    static Unit* getMasterPartList( );
protected:
    static Unit* _masterPartList;
};

#endif /* _UNIT_FACTORY_H_ */

