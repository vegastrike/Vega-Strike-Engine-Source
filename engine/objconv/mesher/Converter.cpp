/**
* Converter.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "PrecompiledHeaders/Converter.h"
#include "Converter.h"

#include <algorithm>

namespace Converter
{
typedef std::map< std::string, Module* >ModuleMap;
typedef std::set< Module* >             ModuleList;

struct Registry
{
    Registry() : defaultModule( 0 )
    {}

    ~Registry()
    {
        for (ModuleList::iterator it = registeredModules.begin(); it != registeredModules.end(); ++it)
            delete *it;
        defaultModule = 0;
        registeredModules.clear();
        commandMap.clear();
    }

    ModuleList registeredModules;
    ModuleMap  commandMap;
    Module    *defaultModule;
};

//This awkward thing makes sure the registry has been constructed when we need it.
//Since we'll be using registry functions at static object initialization, we need this.
static Registry& getRegistry()
{
    static Registry registry;
    return registry;
}

void registerModule( Module *module, bool setDefault )
{
    Registry &registry = getRegistry();

    assert( module );
    const Module::NameList &names = module->getNames();
    registry.registeredModules.insert( module );
    for (Module::NameList::const_iterator nit = names.begin(); nit != names.end(); ++nit)
        registry.commandMap[*nit] = module;
    if (setDefault)
        registry.defaultModule = module;
}

static int _parseParams( const ParameterList &params, unsigned int phase )
{
    Registry     &registry = getRegistry();

    ParameterList::const_iterator pit = params.begin();
    ParameterList feedbackParams;
    while ( ( pit != params.end() ) || (feedbackParams.size() > 0) ) {
        if (feedbackParams.size() > 0) {
            Module     *handler = registry.defaultModule;
            std::string command;
            if (feedbackParams[0][0] == '-') {
                //Copy command name - it will be useful for the handler
                command = feedbackParams[0];
                feedbackParams.erase( feedbackParams.begin() );

                //Command may be options, in which case they may be assigned values.
                //The syntax is --command=value, so strip the '=value' portion for
                //command lookup.
                ModuleMap::const_iterator it = registry.commandMap.find(
                    command.substr( 0, command.rfind( '=' ) ) );
                if ( it != registry.commandMap.end() )
                    handler = it->second;
            }
            int rv = handler->execute( command, feedbackParams, phase );
            if (rv)
                return rv;
        } else {
            feedbackParams.push_back( *pit );
            ++pit;
            while ( ( pit != params.end() ) && ( (*pit)[0] != '-' ) ) {
                feedbackParams.push_back( *pit );
                pit++;
            }
        }
    }
    return 0;
}

int parseParams( int argc, const char*const *argv )
{
    ParameterList params;
    for (unsigned int i = 0; i < (unsigned int) argc; ++i)
        params.push_back( argv[i] );
    parseParams( params );
    return 0;
}

int parseParams( const ParameterList &params )
{
    int rv = _parseParams( params, 0 );
    if (rv)
        return rv;

    else
        return _parseParams( params, 1 );
}

std::map< std::string, std::string >& getNamedOptions()
{
    static std::map< std::string, std::string >options;
    return options;
}

std::string& getNamedOption( const std::string &name, const std::string &defValue )
{
    std::map< std::string, std::string >::iterator it = getNamedOptions().find( name );
    if ( it != getNamedOptions().end() ) {
        return it->second;
    } else {
        it = getNamedOptions().insert( std::pair< std::string, std::string > ( name, defValue ) ).first;
        return it->second;
    }
}

using namespace std;

class HelpHandler : public Module
{
    NameList mNames;

public: HelpHandler()
    {
        //Long names
        mNames.push_back( "--help" );

        //Short names
        mNames.push_back( "-h" );
    }

/*
 *     Module interface
 */

    virtual int execute( const string &command, ParameterList &params, unsigned int phase )
    {
        if (command == "-h" || command == "--help") {
            if (phase == 0) {
                if ( params.size() ) {
                    string hcommand = params[0];
                    params.erase( params.begin() );
                    help( hcommand, params );
                    params.clear();
                } else {
                    help( "", params );
                }
            } else {
                params.clear();
            }
            return 0;
        } else {
            cerr<<"Fatal: Internal error - HelpHandler::execute() received an unrecognized command"<<endl;
            return 1;
        }
    }

    virtual const NameList& getNames() const
    {
        return mNames;
    }

    virtual void help( const string &command, ParameterList &params ) const
    {
        if (command == "-h" || command == "--help") {
            cout<<"Show help for command\n"
                <<"Usage:\n"
                <<"\tmesher (--help|-h) [ (command) *(help-args) ]\n\n"
                <<"\"help-args\" depends on \"command\"\n"
                <<"Use --help alone for a list of commands\n"
                <<"NOTE: \"command\" must not have the heading slashes\n"
                <<endl;
            params.clear();
        } else if ( command.empty() ) {
            cout<<"Commands:\n\n";
            Registry &registry = getRegistry();
            for (ModuleList::iterator it = registry.registeredModules.begin(); it != registry.registeredModules.end(); ++it)
                for (Module::NameList::const_iterator nit = (*it)->getNames().begin();
                     nit != (*it)->getNames().end();
                     ++nit)
                    cout<<nit->c_str()<<"\n";
            cout<<endl;
        } else {
            cout<<"Help for "<<command<<endl;
            Registry &registry = getRegistry();
            for (ModuleList::iterator it = registry.registeredModules.begin(); it != registry.registeredModules.end(); ++it) {
                string hcommand;
                bool   found =
                    ( (*it)->getNames().end()
                     != find( (*it)->getNames().begin(), (*it)->getNames().end(), ( hcommand = (string( "--" )+command) ) ) )
                    || ( (*it)->getNames().end()
                        != find( (*it)->getNames().begin(), (*it)->getNames().end(), ( hcommand = (string( "-" )+command) ) ) );
                if (found)
                    (*it)->help( hcommand, params );
            }
            cout<<"(end help)"<<endl;
        }
    }
};

static ModuleDeclaration< HelpHandler, false >__hh_declaration;
}

