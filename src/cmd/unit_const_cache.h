#include <string>
class Unit;
const Unit * getCachedConstUnit (std::string name, int faction);
const Unit * setCachedConstUnit (std::string nam, int fac, Unit * un);
void purgeCache ();
