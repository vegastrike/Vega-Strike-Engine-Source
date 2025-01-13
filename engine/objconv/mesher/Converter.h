/*
 * Converter.h
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
#ifndef VEGA_STRIKE_ENGINE_OBJCONV_CONVERTER_H
#define VEGA_STRIKE_ENGINE_OBJCONV_CONVERTER_H

namespace Converter {
typedef std::vector<std::string> ParameterList;

/** This class handles commands of many kinds. By registering a module, you may extend
 *       the application's functionality (by making praseParams handle more commands).
 */
class Module {
public:
    typedef std::vector<std::string> NameList;

/** Standard virtual destructor */
    virtual ~Module() {
    }

/** Returns the command names - each must start with '-' or '--'. @see execute for details. */
    virtual const NameList &getNames() const = 0;

/** Handle the command
 *  @remarks
 *       The given list of parameters will have all the parameters following the command,
 *       that are not commands in turn. Commands are identified by having '-' as first
 *       character.
 *  @par
 *       The return value should be 0 if there was no error. If there was, and you return
 *       nonzero, parameter parsing will abort and return that code.
 *  @par
 *       Commands are extracted from the commandline by looking for tokens that begin with
 *       '-'. Commands may also be assigned values (as for options). The syntax for that is
 *       '--command=value'. So, when passing the commandname to execute(), it will contain
 *       the full command token (with the value part). But for lookup in the registry, the
 *       value part is ignored.
 *  @param command It's the command as given by the user.
 *  @param params It's all subsequent commandline tokens that do not begin with '-', in order.
 *       Any parameters remaining in this list after execution will be reinserted in the queue,
 *       so that they may be handled by the default handler. You may also insert new parameters
 *       here, if you wish.
 *  @param phase Commands are handled twice: phase 0, probably parsing global options only,
 *       and phase 1, where commands are actually executed.
 */
    virtual int execute(const std::string &command, ParameterList &params, unsigned int phase) = 0;

/** Show help message for specified command
 *  @remarks
 *       You may want to cull the help message according to params, if the command
 *       being handled is too extensive.
 *  @par
 *       Behave as if it was execute(command,params,0).
 */
    virtual void help(const std::string &command, ParameterList &params) const = 0;
};

/** Add this module to the module list, enabling the command parser to interpret
 *       the module's commands.
 *  @remarks
 *       The registry will delete the object... you need not worry about that.
 *  @param module An instance of the module, which will be automatically deleted.
 *  @param setDefault If true, this will be the default handler either for
 *       unknown commands, or for unbound parameters (parameter tokens not handled
 *       by any command).
 */
void registerModule(Module *module, bool setDefault = false);

/** Parse parameters and execute commands
 *  @remarks
 *       Returns zero if there was no error, or nonzero otherwise.
 *       You should make the returned value the process' exit code.
 */
int parseParams(int argc, const char *const *argv);

/** Parse parameters and execute commands
 *  @remarks
 *       Returns zero if there was no error, or nonzero otherwise.
 *       You should make the returned value the process' exit code.
 */
int parseParams(const ParameterList &params);

/** Helper class, to be able to automatically register modules. */
template<class _Module, bool _Default>
class ModuleDeclaration {
public:
    ModuleDeclaration() {
        registerModule(new _Module, _Default);
    }
};

/** This class handles conversion from one format to another.
 *       It extends the functionality of the builtin --convert command.
 */
class ConversionImpl {
public:
/** Standard virtual destructor */
    ~ConversionImpl() {
    }

    typedef std::vector<std::string> FormatList;

    enum RetCodeEnum {
        /// Everything ok...
        RC_OK = 0,

        /// Invalid parameters - abort processing
        RC_INVALID_PARAMS,

        /// Invalid input file - abort processing
        RC_INVALID_INPUT,

        /// Unspecified internal error - abort processing
        RC_INTERNAL_ERROR,

        /// Specific conversion unimplmeneted - try another implementation
        RC_NOT_IMPLEMENTED
    };

/** Handle the conversion.
 *  @remarks
 *       If you can't handle the conversion because of implementation restrictions,
 *       return RC_NOT_IMPLEMENTED.
 *  @par
 *       --convert will try the next registered implementation when it receives
 *       RC_NOT_IMPLEMENTED, but it will not on other return codes. This allows you
 *       to return a true error code, like RC_INVALID_PARAMS, and abort processing...
 *       but it also allows you to have multiple overlapping implementations, each handling
 *       a specific task (so, the one that actually can handle the task gets used).
 */
    virtual RetCodeEnum convert(const std::string &inputFormat, const std::string &outputFormat,
            const std::string &opCode) = 0;

/** Show help message, enumerating supported conversions and operations.
 *  @remarks
 *       If inputFormat/outputFormat/opCode is given, you should try to filter the message
 *       to only the bits pertinent to that conversion. When they're empty, treat it as a
 *       "wildcard" (show all messages available). This allows for
 *  @par mesher --help convert OgreMesh OgreMesh
 *  @par (which would show all available opCodes from OgreMesh to OgreMesh)
 *  @note
 *       Remember: if you don't support the conversion, don't print anything.
 */
    virtual void conversionHelp(const std::string &inputFormat, const std::string &outputFormat,
            const std::string &opCode) const = 0;
};

/** Add this conversion module to the implementation list, extending the --convert
 *       command.
 *  @remarks
 *       The registry will delete the object... you need not worry about that.
 *  @param module An implementation instance. The registry will autodelete that upon shutdown.
 *  @param priority The priority of the implementation. Implementations will be probed in
 *       ascending priority order.
 */
void registerConversionImplementation(ConversionImpl *module, int priority);

/** Get a map with all named options */
std::map<std::string, std::string> &getNamedOptions();

/** Get a specific named option - create it if it doesn't exist, with said default value. */
std::string &getNamedOption(const std::string &name, const std::string &defValue = std::string());

/** Get the executable's directory @note you may modify it */
inline std::string &getRootPath() {
    return getNamedOption("rootPath");
}

/** Get the user-specified input path @note you may modify it */
inline std::string &getInputPath() {
    return getNamedOption("inputPath");
}

/** Get the user-specified output path @note you may modify it */
inline std::string &getOutputPath() {
    return getNamedOption("outputPath");
}

inline bool hasNamedOption(const std::string &name) {
    return getNamedOptions().find(name) != getNamedOptions().end();
}

/** Helper class, to be able to automatically register conversion implementations. */
template<class _Impl, int _Priority = 0>
class ConversionImplDeclaration {
public:
    ConversionImplDeclaration() {
        registerConversionImplementation(new _Impl, _Priority);
    }
};
}

#endif //VEGA_STRIKE_ENGINE_OBJCONV_CONVERTER_H
