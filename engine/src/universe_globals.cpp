
#include <vector>
using std::vector;

class Cockpit;

// Globals, in liueu of singletons
int _current_cockpit = 0;
vector< Cockpit* > _cockpits;
