#include <string>
class Unit;
const Unit * getCachedContUnit (std::string name, int faction);
void setCachedConstUnit (Unit * un);
void purgeCache ();
