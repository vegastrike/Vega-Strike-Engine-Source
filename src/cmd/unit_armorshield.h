#ifndef __UNIT_ARMOR_SHIELD_H
#define __UNIT_ARMOR_SHIELD_H

  ///Armor values: how much damage armor can withhold before internal damage accrues
  struct Armor {
    float frontlefttop, backlefttop, frontrighttop, backrighttop, frontleftbottom, backleftbottom, frontrightbottom, backrightbottom;

    Armor( ) : frontlefttop(0), backlefttop(0), frontrighttop(0), backrighttop(0), frontleftbottom(0), backleftbottom(0), frontrightbottom(0), backrightbottom(0) { }
  };

  ///Shielding Struct holding values of current shields
  struct Shield {
    ///How much the shield recharges per second
    float recharge;
    
	//A union containing the different shield values and max values depending on number
    union {
      ///if shield is 2 big, 2 floats make this shield up, and 2 floats for max {front,back,frontmax,backmax}
		struct {
			float front, back;
			float frontmax, backmax;
		}shield2fb;

      ///If the shield if 4 big, 4 floats make the shield up, and 4 keep track of max recharge value
      struct {
		float front, back, right, left;
		float frontmax, backmax, rightmax, leftmax;
      }shield4fbrl;
      ///If the shield is 8 sided, 16 floats make it up
      struct {
		float frontlefttop, backlefttop, frontrighttop, backrighttop, frontleftbottom, backleftbottom, frontrightbottom, backrightbottom;
		float frontlefttopmax, backlefttopmax, frontrighttopmax, backrighttopmax, frontleftbottommax, backleftbottommax, frontrightbottommax, backrightbottommax;
      }shield8;

    };
	
    ///the number of shields in the current shielding struct
    signed char number;
    ///What percentage leaks (divide by 100%)
    char leak; 

    Shield( ) : recharge(0), number(0), leak(0) { }
  };

#endif
