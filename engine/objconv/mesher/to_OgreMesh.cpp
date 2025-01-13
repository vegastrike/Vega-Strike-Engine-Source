/*
 * to_OgreMesh.cpp
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


#ifdef HAVE_OGRE

#include <Ogre.h>
#include "to_OgreMesh.h"

#include "OgreDefaultHardwareBufferManager.h"


#include "xml_support.h"

#include <limits>
#include <vector>

using namespace std;

// Ogre >= 1.7.0 conflicts with the std namespace, in a very nasty way
// They define the standard collection types to be different stuff:
//      a struct containing just a typedef. It's a pain to work with.
// So we can't "using namespace Ogre"

using Ogre::MeshPtr;
using Ogre::Vector3;
using Ogre::Real;
using Ogre::String;

//Crappy globals
//NB some of these are not directly used, but are required to
//instantiate the singletons used in the dlls
static Ogre::LogManager *logMgr;
static Ogre::Math *mth;
static Ogre::MaterialManager *matMgr;
static Ogre::SkeletonManager *skelMgr;
static Ogre::MeshSerializer *meshSerializer;
static Ogre::SkeletonSerializer   *skeletonSerializer;
static Ogre::DefaultHardwareBufferManager *bufferManager;
static Ogre::MeshManager    *meshMgr;
static Ogre::ResourceGroupManager *rgm;
static string templatePath;

#if (OGRE_VERSION >= 0x010700)
static Ogre::LodStrategyManager *lodMgr;
#endif

static string LoadString( const string &filename )
{
    ifstream fi( filename.c_str(), std::ios_base::in );
    if ( !fi.fail() ) {
        string ret;

        char   buf[512];
        while ( !fi.eof() )
            ret += string( buf, ( fi.read( buf, sizeof (buf)-1 ) ).gcount() );
        fi.close();

        return ret;
    } else {return string( "" ); }}

//Replaces $(var=default) with the value in variables, or the default (if present) - deletes the reference, if not present.
//Performs recursive replacements - that's useful.
//TODO: Make it process #{ifdef()}, #{else} and #{endif} tags, and make it replace $$ with $, and ## with # (and ignore the resulting character, to allow output of tags themselves).
static void replaceVars( string &text, map< string, string >variables )
{
    //Replace variables
    string::size_type where, when, whip, ip = 0;
    while ( ( where = text.find( "$(", ip ) ) != string::npos ) {
        if ( ( when = text.find( ')', where+2 ) ) != string::npos ) {
            whip = text.find( '=', where+2 );
            string var = (whip != string::npos && whip < when) ? text.substr( where+2, whip-where-2 ) : text.substr(
                where+2,
                when
                -where-2 );
            map< string, string >::const_iterator it = variables.find( var );
            if ( it != variables.end() )
                text.replace( where, when-where+1, it->second );

            else if (whip != string::npos && whip < when)
                text.replace( where, when-where+1, text.substr( whip+1, when-whip-1 ) );

            else
                text.erase( where, when-where+1 );
            //this enables recursive replacements - yay...
            //but no provisions for detecting infinite loops - watch out!
            ip = where;
        } else {break; }}
}

//Removes spurious whitespace, common leftover from variable replacement.
//They're identified by non-empty lines, with only whitespace (' ', \t', '\r', '\n')
static void cleanup( string &text )
{
    for (string::size_type i = 1; i < text.length(); ++i)
        if (text[i-1] == '\n') {
            char c;
            bool spurious = true;
            string::size_type j = i+1;
            while ( spurious && j < text.length() && (c = text[j]) != '\n' && (c == ' ' || c == '\t' || c == '\r') )
                j++, spurious = spurious && (c == ' ' || c == '\t');
            if ( (j < text.length()-1) && (j > i+1) && spurious && c == '\n' ) {
                text.erase( i, j-i+1 );
                i--;
            }
        }
}

static string getID()
{
    static int idnum = 0;
    return XMLSupport::tostring( idnum++ );
}

static string getMatID()
{
    static int idnum = 0;
    return XMLSupport::tostring( idnum++ );
}

static string getMaterialHash( const GFXMaterial &mat,
                               bool reflect,
                               bool lighting,
                               bool cullface,
                               float polygon_offset,
                               int blend_src,
                               int blend_dst,
                               float alpha_test )
{
    static const char hexdigs[] = "0123456789ABCDEF";
    static char tmp[16];
    string ret;
    for (size_t i = 0; i < sizeof (mat); i++) {
        ret += hexdigs[( ( (char*) &mat )[i]>>4 )];
        ret += hexdigs[( ( (char*) &mat )[i]&0xF )];
    }
    ret += (reflect ? '1' : '0');
    ret += (lighting ? '1' : '0');
    ret += (cullface ? '1' : '0');
    sprintf( tmp, "%06X", (unsigned int) blend_src );
    ret += tmp;
    sprintf( tmp, "%06X", (unsigned int) blend_dst );
    ret += tmp;
    sprintf( tmp, "%06X", *(unsigned int*) &polygon_offset );
    ret += tmp;
    sprintf( tmp, "%06X", *(unsigned int*) &alpha_test );
    ret += tmp;

    return ret;
}

static string getMaterialHash( const Mesh_vec3f &v )
{
    static const char hexdigs[] = "0123456789ABCDEF";
    static char tmp[16];
    string ret;
    for (size_t i = 0; i < sizeof (v); i++) {
        ret += hexdigs[( ( (char*) &v )[i]>>4 )];
        ret += hexdigs[( ( (char*) &v )[i]&0xF )];
    }
    return ret;
}

static string& operator+=( string &dst, const vector< unsigned char > &ucarr )
{
    for (vector< unsigned char >::const_iterator it = ucarr.begin(); it != ucarr.end(); it++)
        dst += *it;
    return dst;
}

static string tostring( const vector< unsigned char > &ucarr )
{
    string ret;
    ret += ucarr;
    return ret;
}

static string getMaterialHash( const XML &memfile )
{
    string hash;
    char   tmp[64];
    /* Moved out of for because used for both for loops. */
    vector< textureholder >::size_type i;
    hash += getMaterialHash( memfile.material,
                             memfile.reflect,
                             memfile.lighting,
                             memfile.cullface,
                             memfile.polygon_offset,
                             memfile.blend_src,
                             memfile.blend_dst,
                             memfile.alphatest );
    hash += "-";

    sprintf( tmp, "-%d-", memfile.textures.size() );
    hash += tmp;
    for (i = 0; i < memfile.textures.size(); i++) {
        sprintf( tmp, "%d,%d,", memfile.textures[i].index, memfile.textures[i].type );
        hash += tmp;
        hash += memfile.textures[i].name;
        hash += '#';
    }
    sprintf( tmp, "-%d-", memfile.detailplanes.size() );
    hash += tmp;
    sprintf( tmp, "%d,%d,", memfile.detailtexture.index, memfile.detailtexture.type );
    hash += tmp;
    hash += memfile.detailtexture.name;
    hash += '-';
    for (i = 0; i < memfile.detailplanes.size(); i++)
        hash += getMaterialHash( memfile.detailplanes[i] );
    return hash;
}

struct outputContext
{
    MeshPtr top;
    string  name;    //name - try to make this unique, as it will be used to make material names unique.
    string  basepath;    //base path to be appended to non-builtin textures
    int     cur_lod_level; //initialize to 0 (no lods yet)

    map< string, pair< string, string > >materials;     //maps a material hash (use getMaterialHash()) to an Ogre Material Name/Script.
    set< string >predefined_materials;     //materials that are defined somewhere (as in the .material script already, or in another material scripts) - not to be defined again

    vector< GFXVertex >sharedvert;
    bool    sv_usetex;
    bool    sv_usenorm;

    bool    appending;

    Vector3 min, max;
    Real    maxsqr;
};

namespace OgreMeshConverter
{
void ConverterInit()
{
    logMgr  = new Ogre::LogManager();
    logMgr->createLog( "mesher.log" );

#if (OGRE_VERSION >= 0x010700)
    lodMgr  = new Ogre::LodStrategyManager();
#endif

    rgm     = new Ogre::ResourceGroupManager();
    mth     = new Ogre::Math();
    meshMgr = new Ogre::MeshManager();
    matMgr  = new Ogre::MaterialManager();
    matMgr->initialise();
    skelMgr = new Ogre::SkeletonManager();
    meshSerializer     = new Ogre::MeshSerializer();
    skeletonSerializer = new Ogre::SkeletonSerializer();
    bufferManager = new Ogre::DefaultHardwareBufferManager();     //needed because we don't have a rendersystem
}

void ConverterClose()
{
    delete skeletonSerializer;
    skeletonSerializer = 0;
    delete meshSerializer;
    meshSerializer     = 0;
    delete skelMgr;
    skelMgr = 0;
    delete matMgr;
    matMgr  = 0;
    delete meshMgr;
    meshMgr = 0;
    delete bufferManager;
    bufferManager = 0;
    delete mth;
    mth    = 0;
    delete rgm;
    rgm    = 0;
    delete lodMgr;
    lodMgr = 0;
    delete logMgr;
    logMgr = 0;
}

void * Init()
{
    struct outputContext *ctxt = new struct outputContext;
    ctxt->name = "export-mesh-"+getID();
    ctxt->top  = Ogre::MeshManager::getSingleton().createManual( ctxt->name,
                                                                 Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    ctxt->cur_lod_level = 0;
    ctxt->sv_usenorm    = ctxt->sv_usetex = false;
    ctxt->min = ctxt->max = Vector3( 0, 0, 0 );
    ctxt->maxsqr = 0;
    ctxt->appending     = false;
    return ctxt;
}

void * Init( const char *inputfile, const char *matfile )
{
    struct outputContext *ctxt = new struct outputContext;

    //Load the materials
    {
        //The following would be preferrable, except that it needs all programs accesible
        //by mesher, which will not be the case.
        //So... lets hope material format doesn't change drastically...
        /*
         *  std::ifstream fi;
         *  fi.open(matfile, std::ios_base::in);
         *  DataStreamPtr stream(new FileStreamDataStream(String(matfile), &fi, false)); // Do not free fi
         *
         *  MaterialManager::getSingleton().parseScript(stream,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
         *  ResourceManager::ResourceMapIterator iter = MaterialManager::getSingleton().getResourceIterator();
         *  while (iter.hasMoreElements()) {
         *   ResourcePtr p=iter.getNext();
         *   ctxt->predefined_materials.insert(p->getName());
         *  }
         */

        //Not the preferrable way... since it depends on the script's syntax (which Ogre
        //guys could decide to change), but at least won't fail on missing dependencies.
        int  brak = 0;
        bool mat  = false;
        std::ifstream fi;
        fi.open( matfile, std::ios_base::in );
        if ( !fi.fail() )
            while ( !fi.eof() ) {
                std::string tok;
                fi>>tok;
                if (brak == 0 && tok == "material") {
                    mat = true;
                } else if (tok == "{") {
                    brak++;
                } else if (tok == "}") {
                    brak--;
                } else if (tok.find( "//" ) != std::string::npos) {
                    while (fi.get() != '\n')                      /* do nothing */
                        ;
                } else if (mat) {
                    ctxt->predefined_materials.insert( tok );
                    mat = false;
                } else {
                    mat = false;
                }
            }
    }

    //Load the mesh
    {
        std::ifstream fi;
        fi.open( inputfile, std::ios_base::in|std::ios_base::binary );
        Ogre::DataStreamPtr stream( new Ogre::FileStreamDataStream( String( inputfile ), &fi, false ) );         //Do not free fi

        ctxt->name = "export-mesh-"+getID();
        ctxt->top  = Ogre::MeshManager::getSingleton().createManual( ctxt->name, 
                                                                     Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        meshSerializer->importMesh( stream, ctxt->top.getPointer() );

        ctxt->cur_lod_level = ctxt->top->getNumLodLevels();
        ctxt->sv_usenorm    = ctxt->sv_usetex = false;
        ctxt->min    = ctxt->max = Vector3( 0, 0, 0 );
        ctxt->maxsqr = 0;

        //Must do this... or things won't work...
        ctxt->top->removeLodLevels();
    }

    ctxt->appending = true;

    return ctxt;
}

void SetTemplatePath( string path )
{
    templatePath = path;
}

static const map< string, string >& LoadTemplates()
{
    static map< string, string >tpl;
    static bool tpl_init = false;
    if (tpl_init) return tpl;
    tpl_init = true;

    //We need the following templates:
    static const char *tpl_fnames[] = {
        "unlit",                //no nothing - just colored polygons (used sometimes)
        "unlit+env",            //no nothing - except reflection (used at times)
        "basic",                //no textures
        "basic+env",            //no textures, but with reflection (used sometimes)
        "dif",                  //only diffuse texture
        "dif+glow",             //diffuse and glowmap
        "dif+ppl",              //diffuse with specmap
        "dif+env",              //diffuse with reflection
        "dif+ppl+env",          //diffuse with specmap and reflection
        "dif+glow+ppl",         //everything but reflection (try to support damagemaps)
        "dif+glow+ppl+env"         //everything (try to support damagemaps)
    };
    for (unsigned int i = 0; i < sizeof (tpl_fnames)/sizeof (tpl_fnames[0]); i++)
        tpl.insert( pair< string, string > ( string( tpl_fnames[i] ),
                                            LoadString( templatePath+"/"+string( tpl_fnames[i] )+".materialTemplate" ) ) );
    return tpl;
}

static string GetMaterialCategory( const XML &xml, int tex[] )
{
    if (!xml.lighting && !xml.reflect)
        return string( "unlit" );
    else if (!xml.lighting && xml.reflect)
        return string( "unlit+env" );
    else if ( ( (tex[0] == -1) && (tex[1] == -1) && (tex[2] == -1) && (tex[3] == -1) ) && !xml.reflect )
        return string( "basic" );
    else if ( ( (tex[0] == -1) && (tex[1] == -1) && (tex[2] == -1) && (tex[3] == -1) ) && xml.reflect )
        return string( "basic+env" );
    else if ( ( (tex[0] != -1) && (tex[1] == -1) && (tex[2] == -1) && (tex[3] == -1) ) && !xml.reflect )
        return string( "dif" );
    else if ( ( (tex[0] != -1) && (tex[1] == -1) && (tex[2] == -1) && (tex[3] != -1) ) && !xml.reflect )
        return string( "dif+glow" );
    else if ( ( (tex[0] != -1) && (tex[1] != -1) && (tex[2] == -1) && (tex[3] == -1) ) && !xml.reflect )
        return string( "dif+ppl" );
    else if ( ( (tex[0] != -1) && (tex[1] == -1) && (tex[2] == -1) && (tex[3] == -1) ) && xml.reflect )
        return string( "dif+env" );
    else if ( ( (tex[0] != -1) && (tex[1] != -1) && (tex[2] == -1) && (tex[3] == -1) ) && !xml.reflect )
        return string( "dif+ppl+env" );
    else if (xml.reflect)
        return string( "dif+glow+ppl+env" );
    else
        return string( "dif+glow+ppl" );
}

static void SetupColorTplVars( map< string, string > &vars,
                               string mainvar,
                               string attributename,
                               string rvar,
                               string gvar,
                               string bvar,
                               string avar,
                               float r,
                               float g,
                               float b,
                               float a,
                               float defr,
                               float defg,
                               float defb,
                               float defa,
                               bool force_rgba = false )
{
    bool   eqr   = (fabs( defr-r ) < 0.001);
    bool   eqg   = (fabs( defg-g ) < 0.001);
    bool   eqb   = (fabs( defb-b ) < 0.001);
    bool   eqa   = (fabs( defa-a ) < 0.001);
    bool   eqrgb = eqr && eqb && eqa;

    string rgbs  = "$("+rvar+") $("+gvar+") $("+bvar+")";
    string rgbas = "$("+rvar+") $("+gvar+") $("+bvar+") $("+avar+")";

    vars[mainvar] =
        ( (eqa && !(eqrgb)
           && !force_rgba) ? attributename+string( " "+rgbs ) : ( (eqrgb && eqa) ? "" : attributename+string( " "+rgbas ) ) );
    vars[rvar]    = XMLSupport::tostring( r );
    vars[gvar]    = XMLSupport::tostring( g );
    vars[bvar]    = XMLSupport::tostring( b );
    vars[avar]    = XMLSupport::tostring( a );
}

static void SetupBlendTplVars( map< string, string > &vars, int src, int dst )
{
    string blend, srcblend = "one", dstblend = "zero";

    const string blendnames[] = {
        "",
        "zero",
        "one",
        "src_color",
        "one_minus_src_color",
        "src_alpha",
        "one_minus_src_alpha",
        "dst_alpha",
        "one_minus_dst_alpha",
        "dst_color",
        "one_minus_dst_color",
        "?",                  "?", "?","?", "?"
    };
    if ( (src == ONE) && (dst == ZERO) )
        blend = "";          //default - opaque (why specify?)
    else if ( (src == ONE) && (dst == ONE) )
        blend = "scene_blend add";
    else if ( (src == DESTCOLOR) && (dst == ZERO) )
        blend = "scene_blend modulate";
    else if ( (src == SRCCOLOR) && (dst == INVSRCCOLOR) )
        blend = "scene_blend colour_blend";
    else if ( (src == SRCALPHA) && (dst == INVSRCALPHA) )
        blend = "scene_blend alpha_blend";
    else
        blend = "scene_blend $(SCENE_BLEND_SRC) $(SCENE_BLEND_DST=zero)";
    if (   (src < 0)
        || (dst < 0)
        || ( size_t(src) > sizeof(blendnames)/sizeof(blendnames[0]) )
        || ( size_t(dst) > sizeof(blendnames)/sizeof(blendnames[0]) )
        || ( (srcblend = blendnames[src]) == "?" )
        || ( (dstblend = blendnames[dst]) == "?" ) ) {
        //Oops... must use extended blending
        //let's fallback to one zero
        srcblend = "one";
        dstblend = "zero";
    }
    vars["SCENE_BLEND"]     = blend;
    vars["SCENE_BLEND_SRC"] = srcblend;
    vars["SCENE_BLEND_DST"] = dstblend;
}

void ReplaceInvalidMatnameChars( string &matname )
{
    static const string invalid_chars( ":\t\r\n{} " );
    for (string::size_type pos = 0, len = matname.length(); pos < len; ++pos)
        if (invalid_chars.find( matname[pos] ) != string::npos)
            matname[pos] = '_';
}

//Returns material name
static string AddMaterial( void *outputcontext, const XML &xml )
{
    static const map< string, string > &templates = LoadTemplates();

    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    string mathash = getMaterialHash( xml );

    map< string, pair< string, string > >::iterator mit = ctxt->materials.find( mathash );
    if ( mit == ctxt->materials.end() ) {
        //Sort things out a bit...
        int tex[4] = {-1, -1, -1, -1};
        {
            for (vector< textureholder >::size_type i = 0; i < xml.textures.size(); i++)
                if (   (xml.textures[i].index >= 0)
                    && (size_t(xml.textures[i].index) < ( sizeof(tex)/sizeof(tex[0]) ))
                    && (xml.textures[i].type != UNKNOWN)
                    && (xml.textures[i].name.size() > 0) )
                    tex[xml.textures[i].index] = int(i);
        }

        //Kay... let's load the template...
        string tpl, tplnam;
        map< string, string >::const_iterator tplit = templates.find( GetMaterialCategory( xml, tex ) );
        if ( ( tplit != templates.end() && !tplit->second.empty() )         //First, the appropriate one
            || ( ( tplit = templates.find( "dif+glow+ppl+env" ) ) != templates.end() && !tplit->second.empty() )          //Next, try the �bermaterial
            || ( ( tplit = templates.find( "basic" ) ) != templates.end() && !tplit->second.empty() ) ) {
            //Finally, go to basics
            tpl    = tplit->second;
            tplnam = tplit->first;
        } else {
            //If all else fails, built-in basic
            tpl =
                "material $(MATERIAL_NAME)\n"
                "{\n"
                "\ttechnique\n"
                "\t{\n"
                "\t\tpass\n"
                "\t\t{\n"
                "\t\t\t$(AMBIENT)\n"
                "\t\t\t$(DIFFUSE)\n"
                "\t\t\t$(SPECULAR)\n"
                "\t\t\t$(EMISSIVE)\n"
                "\n"
                "\t\t\ttexture_unit\n"
                "\t\t\t{\n"
                "\t\t\t\ttexture $(TEXTURE_DIF_FILE=white.png) $(TEXTURE_DIF_TYPE=2d)\n"
                "\t\t\t}\n"
                "\t\t}\n"
                "\t}\n"
                "}\n\n";
            tplnam = "error";
        }
        string matname;
        do {
            matname = "mesher-auto-material("+tplnam+"?"+ctxt->basepath+ctxt->name+"#"+getMatID()+")";
            ReplaceInvalidMatnameChars( matname );
        } while (ctxt->predefined_materials.count( matname ) > 0);
        //Hm... setup the variable map... ugly...
        map< string, string >vars;
        vars["MATERIAL_NAME"] = matname;

        SetupColorTplVars( vars, "AMBIENT", "ambient", "AR", "AG", "AB", "AA",
                           xml.material.ar, xml.material.ag, xml.material.ab, xml.material.aa,
                           -1.f, -1.f, -1.f, 1.f );         //-1.0 so that it's never omitted
        SetupColorTplVars( vars, "AMBIENT_H", "ambient", "AR_H", "AG_H", "AB_H", "AA",
                           xml.material.ar/2, xml.material.ag/2, xml.material.ab/2, xml.material.aa,
                           -1.f, -1.f, -1.f, 1.f );         //-1.0 so that it's never omitted
        SetupColorTplVars( vars, "AMBIENT_Q", "ambient", "AR_Q", "AG_Q", "AB_Q", "AA",
                           xml.material.ar/4, xml.material.ag/4, xml.material.ab/4, xml.material.aa,
                           -1.f, -1.f, -1.f, 1.f );         //-1.0 so that it's never omitted
        if (false) {
            //Sadly, XML does not support vertex colors yet... but I'd like the possibility
            vars["DIFFUSE"] = "diffuse vertexcolor";
        } else {
            SetupColorTplVars( vars, "DIFFUSE", "diffuse", "DR", "DG", "DB", "DA",
                               xml.material.dr, xml.material.dg, xml.material.db, xml.material.da,
                               1.f, 1.f, 1.f, 1.f );
            SetupColorTplVars( vars, "DIFFUSE_H", "diffuse", "DR_H", "DG_H", "DB_H", "DA",
                               xml.material.dr/2, xml.material.dg/2, xml.material.db/2, xml.material.da,
                               -1.f, -1.f, -1.f, -1.f );
            SetupColorTplVars( vars, "DIFFUSE_Q", "diffuse", "DR_Q", "DG_Q", "DB_Q", "DA",
                               xml.material.dr/4, xml.material.dg/4, xml.material.db/4, xml.material.da,
                               -1.f, -1.f, -1.f, -1.f );
        }
        SetupColorTplVars( vars, "SPECULAR", "specular", "SR", "SG", "SB", "SA",
                           xml.material.sr, xml.material.sg, xml.material.sb, xml.material.sa,
                           0.f, 0.f, 0.f, 0.f, true );
        if ( !vars["SPECULAR"].empty() )
            vars["SPECULAR"] += " $(SHININESS)";
        SetupColorTplVars( vars, "SPECULAR_H", "specular", "SR_H", "SG_H", "SB_H", "SA",
                           xml.material.sr/2, xml.material.sg/2, xml.material.sb/2, xml.material.sa,
                           0.f, 0.f, 0.f, 0.f, true );
        if ( !vars["SPECULAR_H"].empty() )
            vars["SPECULAR_H"] += " $(SHININESS)";
        SetupColorTplVars( vars, "SPECULAR_Q", "specular", "SR_Q", "SG_Q", "SB_Q", "SA",
                           xml.material.sr/4, xml.material.sg/4, xml.material.sb/4, xml.material.sa,
                           0.f, 0.f, 0.f, 0.f, true );
        if ( !vars["SPECULAR_Q"].empty() )
            vars["SPECULAR_Q"] += " $(SHININESS)";
        vars["SHININESS"] = XMLSupport::tostring( xml.material.power );
        vars["SHININESS_OVER_256"] = XMLSupport::tostring( xml.material.power/256 );

        SetupColorTplVars( vars, "EMISSIVE", "emissive", "ER", "EG", "EB", "EA",
                           xml.material.er, xml.material.eg, xml.material.eb, xml.material.ea,
                           0.f, 0.f, 0.f, 0.f );
        SetupColorTplVars( vars, "EMISSIVE_H", "emissive", "ER_H", "EG_H", "EB_H", "EA",
                           xml.material.er/2, xml.material.eg/2, xml.material.eb/2, xml.material.ea,
                           0.f, 0.f, 0.f, 0.f );
        SetupColorTplVars( vars, "EMISSIVE_Q", "emissive", "ER_Q", "EG_Q", "EB_Q", "EA",
                           xml.material.er/4, xml.material.eg/4, xml.material.eb/4, xml.material.ea,
                           0.f, 0.f, 0.f, 0.f );

        SetupBlendTplVars( vars, xml.blend_src, xml.blend_dst );
        if (xml.blend_dst == ZERO) {
            vars["DEPTH_WRITE"] = "";             //default
            vars["DEPTH_WRITE_VALUE"] = "on";
        } else {
            vars["DEPTH_WRITE"] = "depth_write $(DEPTH_WRITE_VALUE)";             //default
            vars["DEPTH_WRITE_VALUE"] = "off";
        }
        if (fabs( xml.polygon_offset ) < 0.0001f) {
            vars["DEPTH_BIAS"] = "";
            vars["DEPTH_BIAS_VALUE"] = "0";
        } else {
            vars["DEPTH_BIAS"] = "depth_bias $(DEPTH_BIAS_VALUE)";
            vars["DEPTH_BIAS_VALUE"] = XMLSupport::tostring( xml.polygon_offset );
        }
        if (xml.alphatest < 0.0001f) {
            vars["ALPHA_REJECTION"] = "";
            vars["ALPHA_REJECTION_FUNCTION"] = "always_pass";
            vars["ALPHA_REJECTION_VALUE"] = "";
        } else {
            vars["ALPHA_REJECTION"] = "alpha_rejection $(ALPHA_REJECTION_FUNCTION) $(ALPHA_REJECTION_VALUE)";
            vars["ALPHA_REJECTION_FUNCTION"] = "greater_equal";
            vars["ALPHA_REJECTION_VALUE"] = XMLSupport::tostring( int(xml.alphatest*255.f) );
        }
        if (xml.lighting) {
            vars["LIGHTING"] = "";
            vars["LIGHTING_VALUE"] = "on";
        } else {
            vars["LIGHTING"] = "lighting $(LIGHTING_VALUE)";
            vars["LIGHTING_VALUE"] = "off";
        }
        //Culling
        vars["CULL_HARDWARE"] = "cull_hardware $(CULL_HARDWARE_VALUE)";
        vars["CULL_HARDWARE_VALUE"] = (xml.cullface ? "clockwise" : "none");
        //right now, all of them are 2d
        if (tex[0] != -1) vars["TEXTURE_DIF_TYPE"] = "2d";
        if (tex[1] != -1) vars["TEXTURE_PPL_TYPE"] = "2d";
        if (tex[2] != -1) vars["TEXTURE_DMG_TYPE"] = "2d";
        if (tex[3] != -1) vars["TEXTURE_GLOW_TYPE"] = "2d";
        //TODO: Propperly handle alphamaps
        vars["BASEPATH"] = ctxt->basepath;
        if (tex[0] != -1) vars["TEXTURE_DIF_FILE"] = "$(BASEPATH)$(TEXTURE_DIF_FILENAME)";
        if (tex[1] != -1) vars["TEXTURE_PPL_FILE"] = "$(BASEPATH)$(TEXTURE_PPL_FILENAME)";
        if (tex[2] != -1) vars["TEXTURE_DMG_FILE"] = "$(BASEPATH)$(TEXTURE_DMG_FILENAME)";
        if (tex[3] != -1) vars["TEXTURE_GLOW_FILE"] = "$(BASEPATH)$(TEXTURE_GLOW_FILENAME)";
        if (tex[0] != -1) vars["TEXTURE_DIF_FILENAME"] = tostring( xml.textures[tex[0]].name );
        if (tex[1] != -1) vars["TEXTURE_PPL_FILENAME"] = tostring( xml.textures[tex[1]].name );
        if (tex[2] != -1) vars["TEXTURE_DMG_FILENAME"] = tostring( xml.textures[tex[2]].name );
        if (tex[3] != -1) vars["TEXTURE_GLOW_FILENAME"] = tostring( xml.textures[tex[3]].name );
        //Finally, parse template, replace variables and add to material list
        replaceVars( tpl, vars );
        cleanup( tpl );
        ctxt->materials.insert( pair< string, pair< string, string > > ( mathash, pair< string, string > ( matname, tpl ) ) );

        return matname;
    } else {
        return mit->second.first;
    }
}

static void AddSharedVertexData( void *outputcontext, const XML &xml, bool texcoord, bool normals )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    for (size_t i = 0; i < xml.vertices.size(); ++i)
        ctxt->sharedvert.push_back( xml.vertices[i] );
    ctxt->sv_usenorm = ctxt->sv_usenorm || normals;
    ctxt->sv_usetex  = ctxt->sv_usetex || texcoord;
}

static Ogre::VertexData * CreateVertexBuffers( const vector< GFXVertex > &vertices,
                                         bool texcoord,
                                         bool normals,
                                         Vector3 &min,
                                         Vector3 &max,
                                         Real &maxsqr,
                                         vector< unsigned int > *indices = 0 )
{
    Ogre::VertexData *data = new Ogre::VertexData();
    data->vertexCount = vertices.size();

    Ogre::VertexDeclaration   *decl = data->vertexDeclaration;
    Ogre::VertexBufferBinding *bind = data->vertexBufferBinding;

    //--If we're given indices, prepare a list of used/unused ones--
    vector< bool >used;
    vector< unsigned int >idxmap;
    if (indices) {
        size_t vcount;
        size_t i;

        vcount = 0;
        used.resize( vertices.size(), false );
        idxmap.resize( vertices.size() );
        for (i = 0; i < indices->size(); ++i)
            used[(*indices)[i]] = true;
        for (i = 0; i < vertices.size(); ++i)
            if (used[i]) vcount++;
        data->vertexCount = vcount;
    }
    //--Declare--
    size_t offset = 0, ofs_p, ofs_n, ofs_t;

    //Position (always)
    decl->addElement( 0, ofs_p = offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION );
    offset += Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT3 );
    //Normals
    if (normals || true) {
        //Forcing inclusion, because otherwise AutoTangents will fail
        decl->addElement( 0, ofs_n = offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL );
        offset += Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT3 );
    }
    //Texture coordinates
    if (texcoord || true) {
        //Forcing inclusion, because otherwise AutoTangents will fail
        decl->addElement( 0, ofs_t = offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0 );
        offset += Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT2 );
    }
    //--Bind--
    Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
                                         createVertexBuffer( offset,
                                                             data->vertexCount,
                                                             Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
                                                             false );
    bind->setBinding( 0, vbuf );

    //--Fill it--
    char *pVert = (char*) ( vbuf->lock( Ogre::HardwareBuffer::HBL_DISCARD ) );
    for (size_t i = 0, j = 0; i < vertices.size(); ++i) {
        if (indices && !used[i])
            continue;
        if (indices)
            idxmap[i] = j;
        {
            float *p = (float*) (pVert+ofs_p);
            p[0] = vertices[i].x;
            p[1] = vertices[i].y;
            p[2] = vertices[i].z;

            Vector3 P( p[0], p[1], p[2] );
            min.makeFloor( P );
            max.makeCeil( P );
            maxsqr = std::max( P.squaredLength(), maxsqr );
        }
        if (normals) {
            float *n = (float*) (pVert+ofs_n);
            n[0] = vertices[i].i;
            n[1] = vertices[i].j;
            n[2] = vertices[i].k;
        }
        if (texcoord) {
            float *t = (float*) (pVert+ofs_t);
            t[0] = vertices[i].s;
            t[1] = vertices[i].t;
        }
        pVert += vbuf->getVertexSize();
        ++j;
    }
    vbuf->unlock();
    if (indices)
        for (size_t i = 0; i < indices->size(); ++i)
            (*indices)[i] = idxmap[(*indices)[i]];
    return data;
}

static Ogre::HardwareIndexBufferSharedPtr CreateIndexBuffer( const vector< unsigned int >indices )
{
    bool   use32BitIndexes = false;
    size_t i;
    for (i = 0; !use32BitIndexes && i < indices.size(); i++)
        use32BitIndexes = ( indices[i] > (unsigned int) numeric_limits< unsigned short >::max() );
    Ogre::HardwareIndexBufferSharedPtr buf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
        use32BitIndexes ? Ogre::HardwareIndexBuffer::IT_32BIT : Ogre::HardwareIndexBuffer::IT_16BIT,
        indices.size(), Ogre::HardwareBuffer::HBU_DYNAMIC, false );
    if (use32BitIndexes) {
        unsigned int *p = (unsigned int*) buf->lock( Ogre::HardwareBuffer::HBL_DISCARD );
        for (i = 0; i < indices.size(); i++)
            p[i] = (unsigned int) indices[i];
        buf->unlock();
    } else {
        unsigned short *p = (unsigned short*) buf->lock( Ogre::HardwareBuffer::HBL_DISCARD );
        for (i = 0; i < indices.size(); i++)
            p[i] = (unsigned short) indices[i];
        buf->unlock();
    }
    return buf;
}

static unsigned int deambiguatest( const vector< GFXVertex > &origvertices,
                                   vector< GFXVertex > &vertices,
                                   multimap< unsigned int, unsigned int >vertexmap,
                                   unsigned int idx,
                                   float s,
                                   float t )
{
    multimap< unsigned int, unsigned int >::const_iterator it = vertexmap.find( idx );
    while ( (it != vertexmap.end()
             && it->first == idx)
           && ( (fabs( vertices[it->second].s-s ) > 0.00001f) || (fabs( vertices[it->second].t-t ) > 0.00001f) ) )
        ++it;
    if (it == vertexmap.end() || it->first != idx) {
        vertices.push_back( origvertices[idx] );
        vertices.back().s = s;
        vertices.back().t = t;
        vertexmap.insert( pair< unsigned int, unsigned int > ( idx, (unsigned int) vertices.size()-1 ) );
        return (unsigned int) (vertices.size()-1);
    } else {return (unsigned int) it->second; }}

static void AddMesh( void *outputcontext, const XML &xml, const string &matname, bool texcoord, bool normals )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    unsigned int idxbase = 0;

    //---- Vertex Data ----
    int numprimgroups    = 0;
    if ( xml.lines.size() ) numprimgroups++;
    if ( xml.tris.size() ) numprimgroups++;
    if ( xml.quads.size() ) numprimgroups++;
    if ( xml.linestrips.size() ) numprimgroups += int( xml.linestrips.size() );
    if ( xml.tristrips.size() ) numprimgroups += int( xml.tristrips.size() );
    if ( xml.trifans.size() ) numprimgroups += int( xml.trifans.size() );
    if ( xml.quadstrips.size() ) numprimgroups += int( xml.quadstrips.size() );
    //bool useSharedVert = (numprimgroups>1);
    bool useSharedVert = false;     //Not worth it... too difficult to support the 'append' command with this thing
    if (useSharedVert) {
        idxbase = (unsigned int) ctxt->sharedvert.size();
        AddSharedVertexData( outputcontext, xml, texcoord, normals );
    }
    //---- Primitives ----
    MeshPtr m = ctxt->top;
    if ( xml.lines.size() ) {
        const size_t idxPerElement = ( sizeof (xml.lines[0].indexref)/sizeof (xml.lines[0].indexref[0]) );
        Ogre::SubMesh  *sm = m->createSubMesh();
        sm->setMaterialName( matname );
        sm->useSharedVertices     = useSharedVert;
        sm->operationType         = Ogre::RenderOperation::OT_LINE_LIST;
        sm->indexData->indexCount = xml.lines.size()*idxPerElement;

        vector< GFXVertex >   vertices;
        multimap< unsigned int, unsigned int >vertexmap;
        vector< unsigned int >indices;
        for (size_t i = 0; i < xml.lines.size(); ++i)
            for (size_t j = 0; j < idxPerElement; ++j) {
                unsigned int idx = idxbase+xml.lines[i].indexref[j];
                if (!useSharedVert)
                    idx = deambiguatest( xml.vertices, vertices, vertexmap, idx, xml.lines[i].s[j], xml.lines[i].t[j] );
                indices.push_back( idx );
            }
        if (!useSharedVert)
            sm->vertexData = CreateVertexBuffers( vertices, texcoord, normals, ctxt->min, ctxt->max, ctxt->maxsqr, &indices );
        sm->indexData->indexBuffer = CreateIndexBuffer( indices );
    }
    if ( xml.tris.size() ) {
        const size_t idxPerElement = ( sizeof (xml.tris[0].indexref)/sizeof (xml.tris[0].indexref[0]) );
        Ogre::SubMesh  *sm = m->createSubMesh();
        sm->setMaterialName( matname );
        sm->useSharedVertices     = useSharedVert;
        sm->operationType         = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        sm->indexData->indexCount = xml.tris.size()*idxPerElement;

        vector< GFXVertex >   vertices;
        multimap< unsigned int, unsigned int >vertexmap;
        vector< unsigned int >indices;
        for (size_t i = 0; i < xml.tris.size(); ++i)
            for (size_t j = 0; j < idxPerElement; ++j) {
                unsigned int idx = idxbase+xml.tris[i].indexref[j];
                if (!useSharedVert)
                    idx = deambiguatest( xml.vertices, vertices, vertexmap, idx, xml.tris[i].s[j], xml.tris[i].t[j] );
                indices.push_back( idx );
            }
        if (!useSharedVert)
            sm->vertexData = CreateVertexBuffers( vertices, texcoord, normals, ctxt->min, ctxt->max, ctxt->maxsqr, &indices );
        sm->indexData->indexBuffer = CreateIndexBuffer( indices );
    }
    if ( xml.quads.size() ) {
        //No quads... triangulate...
        Ogre::SubMesh *sm = m->createSubMesh();
        sm->setMaterialName( matname );
        sm->useSharedVertices     = useSharedVert;
        sm->operationType         = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        sm->indexData->indexCount = xml.quads.size()*3*2;

        vector< GFXVertex >   vertices;
        multimap< unsigned int, unsigned int >vertexmap;
        vector< unsigned int >indices;
        for (size_t i = 0; i < xml.quads.size(); ++i) {
            unsigned int idx[] = {
                idxbase+xml.quads[i].indexref[0],
                idxbase+xml.quads[i].indexref[1],
                idxbase+xml.quads[i].indexref[2],
                idxbase+xml.quads[i].indexref[3]
            };
            if (!useSharedVert) {
                idx[0] = deambiguatest( xml.vertices,vertices, vertexmap, idx[0], xml.quads[i].s[0], xml.quads[i].t[0] );
                idx[1] = deambiguatest( xml.vertices,vertices, vertexmap, idx[1], xml.quads[i].s[1], xml.quads[i].t[1] );
                idx[2] = deambiguatest( xml.vertices,vertices, vertexmap, idx[2], xml.quads[i].s[2], xml.quads[i].t[2] );
                idx[3] = deambiguatest( xml.vertices,vertices, vertexmap, idx[3], xml.quads[i].s[3], xml.quads[i].t[3] );
            }
            indices.push_back( idx[0] );
            indices.push_back( idx[1] );
            indices.push_back( idx[2] );

            indices.push_back( idx[0] );
            indices.push_back( idx[2] );
            indices.push_back( idx[3] );
        }
        if (!useSharedVert)
            sm->vertexData = CreateVertexBuffers( vertices, texcoord, normals, ctxt->min, ctxt->max, ctxt->maxsqr, &indices );
        sm->indexData->indexBuffer = CreateIndexBuffer( indices );
    }
    if ( xml.linestrips.size() )
        for (size_t i = 0; i < xml.linestrips.size(); ++i) {
            const strip &s = xml.linestrips[i];
            if ( s.points.size() ) {
                Ogre::SubMesh *sm = m->createSubMesh();
                sm->setMaterialName( matname );
                sm->useSharedVertices     = useSharedVert;
                sm->operationType         = Ogre::RenderOperation::OT_LINE_STRIP;
                sm->indexData->indexCount = s.points.size();

                vector< GFXVertex >   vertices;
                multimap< unsigned int, unsigned int >vertexmap;
                vector< unsigned int >indices;
                for (size_t j = 0; j < s.points.size(); ++j) {
                    unsigned int idx = idxbase+s.points[j].indexref;
                    if (!useSharedVert)
                        idx = deambiguatest( xml.vertices, vertices, vertexmap, idx, s.points[j].s, s.points[j].t );
                    indices.push_back( idx );
                }
                if (!useSharedVert)
                    sm->vertexData = CreateVertexBuffers( vertices,
                                                          texcoord,
                                                          normals,
                                                          ctxt->min,
                                                          ctxt->max,
                                                          ctxt->maxsqr,
                                                          &indices );
                sm->indexData->indexBuffer = CreateIndexBuffer( indices );
            }
        }
    if ( xml.tristrips.size() )
        for (size_t i = 0; i < xml.tristrips.size(); ++i) {
            const strip &s = xml.tristrips[i];
            if ( s.points.size() ) {
                Ogre::SubMesh *sm = m->createSubMesh();
                sm->setMaterialName( matname );
                sm->useSharedVertices     = useSharedVert;
                sm->operationType         = Ogre::RenderOperation::OT_TRIANGLE_STRIP;
                sm->indexData->indexCount = s.points.size();

                vector< GFXVertex >   vertices;
                multimap< unsigned int, unsigned int >vertexmap;
                vector< unsigned int >indices;
                for (size_t j = 0; j < s.points.size(); ++j) {
                    unsigned int idx = idxbase+s.points[j].indexref;
                    if (!useSharedVert)
                        idx = deambiguatest( xml.vertices, vertices, vertexmap, idx, s.points[j].s, s.points[j].t );
                    indices.push_back( idx );
                }
                if (!useSharedVert)
                    sm->vertexData = CreateVertexBuffers( vertices,
                                                          texcoord,
                                                          normals,
                                                          ctxt->min,
                                                          ctxt->max,
                                                          ctxt->maxsqr,
                                                          &indices );
                sm->indexData->indexBuffer = CreateIndexBuffer( indices );
            }
        }
    if ( xml.trifans.size() )
        for (size_t i = 0; i < xml.trifans.size(); ++i) {
            const strip &s = xml.trifans[i];
            if ( s.points.size() ) {
                Ogre::SubMesh *sm = m->createSubMesh();
                sm->setMaterialName( matname );
                sm->useSharedVertices     = useSharedVert;
                sm->operationType         = Ogre::RenderOperation::OT_TRIANGLE_FAN;
                sm->indexData->indexCount = s.points.size();

                vector< GFXVertex >   vertices;
                multimap< unsigned int, unsigned int >vertexmap;
                vector< unsigned int >indices;
                for (size_t j = 0; j < s.points.size(); ++j) {
                    unsigned int idx = idxbase+s.points[j].indexref;
                    if (!useSharedVert)
                        idx = deambiguatest( xml.vertices, vertices, vertexmap, idx, s.points[j].s, s.points[j].t );
                    indices.push_back( idx );
                }
                if (!useSharedVert)
                    sm->vertexData = CreateVertexBuffers( vertices,
                                                          texcoord,
                                                          normals,
                                                          ctxt->min,
                                                          ctxt->max,
                                                          ctxt->maxsqr,
                                                          &indices );
                sm->indexData->indexBuffer = CreateIndexBuffer( indices );
            }
        }
    if ( xml.quadstrips.size() )
        for (size_t i = 0; i < xml.linestrips.size(); ++i) {
            const strip &s = xml.linestrips[i];
            if ( s.points.size() ) {
                //No quad strips either...
                //...but if we don't care about triangulation (and GL specs don't ever
                //say that it has to be done some particular way, even if it's common),
                //then we can simply treat them as triangle strips.
                Ogre::SubMesh *sm = m->createSubMesh();
                sm->setMaterialName( matname );
                sm->useSharedVertices     = useSharedVert;
                sm->operationType         = Ogre::RenderOperation::OT_TRIANGLE_STRIP;
                sm->indexData->indexCount = s.points.size();

                vector< GFXVertex >   vertices;
                multimap< unsigned int, unsigned int >vertexmap;
                vector< unsigned int >indices;
                for (size_t j = 0; j < s.points.size(); ++j) {
                    unsigned int idx = idxbase+s.points[j].indexref;
                    if (!useSharedVert)
                        idx = deambiguatest( xml.vertices, vertices, vertexmap, idx, s.points[j].s, s.points[j].t );
                    indices.push_back( idx );
                }
                if (!useSharedVert)
                    sm->vertexData = CreateVertexBuffers( vertices,
                                                          texcoord,
                                                          normals,
                                                          ctxt->min,
                                                          ctxt->max,
                                                          ctxt->maxsqr,
                                                          &indices );
                sm->indexData->indexBuffer = CreateIndexBuffer( indices );
            }
        }
}

static bool MaterialHasTextures( const XML &memfile )
{
    for (vector< textureholder >::const_iterator it = memfile.textures.begin(); it != memfile.textures.end(); ++it)
        if ( ( (*it).index >= 0 ) && ( (*it).type != UNKNOWN ) && !( (*it).name.empty() ) )
            return true;
    return false;
}

void Add( void *outputcontext, const XML &memfile )
{
    AddMesh( outputcontext, memfile, AddMaterial( outputcontext, memfile ), MaterialHasTextures( memfile ), true );
}

static void AutoOrganiseBuffers( Ogre::VertexData *data, Ogre::MeshPtr mesh )
{
#if (OGRE_VERSION_MAJOR == 1) && (OGRE_VERSION_MINOR < 1)
    Ogre::VertexDeclaration *newDcl =
        data->vertexDeclaration->getAutoOrganisedDeclaration(
            mesh->hasSkeleton() );
#else
    Ogre::VertexDeclaration *newDcl =
        data->vertexDeclaration->getAutoOrganisedDeclaration(
            mesh->hasSkeleton(), mesh->hasVertexAnimation() || (mesh->getPoseCount() > 0) );
#endif
    if ( *newDcl != *(data->vertexDeclaration) ) {
        //Usages don't matter here since we're onlly exporting
        Ogre::BufferUsageList bufferUsages;
        for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
            bufferUsages.push_back( Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY );
        data->reorganiseBuffers( newDcl, bufferUsages );
    }
}

void Optimize( void *outputcontext )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    MeshPtr newMesh = ctxt->top;
    //Shared geometry
    if (newMesh->sharedVertexData)
        AutoOrganiseBuffers( newMesh->sharedVertexData, newMesh );
    //Dedicated geometry
    Ogre::Mesh::SubMeshIterator smIt = newMesh->getSubMeshIterator();
    unsigned short idx = 0;
    while ( smIt.hasMoreElements() ) {
        Ogre::SubMesh *sm = smIt.getNext();
        if (!sm->useSharedVertices)
            AutoOrganiseBuffers( sm->vertexData, newMesh );
    }
}

void AutoLOD( void *outputcontext, bool force, int numLod, float reductionFactor, float refDistance )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    MeshPtr newMesh = ctxt->top;
    if ( force || (newMesh->getNumLodLevels() <= 1) ) {
        if (newMesh->getNumLodLevels() <= 1)
            newMesh->removeLodLevels();
        const Ogre::ProgressiveMesh::VertexReductionQuota quota = Ogre::ProgressiveMesh::VRQ_PROPORTIONAL;
        const Real reduction  = Real( 1-reductionFactor );
        
        Real currDist = refDistance;
        
#if (OGRE_VERSION >= 0x010700)
        Ogre::Mesh::LodValueList distanceList;

        // pixel area is squared length, and length is proportional to triangle count
        const Real distFactor = reductionFactor * reductionFactor;
        newMesh->setLodStrategy(Ogre::LodStrategyManager::getSingletonPtr()->
            getStrategy( "PixelCount" ) );
#else
        Ogre::Mesh::LodDistanceList distanceList;
        const Real distFactor = ( (reduction > 0.00001) ? 1/reduction : 1 );
#endif

        for (int iLod = 0; iLod < numLod; ++iLod, currDist *= distFactor)
            distanceList.push_back( currDist );
        newMesh->generateLodLevels( distanceList, quota, reduction );
    }
}

void DoneMeshes( void *outputcontext )
{
    //TODO: Optimize usage of shared vertex buffers (don't do that always)
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    MeshPtr m = ctxt->top;
    //Shared vertex data
    if ( ctxt->sharedvert.size() )
        ctxt->top->sharedVertexData = CreateVertexBuffers( ctxt->sharedvert,
                                                           ctxt->sv_usetex,
                                                           ctxt->sv_usenorm,
                                                           ctxt->min,
                                                           ctxt->max,
                                                           ctxt->maxsqr );
    //Set bounds
    const Ogre::AxisAlignedBox &currBox = m->getBounds();
    Real currRadius = m->getBoundingSphereRadius();
    if ( currBox.isNull() ) {
        //do not pad the bounding box
        m->_setBounds( Ogre::AxisAlignedBox( ctxt->min, ctxt->max ), false );
        m->_setBoundingSphereRadius( Ogre::Math::Sqrt( ctxt->maxsqr ) );
    } else {
        Ogre::AxisAlignedBox newBox( ctxt->min, ctxt->max );
        newBox.merge( currBox );
        //do not pad the bounding box
        m->_setBounds( newBox, false );
        m->_setBoundingSphereRadius( std::max( Ogre::Math::Sqrt( ctxt->maxsqr ), currRadius ) );
    }
}

void AutoEdgeList( void *outputcontext )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    ctxt->top->buildEdgeList();
}

void AutoTangents( void *outputcontext )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    
    Ogre::VertexElementSemantic oves;
    
#if (OGRE_VERSION >= 0x010700)
    oves = Ogre::VES_TANGENT;
#endif
    
    unsigned short src, dest;
    ctxt->top->suggestTangentVectorBuildParams( oves, src, dest );
    ctxt->top->buildTangentVectors( oves, src, dest );
}

void Dump( void *outputcontext, const char *outputfile, const char *materialfile )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;

    //Write .mesh file
    meshSerializer->exportMesh( ctxt->top.get(), String( outputfile ) );

    //Write .material file
    ofstream fo( materialfile, ctxt->appending ? ios::app : ios::out );
    if ( !fo.fail() ) {
        for (map< string, pair< string, string > >::const_iterator it = ctxt->materials.begin();
             it != ctxt->materials.end();
             ++it)
            fo<<it->second.second.c_str()<<endl<<endl;
        fo.close();
    }
    delete ctxt;
}

void SetName( void *outputcontext, const char *name )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    ctxt->name = string( name );
}

void SetBasePath( void *outputcontext, const char *path )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    ctxt->basepath = string( path );
}

float RadialSize( void *outputcontext )
{
    struct outputContext *ctxt = (struct outputContext*) outputcontext;
    return float( sqrt( ctxt->maxsqr ) );
}
}

#endif //HAVE_OGRE

