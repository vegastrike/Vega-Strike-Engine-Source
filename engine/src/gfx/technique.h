/*
 * technique.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_GFX_TECHNIQUE_H
#define VEGA_STRIKE_ENGINE_GFX_TECHNIQUE_H

#include "pass.h"

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

class Texture;
class Technique;

typedef boost::shared_ptr<Technique> TechniquePtr;
namespace pt = boost::property_tree;
using std::string;

void createTechnique(const string &name);

class Technique {
    string name;
    string fallback;
    bool compiled;
    int programVersion;

    //friend createTechnique;

public:

protected:
    vector<Pass> passes;

public:
    void parseTechniqueXML(pt::ptree tree);

    /** Create a technique by loading and parsing the definition in [name].technique */
    Technique(const string &name);

    /** Create a technique by copying another technique */
    explicit Technique(const Technique &src);

    ~Technique();

    const string &getName() const {
        return name;
    }

    const string &getFallback() const {
        return fallback;
    }

    /** returns true if the technique has been compiled */
    bool isCompiled() const {
        return compiled;
    }

    /** returns true if the technique has been compiled with a matching program version */
    bool isCompiled(int programVersion) const {
        return compiled && this->programVersion == programVersion;
    }

    void compile(); //Throws on error

    int getNumPasses() const {
        return passes.size();
    }

    const Pass &getPass(int idx) const {
        return passes[idx];
    }

    Pass &getPass(int idx) {
        return passes[idx];
    }

    /** Cached technique factory function */
    static TechniquePtr getTechnique(const string &name);
};

#endif //VEGA_STRIKE_ENGINE_GFX_TECHNIQUE_H
