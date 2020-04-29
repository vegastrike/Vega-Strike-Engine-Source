#ifndef __TECHNIQUE_H__INCLUDED__
#define __TECHNIQUE_H__INCLUDED__

#include <string>
#include <vector>

#include <boost/smart_ptr.hpp>

class Texture;
class Technique;

typedef boost::shared_ptr< Technique >TechniquePtr;

class Technique
{
    std::string name;
    std::string fallback;
    bool compiled;
    int programVersion;
    
public:
    class Pass
    {
        /** Compiled and linked program */
        int program;
        
        /** Compiled program version stamp */
        int programVersion;
        
        /** Vertex Program file before compilation */
        std::string vertexProgram;
        
        /** Fragment Program file before compilation */
        std::string fragmentProgram;
        
public:
        enum Tristate
        {
            False=0,
            True =1,
            Auto =2
        };

        enum Type
        {
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

        enum BlendMode
        {
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

        enum DepthFunction
        {
            Less,
            LEqual,
            Greater,
            GEqual,
            Equal,
            Always,
            Never
        };

        enum Face
        {
            None,
            Front,
            Back,
            FrontAndBack,
            DefaultFace
        };

        enum PolyMode
        {
            Point,
            Line,
            Fill
        };

        struct TextureUnit
        {
            enum SourceType
            {
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

            enum Kind
            {
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

            int  sourceIndex;
            int  defaultIndex;
            int  targetIndex;
            
            // Backup of mutable fields for shader reloading
            int  origTargetIndex;

            /** For shader passes, the associated sampler uniform */
            std::string targetParamName;
            int targetParamId;

            std::string sourcePath;
            std::string defaultPath;

            /** Contains the source texture if the source type is File */
            boost::shared_ptr< Texture >texture;
        };

        struct ShaderParam
        {
            enum Semantic
            {
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

            std::string name;
            int id;
            Semantic    semantic;
            bool optional;
            float value[4];
        };

private:
        typedef std::vector< TextureUnit >TextureUnitList;
        typedef std::vector< ShaderParam >ShaderParamList;

        TextureUnitList textureUnits;
        ShaderParamList shaderParams;

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
        unsigned int  perLightIteration;

        /** The maximum number of iterations, 0 means infinite - nonzero helps keep performance acceptable */
        unsigned int  maxIterations;

        /** Blending mode - either default or an override */
        BlendMode     blendMode;

        /** Z-test function - far queue always overrides with Always */
        DepthFunction depthFunction;

        /** Polygon backface culling mode. @Note Front will CULL Front faces, so standard is Back */
        Face          cullMode;

        /** Polygon rasterization mode */
        PolyMode      polyMode;

        /** Polygon offset slope factor */
        float         offsetFactor;

        /** Polygon offset units */
        float         offsetUnits;

        /** Line width - for line-mode polygons */
        float         lineWidth;

        /** Sequence number, the order in which passes are rendered */
        int sequence;

        Pass();

        ~Pass();

        /** Set the vertex and fragment programs - throw on error */
        void setProgram( const std::string &vertex, const std::string &fragment );

        /** Get the vertex and fragment programs, compiled. 0 for fixed-function */
        int getCompiledProgram() const
        {
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
        void addTextureUnit( const std::string &source,
                             int target,
                             const std::string &deflt,
                             const std::string &paramName,
                             Technique::Pass::TextureUnit::Kind texKind );

        /** Add a constant shader param
         * @param name A string defining the shader's parameter name.
         * @param value An array of 4 floats with the parameter's value.
         * @param optional Optional parameters are allowed to be left unresolved.
         * @remarks Parameters with less than 4 values will simply ignore the other values and
         *      should be fed with any dummy value (like 0).
         */
        void addShaderParam( const std::string&name, float value[4], bool optional );

        /** Add an automatic shader param
         * @param name A string defining the shader's parameter name.
         * @param semantic The kind of value that will be automatically put in the variable.
         * @param optional Optional parameters are allowed to be left unresolved.
         */
        void addShaderParam( const std::string &name, ShaderParam::Semantic semantic, bool optional );

        /** Get the number of texture units in this pass */
        size_t getNumTextureUnits() const
        {
            return textureUnits.size();
        }

        /** Get the specified texture unit */
        const TextureUnit& getTextureUnit( int index ) const
        {
            return textureUnits[index];
        }

        /** Get the specified texture unit */
        TextureUnit& getTextureUnit( int index )
        {
            return textureUnits[index];
        }

        /** Get the number of shader params in this pass */
        size_t getNumShaderParams() const
        {
            return shaderParams.size();
        }

        /** Get the specified shader param */
        const ShaderParam& getShaderParam( int index ) const
        {
            return shaderParams[index];
        }

        /** Get the specified shader param */
        ShaderParam& getShaderParam( int index )
        {
            return shaderParams[index];
        }

        /** Compile the pass (shaders, fetch shader params, etc...) */
        void compile();

        /** Return whether the pass has been compiled or not */
        bool isCompiled() const;

        /** Return whether the pass has been compiled with a matching program version */
        bool isCompiled(int programVersion) const;
    };

protected:
    typedef std::vector< Pass > PassList;
    PassList passes;

public:
    /** Create a technique by loading and parsing the definition in [name].technique */
    explicit Technique( const std::string &name );

    /** Create a technique by copying another technique */
    explicit Technique( const Technique &src );
    
    ~Technique();

    const std::string& getName() const
    {
        return name;
    }
    const std::string& getFallback() const
    {
        return fallback;
    }
    
    /** returns true if the technique has been compiled */
    bool isCompiled() const
    {
        return compiled;
    }
    
    /** returns true if the technique has been compiled with a matching program version */
    bool isCompiled(int programVersion) const
    {
        return compiled && this->programVersion == programVersion;
    }

    void compile(); //Throws on error

    int getNumPasses() const
    {
        return passes.size();
    }
    const Pass& getPass( int idx ) const
    {
        return passes[idx];
    }
    Pass& getPass( int idx )
    {
        return passes[idx];
    }

    /** Cached technique factory function */
    static TechniquePtr getTechnique( const std::string &name );
};

#endif //__TECHNIQUE_H__INCLUDED__

