#ifndef __TECHNIQUE_H__INCLUDED__
#define __TECHNIQUE_H__INCLUDED__

#include <string>
#include <vector>

#include <boost/smart_ptr.hpp>

class Texture;
class Technique;

typedef boost::shared_ptr<Technique> TechniquePtr;

class Technique
{
    std::string name;
    std::string fallback;
    bool compiled;

public:
    class Pass {
        /** Compiled and linked program */
        int program;
        
        /** Vertex Program file before compilation */
        std::string vertexProgram;
        
        /** Fragment Program file before compilation */
        std::string fragmentProgram;
        
    public:
        enum Tristate {
            False=0,
            True=1,
            Auto=2
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
            Decal
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
            
            SourceType sourceType;
            SourceType defaultType;
            
            int sourceIndex;
            int defaultIndex;
            int targetIndex;
            
            /** For shader passes, the associated sampler uniform */
            std::string targetParamName;
            int targetParamId;
            
            std::string sourcePath;
            std::string defaultPath;

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
                Constant
            };
        
            std::string name;
            int id;
            Semantic semantic;
            bool optional;
            float value[4];
        };
    
    private:
        typedef std::vector<TextureUnit> TextureUnitList;
        typedef std::vector<ShaderParam> ShaderParamList;
        
        TextureUnitList textureUnits;
        ShaderParamList shaderParams;
    
    public:
        /** Type of pass - shader or fixed pipeline */
        Type type;
        
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
        
        /** Set the vertex and fragment programs - throw on error */
        void setProgram(const std::string &vertex, const std::string &fragment);
        
        /** Get the vertex and fragment programs, compiled. 0 for fixed-function */
        int getCompiledProgram() const { return program; }
        
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
        void addTextureUnit(const std::string &source, int target, const std::string &deflt, const std::string &paramName);
        
        /** Add a constant shader param
         * @param name A string defining the shader's parameter name.
         * @param value An array of 4 floats with the parameter's value.
         * @param optional Optional parameters are allowed to be left unresolved.
         * @remarks Parameters with less than 4 values will simply ignore the other values and
         *      should be fed with any dummy value (like 0).
         */
        void addShaderParam(const std::string &name, float value[4], bool optional);
        
        /** Add an automatic shader param
         * @param name A string defining the shader's parameter name.
         * @param semantic The kind of value that will be automatically put in the variable.
         * @param optional Optional parameters are allowed to be left unresolved.
         */
        void addShaderParam(const std::string &name, ShaderParam::Semantic semantic, bool optional);
        
        /** Get the number of texture units in this pass */
        int getNumTextureUnits() const { return textureUnits.size(); }
        
        /** Get the specified texture unit */
        const TextureUnit& getTextureUnit(int index) const { return textureUnits[index]; }
        
        /** Get the number of shader params in this pass */
        int getNumShaderParams() const { return shaderParams.size(); }
        
        /** Get the specified shader param */
        const ShaderParam& getShaderParam(int index) const { return shaderParams[index]; }
        
        /** Compile the pass (shaders, fetch shader params, etc...) */
        void compile();
        
        /** Return whether the pass has been compiled or not */
        bool isCompiled();
    };
    
protected:
    typedef std::vector<Pass> PassList;
    PassList passes;
    
public:
    /** Create a technique by loading and parsing the definition in [name].technique */
    Technique(const std::string &name);
    
    ~Technique();

    const std::string& getName() const { return name; }
    const std::string& getFallback() const { return fallback; }
    bool isCompiled() const { return compiled; }
    
    void compile(); // Throws on error
    
    int getNumPasses() const { return passes.size(); }
    const Pass& getPass(int idx) const { return passes[idx]; }
    
    /** Cached technique factory function */
    static TechniquePtr getTechnique(const std::string &name);
};

#endif//__TECHNIQUE_H__INCLUDED__
