#include "weapon_type.h"

WEAPON_TYPE getWeaponTypeFromString(std::string type) {
    if(type == "Bolt") {
        return WEAPON_TYPE::BOLT;
    }

    if(type == "Ball") {
        return WEAPON_TYPE::BALL;
    }

    if(type == "Beam") {
        return WEAPON_TYPE::BEAM;
    }

    if(type == "Missile") {
        return WEAPON_TYPE::PROJECTILE;
    }

    return WEAPON_TYPE::UNKNOWN;
}

std::string getFilenameFromWeaponType(WEAPON_TYPE type) {
    switch (type)
    {
    case WEAPON_TYPE::BEAM:         return "beamtexture.bmp";
    case WEAPON_TYPE::BALL:         return "ball.ani";
    case WEAPON_TYPE::PROJECTILE:   return "missile.bfxm";

    // Note these two types fall through to default.
    case WEAPON_TYPE::UNKNOWN:
    case WEAPON_TYPE::BOLT:
    default:                        return "";
    }
}
