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
#include "map"

using std::map;

#define VSFS_DEBUG

char *CONFIGFILE;
char pwd[65536];

VSVolumeType isin_bigvolumes = None;

ObjSerial	serial_seed = 0;
ObjSerial	getUniqueSerial()
{
	// MAYBE CHANGE TO SOMETHING MORE "RANDOM"
	serial_seed = (serial_seed+3)%MAXSERIAL;
	return serial_seed;
}

extern string GetUnitDir( string filename);

int	selectdirs( const struct dirent * entry)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
	// Have to check if we have the full path or just relative (which would be a problem)
	cerr<<"Read directory entry : "<<entry->d_name<<endl;
	struct stat s;
	if( stat( entry->d_name, &s)<0)
		return false;
	if( s.st_mode & S_IFDIR)
	{
		return 1;
	}
#else
	if( entry->d_type==DT_DIR)
		return 1;
#endif
	return 0;
}

int	selectpk3s( const struct dirent * entry)
{
	// If this is a regular file and we have ".pk3" in it
	if( (string( entry->d_name).find( ".pk3"))!=std::string::npos)
		return 1;
	return 0;
}

namespace VSFileSystem
{
	#define CASE(a) case a: cerr<<#a; break;
	void	DisplayType( VSFileSystem::VSFileType type)
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
			CASE( VSFileSystem::SpriteFile)
			CASE( VSFileSystem::MissionFile)
			CASE( VSFileSystem::BSPFile)
			CASE( VSFileSystem::MusicFile)
			CASE( VSFileSystem::Unknown)
		}
	}

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
	string sharedanims;
	string sharedsprites;
	string savedunitpath;
	string moddir;
	string datadir;
	string homedir;

	string config_file;
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
	map<string, CPK3 *>	pk3_opened_files;

	/************************************************************************************************/
	/**** vs_path functions                                                                      ****/
	/************************************************************************************************/

	/*
	void vs_mkdir(const std::string &s) {
		mkdir (s.c_str()
#if !defined( _WIN32) || defined( __CYGWIN__)
		   ,0xFFFFFFFF
#endif
		);
	}

	void changehome(bool makehomedir) {
		  static char pw_dir[2000];
		#ifndef _WIN32
		  struct passwd *pwent;
		  pwent = getpwuid (getuid());
		  vs_setdir (pwent->pw_dir);
		#else
		  if (!makehomedir)
			vs_setdir (pw_dir);
		#endif
		  if (makehomedir) {
		  getcwd (pw_dir,1998);
		#ifndef _WIN32
		  vs_resetdir();
		#endif
		  vs_setdir (pw_dir);
		  if (chdir (HOMESUBDIR.c_str())==-1) {
			  //      system ("mkdir " HOMESUBDIR);
			mkdir (HOMESUBDIR.c_str()
		#if !defined(_WIN32) || defined(__CYGWIN__) 
				  , 0xFFFFFFFF
		#endif		  
				  );
			} else {
			  chdir ("..");
			}
		  std::string genbsp(HOMESUBDIR +"/generatedbsp");
			if (chdir (genbsp.c_str())==-1) {
								mkdir (genbsp.c_str()
		#if !defined(_WIN32) || defined(__CYGWIN__) 
					, 0xFFFFFFFF
		#endif
				);

				
			}else {
				chdir (pw_dir);
			}
			std::string savetmp  (HOMESUBDIR+"/save");
			if (chdir (savetmp.c_str())==-1) {
			  mkdir (savetmp.c_str()
		#if !defined(_WIN32) || defined(__CYGWIN__) 
				  , 0xFFFFFFFF
		#endif		  
				  );
			  //system ("mkdir " HOMESUBDIR "/generatedbsp");
			}else {
			  chdir (pw_dir);
			}
		  }
		  vs_chdir (HOMESUBDIR.c_str());
		}
		void returnfromhome() {
		  vs_cdup();
		  vs_resetdir();
		}


	void initpaths (const std::string& modname) {
		#ifndef _WIN32
			
		  datadir = getdatadir();
		  if (modname.size())
			HOMESUBDIR = string(".")+modname;
		  if (modname.size())
			  datadir+=string(DELIMSTR)+modname;
		#else
		  if (modname.size())
			chdir (modname.c_str());
		  getcwd(pwd,65534);
		  
		  pwd[65534]=0;
		  datadir=pwd;
		#endif
		  sharedsounds = datadir;

		  cerr << "Data directory is " << datadir << endl;
		  FILE *fp= VSFileSystem::vs_open (CONFIGFILE,"r");

		  //check if we have a config file in home dir
		  changehome(true);
		  char myhomedir[8192];
		  getcwd (myhomedir,8191);
		  myhomedir[8191]='\0';
		  homedir = myhomedir;
		  FILE *fp1= VSFileSystem::vs_open (CONFIGFILE,"r");
		  if (fp1) {
			//  we have a config file in home directory
			VSFileSystem::vs_close (fp1);
			vs_config=createVegaConfig(CONFIGFILE); // move config to global or some other struct
			cout << "using config file in home dir" << endl;
		  }else if (fp) {
			// we don't have a config file in home dir
			// but we have one in the data dir
			chdir(datadir.c_str());

			VSFileSystem::vs_close (fp);
			fp =NULL;
			returnfromhome();
			vs_config = createVegaConfig (CONFIGFILE);
			cout << "using config file in data dir " << datadir << endl;
			changehome();
		  } else {

			// no config file in home dir or data dir
			VSFileSystem::vs_fprintf (stderr,"Could not open config file in either %s/%s\nOr in ~/.vegastrike/%s\n",datadir.c_str(),CONFIGFILE,CONFIGFILE);
			exit (-1);
		  }
		  if (fp)
			VSFileSystem::vs_close (fp);
		  returnfromhome();

		  char mycwd [256];
		  getcwd(mycwd,255);
		  mycwd[254]=mycwd[255]=0;

		  string config_datadir = vs_config->getVariable ("data","directory",datadir);
		  if(config_datadir!=datadir){
			cout << "using data dir " << config_datadir << " from config file" << endl;
			datadir=config_datadir;
		  }
		  vs_chdir (vs_config->getVariable ("data","sharedtextures","textures").c_str());
		  getcwd (pwd,8191);
		  sharedtextures = string (pwd);
		  vs_cdup();
		  vs_chdir (vs_config->getVariable ("data","sharedsounds","sounds").c_str());
		  getcwd (pwd,8191);
		  sharedsounds = string (pwd);
		  vs_cdup();
		  vs_chdir (vs_config->getVariable ("data","sharedmeshes","meshes").c_str());
		  getcwd (pwd,8191);
		  sharedmeshes = string (pwd);
		  vs_cdup();
		  vs_chdir (vs_config->getVariable ("data","sharedunits","units").c_str());
		  getcwd (pwd,8191);
		  sharedunits = string (pwd);


		  vs_cdup();
		  if (datadir.end()!=datadir.begin()) {
			if (*(datadir.end()-1)!='/'&&*(datadir.end()-1)!='\\') {
			  datadir+=DELIM;
			}
		  }
		  if (sharedtextures.end()!=sharedtextures.begin()) {
			if (*(sharedtextures.end()-1)!='/'&&*(sharedtextures.end()-1)!='\\') {
			  sharedtextures+=DELIM;
			}
		  }
		  if (sharedmeshes.end()!=sharedmeshes.begin()) {
			if (*(sharedmeshes.end()-1)!='/'&&*(sharedmeshes.end()-1)!='\\') {
			  sharedmeshes+=DELIM;
			}
		  }
		  if (sharedunits.end()!=sharedunits.begin()) {
			if (*(sharedunits.end()-1)!='/'&&*(sharedunits.end()-1)!='\\') {
			  sharedunits+=DELIM;
			}
		  }
		  if (sharedsounds.end()!=sharedsounds.begin()) {
			if (*(sharedsounds.end()-1)!='/'&&*(sharedsounds.end()-1)!='\\') {
			  sharedsounds+=DELIM;
			}
		  }
		}
		std::string GetSharedMeshPath (const std::string &name) {
		  return sharedmeshes+"/"+name;
		}
		std::string GetSharedUnitPath () {
		  return sharedunits+"/";
		}
		std::string GetSharedTexturePath (const std::string &name) {
		  return sharedtextures+"/"+name;
		}
		std::string GetSharedSoundPath (const std::string &name) {
		  return sharedsounds+"/"+name;
		}
		*/

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


		/*
		void vs_chdir (const char *path) {
		  if (path[0]!='\0') {
			if (path[0]=='.'&&path[1]=='.') {
			  vs_cdup();
			  return;
			}
		  }
		  if (chdir (path)!=-1) {
			std::string tpath = path;
			if (tpath.end()!=tpath.begin())
			  if ((*(tpath.end()-1)!='/')&&((*(tpath.end()-1))!='\\'))
			tpath+='/';
			curdir.push_back (tpath);
		  } else {
			curdir.push_back (string("~"));
		  }
		}
		void vs_setdir (const char * path) {
		  getcwd (pwd,8191);
		  savedpwd.push_back (string (pwd));
		  savedcurdir.push_back (curdir);
		  curdir.clear();
		  curdir.push_back (path);
		  chdir (path);
		}
		void vs_resetdir () {
		  chdir (savedpwd.back().c_str());
		  curdir = savedcurdir.back();
		  savedcurdir.pop_back();
		  savedpwd.pop_back();
		}

		void vs_cdup() {
		  if (!curdir.empty()) {
			if ((*curdir.back().begin())!='~') {
			  chdir ("..");
			}
			curdir.pop_back ();
		  }
		}
		*/

	/************************************************************************************************/
	/**** VSFileSystem wrappers to stdio function calls                                          ****/
	/************************************************************************************************/

	FILE *	vs_open( const char * filename, const char * mode)
	{
		cerr<<"-= VS_OPEN in mode "<<mode<<" =- ";

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
			#ifdef VSFS_DEBUG
				if( fp)
					cerr<<fullpath<<" SUCCESS !!!"<<endl;
				else
					cerr<<output<<" NOT FOUND !!!"<<endl;
			#endif
			}
		#ifdef VSFS_DEBUG
			else
			{
				fp = fopen( fullpath.c_str(), mode);
				if( fp)
				{
					cerr<<fullpath<<" opened for writing SUCCESS !!!"<<endl;
				}
				else
				{
					cerr<<fullpath<<" FAILED !!!"<<endl;
				}
			}
		#endif
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

#ifdef HAVE_VFSCANF
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
		homedir = datadir+"/.vegastrike";
#endif
		cerr<<"USING HOMEDIR : "<<homedir<<endl;
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
		data_paths.push_back( "../data");
		data_paths.push_back( "../../data");
		data_paths.push_back( "../Resources/data");
		data_paths.push_back( "/usr/share/local/vegastrike/data");
		data_paths.push_back( "/usr/local/vegastrike/data");
		data_paths.push_back( "/usr/share/vegastrike/data");
		data_paths.push_back( "/opt/share/vegastrike/data");
		data_paths.push_back( "../data4.x");
		data_paths.push_back( "../../data4.x");
		data_paths.push_back( "../Resources/data4.x");
		data_paths.push_back( "/usr/share/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/share/vegastrike/data4.x");
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
				{
					datadir = string( tmppath)+"/"+(*vsit);
					if( chdir( datadir.c_str())<0)
					{
						cerr<<"Error changing to datadir"<<endl;
						exit(1);
					}
					getcwd( tmppath, 16384);
					datadir = string( tmppath);
				}
				else
					datadir = (*vsit);
				cerr<<"Using "<<datadir<<" as data directory"<<endl;
				break;
			}
		}
		data_paths.clear();
	}

	void	LoadConfig( string subdir)
	{
		// First check if we have a config file in homedir+"/"+subdir or in datadir+"/"+subdir
		if( subdir!="")
		{
			if( DirectoryExists( homedir+"/mods/"+subdir))
			{
				if( FileExists( homedir+"/mods/"+subdir, config_file)>=0)
					cout<<"CONFIGFILE - Found a config file in home directory, using : "<<(homedir+"/mods/"+subdir+"/"+config_file)<<endl;
			}
			else
			{
				cout<<"WARNING : coudn't find a mod named '"<<subdir<<"' in homedir/mods"<<endl;
				if( DirectoryExists( datadir+"/mods/"+subdir))
				{
					if( FileExists( datadir+"/mods/"+subdir, config_file)>=0)
						cout<<"CONFIGFILE - Found a config file in data directory, using : "<<(datadir+"/mods/"+subdir+"/"+config_file)<<endl;
				}
				else
				{
					cout<<"ERROR : coudn't find a mod named '"<<subdir<<"' in datadir/mods"<<endl;
					exit(1);
				}
			}
		}
		

		// Next check if we have a config file in homedir
		if( FileExists( homedir, config_file)>=0)
		{
			cerr<<"CONFIGFILE - Found a config file in home directory, using : "<<(homedir+"/"+config_file)<<endl;
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
		char * conffile = new char[config_file.length()+1];
		conffile[config_file.length()] = 0;
		memcpy( conffile, config_file.c_str(), config_file.length());
		vs_config = createVegaConfig( conffile);
		delete []conffile;

		// Now check if there is a data directory specified in it
		string data_path( vs_config->getVariable( "data", "datadir", ""));
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
		// Scan for mods
		string curpath;
		struct dirent ** dirlist;
		int ret = scandir( (homedir+"/"+moddir).c_str(), &dirlist, selectdirs, 0);
		if( ret <0)
			return;
		else
		{
			while( ret--)
			{
				curpath = homedir+"/"+moddir+"/"+dirlist[ret]->d_name;
				cout<<"Adding mod path : "<<curpath<<endl;
				Rootdir.push_back( curpath);
			}
		}
		free( dirlist);
		ret = scandir( (datadir+"/"+moddir).c_str(), &dirlist, selectdirs, 0);
		if( ret <0)
			return;
		else
		{
			while( ret--)
			{
				curpath = datadir+"/"+moddir+"/"+dirlist[ret]->d_name;
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
		current_type.push_back( Unknown);

		int i;
		for( i=0; i<=Unknown; i++)
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
		// Sprite are in sprites/ or in ./ (when full subpath is provided) or in the current cockpit dir that is being loaded
		
		// First allocate an empty directory list for each file type
		for( i=0; i<Unknown; i++)
		{
			vector<string> vec;
			Directories.push_back( "");
			SubDirectories.push_back( vec);
		}

		moddir = vs_config->getVariable( "data", "modpath", "mods");
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
		universe_name = vs_config->getVariable( "general", "galaxy", "milky_way.xml");

		// Setup the directory lists we know about - note these are relative paths to datadir or homedir
		// ----- THE Directories vector contains the resource/volume files name without extension or the main directory to files
		Directories[UnitFile] = sharedunits;
		//SubDirectories[UnitFile].push_back( "factions");
		SubDirectories[UnitFile].push_back( "subunits");
		SubDirectories[UnitFile].push_back( "weapons");
		SubDirectories[UnitFile].push_back( "factions/planets");
		SubDirectories[UnitFile].push_back( "factions/upgrades");
		SubDirectories[UnitFile].push_back( "factions/neutral");
		SubDirectories[UnitFile].push_back( "factions/aera");
		SubDirectories[UnitFile].push_back( "factions/confed");
		SubDirectories[UnitFile].push_back( "factions/pirates");
		SubDirectories[UnitFile].push_back( "factions/rlaan");

		Directories[MeshFile] = sharedmeshes;
		SubDirectories[MeshFile].push_back( "mounts");
		SubDirectories[MeshFile].push_back( "nav/default");

		Directories[TextureFile] = sharedtextures;
		SubDirectories[TextureFile].push_back( "nav/default");

		// We will also look in subdirectories with universe name
		Directories[SystemFile] = sharedsectors;
		SubDirectories[SystemFile].push_back( universe_name);

		Directories[UniverseFile] = shareduniverse;
		Directories[SoundFile] = sharedsounds;
		Directories[CockpitFile] = sharedcockpits;
		Directories[AnimFile] = sharedanims;
		Directories[SpriteFile] = sharedsprites;

		Directories[AiFile] = "ai";
		SubDirectories[AiFile].push_back( "events");
		SubDirectories[AiFile].push_back( "script");

		Directories[MissionFile] = "mission";
		Directories[CommFile] = "communications";
		Directories[SaveFile] = "save";
		Directories[MusicFile] = "music";
		Directories[PythonFile] = "bases";
		Directories[BSPFile] = "generatedbsp";

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
		CreateDirectoryHome( moddir);

		// We will be able to automatically add mods files (set of resources or even directory structure similar to the data tree)
		// by just adding a subdirectory named with the mod name in the subdirectory "mods"...
		// I just have to implement that and then add all mods/ subdirs in Rootdir vector
		Rootdir.push_back( homedir);
		//InitMods();
		Rootdir.push_back( datadir);

		// NOTE : UniverseFiles cannot use volumes since some are needed by python
		// Also : Have to try with systems, not sure it would work well
		// Setup the use of volumes for certain VSFileType
		volume_format = vs_config->getVariable( "data", "volume_format", "pk3");

		if( FileExists( datadir, "/data."+volume_format)>=0)
		{
			// Every kind of file will use the big volume except Unknown files and python files that needs to remain standard files
			for( i=0; i<Unknown; i++)
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
				UseVolumes[SpriteFile] = 1;
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
		}
	}

	void	CreateDirectoryAbs( const char * filename)
	{
		int err;
		if( chdir( filename)==-1)
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

	void	CreateDirectoryAbs( string filename) { CreateDirectoryAbs( filename.c_str()); }
	void	CreateDirectoryHome( const char * filename) { CreateDirectoryAbs( homedir+"/"+string( filename)); }
	void	CreateDirectoryHome( string filename) { CreateDirectoryHome( filename.c_str()); }
	void	CreateDirectoryData( const char * filename) { CreateDirectoryAbs( datadir+"/"+string( filename)); }
	void	CreateDirectoryData( string filename) { CreateDirectoryData( filename.c_str()); }

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
	bool	DirectoryExists( string filename) { return DirectoryExists( filename.c_str()); }

	// root is the path to the type directory or the type volume
	// filename is the subdirectory+"/"+filename
	int		FileExists( string root, const char * filename, VSFileType type, bool lookinvolume)
	{
		int found = -1;
		bool volok = false;
		string fullpath;
		const char * file;
		if( filename[0]=='/')
			file = filename+1;
		else
			file = filename;
		if( !UseVolumes[type] || !lookinvolume)
		{
			if( type == Unknown)
				fullpath = root+"/"+file;
			else
				fullpath = root+"/"+Directories[type]+"/"+file;

			struct stat s;
			if( stat( fullpath.c_str(), &s) >= 0)
			{
				if( s.st_mode & S_IFDIR)
				{
					cerr<<" File is a directory ! ";
					found = -1;
				}
				else
				{
					isin_bigvolumes = None;
					found = 1;
				}
			}
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
			{
				CPK3 * vol;
				string filestr;

				// TRY TO OPEN A DATA.VOLFORMAT FILE IN THE ROOT DIRECTORY PASSED AS AN ARG
				filestr = Directories[type]+"/"+file;
				fullpath = root+"/data."+volume_format;
				map<string, CPK3 *>::iterator it;
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
						pair<string, CPK3 *> pk3_pair( fullpath, vol);
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
						isin_bigvolumes = Big;
				}
				else
					found = -1;

				if( found<0)
				{
					// AND THEN A VOLUME FILE BASED ON DIRECTORIES[TYPE]
					filestr = string( file);
					fullpath = root+"/"+Directories[type]+"."+volume_format;
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
							pair<string, CPK3 *> pk3_pair( fullpath, vol);
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
							isin_bigvolumes = Split;
					}
					else
						found = -1;
				}
			}
		}
		if( found<0)
		{
			failed = "";
			fullpath = string( filename);
			if( !UseVolumes[type])
				failed += "\tTRY LOADING : "+fullpath+"... NOT FOUND\n";
			else
				failed += "\tTRY LOADING in "+fullpath+" : "+file+"... NOT FOUND\n";
		}
		else
		{
			failed = "";
			if( !UseVolumes[type])
				failed += "\tTRY LOADING : "+fullpath+"... SUCCESS";
			else
				failed += "\tTRY LOADING in "+fullpath+" : "+file+"... SUCCESS";
		}
		return found;
	}
	int		FileExists( string root, string filename, VSFileType type, bool lookinvolume) { return FileExists( root, filename.c_str(), type, lookinvolume); }

	int		FileExistsData( const char * filename, VSFileType type) { return FileExists( datadir, filename, type); }
	int		FileExistsData( string filename, VSFileType type) { return FileExists( datadir, filename, type); }

	int		FileExistsHome( const char * filename, VSFileType type) { return FileExists( homedir, filename, type); }
	int		FileExistsHome( string filename, VSFileType type) { return FileExists( homedir, filename, type); }

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

	VSError LookForFile( string & file, VSFileType type, VSFileMode mode)
	{
		VSFile vsfile;
		vsfile.SetFilename( string(file));
		VSError err = LookForFile( vsfile, type, mode);
		return err;
	}

	VSError LookForFile( VSFile & f, VSFileType type, VSFileMode mode)
	{
		int found = -1, shared = false;
		string filepath, curpath, dir, extra(""), subdir;
		failed="";
		VSFileType curtype=type;
		// First try in the current path
		if( type==UnitFile)
			extra = "/"+GetUnitDir( f.GetFilename());
		// For cockpits we look in subdirectories that have the same name as the cockpit itself
		if( type==CockpitFile)
			extra = "/"+string( f.GetFilename());
		/* Animations are always in subdir named like the anim itself */
		if( type==AnimFile)
			extra = "/"+string( f.GetFilename());

		// This test lists all the VSFileType that should be looked for in the current directory
		if( current_path.back()!="" && (type==TextureFile || type==MeshFile || type==SpriteFile || type==AnimFile))
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
		unsigned int i=0, j=0;

		// FIRST LOOK IN HOMEDIR FOR A STANDARD FILE, SO WHEN USING VOLUME WE DO NOT LOOK FIRST IN VOLUMES
		if( found<0 && UseVolumes[type])
		{
			curpath = homedir;
			subdir = "";
			if( extra!="")
				subdir += extra;
			found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), type, false);
	
			for( j=0; found<0 && j<SubDirectories[type].size(); j++)
			{
				subdir = SubDirectories[type][j];
				if( extra!="")
					subdir += extra;

				found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), type, false);
				f.SetVolume( None);
			}
		}

		// THEN LOOK IN ALL THE REGISTERED ROOT DIRS
		for( i=0; found<0 && i<Rootdir.size(); i++)
		{
			curpath = Rootdir[i];
			subdir = "";
			if( extra!="")
				subdir += extra;
			found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), type);

			for( j=0; found<0 && j<SubDirectories[type].size(); j++)
			{
				curpath = Rootdir[i];
				subdir = SubDirectories[type][j];
				if( extra!="")
					subdir += extra;

				found = FileExists( curpath, (subdir+"/"+f.GetFilename()).c_str(), type);
			}
		}

	#ifdef VSFS_DEBUG
		//cerr<<failed<<" - VOLUME TYPE="<<isin_bigvolumes<<endl;
		if( isin_bigvolumes>None)
			cerr<<failed<<" - INDEX="<<found<<endl<<endl;
		else
			cerr<<failed<<endl;
	#endif
		if( found>=0)
		{
			// We save the current path only when loading a unit, an animation, a sprite or a cockpit
			if( (type==UnitFile || type==AnimFile || type==SpriteFile || type==CockpitFile))
			{
				current_path.push_back( curpath);
				current_subdirectory.push_back( subdir);
				current_type.push_back( type);
		#ifdef VSFS_DEBUG
				cerr<<endl<<"BEGINNING OF ";
				DisplayType( type);
				cerr<<endl;
		#endif
			}
			if( type==SystemFile && Rootdir[i]==homedir)
				shared = true;
			f.SetDirectory( Directories[curtype]);
			f.SetSubDirectory( subdir);
			f.SetRoot( curpath);
			f.SetVolume( isin_bigvolumes);
			// If we found a file in a volume we store its index in the concerned archive
			if( UseVolumes[curtype] && isin_bigvolumes>None)
				f.SetIndex( found);
			isin_bigvolumes = None;
			if( shared)
				return Shared;
			else
				return Ok;
		}
		return FileNotFound;
	}

	/************************************************************************************************/
	/**** VSFileSystem::VSFile class member functions                                            ****/
	/************************************************************************************************/

	// IMPORTANT NOTE : IN MOST FILE OPERATION FUNCTIONS WE USE THE "alt_type" MEMBER BECAUSE A FILE WHOSE NATIVE TYPE
	// SHOULD BE HANDLED IN VOLUMES MIGHT BE FOUND IN THE CURRENT DIRECTORY OF A TYPE THAT IS NOT HANDLED IN
	// VOLUMES -> SO WE HAVE TO USE THE ALT_TYPE IN MOST OF THE TEST TO USE THE CORRECT FILE OPERATIONS

	VSFile::VSFile()
	{
		fp = NULL;
		size = -1;
		pk3_extracted_file = NULL;
		offset = 0;
		valid = false;
		file_type = alt_type = Unknown;
		file_index = -1;
		volume_type = None;
	}

	VSFile::VSFile( const char * filename, VSFileType type, VSFileMode mode)
	{
		VSFile::VSFile();
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
		if( volume_format == "pk3")
		{
			if( !pk3_extracted_file)
			{
				string full_vol_path;
				if( this->volume_type==Big)
				{
					full_vol_path = this->rootname+"/data."+volume_format;
				}
				else
				{
					full_vol_path = this->rootname+"/"+Directories[this->alt_type]+"."+volume_format;
				}
				map<string, CPK3*>::iterator it;
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
					pair<string, CPK3 *> pk3_pair( full_vol_path, pk3newfile);
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

		cerr<<"Loading a ";
		DisplayType(type);
		cerr<<" :\n";
		if( !UseVolumes[type])
		{
			if( type==Unknown)
			{
				// We look in the current_path or for a full relative path to either homedir or datadir
				if( current_path.back()!="")
				{
					filestr = current_directory.back()+"/"+current_subdirectory.back()+"/"+string( file);
					if( (found = FileExists( current_path.back(),filestr))<0 )
						failed += "\t"+current_path.back()+"/"+filestr+" NOT FOUND !\n";
				}
				if( found<0)
				{
					filestr = homedir+"/"+file;
					cerr<<"TRYING TO OPEN "<<filestr<<"... ";
					if( (found = FileExists( homedir, file))<0 )
					{
						filestr = datadir+"/"+file;
						found = FileExists( datadir, file);
					}
				}
				if( found<0)
				{
			#ifdef VSFS_DEBUG
					cerr<<failed<<endl;
			#endif
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
					VSExit(1);
				}
				this->valid = true;
			}
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
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
				if( this->volume_type==None || this->alt_type!=this->file_type && !UseVolumes[this->alt_type])
				{
					filestr = this->GetFullPath();
					this->fp = fopen( filestr.c_str(), "rb");
					if( !this->fp)
					{
						cerr<<"!!! SERIOUS ERROR : failed to open "<<filestr<<" - this should not happen"<<endl;
						VSExit(1);
					}
				}
				this->valid = true;
			}
		}
		return err;
	}

	// We will always write in homedir+Directories[FileType][0]
	// Open a standard file read/write
	VSError VSFile::OpenReadWrite( const char * filename, VSFileType type)
	{
		this->file_type = this->alt_type = type;
		this->file_mode = ReadWrite;

		return Ok;
	}

	// We will always write in homedir+Directories[FileType][0]
	// Open (truncate) or create a standard file read/write
	VSError VSFile::OpenCreateWrite( const char * filename, VSFileType type)
	{
		this->file_type = this->alt_type = type;
		this->file_mode = CreateWrite;

		if( type==SystemFile)
		{
			string dirpath( sharedsectors+"/"+universe_name);
			CreateDirectoryHome( dirpath);
			string fpath( homedir+"/"+dirpath+"/"+filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==TextureFile)
		{
			string fpath( homedir+"/"+sharedtextures+"/"+filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==UnitFile)
		{
			string fpath( homedir+"/"+savedunitpath+"/"+filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==SaveFile)
		{
			string fpath( homedir+"/save/"+filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==BSPFile)
		{
			string fpath( homedir+"/generatedbsp/"+filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==AccountFile)
		{
			string fpath( datadir+"/accounts/"+filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}
		else if( type==Unknown)
		{
			string fpath( homedir+filename);
			this->fp = fopen( fpath.c_str(), "wb");
			if( !fp)
				return LocalPermissionDenied;
		}

		return Ok;
	}

	size_t  VSFile::Read( void * ptr, size_t length)
	{
		size_t nbread = 0;

		if( !UseVolumes[this->alt_type] || this->volume_type==None)
		{
			assert( fp!=NULL);
			nbread = fread( ptr, 1, length, this->fp);
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
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
		if( !UseVolumes[alt_type] || this->volume_type==None)
		{
			ret = fgets( (char *)ptr, length, this->fp);
			if( !ret)
				return Unspecified;
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
			{
				checkExtracted();
				ret = (char *) ptr;

				bool nl_found = false;
				int i=0;
				//cerr<<"READLINE STARTING OFFSET="<<offset;
				for( i=0; !nl_found && i<length && offset<size; offset++, i++)
				{
					if( pk3_extracted_file[offset]=='\n' || pk3_extracted_file[offset]=='\r')
					{
						offset++;
						nl_found = true;
						if( pk3_extracted_file[offset]=='\n')
							cerr<<"\\n ";
						if( pk3_extracted_file[offset]=='\r')
							cerr<<"\\r ";
					}
					else
					{
						ret[i] = pk3_extracted_file[offset];
						cerr<<hex<<ret[i]<<" ";
					}
				}
				this->GoAfterEOL( length);
				ret[i] = 0;
				cerr<<dec<<" - read "<<i<<" char - "<<ret<<endl;
				if( !nl_found)
					return Unspecified;
			}
		}
		return Ok;
	}

	string  VSFile::ReadFull()
	{
		if( !UseVolumes[alt_type] || this->volume_type==None)
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
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
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
		if( !UseVolumes[this->alt_type] || this->volume_type==None)
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

	size_t  VSFile::Write( string content)
	{
		std::string::size_type length = content.length();
		return this->Write( content.c_str(), length);
	}

	VSError  VSFile::WriteLine( const void * ptr)
	{
		if( !UseVolumes[alt_type] || this->volume_type==None)
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
		if( !UseVolumes[alt_type] || this->volume_type==None)
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
		if( !UseVolumes[alt_type] || this->volume_type==None)
		{
			//return _input(fp,(unsigned char*)format,arglist);
			ret = vfscanf( this->fp, newformat, arglist);
			va_end( arglist);
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
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
		if( !UseVolumes[alt_type] || this->volume_type==None || this->file_mode!=ReadOnly)
		{
			fseek( this->fp, 0, SEEK_SET);
		}
		else
		{
			if( volume_format=="vsr")
				offset = 0;
			else if( volume_format=="pk3")
				offset = 0;
		}
	}

	void  VSFile::End()
	{
		if( !UseVolumes[alt_type] || this->volume_type==None || this->file_mode!=ReadOnly)
		{
			fseek( this->fp, 0, SEEK_END);
		}
		else
		{
			if( volume_format=="vsr")
				offset = size;
			else if( volume_format=="pk3")
				offset = size;
		}
	}

	void  VSFile::GoTo( long foffset)	// Does a VSFileSystem::Fseek( fp, offset, SEEK_SET);
	{
		if( !UseVolumes[alt_type] || this->volume_type==None || this->file_mode!=ReadOnly)
		{
			fseek( this->fp, foffset, SEEK_SET);
		}
		else
		{
			if( volume_format=="vsr")
				offset = foffset;
			else if( volume_format=="pk3")
				offset = foffset;
		}
	}

	long  VSFile::Size()
	{
		if( size == -1)
		{
			if( !UseVolumes[alt_type] || this->volume_type==None || file_mode!=ReadOnly)
			{
				struct stat st;
				if( fstat( fileno(fp), &st)==0)
					return (this->size=st.st_size);
				return -1;
			}
			else
			{
				if( volume_format=="vsr")
				{
				}
				else if( volume_format=="pk3")
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
		if( !UseVolumes[alt_type] || this->volume_type==None || file_mode!=ReadOnly)
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
		if( !UseVolumes[alt_type] || this->volume_type==None || file_mode!=ReadOnly)
		{
			ret = ftell( this->fp);
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
			{
				ret = offset;
			}
		}
		return ret;
	}
	
	bool  VSFile::Eof()
	{
		bool eof = false;
		if( !UseVolumes[alt_type] || this->volume_type==None || file_mode!=ReadOnly)
		{
			eof = vs_feof( this->fp);
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
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
		if( this->valid && this->file_mode==ReadOnly && (file_type==UnitFile || file_type==AnimFile || file_type==SpriteFile || file_type==CockpitFile))
		{
			assert( current_path.size()>1);
			current_path.pop_back();
			current_subdirectory.pop_back();
			current_type.pop_back();
		#ifdef VSFS_DEBUG
			cerr<<"END OF ";
			DisplayType( this->file_type);
			cerr<<endl<<endl;
		#endif
		}
		if( !UseVolumes[file_type] || this->volume_type==None || file_mode!=ReadOnly)
		{
			fclose( this->fp);
			this->fp = NULL;
		}
		else
		{
			if( volume_format=="vsr")
			{
			}
			else if( volume_format=="pk3")
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

	string	VSFile::GetFilename()
	{
		return this->filename;
	}
	string	VSFile::GetDirectory()
	{
		return this->directoryname;
	}
	string	VSFile::GetSubDirectory()
	{
		return this->subdirectoryname;
	}
	string	VSFile::GetRoot()
	{
		return this->rootname;
	}

	void	VSFile::SetFilename( string file)
	{
		this->filename = file;
	}
	void	VSFile::SetDirectory( string directory)
	{
		this->directoryname = directory;
	}
	void	VSFile::SetSubDirectory( string subdirectory)
	{
		this->subdirectoryname = subdirectory;
	}
	void	VSFile::SetRoot( string root)
	{
		this->rootname = root;
	}

	string	VSFile::GetFullPath()
	{
		return (this->rootname+"/"+this->directoryname+"/"+this->subdirectoryname+"/"+this->filename);
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
	{ return (UseVolumes[alt_type] && volume_type!=None); }

	void	VSFile::GoAfterEOL( int length)
	{
		while( this->offset<length && this->offset<this->size && (this->pk3_extracted_file[offset]=='\r' || this->pk3_extracted_file[offset]=='\n'))
		{
			if( pk3_extracted_file[offset]=='\n')
				cerr<<"\\n ";
			if( pk3_extracted_file[offset]=='\r')
				cerr<<"\\r ";
			this->offset++;
		}
	}
	void	VSFile::GoAfterEOL()
	{
		while( this->offset<this->size && (this->pk3_extracted_file[offset]=='\r' || this->pk3_extracted_file[offset]=='\n'))
		{
			if( pk3_extracted_file[offset]=='\n')
				cerr<<"\\n ";
			if( pk3_extracted_file[offset]=='\r')
				cerr<<"\\r ";
			this->offset++;
		}
	}
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

