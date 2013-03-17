//
//C++ Implementation: Technique
//
#include "config.h"

#include <exception>
#include <map>
#include <boost/smart_ptr.hpp>

#include "xml_support.h"
#include "technique.h"
#include "XMLDocument.h"
#include "VSFileXMLSerializer.h"
#include "vsfilesystem.h"
#include "gfxlib.h"
#include "aux_texture.h"
#include "options.h"
#include "gldrv/gl_globals.h"
#include "audio/Exceptions.h"

using namespace XMLDOM;
using std::map;
using std::auto_ptr;

#ifdef _MSC_VER
//Undefine those nasty MS macros - why god why!?
#undef max
#undef min
#endif

namespace __impl
{
//

class Exception : public std::exception
{
private:
    std::string _message;
public:
    virtual ~Exception() throw () {}
    Exception() {}
    Exception( const Exception &other ) : _message( other._message ) {}
    explicit Exception( const std::string &message ) : _message( message ) {}
    virtual const char * what() const throw ()
    {
        return _message.c_str();
    }
};

class InvalidParameters : public Exception
{
public: InvalidParameters() {}
    InvalidParameters( const string &msg ) : Exception( msg ) {}
};

class ProgramCompileError : public Exception
{
public: ProgramCompileError() {}
    ProgramCompileError( const string &msg ) : Exception( msg ) {}
};

template < typename T >
static T parseEnum( const string &s, const map< string, T > &enumMap )
{
    typename map< string, T >::const_iterator it = enumMap.find( s );
    if ( it != enumMap.end() )
        return it->second;
    else throw InvalidParameters( "Enumerated value \""+s+"\" not recognized" );
}

template < typename T >
static T parseEnum( const string &s, const map< string, T > &enumMap, T deflt )
{
    typename map< string, T >::const_iterator it = enumMap.find( s );
    if ( it != enumMap.end() )
        return it->second;
    else
        return deflt;
}

static Technique::Pass::TextureUnit::SourceType parseSourceType( const string &s, string::size_type &sep )
{
    static map< string, Technique::Pass::TextureUnit::SourceType >enumMap;
    if ( enumMap.empty() ) {
        enumMap["decal"]  = Technique::Pass::TextureUnit::Decal;
        enumMap["file"]   = Technique::Pass::TextureUnit::File;
        enumMap["environment"] = Technique::Pass::TextureUnit::Environment;
        enumMap["detail"] = Technique::Pass::TextureUnit::Detail;
    }
    return parseEnum( s.substr( 0, sep = s.find_first_of( ':' ) ), enumMap, Technique::Pass::TextureUnit::None );
}

static Technique::Pass::TextureUnit::Kind parseTexKind( const string &s )
{
    static map< string, Technique::Pass::TextureUnit::Kind >enumMap;
    if ( enumMap.empty() ) {
        enumMap["default"] = Technique::Pass::TextureUnit::TexDefault;
        enumMap["2d"] = Technique::Pass::TextureUnit::Tex2D;
        enumMap["3d"] = Technique::Pass::TextureUnit::Tex3D;
        enumMap["cube"]    = Technique::Pass::TextureUnit::TexCube;
        enumMap["separatedCube"] = Technique::Pass::TextureUnit::TexSepCube;
    }
    return parseEnum( s, enumMap, Technique::Pass::TextureUnit::TexDefault );
}

static Technique::Pass::Type parsePassType( const std::string &s )
{
    static map< string, Technique::Pass::Type >enumMap;
    if ( enumMap.empty() ) {
        enumMap["fixed"]  = Technique::Pass::FixedPass;
        enumMap["shader"] = Technique::Pass::ShaderPass;
    }
    return parseEnum( s, enumMap );
}

static bool parseBool( const std::string &s )
{
    if ( s.empty() ) throw InvalidParameters( "Missing required attribute" );
    else
        return XMLSupport::parse_bool( s );
}

static Technique::Pass::Tristate parseTristate( const std::string &s )
{
    static map< string, Technique::Pass::Tristate >enumMap;
    if ( enumMap.empty() ) {
        enumMap["true"]  = Technique::Pass::True;
        enumMap["false"] = Technique::Pass::False;
        enumMap["auto"]  = Technique::Pass::Auto;
    }
    return parseEnum( s, enumMap );
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
        enumMap["premult_alpha"]=Technique::Pass::PremultAlphaBlend;
        enumMap["multi_alpha_blend"]=Technique::Pass::MultiAlphaBlend;
    }
    return parseEnum(s, enumMap);
}

static Technique::Pass::Face parseFace( const std::string &s )
{
    static map< string, Technique::Pass::Face >enumMap;
    if ( enumMap.empty() ) {
        enumMap["none"]    = Technique::Pass::None;
        enumMap["back"]    = Technique::Pass::Back;
        enumMap["front"]   = Technique::Pass::Front;
        enumMap["both"]    = Technique::Pass::FrontAndBack;
        enumMap["default"] = Technique::Pass::DefaultFace;
    }
    return parseEnum( s, enumMap );
}

static Technique::Pass::DepthFunction parseDepthFunction( const std::string &s )
{
    static map< string, Technique::Pass::DepthFunction >enumMap;
    if ( enumMap.empty() ) {
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

static Technique::Pass::ShaderParam::Semantic parseAutoParamSemantic( const std::string &s )
{
    static map< string, Technique::Pass::ShaderParam::Semantic >enumMap;
    if ( enumMap.empty() ) {
        enumMap["EnvColor"] = Technique::Pass::ShaderParam::EnvColor;
        enumMap["CloakingPhase"]     = Technique::Pass::ShaderParam::CloakingPhase;
        enumMap["Damage"]            = Technique::Pass::ShaderParam::Damage;
        enumMap["Damage4"]           = Technique::Pass::ShaderParam::Damage4;
        enumMap["DetailPlane0"]      = Technique::Pass::ShaderParam::DetailPlane0;
        enumMap["DetailPlane1"]      = Technique::Pass::ShaderParam::DetailPlane1;
        enumMap["NumLights"]         = Technique::Pass::ShaderParam::NumLights;
        enumMap["ActiveLightsArray"] = Technique::Pass::ShaderParam::ActiveLightsArray;
        enumMap["ApparentLightSizeArray"] = 
                                       Technique::Pass::ShaderParam::ApparentLightSizeArray;
        enumMap["GameTime"]          = Technique::Pass::ShaderParam::GameTime;
    }
    return parseEnum( s, enumMap );
}

static int parseIteration( const std::string &s )
{
    static string once( "once" );
    if (s == once)
        return 0;
    else if ( s.empty() ) throw InvalidParameters( "Invalid iteration attribute" );
    else
        return XMLSupport::parse_int( s );
}

static int parseInt( const std::string &s )
{
    if ( s.empty() ) throw InvalidParameters( "Invalid integer attribute" );
    else
        return XMLSupport::parse_int( s );
}

static int parseInt( const std::string &s, int deflt )
{
    if ( s.empty() )
        return deflt;
    else
        return XMLSupport::parse_int( s );
}

static float parseFloat( const std::string &s )
{
    if ( s.empty() ) throw InvalidParameters( "Invalid float attribute" );
    else
        return XMLSupport::parse_floatf( s );
}

static void parseFloat4( const std::string &s, float value[4] )
{
    string::size_type ini = 0, end;
    int i = 0;
    while (i < 4 && ini != string::npos) {
        value[i++] = parseFloat( s.substr( ini, end = s.find_first_of( ',', ini ) ) );
        ini = ( (end == string::npos) ? end : (end+1) );
    }
    if (i >= 4 && ini != string::npos)
        VSFileSystem::vs_dprintf(1, "WARNING: invalid float4: %s\n", s.c_str());
    while (i < 4)
        value[i++] = 0;
}

//end namespace
};

using namespace __impl;

Technique::Pass::Pass()
    : program( 0 )
    , type( FixedPass )
    , colorWrite( true )
    , zWrite( True )
    , perLightIteration( 0 )
    , maxIterations( 0 )
    , blendMode( Default )
    , depthFunction( LEqual )
    , cullMode( DefaultFace )
    , polyMode( Fill )
    , offsetFactor( 0 )
    , offsetUnits( 0 )
    , lineWidth( 1 )
    , sequence( 0 )
{}

Technique::Pass::~Pass()
{
    //Should deallocate the program... but... GFX doesn't have that API.
}

void Technique::Pass::setProgram( const string &vertex, const string &fragment )
{
    vertexProgram   = vertex;
    fragmentProgram = fragment;
    program = 0;
}

void Technique::Pass::addTextureUnit( const string &source,
                                      int target,
                                      const string &deflt,
                                      const string &paramName,
                                      Technique::Pass::TextureUnit::Kind texKind )
{
    textureUnits.resize( textureUnits.size()+1 );
    TextureUnit &newTU     = textureUnits.back();

    string::size_type ssep = string::npos, dsep = string::npos;
    newTU.sourceType = parseSourceType( source, ssep );
    newTU.defaultType     = parseSourceType( deflt, dsep );
    newTU.targetIndex     = 
    newTU.origTargetIndex = target;
    newTU.targetParamName = paramName;
    newTU.targetParamId   = -1;
    newTU.texKind = texKind;
    switch (newTU.sourceType)
    {
    case TextureUnit::Decal:
    case TextureUnit::Detail:
        if (ssep == string::npos) throw InvalidParameters( "Decal/Detail reference missing source index" );
        newTU.sourceIndex = atoi( source.c_str()+ssep+1 );
        break;
    case TextureUnit::File:
        if (ssep == string::npos) throw InvalidParameters( "File reference missing path" );
        newTU.sourcePath.assign( source, ssep+1, string::npos );
        break;
    case TextureUnit::Environment:
        break;
    default: throw InvalidParameters( "Missing source" );
    }
    switch (newTU.defaultType)
    {
    case TextureUnit::Decal:
    case TextureUnit::Detail:
        if (dsep == string::npos) throw InvalidParameters( "Decal/Detail reference missing source index" );
        newTU.defaultIndex = atoi( deflt.c_str()+dsep+1 );
        break;
    case TextureUnit::File:
        if (dsep == string::npos) throw InvalidParameters( "File reference missing path" );
        newTU.defaultPath.assign( deflt, dsep+1, string::npos );
        break;
    case TextureUnit::None: //FIXME added by chuck_starchaser; please verify correctness
    case TextureUnit::Environment: //FIXME added by chuck_starchaser; please verify correctness
    default: //FIXME added by chuck_starchaser; please verify correctness
        break; //FIXME added by chuck_starchaser; please verify correctness
    }
}

void Technique::Pass::addShaderParam( const string &name, float value[4], bool optional )
{
    shaderParams.resize( shaderParams.size()+1 );
    ShaderParam &newSP = shaderParams.back();

    newSP.name     = name;
    newSP.id       = -1;
    newSP.semantic = ShaderParam::Constant;
    newSP.optional = optional;
    for (int i = 0; i < 4; ++i)
        newSP.value[i] = value[i];
}

void Technique::Pass::addShaderParam( const string &name, ShaderParam::Semantic semantic, bool optional )
{
    shaderParams.resize( shaderParams.size()+1 );
    ShaderParam &newSP = shaderParams.back();

    newSP.name     = name;
    newSP.id       = -1;
    newSP.semantic = semantic;
    newSP.optional = optional;
}

/** Compile the pass (shaders, fetch shader params, etc...) */
void Technique::Pass::compile()
{
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
                if (gl_options.ext_srgb_framebuffer)
                    defines += "#define SRGB_FRAMEBUFFER 1\n";
                else
                    defines += "#define SRGB_FRAMEBUFFER 0\n";
            }
            if (gl_options.nv_fp2)
                defines += "#define VGL_NV_fragment_program2 1\n";
            
            // Compile program
            prog = GFXCreateProgram( vertexProgram.c_str(), fragmentProgram.c_str(), 
                                     (defines.empty() ? NULL : defines.c_str()) );
            if (prog == 0) 
                throw ProgramCompileError(
                    "Error compiling program vp:\""+vertexProgram
                    +"\" fp:\""+fragmentProgram+"\"" );
            else
                VSFileSystem::vs_dprintf( 1, "Successfully compiled and linked program \"%s+%s\"\n",
                                          vertexProgram.c_str(), fragmentProgram.c_str() );
        }
        
        for (ShaderParamList::iterator it = shaderParams.begin(); it != shaderParams.end(); ++it) {
            it->id = GFXNamedShaderConstant( prog, it->name.c_str() );
            if (it->id < 0) {
                if (!it->optional) 
                    throw ProgramCompileError( "Cannot resolve shader constant \""+it->name+"\"" );
                else
                    VSFileSystem::vs_dprintf( 1, "Cannot resolve <<optional>> shader constant \"%s\" in program \"%s+%s\"\n", 
                                              it->name.c_str(), vertexProgram.c_str(), fragmentProgram.c_str() );
            }
        }
        int lastTU = -1;
        for (TextureUnitList::iterator tit = textureUnits.begin(); tit != textureUnits.end(); ++tit) {
            if (tit->sourceType == TextureUnit::File) {
                // Yep, we don't want to reload textures
                if (tit->texture.get() == 0) {
                    tit->texture.reset( new Texture( tit->sourcePath.c_str() ) );
                    if ( !tit->texture->LoadSuccess() ) throw InvalidParameters(
                            "Cannot load texture file \""+tit->sourcePath+"\"" );
                }
            } else if (tit->defaultType == TextureUnit::File) {
                // Yep, we don't want to reload textures
                if (tit->texture.get() == 0) {
                    tit->texture.reset( new Texture( tit->defaultPath.c_str() ) );
                    if ( !tit->texture->LoadSuccess() ) throw InvalidParameters(
                            "Cannot load texture file \""+tit->defaultPath+"\"" );
                }
            }
            if (!tit->targetParamName.empty()) {
                tit->targetParamId = GFXNamedShaderConstant( prog, tit->targetParamName.c_str() );
                if (tit->targetParamId < 0) {
                    if (tit->origTargetIndex >= 0)
                        throw ProgramCompileError(
                            "Cannot resolve shader constant \""+tit->targetParamName+"\"" );
                    else
                        tit->targetIndex = -1;
                } else {
                    if (tit->origTargetIndex < 0)
                        tit->targetIndex = lastTU+1;
                    lastTU = std::max( tit->targetIndex, lastTU );
                }
            }
        }
        
        // COMMIT ;-)
        program = prog;         
        programVersion = GFXGetProgramVersion();
    }
}

/** Return whether the pass has been compiled or not */
bool Technique::Pass::isCompiled() const
{
    return (type != ShaderPass) || (program != 0);
}

/** Return whether the pass has been compiled or not with a matching program version */
bool Technique::Pass::isCompiled(int programVersion) const
{
    return (type != ShaderPass) || (program != 0 && this->programVersion == programVersion);
}

Technique::Technique( const string &nam ) :
    name( nam )
    , compiled( false )
    , programVersion( 0 )
{
    static string passTag( "pass" );
    static string techniqueTag( "technique" );
    static string vpTag( "vertex_program" );
    static string fpTag( "fragment_program" );
    static string tuTag( "texture_unit" );
    static string paramTag( "param" );
    static string autoParamTag( "auto_param" );

    VSFileXMLSerializer serializer;
    serializer.options = 0;     //only tags interest us
    serializer.initialise();
    
    try {
        // Try a specialized version
        serializer.importXML(
            game_options.techniquesBasePath+"/"
            +game_options.techniquesSubPath+"/"
            +name+".technique" );
    } catch(Audio::FileOpenException e) {
        VSFileSystem::vs_dprintf(1, "Cannot find specialized technique, trying generic: %s\n", e.what());
        // Else try a default
        serializer.importXML(
            game_options.techniquesBasePath+"/"
            +name+".technique" );
    }

    auto_ptr< XMLDOM::XMLDocument >doc( serializer.close() );

    //Search for the <technique> tag
    XMLElement *techniqueNode = 0;
    {
        for (XMLElement::const_child_iterator it = doc->root.childrenBegin(); it != doc->root.childrenEnd(); ++it) {
            XMLElement *el = *it;
            if (el->type() == XMLElement::XET_TAG && el->tagName() == techniqueTag) {
                techniqueNode = el;
                break;
            }
        }
        if (techniqueNode == 0) throw InvalidParameters( "No technique tag!" );
    }

    fallback = techniqueNode->getAttributeValue( "fallback", "" );

    unsigned int nextSequence = 0;
    for (XMLElement::const_child_iterator it = techniqueNode->childrenBegin(); it != techniqueNode->childrenEnd(); ++it) {
        XMLElement *el = *it;
        if (el->type() == XMLElement::XET_TAG) {
            if (el->tagName() == passTag) {
                passes.resize( passes.size()+1 );
                Pass &pass = passes.back();

                pass.type              = parsePassType( el->getAttributeValue( "type", "" ) );
                pass.colorWrite        = parseBool( el->getAttributeValue( "cwrite", "true" ) );
                pass.zWrite            = parseTristate( el->getAttributeValue( "zwrite", "auto" ) );
                pass.perLightIteration = parseIteration( el->getAttributeValue( "iteration", "once" ) );
                pass.maxIterations     = parseInt( el->getAttributeValue( "maxiterations", "0" ) );
                pass.blendMode         = parseBlendMode( el->getAttributeValue( "blend", "default" ) );
                pass.sequence          = parseInt( el->getAttributeValue( "sequence", "" ), nextSequence );
                pass.depthFunction     = parseDepthFunction( el->getAttributeValue( "depth_function", "lequal" ) );
                pass.cullMode          = parseFace( el->getAttributeValue( "cull", "default" ) );
                pass.polyMode          = parsePolyMode( el->getAttributeValue( "polygon_mode", "fill" ) );
                pass.offsetUnits       = parseFloat( el->getAttributeValue( "polygon_offset_units", "0" ) );
                pass.offsetFactor      = parseFloat( el->getAttributeValue( "polygon_offset_factor", "0" ) );
                pass.lineWidth         = parseFloat( el->getAttributeValue( "line_width", "1" ) );
                pass.sRGBAware         = parseBool( el->getAttributeValue( "srgb_aware", "false" ) );
                nextSequence           = pass.sequence+1;

                string vp, fp;
                for (XMLElement::const_child_iterator cit = el->childrenBegin(); cit != el->childrenEnd(); ++cit) {
                    XMLElement *el = *cit;
                    if (el->type() == XMLElement::XET_TAG) {
                        if (el->tagName() == vpTag) {
                            if ( !vp.empty() ) throw InvalidParameters(
                                    "Duplicate vertex program reference in technique \""+name+"\"" );
                            vp = el->getAttributeValue( "src", "" );
                        } else if (el->tagName() == fpTag) {
                            if ( !fp.empty() ) throw InvalidParameters(
                                    "Duplicate fragment program reference in technique \""+name+"\"" );
                            fp = el->getAttributeValue( "src", "" );
                        } else if (el->tagName() == tuTag) {
                            int target;
                            if (pass.type == Pass::ShaderPass)
                                target = parseInt( el->getAttributeValue( "target", "" ), -1 );
                            else
                                target = parseInt( el->getAttributeValue( "target", "" ) );
                            pass.addTextureUnit(
                                el->getAttributeValue( "src", "" ),
                                target,
                                el->getAttributeValue( "default", "" ),
                                el->getAttributeValue( "name", "" ),
                                parseTexKind( el->getAttributeValue( "kind", "" ) ) );
                            VSFileSystem::vs_dprintf(2, "Added texture unit #%d \"%s\"\n",
                                                     pass.getNumTextureUnits(),
                                                     el->getAttributeValue( "name","" ).c_str());
                        } else if (el->tagName() == paramTag) {
                            float value[4];
                            parseFloat4( el->getAttributeValue( "value", "" ), value );
                            pass.addShaderParam(
                                el->getAttributeValue( "name", "" ),
                                value,
                                parseBool( el->getAttributeValue( "optional", "false" ) ) );
                            VSFileSystem::vs_dprintf(2, "Added constant #%d \"%s\" with value (%.2f,%.2f,%.2f,%.2f) as %s\n",
                                                     pass.getNumShaderParams(),
                                                     el->getAttributeValue( "name","" ).c_str(),
                                                     value[0], value[1], value[2], value[3],
                                                     (parseBool( el->getAttributeValue( "optional", "false" ) ) ? "optional" : "required"));
                        } else if (el->tagName() == autoParamTag) {
                            pass.addShaderParam(
                                el->getAttributeValue( "name", "" ),
                                parseAutoParamSemantic( el->getAttributeValue( "semantic", "" ) ),
                                parseBool( el->getAttributeValue( "optional", "false" ) ) );
                            VSFileSystem::vs_dprintf(2, "Added param #%d \"%s\" with semantic %s as %s\n",
                                                     pass.getNumShaderParams(),
                                                     el->getAttributeValue( "name","" ).c_str(),
                                                     el->getAttributeValue( "semantic", "" ).c_str(),
                                                     (parseBool( el->getAttributeValue( "optional", "false" ) ) ? "optional" : "required"));
                        } else {
                            //TODO: Warn about unrecognized (hence ignored) tag
                        }
                    }
                }
                if (pass.type == Pass::ShaderPass) {
                    if ( vp.empty() )
                        throw InvalidParameters( "Missing vertex program reference in technique \""+name+"\"" );
                    if ( fp.empty() )
                        throw InvalidParameters( "Missing fragment program reference in technique \""+name+"\"" );
                    pass.setProgram( vp, fp );
                }
            } else {
                //TODO: Warn about unrecognized (hence ignored) tag
            }
        }
    }
}

Technique::Technique( const Technique &src ) :
    name( src.name )
    , fallback( src.fallback )
    , compiled( false )
    , programVersion( 0 )
    , passes( src.passes )
{
    // Not much else to do
    // Compiled techniques are still valid, and setting any parameter
    // that would invalidate it would result in recompilation
    // FIXME: should result in recompilation, not necessarily true now
}

Technique::~Technique()
{
}

void Technique::compile()
{
    if (!compiled || (GFXGetProgramVersion() != programVersion)) {
        for (PassList::iterator it = passes.begin(); it != passes.end(); ++it)
            it->compile();
        compiled = true;
        programVersion = GFXGetProgramVersion();
    }
}

typedef map< string, TechniquePtr >TechniqueMap;
static TechniqueMap techniqueCache;

TechniquePtr Technique::getTechnique( const std::string &name )
{
    TechniqueMap::const_iterator it = techniqueCache.find( name );
    if ( it != techniqueCache.end() ) {
        return it->second;
    } else {
        TechniquePtr ptr( new Technique( name ) );
        while ( !ptr->isCompiled() ) {
            try {
                ptr->compile();
                VSFileSystem::vs_fprintf( stdout,
                                         "Compilation of technique %s successful\n",
                                         ptr->getName().c_str() );
            }
            catch (ProgramCompileError e) {
                std::string fallback = ptr->getFallback();
                VSFileSystem::vs_fprintf( stderr,
                                         "Compilation of technique %s failed... trying %s\n"
                                         "Cause: %s\n",
                                         ptr->getName().c_str(),
                                         fallback.c_str(),
                                         e.what() );
                if (!fallback.empty() && fallback != name)
                    ptr = getTechnique( fallback );
                else
                    break;
            }
        }
        if ( ptr->isCompiled() )
            techniqueCache[name] = ptr;
        else
            throw InvalidParameters( "Could not compile any technique for \""+name+"\"" );
        return ptr;
    }
}

