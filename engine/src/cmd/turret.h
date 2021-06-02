#ifndef TURRET_H
#define TURRET_H

#include <string>
#include <map>

class Unit;

class Turret
{
public:
    Turret();
};

std::map< std::string, std::string > parseTurretSizes();
std::string getTurretSize( const std::string &size );
Unit * CreateGenericTurret( std::string tur, int faction );


#endif // TURRET_H
