#include <errno.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>				// definitions of getcwd() and chdir() on win32
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "vs_globals.h"
#include "filesys.h"
#include "configxml.h"

extern char * GetUnitDir( const char * filename);

namespace VSFileSystem
{
	/**** Bool that tells us if we are using volume files ****/
	bool		use_volumes;
	string	volume_format;

	/**** Paths ****/
	// Config file name
	string	config_file;
	// Path to data directory
	string	data_path;
	// Path to home directory
	string	home_path;
	string	home_subdir;

	vector< vector<string> >	Directories;
	vector<string>				Volumes;

	vector<string>	data_paths;

	void	InitHomeDirectories()
	{
		// Setup home directory
		home_subdir = ".vegastrike";
		char * chome_path = NULL;
#ifndef _WIN32
		struct passwd *pwent;
		pwent = getpwuid (getuid());
		chome_path = pwent->pw_dir;
#else
		char chome_path_buf[1024];
		chome_path = chome_path_buf;
		memset( chome_path, 0, 1024);
		getcwd( chome_path, 1023);
#endif
		if( !FileExists( chome_path))
		{
			cerr<<"!!! ERROR : home directory not found"<<endl;
			VSExit(1);
		}
		string user_home_path( chome_path);
		home_path = user_home_path+"/"+home_subdir;
		// If vs homedir doesn't exist we create it
		if( !FileExists( home_path))
			CreateDirectory( home_path);
		// Check if we have a generatedbsp, save,  directory
		if( !FileExists( home_path+"/generatedbsp"))
			CreateDirectory( user_home_path+"/generatedbsp");
		if( !FileExists( home_path+"/save"))
			CreateDirectory( user_home_path+"/save");
	}

	void	InitDataDirectory()
	{
		// First check if we have a config file in home_path
		if( FileExists( home_path+"/"+config_file))
		{
			cerr<<"Found a config file in home directory : using it"<<endl;
			char * conffile = new char[home_path.length()+1+config_file.length()+1];
			conffile[home_path.length()+1+config_file.length()] = 0;
			memcpy( conffile, (home_path+"/"+config_file).c_str(), home_path.length()+1+config_file.length());
			vs_config = createVegaConfig( conffile);
			delete conffile;
			// Now check if there is a data directory specified in it
			string datadir( vs_config->getVariable( "data", "datadir", ""));
			if( datadir != "")
			{
				// We found a path to data in config file
				home_path = datadir;
				return;
			}
			else
				cerr<<"No data directory specified in config file : "<<home_path+"/"+config_file<<endl;
		}

#ifdef DATA_DIR
		data_paths.push_back( DATA_DIR);
#endif
		data_paths.push_back( ".");
		data_paths.push_back( "..");
		data_paths.push_back( "../data");
		data_paths.push_back( "../../data");
		data_paths.push_back( "../Resources/data");
		data_paths.push_back( "/usr/share/local/vegastrike/data");
		data_paths.push_back( "/usr/share/vegastrike/data");
		data_paths.push_back( "/opt/share/vegastrike/data");
		data_paths.push_back( "../data4.x");
		data_paths.push_back( "../../data4.x");
		data_paths.push_back( "../Resources/data4.x");
		data_paths.push_back( "/usr/share/local/vegastrike/data4.x");
		data_paths.push_back( "/usr/share/vegastrike/data4.x");
		data_paths.push_back( "/opt/share/vegastrike/data4.x");

		// Win32 data should be "."
		for( vector<string>::iterator vsit=data_paths.begin(); vsit!=data_paths.end(); vsit++)
		{
			// Test if the dir exist and contains config_file
			if( FileExists( (*vsit)+"/"+config_file))
			{
				cerr<<"Found config file in "<<(*vsit)<<endl;
				data_path = (*vsit);
				break;
			}
		}
		if( vs_config==NULL)
		{
			// We didn't find a config file in home_path so we load the data_path one
			char * conffile = new char[data_path.length()+1+config_file.length()+1];
			conffile[data_path.length()+1+config_file.length()] = 0;
			memcpy( conffile, (data_path+"/"+config_file).c_str(), data_path.length()+1+config_file.length());
			vs_config = createVegaConfig( conffile);
			delete conffile;
		}
		data_paths.clear();
	}

	void	InitPaths( string conf)
	{
		config_file = conf;

		InitHomeDirectories();
		InitDataDirectory();

		// Determine if we will use volume files
		string vol_format( vs_config->getVariable( "data", "use_volumes", "pk3"));
		if( vol_format=="")
			use_volumes = false;
		else
			use_volumes = true;

		// WARNING HERE THINGS MUST BE READ IN THE SAME ORDER AS THE FileType ENUM !!!

		// Config read directories
		string unitdir = vs_config->getVariable( "data", "unitdir", "units");
		string facdir = vs_config->getVariable( "data", "unitfactiondir", "factions");
		string mountdir = vs_config->getVariable( "data", "mountlocation", "mounts");
		string meshdir = vs_config->getVariable( "data", "shareedmeshes", "meshes");
		string texdir = vs_config->getVariable( "data", "sharedtextures", "textures");

		// Universe search paths
		vector<string>	universe_paths;
		universe_paths.push_back( vs_config->getVariable( "data", "universe_path", "universe"));
		Directories.push_back( universe_paths);
		// Sector/system search paths
		vector<string>	sector_paths;
		sector_paths.push_back( vs_config->getVariable( "data", "sectors", "sectors"));
		Directories.push_back( sector_paths);
		// Cockpit search paths
		vector<string>	cockpit_paths;
		cockpit_paths.push_back( vs_config->getVariable( "data", "cockpits", "cockpits"));
		Directories.push_back( cockpit_paths);
		// Unit search paths
		vector<string>	unit_paths;
		unit_paths.push_back( unitdir);
		unit_paths.push_back( unitdir+"/"+facdir+"/neutral");
		unit_paths.push_back( unitdir+"/subunits");
		unit_paths.push_back( unitdir+"/weapons");
		unit_paths.push_back( unitdir+"/"+facdir);
		Directories.push_back( unit_paths);
		// Faction search paths
		vector<string>	faction_paths;
		faction_paths.push_back( unit_paths.back()+"/"+facdir);
		faction_paths.push_back( facdir);
		Directories.push_back( faction_paths);
		// Mount search paths
		vector<string>	mount_paths;
		mount_paths.push_back( meshdir+"/"+mountdir);
		mount_paths.push_back( texdir+"/"+mountdir);
		Directories.push_back( mount_paths);
		// Texture search paths
		vector<string>	texture_paths;
		texture_paths.push_back( texdir);
		Directories.push_back( texture_paths);
		// Sound search paths
		vector<string>	sound_paths;
		sound_paths.push_back( vs_config->getVariable( "data", "sharedsounds", "sounds"));
		Directories.push_back( sound_paths);
		// Mesh search paths
		vector<string>	mesh_paths;
		mesh_paths.push_back( meshdir);
		Directories.push_back( mesh_paths);

		if( use_volumes)
		{
			Volumes.push_back( vs_config->getVariable( "data", "universe_volume", "universe."+volume_format));
			Volumes.push_back( vs_config->getVariable( "data", "sectors_volume", "sectors."+volume_format));
			Volumes.push_back( vs_config->getVariable( "data", "cockpits_volume", "cockpits."+volume_format));
			Volumes.push_back( vs_config->getVariable( "data", "unit_volume", "units."+volume_format));
			Volumes.push_back( vs_config->getVariable( "data", "faction_volume", "factions."+volume_format));
			Volumes.push_back( vs_config->getVariable( "data", "mount_volume", "mounts."+volume_format));
			Volumes.push_back( vs_config->getVariable( "data", "textures_volume", "textures."+volume_format));
			Volumes.push_back( vs_config->getVariable( "data", "sounds_volume", "sounds."+volume_format));
		}

		cout<<"Path initialization done :\n\tHome directory = "<<home_path<<endl<<"\tData directory = "<<data_path<<endl;
	}

	void	CreateDirectory( const char * filename)
	{
		int err = mkdir (filename
#if !defined( _WIN32) || defined( __CYGWIN__)
		   ,0xFFFFFFFF
#endif
		);
		if( err<0)
		{
			GetError();
			VSExit(1);
		}
	}

	void	CreateDirectory( string filename) { CreateDirectory( filename.c_str()); }

	bool	FileExists( const char * filename, FileType type)
	{
		FILE * fp = NULL;
		// We won't check in volumes if the file format is not known
		if( use_volumes && type!=Unknown)
		{
			// If we use volumes we check in those before
			// Return true if found else continue
		}
		fp = fopen( filename, "rb");
		if( !fp)
			return false;
		fclose( fp);
		return true;
	}
	bool	FileExists( string filename, FileType type) { return FileExists( filename.c_str(), type); }

	bool	FileExistsAbsolute( const char * filename, FileType type)
	{
		int total_length = strlen( filename) + data_path.length();
		char * expath = new char[total_length+1];
		expath[total_length] = 0;
		strcpy( expath, data_path.c_str());
		strcat( expath, filename);

		return FileExists( expath);
	}
	bool	FileExistsAbsolute( string filename, FileType type) { return FileExists( (data_path+"/"+filename).c_str(), type); }

	Error GetError()
	{
		cerr<<"!!! ERROR VSFile : ";
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
			cerr<<"Unspecified error (maybe to document in VSFile ?"<<endl;
			return Unspecified;
		}
	}

	VSFile::VSFile()
	{
		fp = NULL;
	}

	VSFile::VSFile( const char * filename, FileType type)
	{
		if( use_volumes)
			// We want to read in volumes
			this->OpenReadOnly( filename, type);
		else
			this->OpenReadWrite( filename, type);
	}

  	VSFile::~VSFile()
	{
		if( fp)
		{
			fclose(fp);
			this->fp = NULL;
		}
	}

	// Loop through the paths according to FileType
	FILE * VSFile::LookForFile( const char * filename, FileType type)
	{
		FILE * fp = NULL;
		for( vector<string>::iterator it=Directories[type].begin(); it!=Directories[type].end(); it++)
		{
			string full_path;
			if( type==Unit)
				full_path = data_path+"/"+(*it)+"/"+string( GetUnitDir(filename))+string( filename);
			else
				full_path = data_path+"/"+(*it)+"/"+string( filename);
			fp = fopen( full_path.c_str(), "rb");
			if( fp)
			{
				this->filepath = full_path;
				break;
			}
		}
		return fp;
	}

	// Open a read only file
	Error VSFile::OpenReadOnly( const char * filename, FileType type)
	{
		if( !use_volumes || type==Unknown)
		{
			this->fp = LookForFile( filename, type);
			if( !fp)
				return GetError();
		}
		else
		{
			// Open the volume
		}
		return Ok;
	}

	// Open a standard file read/write
	Error VSFile::OpenReadWrite( const char * filename, FileType type)
	{
		if( !use_volumes || (use_volumes && type==Unknown))
		{
			this->fp = LookForFile( filename, type);
			if( !fp)
				return GetError();
			return Ok;
		}
		else
		{
			cerr<<"Cannot open file in read/write mode in a volume !"<<endl;
			VSExit(1);
		}
		return Ok;
	}

	Error  VSFile::Read( void * ptr, unsigned int length)
	{
		if( !use_volumes)
		{
			unsigned int nbread = fread( ptr, 1, length, this->fp);
			if( nbread != length)
				return IncompleteRead;
		}
		else
		{
		}
		return Ok;
	}

	Error  VSFile::ReadLine( void * ptr, unsigned int length)
	{
		if( !use_volumes)
			fgets( (char *)ptr, length, this->fp);
		return Ok;
	}

	string  VSFile::ReadFull()
	{
		int size = this->Size();
		char * content = new char[size+1];
		content[size] = 0;
		int readsize = fread( content, 1, size, this->fp);
		if( size!=readsize)
		{
			GetError();
			VSExit(1);
		}
		return string( content);
	}

	Error  VSFile::Write( const void * ptr, unsigned int length)
	{
		if( !use_volumes)
		{
			unsigned int nbwritten = fwrite( ptr, 1, length, this->fp);
			if( nbwritten != length)
				return IncompleteWrite;
		}
		else
		{
			// We can't write in volumes yet
			return LocalPermissionDenied;
		}
		return Ok;
	}

	Error  VSFile::WriteLine( const void * ptr)
	{
		if( !use_volumes)
			fputs( (const char *)ptr, this->fp);
		return Ok;
	}

	void  VSFile::WriteFull( void * ptr)
	{
	}

	void  VSFile::Begin()
	{ fseek( this->fp, 0, SEEK_SET); }

	void  VSFile::End()
	{ fseek( this->fp, 0, SEEK_END); }

	long  VSFile::Size()
	{
		long cur_pos = ftell( this->fp);
		this->End();
		long size = ftell( this->fp);
		fseek( this->fp, cur_pos, SEEK_SET);
		return size;
	}

	void  VSFile::Clear()
	{
		fclose( fp);
		this->fp = fopen( this->filepath.c_str(), "w+b");
		// This should not happen
		if( !fp)
		{
			GetError();
			VSExit(1);
		}
	}

	void  VSFile::Close()
	{
		if( use_volumes)
		{
			// Close volume ??
		}
		else
		{
			fclose( this->fp);
			this->fp = NULL;
		}
	}
}

