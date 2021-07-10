#ifndef __UNIT_ARMOR_SHIELD_H
#define __UNIT_ARMOR_SHIELD_H

#include <algorithm>

///Armor values: how much damage armor can withhold before internal damage accrues
struct Armor
{
    float frontlefttop = 0;
    float backlefttop = 0;
    float frontrighttop = 0;
    float backrighttop = 0;
    float frontleftbottom = 0;
    float backleftbottom = 0;
    float frontrightbottom = 0;
    float backrightbottom = 0;
};
#define MAX_SHIELD_NUMBER 8
struct ShieldFacing
{
    float thetamin = 0;
    float thetamax = 0;
    float rhomin = 0;
    float rhomax = 0;
};
///Shielding Struct holding values of current shields
struct Shield
{
    ///How much the shield recharges per second
    float recharge = 0;
    float efficiency = 1;
    //A union containing the different shield values and max values depending on number
    union
    {
        ///if shield is 2 big, 2 floats make this shield up, and 2 floats for max {front,back,frontmax,backmax}
        struct
        {
            float front, back;
            float padding[6];
            float frontmax, backmax;
        }
        shield2fb;
        ///If the shield if 4 big, 4 floats make the shield up, and 4 keep track of max recharge value
        struct
        {
            float front = 0;
            float back = 0;
            float right = 0;
            float left = 0;
            float padding[4];
            float frontmax, backmax, rightmax, leftmax;
        }
        shield4fbrl;
        ///If the shield is 8 sided, 16 floats make it up
        struct
        {
            float frontrighttop = 0;
            float backrighttop = 0;
            float frontlefttop = 0;
            float backlefttop = 0;
            float frontrightbottom = 0;
            float backrightbottom = 0;
            float frontleftbottom = 0;
            float backleftbottom = 0;
            float frontrighttopmax = 0;
            float backrighttopmax = 0;
            float frontlefttopmax = 0;
            float backlefttopmax = 0;
            float frontrightbottommax = 0;
            float backrightbottommax = 0;
            float frontleftbottommax = 0;
            float backleftbottommax = 0;
        }
        shield8;
        struct
        {
            float cur[MAX_SHIELD_NUMBER];
            float max[MAX_SHIELD_NUMBER];
        }
        shield;
    };
    ShieldFacing range[MAX_SHIELD_NUMBER];
    ///the number of shields in the current shielding struct
    signed char  number = 0;
    ///What percentage leaks (divide by 100%)
    char  leak = 0;

    Shield()
    {
        std::fill_n(shield.cur, sizeof(shield.cur)/sizeof(shield.cur[0]), 0.0f);
        std::fill_n(shield.max, sizeof(shield.max)/sizeof(shield.max[0]), 0.0f);
        std::fill_n(range, sizeof(range)/sizeof(range[0]), ShieldFacing());
    }
};

#endif

