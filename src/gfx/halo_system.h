#ifndef HALOSYSTEM_H_
#define HALOSYSTEM_H_
#include <vector>
#include "matrix.h"
class Mesh;
struct GFXColor;
class Unit;

void DoParticles( QVector pos, float hullpercent, const Vector &velocity, float radial_size, float hullpct, int faction );
void LaunchOneParticle( const Matrix &mat, const Vector &vel, unsigned int seed, Unit *mesh, float hullpct, int faction );

class HaloSystem
{
private:
    struct Halo
    {
        Matrix trans;
        Vector size;
        Mesh *mesh;
        double sparkle_accum;
        float sparkle_rate;
        float activation;
        float oscale;
    };
    std::vector< Halo >halo;

public:
    virtual ~HaloSystem();
    HaloSystem();
	unsigned int NumHalos() const { return halo.size(); }
    unsigned int AddHalo( const char *filename,
                          const Matrix &trans,
                          const Vector &size,
                          const GFXColor &col,
                          std::string halo_type /*when it grows*/,
                          float activation_accel );
//Draw draws smoke and damage fx:
    void Draw( const Matrix &trans,
               const Vector &scale,
               int halo_alpha,
               float nebdist,
               float hullpercentage,
               const Vector &velocity,
               const Vector &linaccel,
               const Vector &angaccel,
               float maxaccel,
               float maxvelocity,
               int faction );
};

#endif

