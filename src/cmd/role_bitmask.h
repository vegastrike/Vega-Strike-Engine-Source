#include <string>
#include <vector>
namespace ROLES {
	int discreteLog (int bitmask);
	std::vector <char>& getPriority(unsigned char rolerow);
	//get the list of all prirotieis 31 is nogo
	std::vector < std::vector <char > > &getAllRolePriorities ();
	unsigned int maxRoleValue();
	//getRole returns a number between 0 and 32 that's the discrete log
	unsigned char getRole (const std::string &role);
	//returns the getRoles or'd together
	unsigned int readBitmask (const std::string &s);
}
