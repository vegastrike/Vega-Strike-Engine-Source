#ifndef _ROLE_BITMASK_H_
#define _ROLE_BITMASK_H_
#include <string>
#include <vector>
namespace ROLES {
        const unsigned int FIRE_ONLY_AUTOTRACKERS=(1<<31);
        const unsigned int FIRE_MISSILES=(1<<30);
	const unsigned int FIRE_GUNS=(1<<29);
        const unsigned int EVERYTHING_ELSE=(~(FIRE_MISSILES|FIRE_GUNS|FIRE_ONLY_AUTOTRACKERS));
	int discreteLog (int bitmask);
	std::vector <char>& getPriority(unsigned char rolerow);
	//get the list of all prirotieis 31 is nogo
	std::vector < std::vector <char > > &getAllRolePriorities ();
	unsigned int maxRoleValue();
	//getRole returns a number between 0 and 32 that's the discrete log
	unsigned char getRole (const std::string &role);
	//returns the getRoles or'd together
	unsigned int readBitmask (const std::string &s);
	const std::string &getRoleEvents (unsigned char ourrole, unsigned char theirs);//get AI ervent associated with this
}
#endif
