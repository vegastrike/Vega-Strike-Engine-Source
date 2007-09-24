#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#include <config.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
struct dirent { char d_name[1]; };
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>
#endif
#include <sys/stat.h>
#include "config.h"
#include "configxml.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "common/common.h"
#include "galaxy_gen.h"
#include "pk3.h"

#include <gnuhash.h>


using stdext::hash_map;

using VSFileSystem::VSVolumeType;
using VSFileSystem::VSFSNone;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
int VSFS_DEBUG() {
  if (vs_config) {
    static int vs_debug = XMLSupport::parse_int(vs_config->getVariable("general","debug_fs","0"));
    return vs_debug;
  }
  return 0;
}
char *CONFIGFILE;
char pwd[65536];
//extern int vfscanf( FILE * fp, const char * format, va_list arglist) ;
//int gcc295vfscanf( FILE * fp, const char * format, va_list arglist) {
//  return vfscanf(fp,format,arglist);
//}
VSVolumeType isin_bigvolumes = VSFSNone;
string curmodpath = "";

ObjSerial	serial_seed = 0; // Server/client serials won't intersect.
std::map<ObjSerial,bool> usedSerials;

void usedSerial(ObjSerial ser, bool used) {
	usedSerials[ser]=used;
}
ObjSerial	getUniqueSerial()
{
	int offset = (SERVER ? 2 : 1);
	
	std::map<ObjSerial,bool>::const_iterator iter;
	ObjSerial ret;
	do {
		serial_seed = (serial_seed+3)%MAXSERIAL;
		ret = serial_seed+offset;
	} while ((iter=usedSerials.find(ret))!=usedSerials.end()
			 && (*iter).second);
	
	usedSerial(ret, true);
	return ret;
}

extern string GetUnitDir( string filename);

#if defined (__FreeBSD__) || defined(__APPLE__)
int	selectdirs( struct dirent * entry)
#else
int	selectdirs( const struct dirent * entry)
#endif
{
#if defined(_WIN32)
	// Have to check if we have the full path or just relative (which would be a problem)
	cerr<<"Read directory entry : "<<(curmodpath+entry->d_name)<<endl;
	struct stat s;
	std::string tmp=curmodpath+entry->d_name;
	if( stat( tmp.c_str(), &s)<0)
		return false;
	if( (s.st_mode & S_IFDIR) && string( entry->d_name)!="." && string( entry->d_name)!="..")
	{
		return 1;
	}
#else
	if( entry->d_type==DT_DIR && string( entry->d_name)!="." && string( entry->d_name)!="..")
		return 1;
#endif
	return 0;
}

#if defined (__FreeBSD__) || defined(__APPLE__)
int	selectpk3s( struct dirent * entry)
#else
int	selectpk3s( const struct dirent * entry)
#endif
{
	// If this is a regular file and we have ".pk3" in it
	if( (string( entry->d_name).find( ".pk3"))!=std::string::npos && (string( entry->d_name).find( "data"))==std::string::npos)
		return 1;
	return 0;
}

#if defined (__FreeBSD__) || defined(__APPLE__)
int	selectbigpk3s( struct dirent * entry)
#else
int	selectbigpk3s( const struct dirent * entry)
#endif
{
	// If this is a regular file and we have ".pk3" in it
	if( (string( entry->d_name).find( "data.pk3"))!=std::string::npos)
		return 1;
	return 0;
}

namespace VSFileSystem
{
	VSError CachedFileLookup(FileLookupCache &cache, const string& file, VSFileType type)
	{
		string hashName = GetHashName(file);
		FileLookupCache::iterator it = cache.find(hashName);
		if (it != cache.end())
			return it->second; else
			return cache[hashName]=LookForFile(file,type);
	}

	void	DisplayType( VSFileSystem::VSFileType type)
	{
	    DisplayType( type, std::cerr );
	}

	#define CASE(a) case a: ostr<<#a; break;
	void	DisplayType( VSFileSystem::VSFileType type, std::ostream& ostr )
	{
		switch( type)
		{
			CASE( VSFileSystem::UniverseFile)
			CASE( VSFileSystem::SystemFile)
			CASE( VSFileSystem::CockpitFile)
			CASE( VSFileSystem::UnitFile)
			CASE( VSFileSystem::PythonFile)
			CASE( VSFileSystem::TextureFile)
			CASE( VSFileSystem::SoundFile)
			CASE( VSFileSystem::MeshFile)
			CASE( VSFileSystem::CommFile)
			CASE( VSFileSystem::AiFile)
			CASE( VSFileSystem::SaveFile)
			CASE( VSFileSystem::AnimFile)
			CASE( VSFileSystem::VSSpriteFile)
			CASE( VSFileSystem::MissionFile)
			CASE( VSFileSystem::BSPFile)
			CASE( VSFileSystem::MusicFile)
			CASE( VSFileSystem::AccountFile)
			CASE( VSFileSystem::ZoneBuffer)
			CASE( VSFileSystem::UnknownFile)
			default: ostr << "VSFileSystem::<undefined VSFileType>"; break;
		}
	}
	#undef CASE

	int		GetReadBytes( char * fmt, va_list ap)
	{
		int ret = 0;

		cerr<<"STARTING ARGLIST"<<endl;
		while( *fmt)
		{
			switch( *fmt++)
			{
				case 'd' :
				break;
				case 'f' :
				break;
				case 's' :
				break;
				case 'g' :
				break;
				case 'n' :	// Number of bytes
					ret = va_arg( ap, int);
					cerr<<"\tFound 'n' : "<<ret<<endl;
				break;
				default :
					cerr<<"\tOther arg"<<endl;
			}
		}
		cerr<<"ENDING ARGLIST"<<endl;
		return ret;
	}

	/************************************************************************************************/
	/**** VSFileSystem global variables                                                          ****/
	/************************************************************************************************/

	bool	use_volumes;
	string	volume_format;
    enum VSVolumeFormat q_volume_format;

	vector< vector <string> >	SubDirectories;		// Subdirectories where we should look for VSFileTypes files
	vector<string>				Directories;		// Directories where we should look for VSFileTypes files
	vector<string>				Rootdir;			// Root directories where we should look for VSFileTypes files
	string sharedtextures;
	string sharedunits;
	string sharedsounds;
	string sharedmeshes;
	string sharedsectors;
	string sharedcockpits;
	string shareduniverse;
        string aidir;
	string sharedanims;
	string sharedsprites;
	string savedunitpath;
	string moddir;
	string datadir;
	string homedir;

	string config_file;
	string weapon_list;
	string universe_name;
	string HOMESUBDIR(".vegastrike");
	vector<string>		current_path;
	vector<string>		current_directory;
	vector<string>		current_subdirectory;
	vector<VSFileType>	current_type;

	// vs_path only stuff
	vector <std::string> savedpwd;
	vector <std::string> curdir;//current dir starting from datadir
	vector <std::vector <std::string> > savedcurdir;//current dir starting from datadir

	vector<int>			UseVolumes;

	string failed;

	// Map of the currently opened PK3 volume/resource files
	stdext::hash_map<string, CPK3 *>	pk3_opened_files;

	/************************************************************************************************/
	/**** vs_path functions                                                                      ****/
	/************************************************************************************************/

		std::string GetHashName (const std::string &name) {
		  std::string result("");
		  result = current_path.back()+current_directory.back()+current_subdirectory.back()+name;
		  return result;
		}
		std::string GetHashName (const std::string &name, const Vector &scale, int faction) {
		  std::string result("");
		  result = current_path.back()+current_directory.back()+current_subdirectory.back()+name;
		  
		  result+=XMLSupport::VectorToString(scale)+"|"+XMLSupport::tostring(faction);
		  return result;
		}

		std::string GetSharedMeshHashName (const std::string &name,const Vector & scale, int faction) {
			return (string ("#")+XMLSupport::VectorToString(scale)+ string("#")+name+string("#")+XMLSupport::tostring(faction));
		}
		std::string GetSharedTextureHashName (const std::string &name) {
		  return (string ("#stex#")+name);
		}
		std::string GetSharedSoundHashName (const std::string &name) {
		  return (string ("#ssnd#")+name);
		}

		std::string MakeSharedPathReturnHome (const std::string &newpath) {
		  CreateDirectoryHome( newpath);
		  return newpath+string("/");	  
		}
		std::string MakeSharedPath (const std::string &s) {
		  VSFileSystem::vs_fprintf (stderr,"MakingSharedPath %s",s.c_str());
		  return MakeSharedPathReturnHome (s)+s;
		}
		std::string MakeSharedStarSysPath (const std::string &s){
		  string syspath = sharedsectors+"/"+getStarSystemSector(s);
		  return MakeSharedPathReturnHome ( syspath)+s;
		}
		std::string GetCorrectStarSysPath (const std::string &name, bool &autogenerated) {
		  autogenerated=false;
		  if (name.length()==0) {
			return string ("");
		  }

			VSFile f;
			VSError err = f.OpenReadOnly( name, SystemFile);
			if( err<=Ok)
			{
				autogenerated = (err==Shared);
				return name;
			}
			return string( "");
		}


	/************************************************************************************************/
	/**** VSFileSystem wrappers to stdio function calls                                          ****/
	/************************************************************************************************/

	FILE *	vs_open( const char * filename, const char * mode)
	{
          if (VSFS_DEBUG()>1) {
		cerr<<"-= VS_OPEN in mode "<<mode<<" =- ";
          }

			FILE * fp;
			string fullpath = homedir+"/"+string( filename);
			if( !use_volumes && (string( mode) == "rb" || string( mode) == "r"))
			{
				string output("");
				fp = fopen( fullpath.c_str(), mode);
				if( !fp)
				{
					fullpath = string(filename);
					output += fullpath+"... NOT FOUND\n\tTry loading : "+fullpath;
					fp = fopen( fullpath.c_str(), mode);
				}
				if( !fp)
				{
					fullpath = datadir+"/"+string(filename);
					output += "... NOT FOUND\n\tTry loading : "+fullpath;
					fp = fopen( fullpath.c_str(), mode);
				}
                                if (VSFS_DEBUG()) {
                                  if( fp){
                                    if (VSFS_DEBUG()>2)
                                      cerr<<fullpath<<" SUCCESS !!!"<<endl;
                                  }else {
                                    cerr<<output<<" NOT FOUND !!!"<<endl;
                                  }
                                }
			}
			else
			{
				fp = fopen( fullpath.c_str(), mode);
				if( fp)
				{
                                  if (VSFS_DEBUG()>2) 
					cerr<<fullpath<<" opened for writing SUCCESS !!!"<<endl;
				}
				else
				{
                                  if (VSFS_DEBUG())
                                    cerr<<fullpath<<" FAILED !!!"<<endl;
				}
			}
		
			return fp;

		return NULL;
	}

	size_t	vs_read( void *ptr, size_t size, size_t nmemb, FILE * fp)
	{
		if( !use_volumes)
		{
			return fread( ptr, size, nmemb, fp);
		}
		else
		{
		}
		return 0;
	}

	size_t	vs_write( const void *ptr, size_t size, size_t nmemb, FILE * fp)
	{
		if( !use_volumes)
		{
			return fwrite( ptr, size, nmemb, fp);
		}
		else
		{
		}
		return 0;
	}

	void	vs_close( FILE * fp)
	{
		if( !use_volumes)
		{
			fclose( fp);
		}
		else
		{
		}
	}

	int		vs_fprintf( FILE * fp, const char * format, ...)
	{
		if( !use_volumes)
		{
			va_list ap;
			va_start( ap, format);

			return vfprintf( fp, format, ap);
		}
		else
		{
		}
		return 0;
	}

#if 0
	int		vs_fscanf( FILE * fp, const char * format, ...)
	{
		if( !use_volumes)
		{
			va_list   arglist;
			va_start( arglist, format);
			//return _input(fp,(unsigned char*)format,arglist);
			return vfscanf( fp, format, arglist);
		}
		else
		{
		}
		return 0;
	}
#endif

	int		vs_fseek( FILE * fp, long offset, int whence)
	{	return fseek( fp, offset, whence); }

	long	vs_ftell( FILE * fp)
	{	return ftell( fp); }

	void	vs_rewind( FILE * fp)
	{	rewind( fp); }

	bool	vs_feof( FILE * fp)
	{	return feof( fp); }

	long	vs_getsize( FILE * fp)
	{
		if( !use_volumes)
		{
			struct stat st;
			if( fstat( fileno(fp), &st)==0)
				return st.st_size;
			return -1;
		}
		return -1;
	}

	/************************************************************************************************/
	/**** VSFileSystem functions                                                                 ****/
	/************************************************************************************************/

	void	InitHomeDirectory()
	{
		// Setup home directory
		char * chome_path = NULL;
#ifndef _WIN32
		struct passwd *pwent;
		pwent = getpwuid (getuid());
		chome_path = pwent->pw_dir;
		if( !DirectoryExists( chome_path))
		{
			cerr<<"!!! ERROR : home directory not found"<<endl;
			VSExit(1);
		}
		string user_home_path( chome_path);
		homedir = user_home_path+"/"+HOMESUBDIR;
#else
		homedir = datadir+"/"+HOMESUBDIR;
#endif
		cerr<<"USING HOMEDIR : "<<homedir<< " As the home directory "<<endl;
		CreateDirectoryAbs( homedir);
	}

	void	InitDataDirectory()
	{
		vector<string>	data_paths;
#ifdef DATA_DIR
		data_paths.push_back( DATA_DIR);
#endif
		data_paths.push_back( ".");
		data_paths.push_back( "..");
		data_paths.push_back( "../data4.x");
		data_paths.push_back( "../../data4.x");
		data_paths.push_back( "../data");
		data_paths.push_back( "../../data");
		data_paths.push_back( "../Resources");
		data_paths.push_back( "../Resources/data");
		data_paths.push_back( "/usr/share/local/vegastrike/data");
		data_paths.push_back( "/usr/local/share/vegastrike/data");
		data_paths.push_back( "/usr/local/vegastrike/data");
		data_paths.push_back( "/usr/share/vegastrike/data");
		data_paths.push_back( "/usr/local/games/vegastrike/data");
		data_paths.push_back( "/usr/games/vegastrike/data");
		data_paths.push_back( "/opt/share/vegastrike/data");
		data_paths.push_back( "../Resources/data4.x");
		data_paths.push_back( "/usr/share/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/share/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/share/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/games/vegastrike/data4.x");
		data_paths.push_back( "/usr/games/vegastrike/data4.x");
		data_paths.push_back( "/opt/share/vegastrike/data4.x");
		
		// Win32 data should be "."
		char tmppath[16384];
		for( vector<string>::iterator vsit=data_paths.begin(); vsit!=data_paths.end(); vsit++)
		{
			// Test if the dir exist and contains config_file
			if( FileExists( (*vsit), config_file)>=0)
			{
				cerr<<"Found data in "<<(*vsit)<<endl;
				getcwd( tmppath, 16384);
				if( (*vsit).substr( 0, 1) == ".")
					datadir = string( tmppath)+"/"+(*vsit);
				else
					datadir = (*vsit);

				if( chdir( datadir.c_str())<0)
				{
					cerr<<"Error changing to datadir"<<endl;
					exit(1);
				}
				getcwd( tmppath, 16384);
				datadir = string( tmppath);

				cerr<<"Using "<<datadir<<" as data directory"<<endl;
				break;
			}
		}
		data_paths.clear();
		string versionloc=datadir+"/Version.txt";
		FILE * version = fopen (versionloc.c_str(),"r");
		if (!version) {
			versionloc=datadir+"Version.txt";
			version=fopen(versionloc.c_str(),"r");
		}
		if (!version) {
			version=fopen("Version.txt","r");
		}
		if (version) {
			string hsd="";
			int c;
			while ((c=fgetc(version))!=EOF) {
				if (isspace(c))
					break;
				hsd+=(char)c;
			}
			fclose(version);
			if (hsd.length()) {
				HOMESUBDIR=hsd;
				printf ("Using %s as the home directory\n",hsd.c_str());
			}			
		}
		
		// Load default VS config file
		char * conffile = new char[config_file.length()+1];
		conffile[config_file.length()] = 0;
		memcpy( conffile, config_file.c_str(), config_file.length());
		// Get the mods path
		moddir = datadir+"/"+string( "mods");
		cout<<"Found MODDIR = "<<moddir<<endl;
	}

	// Config file has been loaded from data dir but now we look at the specified moddir in order
	// to see if we should use a mod config file
	void	LoadConfig( string subdir)
	{
		bool found = false;
                bool foundweapons = false;
		// First check if we have a config file in homedir+"/"+subdir or in datadir+"/"+subdir
		weapon_list = "weapon_list.xml";
		if( subdir!="")
		{
			
			if( DirectoryExists( homedir+"/mods/"+subdir))
			{
				if( FileExists( homedir+"/mods/"+subdir, config_file)>=0)
				{
					cout<<"CONFIGFILE - Found a config file in home mod directory, using : "<<(homedir+"/mods/"+subdir+"/"+config_file)<<endl;
					if( FileExists( homedir+"/mods/"+subdir, "weapon_list.xml")>=0) {
						weapon_list = homedir+"/mods/"+subdir+"/weapon_list.xml";
                                                foundweapons=true;
                                        }
					config_file = homedir+"/mods/"+subdir+"/"+config_file;
					found = true;
				}
			}
			
			
                          if (!found)
                            cout<<"WARNING : coudn't find a mod named '"<<subdir<<"' in homedir/mods"<<endl;
				if( DirectoryExists( moddir+"/"+subdir))
				{
					if( FileExists( moddir+"/"+subdir, config_file)>=0)
					{
						if (!found) cout<<"CONFIGFILE - Found a config file in mods directory, using : "<<(moddir+"/"+subdir+"/"+config_file)<<endl;
						if( (!foundweapons)&&FileExists( moddir+"/"+subdir, "weapon_list.xml")>=0) {
							weapon_list = moddir+"/"+subdir+"/weapon_list.xml";
                                                        foundweapons=true;
                                                }
						if (!found) config_file = moddir+"/"+subdir+"/"+config_file;
						found = true;
					}
				}
				else
				{
					cout<<"ERROR : coudn't find a mod named '"<<subdir<<"' in datadir/mods"<<endl;
					exit(1);
				}
			//}
		}
		

		if( !found)
		{
			// Next check if we have a config file in homedir if we haven't found one for mod
			if( FileExists( homedir, config_file)>=0)
			{
				cerr<<"CONFIGFILE - Found a config file in home directory, using : "<<(homedir+"/"+config_file)<<endl;
                                config_file=homedir+"/"+config_file;
				/*
				char * conffile = new char[homedir.length()+1+config_file.length()+1];
				conffile[homedir.length()+1+config_file.length()] = 0;
				memcpy( conffile, (homedir+"/"+config_file).c_str(), homedir.length()+1+config_file.length());
				vs_config = createVegaConfig( config_file.c_str());
				delete []conffile;
				*/
			}
			else
			{
				cerr<<"CONFIGFILE - No config found in home : "<<(homedir+"/"+config_file)<<endl;
				if( FileExists( datadir,  config_file)>=0)
				{
					cerr<<"CONFIGFILE - No home config file found, using datadir config file : "<<(datadir+"/"+config_file)<<endl;
					// We didn't find a config file in home_path so we load the data_path one
					/*
					char * conffile = new char[datadir.length()+1+config_file.length()+1];
					conffile[datadir.length()+1+config_file.length()] = 0;
					memcpy( conffile, (datadir+"/"+config_file).c_str(), datadir.length()+1+config_file.length());
					vs_config = createVegaConfig( conffile);
					delete []conffile;
					*/
				}
				else
				{
					cerr<<"CONFIGFILE - No config found in data dir : "<<(datadir+"/"+config_file)<<endl;
					cerr<<"CONFIG FILE NOT FOUND !!!"<<endl;
					VSExit(1);
				}
			}
		}else if (subdir!="") {
                  printf ("\nUsing Mod Directory %s",moddir.c_str());
                  CreateDirectoryHome( "mods");
                  CreateDirectoryHome("mods/"+subdir);
                  datadir = moddir+"/"+subdir;
                  homedir = homedir+"/mods/"+subdir;
                }

		// Delete the default config in order to reallocate it with the right one (if it is a mod)
		if (vs_config) {
			fprintf (stderr,"reallocating vs_config \n");
			delete vs_config;
		}
		vs_config=NULL;
		char * conffile = new char[config_file.length()+1];
		conffile[config_file.length()] = 0;
		memcpy( conffile, config_file.c_str(), config_file.length());
		vs_config = createVegaConfig( conffile);
		delete []conffile;

		// Now check if there is a data directory specified in it
		// NOTE : THIS IS NOT A GOOD IDEA TO HAVE A DATADIR SPECIFIED IN THE CONFIG FILE
		static string data_path( vs_config->getVariable( "data", "datadir", ""));
		if( data_path != "")
		{
			// We found a path to data in config file
			cout<<"DATADIR - Found a datadir in config, using : "<<data_path<<endl;
			datadir = data_path;
		}
		else
			cout<<"DATADIR - No datadir specified in config file, using ; "<<datadir<<endl;
	}

	void	InitMods()
	{
		string curpath;
		struct dirent ** dirlist;
		// Scan for mods in specified subdir
		int ret = scandir( moddir.c_str(), &dirlist, selectdirs, 0);
		if( ret <0)
			return;
		else
		{
			while( ret--)
			{
				curpath = moddir+"/"+dirlist[ret]->d_name;
				cout<<"Adding mod path : "<<curpath<<endl;
				Rootdir.push_back( curpath);
			}
		}
		free( dirlist);
		// Scan for mods with standard data subtree
		curmodpath = homedir+"/mods/";
		ret = scandir( curmodpath.c_str(), &dirlist, selectdirs, 0);
		if( ret <0)
			return;
		else
		{
			while( ret--)
			{
				curpath = curmodpath+dirlist[ret]->d_name;
				cout<<"Adding mod path : "<<curpath<<endl;
				Rootdir.push_back( curpath);
			}
		}
		free( dirlist);
	}


	void	InitPaths( string conf, string subdir)
	{
		config_file = conf;

		current_path.push_back( "");
		current_directory.push_back( "");
		current_subdirectory.push_back( "");
		current_type.push_back( UnknownFile);

		int i;
		for( i=0; i<=UnknownFile; i++)
			UseVolumes.push_back( 0);

		/************************** Data and home directory settings *************************/

		InitDataDirectory();	// Need to be first for win32
		InitHomeDirectory();
		LoadConfig( subdir);

		// Paths relative to datadir or homedir (both should have the same structure)
		// Units are in sharedunits/unitname/, sharedunits/subunits/unitname/ or sharedunits/weapons/unitname/ or in sharedunits/faction/unitname/
		// Meshes are in sharedmeshes/ or in current unit that is being loaded
		// Textures are in sharedtextures/ or in current unit dir that is being loaded or in the current animation dir that is being loaded or in the current sprite dir that is being loeded
		// Sounds are in sharedsounds/
		// Universes are in universe/
		// Systems are in "sectors"/ config variable
		// Cockpits are in cockpits/ (also a config var)
		// Animations are in animations/
		// VSSprite are in sprites/ or in ./ (when full subpath is provided) or in the current cockpit dir that is being loaded
		
		// First allocate an empty directory list for each file type
		for( i=0; i<UnknownFile; i++)
		{
			vector<string> vec;
			Directories.push_back( "");
			SubDirectories.push_back( vec);
		}

		sharedsectors = vs_config->getVariable( "data", "sectors", "sectors");
		sharedcockpits = vs_config->getVariable( "data", "cockpits", "cockpits");
		shareduniverse = vs_config->getVariable( "data", "universe_path", "universe");
		sharedanims = vs_config->getVariable( "data", "animations", "animations");
		sharedsprites = vs_config->getVariable( "data", "sprites", "sprites");
		savedunitpath = vs_config->getVariable ("data","serialized_xml","serialized_xml");
		sharedtextures = vs_config->getVariable ("data","sharedtextures","textures");
		sharedsounds = vs_config->getVariable ("data","sharedsounds","sounds");
		sharedmeshes = vs_config->getVariable ("data","sharedmeshes","meshes");
		sharedunits = vs_config->getVariable ("data","sharedunits","units");
		aidir = vs_config->getVariable ("data","ai_directory","ai");
		universe_name = vs_config->getVariable( "general", "galaxy", "milky_way.xml");

		// Setup the directory lists we know about - note these are relative paths to datadir or homedir
		// ----- THE Directories vector contains the resource/volume files name without extension or the main directory to files
		Directories[UnitFile] = sharedunits;
		//SubDirectories[UnitFile].push_back( "factions");
		// Have to put it in first place otherwise VS will find default unit file
		SubDirectories[UnitFile].push_back( "subunits");
		SubDirectories[UnitFile].push_back( "weapons");
		SubDirectories[UnitFile].push_back( "");
		SubDirectories[UnitFile].push_back( "factions/planets");
		SubDirectories[UnitFile].push_back( "factions/upgrades");
		SubDirectories[UnitFile].push_back( "factions/neutral");
		SubDirectories[UnitFile].push_back( "factions/aera");
		SubDirectories[UnitFile].push_back( "factions/confed");
		SubDirectories[UnitFile].push_back( "factions/pirates");
		SubDirectories[UnitFile].push_back( "factions/rlaan");

		Directories[UnitSaveFile] = savedunitpath;

		Directories[MeshFile] = sharedmeshes;
		SubDirectories[MeshFile].push_back( "mounts");
		SubDirectories[MeshFile].push_back( "nav/default");

		Directories[TextureFile] = sharedtextures;
		SubDirectories[TextureFile].push_back( "mounts");
		SubDirectories[TextureFile].push_back( "nav/default");

		// We will also look in subdirectories with universe name
		Directories[SystemFile] = sharedsectors;
		SubDirectories[SystemFile].push_back( universe_name);

		Directories[UniverseFile] = shareduniverse;
		Directories[SoundFile] = sharedsounds;
		Directories[CockpitFile] = sharedcockpits;
		Directories[AnimFile] = sharedanims;
		Directories[VSSpriteFile] = sharedsprites;

		Directories[AiFile] = aidir;
		SubDirectories[AiFile].push_back( "events");
		SubDirectories[AiFile].push_back( "script");

		Directories[MissionFile] = "mission";
		Directories[CommFile] = "communications";
		Directories[SaveFile] = "save";
		Directories[MusicFile] = "music";
		Directories[PythonFile] = "bases";
		Directories[BSPFile] = "generatedbsp";
		Directories[AccountFile] = "accounts";

		simulation_atom_var=atof(vs_config->getVariable("general","simulation_atom","0.1").c_str());
		cout << "SIMULATION_ATOM: " << SIMULATION_ATOM << endl;

		/************************* Home directory subdirectories creation ************************/
		CreateDirectoryHome( savedunitpath);
		CreateDirectoryHome( sharedtextures);
		CreateDirectoryHome( sharedtextures+"/backgrounds");
		CreateDirectoryHome( sharedsectors);
		CreateDirectoryHome( sharedsectors+"/"+universe_name);
		CreateDirectoryHome( sharedsounds);
		CreateDirectoryHome( "save");
		CreateDirectoryHome( "generatedbsp");


		// We will be able to automatically add mods files (set of resources or even directory structure similar to the data tree)
		// by just adding a subdirectory named with the mod name in the subdirectory "mods"...
		// I just have to implement that and then add all mods/ subdirs in Rootdir vector
		Rootdir.push_back( homedir);
		InitMods();
		Rootdir.push_back( datadir);

		// NOTE : UniverseFiles cannot use volumes since some are needed by python
		// Also : Have to try with systems, not sure it would work well
		// Setup the use of volumes for certain VSFileType
		volume_format = vs_config->getVariable( "data", "volume_format", "pk3");
        if (volume_format=="vsr") 
            q_volume_format=vfmtVSR; else if (volume_format=="pk3")
            q_volume_format=vfmtPK3; else
            q_volume_format=vfmtUNK;

		if( FileExists( datadir, "/data."+volume_format)>=0)
		{
			// Every kind of file will use the big volume except Unknown files and python files that needs to remain standard files
			for( i=0; i<UnknownFile; i++)
				UseVolumes[i] = 2;
			UseVolumes[PythonFile] = 0;
			UseVolumes[AccountFile] = 0;
		}
		else
		{
			if( FileExists( datadir, "/"+sharedunits+"."+volume_format)>=0)
			{
				UseVolumes[UnitFile] = 1;
				cout<<"Using volume file "<<(datadir+"/"+sharedunits)<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/"+sharedmeshes+"."+volume_format)>=0)
			{
				UseVolumes[MeshFile] = 1;
				cout<<"Using volume file "<<(datadir+"/"+sharedmeshes)<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/"+sharedtextures+"."+volume_format)>=0)
			{
				UseVolumes[TextureFile] = 1;
				cout<<"Using volume file "<<(datadir+"/"+sharedtextures)<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/"+sharedsounds+"."+volume_format)>=0)
			{
				UseVolumes[SoundFile] = 1;
				cout<<"Using volume file "<<(datadir+"/"+sharedsounds)<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/"+sharedcockpits+"."+volume_format)>=0)
			{
				UseVolumes[CockpitFile] = 1;
				cout<<"Using volume file "<<(datadir+"/"+sharedcockpits)<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/"+sharedsprites+"."+volume_format)>=0)
			{
				UseVolumes[VSSpriteFile] = 1;
				cout<<"Using volume file "<<(datadir+"/"+sharedsprites)<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/animations."+volume_format)>=0)
			{
				UseVolumes[AnimFile] = 1;
				cout<<"Using volume file "<<(datadir+"/animations")<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/communications."+volume_format)>=0)
			{
				UseVolumes[CommFile] = 1;
				cout<<"Using volume file "<<(datadir+"/communications")<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/mission."+volume_format)>=0)
			{
				UseVolumes[MissionFile] = 1;
				cout<<"Using volume file "<<(datadir+"/mission")<<".pk3"<<endl;
			}
			if( FileExists( datadir,"/ai."+volume_format)>=0)
			{
				UseVolumes[AiFile] = 1;
				cout<<"Using volume file "<<(datadir+"/ai")<<".pk3"<<endl;
			}
			UseVolumes[ZoneBuffer] = 0;
		}
	}

	void	CreateDirectoryAbs( const char * filename)
	{
		int err;
		if( !DirectoryExists( filename))
		{
			err = mkdir (filename
#if !defined( _WIN32) || defined( __CYGWIN__)
				,0xFFFFFFFF
#endif
			);
			if( err<0 && errno!=EEXIST)
			{
				cerr<<"Errno="<<errno<<" - FAILED TO CREATE : "<<filename<<endl;
				GetError("CreateDirectory");
				VSExit(1);
			}
		}
	}

	void	CreateDirectoryAbs( const string &filename) { CreateDirectoryAbs( filename.c_str()); }
	void	CreateDirectoryHome( const char * filename) { CreateDirectoryAbs( homedir+"/"+string( filename)); }
	void	CreateDirectoryHome( const string &filename) { CreateDirectoryHome( filename.c_str()); }
	void	CreateDirectoryData( const char * filename) { CreateDirectoryAbs( datadir+"/"+string( filename)); }
	void	CreateDirectoryData( const string &filename) { CreateDirectoryData( filename.c_str()); }

	// Absolute directory -- DO NOT USE FOR TESTS IN VOLUMES !!
	bool	DirectoryExists( const char * filename)
	{
		struct stat s;
		if( stat( filename, &s)<0)
			return false;
		if( s.st_mode & S_IFDIR)
		{
			return true;
		}
		return false;
	}
	bool	DirectoryExists( const string &filename) { return DirectoryExists( filename.c_str()); }

	// root is the path to the type directory or the type volume
	// filename is the subdirectory+"/"+filename
	int		FileExists( const string &root, const char * filename, VSFileType type, bool lookinvolume)
	{
		int found = -1;
		bool volok = false;
		string fullpath;
		const char * file;
		if( filename[0]=='/')
			file = filename+1;
		else
			file = filename;
		const char * rootsep = (root==""||root=="/")?"":"/";

		if( !UseVolumes[type] || !lookinvolume)
		{
			if( type == UnknownFile)
				fullpath = root+rootsep+file;
			else
				fullpath = root+rootsep+Directories[type]+"/"+file;

			struct stat s;
                        //cache doesn't work because we *do* create files....
                        //and this is too lowlevel to know which files need to be created... *sigh*
			//static stdext::hash_map<std::string,bool> fileExistsCache;
			//stdext::hash_map<std::string,bool>::iterator iter;
			//iter=fileExistsCache.find(fullpath);
			//if (iter!=fileExistsCache.end()) {
			//    if (iter->second) {
                        //      found=1;
                        //      isin_bigvolumes = VSFSNone;
                        //    }
			//}else {
			    if( stat( fullpath.c_str(), &s) >= 0){
				if( s.st_mode & S_IFDIR) {
				    cerr<<" File is a directory ! ";
				    found = -1;
				    //fileExistsCache[fullpath]=false;
				}
				else {
				    isin_bigvolumes = VSFSNone;
				    found = 1;
				    //fileExistsCache[fullpath]=true;
				}
			    }else {
                              //fileExistsCache[fullpath]=false;
			    }
                            //}
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				CPK3 * vol;
				string filestr;

				// TRY TO OPEN A DATA.VOLFORMAT FILE IN THE ROOT DIRECTORY PASSED AS AN ARG
				filestr = Directories[type]+"/"+file;
				fullpath = root+rootsep+"data."+volume_format;
				stdext::hash_map<string, CPK3 *>::iterator it;
				it = pk3_opened_files.find( fullpath);
				failed+="Looking for file in VOLUME : "+fullpath+"... ";
				if( it==pk3_opened_files.end())
				{
					// File is not opened so we open it and add it in the pk3 file map
					vol = new CPK3;
					if( (volok = vol->Open( fullpath.c_str())))
					{
						failed+=" VOLUME OPENED\n";
						// We add the resource file to the map only if we could have opened it
						std::pair<std::string, CPK3 *> pk3_pair( fullpath, vol);
						pk3_opened_files.insert( pk3_pair);
					}
					else
					{
						failed+=" COULD NOT OPEN VOLUME\n";
					}
				}
				else
				{
					failed+=" VOLUME FOUND\n";
					vol = it->second;
					volok = true;
				}
				// Try to get the file index in the archive
				if( volok)
				{
					//cerr<<"Volume is ok, looking for file in it"<<endl;
					found = vol->FileExists( filestr.c_str());
					if(found>=0)
						isin_bigvolumes = VSFSBig;
				}
				else
					found = -1;

				if( found<0)
				{
					// AND THEN A VOLUME FILE BASED ON DIRECTORIES[TYPE]
					filestr = string( file);
					fullpath = root+rootsep+Directories[type]+"."+volume_format;
					it = pk3_opened_files.find( fullpath);
					failed+="Looking for file in VOLUME : "+fullpath+"... ";
					if( it==pk3_opened_files.end())
					{
						// File is not opened so we open it and add it in the pk3 file map
						vol = new CPK3;
						if( (volok = vol->Open( fullpath.c_str())))
						{
							failed+=" VOLUME OPENED\n";
							// We add the resource file to the map only if we could have opened it
							std::pair<std::string, CPK3 *> pk3_pair( fullpath, vol);
							pk3_opened_files.insert( pk3_pair);
						}
						else
						{
							failed+=" COULD NOT OPEN VOLUME\n";
						}
					}
					else
					{
						failed+=" VOLUME FOUND\n";
						vol = it->second;
						volok = true;
					}
					// Try to get the file index in the archive
					if( volok)
					{
						//cerr<<"Volume is ok, looking for file in it"<<endl;
						found = vol->FileExists( filestr.c_str());
						if(found>=0)
							isin_bigvolumes = VSFSSplit;
					}
					else
						found = -1;
				}
			}
		}
		if( found<0)
		{
			if( !UseVolumes[type])
				failed += "\tTRY LOADING : "+nameof(type)+" "+fullpath+"... NOT FOUND\n";
			else if (VSFS_DEBUG()>1)
				failed += "\tTRY LOADING in "+nameof(type)+" "+fullpath+" : "+file+"... NOT FOUND\n";
		}
		else
		{
			if( !UseVolumes[type])
				failed = "\tTRY LOADING : "+nameof( type)+" "+fullpath+"... SUCCESS";
			else if (VSFS_DEBUG()>1)
				failed = "\tTRY LOADING in "+nameof( type)+" "+fullpath+" : "+file+"... SUCCESS";
            else
                failed.erase();
		}
                //if (found<0&&root=="")
                //return FileExists("/",filename,type,lookinvolume);
		return found;
	}
	int		FileExists( const string &root, const string &filename, VSFileType type, bool lookinvolume) { return FileExists( root, filename.c_str(), type, lookinvolume); }

	int		FileExistsData( const char * filename, VSFileType type) { return FileExists( datadir, filename, type); }
	int		FileExistsData( const string &filename, VSFileType type) { return FileExists( datadir, filename, type); }

	int		FileExistsHome( const char * filename, VSFileType type) { return FileExists( homedir, filename, type); }
	int		FileExistsHome( const string &filename, VSFileType type) { return FileExists( homedir, filename, type); }

	VSError GetError( char * str)
	{
			cerr<<"!!! ERROR/WARNING VSFile : ";
			if( str)
				cerr<<"on "<<str<<" : ";
			if( errno==ENOENT)
			{
				cerr<<"File not found"<<endl;
				return FileNotFound;
			}
			else if( errno==EPERM)
			{
				cerr<<"Permission denied"<<endl;
				return LocalPermissionDenied;
			}
			else if( errno==EACCES)
			{
				cerr<<"Access denied"<<endl;
				return LocalPermissionDenied;
			}
			else
			{
				cerr<<"Unspecified error (maybe to document in VSFile ?)"<<endl;
				return Unspecified;
			}
	}

	VSError LookForFile( const string &file, VSFileType type, VSFileMode mode)
	{
		VSFile vsfile;
		vsfile.SetFilename(file);
		VSError err = LookForFile( vsfile, type, mode);
		return err;
	}

	VSError LookForFile( VSFile & f, VSFileType type, VSFileMode mode)
	{
		int found = -1, shared = false;
		string filepath, curpath, dir, extra(""), subdir;
		failed.erase();
		VSFileType curtype=type;
		// First try in the current path
        switch(type) {
        case UnitFile: 
            extra = "/"+GetUnitDir( f.GetFilename()); 
            break;
        case CockpitFile:
		    // For cockpits we look in subdirectories that have the same name as the cockpit itself
			extra = "/"+string( f.GetFilename());
            break;
        case AnimFile:
            // Animations are always in subdir named like the anim itself
			extra = "/"+string( f.GetFilename());
            break;
        }

		// This test lists all the VSFileType that should be looked for in the current directory
		unsigned int i=0, j=0;

                for (int LC=0;LC<2&&found<0;(LC+=(extra==""?2:1)),extra="") {
		if( current_path.back()!="" && (type==TextureFile || type==MeshFile || type==VSSpriteFile || type==AnimFile))
		{
			curpath = current_path.back();
			subdir = current_subdirectory.back();
			if( extra!="")
				subdir += extra;
			curtype = current_type.back();

			found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), curtype);
			if( found>=0)
			{
				shared = false;
				f.SetAltType( curtype);
			}
			else
			{
				// Set curtype back to original type if we didn't find the file in the current dir
				curtype = type;
				shared = true;
			}
		}

		// FIRST LOOK IN HOMEDIR FOR A STANDARD FILE, SO WHEN USING VOLUME WE DO NOT LOOK FIRST IN VOLUMES
		if( found<0 && UseVolumes[curtype])
		{
			curpath = homedir;
			subdir = "";
			if( extra!="")
				subdir += extra;
			found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), type, false);
	
			for( j=0; found<0 && j<SubDirectories[curtype].size(); j++)
			{
				subdir = SubDirectories[curtype][j];
				if( extra!="")
					subdir += extra;

				found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), curtype, false);
				f.SetVolume( VSFSNone);
			}
		}

		// THEN LOOK IN ALL THE REGISTERED ROOT DIRS
		for( i=0; found<0 && i<Rootdir.size(); i++)
		{
			curpath = Rootdir[i];
			subdir = f.GetSubDirectory();
			if( extra!="")
				subdir += extra;
			found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), type);

			for( j=0; found<0 && j<SubDirectories[curtype].size(); j++)
			{
				curpath = Rootdir[i];
				subdir = SubDirectories[curtype][j];
				if (f.GetSubDirectory().length()) {
					if (subdir.length())subdir+="/";
					subdir+=f.GetSubDirectory();
				}
				if( extra!="")
					subdir += extra;

				found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), curtype);
			}
		}
		if( curtype==CockpitFile)
		{
			for( i=0; found<0 && i<Rootdir.size(); i++)
			{
				curpath = Rootdir[i];
				subdir = "";
				found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), type);

				for( j=0; found<0 && j<SubDirectories[curtype].size(); j++)
				{
					curpath = Rootdir[i];
					subdir = SubDirectories[curtype][j];
					if( extra!="")
						subdir += extra;
	
					found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), curtype);
				}
			}
		}
                }
                if (VSFS_DEBUG()>1) {
		//cerr<<failed<<" - VOLUME TYPE="<<isin_bigvolumes<<endl;
                  if( isin_bigvolumes>VSFSNone)
			 cerr<<failed<<" - INDEX="<<found<<endl<<endl;
                  else
			cerr<<failed<<endl;
                }
		if( found>=0)
		{
			if( (type==SystemFile && i==0)||(type==SoundFile/*right now only allow shared ones?!*/ ) /* Rootdir[i]==homedir*/ )
				shared = true;
			f.SetDirectory( Directories[curtype]);
			f.SetSubDirectory( subdir);
			f.SetRoot( curpath);
			f.SetVolume( isin_bigvolumes);
			// If we found a file in a volume we store its index in the concerned archive
			if( UseVolumes[curtype] && isin_bigvolumes>VSFSNone)
				f.SetIndex( found);
			isin_bigvolumes = VSFSNone;
			if( shared)
			{
				return Shared;
			}
			else
			{
				return Ok;
			}
		}
		return FileNotFound;
	}

	/************************************************************************************************/
	/**** VSFileSystem::VSFile class member functions                                            ****/
	/************************************************************************************************/

	// IMPORTANT NOTE : IN MOST FILE OPERATION FUNCTIONS WE USE THE "alt_type" MEMBER BECAUSE A FILE WHOSE NATIVE TYPE
	// SHOULD BE HANDLED IN VOLUMES MIGHT BE FOUND IN THE CURRENT DIRECTORY OF A TYPE THAT IS NOT HANDLED IN
	// VOLUMES -> SO WE HAVE TO USE THE ALT_TYPE IN MOST OF THE TEST TO USE THE CORRECT FILE OPERATIONS

	void VSFile::private_init()
	{
		fp = NULL;
		size = -1;
		pk3_file           = NULL;
		pk3_extracted_file = NULL;
		offset = 0;
		valid  = false;
		file_type   = alt_type = UnknownFile;
		file_index  = -1;
		volume_type = VSFSNone;
	}

	VSFile::VSFile()
    {
        private_init( );
    }

	VSFile::VSFile( const char * buffer, long bufsize, VSFileType type, VSFileMode mode)
	{
        private_init( );
		this->size = bufsize;
		this->pk3_extracted_file = new char[bufsize+1];
		memcpy( this->pk3_extracted_file, buffer, bufsize);
		pk3_extracted_file[bufsize]=0;
		this->file_type = this->alt_type = ZoneBuffer;
		this->file_mode = mode;
		// To say we want to read in volume even if it is not the case then it will read in pk3_extracted_file
		this->volume_type = VSFSBig;
	}

	VSFile::VSFile( const char * filename, VSFileType type, VSFileMode mode)
	{
        private_init( );
		if( mode == ReadOnly)
			this->OpenReadOnly( filename, type);
		else if (mode == ReadWrite)
			this->OpenReadWrite( filename, type);
		else if (mode == CreateWrite)
			this->OpenCreateWrite( filename, type);
	}

  	VSFile::~VSFile()
	{
		if( fp)
		{
			fclose(fp);
			this->fp = NULL;
		}
		if( pk3_extracted_file)
			delete []pk3_extracted_file;
	}

	void	VSFile::checkExtracted()
	{
		if( q_volume_format==vfmtPK3)
		{
			if( !pk3_extracted_file)
			{
				string full_vol_path;
				if( this->volume_type==VSFSBig)
				{
					full_vol_path = this->rootname+"/data."+volume_format;
				}
				else
				{
					full_vol_path = this->rootname+"/"+Directories[this->alt_type]+"."+volume_format;
				}
				stdext::hash_map<string, CPK3*>::iterator it;
				it = pk3_opened_files.find( full_vol_path);
				if( it==pk3_opened_files.end())
				{
					// File is not opened so we open it and add it in the pk3 file map
					CPK3 * pk3newfile = new CPK3;
					if( !pk3newfile->Open( full_vol_path.c_str()))
					{
						cerr<<"!!! ERROR : opening volume : "<<full_vol_path<<endl;
						VSExit(1);
					}
					std::pair<std::string, CPK3 *> pk3_pair( full_vol_path, pk3newfile);
					pk3_opened_files.insert( pk3_pair);

					this->pk3_file = pk3newfile;
				}
				else
					this->pk3_file = it->second;
				int pk3size=0;
				if( this->file_index != -1)
					pk3_extracted_file = (char *) pk3_file->ExtractFile( this->file_index, &pk3size);
				else
					pk3_extracted_file = (char *) pk3_file->ExtractFile( (this->subdirectoryname+"/"+this->filename).c_str(), &pk3size);
				this->size = pk3size;
				cerr<<"EXTRACTING "<<(this->subdirectoryname+"/"+this->filename)<<" WITH INDEX="<<this->file_index<<" SIZE="<<pk3size<<endl;
			}
		}
	}

	// Open a read only file
	VSError VSFile::OpenReadOnly( const char * file, VSFileType type)
	{
		string filestr;
		int	   found = -1;
		this->file_type = this->alt_type = type;
		this->file_mode = ReadOnly;
		this->filename = string(file);
		failed="";

		VSError err = Ok;
                if (VSFS_DEBUG()) {
                  cerr<<"Loading a " << type << " : "<<file<<endl;
                }
	if( type < ZoneBuffer || type==UnknownFile) // It is a "classic file"
	{
		if( !UseVolumes[type])
		{
			if( type==UnknownFile)
			{
				// We look in the current_path or for a full relative path to either homedir or datadir
				if( current_path.back()!="")
				{
					string filestr1 = current_directory.back()+
							"/"+current_subdirectory.back()+"/"+string( file);
					filestr = current_path.back() + "/" + filestr1;
					if( (found = FileExists( current_path.back(),filestr1))<0 )
						failed += "\t"+filestr+" NOT FOUND !\n";
				}
				if( found<0)
				{
					/*
					filestr = homedir+"/"+file;
					cerr<<"TRYING TO OPEN "<<filestr<<"... ";
					if( (found = FileExists( homedir, file))<0 )
					{
						filestr = datadir+"/"+file;
						found = FileExists( datadir, file);
					}
					*/
					for( int ij=0; ij<Rootdir.size()&&found<0; ij++)
					{
						filestr = Rootdir[ij]+"/"+file;
						found = FileExists( Rootdir[ij], file);
						if( found<0)
							failed += "\tRootdir : "+filestr+" NOT FOUND !\n";
					}
					// Look for relative (to datadir) or absolute named file
					if( found<0)
					{
						filestr = file;
						if( (found=FileExists( "", filestr))<0)
						{
							failed += "\tAbs or rel : "+filestr+" NOT FOUND !\n";
						}
					}
				}
				if( found<0)
				{
                                  if (VSFS_DEBUG()) {
                                    cerr<<failed<<endl;
                                  }
                                  this->valid = false;
                                  err = FileNotFound;
				}
				else
				{
					if( (this->fp = fopen( filestr.c_str(), "rb"))==NULL)
					{
						cerr<<"!!! SERIOUS ERROR : failed to open Unknown file "<<filestr<<" - this should not happen"<<endl;
						VSExit(1);
					}
					this->valid = true;
                                        if (VSFS_DEBUG()>1)
                                          cerr<<filestr<<" SUCCESS !!!"<<endl;
				}
			}
			else
			{
				err = VSFileSystem::LookForFile( *this, type, file_mode);
				if( err>Ok)
				{
					this->valid = false;
					return FileNotFound;
				}
				filestr = this->GetFullPath();
				this->fp = fopen( filestr.c_str(), "rb");
				if( !this->fp)
				{
					cerr<<"!!! SERIOUS ERROR : failed to open "<<filestr<<" - this should not happen"<<endl;
                                        this->valid=false;
					return FileNotFound; // fault!
				}
				this->valid = true;
			}
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				// Here we look for the file but we don't really open it, we just check if it exists
				err = VSFileSystem::LookForFile( *this, type, file_mode);
				if( err>Ok)
				{
					this->valid = false;
					return FileNotFound;
				}
				// Test if we have found a file in another FileType's dir and if it doesn't use volumes
				// If so we open the file as a normal one
				if( this->volume_type==VSFSNone || this->alt_type!=this->file_type && !UseVolumes[this->alt_type])
				{
					filestr = this->GetFullPath();
					this->fp = fopen( filestr.c_str(), "rb");
					if( !this->fp)
					{
						cerr<<"!!! SERIOUS ERROR : failed to open "<<filestr<<" - this should not happen"<<endl;
                                                this->valid=false;
						return FileNotFound;//fault
					}
				}
				this->valid = true;
			}
		}
		if( err<=Ok)
		{
			// We save the current path only when loading a unit, an animation, a sprite or a cockpit
			if( (type==UnitFile || type==AnimFile || type==VSSpriteFile || type==CockpitFile))
			{
				current_path.push_back( this->rootname);
				current_subdirectory.push_back( this->subdirectoryname);
				current_type.push_back( this->alt_type);
                                if (VSFS_DEBUG()>1) {
                                  cerr<<endl<<"BEGINNING OF ";
                                  DisplayType( type);
                                  cerr<<endl;
                                }
			}
		}
	}
	else	// This is a "buffer file"
	{
		if( !this->pk3_extracted_file)
			err = FileNotFound;
		else
			err = Ok;
	}

	return err;
	}

	// We will always write in homedir+Directories[FileType][0]
	// Open a standard file read/write
	VSError VSFile::OpenReadWrite( const char * filename, VSFileType type)
	{
		if( type >= ZoneBuffer && type != UnknownFile)
			return FileNotFound;

		this->file_type = this->alt_type = type;
		this->file_mode = ReadWrite;

		return Ok;
	}

	// We will always write in homedir+Directories[FileType][0]
	// Open (truncate) or create a standard file read/write
	VSError VSFile::OpenCreateWrite( const char * filenam, VSFileType type)
	{
		if( type >= ZoneBuffer && type != UnknownFile)
			return FileNotFound;

		this->file_type = this->alt_type = type;
		this->filename = string( filenam);
		this->file_mode = CreateWrite;

		if( type==SystemFile)
		{
			string dirpath( sharedsectors+"/"+universe_name);
			CreateDirectoryHome( dirpath);
			CreateDirectoryHome( dirpath+"/"+getStarSystemSector( this->filename));
			string fpath( homedir+"/"+dirpath+"/"+this->filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==TextureFile)
		{
			string fpath( homedir+"/"+sharedtextures+"/"+this->filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==UnitFile)
		{
			string fpath( homedir+"/"+savedunitpath+"/"+this->filename);
			this->rootname = homedir;
			this->directoryname = savedunitpath;
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==SaveFile)
		{
			string fpath( homedir+"/save/"+this->filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==BSPFile)
		{
			string fpath( homedir+"/generatedbsp/"+this->filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==AccountFile)
		{
			string fpath( datadir+"/accounts/"+this->filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==UnknownFile)
		{
			string fpath( homedir+"/"+this->filename);
			this->rootname=homedir;
			this->directoryname="";
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}

		return Ok;
	}

	size_t  VSFile::Read( void * ptr, size_t length)
	{
		size_t nbread = 0;

		if( !UseVolumes[this->alt_type] || this->volume_type==VSFSNone)
		{
			assert( fp!=NULL);
			nbread = fread( ptr, 1, length, this->fp);
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				checkExtracted();
				if( length > this->size-this->offset)
					length = this->size - this->offset;
				memcpy( ptr, (pk3_extracted_file+offset), length);
				offset += length;
				nbread = length;
			}
		}
		return nbread;
	}

	VSError  VSFile::ReadLine( void * ptr, size_t length)
	{
		char * ret;
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone)
		{
			ret = fgets( (char *)ptr, length, this->fp);
			if( !ret)
				return Unspecified;
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				checkExtracted();
				ret = (char *) ptr;

				bool nl_found = false;
				int i=0;
				if (VSFS_DEBUG()>1) {
					cerr<<"READLINE STARTING OFFSET="<<offset;
				}
				for( i=0; !nl_found && i<length && offset<size; offset++, i++)
				{
					if( pk3_extracted_file[offset]=='\n' || pk3_extracted_file[offset]=='\r')
					{
						nl_found = true;
						if (VSFS_DEBUG()>1) {
							if( pk3_extracted_file[offset]=='\n')
								cerr<<"\\n ";
							if( pk3_extracted_file[offset]=='\r')
								cerr<<"\\r ";
						}
					}
					else
					{
						ret[i] = pk3_extracted_file[offset];
						if (VSFS_DEBUG()>1) {
							cerr<<std::hex<<ret[i]<<" ";
						}
					}
				}
				this->GoAfterEOL( length);
				ret[i] = 0;
				if (VSFS_DEBUG()>1) {
					cerr<<std::dec<<" - read "<<i<<" char - "<<ret<<endl;
				}
				if( !nl_found)
					return Unspecified;
			}
		}
		return Ok;
	}

	string  VSFile::ReadFull()
	{
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone)
		{
			char * content = new char[this->Size()+1];
			content[this->Size()] = 0;
			int readsize = fread( content, 1, this->Size(), this->fp);
			if( this->Size()!=readsize)
			{
				cerr<<"Read "<<readsize<<" bytes and "<<this->Size()<<" were to be read"<<endl;
				GetError("ReadFull");
				VSExit(1);
			}
			string res( content);
			delete []content;
			return res;
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				checkExtracted();
				offset = this->Size();
				return string( pk3_extracted_file);
			}
		}
		return string("");
	}

	size_t  VSFile::Write( const void * ptr, size_t length)
	{
		if( !UseVolumes[this->alt_type] || this->volume_type==VSFSNone)
		{
			size_t nbwritten = fwrite( ptr, 1, length, this->fp);
			return nbwritten;
		}
		else
		{
			cerr<<"!!! ERROR : Writing is not supported within resource/volume files"<<endl;
			VSExit(1);
		}
		return Ok;
	}

	size_t  VSFile::Write( const string &content)
	{
		std::string::size_type length = content.length();
		return this->Write( content.c_str(), length);
	}

	VSError  VSFile::WriteLine( const void * ptr)
	{
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone)
			fputs( (const char *)ptr, this->fp);
		else
		{
			cerr<<"!!! ERROR : Writing is not supported within resource/volume files"<<endl;
			VSExit(1);
		}
		return Ok;
	}

	void  VSFile::WriteFull( void * ptr)
	{
	}

	int		VSFile::Fprintf( const char * format, ...)
	{
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone)
		{
			va_list ap;
			va_start( ap, format);

			return vfprintf( this->fp, format, ap);
		}
		else
		{
			cerr<<"!!! ERROR : Writing is not supported within resource/volume files"<<endl;
			VSExit(1);
		}
		return 0;
	}

#if 0
#ifdef HAVE_VFSCANF
	int		VSFile::Fscanf( const char * format, ...)
	{
		int ret = -1;
		int readbytes = 0;
		// We add the parameter %n to the format string in order to get the number of bytes read
		int format_length = strlen( format);
		char * newformat = new char[format_length+3];
		memset( newformat, 0, format_length+3);
		memcpy( newformat, format, format_length);
		strcat( newformat, "%n");
		va_list   arglist;
		va_start( arglist, format);
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone)
		{
			//return _input(fp,(unsigned char*)format,arglist);
			ret = vfscanf( this->fp, newformat, arglist);
			va_end( arglist);
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				// If the file has not been extracted yet we do now
				checkExtracted();
				ret = vsscanf( pk3_extracted_file+offset, newformat, arglist);
				readbytes = GetReadBytes( newformat, arglist);
				va_end( arglist);
				cerr<<" SSCANF : Read "<<readbytes<<" bytes"<<endl;
				this->offset += readbytes;
			}
		}
		delete []newformat;
		return ret;
	}
#endif
#endif

	void  VSFile::Begin()
	{
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone || this->file_mode!=ReadOnly)
		{
			fseek( this->fp, 0, SEEK_SET);
		}
		else
		{
			if( q_volume_format==vfmtVSR)
				offset = 0;
			else if( q_volume_format==vfmtPK3)
				offset = 0;
		}
	}

	void  VSFile::End()
	{
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone || this->file_mode!=ReadOnly)
		{
			fseek( this->fp, 0, SEEK_END);
		}
		else
		{
			if( q_volume_format==vfmtVSR)
				offset = size;
			else if( q_volume_format==vfmtPK3)
				offset = size;
		}
	}

	void  VSFile::GoTo( long foffset)	// Does a VSFileSystem::Fseek( fp, offset, SEEK_SET);
	{
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone || this->file_mode!=ReadOnly)
		{
			fseek( this->fp, foffset, SEEK_SET);
		}
		else
		{
			if( q_volume_format==vfmtVSR)
				offset = foffset;
			else if( q_volume_format==vfmtPK3)
				offset = foffset;
		}
	}

	long  VSFile::Size()
	{
		if( size == -1)
		{
			if( !UseVolumes[alt_type] || this->volume_type==VSFSNone || file_mode!=ReadOnly)
			{
				struct stat st;
				if( (fp!=NULL) && fstat( fileno(fp), &st)==0 )
					return (this->size=st.st_size);
				return -1;
			}
			else
			{
				if( q_volume_format==vfmtVSR)
				{
				}
				else if( q_volume_format==vfmtPK3)
				{
					checkExtracted();
					return this->size;
				}
			}
			return -1;
		}
		return this->size;
	}

	void  VSFile::Clear()
	{
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone || file_mode!=ReadOnly)
		{
			fclose( fp);
			this->fp = fopen( this->GetFullPath().c_str(), "w+b");
			// This should not happen
			if( !fp)
			{
				GetError("Clear");
				VSExit(1);
			}
		}
		else
		{
			cerr<<"!!! ERROR : Writing is not supported within resource/volume files"<<endl;
			VSExit(1);
		}
	}

	long  VSFile::GetPosition()
	{
		long ret=0;
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone || file_mode!=ReadOnly)
		{
			ret = ftell( this->fp);
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				ret = offset;
			}
		}
		return ret;
	}
	
	bool  VSFile::Eof()
	{
		bool eof = false;
		if( !UseVolumes[alt_type] || this->volume_type==VSFSNone || file_mode!=ReadOnly)
		{
			eof = vs_feof( this->fp);
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				eof = (offset==this->Size());
			}
		}
		return eof;
	}

	bool  VSFile::Valid()
	{
		return valid;
	}

	void  VSFile::Close()
	{
		if( this->file_type >= ZoneBuffer && this->file_type!=UnknownFile && this->pk3_extracted_file)
		{
			delete this->pk3_extracted_file;
			this->pk3_extracted_file = NULL;
			return;
		}

		if( this->valid && this->file_mode==ReadOnly && (file_type==UnitFile || file_type==AnimFile || file_type==VSSpriteFile || file_type==CockpitFile))
		{
			assert( current_path.size()>1);
			current_path.pop_back();
			current_subdirectory.pop_back();
			current_type.pop_back();
                        if (VSFS_DEBUG()>2) {
                          cerr<<"END OF ";
                          DisplayType( this->file_type);
                          cerr<<endl<<endl;
                        }
		}
		if( !UseVolumes[file_type] || this->volume_type==VSFSNone || file_mode!=ReadOnly)
		{
			fclose( this->fp);
			this->fp = NULL;
		}
		else
		{
			if( q_volume_format==vfmtVSR)
			{
			}
			else if( q_volume_format==vfmtPK3)
			{
				if( pk3_extracted_file)
					delete []pk3_extracted_file;
				pk3_extracted_file = NULL;
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

	string	VSFile::GetFullPath()
	{
        string tmp=this->rootname;
        tmp += (((tmp[tmp.length()-1]=='/')||(tmp[tmp.length()-1]=='/'))?"":"/")+this->directoryname;
        tmp += (((tmp[tmp.length()-1]=='/')||(tmp[tmp.length()-1]=='/'))?"":"/")+this->subdirectoryname;
        tmp += (((tmp[tmp.length()-1]=='/')||(tmp[tmp.length()-1]=='/'))?"":"/")+this->filename;
        return tmp;
	}

   	string	VSFile::GetAbsPath()
	{
        string tmp=this->directoryname;
        tmp += (((tmp[tmp.length()-1]=='/')||(tmp[tmp.length()-1]=='/'))?"":"/")+this->subdirectoryname;
        tmp += (((tmp[tmp.length()-1]=='/')||(tmp[tmp.length()-1]=='/'))?"":"/")+this->filename;
        return tmp;
	}

	void	VSFile::SetType( VSFileType type)
	{ this->file_type = type; }
	void	VSFile::SetAltType( VSFileType type)
	{ this->alt_type = type; }
	void	VSFile::SetIndex( int index)
	{ this->file_index = index; }

	void	VSFile::SetVolume( VSVolumeType big)
	{ this->volume_type = big; }
	bool	VSFile::UseVolume()
	{ return (UseVolumes[alt_type] && volume_type!=VSFSNone); }

	void	VSFile::GoAfterEOL( int length)
	{
		while( this->offset<length && this->offset<this->size && (this->pk3_extracted_file[offset]=='\r' || this->pk3_extracted_file[offset]=='\n'))
		{
                  /*			if( pk3_extracted_file[offset]=='\n')
				cerr<<"\\n ";
			if( pk3_extracted_file[offset]=='\r')
                        cerr<<"\\r ";*/
			this->offset++;
		}
	}
	void	VSFile::GoAfterEOL()
	{
		while( this->offset<this->size && (this->pk3_extracted_file[offset]=='\r' || this->pk3_extracted_file[offset]=='\n'))
		{
                  /*			if( pk3_extracted_file[offset]=='\n')
				cerr<<"\\n ";
			if( pk3_extracted_file[offset]=='\r')
                        cerr<<"\\r ";*/
			this->offset++;
		}
	}
}

#define CASE(a) case a: ostr<<#a; break;
std::ostream& operator<<( std::ostream& ostr, VSFileSystem::VSError err )
{
    switch( err )
    {
        CASE( VSFileSystem::Shared )
        CASE( VSFileSystem::Ok )
        CASE( VSFileSystem::SocketError )
        CASE( VSFileSystem::FileNotFound )
        CASE( VSFileSystem::LocalPermissionDenied )
        CASE( VSFileSystem::RemotePermissionDenied )
        CASE( VSFileSystem::DownloadInterrupted )
        CASE( VSFileSystem::IncompleteWrite )
        CASE( VSFileSystem::IncompleteRead )
        CASE( VSFileSystem::EndOfFile )
        CASE( VSFileSystem::IsDirectory )
        CASE( VSFileSystem::BadFormat )
        CASE( VSFileSystem::Unspecified )
	default : ostr << "VSFileSystem::<undefined VSError>"; break;
    }
    return ostr;
}
#undef CASE

std::ostream& operator<<( std::ostream& ostr, VSFileSystem::VSFileType type )
{
    VSFileSystem::DisplayType( type, ostr );
    return ostr;
}

std::string   nameof( VSFileSystem::VSFileType type )
{
#define CASE(a) case a: return #a; break;
	switch( type)
	{
		CASE( VSFileSystem::UniverseFile)
		CASE( VSFileSystem::SystemFile)
		CASE( VSFileSystem::CockpitFile)
		CASE( VSFileSystem::UnitFile)
		CASE( VSFileSystem::PythonFile)
		CASE( VSFileSystem::TextureFile)
		CASE( VSFileSystem::SoundFile)
		CASE( VSFileSystem::MeshFile)
		CASE( VSFileSystem::CommFile)
		CASE( VSFileSystem::AiFile)
		CASE( VSFileSystem::SaveFile)
		CASE( VSFileSystem::AnimFile)
		CASE( VSFileSystem::VSSpriteFile)
		CASE( VSFileSystem::MissionFile)
		CASE( VSFileSystem::BSPFile)
		CASE( VSFileSystem::MusicFile)
		CASE( VSFileSystem::AccountFile)
		CASE( VSFileSystem::ZoneBuffer)
		CASE( VSFileSystem::UnknownFile)
		default : return "VSFileSystem::<undefined VSFileType>"; break;
	}
#undef CASE
}

#if defined( _WIN32) && !defined( __CYGWIN__)

int scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *),
    int (*compar)(const struct dirent **, const struct dirent **)) {
  int len;
  char *findIn, *d;
  WIN32_FIND_DATA find;
  HANDLE h;
  int nDir = 0, NDir = 0;
  struct dirent **dir = 0, *selectDir;
  unsigned long ret;

  len    = strlen(dirname);
  findIn = (char *) malloc(len+5);
  strcpy(findIn, dirname);
  for (d = findIn; *d; d++) if (*d=='/') *d='\\';
  if ((len==0)) { strcpy(findIn, ".\\*"); }
  if ((len==1)&& (d[-1]=='.')) { strcpy(findIn, ".\\*"); }
  if ((len>0) && (d[-1]=='\\')) { *d++ = '*'; *d = 0; }
  if ((len>1) && (d[-1]=='.') && (d[-2]=='\\')) { d[-1] = '*'; }
  
  if ((h=FindFirstFile(findIn, &find))==INVALID_HANDLE_VALUE) {
    ret = GetLastError();
    if (ret != ERROR_NO_MORE_FILES) {
      // TODO: return some error code
    }
    *namelist = dir;
    return nDir;
  }
  do {
    selectDir=(struct dirent*)malloc(sizeof(struct dirent)+strlen(find.cFileName));
    strcpy(selectDir->d_name, find.cFileName);
    if (!select || (*select)(selectDir)) {
      if (nDir==NDir) {
	struct dirent **tempDir = (dirent **) calloc(sizeof(struct dirent*), NDir+33);
	if (NDir) memcpy(tempDir, dir, sizeof(struct dirent*)*NDir);
	if (dir) free(dir);
	dir = tempDir;
	NDir += 32;
      }
      dir[nDir] = selectDir;
      nDir++;
      dir[nDir] = 0;
    } else {
      free(selectDir);
    }
  } while (FindNextFile(h, &find));
  ret = GetLastError();
  if (ret != ERROR_NO_MORE_FILES) {
    // TODO: return some error code
  }
  FindClose(h);

  free (findIn);

  if (compar) qsort (dir, nDir, sizeof(*dir),
		     (int(*)(const void*, const void*))compar);

  *namelist = dir;
  return nDir;
}

int alphasort (struct dirent **a, struct dirent **b) {
  return strcmp ((*a)->d_name, (*b)->d_name);
}

#endif

