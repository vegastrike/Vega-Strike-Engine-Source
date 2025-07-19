/*
 * particle.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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


#include "particle.h"
#include "src/gfxlib.h"
#include "root_generic/lin_time.h"
#include "root_generic/vs_globals.h"
#include "cmd/unit_generic.h"
#include "src/config_xml.h"
#include "gfx/camera.h"
#include "gfx/aux_texture.h"
#include "gldrv/gl_globals.h"
#include "src/universe.h"

#include <iterator>
#include <limits>

#include "aligned.h"
#include "src/vs_logging.h"

ParticleTrail particleTrail("sparkle", 500, SRCALPHA, ONE, 0.05f, false, true);
ParticleTrail smokeTrail("smoke", 500, SRCALPHA, INVSRCALPHA);
ParticleTrail debrisTrail("debris", 500, SRCALPHA, INVSRCALPHA, 0.5, true);

void ParticleTrail::ChangeMax(unsigned int max) {
    unsigned int vertices_per_particle;
    unsigned int indices_per_particle;

    if (config.use_points) {
        vertices_per_particle = indices_per_particle = 1;
    } else {
        vertices_per_particle = indices_per_particle = 12;
    }

    if (gl_options.max_array_vertices > 0 &&
            (max * vertices_per_particle) > gl_options.max_array_vertices) {
        max = gl_options.max_array_vertices / vertices_per_particle;
        if (max > 0) {
            max--;
        } // for a margin
    }
    if (gl_options.max_array_indices > 0 &&
            (max * indices_per_particle) > gl_options.max_array_indices) {
        max = gl_options.max_array_indices / indices_per_particle;
        if (max > 0) {
            max--;
        } // for a margin
    }
    if (max < 4) {
        max = 4;
    }

    this->max_particles = max;
}

//Write 12 * 3 pos and 12 * 4 col and 12 * 2 tex float values into v and increment v by 108
static inline void SetQuadVertex(const QVector &loc,
        const GFXColor &col,
        const float psize,
        const float grow,
        const float trans,
        std::back_insert_iterator<std::vector<float> > &v,
        const QVector &campos) {
    float size = psize * (grow * (1 - col.a) + col.a);
    float maxsize = (psize > size) ? psize : size;
    float minsize = (psize <= size) ? psize : size;

    //Squared, surface-linked decay - looks nicer, more real for emissive gasses
    //NOTE: maxsize/minsize allows for inverted growth (shrinkage) while still fading correctly. Cheers!
    GFXColor c = col * (col.a * trans * (minsize / ((maxsize > 0) ? maxsize : 1.f)));
    QVector l = loc - campos;

    *v++ = l.i + size;
    *v++ = l.j + size;
    *v++ = l.k;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 0;
    *v++ = 0;
    *v++ = l.i + size;
    *v++ = l.j - size;
    *v++ = l.k;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 0;
    *v++ = 1;
    *v++ = l.i - size;
    *v++ = l.j - size;
    *v++ = l.k;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 1;
    *v++ = 1;
    *v++ = l.i - size;
    *v++ = l.j + size;
    *v++ = l.k;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 1;
    *v++ = 0;

    *v++ = l.i;
    *v++ = l.j + size;
    *v++ = l.k + size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 0;
    *v++ = 0;
    *v++ = l.i;
    *v++ = l.j - size;
    *v++ = l.k + size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 0;
    *v++ = 1;
    *v++ = l.i;
    *v++ = l.j - size;
    *v++ = l.k - size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 1;
    *v++ = 1;
    *v++ = l.i;
    *v++ = l.j + size;
    *v++ = l.k - size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 1;
    *v++ = 0;

    *v++ = l.i + size;
    *v++ = l.j;
    *v++ = l.k + size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 0;
    *v++ = 0;
    *v++ = l.i + size;
    *v++ = l.j;
    *v++ = l.k - size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 0;
    *v++ = 1;
    *v++ = l.i - size;
    *v++ = l.j;
    *v++ = l.k - size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 1;
    *v++ = 1;
    *v++ = l.i - size;
    *v++ = l.j;
    *v++ = l.k + size;
    *v++ = c.r;
    *v++ = c.g;
    *v++ = c.b;
    *v++ = c.a;
    *v++ = 1;
    *v++ = 0;
}

template<typename R, typename T>
class IndexCompare {
    const std::vector<R> &ref;

public:
    explicit IndexCompare(const std::vector<R> &ref_) : ref(ref_) {
    }

    bool operator()(const T &a, const T &b) const {
        return ref[a] > ref[b];
    }
};

ParticleTrail::Config::Config(const std::string &prefix) {
    texture = nullptr;
    initialized = false;
    this->prefix = prefix;
}

ParticleTrail::Config::~Config() {
    if (texture != nullptr) {
        delete texture;
        texture = nullptr;
    }
}

void ParticleTrail::Config::init() {
    if (initialized) {
        return;
    }

    use = XMLSupport::parse_bool(vs_config->getVariable("graphics", prefix, "true"));
    use_points = XMLSupport::parse_bool(vs_config->getVariable("graphics", prefix + "point", "false"));
    pblend = XMLSupport::parse_bool(vs_config->getVariable("graphics", prefix + "blend", "false"));
    pgrow = XMLSupport::parse_floatf(vs_config->getVariable("graphics",
            prefix + "growrate",
            "50.0"));     //50x size when disappearing
    ptrans = XMLSupport::parse_floatf(vs_config->getVariable("graphics",
            prefix + "alpha",
            "2.5"));     //NOTE: It's the base transparency, before surface attenuation, so it needn't be within the [0-1] range.
    pfade = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "fade", "0.1"));

    if (use_points) {
        psize = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "size", "1.5"));
        psmooth = XMLSupport::parse_bool(vs_config->getVariable("graphics", prefix + "smooth", "false"));
    } else {
        std::string s = vs_config->getVariable("graphics", prefix + "texture", "supernova.bmp");
        texture = new Texture(s.c_str());
    }

    initialized = true;
}

ParticleTrail::Config::Config() : texture(nullptr), initialized(false), prefix("") {}

void ParticleEmitter::Config::init(const std::string &prefix) {
    rate = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "scale", "8"));
    speed = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "speed", ".5"));
    locSpread = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "flare", ".15"));
    spread = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "spread", ".04"));
    absSpeed = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "absolutespeed", ".02"));
    relSize = XMLSupport::parse_floatf(vs_config->getVariable("graphics", prefix + "sizerelative", ".125"));
    fixedSize = XMLSupport::parse_bool(vs_config->getVariable("graphics", prefix + "fixedsize", "0"));
}

void ParticleTrail::DrawAndUpdate() {
    // Short-circuit, not only an optimization, it avoids assertion failures in GFXDraw
    if (!config.initialized) {
        config.init();
        ChangeMax(max_particles);
        VS_LOG(info,
                (boost::format("Configured particle system %1% with %2% particles") % config.prefix % max_particles));
    }
    if (!config.use || particles.empty()) {
        return;
    }

    bool use_points = config.use_points;
    bool pblend = config.pblend;
    float pgrow = config.pgrow;
    float ptrans = config.ptrans;
    float pfade = config.pfade;

    const QVector kCameraPosition = _Universe->AccessCamera()->GetPosition();
    size_t nparticles = particles.size();

    // Draw particles
    GFXDisable(CULLFACE);
    GFXDisable(LIGHTING);
    GFXLoadIdentity(MODEL);
    GFXTranslateModel(kCameraPosition);
    if (use_points) {
        float psize = config.psize;
        bool psmooth = config.psmooth;

        GFXDisable(TEXTURE0);
        GFXPointSize(psize);
        if (psmooth && configuration()->graphics.smooth_points) {
            glEnable(GL_POINT_SMOOTH);
        }
        if (pblend) {
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
        } else {
            GFXBlendMode(ONE, ZERO);
        }

        particleVert.clear();
        particleVert.reserve(particles.size() * (3 + 4));
        std::back_insert_iterator<std::vector<float> > v(particleVert);
        for (size_t i = 0; i < nparticles; ++i) {
            const QVector &loc = particles[i].location;
            std::back_insert_iterator<std::vector<float>> &v_1 = v;
            float size = particles[i].size * (pgrow * (1.0f - particles[i].color.a) + particles[i].color.a);
            float maxsize = (particles[i].size > size) ? particles[i].size : size;
            float minsize = (particles[i].size <= size) ? particles[i].size : size;

            //Squared, surface-linked decay - looks nicer, more real for emissive gasses
            //NOTE: maxsize/minsize allows for inverted growth (shrinkage) while still fading correctly. Cheers!
            GFXColor c_1 =
                    particles[i].color * (particles[i].color.a * ptrans * (minsize / ((maxsize > 0) ? maxsize : 1.0F)));
            QVector l = loc - kCameraPosition;

            *v_1++ = l.x;
            *v_1++ = l.y;
            *v_1++ = l.y;
            *v_1++ = c_1.r;
            *v_1++ = c_1.g;
            *v_1++ = c_1.b;
            *v_1++ = c_1.a;
        }
        GFXDraw(GFXPOINT, &particleVert[0], particles.size(), 3, 4);

        glDisable(GL_POINT_SMOOTH);
        GFXPointSize(1);
    } else {
        Texture *t = config.texture;
        const int vertsPerParticle = 12;
        bool dosort = blenddst != ONE && (blenddst != ZERO || !writeDepth);

        GFXEnable(TEXTURE0);
        GFXDisable(TEXTURE1);
        GFXBlendMode(blendsrc, blenddst);
        if (writeDepth) {
            GFXEnable(DEPTHWRITE);
        } else {
            GFXDisable(DEPTHWRITE);
        }
        if (alphaMask > 0) {
            GFXAlphaTest(GEQUAL, alphaMask);
        }
        t->MakeActive();

        if (dosort) {
            // Must sort
            distances.clear();
            distances.reserve(nparticles);
            {
                for (const auto & it : particles) {
                    distances.push_back((kCameraPosition - it.location).MagnitudeSquared());
                }
            }
            IndexCompare<float, unsigned short> dcomp(distances);

            unsigned short nindices;
            if (nparticles >= (size_t) std::numeric_limits<unsigned short>::max() / vertsPerParticle) {
                nindices = (unsigned short) std::numeric_limits<unsigned short>::max() / vertsPerParticle;
            } else {
                nindices = (unsigned short) nparticles;
            }
            pointIndices.clear();
            pointIndices.reserve(nindices);
            {
                for (unsigned short i = 0; i < nindices; ++i) {
                    pointIndices.push_back(i);
                }
            }

            std::sort(pointIndices.begin(), pointIndices.end(), dcomp);

            indices.clear();
            indices.reserve(static_cast<size_t>(nindices) * vertsPerParticle);
            {
                for (unsigned short point_index : pointIndices) {
                    for (int i = 0; i < vertsPerParticle; ++i) {
                        indices.push_back(point_index * vertsPerParticle + i);
                    }
                }
            }
        }

        particleVert.clear();
        particleVert.reserve(nparticles * vertsPerParticle * (3 + 4 + 2));
        std::back_insert_iterator<std::vector<float> > v(particleVert);
        for (size_t i = 0; i < nparticles; ++i) {
            SetQuadVertex(particles[i].location, particles[i].color, particles[i].size, pgrow, ptrans, v, kCameraPosition);
        }

        if (dosort) {
            VS_LOG(trace, (boost::format("Drawing %1%/%2% sorted particles") % nparticles % max_particles));
            GFXDrawElements(GFXQUAD,
                    &particleVert[0], nparticles * vertsPerParticle,
                    &indices[0], indices.size(),
                    3, 4, 2);
        } else {
            VS_LOG(trace, (boost::format("Drawing %1%/%2% unsorted particles") % nparticles % max_particles));
            GFXDraw(GFXQUAD, &particleVert[0], nparticles * 12, 3, 4, 2);
        }

        if (alphaMask > 0) {
            GFXAlphaTest(ALWAYS, 0);
        }

        GFXBlendMode(ONE, ZERO);
    }
    GFXLoadIdentity(MODEL);

    // Update particles
    const double elapsed_time_as_double = GetElapsedTime();
    const float elapsed_time_as_float = static_cast<float>(elapsed_time_as_double);
    if (fadeColor) {
        float fadetimef = pfade * elapsed_time_as_double;
        _ALIGNED(16) GFXColor fadetime = GFXColor(fadetimef, fadetimef, fadetimef, fadetimef);

        for (auto & particle : particles) {
            particle.location += particle.velocity * elapsed_time_as_double;
            particle.color = particle.color - fadetime;
            particle.color.clamp();
        }
    } else {
        for (auto & particle : particles) {
            particle.location += particle.velocity * elapsed_time_as_double;
            particle.color.a = std::max(0.0F, particle.color.a - pfade * elapsed_time_as_float);
        }
    }

    // Compute alpha for dead particles
    float min_alpha = (ptrans > 0.0f) ? sqrtf(alphaMask / ptrans) : 0.0f;

    //if (nparticles) {
    //VS_LOG(trace, (boost::format("Drawn %1% particles, min_alpha %2%") % nparticles % min_alpha));
    //}

//    // Quickly remove dead particles at the end
//    while (!particleColor.empty() && particleColor.back().a <= min_alpha) {
//        particleVel.pop_back();
//        particleLoc.pop_back();
//        particleColor.pop_back();
//        particleSize.pop_back();
//    }

    // Remove dead particles anywhere
    auto first_to_remove = std::stable_partition(particles.begin(), particles.end(), [min_alpha](Particle& item){ return item.color.a > min_alpha; });
//    std::for_each(first_to_remove, particles.end(), [](Particle& particle){ delete });
    particles.erase(first_to_remove, particles.end());
//    vector<Vector, aligned_allocator<Vector> >::iterator v = particleVel.begin();
//    vector<QVector, aligned_allocator<QVector> >::iterator loc = particleLoc.begin();
//    vector<GFXColor, aligned_allocator<GFXColor> >::iterator col = particleColor.begin();
//    vector<float, aligned_allocator<float> >::iterator sz = particleSize.begin();
//    while (col != particleColor.end()) {
//        if (!(col->a > min_alpha)) {
//            vector<Vector, aligned_allocator<Vector> >::iterator vlast = particleVel.end() - 1;
//            vector<QVector, aligned_allocator<QVector> >::iterator loclast = particleLoc.end() - 1;
//            vector<GFXColor, aligned_allocator<GFXColor> >::iterator collast = particleColor.end() - 1;
//            vector<float, aligned_allocator<float> >::iterator szlast = particleSize.end() - 1;
//            if (col != collast) {
//                *v = *vlast;
//                *loc = *loclast;
//                *col = *collast;
//                *sz = *szlast;
//            }
//            size_t index = col - particleColor.begin();
//            particleColor.pop_back();
//            particleVel.pop_back();
//            particleLoc.pop_back();
//            particleSize.pop_back();
//            col = particleColor.begin() + index;
//            v = particleVel.begin() + index;
//            loc = particleLoc.begin() + index;
//            sz = particleSize.begin() + index;
//        } else {
//            ++loc;
//            ++col;
//            ++sz;
//            ++v;
//        }
//    }
}

void ParticleTrail::AddParticle(const ParticlePoint &P, const Vector &V, float size) {
    if (!config.initialized) {
        config.init();
    }
    if (!config.use) {
        return;
    }

    Particle particle{};
    particle.velocity = V;
    particle.location = P.loc;
    particle.color = P.col;
    particle.size = P.size;

    if (particles.size() >= max_particles) {
        size_t off = ((size_t) rand()) % particles.size();
        particles.at(off) = particle;
    } else {
        particles.push_back(particle);
    }
}

void ParticleEmitter::doParticles(const QVector &pos,
        float rSize,
        float percent,
        const Vector &basevelocity,
        const Vector &velocity,
        float pSize,
        const GFXColor &color) const {
    percent = 1 - percent;
    int i = rand();
    if (i < RAND_MAX * percent * config.rate) {
        ParticlePoint pp;
        float r1 = rand() / ((float) RAND_MAX * .5) - 1;
        float r2 = rand() / ((float) RAND_MAX * .5) - 1;
        float r3 = rand() / ((float) RAND_MAX * .5) - 1;
        float r4 = rand() / ((float) RAND_MAX * .5) - 1;
        Vector rand(r1, r2, r3);
        rand.Normalize();
        pp.loc = pos + rand * rSize * config.locSpread;

        // Make randomization direction-centric
        Vector direction = velocity;
        direction.Normalize();
        rand *= config.spread;
        rand += direction;
        rand.Normalize();
        rand *= 1.0 + config.spread * r4;

        pp.col = color;
        particleTrail.AddParticle(pp,
                rand * (std::max(velocity.Magnitude(), config.absSpeed) * config.spread
                        + config.absSpeed)
                        + velocity * config.speed + basevelocity,
                config.fixedSize ? config.relSize : (pSize * config.relSize)
        );
    }
}
