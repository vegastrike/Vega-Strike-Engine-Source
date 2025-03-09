/*
 * setup.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; David Ranger specifically
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

#include "../include/central.h"
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/dir.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif
#include <vector>
#include <string>
#include <boost/program_options.hpp>

using std::string;
using std::vector;
char origpath[65536];

static int bogus_int; //added by chuck_starchaser to squash a warning or two
static char *bogus_str; //added by chuck_starchaser to squash a warning or two

static void changeToProgramDirectory(char *argv0) {
    int ret = -1;   /* Should it use argv[0] directly? */
    char *program = argv0;
#ifndef _WIN32
    char buf[65536];
    {
        char linkname[128];        /* /proc/<pid>/exe */
        linkname[0] = '\0';
        pid_t pid;

        /* Get our PID and build the name of the link in /proc */
        pid = getpid();

        sprintf(linkname, "/proc/%d/exe", pid);
        ret = readlink(linkname, buf, 65535);
        if (ret <= 0) {
            sprintf(linkname, "/proc/%d/file", pid);
            ret = readlink(linkname, buf, 65535);
        }
        if (ret <= 0) {
            ret = readlink(program, buf, 65535);
        }
        if (ret > 0) {
            buf[ret] = '\0';
            /* Ensure proper NUL termination */
            program = buf;
        }
    }
#endif
    char *parentdir;
    size_t pathlen = strlen(program); //TODO[String Safety] -- strlen assumes null terminated string
    parentdir = new char[pathlen + 1];
    char *c;
    strncpy(parentdir, program, pathlen + 1); // guarantees null termination due to zero-pad in strncpy //[MSVC-Warn]
    c = (char *) parentdir;
    while (*c != '\0') {      /* go to end */
        c++;
    }
    while ((*c != '/') && (*c != '\\') && c > parentdir) {        /* back up to parent */
        c--;
    }
    *c = '\0';             /* cut off last part (binary name) */
    if (strlen(parentdir) > 0) { // safe -- code above inserts null terminator
        bogus_int = chdir(parentdir);
    }          /* chdir to the binary app's parent */ //[MSVC-Warn]
    delete[] parentdir;
}

#if defined (_WINDOWS) && defined (_WIN32)
typedef char FileNameCharType[65536];
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd )
{
    char **argv;
    int  argc;
    argv = __argv; // did not realize this was actually still defined by MSVC
    argc = __argc; // same here

#else

int main(int argc, char *argv[]) {
#endif
    CONFIG.data_path = NULL;
    CONFIG.config_file = NULL;
    CONFIG.program_name = NULL;
    CONFIG.temp_file = NULL;

    bogus_str = getcwd(origpath, 65535); //[MSVC-Warn]
    origpath[65535] = 0;

    changeToProgramDirectory(argv[0]);
    {
        boost::program_options::options_description setup_options("Vega Strike setup utility");
        setup_options.add_options()
            ("target,D", boost::program_options::value<std::string>(), "Specify data directory, full path expected")
            ("help,h", "Show this help and exit")
            ;

        vector<string> data_paths;

        try {
            auto style = static_cast<boost::program_options::command_line_style::style_t>(boost::program_options::command_line_style::unix_style
                                                                                        | boost::program_options::command_line_style::case_insensitive);
            boost::program_options::variables_map vm;
            boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(setup_options).style(style).run(), vm);
            boost::program_options::notify(vm);

            if (vm.count("help")) {
                std::cout << setup_options << std::endl;
                return EXIT_SUCCESS;
            }

            if (vm.count("target")) {
                data_paths.push_back(vm["target"].as<std::string>());
                std::cout << "Set data directory to " << vm["target"].as<std::string>() << std::endl;
            } else {
                std::cerr << setup_options << std::endl;
                return EXIT_FAILURE;
            }
        } catch (const boost::program_options::error& e) {
            std::cerr << setup_options << std::endl;
            return EXIT_FAILURE;
        }

#ifdef DATA_DIR
        data_paths.push_back( DATA_DIR );
#endif
        data_paths.push_back(origpath);
        data_paths.push_back(string(origpath) + "/..");
        data_paths.push_back(string(origpath) + "/../data4.x");
        data_paths.push_back(string(origpath) + "/../../data4.x");
        data_paths.push_back(string(origpath) + "/data4.x");
        data_paths.push_back(string(origpath) + "/data");
        data_paths.push_back(string(origpath) + "/../data");
        data_paths.push_back(string(origpath) + "/../Resources");
        bogus_str = getcwd(origpath, 65535); //[MSVC-Warn]
        origpath[65535] = 0;
        data_paths.push_back(".");
        data_paths.push_back("..");
        data_paths.push_back("../data4.x");
        data_paths.push_back("../../data4.x");
        data_paths.push_back("../data");
        data_paths.push_back("../../data");
        data_paths.push_back("../Resources");
        data_paths.push_back("../Resources/data");
        data_paths.push_back("../Resources/data4.x");
/*
 *               data_paths.push_back( "/usr/share/local/vegastrike/data");
 *               data_paths.push_back( "/usr/local/share/vegastrike/data");
 *               data_paths.push_back( "/usr/local/vegastrike/data");
 *               data_paths.push_back( "/usr/share/vegastrike/data");
 *               data_paths.push_back( "/usr/local/games/vegastrike/data");
 *               data_paths.push_back( "/usr/games/vegastrike/data");
 *               data_paths.push_back( "/opt/share/vegastrike/data");
 *               data_paths.push_back( "/usr/share/local/vegastrike/data4.x");
 *               data_paths.push_back( "/usr/local/share/vegastrike/data4.x");
 *               data_paths.push_back( "/usr/local/vegastrike/data4.x");
 *               data_paths.push_back( "/usr/share/vegastrike/data4.x");
 *               data_paths.push_back( "/usr/local/games/vegastrike/data4.x");
 *               data_paths.push_back( "/usr/games/vegastrike/data4.x");
 *               data_paths.push_back( "/opt/share/vegastrike/data4.x");
 */
        //Win32 data should be "."
        for (vector<string>::iterator vsit = data_paths.begin(); vsit != data_paths.end(); vsit++) {
            //Test if the dir exist and contains config_file
            bogus_int = chdir(origpath); //[MSVC-Warn]
            bogus_int = chdir((*vsit).c_str()); //[MSVC-Warn]
            FILE *setupcfg;
            setupcfg = fopen("setup.config", "r"); //[MSVC-Warn]
            if (!setupcfg) {
                continue;
            }
            fclose(setupcfg);
            setupcfg = fopen("Version.txt", "r"); //[MSVC-Warn]
            if (!setupcfg) {
                continue;
            }
            bogus_str = getcwd(origpath, 65535); //[MSVC-Warn]
            origpath[65535] = 0;
            printf("Found data in %s\n", origpath);
            CONFIG.data_path =
                    strdup(origpath); //TODO[String Safety] -- future platform specific intrinsic options relevant here //[MSVC-Warn]
            break;
        }
    }
#ifndef _WIN32
    struct passwd *pwent;
    pwent = getpwuid(getuid());
    string HOMESUBDIR;
    FILE *version = fopen("Version.txt", "r");
    if (!version) {
        version = fopen("../Version.txt", "r");
    }
    if (version) {
        std::string hsd = "";
        int c;
        while ((c = fgetc(version)) != EOF) {
            if (isspace(c)) {
                break;
            }
            hsd += (char) c;
        }
        fclose(version);
        if (hsd.length()) {
            HOMESUBDIR = hsd;
        }
        //fprintf (STD_OUT,"Using %s as the home directory\n",hsd.c_str());
    }
    if (HOMESUBDIR.empty()) {
        fprintf(stderr, "Error: Failed to find Version.txt anywhere.\n");
        return 1;
    }
    bogus_int = chdir(pwent->pw_dir);

    mkdir(HOMESUBDIR.c_str(), 0755);
    bogus_int = chdir(HOMESUBDIR.c_str());
#endif
    Start(&argc, &argv);
    return 0;
}
