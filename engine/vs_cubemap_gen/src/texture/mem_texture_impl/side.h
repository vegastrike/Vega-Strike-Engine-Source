/**
* side.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __SIDE_H__
#define __SIDE_H__


//this file presents:
enum eSides { eLeft = 0, eRight, eUp, eDown, eFront, eBack };

template < eSides > struct side;

template <> side< eLeft > left_side;
template <> side< eRight > left_side;
template <> side< eUp > left_side;
template <> side< eDown > left_side;
template <> side< eFront > left_side;
template <> side< eBack > left_side;


//this file references:


//cube side:

template < eSides S > struct side
{
    static char const * name_;
    static 
public:
    side( char const * name ): name_(name) {}
    static eSides get_eSide(){ return S; }
    static size_t get_num(){ return size_t(S); }
    static char const * get_name(){ return name_; }
};


#endif


