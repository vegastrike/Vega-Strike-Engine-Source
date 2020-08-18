
#include <vector>
using std::vector;

class Cockpit;
class StarSystem;

// Globals, in liueu of singletons
int _current_cockpit = 0;
vector< Cockpit* > _cockpits;

vector< StarSystem* > _active_star_systems;
StarSystem* _script_system;
