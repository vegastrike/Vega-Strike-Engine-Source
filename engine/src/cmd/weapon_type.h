#ifndef WEAPONTYPE_H
#define WEAPONTYPE_H

#include <string>

enum class WEAPON_TYPE
{
    UNKNOWN,
    BEAM,
    BALL,
    BOLT,
    PROJECTILE
};

WEAPON_TYPE getWeaponTypeFromString(std::string type);
std::string getFilenameFromWeaponType(WEAPON_TYPE type);

#endif // WEAPONTYPE_H
