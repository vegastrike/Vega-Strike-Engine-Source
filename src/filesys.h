#ifndef __FILESYS_H
#define __FILESYS_H

#include <string>
#include <vector>
using std::string;
using std::vector;

class VegaConfig;
extern VegaConfig * createVegaConfig( char * file);

/*
 * Maybe open all the volumes read only for the whole VS game
 * -> in InitPaths() ?
 */

// Sometimes include this in the vsnet_dloadenum.h file
enum Error
{
    Ok,
    SocketError,
    FileNotFound,
    LocalPermissionDenied,
    RemotePermissionDenied,
    DownloadInterrupted,
	IncompleteWrite,
	IncompleteRead,
	Unspecified
};

namespace	VSFileSystem
{

	enum FileType
	{
		Universe,
		System,
		Cockpit,
		Unit,
		Faction,
		Mount,
		Texture,
		Sound,
		Mesh,
		Unknown
	};

	/**** Bool that tells us if we are using volume files ****/
	extern bool		use_volumes;
	extern string	volume_format;

	/**** Paths ****/
	// Config file name
	extern 	string	config_file;
	// Path to data directory
	extern 	string	data_path;
	// Path to home directory
	extern  string	home_path;
	extern  string	home_subdir;

	extern vector< vector<string> >	Directories;
	extern vector<string>	Volumes;

	// Initialize paths
	void	InitPaths( string conf);
	// Create a directory
	void	CreateDirectory( const char * filename);
	void	CreateDirectory( string filename);
	// Test if a file exists (first 2 are testing relative paths and the last 2 are testing absolute paths)
	bool	FileExists( const char * filename, FileType type=Unknown);
	bool	FileExists( string filename, FileType type=Unknown);
	bool	FileExistsAbsolute( const char * filename, FileType type=Unknown);
	bool	FileExistsAbsolute( string filename, FileType type=Unknown);
	// Look for a file in know directories, maybe add a filetype parameter to narrow the search
	//string	FindFile( const char * filename, FileType type=Unknown);
	//string	FindFile( string filename, FileType type=Unknown) { return FindFile( filename.c_str(), type); }

	Error	GetError();

	class VSFile
	{
		private:
			string			filepath;
			FileType		file_type;
			string			volume_name;
			FILE *			fp;

			FILE * 			LookForFile( const char * filename, FileType type);

		public:
			VSFile();
			VSFile( const char * filename, FileType type=Unknown);
			VSFile( string filename, FileType type=Unknown) { VSFile::VSFile( filename.c_str(), type); }
  			~VSFile();

			Error	OpenReadOnly( const char * filename, FileType type=Unknown);
			Error	OpenReadOnly( string filename, FileType type=Unknown) { return OpenReadOnly( filename.c_str(), type); }
			Error	OpenReadWrite( const char * filename, FileType type=Unknown);
			Error	OpenReadWrite( string filename, FileType type=Unknown) { return OpenReadWrite( filename.c_str(), type); }
			Error	Read( void * ptr, unsigned int length);
			Error	ReadLine( void * ptr, unsigned int length);
			string	ReadFull();
			Error	Write( const void * ptr, unsigned int length);
			Error	WriteLine( const void * ptr);
			void	WriteFull( void * ptr);
			long	Size();			// Returns the size of the file : current position in file is not changed
			void	Begin();		// Set the file pointer to the beginning of the file
			void	End();			// Set the file pointer to the end of the file
			void	Clear();		// Empty the file and then set file pointer to the beginning
			void	Close();		// Close the file
	};
};

#endif

