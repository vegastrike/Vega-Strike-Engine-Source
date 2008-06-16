//
// C++ Implementation: Technique
//

#include "technique.h"
#include "config.h"

#include "XMLDocument.h"
#include "VSFileXMLSerializer.h"
#include "vsfilesystem.h"
#include "gfxlib.h"
#include "aux_texture.h"

#include <exception>
#include <map>
#include <boost/smart_ptr.hpp>

#include "xml_support.h"

using namespace std;
using namespace XMLDOM;

#ifdef _MSC_VER
// Undefine those nasty MS macros - why god why!?
#undef max
#undef min
#endif

namespace __impl {

    class Exception : public std::exception
    {
    private:
        std::string _message;
        
    public:
        Exception() {};
        Exception(const Exception &other) : _message(other._message) {}
        explicit Exception(const std::string &message) : _message(message) {}
        virtual ~Exception() throw() {}
        virtual const char* what() const throw() { return _message.c_str(); }
    };

    class InvalidParameters : public Exception {
    public: 
        InvalidParameters() {}
        InvalidParameters(const string &msg) : Exception(msg) {}
    };
    
    class ProgramCompileError : public Exception {
    public: 
        ProgramCompileError() {}
        ProgramCompileError(const string &msg) : Exception(msg) {}
    };
    
    template<typename T> static T parseEnum(const string &s, const map<string, T> &enumMap)
    {
        typename map<string, T>::const_iterator it = enumMap.find(s);
        if (it != enumMap.end())
            return it->second; else
            throw InvalidParameters("Enumerated value \"" + s + "\" not recognized");
    }
    
    template<typename T> static T parseEnum(const string &s, const map<string, T> &enumMap, T deflt)
    {
        typename map<string, T>::const_iterator it = enumMap.find(s);
        if (it != enumMap.end())
            return it->second; else
            return deflt;
    }
    
    static Technique::Pass::TextureUnit::SourceType parseSourceType(const string &s, string::size_type &sep)
    {
        static map<string, Technique::Pass::TextureUnit::SourceType> enumMap;
        if (enumMap.empty()) {
            enumMap["decal"]       = Technique::Pass::TextureUnit::Decal;
            enumMap["file"]        = Technique::Pass::TextureUnit::File;
            enumMap["environment"] = Technique::Pass::TextureUnit::Environment;
            enumMap["detail"]      = Technique::Pass::TextureUnit::Detail;
        }
        return parseEnum(s.substr(0,sep = s.find_first_of(':')), enumMap, Technique::Pass::TextureUnit::None);
    }
    
    static Technique::Pass::Type parsePassType(const std::string &s)
    {
        static map<string, Technique::Pass::Type> enumMap;
        if (enumMap.empty()) {
            enumMap["fixed"]  = Technique::Pass::FixedPass;
            enumMap["shader"] = Technique::Pass::ShaderPass;
        }
        return parseEnum(s, enumMap);
    }
    
    static bool parseBool(const std::string &s)
    {
        if (s.empty())
            throw InvalidParameters("Missing required attribute");
        else
            return XMLSupport::parse_bool(s);
    }
    
    static Technique::Pass::Tristate parseTristate(const std::string &s)
    {
        static map<string, Technique::Pass::Tristate> enumMap;
        if (enumMap.empty()) {
            enumMap["true"]  = Technique::Pass::True;
            enumMap["false"] = Technique::Pass::False;
            enumMap["auto"]  = Technique::Pass::Auto;
        }
        return parseEnum(s, enumMap);
    }
    
    static Technique::Pass::BlendMode parseBlendMode(const std::string &s)
    {
        static map<string, Technique::Pass::BlendMode> enumMap;
        if (enumMap.empty()) {
            enumMap["default"]     = Technique::Pass::Default;
            enumMap["add"]         = Technique::Pass::Add;
            enumMap["multiply"]    = Technique::Pass::Multiply;
            enumMap["alpha_blend"] = Technique::Pass::AlphaBlend;
            enumMap["decal"]       = Technique::Pass::Decal;
        }
        return parseEnum(s, enumMap);
    }
    
    static Technique::Pass::Face parseFace(const std::string &s)
    {
        static map<string, Technique::Pass::Face> enumMap;
        if (enumMap.empty()) {
            enumMap["none"]   = Technique::Pass::None;
            enumMap["back"]   = Technique::Pass::Back;
            enumMap["front"]  = Technique::Pass::Front;
            enumMap["both"]   = Technique::Pass::FrontAndBack;
            enumMap["default"]= Technique::Pass::DefaultFace;
        }
        return parseEnum(s, enumMap);
    }
    
    static Technique::Pass::DepthFunction parseDepthFunction(const std::string &s)
    {
        static map<string, Technique::Pass::DepthFunction> enumMap;
        if (enumMap.empty()) {
            enumMap["less"]    = Technique::Pass::Less;
            enumMap["lequal"]  = Technique::Pass::LEqual;
            enumMap["greater"] = Technique::Pass::Greater;
            enumMap["gequal"]  = Technique::Pass::GEqual;
            enumMap["equal"]   = Technique::Pass::Equal;
            enumMap["always"]  = Technique::Pass::Always;
            enumMap["never"]   = Technique::Pass::Never;
        }
        return parseEnum(s, enumMap);
    }
    
    static Technique::Pass::PolyMode parsePolyMode(const std::string &s)
    {
        static map<string, Technique::Pass::PolyMode> enumMap;
        if (enumMap.empty()) {
            enumMap["point"]  = Technique::Pass::Point;
            enumMap["line"]   = Technique::Pass::Line;
            enumMap["fill"]   = Technique::Pass::Fill;
        }
        return parseEnum(s, enumMap);
    }
    
    static Technique::Pass::ShaderParam::Semantic parseAutoParamSemantic(const std::string &s)
    {
        static map<string, Technique::Pass::ShaderParam::Semantic> enumMap;
        if (enumMap.empty()) {
            enumMap["EnvColor"]     = Technique::Pass::ShaderParam::EnvColor;
            enumMap["CloakingPhase"]= Technique::Pass::ShaderParam::CloakingPhase;
            enumMap["Damage"]       = Technique::Pass::ShaderParam::Damage;
            enumMap["DetailPlane0"] = Technique::Pass::ShaderParam::DetailPlane0;
            enumMap["DetailPlane1"] = Technique::Pass::ShaderParam::DetailPlane1;
            enumMap["NumLights"]    = Technique::Pass::ShaderParam::NumLights;
            enumMap["ActiveLightsArray"] = Technique::Pass::ShaderParam::ActiveLightsArray;
            enumMap["GameTime"]     = Technique::Pass::ShaderParam::GameTime;
        }
        return parseEnum(s, enumMap);
    }
    
    
    static int parseIteration(const std::string &s)
    {
        static string once("once");
    
        if (s == once)
            return 0;
        else if (s.empty())
            throw InvalidParameters("Invalid iteration attribute");
        else
            return XMLSupport::parse_int(s);
    }
    
    static int parseInt(const std::string &s)
    {
        if (s.empty())
            throw InvalidParameters("Invalid integer attribute");
        else
            return XMLSupport::parse_int(s);
    }
    
    static int parseInt(const std::string &s, int deflt)
    {
        if (s.empty())
            return deflt;
        else
            return XMLSupport::parse_int(s);
    }
    
    static float parseFloat(const std::string &s)
    {
        if (s.empty())
            throw InvalidParameters("Invalid float attribute");
        else
            return XMLSupport::parse_floatf(s);
    }
    
    static void parseFloat4(const std::string &s, float value[4])
    {
        string::size_type ini = 0, end;
        int i=0;
        while (ini != string::npos) {
            value[i++] = parseFloat(s.substr(ini, end = s.find_first_of(',', ini)));
            ini = ( (end == string::npos) ? end : (end+1) );
        }
        while (i<4) {
            value[i++] = 0;
        }
    }

};

using namespace __impl;


Technique::Pass::Pass() :
    type(FixedPass),
    colorWrite(true),
    zWrite(True),
    perLightIteration(0),
    maxIterations(0),
    program(0),
    blendMode(Default),
    sequence(0),
    depthFunction(LEqual),
    cullMode(DefaultFace),
    polyMode(Fill),
    offsetFactor(0),
    offsetUnits(0),
    lineWidth(1)
{
}

Technique::Pass::~Pass()
{
    // Should deallocate the program... but... GFX doesn't have that API.
}

void Technique::Pass::setProgram(const string &vertex, const string &fragment)
{
    vertexProgram = vertex;
    fragmentProgram = fragment;
    program = 0;
}

void Technique::Pass::addTextureUnit(const string &source, int target, const string &deflt, const string &paramName)
{
    textureUnits.resize(textureUnits.size()+1);
    TextureUnit &newTU = textureUnits.back();
    
    string::size_type ssep = string::npos, dsep = string::npos;
    newTU.sourceType = parseSourceType(source, ssep);
    newTU.defaultType = parseSourceType(deflt, dsep);
    newTU.targetIndex = target;
    newTU.targetParamName = paramName;
    newTU.targetParamId = -1;
    
    switch(newTU.sourceType) {
    case TextureUnit::Decal: 
    case TextureUnit::Detail:
        if (ssep == string::npos)
            throw InvalidParameters("Decal/Detail reference missing source index");
        newTU.sourceIndex = atoi( source.c_str() + ssep + 1 ); 
        break;
    case TextureUnit::File: 
        if (ssep == string::npos)
            throw InvalidParameters("File reference missing path");
        newTU.sourcePath.assign( source, ssep + 1, string::npos ); 
        break;
    case TextureUnit::Environment: 
        break;
    default: 
        throw InvalidParameters("Missing source");
    }
    
    switch(newTU.defaultType) {
    case TextureUnit::Decal: 
    case TextureUnit::Detail:
        if (dsep == string::npos)
            throw InvalidParameters("Decal/Detail reference missing source index");
        newTU.defaultIndex = atoi( deflt.c_str() + dsep + 1 ); 
        break;
    case TextureUnit::File: 
        if (dsep == string::npos)
            throw InvalidParameters("File reference missing path");
        newTU.defaultPath.assign( deflt, dsep + 1, string::npos ); 
        break;
    }
}

void Technique::Pass::addShaderParam(const string &name, float value[4], bool optional)
{
    shaderParams.resize(shaderParams.size()+1);
    ShaderParam &newSP = shaderParams.back();
    
    newSP.name = name;
    newSP.id = -1;
    newSP.semantic = ShaderParam::Constant;
    newSP.optional = optional;
    for (int i=0; i<4; ++i)
        newSP.value[i] = value[i];
}

void Technique::Pass::addShaderParam(const string &name, ShaderParam::Semantic semantic, bool optional)
{
    shaderParams.resize(shaderParams.size()+1);
    ShaderParam &newSP = shaderParams.back();
    
    newSP.name = name;
    newSP.id = -1;
    newSP.semantic = semantic;
    newSP.optional = optional;
}

/** Compile the pass (shaders, fetch shader params, etc...) */
void Technique::Pass::compile()
{
    if (type == ShaderPass) {
        int prog = program; // BEGIN TRANSACTION
        if (prog == 0) {
            prog = GFXCreateProgram(vertexProgram.c_str(), fragmentProgram.c_str());
            if (prog == 0)
                throw ProgramCompileError(
                    "Error compiling program vp:\"" + vertexProgram + 
                    "\" fp:\"" + fragmentProgram + "\"");
        }
        for (ShaderParamList::iterator it = shaderParams.begin(); it != shaderParams.end(); ++it) {
            it->id = GFXNamedShaderConstant(prog, it->name.c_str());
            if (it->id < 0 && !it->optional)
                throw ProgramCompileError("Cannot resolve shader constant \"" + it->name + "\"");
        }
        int lastTU = -1;
        for (TextureUnitList::iterator tit = textureUnits.begin(); tit != textureUnits.end(); ++tit) {
            if (tit->sourceType == TextureUnit::File) {
                if (tit->texture.get() == 0) {
                    tit->texture.reset( new Texture(tit->sourcePath.c_str()) );
                    if (!tit->texture->LoadSuccess())
                        throw InvalidParameters("Cannot load texture file \"" + tit->sourcePath + "\"");
                }
            } else if (tit->defaultType == TextureUnit::File) {
                if (tit->texture.get() == 0) {
                    tit->texture.reset( new Texture(tit->defaultPath.c_str()) );
                    if (!tit->texture->LoadSuccess())
                        throw InvalidParameters("Cannot load texture file \"" + tit->defaultPath + "\"");
                }
            }
            if (!tit->targetParamName.empty() && tit->targetParamId < 0) {
                tit->targetParamId = GFXNamedShaderConstant(prog, tit->targetParamName.c_str());
                if (tit->targetParamId < 0) {
                    if (tit->targetIndex >= 0)
                        throw ProgramCompileError("Cannot resolve shader constant \"" + tit->targetParamName + "\"");
                } else {
                    if (tit->targetIndex < 0)
                        tit->targetIndex = lastTU + 1;
                    lastTU = std::max(tit->targetIndex, lastTU);
                }
            }
        }
        program = prog; // COMMIT ;)
    }
}

/** Return whether the pass has been compiled or not */
bool Technique::Pass::isCompiled()
{
    return (type != ShaderPass) || (program != 0);
}


Technique::Technique(const string &nam) :
    name(nam),
    compiled(false)
{
    static string passTag("pass");
    static string techniqueTag("technique");
    static string vpTag("vertex_program");
    static string fpTag("fragment_program");
    static string tuTag("texture_unit");
    static string paramTag("param");
    static string autoParamTag("auto_param");
    
    VSFileXMLSerializer serializer;
    serializer.options = 0; // only tags interest us
    serializer.initialise();
    serializer.importXML("techniques/" + name + ".technique");
    
	auto_ptr<XMLDOM::XMLDocument> doc(serializer.close());
    
    // Search for the <technique> tag
    XMLElement *techniqueNode = 0;
    {
        for (XMLElement::const_child_iterator it = doc->root.childrenBegin(); it != doc->root.childrenEnd(); ++it) {
            XMLElement *el = *it;
            if (el->type() == XMLElement::XET_TAG && el->tagName() == techniqueTag) {
                techniqueNode = el;
                break;
            }
        }
        if (techniqueNode == 0)
            throw InvalidParameters("No technique tag!");
    }
    
    fallback = techniqueNode->getAttributeValue("fallback", "");
    
    unsigned int nextSequence = 0;
    
    for (XMLElement::const_child_iterator it = techniqueNode->childrenBegin(); it != techniqueNode->childrenEnd(); ++it) {
        XMLElement *el = *it;
        
        if (el->type() == XMLElement::XET_TAG) {
            if (el->tagName() == passTag) {
                passes.resize(passes.size() + 1);
                Pass &pass = passes.back();
                
                pass.type = parsePassType( el->getAttributeValue("type", "") );
                pass.colorWrite = parseBool( el->getAttributeValue("cwrite", "true") );
                pass.zWrite = parseTristate( el->getAttributeValue("zwrite", "auto") );
                pass.perLightIteration = parseIteration( el->getAttributeValue("iteration", "once") );
                pass.maxIterations = parseInt( el->getAttributeValue("maxiterations", "0") );
                pass.blendMode = parseBlendMode( el->getAttributeValue("blend", "default") );
                pass.sequence = parseInt( el->getAttributeValue("sequence", ""), nextSequence );
                pass.depthFunction = parseDepthFunction( el->getAttributeValue("depth_function", "lequal") );
                pass.cullMode = parseFace( el->getAttributeValue("cull", "default") );
                pass.polyMode = parsePolyMode( el->getAttributeValue("polygon_mode", "fill") );
                pass.offsetUnits = parseFloat( el->getAttributeValue("polygon_offset_units", "0") );
                pass.offsetFactor = parseFloat( el->getAttributeValue("polygon_offset_factor", "0") );
                pass.lineWidth = parseFloat( el->getAttributeValue("line_width", "1") );
                nextSequence = pass.sequence + 1;
                
                string vp, fp;
                
                for (XMLElement::const_child_iterator cit = el->childrenBegin(); cit != el->childrenEnd(); ++cit) {
                    XMLElement *el = *cit;
                    if (el->type() == XMLElement::XET_TAG) {
                        if (el->tagName() == vpTag) {
                            if (!vp.empty())
                                throw InvalidParameters("Duplicate vertex program reference in technique \"" + name + "\"");
                            vp = el->getAttributeValue("src", "");
                        } else if (el->tagName() == fpTag) {
                            if (!fp.empty())
                                throw InvalidParameters("Duplicate fragment program reference in technique \"" + name + "\"");
                            fp = el->getAttributeValue("src", "");
                        } else if (el->tagName() == tuTag) {
                            int target;
                            if (pass.type == Pass::ShaderPass)
                                target = parseInt( el->getAttributeValue("target", ""), -1 ); else
                                target = parseInt( el->getAttributeValue("target", "") );
                            pass.addTextureUnit(
                                el->getAttributeValue("src", ""),
                                target,
                                el->getAttributeValue("default", ""),
                                el->getAttributeValue("name", "") );
                        } else if (el->tagName() == paramTag) {
                            float value[4];
                            parseFloat4( el->getAttributeValue("value",""), value);
                            pass.addShaderParam(
                                el->getAttributeValue("name",""), 
                                value,
                                parseBool( el->getAttributeValue("optional","false") ) );
                        } else if (el->tagName() == autoParamTag) {
                            pass.addShaderParam(
                                el->getAttributeValue("name",""),
                                parseAutoParamSemantic( el->getAttributeValue("semantic","") ),
                                parseBool( el->getAttributeValue("optional","false") ) );
                        } else {
                            // TODO: Warn about unrecognized (hence ignored) tag
                        }
                    }
                }
                
                if (pass.type == Pass::ShaderPass) {
                    if (vp.empty())
                        throw InvalidParameters("Missing vertex program reference in technique \"" + name + "\"");
                    if (fp.empty())
                        throw InvalidParameters("Missing fragment program reference in technique \"" + name + "\"");
                    
                    pass.setProgram(vp, fp);
                }
            } else {
                // TODO: Warn about unrecognized (hence ignored) tag
            }
        }
    }
}

Technique::~Technique()
{
}

void Technique::compile()
{
    if (!compiled) {
        for (PassList::iterator it = passes.begin(); it != passes.end(); ++it)
            it->compile();
        compiled = true;
    }
}

typedef map<string, TechniquePtr> TechniqueMap;
static TechniqueMap techniqueCache;

TechniquePtr Technique::getTechnique(const std::string &name)
{
    TechniqueMap::const_iterator it = techniqueCache.find(name);
    if (it != techniqueCache.end()) {
        return it->second;
    } else {
        TechniquePtr ptr(new Technique(name));
        while (!ptr->isCompiled()) {
            try {
                ptr->compile();
            } catch(ProgramCompileError e) {
                std::string fallback = ptr->getFallback();
                VSFileSystem::vs_fprintf(stderr,
                    "Compilation of technique %s failed... trying %s\n"
                    "Cause: %s\n", 
                    ptr->getName().c_str(), 
                    fallback.c_str(),
                    e.what() );
                if (!fallback.empty() && fallback != name)
                    ptr = getTechnique(fallback); else
                    break;
            }
        }
        if (ptr->isCompiled())
            techniqueCache[name] = ptr; else
            throw InvalidParameters("Could not compile any technique for \"" + name + "\"");
        return ptr;
    }
}
