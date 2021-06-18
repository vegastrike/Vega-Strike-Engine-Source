#ifndef MOUNT_SIZE_H
#define MOUNT_SIZE_H

#include <string>

enum class MOUNT_SIZE
{
    NOWEAP              = 0x0,

    LIGHT               = 0x1,
    MEDIUM              = 0x2,
    HEAVY               = 0x4,
    CAPSHIPLIGHT        = 0x8,
    CAPSHIPHEAVY        = 0x10,
    SPECIAL             = 0x20,

    LIGHTMISSILE        = 0x40,
    MEDIUMMISSILE       = 0x80,
    HEAVYMISSILE        = 0x100,
    CAPSHIPLIGHTMISSILE = 0x200,
    CAPSHIPHEAVYMISSILE = 0x400,
    SPECIALMISSILE      = 0x800,

    AUTOTRACKING        = 0x1000
};

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

MOUNT_SIZE getMountSizeFromItsValue(int value);
MOUNT_SIZE getMountSize(const std::string& mount_string);
int getMountSizes(const std::string& mounts_string);
std::string getMountSizeString(const int mount);

bool isNormalGunMount(const int mount);
bool isNormalMissileMount(const int mount);

bool isNormalMount(const int mount);

bool isSpecialMount(const int mount);
bool isSpecialGunMount(const int mount);
bool isSpecialMissileMount(const int mount);

bool isGunMount(const int mount);
bool isMissileMount(const int mount);

bool isAutoTrackingMount(const int mount);




class MountSize
{
public:
    MountSize();
};

#endif // MOUNT_SIZE_H
