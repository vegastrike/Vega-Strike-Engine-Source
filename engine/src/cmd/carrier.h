#ifndef CARRIER_H
#define CARRIER_H

#include "cargo.h"

#include <string>

// A unit (ship) that carries cargo
class Carrier
{
public:
    std::vector< Cargo > cargo;

    Carrier();
    void SortCargo();
    static std::string cargoSerializer( const struct XMLType &input, void *mythis );

    static Unit * makeMasterPartList();
    bool CanAddCargo( const Cargo &carg ) const;
    void AddCargo( const Cargo &carg, bool sort = true );
    int RemoveCargo( unsigned int i, int quantity, bool eraseZero = true );
    float PriceCargo( const std::string &s );
    Cargo& GetCargo( unsigned int i );
    const Cargo& GetCargo( unsigned int i ) const;
    void GetSortedCargoCat( const std::string &category, size_t &catbegin, size_t &catend );
//below function returns NULL if not found
    Cargo * GetCargo( const std::string &s, unsigned int &i );
    const Cargo * GetCargo( const std::string &s, unsigned int &i ) const;
    unsigned int numCargo() const;
    std::string GetManifest( unsigned int i, Unit *scanningUnit, const Vector &original_velocity ) const;
    bool SellCargo( unsigned int i, int quantity, float &creds, Cargo &carg, Unit *buyer );
    bool SellCargo( const std::string &s, int quantity, float &creds, Cargo &carg, Unit *buyer );
    bool BuyCargo( const Cargo &carg, float &creds );
    bool BuyCargo( unsigned int i, unsigned int quantity, Unit *buyer, float &creds );
    bool BuyCargo( const std::string &cargo, unsigned int quantity, Unit *buyer, float &creds );
    void EjectCargo( unsigned int index );
    float getEmptyCargoVolume( void ) const;
    float getCargoVolume( void ) const;
    float getEmptyUpgradeVolume( void ) const;
    float getUpgradeVolume( void ) const;
    float getHiddenCargoVolume( void ) const;
};

#endif // CARRIER_H
