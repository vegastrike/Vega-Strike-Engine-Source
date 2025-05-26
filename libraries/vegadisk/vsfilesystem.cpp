/*
 * vsfilesystem.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Also: Nachum Barcohen, pyramid3d
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


#include "vegadisk/vsfilesystem.h"

#include <cstdio>
#include <cassert>
#include <cstdarg>
#include <exception>
#include <configuration/configuration.h>
#if defined (_WIN32) && !defined (__CYGWIN__)
#include <Shlobj.h>
#include <direct.h>
#include <cstring>
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <windows.h>
#include <cstdlib>
struct dirent
{
    char d_name[1];
};
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>
#endif
#include <sys/stat.h>
#include "root_generic/configxml.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "common/common.h"
#include "root_generic/galaxy_gen.h"
#include "root_generic/pk3.h"

#include "src/gnuhash.h"

#include "root_generic/options.h"

#include "root_generic/galaxy.h"

#include <boost/filesystem.hpp>

#include "configuration/game_config.h"
#include "src/vs_exit.h"

#include <string>
#include <utility>

// from main.cpp
extern bool legacy_data_dir_mode;

using VSFileSystem::VSVolumeType;
using VSFileSystem::VSFSNone;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

int VSFS_DEBUG() {
//    return 3;
    if (vs_config) {
        return (game_options()->debug_fs);
    }
    return 0;
}

char *CONFIGFILE;
const size_t VS_PATH_BUF_SIZE = 65536;
char pwd[VS_PATH_BUF_SIZE];
VSVolumeType isin_bigvolumes = VSFSNone;
std::string curmodpath{};

extern string GetUnitDir(string filename);

//std::vector<dirent *> sortTheDirectoryEntries(dirent **p_dirent, const size_t num_entries, int (*compar)(const dirent **, const dirent **));
std::string selectcurrentdir;

int selectdirs(const struct dirent *entry) {
    if (string(entry->d_name) == "." && string(entry->d_name) == "..") {
        return 0;
    }
    //Have to check if we have the full path or just relative (which would be a problem)
    std::string tmp = selectcurrentdir + '/' + entry->d_name;
    //cerr<<"Read directory entry : "<< tmp <<endl;
    struct stat s{};
    if (stat(tmp.c_str(), &s) < 0) {
        return 0;
    }
    if ((s.st_mode & S_IFDIR)) {
        return 1;
    }
    return 0;
}

#if defined (__FreeBSD__) || defined (__APPLE__)
int selectpk3s( struct dirent *entry )
#else

int selectpk3s(const struct dirent *entry)
#endif
{
    //If this is a regular file and we have ".pk3" in it
    if ((string(entry->d_name).find(".pk3")) != std::string::npos && (string(entry->d_name).find("data"))
            == std::string::npos) {
        return 1;
    }
    return 0;
}

#if defined (__FreeBSD__) || defined (__APPLE__)
int selectbigpk3s( struct dirent *entry )
#else

int selectbigpk3s(const struct dirent *entry)
#endif
{
    //If this is a regular file and we have ".pk3" in it
    if ((string(entry->d_name).find("data.pk3")) != std::string::npos) {
        return 1;
    }
    return 0;
}

namespace VSFileSystem {
std::string vegastrike_cwd;

VSError CachedFileLookup(FileLookupCache &cache, const std::string &file, VSFileType type) {
    std::string hashName = GetHashName(file);
    FileLookupCache::iterator it = cache.find(hashName);
    if (it != cache.end()) {
        return it->second;
    } else {
        return cache[hashName] = LookForFile(file, type);
    }
}

void DisplayType(VSFileSystem::VSFileType type) {
    DisplayType(type, std::cerr);
}

#define CASE(a) \
case a:                   \
    ostr<<#a; break;

void DisplayType(VSFileSystem::VSFileType type, std::ostream &ostr) {
    switch (type) {
        CASE(VSFileSystem::UniverseFile)
        CASE(VSFileSystem::SystemFile)
        CASE(VSFileSystem::CockpitFile)
        CASE(VSFileSystem::UnitFile)
        CASE(VSFileSystem::PythonFile)
        CASE(VSFileSystem::TextureFile)
        CASE(VSFileSystem::SoundFile)
        CASE(VSFileSystem::MeshFile)
        CASE(VSFileSystem::CommFile)
        CASE(VSFileSystem::AiFile)
        CASE(VSFileSystem::SaveFile)
        CASE(VSFileSystem::AnimFile)
        CASE(VSFileSystem::VideoFile)
        CASE(VSFileSystem::VSSpriteFile)
        CASE(VSFileSystem::MissionFile)
        CASE(VSFileSystem::MusicFile)
        CASE(VSFileSystem::AccountFile)
        CASE(VSFileSystem::ZoneBuffer)
        CASE(VSFileSystem::UnknownFile)
        default:
            ostr << "VSFileSystem::<undefined VSFileType>";
            break;
    }
}

#undef CASE

int GetReadBytes(char *fmt, va_list ap) {
    int ret = 0;

    cerr << "STARTING ARGLIST" << endl;
    while (*fmt) {
        switch (*fmt++) {
            case 'd':
                break;
            case 'f':
                break;
            case 's':
                break;
            case 'g':
                break;
            case 'n':                               //Number of bytes
                ret = va_arg(ap, int);
                cerr << "\tFound 'n' : " << ret << endl;
                break;
            default:
                cerr << "\tOther arg" << endl;
        }
    }
    cerr << "ENDING ARGLIST" << endl;
    return ret;
}

/*
 ***********************************************************************************************
 **** VSFileSystem global variables                                                          ***
 ***********************************************************************************************
 */

bool use_volumes;
std::string volume_format;
enum VSVolumeFormat q_volume_format;

std::vector<std::vector<std::string>>
        SubDirectories;                       //Subdirectories where we should look for VSFileTypes files
std::vector<std::string>
        Directories;                                        //Directories where we should look for VSFileTypes files
std::vector<std::string>
        Rootdir;                                                    //Root directories where we should look for VSFileTypes files
std::string sharedtextures;
std::string sharedunits;
std::string sharedsounds;
std::string sharedmeshes;
std::string sharedsectors;
std::string sharedcockpits;
std::string shareduniverse;
std::string aidir;
std::string sharedanims;
std::string sharedvideos;
std::string sharedsprites;
std::string savedunitpath;
std::string modname;
std::string moddir;
std::string programdir;
std::string datadir;
std::string homedir;

std::string config_file;
std::string weapon_list;
std::string universe_name;
std::string HOMESUBDIR(".vegastrike");
vector<string> current_path;
vector<string> current_directory;
vector<string> current_subdirectory;
vector<VSFileType> current_type;

//vs_path only stuff
vector<std::string> savedpwd;
vector<std::string> curdir;        //current dir starting from datadir
vector<std::vector<std::string> > savedcurdir;        //current dir starting from datadir

vector<int> UseVolumes;

std::string failed;

//Map of the currently opened PK3 volume/resource files
// FIXME: Clang-Tidy: Initialization of 'pk3_opened_files' with static storage duration may throw an exception that cannot be caught
vsUMap<std::string, CPK3 *> pk3_opened_files;

/*
 ***********************************************************************************************
 **** vs_path functions                                                                      ***
 ***********************************************************************************************
 */

std::string GetHashName(const std::string &name) {
    std::string result;
    result = current_path.back() + current_directory.back() + current_subdirectory.back() + name;
    return result;
}

std::string GetHashName(const std::string &name, const Vector &scale, int faction) {
    std::string result;
    result = current_path.back() + current_directory.back() + current_subdirectory.back() + name;

    result += XMLSupport::VectorToString(scale) + "|" + XMLSupport::tostring(faction);
    return result;
}

std::string GetSharedMeshHashName(const std::string &name, const Vector &scale, int faction) {
    return string("#") + XMLSupport::VectorToString(scale) + string("#") + name + string("#")
            + XMLSupport::tostring(faction);
}

std::string GetSharedTextureHashName(const std::string &name) {
    return string("#stex#") + name;
}

std::string GetSharedSoundHashName(const std::string &name) {
    return string("#ssnd#") + name;
}

std::string MakeSharedPathReturnHome(const std::string &newpath) {
    CreateDirectoryHome(newpath);
    return newpath + string("/");
}

std::string MakeSharedPath(const std::string &s) {
    VS_LOG(info, (boost::format("MakingSharedPath %1%") % s));
    return MakeSharedPathReturnHome(s) + s;
}

std::string MakeSharedStarSysPath(const std::string &s) {
    string syspath = sharedsectors + "/" + getStarSystemSector(s);
    return MakeSharedPathReturnHome(syspath) + s;
}

std::string GetCorrectStarSysPath(const std::string &name, bool &autogenerated) {
    autogenerated = false;
    if (name.length() == 0) {
        return string("");
    }
    string newname(name);

    VSFile f;
    VSError err = f.OpenReadOnly(newname, SystemFile);
    autogenerated = false;
    if (err <= Ok) {
        autogenerated = (err == Shared);
    }
    return newname;
}

/*
 ***********************************************************************************************
 **** VSFileSystem wrappers to stdio function calls                                          ***
 ***********************************************************************************************
 */

FILE *vs_open(const char *filename, const char *mode) {
    if (VSFS_DEBUG() > 1) {
        cerr << "-= VS_OPEN in mode " << mode << " =- ";
    }
    FILE *fp;
    string fullpath = homedir + "/" + string(filename);
    if (!use_volumes && (string(mode) == "rb" || string(mode) == "r")) {
        string output;
        fp = fopen(fullpath.c_str(), mode);
        if (!fp) {
            fullpath = string(filename);
            output += fullpath + "... NOT FOUND\n\tTry loading : " + fullpath;
            fp = fopen(fullpath.c_str(), mode);
        }
        if (!fp) {
            fullpath = datadir + "/" + string(filename);
            output += "... NOT FOUND\n\tTry loading : " + fullpath;
            fp = fopen(fullpath.c_str(), mode);
        }
        if (VSFS_DEBUG()) {
            if (fp) {
                if (VSFS_DEBUG() > 2) {
                    cerr << fullpath << " SUCCESS !!!" << endl;
                }
            } else {
                cerr << output << " NOT FOUND !!!" << endl;
            }
        }
    } else {
        fp = fopen(fullpath.c_str(), mode);
        if (fp) {
            if (VSFS_DEBUG() > 2) {
                cerr << fullpath << " opened for writing SUCCESS !!!" << endl;
            }
        } else if (VSFS_DEBUG()) {
            cerr << fullpath << " FAILED !!!" << endl;
        }
    }
    return fp;

    return nullptr;
}

size_t vs_read(void *ptr, size_t size, size_t nmemb, FILE *fp) {
    if (!use_volumes) {
        return fread(ptr, size, nmemb, fp);
    } else {
    }
    return 0;
}

size_t vs_write(const void *ptr, size_t size, size_t nmemb, FILE *fp) {
    if (!use_volumes) {
        return fwrite(ptr, size, nmemb, fp);
    } else {
    }
    return 0;
}

void vs_close(FILE *fp) {
    if (!use_volumes) {
        fclose(fp);
    } else {
    }
}

int vs_fprintf(FILE *fp, const char *format, ...) {
    if (!use_volumes) {
        va_list ap;
        va_start(ap, format);

        int retVal = vfprintf(fp, format, ap);
        va_end(ap);
        return retVal;
    } else {
    }
    return 0;
}

int vs_fseek(FILE *fp, long offset, int whence) {
    return fseek(fp, offset, whence);
}

long vs_ftell(FILE *fp) {
    return ftell(fp);
}

void vs_rewind(FILE *fp) {
    rewind(fp);
}

bool vs_feof(FILE *fp) {
    return feof(fp);
}

long vs_getsize(FILE *fp) {
    if (!use_volumes) {
        struct stat st{};
        if (fstat(fileno(fp), &st) == 0) {
            return st.st_size;
        }
        return -1;
    }
    return -1;
}

/*
 ***********************************************************************************************
 **** VSFileSystem functions                                                                 ***
 ***********************************************************************************************
 */

#ifdef WIN32
void InitHomeDirectory()
{
    std::string userdir;
    PWSTR pszPath = nullptr;
    char mbcsPath[VS_PATH_BUF_SIZE];
    size_t pathSize = 0;
    errno_t conversionResult = 0;

    HRESULT getPathResult = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE | KF_FLAG_NO_ALIAS, nullptr, &pszPath);
    if (SUCCEEDED(getPathResult)) {
        conversionResult = wcstombs_s(&pathSize, mbcsPath, pszPath, VS_PATH_BUF_SIZE - 1);
        CoTaskMemFree(pszPath);
        if (conversionResult == 0) {
            userdir = mbcsPath;
        } else {
            VS_LOG_AND_FLUSH(fatal, "!!! Fatal Error converting user's home directory to MBCS");
            VSExit(1);
        }
    } else {
        VS_LOG_AND_FLUSH(fatal, "!!! Fatal Error getting user's home directory");
        CoTaskMemFree(pszPath);
        VSExit(1);
    }

    boost::filesystem::path home_path{userdir};
    boost::filesystem::path home_sub_path{HOMESUBDIR};
    boost::filesystem::path absolute_home_path{boost::filesystem::absolute(home_sub_path, home_path)};
    homedir = absolute_home_path.string();
    CreateDirectoryAbs( homedir );

    VS_LOG(info, (boost::format("USING HOMEDIR : %1% as the home directory ") % homedir));
}
#else

void InitHomeDirectory() {
    //Setup home directory
    char *chome_path = NULL;
    struct passwd *pwent;
    pwent = getpwuid(getuid());
    chome_path = pwent->pw_dir;
    if (!DirectoryExists(chome_path)) {
        VS_LOG_AND_FLUSH(fatal, "!!! ERROR : home directory not found");
        VSExit(1);
    }
    string user_home_path(chome_path);
    homedir = user_home_path + "/" + HOMESUBDIR;

    VS_LOG(info, (boost::format("USING HOMEDIR : %1% As the home directory ") % homedir));
    CreateDirectoryAbs(homedir);
}

#endif

void InitDataDirectory() {
    std::vector<std::string> data_paths;

    /* commandline-specified paths come first */
    if (!datadir.empty()) {
        data_paths.push_back(datadir);
    }

    if (true == legacy_data_dir_mode) {
#ifdef WIN32
        // TODO: push back the following:
        //      %{programdata}%/Vegastrike
        //      %{programfiles}%/Vegastrike
        //      %{programfiles(x86)}%/Vegastrike
        //      %{programfilesw6432}%/Vegastrike
        //      %{commonprogramfiles}%/Vegastrike
        //      %{appdata%}
        // data_paths.push_back();
#else
        data_paths.push_back("/usr/share/vegastrike");
#endif
    }

    /* DATA_DIR should no longer be necessary--it will either use the path
     *  to the binary, or the current directory. */
#ifdef DATA_DIR
    data_paths.push_back( DATA_DIR );
#endif
    if (!vegastrike_cwd.empty()) {
        data_paths.push_back(vegastrike_cwd);
        data_paths.push_back(vegastrike_cwd + "/..");
        data_paths.push_back(vegastrike_cwd + "/../../data");
        data_paths.push_back(vegastrike_cwd + "/data");
        data_paths.push_back(vegastrike_cwd + "/../data");
        data_paths.push_back(vegastrike_cwd + "/../Resources");
        data_paths.push_back(vegastrike_cwd + "/Assets-Production");
        data_paths.push_back(vegastrike_cwd + "/../Assets-Production");
        data_paths.push_back(vegastrike_cwd + "/../../Assets-Production");
    }
    data_paths.emplace_back(".");
    data_paths.emplace_back("..");
    data_paths.emplace_back("../data");
    data_paths.emplace_back("../../data");
    data_paths.emplace_back("../Resources");
    data_paths.emplace_back("../Resources/data");
    data_paths.emplace_back("Assets-Production");
    data_paths.emplace_back("../Assets-Production");
    data_paths.emplace_back("../../Assets-Production");

    //Win32 data should be "."
    char tmppath[VS_PATH_BUF_SIZE];
    for (auto & data_path : data_paths) {
        //Test if the dir exist and contains config_file
        if (FileExists(data_path, config_file) >= 0) {
            VS_LOG(info, (boost::format("Found data in %1%") % data_path));
            if (nullptr != getcwd(tmppath, VS_PATH_BUF_SIZE - 1)) {
                if (data_path.substr(0, 1) == ".") {
                    datadir = string(tmppath) + "/" + data_path;
                } else {
                    datadir = data_path;
                }
            } else {
                VS_LOG(error, "Cannot get current path: path too long");
            }

            if (chdir(datadir.c_str()) < 0) {
                VS_LOG_AND_FLUSH(fatal, "Error changing to datadir");
                VSExit(1);
            }
            if (nullptr != getcwd(tmppath, VS_PATH_BUF_SIZE - 1)) {
                datadir = string(tmppath);
            } else {
                VS_LOG(error, "Cannot get current path: path too long");
            }

            VS_LOG(info, (boost::format("Using %1% as data directory") % datadir));
            break;
        }
    }
    data_paths.clear();
    string versionloc = datadir + "/Version.txt";
    FILE *version = fopen(versionloc.c_str(), "r");
    if (!version) {
        versionloc = datadir + "Version.txt";
        version = fopen(versionloc.c_str(), "r");
    }
    if (!version) {
        version = fopen("Version.txt", "r");
    }
    if (version) {
        std::string hsd;
        int c;
        while ((c = fgetc(version)) != EOF) {
            if (isspace(c)) {
                break;
            }
            hsd += static_cast<char>(c);
        }
        fclose(version);
        if (!hsd.empty()) {
            HOMESUBDIR = hsd;
            VS_LOG(info, (boost::format("Using %1% as the home directory") % hsd));
        }
    }
    //Get the mods path
    moddir = datadir + "/" + string("mods");
    VS_LOG(info, (boost::format("Found MODDIR = %1%") % moddir));
}

//Config file has been loaded from data dir but now we look at the specified moddir in order
//to see if we should use a mod config file
void LoadConfig(string subdir) {
    bool found = false;
    bool foundweapons = false;
    //First check if we have a config file in homedir+"/"+subdir or in datadir+"/"+subdir
    weapon_list = "weapons.json";
    if (!subdir.empty()) {
        modname = subdir;
        if (DirectoryExists(homedir + "/mods/" + subdir)) {
            if (FileExists(homedir + "/mods/" + subdir, config_file) >= 0) {
                VS_LOG(info,
                        (boost::format("CONFIGFILE - Found a config file in home mod directory, using : %1%")
                                % (homedir + "/mods/" + subdir + "/" + config_file)));
                if (FileExists(homedir + "/mods/" + subdir, "weapons.json") >= 0) {
                    weapon_list = homedir + "/mods/" + subdir + "/weapons.json";
                    foundweapons = true;
                }
                config_file = homedir + "/mods/" + subdir + "/" + config_file;
                found = true;
            }
        }
        if (!found) {
            VS_LOG(warning, (boost::format("WARNING : couldn't find a mod named '%1%' in homedir/mods") % subdir));
        }
        if (DirectoryExists(moddir + "/" + subdir)) {
            if (FileExists(moddir + "/" + subdir, config_file) >= 0) {
                if (!found) {
                    VS_LOG(info,
                            (boost::format("CONFIGFILE - Found a config file in mods directory, using : %1%")
                                    % (moddir + "/" + subdir + "/" + config_file)));
                }
                if ((!foundweapons) && FileExists(moddir + "/" + subdir, "weapons.json") >= 0) {
                    weapon_list = moddir + "/" + subdir + "/weapons.json";
                    foundweapons = true;
                }
                if (!found) {
                    config_file = moddir + "/" + subdir + "/" + config_file;
                }
                found = true;
            }
        } else {
            VS_LOG(error, (boost::format("ERROR : couldn't find a mod named '%1%' in datadir/mods") % subdir));
        }
        //}
    }
    if (!found) {
        //Next check if we have a config file in homedir if we haven't found one for mod
        if (FileExists(homedir, config_file) >= 0) {
            VS_LOG(info,
                    (boost::format("CONFIGFILE - Found a config file in home directory, using : %1%")
                            % (homedir + "/" + config_file)));
            config_file = homedir + "/" + config_file;
        } else {
            VS_LOG(info, (boost::format("CONFIGFILE - No config found in home : %1%") % (homedir + "/" + config_file)));
            if (FileExists(datadir, config_file) >= 0) {
                VS_LOG(info,
                        (boost::format("CONFIGFILE - No home config file found, using datadir config file : %1%")
                                % (datadir + "/" + config_file)));
                //We didn't find a config file in home_path so we load the data_path one
                config_file = datadir + "/" + config_file;
            } else {
                VS_LOG_AND_FLUSH(fatal,
                        (boost::format("CONFIGFILE - No config found in data dir : %1%")
                                % (datadir + "/" + config_file)));
                VS_LOG_AND_FLUSH(fatal, "CONFIG FILE NOT FOUND !!!");
                VSExit(1);
            }
        }
    } else if (!subdir.empty()) {
        VS_LOG(info, (boost::format("Using Mod Directory %1%") % moddir));
        CreateDirectoryHome("mods");
        CreateDirectoryHome("mods/" + subdir);
        homedir = homedir + "/mods/" + subdir;
    }
    //Delete the default config in order to reallocate it with the right one (if it is a mod)
    if (vs_config) {
        VS_LOG(info, "reallocating vs_config ");
        delete vs_config;
    }

    // This is a replacement for the old config xml files
    vega_config::GetGameConfig().LoadGameConfig(config_file);

    vs_config = createVegaConfig(config_file.c_str());

    string universe_file = datadir + "/" \
 + vs_config->getVariable("data", "universe_path", "universe") + "/" \
 + vs_config->getVariable("general", "galaxy", "milky_way.xml");
    VS_LOG(debug, (boost::format("Force galaxy to %1%") % universe_file));
    try {
        Galaxy galaxy = Galaxy(universe_file);
    } catch (std::exception &e) {
        VS_LOG_AND_FLUSH(fatal,
                (boost::format(
                        "Error while loading configuration. Did you specify the asset directory? Error: %1%")
                        % e.what()));
        VSExit(1);
    }
}

void InitMods() {
    string curpath;
    struct dirent **dirlist;
    //new config program should insert hqtextures variable
    //with value "hqtextures" in data section.
    if (!game_options()->hqtextures.empty()) {
        //HQ Texture dir sits alongside data dir.
        selectcurrentdir = datadir + "/..";
        boost::filesystem::path p(selectcurrentdir);
        for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(p)) {
            const boost::filesystem::path& filename = entry.path().filename();
            const std::string filename_string = filename.string();
            if (filename_string == game_options()->hqtextures && is_directory(entry.status())) {
                curpath.clear();
                curpath.append(selectcurrentdir);
                curpath.append("/");
                curpath.append(filename_string);
                VS_LOG(important_info, "\n\nAdding HQ Textures Pack\n\n");
                Rootdir.push_back(curpath);
            }
        }

//        int ret = scandir(selectcurrentdir.c_str(), &dirlist, selectdirs, nullptr);
//        if (ret >= 0) {
//            while (ret--) {
//                string dname(dirlist[ret]->d_name);
//                if (dname == game_options()->hqtextures) {
//                    curpath.clear();
//                    curpath.append(selectcurrentdir);
//                    curpath.append("/");
//                    curpath.append(dname);
//                    VS_LOG(info, "\n\nAdding HQ Textures Pack\n\n");
//                    Rootdir.push_back(curpath);
//                }
//            }
//        }
//        free(dirlist);
    }

    selectcurrentdir = moddir;
    boost::filesystem::path mod_path(selectcurrentdir);
    if (exists(mod_path)) {
        for (boost::filesystem::directory_entry &entry : boost::filesystem::directory_iterator(mod_path)) {
            const boost::filesystem::path &filename = entry.path().filename();
            const std::string filename_string = filename.string();
            if (filename_string == modname && is_directory(entry.status())) {
                curpath.clear();
                curpath.append(selectcurrentdir);
                curpath.append("/");
                curpath.append(filename_string);
                VS_LOG(important_info, (boost::format("Adding mod path : %1%") % curpath));
                Rootdir.push_back(curpath);
            }
        }
    }

//    int ret = scandir(selectcurrentdir.c_str(), &dirlist, selectdirs, nullptr);
//    if (ret < 0) {
//        return;
//    } else {
//        while (ret--) {
//            string dname(dirlist[ret]->d_name);
//            if (dname == modname) {
//                curpath.clear();
//                curpath.append(moddir);
//                curpath.append("/");
//                curpath.append(dname);
//                VS_LOG(info, (boost::format("Adding mod path : %1%") % curpath));
//                Rootdir.push_back(curpath);
//            }
//        }
//    }
//    free(dirlist);

//Scan for mods with standard data subtree
    curmodpath = homedir + "/mods/";
    selectcurrentdir = curmodpath;
    boost::filesystem::path mods_subtree_path(selectcurrentdir);
    if (exists(mods_subtree_path)) {
        for (boost::filesystem::directory_entry &entry : boost::filesystem::directory_iterator(mods_subtree_path)) {
            const boost::filesystem::path &filename = entry.path().filename();
            const std::string filename_string = filename.string();
            if (filename_string == modname && is_directory(entry.status())) {
                curpath = curmodpath + filename_string;
                VS_LOG(important_info, (boost::format("Adding mod path : %1%") % curpath));
                Rootdir.push_back(curpath);
            }
        }
    }

//    ret = scandir(selectcurrentdir.c_str(), &dirlist, selectdirs, nullptr);
//    if (ret < 0) {
//        return;
//    } else {
//        while (ret--) {
//            string dname(dirlist[ret]->d_name);
//            if (dname == modname) {
//                curpath = curmodpath + dname;
//                VS_LOG(info, (boost::format("Adding mod path : %1%") % curpath));
//                Rootdir.push_back(curpath);
//            }
//        }
//    }
//    free(dirlist);
}

void InitPaths(string conf, string subdir) {
    config_file = std::move(conf);

    current_path.emplace_back("");
    current_directory.emplace_back("");
    current_subdirectory.emplace_back("");
    current_type.push_back(UnknownFile);

    int i;
    for (i = 0; i <= UnknownFile; ++i) {
        UseVolumes.push_back(0);
    }
    /************************** Data and home directory settings *************************/

    InitDataDirectory();
    InitHomeDirectory();
    /*
      Paths relative to datadir or homedir (both should have the same structure)
      Units are in sharedunits/unitname/, sharedunits/subunits/unitname/ or sharedunits/weapons/unitname/ or in sharedunits/faction/unitname/
      Meshes are in sharedmeshes/ or in current unit that is being loaded
      Textures are in sharedtextures/ or in current unit dir that is being loaded or in the current animation dir that is being loaded or in the current sprite dir that is being loeded
      Sounds are in sharedsounds/
      Universes are in universe/
      Systems are in "sectors"/ config variable
      Cockpits are in cockpits/ (also a config var)
      Animations are in animations/
      VSSprite are in sprites/ or in ./ (when full subpath is provided) or in the current cockpit dir that is being loaded
      First allocate an empty directory list for each file type
    */
    for (i = 0; i < UnknownFile; i++) {
        vector<string> vec;
        Directories.emplace_back("");
        SubDirectories.push_back(vec);
    }

    boost::filesystem::path config_file_path{datadir + "/config.json"};
    configuration()->load_config(config_file_path);
    boost::filesystem::path config_file_path2{homedir + "/config.json"};
    configuration()->load_config(config_file_path2);

    LoadConfig(std::move(subdir));

    game_options()->init();

    sharedsectors = game_options()->sectors;
    sharedcockpits = game_options()->cockpits;
    shareduniverse = game_options()->universe_path;
    sharedanims = game_options()->animations;
    sharedvideos = game_options()->movies;
    sharedsprites = game_options()->sprites;
    savedunitpath = game_options()->serialized_xml;
    sharedtextures = game_options()->sharedtextures;
    sharedsounds = game_options()->sharedsounds;
    sharedmeshes = game_options()->sharedmeshes;
    sharedunits = game_options()->sharedunits;
    aidir = game_options()->ai_directory;
    universe_name = game_options()->galaxy;

    //Setup the directory lists we know about - note these are relative paths to datadir or homedir
    //----- THE Directories vector contains the resource/volume files name without extension or the main directory to files
    Directories[UnitFile] = sharedunits;
    //Have to put it in first place otherwise VS will find default unit file
    SubDirectories[UnitFile].emplace_back("subunits");
    SubDirectories[UnitFile].emplace_back("weapons");
    SubDirectories[UnitFile].emplace_back("");
    SubDirectories[UnitFile].emplace_back("factions/planets");
    SubDirectories[UnitFile].emplace_back("factions/upgrades");
    SubDirectories[UnitFile].emplace_back("factions/neutral");
    SubDirectories[UnitFile].emplace_back("factions/aera");
    SubDirectories[UnitFile].emplace_back("factions/confed");
    SubDirectories[UnitFile].emplace_back("factions/pirates");
    SubDirectories[UnitFile].emplace_back("factions/rlaan");

    Directories[UnitSaveFile] = savedunitpath;

    Directories[MeshFile] = sharedmeshes;
    SubDirectories[MeshFile].emplace_back("mounts");
    SubDirectories[MeshFile].emplace_back("nav/default");

    Directories[TextureFile] = sharedtextures;
    SubDirectories[TextureFile].emplace_back("mounts");
    SubDirectories[TextureFile].emplace_back("nav/default");

    //We will also look in subdirectories with universe name
    Directories[SystemFile] = sharedsectors;
    SubDirectories[SystemFile].push_back(universe_name);

    Directories[UniverseFile] = shareduniverse;
    Directories[SoundFile] = sharedsounds;
    Directories[CockpitFile] = sharedcockpits;
    Directories[AnimFile] = sharedanims;
    Directories[VideoFile] = sharedvideos;
    Directories[VSSpriteFile] = sharedsprites;

    Directories[AiFile] = aidir;
    SubDirectories[AiFile].emplace_back("events");
    SubDirectories[AiFile].emplace_back("script");

    Directories[MissionFile] = "mission";
    Directories[CommFile] = "communications";
    Directories[SaveFile] = "save";
    Directories[MusicFile] = "music";
    Directories[PythonFile] = "bases";
    Directories[AccountFile] = "accounts";

    SIMULATION_ATOM = configuration()->general.simulation_atom;
    simulation_atom_var = SIMULATION_ATOM;
    AUDIO_ATOM = configuration()->general.audio_atom;
    audio_atom_var = AUDIO_ATOM;
    VS_LOG(info, (boost::format("SIMULATION_ATOM: %1%") % SIMULATION_ATOM));

    /************************* Home directory subdirectories creation ************************/
    CreateDirectoryHome(savedunitpath);
    CreateDirectoryHome(sharedtextures);
    CreateDirectoryHome(sharedtextures + "/backgrounds");
    CreateDirectoryHome(sharedsectors);
    CreateDirectoryHome(sharedsectors + "/" + universe_name);
    CreateDirectoryHome(sharedsounds);
    CreateDirectoryHome("save");

    //We will be able to automatically add mods files (set of resources or even directory structure similar to the data tree)
    //by just adding a subdirectory named with the mod name in the subdirectory "mods"...
    //I just have to implement that and then add all mods/ subdirs in Rootdir vector
    Rootdir.push_back(homedir);
    InitMods();
    Rootdir.push_back(datadir);

    //NOTE : UniverseFiles cannot use volumes since some are needed by python
    //Also : Have to try with systems, not sure it would work well
    //Setup the use of volumes for certain VSFileType
    volume_format = game_options()->volume_format;
    if (volume_format == "vsr") {
        q_volume_format = vfmtVSR;
    } else if (volume_format == "pk3") {
        q_volume_format = vfmtPK3;
    } else {
        q_volume_format = vfmtUNK;
    }
    if (FileExists(datadir, "/data." + volume_format) >= 0) {
        //Every kind of file will use the big volume except Unknown files and python files that needs to remain standard files
        for (i = 0; i < UnknownFile; i++) {
            UseVolumes[i] = 2;
        }
        UseVolumes[PythonFile] = 0;
        UseVolumes[AccountFile] = 0;
    } else {
        if (FileExists(datadir, "/" + sharedunits + "." + volume_format) >= 0) {
            UseVolumes[UnitFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/" + sharedunits) % volume_format));
        }
        if (FileExists(datadir, "/" + sharedmeshes + "." + volume_format) >= 0) {
            UseVolumes[MeshFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/" + sharedmeshes) % volume_format));
        }
        if (FileExists(datadir, "/" + sharedtextures + "." + volume_format) >= 0) {
            UseVolumes[TextureFile] = 1;
            VS_LOG(info,
                    (boost::format("Using volume file %1%.%2%") % (datadir + "/" + sharedtextures) % volume_format));
        }
        if (FileExists(datadir, "/" + sharedsounds + "." + volume_format) >= 0) {
            UseVolumes[SoundFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/" + sharedsounds) % volume_format));
        }
        if (FileExists(datadir, "/" + sharedcockpits + "." + volume_format) >= 0) {
            UseVolumes[CockpitFile] = 1;
            VS_LOG(info,
                    (boost::format("Using volume file %1%.%2%") % (datadir + "/" + sharedcockpits) % volume_format));
        }
        if (FileExists(datadir, "/" + sharedsprites + "." + volume_format) >= 0) {
            UseVolumes[VSSpriteFile] = 1;
            VS_LOG(info,
                    (boost::format("Using volume file %1%.%2%") % (datadir + "/" + sharedsprites) % volume_format));
        }
        if (FileExists(datadir, "/animations." + volume_format) >= 0) {
            UseVolumes[AnimFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/animations") % volume_format));
        }
        if (FileExists(datadir, "/movies." + volume_format) >= 0) {
            UseVolumes[VideoFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/movies") % volume_format));
        }
        if (FileExists(datadir, "/communications." + volume_format) >= 0) {
            UseVolumes[CommFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/communications") % volume_format));
        }
        if (FileExists(datadir, "/mission." + volume_format) >= 0) {
            UseVolumes[MissionFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/mission") % volume_format));
        }
        if (FileExists(datadir, "/ai." + volume_format) >= 0) {
            UseVolumes[AiFile] = 1;
            VS_LOG(info, (boost::format("Using volume file %1%.%2%") % (datadir + "/ai") % volume_format));
        }
        UseVolumes[ZoneBuffer] = 0;
    }
}

void CreateDirectoryAbs(const char *filename) {
    int err;
    if (!DirectoryExists(filename)) {
        err = mkdir(filename
#if !defined (_WIN32) || defined (__CYGWIN__)
                , 0xFFFFFFFF
#endif
        );
        if (err < 0 && errno != EEXIST) {
            VS_LOG_AND_FLUSH(fatal, (boost::format("Errno=%1% - FAILED TO CREATE : %2%") % errno % filename));
            GetError("CreateDirectory");
            VSExit(1);
        }
    }
}

void CreateDirectoryAbs(const string &filename) {
    CreateDirectoryAbs(filename.c_str());
}

void CreateDirectoryHome(const char *filename) {
    CreateDirectoryAbs(homedir + "/" + string(filename));
}

void CreateDirectoryHome(const string &filename) {
    CreateDirectoryHome(filename.c_str());
}

void CreateDirectoryData(const char *filename) {
    CreateDirectoryAbs(datadir + "/" + string(filename));
}

void CreateDirectoryData(const string &filename) {
    CreateDirectoryData(filename.c_str());
}

//Absolute directory -- DO NOT USE FOR TESTS IN VOLUMES !!
bool DirectoryExists(const char *filename) {
    struct stat s{};
    if (stat(filename, &s) < 0) {
        return false;
    }
    if (s.st_mode & S_IFDIR) {
        return true;
    }
    return false;
}

bool DirectoryExists(const string &filename) {
    return DirectoryExists(filename.c_str());
}

//root is the path to the type directory or the type volume
//filename is the subdirectory+"/"+filename
int FileExists(const string &root, const char *filename, VSFileType type, bool lookinvolume) {
    int found = -1;
    bool volok = false;
    string fullpath;
    const char *file;
    if (filename[0] == '/') {
        file = filename + 1;
    } else {
        file = filename;
    }
    const char *rootsep = (root.empty() || root == "/") ? "" : "/";
    if (!UseVolumes[type] || !lookinvolume) {
        if (type == UnknownFile) {
            fullpath = root + rootsep + file;
        } else {
            fullpath = root + rootsep + Directories[type] + "/" + file;
        }
        struct stat s{};
        if (stat(fullpath.c_str(), &s) >= 0) {
            if (s.st_mode & S_IFDIR) {
                VS_LOG(error, " File is a directory ! ");
                found = -1;
            } else {
                isin_bigvolumes = VSFSNone;
                found = 1;
            }
        }
        //}
    } else {
        if (q_volume_format == vfmtVSR) {
        } else if (q_volume_format == vfmtPK3) {
            CPK3 *vol;
            string filestr;

            //TRY TO OPEN A DATA.VOLFORMAT FILE IN THE ROOT DIRECTORY PASSED AS AN ARG
            filestr = Directories[type] + "/" + file;
            fullpath = root + rootsep + "data." + volume_format;
            vsUMap<string, CPK3 *>::iterator it;
            it = pk3_opened_files.find(fullpath);
            failed += "Looking for file in VOLUME : " + fullpath + "... ";
            if (it == pk3_opened_files.end()) {
                //File is not opened so we open it and add it in the pk3 file map
                vol = new CPK3;
                if ((volok = vol->Open(fullpath.c_str()))) {
                    failed += " VOLUME OPENED\n";
                    //We add the resource file to the map only if we could have opened it
                    std::pair<std::string, CPK3 *> pk3_pair(fullpath, vol);
                    pk3_opened_files.insert(pk3_pair);
                } else {
                    failed += " COULD NOT OPEN VOLUME\n";
                }
            } else {
                failed += " VOLUME FOUND\n";
                vol = it->second;
                volok = true;
            }
            //Try to get the file index in the archive
            if (volok) {
                found = vol->FileExists(filestr.c_str());
                if (found >= 0) {
                    isin_bigvolumes = VSFSBig;
                }
            } else {
                found = -1;
            }
            if (found < 0) {
                //AND THEN A VOLUME FILE BASED ON DIRECTORIES[TYPE]
                filestr = string(file);
                fullpath = root + rootsep + Directories[type] + "." + volume_format;
                it = pk3_opened_files.find(fullpath);
                failed += "Looking for file in VOLUME : " + fullpath + "... ";
                if (it == pk3_opened_files.end()) {
                    //File is not opened so we open it and add it in the pk3 file map
                    vol = new CPK3;
                    if ((volok = vol->Open(fullpath.c_str()))) {
                        failed += " VOLUME OPENED\n";
                        //We add the resource file to the map only if we could have opened it
                        std::pair<std::string, CPK3 *> pk3_pair(fullpath, vol);
                        pk3_opened_files.insert(pk3_pair);
                    } else {
                        failed += " COULD NOT OPEN VOLUME\n";
                    }
                } else {
                    failed += " VOLUME FOUND\n";
                    vol = it->second;
                    volok = true;
                }
                //Try to get the file index in the archive
                if (volok) {
                    found = vol->FileExists(filestr.c_str());
                    if (found >= 0) {
                        isin_bigvolumes = VSFSSplit;
                    }
                } else {
                    found = -1;
                }
            }
        }
    }
    if (found < 0) {
        if (!UseVolumes[type]) {
            failed += "\tTRY LOADING : " + nameof(type) + " " + fullpath + "... NOT FOUND\n";
        } else if (VSFS_DEBUG() > 1) {
            failed += "\tTRY LOADING in " + nameof(type) + " " + fullpath + " : " + file + "... NOT FOUND\n";
        }
    } else {
        if (!UseVolumes[type]) {
            failed = "\tTRY LOADING : " + nameof(type) + " " + fullpath + "... SUCCESS";
        } else if (VSFS_DEBUG() > 1) {
            failed = "\tTRY LOADING in " + nameof(type) + " " + fullpath + " : " + file + "... SUCCESS";
        } else {
            failed.erase();
        }
    }
    return found;
}

int FileExists(const string &root, const string &filename, VSFileType type, bool lookinvolume) {
    return FileExists(root, filename.c_str(), type, lookinvolume);
}

int FileExistsData(const char *filename, VSFileType type) {
    return FileExists(datadir, filename, type);
}

int FileExistsData(const string &filename, VSFileType type) {
    return FileExists(datadir, filename, type);
}

int FileExistsHome(const char *filename, VSFileType type) {
    return FileExists(homedir, filename, type);
}

int FileExistsHome(const string &filename, VSFileType type) {
    return FileExists(homedir, filename, type);
}

VSError GetError(const char *str) {
    std::string prefix = "!!! ERROR/WARNING VSFile : ";
    if (str) {
        prefix += "on ";
        prefix += str;
        prefix += " : ";
    }
    if (errno == ENOENT) {
        VS_LOG(error, (prefix + "File not found"));
        return FileNotFound;
    } else if (errno == EPERM) {
        VS_LOG(error, (prefix + "Permission denied"));
        return LocalPermissionDenied;
    } else if (errno == EACCES) {
        VS_LOG(error, (prefix + "Access denied"));
        return LocalPermissionDenied;
    } else {
        VS_LOG(error, (prefix + "Unspecified error (maybe to document in VSFile ?)"));
        return Unspecified;
    }
}

VSError LookForFile(const string &file, VSFileType type, VSFileMode mode) {
    VSFile vsfile;
    vsfile.SetFilename(file);
    VSError err = LookForFile(vsfile, type, mode);
    return err;
}

VSError LookForFile(VSFile &f, VSFileType type, VSFileMode mode) {
    int found = -1;
    bool shared = false;
    string filepath;
    string curpath;
    string dir;
    string extra;
    string subdir;
    failed.erase();
    VSFileType curtype = type;
    //First try in the current path
    switch (type) {
        case UnitFile:
            extra = "/" + GetUnitDir(f.GetFilename());
            break;
        case CockpitFile:
            //For cockpits we look in subdirectories that have the same name as the cockpit itself
            extra = "/" + string(f.GetFilename());
            break;
        case AnimFile:
            //Animations are always in subdir named like the anim itself
            extra += "/" + f.GetFilename();
            break;
        case UniverseFile:
        case SystemFile:
        case UnitSaveFile:
        case TextureFile:
        case SoundFile:
        case PythonFile:
        case MeshFile:
        case CommFile:
        case AiFile:
        case SaveFile:
        case VideoFile:
        case VSSpriteFile:
        case MissionFile:
        case MusicFile:
        case AccountFile:
        case ZoneBuffer:
        case JPEGBuffer:
        case UnknownFile:
            break;
    }
    //This test lists all the VSFileType that should be looked for in the current directory
    unsigned int i = 0, j = 0;
    for (int LC = 0; LC < 2 && found < 0; (LC += (extra.empty() ? 2 : 1)), extra = "") {
        if (!current_path.back().empty()
                && (type == TextureFile || type == MeshFile || type == VSSpriteFile || type == AnimFile
                        || type == VideoFile)) {
            for (i = 0; found < 0 && i < Rootdir.size(); i++) {
                curpath = Rootdir[i];
                subdir = current_subdirectory.back();
                if (!extra.empty()) {
                    subdir += extra;
                }
                curtype = current_type.back();
                found = FileExists(curpath, (subdir + "/" + f.GetFilename()).c_str(), curtype);
                if (found >= 0) {
                    shared = false;
                    f.SetAltType(curtype);
                } else {
                    //Set curtype back to original type if we didn't find the file in the current dir

                    curtype = type;
                    shared = true;
                }
            }
        }
        //FIRST LOOK IN HOMEDIR FOR A STANDARD FILE, SO WHEN USING VOLUME WE DO NOT LOOK FIRST IN VOLUMES
        if (found < 0 && UseVolumes[curtype]) {
            curpath = homedir;
            subdir = "";
            if (!extra.empty()) {
                subdir += extra;
            }
            found = FileExists(curpath, (subdir + "/" + f.GetFilename()).c_str(), type, false);
            for (j = 0; found < 0 && j < SubDirectories[curtype].size(); j++) {
                subdir = SubDirectories[curtype][j];
                if (!extra.empty()) {
                    subdir += extra;
                }
                found = FileExists(curpath, (subdir + "/" + f.GetFilename()).c_str(), curtype, false);
                f.SetVolume(VSFSNone);
            }
        }
        //THEN LOOK IN ALL THE REGISTERED ROOT DIRS
        for (i = 0; found < 0 && i < Rootdir.size(); i++) {
            curpath = Rootdir[i];
            subdir = f.GetSubDirectory();
            if (!extra.empty()) {
                subdir += extra;
            }
            found = FileExists(curpath, (subdir + "/" + f.GetFilename()).c_str(), curtype);
            for (j = 0; found < 0 && j < SubDirectories[curtype].size(); j++) {
                curpath = Rootdir[i];
                subdir = SubDirectories[curtype][j];
                if (f.GetSubDirectory().length()) {
                    if (subdir.length()) {
                        subdir += "/";
                    }
                    subdir += f.GetSubDirectory();
                }
                if (!extra.empty()) {
                    subdir += extra;
                }
                found = FileExists(curpath, (subdir + "/" + f.GetFilename()).c_str(), curtype);
            }
        }
        if (curtype == CockpitFile) {
            for (i = 0; found < 0 && i < Rootdir.size(); i++) {
                curpath = Rootdir[i];
                subdir = "";
                found = FileExists(curpath, (subdir + "/" + f.GetFilename()).c_str(), type);
                for (j = 0; found < 0 && j < SubDirectories[curtype].size(); j++) {
                    curpath = Rootdir[i];
                    subdir = SubDirectories[curtype][j];
                    if (!extra.empty()) {
                        subdir += extra;
                    }
                    found = FileExists(curpath, (subdir + "/" + f.GetFilename()).c_str(), curtype);
                }
            }
        }
    }
    if (VSFS_DEBUG() > 1) {
        if (isin_bigvolumes > VSFSNone) {
            cerr << failed << " - INDEX=" << found << endl << endl;
        } else {
            cerr << failed << endl;
        }
    }
    if (found >= 0) {
        if ((type == SystemFile && i == 0)
                || (type == SoundFile /*right now only allow shared ones?!*/) /* Rootdir[i]==homedir*/ ) {
            shared = true;
        }
        f.SetDirectory(Directories[curtype]);
        f.SetSubDirectory(subdir);
        f.SetRoot(curpath);
        f.SetVolume(isin_bigvolumes);
        //If we found a file in a volume we store its index in the concerned archive
        if (UseVolumes[curtype] && isin_bigvolumes > VSFSNone) {
            f.SetIndex(found);
        }
        isin_bigvolumes = VSFSNone;
        if (shared) {
            return Shared;
        } else {
            return Ok;
        }
    }
    return FileNotFound;
}

const boost::filesystem::path GetSavePath() {
    const boost::filesystem::path home_path{homedir};
    const boost::filesystem::path save_path{"save"};
    const boost::filesystem::path ret_val{boost::filesystem::absolute(save_path, home_path)};
    return ret_val;
}

/*
 ***********************************************************************************************
 **** VSFileSystem::VSFile class member functions                                            ***
 ***********************************************************************************************
 */

//IMPORTANT NOTE : IN MOST FILE OPERATION FUNCTIONS WE USE THE "alt_type" MEMBER BECAUSE A FILE WHOSE NATIVE TYPE
//SHOULD BE HANDLED IN VOLUMES MIGHT BE FOUND IN THE CURRENT DIRECTORY OF A TYPE THAT IS NOT HANDLED IN
//VOLUMES -> SO WE HAVE TO USE THE ALT_TYPE IN MOST OF THE TEST TO USE THE CORRECT FILE OPERATIONS

void VSFile::private_init() {
    fp = nullptr;
    size = 0;
    pk3_file = nullptr;
    pk3_extracted_file = nullptr;
    offset = 0;
    valid = false;
    file_type = alt_type = UnknownFile;
    file_index = -1;
    volume_type = VSFSNone;
}

VSFile::VSFile() {
    private_init();
}

VSFile::VSFile(const char *buffer, long bufsize, VSFileType type, VSFileMode mode) {
    private_init();
    this->size = bufsize;
    this->pk3_extracted_file = new char[bufsize + 1];
    memcpy(this->pk3_extracted_file, buffer, bufsize);
    pk3_extracted_file[bufsize] = 0;
    this->file_type = this->alt_type = ZoneBuffer;
    this->file_mode = mode;
    //To say we want to read in volume even if it is not the case then it will read in pk3_extracted_file
    this->volume_type = VSFSBig;
}

VSFile::VSFile(const char *filename, VSFileType type, VSFileMode mode) {
    private_init();
    if (mode == ReadOnly) {
        this->OpenReadOnly(filename, type);
    } else if (mode == ReadWrite) {
        this->OpenReadWrite(filename, type);
    } else if (mode == CreateWrite) {
        this->OpenCreateWrite(filename, type);
    }
}

VSFile::~VSFile() {
    if (fp != nullptr) {
        fclose(fp);
        this->fp = nullptr;
    }
    if (pk3_extracted_file != nullptr) {
        delete[] pk3_extracted_file;
        pk3_extracted_file = nullptr;
    }
}

void VSFile::checkExtracted() {
    if (q_volume_format == vfmtPK3) {
        if (!pk3_extracted_file) {
            string full_vol_path;
            if (this->volume_type == VSFSBig) {
                full_vol_path = this->rootname + "/data." + volume_format;
            } else {
                full_vol_path = this->rootname + "/" + Directories[this->alt_type] + "." + volume_format;
            }
            vsUMap<string, CPK3 *>::iterator it;
            it = pk3_opened_files.find(full_vol_path);
            if (it == pk3_opened_files.end()) {
                //File is not opened so we open it and add it in the pk3 file map
                CPK3 *pk3newfile = new CPK3;
                if (!pk3newfile->Open(full_vol_path.c_str())) {
                    VS_LOG_AND_FLUSH(fatal, (boost::format("!!! ERROR : opening volume : %1%") % full_vol_path));
                    VSExit(1);
                }
                std::pair<std::string, CPK3 *> pk3_pair(full_vol_path, pk3newfile);
                pk3_opened_files.insert(pk3_pair);

                this->pk3_file = pk3newfile;
            } else {
                this->pk3_file = it->second;
            }
            int pk3size = 0;
            if (this->file_index != -1) {
                pk3_extracted_file = (char *) pk3_file->ExtractFile(this->file_index, &pk3size);
            } else {
                pk3_extracted_file = (char *) pk3_file->ExtractFile(
                        (this->subdirectoryname + "/" + this->filename).c_str(), &pk3size);
            }
            this->size = pk3size;
            VS_LOG(info,
                    (boost::format("EXTRACTING %1% WITH INDEX=%2% SIZE=%3%")
                            % (this->subdirectoryname + "/" + this->filename)
                            % this->file_index
                            % pk3size));
        }
    }
}

const string VSFile::GetSystemDirectoryPath(string &file) {
    this->file_type = VSFileType::SystemFile;
    this->file_mode = ReadOnly;
    this->filename = file;
    VSError err = VSFileSystem::LookForFile(*this, VSFileType::SystemFile, ReadOnly);
    if (err > Ok) {
        this->valid = false;
        return file;
    }
    return this->GetFullPath();
}

//Open a read only file
VSError VSFile::OpenReadOnly(const char *file, VSFileType type) {
    string filestr;
    int found = -1;
    this->file_type = this->alt_type = type;
    this->file_mode = ReadOnly;
    this->filename = string(file);
    failed = "";

    VSError err = Ok;
    if (VSFS_DEBUG()) {
        VS_LOG(debug, (boost::format("Loading a %1% : %2%") % type % file));
    }
    if (type < ZoneBuffer || type == UnknownFile) {
        //It is a "classic file"
        if (!UseVolumes[type]) {
            if (type == UnknownFile) {
                //We look in the current_path or for a full relative path to either homedir or datadir
                if (!current_path.back().empty()) {
                    string filestr1 = current_directory.back()
                            + "/" + current_subdirectory.back() + "/" + string(file);
                    filestr = current_path.back() + "/" + filestr1;
                    if ((found = FileExists(current_path.back(), filestr1)) < 0) {
                        failed += "\t" + filestr + " NOT FOUND !\n";
                    }
                }
                if (found < 0) {
                    for (unsigned int ij = 0; ij < Rootdir.size() && found < 0; ij++) {
                        filestr = Rootdir[ij] + "/" + file;
                        found = FileExists(Rootdir[ij], file);
                        if (found < 0) {
                            failed += "\tRootdir : " + filestr + " NOT FOUND !\n";
                        }
                    }
                    //Look for relative (to datadir) or absolute named file
                    if (found < 0) {
                        filestr = file;
                        if ((found = FileExists("", filestr)) < 0) {
                            failed += "\tAbs or rel : " + filestr + " NOT FOUND !\n";
                        }
                    }
                }
                if (found < 0) {
                    if (VSFS_DEBUG()) {
                        cerr << failed << endl;
                    }
                    this->valid = false;
                    err = FileNotFound;
                } else {
                    if ((this->fp = fopen(filestr.c_str(), "rb")) == nullptr) {
                        VS_LOG_AND_FLUSH(fatal,
                                (boost::format(
                                        "!!! SERIOUS ERROR : failed to open Unknown file %1% - this should not happen")
                                        % filestr));
                        VSExit(1);
                    }
                    this->valid = true;
                    if (VSFS_DEBUG() > 1) {
                        VS_LOG(debug, (boost::format("%1% SUCCESS !!!") % filestr));
                    }
                }
            } else {
                err = VSFileSystem::LookForFile(*this, type, file_mode);
                if (err > Ok) {
                    this->valid = false;
                    return FileNotFound;
                }
                filestr = this->GetFullPath();
                this->fp = fopen(filestr.c_str(), "rb");
                if (!this->fp) {
                    VS_LOG(error,
                            (boost::format("!!! SERIOUS ERROR : failed to open %1% - this should not happen")
                                    % filestr));
                    this->valid = false;
                    return FileNotFound;                     //fault!
                }
                this->valid = true;
            }
        } else {
            if (q_volume_format == vfmtVSR) {
            } else if (q_volume_format == vfmtPK3) {
                //Here we look for the file but we don't really open it, we just check if it exists
                err = VSFileSystem::LookForFile(*this, type, file_mode);
                if (err > Ok) {
                    this->valid = false;
                    return FileNotFound;
                }
                //Test if we have found a file in another FileType's dir and if it doesn't use volumes
                //If so we open the file as a normal one
                if (this->volume_type == VSFSNone
                        || (this->alt_type != this->file_type && !UseVolumes[this->alt_type])) {
                    filestr = this->GetFullPath();
                    this->fp = fopen(filestr.c_str(), "rb");
                    if (!this->fp) {
                        VS_LOG(error,
                                (boost::format("!!! SERIOUS ERROR : failed to open %1% - this should not happen")
                                        % filestr));
                        this->valid = false;
                        return FileNotFound;                         //fault
                    }
                }
                this->valid = true;
            }
        }
        if (err <= Ok) {
            //We save the current path only when loading a unit, an animation, a sprite or a cockpit
            if ((type == UnitFile || type == AnimFile || type == VSSpriteFile || type == CockpitFile)) {
                current_path.push_back(this->rootname);
                current_subdirectory.push_back(this->subdirectoryname);
                current_type.push_back(this->alt_type);
                // stephengtuggy 2020-07-24 Leaving this boost logging conversion for later
                if (VSFS_DEBUG() > 1) {
                    cerr << endl << "BEGINNING OF ";
                    DisplayType(type);
                    cerr << endl;
                }
            }
        }
    } else {
        //This is a "buffer file"
        if (!this->pk3_extracted_file) {
            err = FileNotFound;
        } else {
            err = Ok;
        }
    }
    return err;
}

//We will always write in homedir+Directories[FileType][0]
//Open a standard file read/write
VSError VSFile::OpenReadWrite(const char *filename, VSFileType type) {
    if (type >= ZoneBuffer && type != UnknownFile) {
        return FileNotFound;
    }
    this->file_type = this->alt_type = type;
    this->file_mode = ReadWrite;

    return Ok;
}

//We will always write in homedir+Directories[FileType][0]
//Open (truncate) or create a standard file read/write
VSError VSFile::OpenCreateWrite(const char *filenam, VSFileType type) {
    if (type >= ZoneBuffer && type != UnknownFile) {
        return FileNotFound;
    }
    this->file_type = this->alt_type = type;
    this->filename = string(filenam);
    this->file_mode = CreateWrite;
    if (type == SystemFile) {
        string dirpath(sharedsectors + "/" + universe_name);
        CreateDirectoryHome(dirpath);
        CreateDirectoryHome(dirpath + "/" + getStarSystemSector(this->filename));
        string fpath(homedir + "/" + dirpath + "/" + this->filename);
        this->fp = fopen(fpath.c_str(), "wb");
        if (!fp) {
            return LocalPermissionDenied;
        }
    } else if (type == TextureFile) {
        string fpath(homedir + "/" + sharedtextures + "/" + this->filename);
        this->fp = fopen(fpath.c_str(), "wb");
        if (!fp) {
            return LocalPermissionDenied;
        }
    } else if (type == UnitFile) {
        string fpath(homedir + "/" + savedunitpath + "/" + this->filename);
        this->rootname = homedir;
        this->directoryname = savedunitpath;
        this->fp = fopen(fpath.c_str(), "wb");
        if (!fp) {
            return LocalPermissionDenied;
        }
    } else if (type == SaveFile) {
        string fpath(homedir + "/save/" + this->filename);
        this->fp = fopen(fpath.c_str(), "wb");
        if (!fp) {
            return LocalPermissionDenied;
        }
    } else if (type == AccountFile) {
        string fpath(datadir + "/accounts/" + this->filename);
        this->fp = fopen(fpath.c_str(), "wb");
        if (!fp) {
            return LocalPermissionDenied;
        }
    } else if (type == UnknownFile) {
        string fpath(homedir + "/" + this->filename);
        this->rootname = homedir;
        this->directoryname = "";
        this->fp = fopen(fpath.c_str(), "wb");
        if (!fp) {
            return LocalPermissionDenied;
        }
    }
    return Ok;
}

size_t VSFile::Read(void *ptr, size_t length) {
    size_t nbread = 0;
    if (!UseVolumes[this->alt_type] || this->volume_type == VSFSNone) {
        assert(fp != NULL);
        nbread = fread(ptr, 1, length, this->fp);
    } else {
        if (q_volume_format == vfmtVSR) {
        } else if (q_volume_format == vfmtPK3) {
            checkExtracted();
            if (length > this->size - this->offset) {
                length = this->size - this->offset;
            }
            memcpy(ptr, (pk3_extracted_file + offset), length);
            offset += length;
            nbread = length;
        }
    }
    return nbread;
}

VSError VSFile::ReadLine(void *ptr, size_t length) {
    char *ret;
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone) {
        ret = fgets((char *) ptr, length, this->fp);
        if (!ret) {
            return Unspecified;
        }
    } else {
        if (q_volume_format == vfmtVSR) {
        } else if (q_volume_format == vfmtPK3) {
            checkExtracted();
            ret = (char *) ptr;

            bool nl_found = false;
            unsigned int i = 0;
            if (VSFS_DEBUG() > 1) {
                cerr << "READLINE STARTING OFFSET=" << offset;
            }
            for (i = 0; !nl_found && i < length && offset < size; offset++, i++) {
                if (pk3_extracted_file[offset] == '\n' || pk3_extracted_file[offset] == '\r') {
                    nl_found = true;
                    if (VSFS_DEBUG() > 1) {
                        if (pk3_extracted_file[offset] == '\n') {
                            cerr << "\\n ";
                        }
                        if (pk3_extracted_file[offset] == '\r') {
                            cerr << "\\r ";
                        }
                    }
                } else {
                    ret[i] = pk3_extracted_file[offset];
                    if (VSFS_DEBUG() > 1) {
                        cerr << std::hex << ret[i] << " ";
                    }
                }
            }
            this->GoAfterEOL(length);
            ret[i] = 0;
            if (VSFS_DEBUG() > 1) {
                cerr << std::dec << " - read " << i << " char - " << ret << endl;
            }
            if (!nl_found) {
                return Unspecified;
            }
        }
    }
    return Ok;
}

std::string VSFile::ReadFull() {
    if (this->Size() < 0) {
        VS_LOG(error,
                (boost::format("Attempt to call ReadFull on a bad file %1% %2% %3%") % this->filename % this->Size()
                        % this->GetFullPath().c_str()));
        return string();
    }
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone) {
        long sz = this->Size();
        if (sz <= 0) {
            return string();
        }

        char *content = new char[this->Size() + 1];
        content[this->Size()] = 0;
        size_t readsize = fread(content, 1, this->Size(), this->fp);
        if (this->Size() != readsize) {
            VS_LOG(error,
                    (boost::format("Only read %1% out of %2% bytes of %3%") % readsize % this->Size()
                            % this->filename));
            GetError("ReadFull");
            if (readsize <= 0) {
                content[0] = '\0';
            } else {
                content[readsize] = '\0';
            }
        }
        string res(content);
        delete[] content;
        return res;
    } else {
        if (q_volume_format == vfmtVSR) {
        } else if (q_volume_format == vfmtPK3) {
            checkExtracted();
            offset = this->Size();
            return string(pk3_extracted_file);
        }
    }
    return string("");
}

size_t VSFile::Write(const void *ptr, size_t length) {
    if (!UseVolumes[this->alt_type] || this->volume_type == VSFSNone) {
        size_t nbwritten = fwrite(ptr, 1, length, this->fp);
        return nbwritten;
    } else {
        VS_LOG_AND_FLUSH(fatal, "!!! ERROR : Writing is not supported within resource/volume files");
        VSExit(1);
    }
    return Ok;
}

size_t VSFile::Write(const string &content) {
    std::string::size_type length = content.length();
    return this->Write(content.c_str(), length);
}

VSError VSFile::WriteLine(const void *ptr) {
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone) {
        fputs((const char *) ptr, this->fp);
    } else {
        VS_LOG_AND_FLUSH(fatal, "!!! ERROR : Writing is not supported within resource/volume files");
        VSExit(1);
    }
    return Ok;
}

void VSFile::WriteFull(void *ptr) {
}

int VSFile::Fprintf(const char *format, ...) {
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone) {
        va_list ap;
        va_start(ap, format);

        int retVal = vfprintf(this->fp, format, ap);
        va_end(ap);
        return retVal;
    } else {
        VS_LOG_AND_FLUSH(fatal, "!!! ERROR : Writing is not supported within resource/volume files");
        VSExit(1);
    }
    return 0;
}

void VSFile::Begin() {
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone || this->file_mode != ReadOnly) {
        fseek(this->fp, 0, SEEK_SET);
    } else {
        if (q_volume_format == vfmtVSR) {
            offset = 0;
        } else if (q_volume_format == vfmtPK3) {
            offset = 0;
        }
    }
}

void VSFile::End() {
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone || this->file_mode != ReadOnly) {
        fseek(this->fp, 0, SEEK_END);
    } else {
        if (q_volume_format == vfmtVSR) {
            offset = size;
        } else if (q_volume_format == vfmtPK3) {
            offset = size;
        }
    }
}

void VSFile::GoTo(long foffset)                //Does a VSFileSystem::Fseek( fp, offset, SEEK_SET);
{
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone || this->file_mode != ReadOnly) {
        fseek(this->fp, foffset, SEEK_SET);
    } else {
        if (q_volume_format == vfmtVSR) {
            offset = foffset;
        } else if (q_volume_format == vfmtPK3) {
            offset = foffset;
        }
    }
}

// FIXME: Return type should be either unsigned long or size_t
long VSFile::Size() {
    if (size == 0) {
        if (!UseVolumes[alt_type] || this->volume_type == VSFSNone || file_mode != ReadOnly) {
            struct stat st{};
            if ((fp != nullptr) && fstat(fileno(fp), &st) == 0) {
                return this->size = st.st_size;
            }
            return -1;
        } else {
            if (q_volume_format == vfmtVSR) {
            } else if (q_volume_format == vfmtPK3) {
                checkExtracted();
                return this->size;
            }
        }
        return -1;
    }
    return this->size;
}

void VSFile::Clear() {
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone || file_mode != ReadOnly) {
        fclose(fp);
        this->fp = fopen(this->GetFullPath().c_str(), "w+b");
        //This should not happen
        if (!fp) {
            GetError("Clear");
            VSExit(1);
        }
    } else {
        VS_LOG_AND_FLUSH(fatal, "!!! ERROR : Writing is not supported within resource/volume files");
        VSExit(1);
    }
}

long VSFile::GetPosition() {
    long ret = 0;
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone || file_mode != ReadOnly) {
        ret = ftell(this->fp);
    } else {
        if (q_volume_format == vfmtVSR) {
        } else if (q_volume_format == vfmtPK3) {
            ret = offset;
        }
    }
    return ret;
}

bool VSFile::Eof() {
    bool eof = false;
    if (!UseVolumes[alt_type] || this->volume_type == VSFSNone || file_mode != ReadOnly) {
        eof = vs_feof(this->fp);
    } else {
        if (q_volume_format == vfmtVSR) {
        } else if (q_volume_format == vfmtPK3) {
            eof = (this->Size() < 0) || (offset == (unsigned long) this->Size());
        }
    }
    return eof;
}

bool VSFile::Valid() {
    return valid;
}

void VSFile::Close() {
    if (this->file_type >= ZoneBuffer && this->file_type != UnknownFile && this->pk3_extracted_file) {
        delete this->pk3_extracted_file;
        this->pk3_extracted_file = nullptr;
        return;
    }
    if (this->valid && this->file_mode == ReadOnly
            && (file_type == UnitFile || file_type == AnimFile || file_type == VSSpriteFile
                    || file_type == CockpitFile)) {
        assert(current_path.size() > 1);
        current_path.pop_back();
        current_subdirectory.pop_back();
        current_type.pop_back();
        if (VSFS_DEBUG() > 2) {
            cerr << "END OF ";
            DisplayType(this->file_type);
            cerr << endl << endl;
        }
    }
    if (!UseVolumes[file_type] || this->volume_type == VSFSNone || file_mode != ReadOnly) {
        fclose(this->fp);
        this->fp = nullptr;
    } else {
        if (q_volume_format == vfmtVSR) {
        } else if (q_volume_format == vfmtPK3) {
            if (pk3_extracted_file != nullptr) {
                delete[] pk3_extracted_file;
                pk3_extracted_file = nullptr;
            }
        }
    }
    this->size = -1;
    this->valid = false;
    this->filename = "";
    this->directoryname = "";
    this->subdirectoryname = "";
    this->rootname = "";
    this->offset = 0;
    this->file_index = -1;
}

static void pathAppend(string &dest, string &suffix) {
    if (suffix.empty()) {
        return;
    }
    if (suffix[0] != '/' && !dest.empty() && dest[dest.length() - 1] != '/') {
        dest += "/";
    }
    dest += suffix;
}

std::string VSFile::GetFullPath() {
    string tmp = this->rootname;
    pathAppend(tmp, this->directoryname);
    pathAppend(tmp, this->subdirectoryname);
    pathAppend(tmp, this->filename);
    return tmp;
}

std::string VSFile::GetAbsPath() {
    string tmp = this->directoryname;
    pathAppend(tmp, this->subdirectoryname);
    pathAppend(tmp, this->filename);
    return tmp;
}

void VSFile::SetType(VSFileType type) {
    this->file_type = type;
}

void VSFile::SetAltType(VSFileType type) {
    this->alt_type = type;
}

void VSFile::SetIndex(int index) {
    this->file_index = index;
}

void VSFile::SetVolume(VSVolumeType big) {
    this->volume_type = big;
}

bool VSFile::UseVolume() {
    return UseVolumes[alt_type] && volume_type != VSFSNone;
}

void VSFile::GoAfterEOL(unsigned int length) {
    while (this->offset < length && this->offset < this->size
            && (this->pk3_extracted_file[offset] == '\r' || this->pk3_extracted_file[offset] == '\n')) {
        this->offset++;
    }
}

void VSFile::GoAfterEOL() {
    while (this->offset < this->size
            && (this->pk3_extracted_file[offset] == '\r' || this->pk3_extracted_file[offset] == '\n')) {
        this->offset++;
    }
}
}

#define CASE(a) \
case a:           \
    ostr<<#a; break;

std::ostream &operator<<(std::ostream &ostr, VSFileSystem::VSError err) {
    switch (err) {
        CASE(VSFileSystem::Shared)
        CASE(VSFileSystem::Ok)
        CASE(VSFileSystem::SocketError)
        CASE(VSFileSystem::FileNotFound)
        CASE(VSFileSystem::LocalPermissionDenied)
        CASE(VSFileSystem::RemotePermissionDenied)
        CASE(VSFileSystem::DownloadInterrupted)
        CASE(VSFileSystem::IncompleteWrite)
        CASE(VSFileSystem::IncompleteRead)
        CASE(VSFileSystem::EndOfFile)
        CASE(VSFileSystem::IsDirectory)
        CASE(VSFileSystem::BadFormat)
        CASE(VSFileSystem::Unspecified)
        default:
            ostr << "VSFileSystem::<undefined VSError>";
            break;
    }
    return ostr;
}

#undef CASE

std::ostream &operator<<(std::ostream &ostr, VSFileSystem::VSFileType type) {
    VSFileSystem::DisplayType(type, ostr);
    return ostr;
}

std::string nameof(VSFileSystem::VSFileType type) {
#define CASE(a) \
case a:           \
    return #a; break;
    switch (type) {
        CASE(VSFileSystem::UniverseFile)
        CASE(VSFileSystem::SystemFile)
        CASE(VSFileSystem::CockpitFile)
        CASE(VSFileSystem::UnitFile)
        CASE(VSFileSystem::PythonFile)
        CASE(VSFileSystem::TextureFile)
        CASE(VSFileSystem::SoundFile)
        CASE(VSFileSystem::MeshFile)
        CASE(VSFileSystem::CommFile)
        CASE(VSFileSystem::AiFile)
        CASE(VSFileSystem::SaveFile)
        CASE(VSFileSystem::AnimFile)
        CASE(VSFileSystem::VSSpriteFile)
        CASE(VSFileSystem::MissionFile)
        CASE(VSFileSystem::MusicFile)
        CASE(VSFileSystem::AccountFile)
        CASE(VSFileSystem::ZoneBuffer)
        CASE(VSFileSystem::UnknownFile)
        default:
            return "VSFileSystem::<undefined VSFileType>";

            break;
    }
#undef CASE
}

//#if defined (_WIN32) && !defined (__CYGWIN__)
//
//int scandir( const char *dirname, struct dirent ***namelist, int (*select)( const struct dirent* ), int (*compar)(
//                const struct dirent**,
//                const struct dirent** ) )
//{
//    size_t len;
//    char  *findIn;
//    char  *d;
//    WIN32_FIND_DATA find;
//    HANDLE h;
//    int    nDir = 0;
//    int    NDir = 0;
//    struct dirent **dir = nullptr;
//    struct dirent *selectDir;
//    unsigned long   ret;
//
//    len    = strlen( dirname );
//    findIn = (char*) malloc( len+5 );
//    strcpy( findIn, dirname );
//    for (d = findIn; *d; d++)
//        if (*d == '/') *d = '\\';
//    if ( (len == 0) ) strcpy( findIn, ".\\*" );
//    if ( (len == 1) && (d[-1] == '.') ) strcpy( findIn, ".\\*" );
//    if ( (len > 0) && (d[-1] == '\\') ) {
//        *d++ = '*';
//        *d   = 0;
//    }
//    if ( (len > 1) && (d[-1] == '.') && (d[-2] == '\\') ) d[-1] = '*';
//    if ( ( h = FindFirstFile( findIn, &find ) ) == INVALID_HANDLE_VALUE ) {
//        ret = GetLastError();
//        if (ret != ERROR_NO_MORE_FILES) {
//            //TODO: return some error code
//        }
//        free( findIn );
//        *namelist = dir;
//        return nDir;
//    }
//    do {
//        selectDir = (struct dirent*) malloc( sizeof (struct dirent)+strlen( find.cFileName ) );
//        strcpy( selectDir->d_name, find.cFileName );
//        if ( !select || (*select)(selectDir) ) {
//            if (nDir == NDir) {
//                struct dirent **tempDir = (dirent**) calloc( sizeof (struct dirent*), NDir+33 );
//                if (NDir) {
//                    memcpy( tempDir, dir, sizeof (struct dirent*) *NDir );
//                }
//                if (dir != nullptr) {
//                    free( dir );
//                    dir = nullptr;
//                }
//                dir   = tempDir;
//                NDir += 32;
//            }
//            dir[nDir] = selectDir;
//            nDir++;
//            dir[nDir] = nullptr;
//        } else {
//            free( selectDir );
//        }
//    } while ( FindNextFile( h, &find ) );
//    ret = GetLastError();
//    if (ret != ERROR_NO_MORE_FILES) {
//        //TODO: return some error code
//    }
//    FindClose( h );
//
//    free( findIn );
//    if (compar) {
//        //sortTheDirectoryEntries(dir, nDir, compar);
//        qsort(dir, nDir, sizeof(*dir),
//                (int (*)(const void *, const void *)) compar);
//    }
//    *namelist = dir;
//    return nDir;
//}

//std::vector<dirent *> sortTheDirectoryEntries(dirent **p_dirent, const size_t num_entries, int (*compar)(const dirent **, const dirent **)) {
//    std::vector<dirent *> ret_val{};
//    for (size_t i = 0; i < num_entries; ++i) {
//        ret_val.emplace_back(p_dirent[i]);
//    }
//
////    std::sort(ret_val.begin(), ret_val.end(), compar);
//
//    return ret_val;
//}

//int alphasort( struct dirent **a, struct dirent **b )
//{
//    return strcmp( (*a)->d_name, (*b)->d_name );
//}

//#endif
