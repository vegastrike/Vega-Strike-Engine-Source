#include "mount_size.h"

#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>

// TODO: introduce using enum MOUNT_SIZE; when we migrate to C++20 (P1099R5)


static const std::map<std::string, MOUNT_SIZE> mount_map = {
    {"LIGHT",               MOUNT_SIZE::LIGHT},
    {"MEDIUM",              MOUNT_SIZE::MEDIUM},
    {"HEAVY",               MOUNT_SIZE::HEAVY},
    {"CAPSHIPLIGHT",        MOUNT_SIZE::CAPSHIPLIGHT},
    {"CAPSHIPHEAVY",        MOUNT_SIZE::CAPSHIPHEAVY},
    {"SPECIAL",             MOUNT_SIZE::SPECIAL},
    {"LIGHTMISSILE",        MOUNT_SIZE::LIGHTMISSILE},
    {"MEDIUMMISSILE",       MOUNT_SIZE::MEDIUMMISSILE},
    {"HEAVYMISSILE",        MOUNT_SIZE::HEAVYMISSILE},
    {"CAPSHIPLIGHTMISSILE", MOUNT_SIZE::CAPSHIPLIGHTMISSILE},
    {"CAPSHIPHEAVYMISSILE", MOUNT_SIZE::CAPSHIPHEAVYMISSILE},
    {"SPECIALMISSILE",      MOUNT_SIZE::SPECIALMISSILE},
    {"AUTOTRACKING",        MOUNT_SIZE::AUTOTRACKING}
};


MountSize::MountSize()
{

}

MOUNT_SIZE getMountSizeFromItsValue(int value) {
    MOUNT_SIZE mount_sizes[] = {MOUNT_SIZE::NOWEAP,
                                MOUNT_SIZE::LIGHT, MOUNT_SIZE::MEDIUM, MOUNT_SIZE::HEAVY,
                                MOUNT_SIZE::CAPSHIPLIGHT, MOUNT_SIZE::CAPSHIPHEAVY, MOUNT_SIZE::SPECIAL,
                                MOUNT_SIZE::LIGHTMISSILE, MOUNT_SIZE::MEDIUMMISSILE, MOUNT_SIZE::HEAVYMISSILE,
                                MOUNT_SIZE::CAPSHIPLIGHTMISSILE, MOUNT_SIZE::CAPSHIPHEAVYMISSILE, MOUNT_SIZE::SPECIALMISSILE,
                                MOUNT_SIZE::AUTOTRACKING};

    for (auto&& mount_size : mount_sizes) {
        if(value == as_integer(mount_size)) {
            return mount_size;
        }
    }

    return MOUNT_SIZE::NOWEAP;
}

MOUNT_SIZE getMountSize(const std::string& mount_string) {
    // TODO: just use map here?! but then need to do something about case sensitivity
    for (const auto& [key, value]: mount_map) {
        if(boost::iequals(mount_string, key)) {
            return value;
        }
    }

    return MOUNT_SIZE::NOWEAP;
}

int getMountSizes(const std::string& mounts_string) {
    std::vector<std::string> mounts_vector;
    boost::split(mounts_vector, mounts_string, boost::is_any_of(" "));
    int mountSize = 0;

    for (const auto& mount : mounts_vector) {
        mountSize += as_integer(getMountSize(mount));
    }

    return mountSize;
}

std::string getMountSizeString(const int mount){
    std::string mount_string;

    for (const auto& [key, value]: mount_map) {
        if(as_integer(value) & mount) {
            mount_string += key + " ";
        }
    }

    // Snip trailing space
    boost::trim_right(mount_string);
    return mount_string;
}

bool isNormalGunMount(const int mount) {
    return ( mount & (
                 as_integer(MOUNT_SIZE::LIGHT)|
                 as_integer(MOUNT_SIZE::MEDIUM)|
                 as_integer(MOUNT_SIZE::HEAVY)|
                 as_integer(MOUNT_SIZE::CAPSHIPLIGHT)|
                 as_integer(MOUNT_SIZE::CAPSHIPHEAVY) )) != 0;
}

bool isNormalMissileMount(const int mount) {
    return ( mount & (
                 as_integer(MOUNT_SIZE::LIGHTMISSILE)|
                 as_integer(MOUNT_SIZE::MEDIUMMISSILE)|
                 as_integer(MOUNT_SIZE::HEAVYMISSILE)|
                 as_integer(MOUNT_SIZE::CAPSHIPLIGHTMISSILE)|
                 as_integer(MOUNT_SIZE::CAPSHIPHEAVYMISSILE) )) != 0;
}

bool isNormalMount(const int mount) {
    return isNormalGunMount(mount) || isNormalMissileMount(mount);
}

bool isSpecialMount(const int mount) {
    return mount & (as_integer(MOUNT_SIZE::SPECIAL) ||
                    as_integer(MOUNT_SIZE::SPECIALMISSILE));
}

bool isSpecialGunMount(const int mount) {
    return mount & as_integer(MOUNT_SIZE::SPECIAL);
}

bool isSpecialMissileMount(const int mount) {
    return mount & as_integer(MOUNT_SIZE::SPECIALMISSILE);
}

bool isAutoTrackingMount(const int mount) {
    return mount & as_integer(MOUNT_SIZE::AUTOTRACKING);
}

bool isGunMount(const int mount) {
    return ( mount & (
                 as_integer(MOUNT_SIZE::LIGHT)|
                 as_integer(MOUNT_SIZE::MEDIUM)|
                 as_integer(MOUNT_SIZE::HEAVY)|
                 as_integer(MOUNT_SIZE::CAPSHIPLIGHT)|
                 as_integer(MOUNT_SIZE::CAPSHIPHEAVY)|
                 as_integer(MOUNT_SIZE::SPECIAL))) != 0;
}

bool isMissileMount(const int mount) {
    return ( mount & (
                 as_integer(MOUNT_SIZE::LIGHTMISSILE)|
                 as_integer(MOUNT_SIZE::MEDIUMMISSILE)|
                 as_integer(MOUNT_SIZE::HEAVYMISSILE)|
                 as_integer(MOUNT_SIZE::CAPSHIPLIGHTMISSILE)|
                 as_integer(MOUNT_SIZE::CAPSHIPHEAVYMISSILE)|
                 as_integer(MOUNT_SIZE::SPECIALMISSILE))) != 0;
}

