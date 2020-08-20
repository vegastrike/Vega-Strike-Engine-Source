#ifndef UNIVERSE_GLOBALS_H
#define UNIVERSE_GLOBALS_H

#include <vector>
using std::vector;

class Cockpit;
class StarSystem;

// Globals, in liueu of singletons
extern int _current_cockpit;
extern vector< Cockpit* > _cockpits;

// currently only 1 star system is stored
// TODO: make into a simple variable
extern vector< StarSystem* > _active_star_systems;

// the system currently used by the scripting
// TODO: understand why we can't use active_star_system instead
extern StarSystem *_script_system;

#endif // UNIVERSE_GLOBALS_H
