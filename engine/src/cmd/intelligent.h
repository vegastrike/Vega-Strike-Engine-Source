/**
 * intelligent.h
 *
 * Copyright (C) 2020 Roy Falk
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INTELLIGENT_H
#define INTELLIGENT_H

#include "gfx/vec.h"

#include <vector>
#include <string>

class Unit;
class Order;

class Intelligent {
public:
    Intelligent();

public:
    class csOPCODECollider *getCollideTree(const Vector &scale = Vector(1,
                                                                        1,
                                                                        1), std::vector<struct mesh_polygon> * = NULL);
//Because accessing in daughter classes member function from Unit * instances
    Order *aistate = nullptr;

    Order *getAIState() const
    {
        return aistate;
    }

//Sets up a null queue for orders
//Uses AI so only in NetUnit and Unit classes
    void PrimeOrders();
    void PrimeOrdersLaunched();
    void PrimeOrders(Order *newAI);
//Sets the AI to be a specific order
    void SetAI(Order *newAI);
//Enqueues an order to the unit's order queue
    void EnqueueAI(Order *newAI);
//EnqueuesAI first
    void EnqueueAIFirst(Order *newAI);
//num subunits
    void LoadAIScript(const std::string &aiscript);
    bool LoadLastPythonAIScript();
    bool EnqueueLastPythonAIScript();
//Uses Order class but just a poiner so ok
//Uses AI so only in NetUnit and Unit classes
//for clicklist
    double getMinDis(const QVector &pnt) const;
//Uses AI stuff so only in NetUnit and Unit classes
    void SetTurretAI();
    void DisableTurretAI();
//AI so only in NetUnit and Unit classes
    std::string getFullAIDescription();
//Erases all orders that bitwise OR with that type
//Uses AI so only in NetUnit and Unit classes
    void eraseOrderType(unsigned int type);
//Executes 1 frame of physics-based AI
    void ExecuteAI();

    //Uses Universe stuff -> maybe only needed in Unit class
    bool isEnemy(const Unit *other) const
    {
        return getRelation(other) < 0.0;
    }

    bool isFriend(const Unit *other) const
    {
        return getRelation(other) > 0.0;
    }

    bool isNeutral(const Unit *other) const
    {
        return getRelation(other) == 0.0;
    }

    float getRelation(const Unit *other) const;
};

#endif // INTELLIGENT_H
