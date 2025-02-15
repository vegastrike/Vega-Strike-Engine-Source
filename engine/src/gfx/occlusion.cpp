/*
 * occlusion.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Claudio Freire specifically
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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


#include "vegastrike.h"

#include "occlusion.h"

#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "vs_logging.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "xml_support.h"
#include "camera.h"
#include "universe.h"

#include "../gldrv/gl_globals.h"
#include <physics.h>

#include <limits>
#include <set>
#include "heap.h"

namespace Occlusion {

class Occluder {
    // Camera and light position for rating computations
    QVector cam;
    QVector light;
    float lightSize;

    // Occluding object data
    QVector pos;
    float rSize;

    // Occlusion influence
    float maxOcclusionDistance;
    float occlusionRating;

    void computeOcclusionRating() {
        double distanceSq = (pos - light).MagnitudeSquared();
        const auto max_occlusion_distance_dbl = static_cast<double>(maxOcclusionDistance);
        double maxSq = max_occlusion_distance_dbl * max_occlusion_distance_dbl;
        if (distanceSq >= maxSq) {
            occlusionRating = 0.f;
        } else {
            double distance = sqrt(distanceSq) - rSize - lightSize;
            if (distance <= 0.0) {
                occlusionRating = 1.0;
            } else {
                double camDistance = (pos - cam).Magnitude();
                occlusionRating = rSize / (rSize + distance + camDistance * 0.2);
            }
        }
    }

    void computeMaxOcclusionDistance() {
        double distance = (pos - light).Magnitude() - rSize - lightSize;
        float inner = (rSize - lightSize);
        float outer = (rSize + lightSize);
        // TODO: Think

        // Inner cone tangent is (rSize - lSize) / distance
        // Outer cone tangent is (rSize + lSize) / distance
        // Factor out the /distance thing, for performance and
        // precision sake, though.

        if (inner >= 0) {
            // Positive inner tagnent means infinite shadow cone
            maxOcclusionDistance = std::numeric_limits<float>::infinity();
        } else {
            if (outer >= 0) {
                // Positive outer cone means decreasing shadow intensity
                // over distance, so max distance will be a multiple
                // of maximum inner cone distance that will result in
                // minimal shadowing (4x distance = 16x less shadow)
                maxOcclusionDistance = -(distance / inner) * 4.0;
            } else {
                // All negative, quite impossible, but if it was possible,
                // max distance would be the length of the outer cone
                maxOcclusionDistance = -(distance / outer);
            }

            // Add some margin
            maxOcclusionDistance += lightSize + rSize;
        }
    }

public:
    Occluder(const QVector &pos_, float rSize_, const QVector &cam_, const QVector &light_, float lightSize_)
            : cam(cam_), light(light_), lightSize(lightSize_), pos(pos_), rSize(rSize_) {
        cam = _Universe->AccessCamera()->GetPosition();
        computeMaxOcclusionDistance();
        computeOcclusionRating();
    }

    /**
     * Comparison of occluder objects based on importance.
     * This comparison will return a < b when a is more
     * important than b, to make max-heap-based N-most-important
     * data structures easily implementable.
     */
    bool operator<(const Occluder &other) const {
        return occlusionRating > other.occlusionRating;
    }

    bool affects(const QVector &ctr, float rSize, float threshSize) const {
        return (
                (this->rSize >= threshSize)
                        && (((pos - ctr).Magnitude() - rSize) <= maxOcclusionDistance)
        );
    }

    float test(const QVector &lightPos, float lightSize, const QVector &pos, float rSize) const {
        // To maintain computational precision,
        // compute object position in reference to the light-occluder segment
        // and scale the tangent direction by the occluder's size

        QVector relOccluder = this->pos - lightPos;
        QVector relObject = pos - lightPos;

        double D = relOccluder.Magnitude();

        // Short-circuit the emitter itself
        if (D <= (lightSize + rSize)) {
            return 1.f;
        }

        // And no self-occluding
        if ((pos - this->pos).MagnitudeSquared() <= (rSize * rSize)) {
            return 1.f;
        }

        double Dinv = 1.0 / D;
        double Tinv = 1.0 / this->rSize;
        relOccluder *= Dinv;

        // Here, occD = 1.0, occT = 0.0
        double objD = relObject.Dot(relOccluder);
        double objT = (relObject - relOccluder * objD).Magnitude();

        // Shortcircuit if object is before occluder
        if (objD <= D) {
            return 1.0;
        }

        // pSize = 1.0 due to T scaling
        // Scale rSize and lightSize accordingly
        objD *= Dinv;
        objT *= Tinv;
        lightSize *= Tinv;
        rSize *= Tinv;

        // Our math can't handle objD==1.0, and it could happen
        // due to rounding error. In that case, it won't be
        // shadowed anyway.
        if (objD <= 1.0) {
            return 1.0;
        }

        // Occluder cone spans betwee tangents for (occD,occT-1) , (occD,occT+1)
        // counting from light edges (not origin)
        // Inner cone tangent is (pSize - lSize) / occD
        // Outer cone tangent is (pSize + lSize) / occD
        // Add rSize to lSize to somewhat account for target size
        double occInner = 1.0 - lightSize - rSize;
        double occOuter = 1.0 + lightSize + rSize;

        // We don't bother with object cone, just the direction to its center
        // from the occluder's edge
        double objTan = (objT - 1.0) / (objD - 1.0);

        if (objTan > occOuter) {
            return 1.f;
        } else if (objTan < occInner) {
            return 0.f;
        } else if (occOuter != occInner) {
            return float((objTan - occInner) / (occOuter - occInner));
        } else {
            return 1.f;
        }
    }

    unsigned long hash() const {
        return ((unsigned long) (long) (pos.x * 17)
                ^ (unsigned long) (long) (pos.y * 19)
                ^ (unsigned long) (long) (pos.z * 5));
    }
};

static std::vector<Occluder> forced_occluders;
static std::set<unsigned long> forced_occluders_set;

static VS::priority_queue<Occluder> dynamic_occluders;

static QVector biggestLightPos;
static float biggestLightSize;

void /*GFXDRVAPI*/ start() {
    end();

    std::vector<int> globalLights;
    GFXGlobalLights(globalLights);

    float bigSize = 0.f;
    int bigLight = -1;

    for (std::vector<int>::const_iterator it = globalLights.begin(); it != globalLights.end(); ++it) {
        const GFXLight &light = GFXGetLight(*it);
        if (bigLight < 0 || light.size > bigSize) {
            bigLight = *it;
            bigSize = light.size;
        }
    }

    if (bigLight >= 0) {
        biggestLightPos = GFXGetLight(bigLight).getPosition().Cast();
        biggestLightSize = bigSize;
    } else {
        biggestLightPos = QVector(0, 0, 0);
        biggestLightSize = 1.f;
    }
}

void /*GFXDRVAPI*/ end() {
    VS_LOG(trace, (boost::format("Occluders: %1% forced and %2% dynamic")
            % forced_occluders.size()
            % dynamic_occluders.size()));
    // FIXME - I think these three lines are memory leaks -- stephengtuggy 2019-10-01
    forced_occluders.clear();
    forced_occluders_set.clear();
    dynamic_occluders.clear();
}

void /*GFXDRVAPI*/ addOccluder(const QVector &pos, float rSize, bool significant) {
    Occluder occ(
            pos, rSize,
            _Universe->AccessCamera()->GetPosition(),
            biggestLightPos, biggestLightSize
    );

    if (significant) {
        unsigned long occHash = occ.hash();

        if (!forced_occluders_set.count(occHash)) {
            forced_occluders.push_back(occ);
            forced_occluders_set.insert(occHash);
        }
    } else {
        dynamic_occluders.push(occ);
        while (dynamic_occluders.size() > 16) {
            dynamic_occluders.pop();
        }
    }
}

float /*GFXDRVAPI*/ testOcclusion(const QVector &lightPos, float lightSize, const QVector &pos, float rSize) {
    float rv = 1.0f;

    {
        for (std::vector<Occluder>::const_iterator it = forced_occluders.begin(); it != forced_occluders.end(); ++it) {
            if (it->affects(pos, rSize, rSize * 4.f)) {
                rv *= it->test(lightPos, lightSize, pos, rSize);
                if (rv <= 0.f) {
                    return rv;
                }
            }
        }
    }

    {
        for (VS::priority_queue<Occluder>::const_iterator it = dynamic_occluders.begin(); it != dynamic_occluders.end();
                ++it) {
            if (it->affects(pos, rSize, rSize * 4.f)) {
                rv *= it->test(lightPos, lightSize, pos, rSize);
                if (rv <= 0.f) {
                    return rv;
                }
            }
        }
    }

    return rv;
}

} /* namespace Occlusion */
