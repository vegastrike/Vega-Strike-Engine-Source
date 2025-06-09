/*
 * technique.cpp
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


//
//C++ Implementation: Technique
//

#include <exception>
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>

#include "root_generic/xml_support.h"
#include "technique.h"
#include "root_generic/XMLDocument.h"
#include "src/vs_logging.h"
#include "src/gfxlib.h"
#include "gfx/aux_texture.h"
#include "root_generic/options.h"
#include "gldrv/gl_globals.h"
#include "audio/Exceptions.h"

using namespace XMLDOM;
using std::map;
using std::unique_ptr;

namespace __impl {

// TODO:Most of the code below up to end namespace should be refactored out
class Exception : public std::exception {
private:
    std::string _message;
public:
    virtual ~Exception() {
    }

    Exception() {
    }

    Exception(const Exception &other) : _message(other._message) {
    }

    explicit Exception(const std::string &message) : _message(message) {
    }

    virtual const char *what() const noexcept {
        return _message.c_str();
    }
};

class InvalidParameters : public Exception {
public:
    InvalidParameters() {
    }

    InvalidParameters(const string &msg) : Exception(msg) {
    }
};

class ProgramCompileError : public Exception {
public:
    ProgramCompileError() {
    }

    ProgramCompileError(const string &msg) : Exception(msg) {
    }
};

template<typename T>
static T parseEnum(const string &s, const map<string, T> &enumMap) {
    typename map<string, T>::const_iterator it = enumMap.find(s);
    if (it != enumMap.end()) {
        return it->second;
    } else {
        throw InvalidParameters("Enumerated value \"" + s + "\" not recognized");
    }
}

template<typename T>
static T parseEnum(const string &s, const map<string, T> &enumMap, T deflt) {
    typename map<string, T>::const_iterator it = enumMap.find(s);
    if (it != enumMap.end()) {
        return it->second;
    } else {
        return deflt;
    }
}

static Pass::TextureUnit::SourceType parseSourceType(const string &s, string::size_type &sep) {
    static map<string, Pass::TextureUnit::SourceType> enumMap;
    if (enumMap.empty()) {
        enumMap["decal"] = Pass::TextureUnit::Decal;
        enumMap["file"] = Pass::TextureUnit::File;
        enumMap["environment"] = Pass::TextureUnit::Environment;
        enumMap["detail"] = Pass::TextureUnit::Detail;
    }
    return parseEnum(s.substr(0, sep = s.find_first_of(':')), enumMap, Pass::TextureUnit::None);
}

Pass::Type parsePassType(const std::string &s) {
    static map<string, Pass::Type> enumMap;
    if (enumMap.empty()) {
        enumMap["fixed"] = Pass::FixedPass;
        enumMap["shader"] = Pass::ShaderPass;
    }
    return parseEnum(s, enumMap);
}

Pass::Tristate parseTristate(const std::string &s) {
    static map<string, Pass::Tristate> enumMap;
    if (enumMap.empty()) {
        enumMap["true"] = Pass::True;
        enumMap["false"] = Pass::False;
        enumMap["auto"] = Pass::Auto;
    }
    return parseEnum(s, enumMap);
}

Pass::BlendMode parseBlendMode(const std::string &s) {
    static map<string, Pass::BlendMode> enumMap;
    if (enumMap.empty()) {
        enumMap["default"] = Pass::Default;
        enumMap["add"] = Pass::Add;
        enumMap["multiply"] = Pass::Multiply;
        enumMap["alpha_blend"] = Pass::AlphaBlend;
        enumMap["decal"] = Pass::Decal;
        enumMap["premult_alpha"] = Pass::PremultAlphaBlend;
        enumMap["multi_alpha_blend"] = Pass::MultiAlphaBlend;
    }
    return parseEnum(s, enumMap);
}





//end namespace
};

using namespace __impl;

void Pass::setProgram(const string &vertex, const string &fragment) {
    vertexProgram = vertex;
    fragmentProgram = fragment;
    program = 0;
}

void Pass::addTextureUnit(const string &source,
        int target,
        const string &deflt,
        const string &paramName,
        Pass::TextureUnit::Kind texKind) {
    textureUnits.resize(textureUnits.size() + 1);
    TextureUnit &newTU = textureUnits.back();

    string::size_type ssep = string::npos, dsep = string::npos;
    newTU.sourceType = parseSourceType(source, ssep);
    newTU.defaultType = parseSourceType(deflt, dsep);
    newTU.targetIndex =
            newTU.origTargetIndex = target;
    newTU.targetParamName = paramName;
    newTU.targetParamId = -1;
    newTU.texKind = texKind;
    switch (newTU.sourceType) {
        case TextureUnit::Decal:
        case TextureUnit::Detail:
            if (ssep == string::npos) {
                throw InvalidParameters("Decal/Detail reference missing source index");
            }
            newTU.sourceIndex = atoi(source.c_str() + ssep + 1);
            break;
        case TextureUnit::File:
            if (ssep == string::npos) {
                throw InvalidParameters("File reference missing path");
            }
            newTU.sourcePath.assign(source, ssep + 1, string::npos);
            break;
        case TextureUnit::Environment:
            break;
        default:
            throw InvalidParameters("Missing source");
    }
    switch (newTU.defaultType) {
        case TextureUnit::Decal:
        case TextureUnit::Detail:
            if (dsep == string::npos) {
                throw InvalidParameters("Decal/Detail reference missing source index");
            }
            newTU.defaultIndex = atoi(deflt.c_str() + dsep + 1);
            break;
        case TextureUnit::File:
            if (dsep == string::npos) {
                throw InvalidParameters("File reference missing path");
            }
            newTU.defaultPath.assign(deflt, dsep + 1, string::npos);
            break;
        case TextureUnit::None: //FIXME added by chuck_starchaser; please verify correctness
        case TextureUnit::Environment: //FIXME added by chuck_starchaser; please verify correctness
        default: //FIXME added by chuck_starchaser; please verify correctness
            break; //FIXME added by chuck_starchaser; please verify correctness
    }
}

void Pass::addShaderParam(const string &name, float value[4], bool optional) {
    shaderParams.resize(shaderParams.size() + 1);
    ShaderParam &newSP = shaderParams.back();

    newSP.name = name;
    newSP.id = -1;
    newSP.semantic = ShaderParam::Constant;
    newSP.optional = optional;
    for (int i = 0; i < 4; ++i) {
        newSP.value[i] = value[i];
    }
}

void Pass::addShaderParam(const string &name, ShaderParam::Semantic semantic, bool optional) {
    shaderParams.resize(shaderParams.size() + 1);
    ShaderParam &newSP = shaderParams.back();

    newSP.name = name;
    newSP.id = -1;
    newSP.semantic = semantic;
    newSP.optional = optional;
}

/** Compile the pass (shaders, fetch shader params, etc...) */
void Pass::compile() {
    if (type == ShaderPass) {
        int prog = program; // BEGIN TRANSACTION

        if (prog != 0 && programVersion != GFXGetProgramVersion()) {
            GFXDestroyProgram(program);
            prog = 0;
        }

        if (prog == 0) {
            std::string defines;

            // Automatic defines
            if (sRGBAware) {
                if (gl_options.ext_srgb_framebuffer) {
                    defines += "#define SRGB_FRAMEBUFFER 1\n";
                } else {
                    defines += "#define SRGB_FRAMEBUFFER 0\n";
                }
            }
            if (gl_options.nv_fp2) {
                defines += "#define VGL_NV_fragment_program2 1\n";
            }

            // Compile program
            prog = GFXCreateProgram(vertexProgram.c_str(), fragmentProgram.c_str(),
                    (defines.empty() ? nullptr : defines.c_str()));
            if (prog == 0) {
                throw ProgramCompileError("Error compiling program vp:\"" + vertexProgram +
                        "\" fp:\"" + fragmentProgram + "\"");
            } //else {
            //BOOST_LOG_TRIVIAL(info) << boost::format("Successfully compiled and linked program \"%1%+%2%\"") % vertexProgram %
            //   fragmentProgram;
            //}
        }

        for (ShaderParamList::iterator it = shaderParams.begin(); it != shaderParams.end(); ++it) {
            it->id = GFXNamedShaderConstant(prog, it->name.c_str());
            if (it->id < 0) {
                if (!it->optional) {
                    throw ProgramCompileError("Cannot resolve shader constant \"" + it->name +
                            "\"");
                } //else {
                //  BOOST_LOG_TRIVIAL(info) << boost::format("Cannot resolve <<optional>> shader constant \"%1%\" in "
                //                                           "program \"%2%+%3%\"") %
                //                                 it->name % vertexProgram % fragmentProgram;
                //}
            }
        }
        int lastTU = -1;
        for (TextureUnitList::iterator tit = textureUnits.begin(); tit != textureUnits.end(); ++tit) {
            if (tit->sourceType == TextureUnit::File) {
                // Yep, we don't want to reload textures
                if (tit->texture.get() == nullptr) {
                    tit->texture.reset(new Texture(tit->sourcePath.c_str()));
                    if (!tit->texture->LoadSuccess()) {
                        throw InvalidParameters(
                                "Cannot load texture file \"" + tit->sourcePath + "\"");
                    }
                }
            } else if (tit->defaultType == TextureUnit::File) {
                // Yep, we don't want to reload textures
                if (tit->texture.get() == nullptr) {
                    tit->texture.reset(new Texture(tit->defaultPath.c_str()));
                    if (!tit->texture->LoadSuccess()) {
                        throw InvalidParameters(
                                "Cannot load texture file \"" + tit->defaultPath + "\"");
                    }
                }
            }
            if (!tit->targetParamName.empty()) {
                tit->targetParamId = GFXNamedShaderConstant(prog, tit->targetParamName.c_str());
                if (tit->targetParamId < 0) {
                    if (tit->origTargetIndex >= 0) {
                        throw ProgramCompileError(
                                "Cannot resolve shader constant \"" + tit->targetParamName + "\"");
                    } else {
                        tit->targetIndex = -1;
                    }
                } else {
                    if (tit->origTargetIndex < 0) {
                        tit->targetIndex = lastTU + 1;
                    }
                    lastTU = std::max(tit->targetIndex, lastTU);
                }
            }
        }

        // COMMIT ;-)
        program = prog;
        programVersion = GFXGetProgramVersion();
    }
}

/** Return whether the pass has been compiled or not */
bool Pass::isCompiled() const {
    return (type != ShaderPass) || (program != 0);
}

/** Return whether the pass has been compiled or not with a matching program version */
bool Pass::isCompiled(int programVersion) const {
    return (type != ShaderPass) || (program != 0 && this->programVersion == programVersion);
}

Technique::Technique(const string &name) :
        name(name), compiled(false), programVersion(0) {
    string root_technique_filename =
            game_options()->techniquesBasePath + "/"
                    + name + ".technique";
    string sub_technique_filename =
            game_options()->techniquesBasePath + "/"
                    + game_options()->techniquesSubPath + "/"
                    + name + ".technique";

    string filename;
    if (boost::filesystem::exists(root_technique_filename)) {
        filename = root_technique_filename;
    } else {
        filename = sub_technique_filename;
    }

    pt::ptree tree;
    pt::read_xml(filename, tree);

    for (const auto &iterator : tree) {
        parseTechniqueXML(iterator.second);
        break;
    }
}

void Technique::parseTechniqueXML(pt::ptree tree) {
    fallback = tree.get("<xmlattr>.fallback", "");
    int nextSequence = 0;

    for (const auto &iterator : tree) {
        if (iterator.second.empty()) {
            continue;
        }

        std::string key = iterator.first.data();

        if (key == "pass") {
            Pass pass;
            pass.parsePass(iterator.second, name, nextSequence);
            passes.push_back(pass);

        }
    }
}

Technique::Technique(const Technique &src) :
        name(src.name), fallback(src.fallback), compiled(false), programVersion(0), passes(src.passes) {
    // Not much else to do
    // Compiled techniques are still valid, and setting any parameter
    // that would invalidate it would result in recompilation
    // FIXME: should result in recompilation, not necessarily true now
}

Technique::~Technique() {
}

void Technique::compile() {
    if (!compiled || (GFXGetProgramVersion() != programVersion)) {
        //for (PassList::iterator it = passes.begin(); it != passes.end(); ++it)
        for (auto &pass : passes) {
            pass.compile();
        }

        compiled = true;
        programVersion = GFXGetProgramVersion();
    }
}

typedef map<string, TechniquePtr> TechniqueMap;
static TechniqueMap techniqueCache;

TechniquePtr Technique::getTechnique(const std::string &name) {
    TechniqueMap::const_iterator it = techniqueCache.find(name);
    if (it != techniqueCache.end()) {
        return it->second;
    } else {
        TechniquePtr ptr(new Technique(name));
        while (!ptr->isCompiled()) {
            try {
                ptr->compile();
                VS_LOG(info, (boost::format("Compilation of technique %1$s successful\n")
                        % ptr->getName().c_str()));
            }
            catch (const ProgramCompileError &e) {
                std::string fallback = ptr->getFallback();
                VS_LOG(warning, (boost::format("Compilation of technique %1$s failed... trying %2$s\nCause: %3$s\n")
                        % ptr->getName().c_str()
                        % fallback.c_str()
                        % e.what()));
                if (!fallback.empty() && fallback != name) {
                    ptr = getTechnique(fallback);
                } else {
                    break;
                }
            }
        }
        if (ptr->isCompiled()) {
            techniqueCache[name] = ptr;
        } else {
            throw InvalidParameters("Could not compile any technique for \"" + name + "\"");
        }
        return ptr;
    }
}

