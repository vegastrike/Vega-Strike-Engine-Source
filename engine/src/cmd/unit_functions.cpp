/**
* unit_functions.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "unit.h"
#include "gfx/vec.h"
#include "gfx/animation.h"
#include "gfx/cockpit_generic.h"
#include "faction_generic.h"
#include "savegame.h"
#include "xml_support.h"
#include "gfx/halo.h"
#include "gfx/mesh.h"
#include "gfx/sphere.h"
#include "gfx/sprite.h"
#include "audiolib.h"
#include "collide2/CSopcodecollider.h"
#include "unit_collide.h"
#include <string>
#include <set>

//Various functions that were used in .cpp files that are now included because of
//the temple GameUnit class
//If not separated from those files functions would be defined in multiple places
//Those functions are client specific

//Wrappers used in unit_xml.cpp
VSSprite * createVSSprite( const char *file )
{
    return new VSSprite( file );
}
bool isVSSpriteLoaded( const VSSprite *sprite )
{
    return sprite->LoadSuccess();
}
void deleteVSSprite( VSSprite *sprite )
{
    delete sprite;
}
//From communication_xml.cpp
int createSound( string file, bool val )
{
    return AUDCreateSoundWAV( file, val );
}

//From unit_xml.cpp
using vsUMap;
static vsUMap< std::string, Animation* >cached_ani;
std::set< std::string >tempcache;
void cache_ani( string s )
{
    tempcache.insert( s );
}
void update_ani_cache()
{
    for (std::set< std::string >::iterator it = tempcache.begin(); it != tempcache.end(); it++)
        if ( cached_ani.find( *it ) == cached_ani.end() )
            cached_ani.insert( std::pair< std::string, Animation* > ( *it,
                                                                     new Animation( (*it).c_str(), false, .1, BILINEAR, false ) ) );
    tempcache.clear();
}
std::string getRandomCachedAniString()
{
    if ( cached_ani.size() ) {
        unsigned int rn = rand()%cached_ani.size();
        vsUMap< std::string, Animation* >::iterator j = cached_ani.begin();
        for (unsigned int i = 0; i < rn; i++)
            j++;
        return (*j).first;
    } else {
        return "";
    }
}
Animation * getRandomCachedAni()
{
    if ( cached_ani.size() ) {
        unsigned int rn = rand()%cached_ani.size();
        vsUMap< std::string, Animation* >::iterator j = cached_ani.begin();
        for (unsigned int i = 0; i < rn; i++)
            j++;
        return (*j).second;
    } else {
        return NULL;
    }
}

