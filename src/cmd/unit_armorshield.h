#ifndef __UNIT_ARMOR_SHIELD_H
#define __UNIT_ARMOR_SHIELD_H

  ///Armor values: how much damage armor can withhold before internal damage accrues
  struct Armor {
    unsigned short front, back, right, left;

    Armor( ) : front(0), back(0), right(0), left(0) { }
  };

  ///Shielding Struct holding values of current shields
  struct Shield {
    ///How much the shield recharges per second
    float recharge;
    ///A union containing the different shield values and max values depending on number
    union {
      ///if shield is 2 big, 2 floats make this shield up, and 2 floats for max {front,back,frontmax,backmax}
      float fb[4];
      ///If the shield if 4 big, 4 floats make the shield up, and 4 keep track of max recharge value
      struct {
	unsigned short front, back, right, left;
	unsigned short frontmax, backmax, rightmax, leftmax;
      }fbrl;
      ///If the shield is 6 sided, 6 floats make it up, 2 indicating the max value of various sides, and 6 being the actual vals
      struct {
	unsigned short v[6];
	unsigned short fbmax,rltbmax;
      }fbrltb;

    };
    ///the number of shields in the current shielding struct
    signed char number;
    ///What percentage leaks (divide by 100%)
    char leak; 

    Shield( ) : recharge(0), number(0), leak(0) { }
  };

#endif
