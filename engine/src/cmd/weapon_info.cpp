#include "weapon_info.h"

#include "vs_globals.h"
#include "options.h"
#include "unit_const_cache.h"

// TODO: this should not be here
#include "vsfilesystem.h"
#include "gfx/mesh.h"

#include <boost/algorithm/string.hpp>

namespace alg = boost::algorithm;

Hashtable< std::string, weapon_info, 257 >lookuptable;

weapon_info::weapon_info() {}

weapon_info::weapon_info( WEAPON_TYPE type )
{
    this->type = type;
}

weapon_info::weapon_info( const weapon_info &tmp )
{
    *this = tmp;
}

float weapon_info::Refire() const
{
    unsigned int len = name.length();
    // TODO: what is this???
    if (g_game.difficulty > .98 || len < 9 || name[len-8] != 'C' || name[len-9] != '_' || name[len-7]
        != 'o' || name[len-6] != 'm' || name[len-5] != 'p' || name[len-4] != 'u' || name[len-3]
        != 't' || name[len-2] != 'e' || name[len-1] != 'r')
        return refire_rate;
    return this->refire_rate*( game_options.refire_difficulty_scaling/(1.0f+(game_options.refire_difficulty_scaling-1.0f)*g_game.difficulty) );
}

bool weapon_info::isMissile() const
{
    if (game_options.projectile_means_missile  && this->type == WEAPON_TYPE::PROJECTILE)
        return true;
    if (game_options.projectile_means_missile == false && this->size >= MOUNT_SIZE::LIGHTMISSILE)
        return true;
    return false;
}

// TODO: this should not be here
using namespace VSFileSystem;

weapon_info * getWeapon( const std::string &key )
{
    weapon_info *wi = lookuptable.Get( boost::to_upper_copy( key ) );
    if (wi) {
        if ( !WeaponMeshCache::getCachedMutable( wi->name ) ) {
            static FileLookupCache lookup_cache;
            string meshname = boost::to_lower_copy( key )+".bfxm";
            if (CachedFileLookup( lookup_cache, meshname, MeshFile ) <= Ok) {
                WeaponMeshCache::setCachedMutable( wi->name, wi->gun =
                                                      Mesh::LoadMesh( meshname.c_str(), Vector( 1, 1, 1 ), 0, NULL ) );
                WeaponMeshCache::setCachedMutable( wi->name+"_flare", wi->gun1 =
                                                      Mesh::LoadMesh( meshname.c_str(), Vector( 1, 1, 1 ), 0, NULL ) );
            }
        }
    }
    return wi;
}


void weapon_info::netswap()
{
    //Enum elements are the size of an int
    //byte order swap doesn't work with ENUM - MAY NEED TO FIND A WORKAROUND SOMEDAY
    //type = VSSwapHostIntToLittle( type);
    //size = VSSwapHostIntToLittle( size);

    damage          = VSSwapHostFloatToLittle( damage );
    energy_rate     = VSSwapHostFloatToLittle( energy_rate );
    length          = VSSwapHostFloatToLittle( length );
    lock_time       = VSSwapHostFloatToLittle( lock_time );
    long_range      = VSSwapHostFloatToLittle( long_range );
    offset.netswap();
    phase_damage    = VSSwapHostFloatToLittle( phase_damage );
    pulse_speed     = VSSwapHostFloatToLittle( pulse_speed );
    radial_speed    = VSSwapHostFloatToLittle( radial_speed );
    radius          = VSSwapHostFloatToLittle( radius );
    range           = VSSwapHostFloatToLittle( range );
    refire_rate     = VSSwapHostFloatToLittle( refire_rate );
    //role_bits       = VSSwapHostIntToLittle( role_bits );
    stability       = VSSwapHostFloatToLittle( stability );
    sound           = VSSwapHostIntToLittle( sound );
    speed           = VSSwapHostFloatToLittle( speed );
    texture_stretch = VSSwapHostFloatToLittle( texture_stretch );
    volume          = VSSwapHostFloatToLittle( volume );

    r               = VSSwapHostFloatToLittle( r );
    g               = VSSwapHostFloatToLittle( g );
    b               = VSSwapHostFloatToLittle( b );
    a               = VSSwapHostFloatToLittle( a );
}
