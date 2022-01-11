/**
* weapon_xml.h
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

#ifndef _CMD_WEAPON_XML_H_
#define _CMD_WEAPON_XML_H_
#include <string>
#include "gfx/vec.h"

namespace BeamXML
{
void beginElement( void *userData, const char *name, const char **atts );
}

struct weapon_info
{
    friend void BeamXML::beginElement( void *userData, const char *name, const char **atts );
    enum WEAPON_TYPE
    {
        UNKNOWN,
        BEAM,
        BALL,
        BOLT,
        PROJECTILE
    }
    type;
    enum MOUNT_SIZE
    {
        NOWEAP=0x0, LIGHT=0x1, MEDIUM=0x2, HEAVY=0x4, CAPSHIPLIGHT=0x8, CAPSHIPHEAVY=0x10, SPECIAL=0x20,
        LIGHTMISSILE  =0x40, MEDIUMMISSILE=0x80, HEAVYMISSILE=0x100, CAPSHIPLIGHTMISSILE=0x200, CAPSHIPHEAVYMISSILE=
            0x400,
        SPECIALMISSILE=0x800, AUTOTRACKING=0x1000
    }
    size;
    Vector offset;
    int    role_bits;
    int    sound;
    float  r, g, b, a;
    float  Speed, PulseSpeed, RadialSpeed, Range, Radius, Length;
    float  Damage, PhaseDamage, Stability, Longrange, LockTime;
    float  EnergyRate, volume;
    float  Refire() const;
    bool   isMissile() const;
    float  TextureStretch;
    std::string   file;
    std::string   weapon_name;
    mutable class Mesh *gun;      //requres nonconst to add to orig drawing queue when drawing
    mutable class Mesh *gun1;      //requres nonconst to add to orig drawing queue when drawing
    void init()
    {
        gun            = gun1 = NULL;
        TextureStretch = 1;
        role_bits      = 0;
        offset         = Vector( 0, 0, 0 );
        size           = NOWEAP;
        r = g = b = a = 127;
        Length         = 5;
        Speed          = 10;
        PulseSpeed     = 15;
        RadialSpeed    = 1;
        Range          = 100;
        Radius         = .5;
        Damage         = 1.8;
        PhaseDamage    = 0;
        Stability      = 60;
        Longrange      = .5;
        LockTime       = 0;
        EnergyRate     = 18;
        RefireRate     = .2;
        sound          = -1;
        volume         = 0;
    }
    void Type( enum WEAPON_TYPE typ )
    {
        type = typ;
        switch (typ)
        {
        case BOLT:
            file = std::string( "" );
            break;
        case BEAM:
            file = std::string( "beamtexture.bmp" );
            break;
        case BALL:
            file = std::string( "ball.ani" );
            break;
        case PROJECTILE:
            file = std::string( "missile.bfxm" );
            break;
        default:
            break;
        }
    }
    void MntSize( enum MOUNT_SIZE size )
    {
        this->size = size;
    }
    weapon_info( enum WEAPON_TYPE typ )
    {
        init();
        Type( typ );
    }
    weapon_info( const weapon_info &tmp )
    {
        *this = tmp;
    }
    void netswap();
private:
    float RefireRate;
};
void setWeaponInfoToBuffer( weapon_info wi, char *netbuf, int &bufsize );             //WARNING : ALLOCATES A CHAR * BUFFER SO IT MUST BE DELETED AFTER THAT CALL

enum weapon_info::MOUNT_SIZE lookupMountSize( const char *str );
std::string lookupMountSize( int size );
void LoadWeapons( const char *filename );
weapon_info * getTemplate( const std::string &key );

#endif

