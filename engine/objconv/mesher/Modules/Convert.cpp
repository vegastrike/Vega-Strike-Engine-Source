/*
 * Convert.cpp
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

#include "../PrecompiledHeaders/Converter.h"
#include "../Converter.h"

using namespace std;

namespace Converter {

typedef multimap<int, ConversionImpl *> ConversionImplList;

struct CRegistry {
    ConversionImplList list;

    CRegistry() {
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

// This is ok because our modules are all the same size as the abstract
    ~CRegistry() {
        for (ConversionImplList::iterator it = list.begin(); it != list.end(); ++it) {
            delete it->second;
        }
        list.clear();
    }
};
#pragma GCC diagnostic pop

// This awkward thing makes sure the registry has been constructed when we need it.
// Since we'll be using registry functions at static object initialization, we need this.
static ConversionImplList &getRegistry() {
    static CRegistry registry;
    return registry.list;
}

void registerConversionImplementation(ConversionImpl *module, int priority) {
    assert(module);
    getRegistry().insert(pair<int, ConversionImpl *>(priority, module));
}

class ConvertHandler : public Module {
    typedef set<string> OptionList;
    typedef map<string, string> AliasList;

    NameList mNames;
    AliasList mAliases;
    OptionList mOptions;

public:
    ConvertHandler() {
#define ADD_OPTION(name) mNames.push_back("-" name); mOptions.insert(name)
#define ADD_ALIAS(alias, original) ADD_OPTION(alias); mAliases.insert(pair<string,string>(alias,original))

        // Long names
        mNames.push_back("--convert");
        mNames.push_back("--input");
        mNames.push_back("--output");

        // Short names
        mNames.push_back("-c");
        mNames.push_back("-i");
        mNames.push_back("-o");

        // Options
        ADD_OPTION("flips"); //flip s coordinate
        ADD_OPTION("flipt"); //flip t coordinate
        ADD_OPTION("flipr"); //flip r coordinate
        ADD_OPTION("flipx"); //flip x coordinate
        ADD_OPTION("flipy"); //flip y coordinate
        ADD_OPTION("flipz"); //flip z coordinate
        ADD_OPTION("flip");  //?
        ADD_OPTION("flop");  //?
        ADD_OPTION("flipn"); //flip normals
        ADD_OPTION("dims");  //show dimensions
        ADD_OPTION("basepath");       //specify model base path for building material scripts
        ADD_OPTION("autolod");        //create automatic lods (TODO: may specify distance list)
        ADD_OPTION("autoedge");       //create edge list (prepare for stencil shadows)
        ADD_OPTION("autotangent");    //create tangent texcoord unit
        ADD_OPTION("forceflatshade"); //force flat shading
        ADD_OPTION("no-optimize");    //forcefully disable mesh optimization
        ADD_ALIAS ("x", "addx"); //translate x (alias)
        ADD_ALIAS ("y", "addy"); //translate y (alias)
        ADD_ALIAS ("z", "addz"); //translate z (alias)
        ADD_OPTION("addx"); //translate x
        ADD_OPTION("addy"); //translate y
        ADD_OPTION("addz"); //translate z
        ADD_OPTION("adds"); //translate s
        ADD_OPTION("addt"); //translate t
        ADD_OPTION("addr"); //translate r
        ADD_OPTION("mpyx"); //multiply x
        ADD_OPTION("mpyy"); //multiply y
        ADD_OPTION("mpyz"); //multiply z
        ADD_OPTION("mpys"); //multiply s
        ADD_OPTION("mpyt"); //multiply t
        ADD_OPTION("mpyr"); //multiply r
        ADD_OPTION("inputPath");  //override input path
        ADD_OPTION("outputPath"); //override output path
        ADD_OPTION("rootPath");   //override executable path

#undef ADD_OPTION
    }

    /*
     *     Module interface
     */

    virtual int execute(const string &command, ParameterList &params, unsigned int phase) {
        if (command == "-i" || command == "--input") {
            if (params.size() == 0) {
                cerr << "Warning: " << command << " needs a file path to follow. Ignoring." << endl;
                return 0;
            } else {
                getInputPath() = params[0];
                params.erase(params.begin());
                return 0;
            }
        } else if (command == "-o" || command == "--output") {
            if (params.size() == 0) {
                cerr << "Warning: " << command << " needs a file path to follow. Ignoring." << endl;
                return 0;
            } else {
                getOutputPath() = params[0];
                params.erase(params.begin());
                return 0;
            }
        } else if (command == "-c" || command == "--convert") {
            if (params.size() < 3) {
                cerr << "Fatal: " << command << " needs three arguments:\n"
                        << "\tmesher <...> " << command
                        << " {XMesh|BFXM|Wavefront} (XMesh|BFXM|Wafefront|Ogre) (create|append|optimize)\n"
                        << "\n"
                        << "Do \"mesher --help convert\" for details" << endl;
                return 1;
            } else {
                if (phase == 0) {
                    // Do nothing in phase 0.
                    return 0;
                } else if (phase == 1) {
                    ConversionImplList &registry = getRegistry();
                    ConversionImpl::RetCodeEnum rc = ConversionImpl::RC_NOT_IMPLEMENTED;
                    for (ConversionImplList::iterator cit = registry.begin();
                            (rc == ConversionImpl::RC_NOT_IMPLEMENTED) && (cit != registry.end()); ++cit) {
                        rc = cit->second->convert(params[0], params[1], params[2]);
                    }
                    if (rc == ConversionImpl::RC_NOT_IMPLEMENTED) {
                        cerr << "Error: " << params[2] << " from " << params[0] << " to " << params[1]
                                << " unimplemented" << endl;
                        return ConversionImpl::RC_NOT_IMPLEMENTED;
                    } else if (rc == ConversionImpl::RC_OK) {
                        params.erase(params.begin(), params.begin() + 3);
                        return ConversionImpl::RC_OK;
                    } else {
                        cerr << "Error: ";
                        switch (rc) {
                            case ConversionImpl::RC_INVALID_PARAMS:
                                cerr << "Invalid parameters specified for " << command;
                                break;
                            case ConversionImpl::RC_INVALID_INPUT:
                                cerr << "Invalid input";
                                break;
                            case ConversionImpl::RC_INTERNAL_ERROR:
                                cerr << "Internal error";
                                break;
                            default:
                                cerr << "(see above messages)";
                                break;
                        }
                        cerr << endl;
                        return rc;
                    }
                } else {
                    cerr << "Warning: ConvertHandler::execute(): received an unexpected phase. Ignoring." << endl;
                    return 0;
                }
            }
        } else if (command.length() > 2 && command[0] == '-' && command[1] != '-') {
            string::size_type eq = command.rfind('=');
            string::size_type nn = ((eq == string::npos) ? string::npos : eq - 1);
            string option = command.substr(1, nn);
            if (mOptions.count(option)) {
                string value = ((eq != string::npos) ? command.substr(eq + 1) : string("1"));
                AliasList::const_iterator ait = mAliases.find(option);
                if (ait != mAliases.end()) {
                    option = ait->second;
                }
                getNamedOption(option) = value;
                return 0;
            } else {
                cerr << "Fatal: Internal error - ConvertHandler::execute() received an unrecognized option" << endl;
                return 1;
            }
        } else {
            cerr << "Fatal: Internal error - ConvertHandler::execute() received an unrecognized command" << endl;
            return 1;
        }
    }

    virtual const NameList &getNames() const {
        return mNames;
    }

    virtual void help(const string &command, ParameterList &params) const {
        if (command == "-i" || command == "--input" || command == "-o" || command == "--output") {
            cout << "Set current input/output file.\n"
                    << "Usage:\n"
                    << "\tmesher ((--input|-i)|(--output|-o)) (path)\n\n"
                    << "The command is transitory and passive.\n"
                    << "Any further instance of the command will override any previous one.\n"
                    << "Executive commands will use the most recently bound pathname.\n"
                    << endl;
            params.clear();
        } else if (command == "-c" || command == "--convert") {
            string inf, outf, op;
            if (0 < params.size()) {
                inf = params[0];
            }
            if (1 < params.size()) {
                outf = params[1];
            }
            if (2 < params.size()) {
                op = params[2];
            }
            params.clear();

            ConversionImplList &registry = getRegistry();
            for (ConversionImplList::iterator cit = registry.begin(); cit != registry.end(); ++cit) {
                cit->second->conversionHelp(inf, outf, op);
            }
        } else {
            cerr << "Warning: ConvertHandler::help() received an unrecognized command" << endl;
        }
    }
};

static ModuleDeclaration<ConvertHandler, false> __ch_declaration;

}
