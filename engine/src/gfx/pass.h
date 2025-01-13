/*
 * pass.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_PASS_H
#define VEGA_STRIKE_ENGINE_GFX_PASS_H

#include <string>
#include <vector>

#include <boost/smart_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace pt = boost::property_tree;

using std::string;
using std::vector;
using pt::ptree;

class Texture;

class Pass {
    /** Compiled and linked program */
    int program;

    /** Compiled program version stamp */
    int programVersion;

    /** Vertex Program file before compilation */
    string vertexProgram;

    /** Fragment Program file before compilation */
    string fragmentProgram;

public:
    enum Tristate {
        False = 0,
        True = 1,
        Auto = 2
    };

    enum Type {
        /** A fixed pass is one that uses up to 4 optional fixed-function texture targets:
         * 0: base diffuse
         * 1: specmap
         * 2: damage diffuse
         * 3: glow
         *
         * It is rendered using the fixed function pipeline.
         */
        FixedPass,

        /** A shader pass is a configurable pass that uses shaders to do everything.
         * Vertex and fragment programs must be specified.
         */
        ShaderPass
    };

    enum BlendMode {
        /** Whatever the mesh specifies as blend mode */
        Default,

        /** Blend function ONE ONE */
        Add,

        /** Blend function SRCCOLOR ZERO */
        Multiply,

        /** Blend function ALPHA INVSRCALPHA */
        AlphaBlend,

        /** Blend function ONE ZERO */
        Decal,

        /** Blend function ONE INVSRCALPHA */
        PremultAlphaBlend,

        /** Blend function ALPHA INVSRCALPHA for the first pass, ALPHA ONE for subsequent passes */
        MultiAlphaBlend
    };

    enum DepthFunction {
        Less,
        LEqual,
        Greater,
        GEqual,
        Equal,
        Always,
        Never
    };

    enum Face {
        None,
        Front,
        Back,
        FrontAndBack,
        DefaultFace
    };

    enum PolyMode {
        Point,
        Line,
        Fill
    };

    struct TextureUnit {
        enum SourceType {
            /** Nothing - blank */
            None,

            /** The mesh's Decal with specified numeric index */
            Decal,

            /** The texture specified with a path */
            File,

            /** The current environment cubemap */
            Environment,

            /** The mesh's detail texture (if there are more than one, it can be indexed) */
            Detail
        };

        enum Kind {
            /** Nothing - blank */
            TexDefault,

            /** Normal, 2D texture */
            Tex2D,

            /** 3D DDS texture */
            Tex3D,

            /** Packed cube texture */
            TexCube,

            /** Separated cube texture */
            TexSepCube
        };

        SourceType sourceType;
        SourceType defaultType;

        Kind texKind;

        int sourceIndex;
        int defaultIndex;
        int targetIndex;

        // Backup of mutable fields for shader reloading
        int origTargetIndex;

        /** For shader passes, the associated sampler uniform */
        string targetParamName;
        int targetParamId;

        string sourcePath;
        string defaultPath;

        /** Contains the source texture if the source type is File */
        boost::shared_ptr<Texture> texture;
    };

    struct ShaderParam {
        enum Semantic {
            /** Actually, two parameters in one.
             * EnvColor.rgb holds the color modulating environmental reflections.
             * EnvColor.a holds a mesh-defined, scale-less mip offset into environmental reflections.
             * Scale-less means that it is up to the shader how to interpret. 1.0 is max blur, 0.0 means no blur.
             */
            EnvColor,

            /** The cloaking phase data
             * CloakingPhase.r holds color phase
             * CloakingPhase.g holds alpha phase
             * CloakingPhase.b holds 1 cloak active, 0 for inactive
             * CloakingPhase.a holds 1 for glass-cloak, 0 for normal cloak
             * Pase 0 means not cloaked, 1 means fully cloaked, in-between means cloaking
             */
            CloakingPhase,

            /** The amount of damage received by the mesh */
            Damage,

            /** The amount of damage received by the mesh as a vec4 (.x=damage) */
            Damage4,

            /** The mesh's first detail plane normal */
            DetailPlane0,

            /** The mesh's second detail plane normal */
            DetailPlane1,

            /** The number of lights activated in the scene */
            NumLights,

            /** The array of active lights */
            ActiveLightsArray,

            /** The game time in seconds */
            GameTime,

            /** Not an auto-param, a manual param, with constant value */
            Constant,

            /** LightApparentSizeArray
             * An float4 array with apparent sizes of all lights
             * (only active ones have valid data).
             *
             * The components each have a different "view" of "apparent size"
             *   x : the worldspace size of the light
             *   y : cos(angle) where anlgle is the angle formed by the light
             *       vector towards the center and the light vector towards
             *       the edge of the light source.
             *   z : solid angle of the light source in steradians
             *   w : reserved
             */
            ApparentLightSizeArray
        };

        string name;
        int id;
        Semantic semantic;
        bool optional;
        float value[4];
    };

private:
    string techniqueName;

    typedef vector<TextureUnit> TextureUnitList;
    typedef vector<ShaderParam> ShaderParamList;

    TextureUnitList textureUnits;
    ShaderParamList shaderParams;

    void parseTextureUnit(ptree tree);
    void parseParam(ptree tree);
    void parseAutoParam(ptree tree);

    void setType(string typeString);
    void setZWrite(string zWriteString);
    void setPerLightIteration(ptree tree, string iterationString);
    void setBlendMode(string blendModeString);
    void setDepthFunction(string depthString);
    void setCullMode(string cullModeString);
    void setPolyMode(string polyModeString);
    ShaderParam::Semantic getShaderParam(const string &shaderString);

public:
    /** Type of pass - shader or fixed pipeline */
    Type type;

    /** Shaders are aware of SRGB frambuffers, so everything should be set up
     * for sRGB blending if sRGB framebuffers are available. Shaders will be compiled
     * with SRGB_FRAMEBUFFER set to 0 (not supported) or 1 (supported).
     */
    bool sRGBAware;

    /** Whether to write the color buffer or not */
    bool colorWrite;

    /** Whether to write the z-buffer or not - default or an override */
    Tristate zWrite;

    /** The number of lights this pass can handle, 0 means infinite */
    unsigned int perLightIteration;

    /** The maximum number of iterations, 0 means infinite - nonzero helps keep performance acceptable */
    unsigned int maxIterations;

    /** Blending mode - either default or an override */
    BlendMode blendMode;

    /** Z-test function - far queue always overrides with Always */
    DepthFunction depthFunction;

    /** Polygon backface culling mode. @Note Front will CULL Front faces, so standard is Back */
    Face cullMode;

    /** Polygon rasterization mode */
    PolyMode polyMode;

    /** Polygon offset slope factor */
    float offsetFactor;

    /** Polygon offset units */
    float offsetUnits;

    /** Line width - for line-mode polygons */
    float lineWidth;

    /** Sequence number, the order in which passes are rendered */
    int sequence;

    Pass();
    ~Pass();

    void parsePass(ptree tree, string techniqueName, int &nextSequence);
    static Pass parsePass(ptree tree);

    /** Set the vertex and fragment programs - throw on error */
    void setProgram(const string &vertex, const string &fragment);

    /** Get the vertex and fragment programs, compiled. 0 for fixed-function */
    int getCompiledProgram() const {
        return program;
    }

    /** Add a texture unit
     * @param source A string of the form [type]:[path or index] that specifies
     *      the texture unit's data source.
     * @param target The target texturing unit
     * @param deflt A string of the form [type]:[path or index] that specifies
     *      the texture unit's default data source if the mesh lacks
     *      the specified decal in 'source'
     * @param paramName The shader parameter name (for a shader pass) that is to be
     *      bound to this texture unit.
     * @remarks [type] may be either Decal or File, mapping to equally named SourceType s
     */
    void addTextureUnit(const string &source,
            int target,
            const string &deflt,
            const string &paramName,
            Pass::TextureUnit::Kind texKind);

    /** Add a constant shader param
     * @param name A string defining the shader's parameter name.
     * @param value An array of 4 floats with the parameter's value.
     * @param optional Optional parameters are allowed to be left unresolved.
     * @remarks Parameters with less than 4 values will simply ignore the other values and
     *      should be fed with any dummy value (like 0).
     */
    void addShaderParam(const string &name, float value[4], bool optional);

    /** Add an automatic shader param
     * @param name A string defining the shader's parameter name.
     * @param semantic The kind of value that will be automatically put in the variable.
     * @param optional Optional parameters are allowed to be left unresolved.
     */
    void addShaderParam(const string &name, ShaderParam::Semantic semantic, bool optional);

    /** Get the number of texture units in this pass */
    size_t getNumTextureUnits() const {
        return textureUnits.size();
    }

    /** Get the specified texture unit */
    const TextureUnit &getTextureUnit(int index) const {
        return textureUnits[index];
    }

    /** Get the specified texture unit */
    TextureUnit &getTextureUnit(int index) {
        return textureUnits[index];
    }

    /** Get the number of shader params in this pass */
    size_t getNumShaderParams() const {
        return shaderParams.size();
    }

    /** Get the specified shader param */
    const ShaderParam &getShaderParam(int index) const {
        return shaderParams[index];
    }

    /** Get the specified shader param */
    ShaderParam &getShaderParam(int index) {
        return shaderParams[index];
    }

    /** Compile the pass (shaders, fetch shader params, etc...) */
    void compile();

    /** Return whether the pass has been compiled or not */
    bool isCompiled() const;

    /** Return whether the pass has been compiled with a matching program version */
    bool isCompiled(int programVersion) const;
};

#endif //VEGA_STRIKE_ENGINE_GFX_PASS_H
