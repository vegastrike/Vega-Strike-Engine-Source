#ifndef __VSFILESYS_H
#define __VSFILESYS_H

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
using std::string;
using std::vector;
#include <stdarg.h>
#include "gfx/vec.h"
#include "networking/const.h"
#include "pk3.h"
#include "map"
using std::map;

class VegaConfig;
class VSImage;

extern char *		CONFIGFILE;
extern VegaConfig * createVegaConfig( char * file);
extern ObjSerial	serial_seed;
ObjSerial			getUniqueSerial();
#define DELIM '/'
#define DELIMSTR "/"

namespace VSFileSystem
{
	class VSFile;

	/************************************************************************************************/
	/**** VSFileSystem enums                                                                     ****/
	/************************************************************************************************/

	// VSErrors that can be return by various functions
	enum VSError	{	Shared, Ok, SocketError, FileNotFound, LocalPermissionDenied, RemotePermissionDenied, DownloadInterrupted,
						IncompleteWrite, IncompleteRead, EndOfFile, IsDirectory, BadFormat, Unspecified };

	// The mode of an opened file
	enum VSFileMode {	ReadOnly, ReadWrite, CreateWrite };

	// Different file type we have to deal with
	enum VSFileType { 	UniverseFile,	// Loaded from universe subdir
						SystemFile,		// Loaded from sectors subdir written in homedir/sectors/universename
						CockpitFile,	// Loaded from cockpits subdir
						UnitFile,		// Loaded from units subdir
						TextureFile,	// Loaded from current path or from textures subdir
						SoundFile,		// Loaded from current path or from sounds subdir
						PythonFile,		// Loaded from bases subdir // not really used
						MeshFile,		// Loaded from current path or from meshes subdir
						CommFile,		// Loaded from communications subdir
						AiFile,			// Loaded from ai subdir
						SaveFile,		// Loaded from homedir/save and saved in homedir/save
						AnimFile,		// Loaded from animations subdir
						SpriteFile,		// Loaded from current path or from sprites subdir
						MissionFile,	// Loaded from mission subdir
						BSPFile,		// Loaded from homedir/generatedbsp and written in homedir/generatedbsp
						MusicFile,		// Loaded from homedir
						AccountFile,	// Loaded from datadir/accounts and written in datadir/accounts
						Unknown			// Loaded from homedir or datadir and written in homedir
					};

	enum VSVolumeType { None, Split, Big };

	void	      DisplayType( VSFileType type);
	void	      DisplayType( VSFileType type, std::ostream& ostr );

	int		GetReadBytes( char * fmt, va_list ap);

	/************************************************************************************************/
	/**** VSFileSystem global variables                                                          ****/
	/************************************************************************************************/

	extern	bool	use_volumes;
	extern	string	volume_format;

	extern vector <std::string> curdir;//current dir starting from datadir
	extern vector <std::string> savedpwd;
	extern string sharedtextures;
	extern string sharedsounds;
	extern string sharedmeshes;
	extern string sharedunits;
	extern string sharedsectors;
	extern string sharedcockpits;
	extern string shareduniverse;
	extern string sharedanims;
	extern string sharedsprites;
	extern string savedunitpath;
	extern string moddir;
	extern string datadir;
	extern string homedir;									// User home directory including .vegastrike subdir

	extern string config_file;
	extern string weapon_list;
	extern string universe_name;
	extern string HOMESUBDIR;
	extern vector<string>		current_path;
	extern vector<string>		current_directory;
	extern vector<string>		current_subdirectory;
	extern vector<VSFileType>	current_type;

	extern vector< vector <string> >	SubDirectories;		// Subdirectories where we should look for VSFileTypes files
	extern vector<string>				Directories;
	extern vector<string>				Rootdir;			// Root directories where we should look for VSFileTypes files

	extern vector<int>					UseVolumes;			// Tells us for which VSFileType we will use volumes
															// 0 tells FileType doesn't use volumes
															// 1 tells it uses a volume based on the FileType
															// 2 tells it uses a big data volume

	extern string failed;

	extern map<string, CPK3 *>	pk3_opened_files; 			// Map of the currently opened PK3 volume/resource files

	/************************************************************************************************/
	/**** VSFileSystem wrappers to stdio calls                                                   ****/
	/************************************************************************************************/

	FILE *	vs_open( const char * filename, const char * mode);
	size_t	vs_read( void *ptr, size_t size, size_t nmemb, FILE * fp);
	size_t	vs_write( const void *ptr, size_t size, size_t nmemb, FILE * fp);
	int		vs_fseek( FILE * fp, long offset, int whence);
	long	vs_ftell( FILE * fp);
	void	vs_rewind( FILE * fp);
	void	vs_close( FILE * fp);
	bool	vs_feof( FILE * fp);
	long	vs_getsize( FILE * fp);

	int		vs_fprintf( FILE * fp, const char * format, ...);
#if 0
	int		vs_fscanf( FILE * fp, const char * format, ...);
#endif
	template<class A>
	int	vs_fscanf( FILE * fp, const char * format, A * a) { return fscanf( fp, format, a); }
	template<class A, class B>
	int	vs_fscanf( FILE * fp, const char * format, A * a, B * b) { return fscanf( fp, format, a, b); }
	template<class A, class B, class C>
	int	vs_fscanf( FILE * fp, const char * format, A * a, B * b, C * c) { return fscanf( fp, format, a, b, c); }
	template< class A, class B, class C, class D>
	int	vs_fscanf( FILE * fp, const char * format, A * a, B * b, C * c, D * d) { return fscanf( fp, format, a, b, c, d); }
	template< class A, class B, class C, class D, class E>
	int	vs_fscanf( FILE * fp, const char * format, A * a, B * b, C * c, D * d, E * e) { return fscanf( fp, format, a, b, c, d, e); }
	template< class A, class B, class C, class D, class E, class F>
	int	vs_fscanf( FILE * fp, const char * format, A * a, B * b, C * c, D * d, E * e, F * f) { return fscanf( fp, format, a, b, c, d, e, f); }

	/*
	template< class A>
	int	vs_sscanf( FILE * fp, const char * format, A * a)
	{ return sscanf( buf, newformat, a); }
	template< class A, class B>
	int	vs_sscanf( FILE * fp, const char * format, A * a, B * b)
	{ return sscanf( buf, newformat, a, b); }
	template< class A, class B, class C>
	int	vs_sscanf( FILE * fp, const char * format, A * a, B * b, C * c)
	{ return sscanf( buf, newformat, a, b, c); }
	template< class A, class B, class C, class D>
	int	vs_sscanf( FILE * fp, const char * format, A * a, B * b, C * c, D * d)
	{ return sscanf( buf, newformat, a, b, d); }
	template< class A, class B, class C, class D, class E>
	int	vs_sscanf( const char * buf, const char * format, A * a, B * b, C * c, D * d, E * e)
	{ return sscanf( buf, newformat, a, b, c, d, e); }
	template< class A, class B, class C, class D, class E, class F>
	int	vs_sscanf( const char * buf, const char * format, A * a, B * b, C * c, D * d, E * e, F * f)
	{ return sscanf( buf, newformat, a, b, c, d, e, f); }
	// We need 1 arg more than vs_fscanf because we pass an extra arg to get the number of bytes read by sscanf in the last arg
	template< class A, class B, class C, class D, class E, class F, class G>
	int	vs_sscanf( const char * buf, const char * format, A * a, B * b, C * c, D * d, E * e, F * f, G * g)
	{ return sscanf( buf, newformat, a, b, c, d, e, f); }
	*/

	/************************************************************************************************/
	/**** vs_path functions : mostly obsolete stuff now                                          ****/
	/************************************************************************************************/

	string GetHashName (const std::string &name);
	string GetHashName (const std::string &name,const class Vector & scale, int faction);
	string GetSharedTextureHashName(const std::string &);
	string GetSharedSoundHashName(const std::string&);
	string GetSharedMeshHashName(const std::string &, const class Vector &scale, int faction);
	string MakeSharedStarSysPath (const std::string &);
	string MakeSharedPath (const std::string &path);
	string GetCorrectStarSysPath (const std::string &,bool &autogenerated);

	//string GetSharedUnitPath ();
	//string GetSharedMeshPath (const std::string &name);
	//string GetSharedSoundPath (const std::string &name);
	//string GetSharedTexturePath (const std::string &name);

	/*
	void initpaths(const std::string& modname="");			// Sets up datadir and load VS config file
	void changehome(bool makehomedir=false);				// Changes to user homedir in an OS independant way
	void returnfromhome();									// Return from homedir to calling directory
	void vs_chdir (const char *path);						// Changes the directory and the current path to reflect it
															// Note : may only change ONE level of directory or it breaks
	void vs_cdup();											// Goes up one level and changes the direcrtory accordingly
	void vs_mkdir (const std::string &nam);					// Creates a directory
	void vs_setdir (const char * path);						// Sets the current directory
	void vs_resetdir ();									// Reset the current directory
	*/


	/************************************************************************************************/
	/**** VSFileSystem functions                                                                 ****/
	/************************************************************************************************/

	// Initialize paths
	void	InitPaths( string conf, string subdir="");
	void	InitDataDirectory();
	void	InitHomeDirectory();
	void	LoadConfig( string subdir="");
	void	InitMods();

	// Create a directory
	void	CreateDirectoryAbs( const char * filename);
	void	CreateDirectoryAbs( string filename);
	// Create a directory in home_path
	void	CreateDirectoryHome( const char * filename);
	void	CreateDirectoryHome( string filename);
	// Create a directory in data_path_path
	void	CreateDirectoryData( const char * filename);
	void	CreateDirectoryData( string filename);

	/********** DO NO USE FileExists functions directly : USE LookForFile instead **********/
	// Test if a directory exists (absolute path)
	bool	DirectoryExists( const char * filename);
	bool	DirectoryExists( string filename);
	// Returns positive int or index in archive if found or -1 if not found
	// Test if a file exists (absolute path)
	int		FileExists( string root, const char * filename, VSFileType type=Unknown, bool lookinvolume=true);
	int		FileExists( string root, string filename, VSFileType type=Unknown, bool lookinvolume=true);
	// Test if a file exists relative to home_path
	int		FileExistsHome( const char * filename, VSFileType type=Unknown);
	int		FileExistsHome( string filename, VSFileType type=Unknown);
	// Test if a file exists relative to data_path
	int		FileExistsData( const char * filename, VSFileType type=Unknown);
	int		FileExistsData( string filename, VSFileType type=Unknown);

	VSError	GetError(char * str=NULL);

	VSError	LookForFile( VSFile & f, VSFileType type, VSFileMode mode=ReadOnly);
	VSError	LookForFile( string & filename, VSFileType type, VSFileMode mode=ReadOnly);

	/************************************************************************************************/
	/**** VSFileSystem::VSFile functions                                                         ****/
	/************************************************************************************************/

	class VSFile
	{
		private:
			// STDIO stuff
			FILE *			fp;

			// PK3 stuff
			CPK3 *			pk3_file;
			char *			pk3_extracted_file;
			int				file_index;
			int				offset;

			void			checkExtracted();

			// VSFile internals
			VSFileType		file_type;
			VSFileType		alt_type;
			VSFileMode		file_mode;
			VSVolumeType	volume_type;

			string			filename;
			string			subdirectoryname;
			string			directoryname;
			string			rootname;

			long			size;
			bool			valid;

		public:
			VSFile();
			VSFile( const char * filename, VSFileType type=Unknown, VSFileMode=ReadOnly);
			VSFile( string filename, VSFileType type=Unknown) { VSFile::VSFile( filename.c_str(), type); }
  			~VSFile();

			FILE *	GetFP() { return this->fp; } // This is still needed for special cases (when loading PNG files)

			/********************************** OPEN A FILE *********************************/
			// Open an existing file read only
			VSError	OpenReadOnly( const char * filename, VSFileType type=Unknown);
			VSError	OpenReadOnly( string filename, VSFileType type=Unknown) { return OpenReadOnly( filename.c_str(), type); }
			// Open an existing file read/write
			VSError	OpenReadWrite( const char * filename, VSFileType type=Unknown);
			VSError	OpenReadWrite( string filename, VSFileType type=Unknown) { return OpenReadWrite( filename.c_str(), type); }
			// Open (truncate) or create a file read/write
			VSError	OpenCreateWrite( const char * filename, VSFileType type=Unknown);
			VSError	OpenCreateWrite( string filename, VSFileType type=Unknown) { return OpenCreateWrite( filename.c_str(), type); }
			// Close the file
			void	Close();

			/********************************** READ/WRITE OPERATIONS IN A FILE *********************************/
			size_t	Read( void * ptr, size_t length);			// Read length in ptr (store read bytes number in length)
			VSError	ReadLine( void * ptr, size_t length);		// Read a line of maximum length
			string	ReadFull();									// Read the entire file and returns the content in a string
			size_t	Write( const void * ptr, size_t length);	// Write length from ptr (store written bytes number in length)
			size_t	Write( string content);						// Write a string
			VSError	WriteLine( const void * ptr);				// Write a line
			void	WriteFull( void * ptr);						// Write

#if 0
			int		Fscanf( const char * format, ...);
#endif
			void	GoAfterEOL( int length);
			void	GoAfterEOL();

			template<class A>
			int	Fscanf( const char * format, A * a)
			{
				int ret = -1;
				if( !UseVolumes[this->alt_type])
					ret = fscanf( fp, format, a);
				else
				{
					if( volume_format=="pk3")
					{
						checkExtracted();
						int ret = -1, readbytes=0;
						int length = strlen( format);
						int newlength = length+3;
						char * newformat = new char[newlength];
						memset( newformat, 0, newlength);
						memcpy( newformat, format, length);
						strcat( newformat, "%n");
						ret = sscanf( this->pk3_extracted_file+offset, newformat, a, &readbytes);
						delete []newformat;
						cerr<<"FSCANF : sscanf read "<<readbytes<<" bytes - OFFSET="<<offset<<" VALUES : a="<<(*a)<<endl;
						this->offset += readbytes;
						this->GoAfterEOL();
					}
					else if( volume_format=="vsr")
					{
					}
				}
				return ret;
			}
			template<class A, class B>
			int	Fscanf( const char * format, A * a, B * b)
			{
				int ret = -1;
				if( !UseVolumes[this->alt_type])
					ret = fscanf( fp, format, a, b);
				else
				{
					if( volume_format=="pk3")
					{
						checkExtracted();
						int ret = -1, readbytes=0;
						int length = strlen( format);
						int newlength = length+3;
						char * newformat = new char[newlength];
						memset( newformat, 0, newlength);
						memcpy( newformat, format, length);
						strcat( newformat, "%n");
						ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, &readbytes);
						delete []newformat;
						cerr<<"FSCANF : sscanf read "<<readbytes<<" bytes - OFFSET="<<offset<<" VALUES : a="<<(*a)<<", b="<<(*b)<<endl;
						this->offset += readbytes;
						this->GoAfterEOL();
					}
					else if( volume_format=="vsr")
					{
					}
				}
				return ret;
			}
			template<class A, class B, class C>
			int	Fscanf( const char * format, A * a, B * b, C * c)
			{
				int ret = -1;
				if( !UseVolumes[this->alt_type])
					ret = fscanf( fp, format, a, b, c);
				else
				{
					if( volume_format=="pk3")
					{
						checkExtracted();
						int ret = -1, readbytes=0;
						int length = strlen( format);
						int newlength = length+3;
						char * newformat = new char[newlength];
						memset( newformat, 0, newlength);
						memcpy( newformat, format, length);
						strcat( newformat, "%n");
						ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, &readbytes);
						delete []newformat;
						cerr<<"FSCANF : sscanf read "<<readbytes<<" bytes - OFFSET="<<offset<<" VALUES : a="<<(*a)<<", b="<<(*b)<<", c="<<(*c)<<endl;
						this->offset += readbytes;
						this->GoAfterEOL();
					}
					else if( volume_format=="vsr")
					{
					}
				}
				return ret;
			}
			template< class A, class B, class C, class D>
			int	Fscanf( const char * format, A * a, B * b, C * c, D * d)
			{
				int ret = -1;
				if( !UseVolumes[this->alt_type])
					ret = fscanf( fp, format, a, b, c, d);
				else
				{
					if( volume_format=="pk3")
					{
						checkExtracted();
						int ret = -1, readbytes=0;
						int length = strlen( format);
						int newlength = length+3;
						char * newformat = new char[newlength];
						memset( newformat, 0, newlength);
						memcpy( newformat, format, length);
						strcat( newformat, "%n");
						ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, d, &readbytes);
						delete []newformat;
						this->offset += readbytes;
						this->GoAfterEOL();
					}
					else if( volume_format=="vsr")
					{
					}
				}
				return ret;
			}
			template< class A, class B, class C, class D, class E>
			int	Fscanf( const char * format, A * a, B * b, C * c, D * d, E * e)
			{
				int ret = -1;
				if( !UseVolumes[this->alt_type])
					ret = fscanf( fp, format, a, b, c, d, e);
				else
				{
					if( volume_format=="pk3")
					{
						checkExtracted();
						int ret = -1, readbytes=0;
						int length = strlen( format);
						int newlength = length+3;
						char * newformat = new char[newlength];
						memset( newformat, 0, newlength);
						memcpy( newformat, format, length);
						strcat( newformat, "%n");
						ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, d, e, &readbytes);
						delete []newformat;
						this->offset += readbytes;
						this->GoAfterEOL();
					}
					else if( volume_format=="vsr")
					{
					}
				}
				return ret;
			}
			template< class A, class B, class C, class D, class E, class F>
			int	Fscanf( const char * format, A * a, B * b, C * c, D * d, E * e, F * f)
			{
				int ret = -1;
				if( !UseVolumes[this->alt_type])
					ret = fscanf( fp, format, a, b, c, d, e, f);
				else
				{
					if( volume_format=="pk3")
					{
						checkExtracted();
						int ret = -1, readbytes=0;
						int length = strlen( format);
						int newlength = length+3;
						char * newformat = new char[newlength];
						memset( newformat, 0, newlength);
						memcpy( newformat, format, length);
						strcat( newformat, "%n");
						ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, d, e, f, &readbytes);
						delete []newformat;
						this->offset += readbytes;
						this->GoAfterEOL();
					}
					else if( volume_format=="vsr")
					{
					}
				}
				return ret;
			}

			int		Fprintf( const char * format, ...);

			/********************************** FILE POINTER POSITION OPERATIONS IN A FILE *********************************/
			long	Size();					// Returns the size of the file : current position in file is not changed
			void	Begin();				// Set the file pointer to the beginning of the file
			long	GetPosition();			// Get the current file pointer position
			void	End();					// Set the file pointer to the end of the file
			void	GoTo( long offset);		// Does a fseek( fp, offset, SEEK_SET);
			void	Clear();				// Empty the file and then set file pointer to the beginning
			bool	Eof();					// Return true if end of file
			bool	Valid();				// Tells wether the file is valid or not

			/********************************** FILE PATH *********************************/
			string	GetFilename();
			string	GetDirectory();
			string	GetSubDirectory();
			string	GetRoot();

			void	SetFilename( string filename);
			void	SetDirectory( string directory);
			void	SetSubDirectory( string subdirectory);
			void	SetRoot( string root);

			string	GetFullPath();

			void	SetType( VSFileType type);
			void	SetAltType( VSFileType type);

			void	SetIndex( int index);
			void	SetVolume( VSVolumeType big);
			bool	UseVolume();

			friend class VSImage;
	};
};

std::ostream& operator<<( std::ostream& ostr, VSFileSystem::VSError err );
std::ostream& operator<<( std::ostream& ostr, VSFileSystem::VSFileType type );
std::string   nameof( VSFileSystem::VSFileType type );

#if defined( _WIN32) && !defined( __CYGWIN__)
// Emulation of posix scandir() call
int scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *),
    int (*compar)(const struct dirent **, const struct dirent **));
#endif

#endif

