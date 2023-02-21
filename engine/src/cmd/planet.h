/*
 * planet.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Roy Falk, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


/* planet.h
 *  ok *
 * Unit type that is a gravity source, and follows a set orbit pattern
 */
#ifndef _PLANET_H_
#define _PLANET_H_

#include <stdio.h>
#include "unit_generic.h"
#include "ai/order.h"
#include "collection.h"
#include <vector>
#include <string>

struct GFXMaterial;
/* Orbits in the xy plane with the given radius. Depends on a reorientation of coordinate bases */

class Atmosphere;
struct AtmosphericFogMesh;
class ContinuousTerrain;
class PlanetaryTransform;
class Texture;

class Planet : public Unit {
private:
    vega_types::SharedPtr<Animation> shine{};

protected:
    Atmosphere *atmosphere = nullptr;

    ContinuousTerrain *terrain = nullptr;
    PlanetaryTransform *terraintrans = nullptr;
    Vector TerrainUp;
    Vector TerrainH;
    bool inside = false;
    bool atmospheric = false; //then users can go inside!
    float radius = 0.0f;
    float gravity;
    UnitCollection insiders;
    std::vector<int> lights;
public:
    UnitCollection satellites = UnitCollection();

public:
    Planet();

    Planet(QVector x,
            QVector y,
            float vely,
            const Vector &rotvel,
            float pos,
            float gravity,
            float radius,
            const std::string &filename,
            const std::string &technique,
            const std::string &unitname,
            BLENDFUNC blendsrc,
            BLENDFUNC blenddst,
            const std::vector<std::string> &dest,
            const QVector &orbitcent,
            Unit *parent,
            const GFXMaterial &ourmat,
            const std::vector<GFXLightLocal> &,
            int faction,
            string fullname,
            bool inside_out = false);

    void InitPlanet(QVector x,
            QVector y,
            float vely,
            const Vector &rotvel,
            float pos,
            float gravity,
            float radius,
            const string &filename,
            const string &technique,
            const string &unitname,
            const vector<string> &dest,
            const QVector &orbitcent,
            Unit *parent,
            int faction,
            string fullname,
            bool inside_out,
            unsigned int lights_num);

    virtual ~Planet();

    friend class PlanetaryOrbit;

    enum Vega_UnitType isUnit() const {
        return Vega_UnitType::planet;
    }

    // Methods
    void AddAtmosphere(const std::string &texture,
            float radius,
            BLENDFUNC blendSrc,
            BLENDFUNC blendDst,
            bool inside_out);
    void AddCity(const std::string &texture,
            float radius,
            int numwrapx,
            int numwrapy,
            BLENDFUNC blendSrc,
            BLENDFUNC blendDst,
            bool inside_out = false,
            bool reverse_normals = true);
    virtual void AddFog(const std::vector<AtmosphericFogMesh> &, bool optical_illusion);
    void AddRing(const std::string &texture,
            float iradius,
            float oradius,
            const QVector &r,
            const QVector &s,
            int slices,
            int numwrapx,
            int numwrapy,
            BLENDFUNC blendSrc,
            BLENDFUNC blendDst);
    void AddSatellite(Unit *orbiter);

    Vector AddSpaceElevator(const std::string &name, const std::string &faction, char direction);

    Unit *beginElement(QVector x,
            QVector y,
            float vely,
            const Vector &rotvel,
            float pos,
            float gravity,
            float radius,
            const string &filename,
            const string &technique,
            const string &unitname,
            BLENDFUNC blendsrc,
            BLENDFUNC blenddst,
            const vector<string> &dest,
            int level,
            const GFXMaterial &ourmat,
            const std::vector<GFXLightLocal> &ligh,
            bool isunit,
            int faction,
            string fullname,
            bool inside_out);

    void DisableLights();
    virtual void Draw(const Transformation &quat = identity_transformation, const Matrix &m = identity_matrix);
    void DrawTerrain();
    void EnableLights();
    void endElement();

    Atmosphere *getAtmosphere() {
        return atmosphere;
    }

    string getCargoUnitName() const {
        return getFullname();
    }

    string getHumanReadablePlanetType() const;

    inline const float getRadius() const {
        return radius;
    }

    ContinuousTerrain *getTerrain(PlanetaryTransform *&t) {
        t = terraintrans;
        return terrain;
    }

    Planet *GetTopPlanet(int level);

    virtual void Kill(bool erasefromsave = false);

    static void ProcessTerrains();

    void setAtmosphere(Atmosphere *);
    PlanetaryTransform *setTerrain(ContinuousTerrain *, float ratiox, int numwraps, float scaleatmos);

    class PlanetIterator {
    public:
        PlanetIterator(Planet *p) {
            localCollection.append(p);
            pos = localCollection.createIterator();
        }

        ~PlanetIterator() {
        }

        void preinsert(Unit *unit) {
            abort();
        }

        void postinsert(Unit *unit) {
            abort();
        }

        void remove() {
            abort();
        }

        inline Unit *current() {
            if (!pos.isDone()) {
                return *pos;
            }
            return NULL;
        }

        void advance() {
            if (current() != NULL) {
                Unit *cur = *pos;
                if (cur->isUnit() == Vega_UnitType::planet) {
                    for (un_iter tmp(((Planet *) cur)->satellites.createIterator()); !tmp.isDone(); ++tmp) {
                        localCollection.append((*tmp));
                    }
                }
                ++pos;
            }
        }

        inline PlanetIterator &operator++() {
            advance();
            return *this;
        }

        inline Unit *operator*() {
            return current();
        }

    private:
        inline un_iter operator++(int) {
            abort();
        }

        UnitCollection localCollection;
        un_iter pos;
    };

    PlanetIterator createIterator() {
        return PlanetIterator(this);
    }

    bool isAtmospheric() const {
        return hasLights() || atmospheric;
    }

    bool hasLights() const {
        return !lights.empty();
    }

    const std::vector<int> &activeLights() {
        return lights;
    }

    virtual const float GetHullPercent() const {
        return 1.0f;
    }

    virtual const float GetShieldPercent() const {
        return 0.0f;
    }

private:

/// copy constructor forbidden
    Planet(const Planet &);

/// assignment operator forbidden
    Planet &operator=(const Planet &);
    friend bool operator==(const Planet &lhs, const Planet &rhs);
};

#endif

