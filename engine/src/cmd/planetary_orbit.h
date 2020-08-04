#ifndef PLANETARY_ORBIT_H
#define PLANETARY_ORBIT_H

#include "gfx/vec.h"
#include "star_system_generic.h"
#include "ai/order.h"

class PlanetaryOrbit : public Order
{
private:
    double  velocity;
    double  theta;
    double  inittheta;

    QVector x_size;
    QVector y_size;
    QVector focus;
#define ORBIT_PRIORITY 8
#define NUM_ORBIT_AVERAGE (SIM_QUEUE_SIZE/ORBIT_PRIORITY)
    QVector orbiting_average[NUM_ORBIT_AVERAGE];
    float   orbiting_last_simatom;
    int     current_orbit_frame;
    bool    orbit_list_filled;
protected:
///A vector containing all lihgts currently activated on current planet
    std::vector< int >lights;

public: PlanetaryOrbit( Unit *p,
                        double velocity,
                        double initpos,
                        const QVector &x_axis,
                        const QVector &y_axis,
                        const QVector &Centre,
                        Unit *target = NULL );
    ~PlanetaryOrbit();
    void Execute();
};

#endif // PLANETARY_ORBIT_H
