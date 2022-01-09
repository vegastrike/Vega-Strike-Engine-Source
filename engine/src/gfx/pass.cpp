/**
* pass.cpp
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#include "pass.h"

#include <iostream>
#include <sstream>
#include <map>
#include <exception>


#include <boost/range/adaptors.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>

using boost::property_tree::ptree;


Pass::Pass()
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


Pass::~Pass()
{
    //Should deallocate the program... but... GFX doesn't have that API.
}

// TODO: consider making this a constructor
void Pass::parsePass(ptree tree, string techniqueName, int &nextSequence)
{
    this->techniqueName = techniqueName;
    setType(tree.get("<xmlattr>.type",""));
    colorWrite = tree.get( "<xmlattr>.cwrite", true );
    setZWrite(tree.get( "<xmlattr>.zwrite", "auto" ));
    setPerLightIteration(tree, tree.get( "<xmlattr>.iteration", "once"));
    maxIterations = static_cast<unsigned int>(tree.get( "<xmlattr>.maxiterations", 0 ));
    setBlendMode(tree.get( "<xmlattr>.blend", "default" ));
    sequence = tree.get( "<xmlattr>.sequence", nextSequence );
    setDepthFunction(tree.get( "<xmlattr>.depth_function", "lequal"));
    setCullMode(tree.get( "<xmlattr>.cull", "default" ));
    setPolyMode(tree.get( "<xmlattr>.polygon_mode", "fill"));
    offsetUnits = tree.get( "<xmlattr>.polygon_offset_units", 0.0f );
    offsetFactor = tree.get( "<xmlattr>.polygon_offset_factor", 0.0f );
    lineWidth = tree.get( "<xmlattr>.line_width", 1.0f );
    sRGBAware = tree.get( "<xmlattr>.srgb_aware", false );

    nextSequence = sequence+1;

    for (const auto& iterator : tree)
    {
        string key = iterator.first;
        pt::ptree value = iterator.second;

        // Process attributes
        if(key == "texture_unit")
        {
            parseTextureUnit(value);
        } else if(key == "param")
        {
            parseParam(value);
        } else if(key == "auto_param")
        {
            parseAutoParam(value);
        }
    }

    string vp = tree.get( "vertex_program.<xmlattr>.src", "" );
    string fp = tree.get( "fragment_program.<xmlattr>.src", "" );

    if(type == ShaderPass) {
        if ( vp.empty() )
            throw std::exception(); //throw InvalidParameters( "Missing vertex program reference in technique \""+techniqueName+"\"" );
        if ( fp.empty() )
            throw std::exception(); //throw InvalidParameters( "Missing fragment program reference in technique \""+techniqueName+"\"" );
        setProgram( vp, fp );
    }
}


void Pass::parseTextureUnit(ptree tree) {
    int target;
    if (type == Pass::ShaderPass) {
        target = tree.get( "<xmlattr>.target", -1 );
    } else {
        target = tree.get( "<xmlattr>.target", 0 ); // Behavior change. This used to throw an exception and now works. Think about.
    }

    string source = tree.get( "<xmlattr>.src", "" );
    string defaultSource = tree.get( "<xmlattr>.default", "" );
    string name = tree.get( "<xmlattr>.name", "" );
    string kindString = tree.get( "<xmlattr>.kind", "" );

    TextureUnit::Kind kind = TextureUnit::TexDefault;
    if(kindString == "default") kind = TextureUnit::TexDefault;
    else if(kindString == "2d") kind = TextureUnit::Tex2D;
    else if(kindString == "3d") kind = TextureUnit::Tex3D;
    else if(kindString == "cube") kind = TextureUnit::TexCube;
    else if(kindString == "separatedCube") kind = TextureUnit::TexSepCube;

    addTextureUnit( source, target, defaultSource, name, kind );

    BOOST_LOG_TRIVIAL(debug) << boost::format("Added texture unit #%1% \"%2%\"") % getNumTextureUnits() % name;
}

void Pass::parseParam(ptree tree)
{
    string floatsString = tree.get( "<xmlattr>.value", "" );
    string name = tree.get( "<xmlattr>.name", "" );
    bool optional = tree.get("<xmlattr>.optional", false);

    float floats[4];

    std::stringstream ss (floatsString);
    string floatString;

    for(int i=0;i<4;i++) {
        floats[i] = 0;
        std::getline (ss, floatString, ',');
        floats[i] = std::stof(floatString);
    }

    addShaderParam(name, floats, optional );

    BOOST_LOG_TRIVIAL(debug)
            << boost::format("Added constant #%1% \"%2%\" with value "
                             "(%3$.2f,%4$.2f,%5$.2f,%6$.2f) as %7%") %
               getNumShaderParams() % name % floats[0] % floats[1] % floats[2] %
            floats[3] % (optional ? "optional" : "required");

}

void Pass::parseAutoParam(ptree tree)
{
    string name = tree.get( "<xmlattr>.name", "" );
    string semantic = tree.get( "<xmlattr>.semantic", "" );
    bool optional = tree.get("<xmlattr>.optional", false);

    addShaderParam(name, getShaderParam(semantic), optional );

    BOOST_LOG_TRIVIAL(debug)
            << boost::format("Added param #%1% \"%2%\" with semantic %3% as %4%") %
               getNumShaderParams() %
               name % semantic %
               (optional ? "optional" : "required");
}

void Pass::setType(string typeString)
{
    if(typeString == "fixed") type = FixedPass;
    else if(typeString == "shader") type = ShaderPass;
    // TODO: Should we handle other cases? Throw exception?
}

void Pass::setZWrite(string zWriteString)
{
    if(zWriteString == "true") zWrite = True;
    else if(zWriteString == "false") zWrite = False;
    else if(zWriteString == "auto") zWrite = Auto;
    // TODO: same as above
}

void Pass::setPerLightIteration(ptree tree, string iterationString)
{
    if(iterationString == "once") perLightIteration = 0;
    else if ( iterationString.empty() ) throw std::exception(); // "Invalid iteration attribute" );
    else perLightIteration = tree.get( "iteration", 0u );
}

void Pass::setBlendMode(string blendModeString)
{
    if(blendModeString == "default") blendMode = Default;
    else if(blendModeString == "add") blendMode = Add;
    else if(blendModeString == "multiply") blendMode = MultiAlphaBlend;
    else if(blendModeString == "alpha_blend") blendMode = AlphaBlend;
    else if(blendModeString == "decal") blendMode = Decal;
    else if(blendModeString == "premult_alpha") blendMode = PremultAlphaBlend;
    else if(blendModeString == "multi_alpha_blend") blendMode = MultiAlphaBlend;
}

void Pass::setDepthFunction(string depthString)
{
    if(depthString == "less") depthFunction = Less;
    else if(depthString == "lequal") depthFunction = LEqual;
    else if(depthString == "greater") depthFunction = Greater;
    else if(depthString == "gequal") depthFunction = GEqual;
    else if(depthString == "equal") depthFunction = Equal;
    else if(depthString == "always") depthFunction = Always;
    else if(depthString == "never") depthFunction = Never;
}

void Pass::setCullMode(string cullModeString)
{
    if(cullModeString == "none") cullMode = None;
    else if(cullModeString == "back") cullMode = Back;
    else if(cullModeString == "front") cullMode = Front;
    else if(cullModeString == "both") cullMode = FrontAndBack;
    else if(cullModeString == "default") cullMode = DefaultFace;
}

void Pass::setPolyMode(string polyModeString)
{
    if(polyModeString == "point") polyMode = Point;
    else if(polyModeString == "line") polyMode = Line;
    else if(polyModeString == "fill") polyMode = Fill;
}

Pass::ShaderParam::Semantic Pass::getShaderParam(const string &shaderString)
{
    if(shaderString == "EnvColor") return ShaderParam::EnvColor;
    else if(shaderString == "CloakingPhase") return ShaderParam::CloakingPhase;
    else if(shaderString == "Damage") return ShaderParam::Damage;
    else if(shaderString == "Damage4") return ShaderParam::Damage4;
    else if(shaderString == "DetailPlane0") return ShaderParam::DetailPlane0;
    else if(shaderString == "DetailPlane1") return ShaderParam::DetailPlane1;
    else if(shaderString == "NumLights") return ShaderParam::NumLights;
    else if(shaderString == "ActiveLightsArray") return ShaderParam::ActiveLightsArray;
    else if(shaderString == "ApparentLightSizeArray") return ShaderParam::ApparentLightSizeArray;
    else if(shaderString == "GameTime") return ShaderParam::GameTime;

    // Default return - should not occur
    // I just picked something to return at random
    return ShaderParam::EnvColor;
}
