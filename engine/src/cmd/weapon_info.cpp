#include "weapon_info.h"

#include "vs_globals.h"
#include "options.h"

float weapon_info::Refire() const
{
    unsigned int len = weapon_name.length();
    if (g_game.difficulty > .98 || len < 9 || weapon_name[len-8] != 'C' || weapon_name[len-9] != '_' || weapon_name[len-7]
        != 'o' || weapon_name[len-6] != 'm' || weapon_name[len-5] != 'p' || weapon_name[len-4] != 'u' || weapon_name[len-3]
        != 't' || weapon_name[len-2] != 'e' || weapon_name[len-1] != 'r')
        return RefireRate;
    return this->RefireRate*( game_options.refire_difficulty_scaling/(1.0f+(game_options.refire_difficulty_scaling-1.0f)*g_game.difficulty) );
}

bool weapon_info::isMissile() const
{
    if (game_options.projectile_means_missile  && this->type == weapon_info::PROJECTILE)
        return true;
    if (game_options.projectile_means_missile == false && this->size >= weapon_info::LIGHTMISSILE)
        return true;
    return false;
}
