#ifndef CARGO_H
#define CARGO_H

#include "SharedPool.h"
#include "gfxlib_struct.h"

#include <string>


class Cargo
{
public:
    StringPool::Reference content;
    StringPool::Reference category;
    StringPool::Reference description;
    int   quantity;
    float price;
    float mass;
    float volume;
    bool  mission;
    bool  installed;
    float functionality;
    float maxfunctionality;
    Cargo();
    Cargo( std::string name, std::string cc, float pp, int qq, float mm, float vv, float func, float maxfunc );
    Cargo( std::string name, std::string cc, float pp, int qq, float mm, float vv );
    float GetFunctionality();
    float GetMaxFunctionality();
    void SetFunctionality( float func );
    void SetMaxFunctionality( float func );
    void SetMissionFlag( bool flag );
    void SetPrice( float price );
    void SetMass( float mass );
    void SetVolume( float vol );
    void SetQuantity( int quantity );
    void SetContent( const std::string &content );
    void SetCategory( const std::string &category );

    bool GetMissionFlag() const;
    const std::string& GetCategory() const;
    const std::string& GetContent() const;
    const std::string& GetDescription() const;
    std::string GetCategoryPython();
    std::string GetContentPython();
    std::string GetDescriptionPython();
    int GetQuantity() const;
    float GetVolume() const;
    float GetMass() const;
    float GetPrice() const;
    bool operator==( const Cargo &other ) const;
    bool operator<( const Cargo &other ) const;


};

// A stupid struct that is only for grouping 2 different types of variables together in one return value
// Must come after Cargo for obvious reasons
class CargoColor
{
public:
    Cargo    cargo;
    GFXColor color;
    CargoColor() : cargo()
        , color( 1, 1, 1, 1 ) {}
};

#endif // CARGO_H
