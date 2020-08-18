#ifndef UNIVERSE_GLOBALS_H
#define UNIVERSE_GLOBALS_H

#include <vector>
using std::vector;

class Cockpit;

// Globals, in liueu of singletons
extern int _current_cockpit;
extern vector< Cockpit* > _cockpits;

#endif // UNIVERSE_GLOBALS_H
