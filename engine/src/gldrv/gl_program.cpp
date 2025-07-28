/*
 * gl_program.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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


#include <map>
#include <set>

#include <boost/algorithm/string/predicate.hpp>

#include "gl_globals.h"
#include "configuration/configuration.h"
#include "root_generic/vs_globals.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "src/vegastrike.h"
#include "src/config_xml.h"
#include "src/gfxlib.h"
#include "root_generic/lin_time.h"
#include "root_generic/options.h"

using boost::algorithm::icontains;
using VSFileSystem::UnknownFile;
using VSFileSystem::Ok;

#if _MSC_VER >= 1300
#define snprintf _snprintf
#endif

typedef std::pair<unsigned int, std::pair<std::string, std::string> > ProgramCacheKey;
typedef std::map<ProgramCacheKey, int> ProgramCache;
typedef std::map<int, ProgramCacheKey> ProgramICache;

static ProgramCache programCache;
static ProgramICache programICache;

static ProgramCache::key_type cacheKey(const std::string &vp, const std::string &fp, const char *defines) {
    unsigned int defhash = 0;
    if (defines != NULL) {
        defhash = 0xBA0BAB00;
        while (*defines) {
            defhash ^= (defhash * 127) | *(defines++);
        }
    }
    return std::pair<unsigned int, std::pair<std::string, std::string> >(defhash,
                                                                         std::pair<std::string, std::string>(vp, fp));
}

static bool validateLog(GLuint obj, bool shader,
                        bool allowSoftwareEmulation = false) {
    // Retrieve compiler log
    const GLsizei LOGBUF = 1024;
    GLsizei infologLength = 0;
    char infoLog[LOGBUF + 1]; // +1 for null terminator

    if (shader) {
        glGetShaderInfoLog_p(obj, LOGBUF, &infologLength, infoLog);
    } else {
        glGetProgramInfoLog_p(obj, LOGBUF, &infologLength, infoLog);
    }

    if (infologLength > 0) {
        // make sure infoLog is null-termiated;
        assert(infologLength <= LOGBUF);
        infoLog[infologLength] = 0;

        // search for signs of emulated execution
        if (!allowSoftwareEmulation) {
            if (icontains(infoLog, "run in software")) {
                return false;
            }
            if (icontains(infoLog, "run on software")) {
                return false;
            }
        }
    }

    // No validation failed...
    return true;
}

void printLog(GLuint obj, bool shader) {
    const GLsizei LOGBUF = 1024;
    GLsizei infologLength = 0;
    char infoLog[LOGBUF + 1]; // +1 for null terminator

    if (shader) {
        glGetShaderInfoLog_p(obj, 1024, &infologLength, infoLog);
    } else {
        glGetProgramInfoLog_p(obj, 1024, &infologLength, infoLog);
    }

    // make sure infoLog is null-termiated;
    assert(infologLength <= LOGBUF);
    infoLog[infologLength] = 0;

    if (infologLength > 0) {
        VS_LOG(error, (boost::format("%1%") % infoLog));
    }
}

static VSFileSystem::VSError getProgramSource(const std::string &path,
                                              std::vector<std::string> &lines,
                                              std::set<std::string> &processed_includes,
                                              char *buf,
                                              size_t buflen) {
    std::string dirname = path.substr(0, path.find_last_of('/'));

    VSFileSystem::VSFile f;
    VSFileSystem::VSError err = f.OpenReadOnly(path.c_str(), UnknownFile);

    const char *include_directive = "#include \"";
    const size_t include_directive_len = 10;
    size_t lineno = 0;

    if (err <= Ok) {
        processed_includes.insert(path);

        while (Ok == f.ReadLine(buf, buflen)) {
            ++lineno;
            if (strncmp(buf, include_directive, include_directive_len) == 0) {
                // Process include directives
                char *eos = strchr(buf + include_directive_len, '\"');
                if (eos != NULL) {
                    *eos = 0;
                    std::string includepath = dirname + "/" + std::string(buf + include_directive_len);
                    if (processed_includes.count(includepath) == 0) {
                        // Set up line numbers for include file
                        lines.push_back("#line 0\n");

                        VSFileSystem::VSError
                                ierr = getProgramSource(includepath, lines, processed_includes, buf, buflen);
                        if (ierr > Ok) {
                            f.Close();
                            VS_LOG(error, (boost::format("ERROR: included from %1%") % path.c_str()));
                            return ierr;
                        } else {
                            // Append a blank line to avoid issues and restore line numbers
                            lines.push_back("\n");
                            snprintf(buf, buflen - 1, "#line %zu\n", lineno);
                            lines.push_back(buf);
                        }
                    } else {
                        // Insert blank line to keep line numbers consistent
                        lines.push_back("\n");
                    }
                } else {
                    VS_LOG(warning, (boost::format("WARNING: broken include directive at file %1%, line %2% - skipping")
                                     % path.c_str()
                                     % lineno));
                }
            } else {
                // Append a line to the list
                lines.push_back(buf);
            }
        }

        f.Close();
    } else {
        VS_LOG(error, (boost::format("ERROR: at %1%") % path.c_str()));
    }
    return err;
}

static VSFileSystem::VSError getProgramSource(const std::string &path, std::string &source) {
    std::set<std::string> processed_includes;
    std::vector<std::string> lines;
    char buf[16384];

    source.clear();

    VSFileSystem::VSError err = getProgramSource(path, lines, processed_includes, buf, sizeof(buf));

    if (err <= Ok) {
        size_t sourcelen = 0;
        for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
            sourcelen += it->length();
        }
        source.reserve(sourcelen);
        for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
            source += *it;
        }
    }
    return err;
}

static std::string appendDefines(const std::string &prog, const char *extra_defines) {
    std::string::size_type nlpos = prog.find_first_of('\n');

    if (nlpos == std::string::npos) {
        nlpos = 0;
    }

    std::string firstline = prog.substr(0, nlpos);

    if (firstline.find("#version") != std::string::npos) {
        return firstline
               + "\n" + std::string(extra_defines)
               + "\n#line 1"
               + prog.substr(nlpos);
    } else {
        return std::string(extra_defines)
               + "\n#line 0\n"
               + prog;
    }
}

static int GFXCreateProgramNoCache(const char *vprogram, const char *fprogram, const char *extra_defines) {
    if (vprogram[0] == '\0' && fprogram[0] == '\0') {
        return 0;
    }
#ifndef __APPLE__
    if (glGetProgramInfoLog_p == NULL || glCreateShader_p == NULL || glShaderSource_p == NULL
            || glCompileShader_p == NULL
            || glAttachShader_p == NULL || glLinkProgram_p == NULL || glGetShaderiv_p == NULL
            || glGetProgramiv_p == NULL) {
        return 0;
    }
#else
#ifdef OSX_LOWER_THAN_10_4
    return 0;
#endif
#endif
    GLenum errCode;
    while ((errCode = glGetError()) != GL_NO_ERROR) {
        VS_LOG(error, (boost::format("Error code %1%") % gluErrorString(errCode)));
    }
    VSFileSystem::VSFile vf, ff;
    std::string vpfilename = std::string("programs/") + vprogram + ".vp";
    std::string fpfilename = std::string("programs/") + fprogram + ".fp";

    std::string vertexprg, fragprg;
    VSFileSystem::VSError vperr = getProgramSource(vpfilename, vertexprg);
    VSFileSystem::VSError fperr = getProgramSource(fpfilename, fragprg);
    if ((vperr > Ok) || (fperr > Ok)) {
        if (vperr > Ok) {
            VS_LOG(error, (boost::format("Vertex Program Error: Failed to open file %1%") % vpfilename));
        }
        if (fperr > Ok) {
            VS_LOG(error, (boost::format("Fragment Program Error: Failed to open file %1%") % fpfilename));
        }
        return 0;
    }

    if (extra_defines != NULL) {
        vertexprg = appendDefines(vertexprg, extra_defines);
        fragprg = appendDefines(fragprg, extra_defines);
    }

    GLint vproghandle = 0;
    GLint fproghandle = 0;
    GLint sp = 0;
    if (vperr <= Ok) {
        vproghandle = glCreateShader_p(GL_VERTEX_SHADER);
        const char *tmp = vertexprg.c_str();
        glShaderSource_p(vproghandle, 1, &tmp, NULL);
        glCompileShader_p(vproghandle);
        GLint successp = 0;
        glGetShaderiv_p(vproghandle, GL_COMPILE_STATUS, &successp);
        if (successp == 0) {
            printLog(vproghandle, true);
            VS_LOG(error, (boost::format("Vertex Program Error: Failed to compile %1%") % vprogram));
            glDeleteShader_p(vproghandle);
            return 0;
        } else if (!validateLog(vproghandle, true)) {
            printLog(vproghandle, true);
            VS_LOG(error,
                   (boost::format("Vertex Program Error: Failed log validation for %1%. Inspect log above for details.")
                    % vprogram));
            glDeleteShader_p(vproghandle);
            return 0;
        }
        printLog(vproghandle, true);
    }
    if (fperr <= Ok) {
        fproghandle = glCreateShader_p(GL_FRAGMENT_SHADER);
        const char *tmp = fragprg.c_str();
        glShaderSource_p(fproghandle, 1, &tmp, NULL);
        glCompileShader_p(fproghandle);
        GLint successp = 0;
        glGetShaderiv_p(fproghandle, GL_COMPILE_STATUS, &successp);
        if (successp == 0) {
            printLog(fproghandle, true);
            VS_LOG(error, (boost::format("Fragment Program Error: Failed to compile %1%") % fprogram));
            glDeleteShader_p(vproghandle);
            glDeleteShader_p(fproghandle);
            return 0;
        } else if (!validateLog(fproghandle, true)) {
            // FIXME: Should this be fproghandle instead of vproghandle? Same throughout this if block?
            printLog(vproghandle, true);
            VS_LOG(error,
                   (boost::format("Vertex Program Error: Failed log validation for %1%. Inspect log above for details.")
                    % vprogram));
            glDeleteShader_p(vproghandle);
            glDeleteShader_p(fproghandle);
            return 0;
        }
        printLog(fproghandle, true);
    }

    sp = glCreateProgram_p();
    glAttachShader_p(sp, vproghandle);
    glAttachShader_p(sp, fproghandle);
    glLinkProgram_p(sp);

    GLint successp = 0;
    glGetProgramiv_p(sp, GL_LINK_STATUS, &successp);
    if (successp == 0) {
        printLog(sp, false);
        VS_LOG(error, (boost::format("Shader Program Error: Failed to link %1% to %2%") % vprogram % fprogram));
        return 0;
    } else if (!validateLog(sp, false)) {
        printLog(sp, false);
        VS_LOG(error,
               (boost::format(
                       "Shader Program Error: Failed log validation for vp:%1% fp:%2%. Inspect log above for details.")
                % vprogram % fprogram));
        glDeleteShader_p(vproghandle);
        glDeleteShader_p(fproghandle);
        glDeleteProgram_p(sp);
        return 0;
    }
    printLog(sp, false);

    /* only for dev work
     *  glGetProgramiv_p(sp,GL_VALIDATE_STATUS,&successp);
     *  if (successp==0) {
     *      VS_LOG(error, (boost::format("Shader Program Error: Failed to validate %1% linking to %2%") % vprogram % fprogram));
     *      return 0;
     *  }
     */
    while ((errCode = glGetError()) != GL_NO_ERROR) {
        VS_LOG(error, (boost::format("Error code %1%") % gluErrorString(errCode)));
        sp = 0;         //no proper vertex prog support
    }
    return sp;
}

int GFXCreateProgram(const char *vprogram, const char *fprogram, const char *extra_defines) {
    ProgramCache::key_type key = cacheKey(vprogram, fprogram, extra_defines);
    ProgramCache::const_iterator it = programCache.find(key);
    if (it != programCache.end()) {
        return it->second;
    }
    int rv = programCache[key] = GFXCreateProgramNoCache(vprogram, fprogram, extra_defines);
    programICache[rv] = key;
    return rv;
}

int GFXCreateProgram(char *vprogram, char *fprogram, char *extra_defines) {
    return GFXCreateProgram((const char *) vprogram, (const char *) fprogram, (const char *) extra_defines);
}

void GFXDestroyProgram(int program) {
    // Find program
    ProgramICache::iterator it = programICache.find(program);
    if (it != programICache.end()) {
        /*
        if (glDeleteProgram_p)
            glDeleteProgram_p( program );
        */
        // FIXME: Real problem here with program leakage,
        //      but cannot destroy like this, brings all kind of issues
        //      since the caller may not hold the only reference.
        programCache.erase(it->second);
        programICache.erase(it);
    }
}

static int programChanged = false;
static int programVersion = 0;
static int defaultprog = 0;
static int lowfiprog = 0;
static int hifiprog = 0;


// THIS IS STUPID!

#if defined(__APPLE__) && defined (__MACH__)
std::string hifiProgramName  = "mac";
std::string lowfiProgramName = "maclite";
#else
std::string hifiProgramName = "default";
std::string lowfiProgramName = "lite";
#endif
// END STUPID


int getDefaultProgram() {
    static bool initted = false;
    if (!initted) {

// THIS IS STUPID,  Also why is lofi not configurable ?
#if defined(__APPLE__) && defined (__MACH__)
        hifiProgramName = configuration()->graphics.mac_shader_name;
#else
        hifiProgramName = configuration()->graphics.shader_name;
#endif
// END STUPID

        if (hifiProgramName.length() == 0) {
            lowfiprog = hifiprog = 0;
        } else {
            lowfiprog = GFXCreateProgram(lowfiProgramName.c_str(), lowfiProgramName.c_str(), NULL);
            if (lowfiprog == 0) {
                lowfiprog = GFXCreateProgram(hifiProgramName.c_str(), hifiProgramName.c_str(), NULL);
            }
            hifiprog = GFXCreateProgram(hifiProgramName.c_str(), hifiProgramName.c_str(), NULL);
            if (hifiprog == 0) {
                hifiprog = GFXCreateProgram(lowfiProgramName.c_str(), lowfiProgramName.c_str(), NULL);
            }
        }
        defaultprog = hifiprog;
        programChanged = true;
        initted = true;
    }
    return defaultprog;
}

void GFXReloadDefaultShader() {
    VS_LOG(info, "Reloading all shaders");

    // Increasing the timestamp makes all programs elsewhere recompile
    ++programVersion;

    bool islow = (lowfiprog == defaultprog);
    if (glDeleteProgram_p && defaultprog) {
        glDeleteProgram_p(lowfiprog);
        glDeleteProgram_p(hifiprog);
    }
    programChanged = true;
    if (islow) {
        hifiprog = GFXCreateProgram(hifiProgramName.c_str(), hifiProgramName.c_str(), NULL);
        if (hifiprog == 0) {
            hifiprog = GFXCreateProgram(lowfiProgramName.c_str(), lowfiProgramName.c_str(), NULL);
        }
        lowfiprog = GFXCreateProgram(lowfiProgramName.c_str(), lowfiProgramName.c_str(), NULL);
        if (lowfiprog == 0) {
            lowfiprog = GFXCreateProgram(hifiProgramName.c_str(), hifiProgramName.c_str(), NULL);
        }
        defaultprog = lowfiprog;
    } else {
        lowfiprog = GFXCreateProgram(lowfiProgramName.c_str(), lowfiProgramName.c_str(), NULL);
        if (lowfiprog == 0) {
            lowfiprog = GFXCreateProgram(hifiProgramName.c_str(), hifiProgramName.c_str(), NULL);
        }
        hifiprog = GFXCreateProgram(hifiProgramName.c_str(), hifiProgramName.c_str(), NULL);
        if (hifiprog == 0) {
            hifiprog = GFXCreateProgram(lowfiProgramName.c_str(), lowfiProgramName.c_str(), NULL);
        }
        defaultprog = hifiprog;
    }
}

enum GameSpeed {
    JUSTRIGHT,
    TOOSLOW,
    TOOFAST
};

unsigned int gpdcounter = (1 << 30);
#define NUMFRAMESLOOK 128
GameSpeed gameplaydata[NUMFRAMESLOOK] = {JUSTRIGHT};

GameSpeed GFXGetFramerate() {
    GameSpeed retval = JUSTRIGHT;
    static double lasttime = queryTime();
    double thistime = queryTime();
    double framerate = 1. / (thistime - lasttime);
    static double toofast = 80;
    static double tooslow = 29;
    static unsigned lim = 10;
    static int penalty = 10;
    static float lowratio = .125;
    static float highratio = .75;
    GameSpeed curframe;
    if (framerate > toofast) {
        curframe = TOOFAST;
    } else if (framerate > tooslow) {
        curframe = JUSTRIGHT;
    } else {
        curframe = TOOSLOW;
    }
    gameplaydata[((unsigned int) gpdcounter++) % NUMFRAMESLOOK] = curframe;
    unsigned int i = 0;
    if (!(curframe == JUSTRIGHT
          || (curframe == TOOFAST && defaultprog == hifiprog) || (curframe == TOOSLOW && defaultprog == 0))) {
        for (; i < lim; ++i) {
            if (curframe != gameplaydata[((unsigned int) (gpdcounter - i)) % NUMFRAMESLOOK]) {
                break;
            }
        }
        if (i == lim) {
            int correct = 0;
            int incorrect = 0;
            for (unsigned int j = 0; j < NUMFRAMESLOOK; ++j) {
                if (gameplaydata[j] == curframe) {
                    correct++;
                }
                if (gameplaydata[j] != curframe) {
                    incorrect++;
                }
                if (curframe == TOOFAST && gameplaydata[j] == TOOSLOW) {
                    incorrect += penalty;
                }
            }
            double myratio = (double) correct / (double) (correct + incorrect);
            if (curframe == TOOFAST && myratio > highratio) {
                static int toomanyswitches = 3;
                toomanyswitches -= 1;
                if (toomanyswitches >= 0) {
                    retval = curframe;
                }                      //only switch back and forth so many times
            } else if (myratio > lowratio) {
                retval = curframe;
            }
        }
    }
    lasttime = thistime;
    if (retval != JUSTRIGHT) {
        for (unsigned int i = 0; i < NUMFRAMESLOOK; ++i) {
            gameplaydata[i] = JUSTRIGHT;
        }
    }
    return retval;
}

bool GFXShaderReloaded() {
    bool retval = programChanged;
    if (configuration()->graphics.framerate_changes_shader) {
        switch (GFXGetFramerate()) {
            case TOOSLOW:
                if (defaultprog) {
                    retval = true;
                    if (defaultprog == hifiprog) {
                        defaultprog = lowfiprog;
                    } else {
                        defaultprog = 0;
                    }
                    GFXActivateShader((char *) NULL);
                }
                break;
            case TOOFAST:
                if (defaultprog != hifiprog) {
                    retval = true;
                    if (defaultprog == 0) {
                        defaultprog = lowfiprog;
                    } else {
                        defaultprog = hifiprog;
                    }
                    GFXActivateShader((char *) NULL);
                }
                break;
            default:
                break;
        }
    }
    programChanged = false;
    return retval;
}

bool GFXDefaultShaderSupported() {
    return getDefaultProgram() != 0;
}

int GFXActivateShader(int program) {
    static int lastprogram = 0;
    if (program != lastprogram) {
        programChanged = true;
    }
    if (program != lastprogram
#ifndef __APPLE__
            && glUseProgram_p
#endif
            ) {
        glUseProgram_p(program);
        lastprogram = program;
    } else {
        return 0;
    }
    return program;
}

int GFXActivateShader(const char *program) {
    int defaultprogram = getDefaultProgram();
    int curprogram = defaultprogram;
    if (program) {
        curprogram = GFXCreateProgram(program, program, NULL);
    }
    return GFXActivateShader(curprogram);
}

void GFXDeactivateShader() {
    GFXActivateShader((int) 0);
}

int GFXShaderConstant(int name, float v1, float v2, float v3, float v4) {
    if (1
#ifndef __APPLE__
            && glUniform4f_p
#endif
            ) {
        glUniform4f_p(name, v1, v2, v3, v4);
        return 1;
    }
    return 0;
}

int GFXShaderConstant(int name, const float *values) {
    return GFXShaderConstant(name, values[0], values[1], values[2], values[3]);
}

int GFXShaderConstant(int name, GFXColor v) {
    return GFXShaderConstant(name, v.r, v.g, v.b, v.a);
}

int GFXShaderConstant(int name, Vector v) {
    return GFXShaderConstant(name, v.i, v.j, v.k, 0);
}

int GFXShaderConstant(int name, float v1) {
    if (1
#ifndef __APPLE__
            && glUniform1f_p
#endif
            ) {
        glUniform1f_p(name, v1);
        return 1;
    }
    return 0;
}

int GFXShaderConstantv(int name, unsigned int count, const float *values) {
    if (1
#ifndef __APPLE__
            && glUniform1fv_p
#endif
            ) {
        glUniform1fv_p(name, count, values);
        return 1;
    }
    return 0;
}

int GFXShaderConstant4v(int name, unsigned int count, const float *values) {
    if (1
#ifndef __APPLE__
            && glUniform4fv_p
#endif
            ) {
        glUniform4fv_p(name, count, values);
        return 1;
    }
    return 0;
}

int GFXShaderConstanti(int name, int value) {
    if (1
#ifndef __APPLE__
            && glUniform1i_p
#endif
            ) {
        glUniform1i_p(name, value);
        return 1;
    }
    return 0;
}

int GFXShaderConstantv(int name, unsigned int count, const int *value) {
    if (1
#ifndef __APPLE__
            && glUniform1i_p
#endif
            ) {
        glUniform1iv_p(name, count, (GLint *) value);
        return 1;
    }
    return 0;
}

int GFXNamedShaderConstant(int progID, const char *name) {
    if (1
#ifndef __APPLE__
            && glGetUniformLocation_p
#endif
            ) {
        int varloc = glGetUniformLocation_p(progID, name);
        return varloc;
    }
    return -1;     //varloc cound be 0
}

int GFXNamedShaderConstant(char *progID, const char *name) {
    int programname = defaultprog;
    if (progID) {
        programname = programCache[cacheKey(progID, progID, NULL)];
    }
    return GFXNamedShaderConstant(programname, name);
}

int GFXGetProgramVersion() {
    return programVersion;
}
