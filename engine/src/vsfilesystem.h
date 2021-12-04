/**
 * vsfilesystem.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020-2021 pyramid3d, Nachum Barcohen, Roy Falk,
 * Stephen G. Tuggy, and other Vega Strike contributors
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
#include "pk3.h"
#include <gnuhash.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

#include <boost/filesystem.hpp>

class VegaConfig;
class VSImage;

extern char     *CONFIGFILE;
extern VegaConfig * createVegaConfig( const char *file );

#define DELIM '/'
#define DELIMSTR "/"
namespace VSFileSystem
{
class VSFile;

/*
 ***********************************************************************************************
 **** VSFileSystem enums                                                                     ***
 ***********************************************************************************************
 */

//VSErrors that can be return by various functions
enum VSError
{
    Shared, Ok, SocketError, FileNotFound, LocalPermissionDenied, RemotePermissionDenied, DownloadInterrupted,
    IncompleteWrite, IncompleteRead, EndOfFile, IsDirectory, BadFormat, Unspecified
};                                                                                                                                //Failure condition when result > Ok

//The mode of an opened file
enum VSFileMode {ReadOnly, ReadWrite, CreateWrite};

//Different file type we have to deal with
enum VSFileType
{
    //File types
    UniverseFile,                                               //Loaded from universe subdir
    SystemFile,                                                         //Loaded from sectors subdir written in homedir/sectors/universename
    CockpitFile,                                                //Loaded from cockpits subdir
    UnitFile,                                                           //Loaded from units subdir
    UnitSaveFile,                                               //Saved unit file
    TextureFile,                                                //Loaded from current path or from textures subdir
    SoundFile,                                                          //Loaded from current path or from sounds subdir
    PythonFile,                                                         //Loaded from bases subdir // not really used
    MeshFile,                                                           //Loaded from current path or from meshes subdir
    CommFile,                                                           //Loaded from communications subdir
    AiFile,                                                             //Loaded from ai subdir
    SaveFile,                                                           //Loaded from homedir/save and saved in homedir/save
    AnimFile,                                                           //Loaded from animations subdir
    VideoFile,                                  //Loaded from movies subdir
    VSSpriteFile,                                                       //Loaded from current path or from sprites subdir
    MissionFile,                                                //Loaded from mission subdir
    MusicFile,                                                          //Loaded from homedir
    AccountFile,                                                //Loaded from datadir/accounts and written in datadir/accounts
    //Buffer types
    ZoneBuffer,                                                         //Indicates a ZoneInfo buffer coming from server
    JPEGBuffer,                                                         //Indicates a JPEG buffer coming from network
    //Unknown
    UnknownFile                                                         //Loaded from homedir or datadir and written in homedir
};

enum VSVolumeType {VSFSNone, VSFSSplit, VSFSBig};
enum VSVolumeFormat {vfmtUNK, vfmtVSR, vfmtPK3};

void DisplayType( VSFileType type );
void DisplayType( VSFileType type, std::ostream &ostr );

int GetReadBytes( char *fmt, va_list ap );

typedef vsUMap< string, VSError > FileLookupCache;
VSError CachedFileLookup( FileLookupCache &cache, const string &file, VSFileType type );

/*
 ***********************************************************************************************
 **** VSFileSystem global variables                                                          ***
 ***********************************************************************************************
 */

extern bool   use_volumes;
extern string volume_format;
extern enum VSVolumeFormat  q_volume_format;

extern vector< std::string >curdir;        //current dir starting from datadir
extern vector< std::string >savedpwd;
extern vector< string >Rootdir;
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
extern string homedir;                                                                          //User home directory including .vegastrike subdir

extern string config_file;
extern string weapon_list;
extern string universe_name;
extern string HOMESUBDIR;
extern vector< string >    current_path;
extern vector< string >    current_directory;
extern vector< string >    current_subdirectory;
extern vector< VSFileType >current_type;

extern vector< vector< string > >SubDirectories;                        //Subdirectories where we should look for VSFileTypes files
extern vector< string >    Directories;
extern vector< string >    Rootdir;                                                     //Root directories where we should look for VSFileTypes files

extern vector< int >UseVolumes;                                                                 //Tells us for which VSFileType we will use volumes
//0 tells FileType doesn't use volumes
//1 tells it uses a volume based on the FileType
//2 tells it uses a big data volume

extern string failed;
extern vsUMap< string, CPK3* >pk3_opened_files;                                 //Map of the currently opened PK3 volume/resource files

typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> console_log_sink;
typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>    file_log_sink;
extern boost::shared_ptr<console_log_sink> pConsoleLogSink;
extern boost::shared_ptr<file_log_sink> pFileLogSink;

/*
 ***********************************************************************************************
 **** VSFileSystem wrappers to stdio calls                                                   ***
 ***********************************************************************************************
 */

FILE * vs_open( const char *filename, const char *mode );
size_t vs_read( void *ptr, size_t size, size_t nmemb, FILE *fp );
size_t vs_write( const void *ptr, size_t size, size_t nmemb, FILE *fp );
int vs_fseek( FILE *fp, long offset, int whence );
long vs_ftell( FILE *fp );
void vs_rewind( FILE *fp );
void vs_close( FILE *fp );
bool vs_feof( FILE *fp );
long vs_getsize( FILE *fp );

int vs_fprintf( FILE *fp, const char *format, ... );

template < class A >
int vs_fscanf( FILE *fp, const char *format, A *a )
{
    return fscanf( fp, format, a );
}
template < class A, class B >
int vs_fscanf( FILE *fp, const char *format, A *a, B *b )
{
    return fscanf( fp, format, a, b );
}
template < class A, class B, class C >
int vs_fscanf( FILE *fp, const char *format, A *a, B *b, C *c )
{
    return fscanf( fp, format, a, b, c );
}
template < class A, class B, class C, class D >
int vs_fscanf( FILE *fp, const char *format, A *a, B *b, C *c, D *d )
{
    return fscanf( fp, format, a, b, c, d );
}
template < class A, class B, class C, class D, class E >
int vs_fscanf( FILE *fp, const char *format, A *a, B *b, C *c, D *d, E *e )
{
    return fscanf( fp, format, a, b, c, d, e );
}
template < class A, class B, class C, class D, class E, class F >
int vs_fscanf( FILE *fp, const char *format, A *a, B *b, C *c, D *d, E *e, F *f )
{
    return fscanf( fp, format, a, b, c, d, e, f );
}

/*
 *  template< class A>
 *  int	vs_sscanf( FILE * fp, const char * format, A * a)
 *  { return sscanf( buf, newformat, a); }
 *  template< class A, class B>
 *  int	vs_sscanf( FILE * fp, const char * format, A * a, B * b)
 *  { return sscanf( buf, newformat, a, b); }
 *  template< class A, class B, class C>
 *  int	vs_sscanf( FILE * fp, const char * format, A * a, B * b, C * c)
 *  { return sscanf( buf, newformat, a, b, c); }
 *  template< class A, class B, class C, class D>
 *  int	vs_sscanf( FILE * fp, const char * format, A * a, B * b, C * c, D * d)
 *  { return sscanf( buf, newformat, a, b, d); }
 *  template< class A, class B, class C, class D, class E>
 *  int	vs_sscanf( const char * buf, const char * format, A * a, B * b, C * c, D * d, E * e)
 *  { return sscanf( buf, newformat, a, b, c, d, e); }
 *  template< class A, class B, class C, class D, class E, class F>
 *  int	vs_sscanf( const char * buf, const char * format, A * a, B * b, C * c, D * d, E * e, F * f)
 *  { return sscanf( buf, newformat, a, b, c, d, e, f); }
 *  // We need 1 arg more than vs_fscanf because we pass an extra arg to get the number of bytes read by sscanf in the last arg
 *  template< class A, class B, class C, class D, class E, class F, class G>
 *  int	vs_sscanf( const char * buf, const char * format, A * a, B * b, C * c, D * d, E * e, F * f, G * g)
 *  { return sscanf( buf, newformat, a, b, c, d, e, f); }
 */

/*
 ***********************************************************************************************
 **** vs_path functions : mostly obsolete stuff now                                          ***
 ***********************************************************************************************
 */

string GetHashName( const std::string &name );
string GetHashName( const std::string &name, const Vector &scale, int faction );
string GetSharedTextureHashName( const std::string& );
string GetSharedSoundHashName( const std::string& );
string GetSharedMeshHashName( const std::string&, const Vector &scale, int faction );
string MakeSharedStarSysPath( const std::string& );
string MakeSharedPath( const std::string &path );
string GetCorrectStarSysPath( const std::string&, bool &autogenerated );

//string GetSharedUnitPath ();
//string GetSharedMeshPath (const std::string &name);
//string GetSharedSoundPath (const std::string &name);
//string GetSharedTexturePath (const std::string &name);

/*
 *  void initpaths(const std::string& modname="");			// Sets up datadir and load VS config file
 *  void changehome(bool makehomedir=false);				// Changes to user homedir in an OS independant way
 *  void returnfromhome();									// Return from homedir to calling directory
 *  void vs_chdir (const char *path);						// Changes the directory and the current path to reflect it
 *                                                                                                               // Note : may only change ONE level of directory or it breaks
 *  void vs_cdup();											// Goes up one level and changes the direcrtory accordingly
 *  void vs_mkdir (const std::string &nam);					// Creates a directory
 *  void vs_setdir (const char * path);						// Sets the current directory
 *  void vs_resetdir ();									// Reset the current directory
 */

/*
 ***********************************************************************************************
 **** VSFileSystem functions                                                                 ***
 ***********************************************************************************************
 */

void flushLogs();

//Initialize paths
void InitPaths( string conf, string subdir = "" );
void InitDataDirectory();
void InitHomeDirectory();
void LoadConfig( string subdir = "" );
void InitMods();

//Create a directory
void CreateDirectoryAbs( const char *filename );
void CreateDirectoryAbs( const string &filename );
//Create a directory in home_path
void CreateDirectoryHome( const char *filename );
void CreateDirectoryHome( const string &filename );
//Create a directory in data_path_path
void CreateDirectoryData( const char *filename );
void CreateDirectoryData( const string &filename );

/********** DO NO USE FileExists functions directly : USE LookForFile instead **********/
//Test if a directory exists (absolute path)
bool DirectoryExists( const char *filename );
bool DirectoryExists( const string &filename );
//Returns positive int or index in archive if found or -1 if not found
//Test if a file exists (absolute path)
int FileExists( const string &root, const char *filename, VSFileType type = UnknownFile, bool lookinvolume = true );
int FileExists( const string &root, const string &filename, VSFileType type = UnknownFile, bool lookinvolume = true );
//Test if a file exists relative to home_path
int FileExistsHome( const char *filename, VSFileType type = UnknownFile );
int FileExistsHome( const string &filename, VSFileType type = UnknownFile );
//Test if a file exists relative to data_path
int FileExistsData( const char *filename, VSFileType type = UnknownFile );
int FileExistsData( const string &filename, VSFileType type = UnknownFile );

VSError GetError( const char *str = NULL );

VSError LookForFile( VSFile &f, VSFileType type, VSFileMode mode = ReadOnly );
VSError LookForFile( const string &filename, VSFileType type, VSFileMode mode = ReadOnly );

const boost::filesystem::path GetSavePath();

/*
 ***********************************************************************************************
 **** VSFileSystem::VSFile functions                                                         ***
 ***********************************************************************************************
 */

class VSFile
{
private:
//STDIO stuff
    FILE *fp;

//PK3 stuff
    CPK3 *pk3_file;
    char *pk3_extracted_file;
    int   file_index;
    unsigned int offset;

    void checkExtracted();

//VSFile internals
    VSFileType   file_type;
    VSFileType   alt_type;
    VSFileMode   file_mode;
    VSVolumeType volume_type;

    string filename;
    string subdirectoryname;
    string directoryname;
    string rootname;

    unsigned long size;
    bool   valid;

public:
    char * get_pk3_data()
    {
        return pk3_extracted_file;
    }

public: VSFile();
    VSFile( const char *buffer, long size, VSFileType type = ZoneBuffer, VSFileMode = ReadOnly );
    VSFile( const char *filename, VSFileType type = UnknownFile, VSFileMode = ReadOnly );
    ~VSFile();

    FILE * GetFP()
    {
        return this->fp;
    }                                                        //This is still needed for special cases (when loading PNG files)
    char * GetFileBuffer()
    {
        return this->pk3_extracted_file;
    }

    const string GetSystemDirectoryPath(string& file);

/********************************** OPEN A FILE *********************************/
//Open an existing file read only
    VSError OpenReadOnly( const char *filename, VSFileType type = UnknownFile );
    VSError OpenReadOnly( const string &filename, VSFileType type = UnknownFile )
    {
        return OpenReadOnly( filename.c_str(), type );
    }
//Open an existing file read/write
    VSError OpenReadWrite( const char *filename, VSFileType type = UnknownFile );
    VSError OpenReadWrite( const string &filename, VSFileType type = UnknownFile )
    {
        return OpenReadWrite( filename.c_str(), type );
    }
//Open (truncate) or create a file read/write
    VSError OpenCreateWrite( const char *filename, VSFileType type = UnknownFile );
    VSError OpenCreateWrite( const string &filename, VSFileType type = UnknownFile )
    {
        return OpenCreateWrite( filename.c_str(), type );
    }
//Close the file
    void Close();

/********************************** READ/WRITE OPERATIONS IN A FILE *********************************/
    size_t Read( void *ptr, size_t length );                                            //Read length in ptr (store read bytes number in length)
    VSError ReadLine( void *ptr, size_t length );                               //Read a line of maximum length
    string ReadFull();                                                                                          //Read the entire file and returns the content in a string
    size_t Write( const void *ptr, size_t length );                             //Write length from ptr (store written bytes number in length)
    size_t Write( const string &content );                                              //Write a string
    VSError WriteLine( const void *ptr );                                               //Write a line
    void WriteFull( void *ptr );                                                                //Write

#if 0
    int Fscanf( const char *format, ... );
#endif
    void GoAfterEOL( unsigned int length );
    void GoAfterEOL();

    template < class A >
    int Fscanf( const char *format, A *a )
    {
        int ret = -1;
        if (!UseVolumes[this->alt_type]) {
            ret = fscanf( fp, format, a );
        } else {
            if (q_volume_format == vfmtPK3) {
                checkExtracted();
                int   readbytes = 0;
                int   length    = strlen( format );
                int   newlength = length+3;
                char *newformat = new char[newlength];
                memset( newformat, 0, newlength );
                memcpy( newformat, format, length );
                strcat( newformat, "%n" );
                ret = sscanf( this->pk3_extracted_file+offset, newformat, a, &readbytes );
                delete[] newformat;
                BOOST_LOG_TRIVIAL(info)<<"FSCANF : sscanf read "<<readbytes<<" bytes - OFFSET="<<offset<<" VALUES : a="<<(*a)<<std::endl;
                this->offset += readbytes;
                this->GoAfterEOL();
            } else if (q_volume_format == vfmtVSR) {}
        }
        return ret;
    }
    template < class A, class B >
    int Fscanf( const char *format, A *a, B *b )
    {
        int ret = -1;
        if (!UseVolumes[this->alt_type]) {
            ret = fscanf( fp, format, a, b );
        } else {
            if (q_volume_format == vfmtPK3) {
                checkExtracted();
                int   readbytes = 0;
                int   length    = strlen( format );
                int   newlength = length+3;
                char *newformat = new char[newlength];
                memset( newformat, 0, newlength );
                memcpy( newformat, format, length );
                strcat( newformat, "%n" );
                ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, &readbytes );
                delete[] newformat;
                BOOST_LOG_TRIVIAL(info)<<"FSCANF : sscanf read "<<readbytes<<" bytes - OFFSET="<<offset<<" VALUES : a="<<(*a)<<", b="<<(*b)
                         <<std::endl;
                this->offset += readbytes;
                this->GoAfterEOL();
            } else if (q_volume_format == vfmtVSR) {}
        }
        return ret;
    }
    template < class A, class B, class C >
    int Fscanf( const char *format, A *a, B *b, C *c )
    {
        int ret = -1;
        if (!UseVolumes[this->alt_type]) {
            ret = fscanf( fp, format, a, b, c );
        } else {
            if (q_volume_format == vfmtPK3) {
                checkExtracted();
                int   ret       = -1, readbytes = 0;
                int   length    = strlen( format );
                int   newlength = length+3;
                char *newformat = new char[newlength];
                memset( newformat, 0, newlength );
                memcpy( newformat, format, length );
                strcat( newformat, "%n" );
                ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, &readbytes );
                delete[] newformat;
                BOOST_LOG_TRIVIAL(info)<<"FSCANF : sscanf read "<<readbytes<<" bytes - OFFSET="<<offset<<" VALUES : a="<<(*a)<<", b="<<(*b)
                         <<", c="<<(*c)<<std::endl;
                this->offset += readbytes;
                this->GoAfterEOL();
            } else if (q_volume_format == vfmtVSR) {}
        }
        return ret;
    }
    template < class A, class B, class C, class D >
    int Fscanf( const char *format, A *a, B *b, C *c, D *d )
    {
        int ret = -1;
        if (!UseVolumes[this->alt_type]) {
            ret = fscanf( fp, format, a, b, c, d );
        } else {
            if (q_volume_format == vfmtPK3) {
                checkExtracted();
                int   ret       = -1, readbytes = 0;
                int   length    = strlen( format );
                int   newlength = length+3;
                char *newformat = new char[newlength];
                memset( newformat, 0, newlength );
                memcpy( newformat, format, length );
                strcat( newformat, "%n" );
                ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, d, &readbytes );
                delete[] newformat;
                this->offset += readbytes;
                this->GoAfterEOL();
            } else if (q_volume_format == vfmtVSR) {}
        }
        return ret;
    }
    template < class A, class B, class C, class D, class E >
    int Fscanf( const char *format, A *a, B *b, C *c, D *d, E *e )
    {
        int ret = -1;
        if (!UseVolumes[this->alt_type]) {
            ret = fscanf( fp, format, a, b, c, d, e );
        } else {
            if (q_volume_format == vfmtPK3) {
                checkExtracted();
                int   ret       = -1, readbytes = 0;
                int   length    = strlen( format );
                int   newlength = length+3;
                char *newformat = new char[newlength];
                memset( newformat, 0, newlength );
                memcpy( newformat, format, length );
                strcat( newformat, "%n" );
                ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, d, e, &readbytes );
                delete[] newformat;
                this->offset += readbytes;
                this->GoAfterEOL();
            } else if (q_volume_format == vfmtVSR) {}
        }
        return ret;
    }
    template < class A, class B, class C, class D, class E, class F >
    int Fscanf( const char *format, A *a, B *b, C *c, D *d, E *e, F *f )
    {
        int ret = -1;
        if (!UseVolumes[this->alt_type]) {
            ret = fscanf( fp, format, a, b, c, d, e, f );
        } else {
            if (q_volume_format == vfmtPK3) {
                checkExtracted();
                int   ret       = -1, readbytes = 0;
                int   length    = strlen( format );
                int   newlength = length+3;
                char *newformat = new char[newlength];
                memset( newformat, 0, newlength );
                memcpy( newformat, format, length );
                strcat( newformat, "%n" );
                ret = sscanf( this->pk3_extracted_file+offset, newformat, a, b, c, d, e, f, &readbytes );
                delete[] newformat;
                this->offset += readbytes;
                this->GoAfterEOL();
            } else if (q_volume_format == vfmtVSR) {}
        }
        return ret;
    }

    int Fprintf( const char *format, ... );

/********************************** FILE POINTER POSITION OPERATIONS IN A FILE *********************************/
    long Size();                                                        //Returns the size of the file : current position in file is not changed
    void Begin();                                                       //Set the file pointer to the beginning of the file
    long GetPosition();                                         //Get the current file pointer position
    void End();                                                         //Set the file pointer to the end of the file
    void GoTo( long offset );                                   //Does a fseek( fp, offset, SEEK_SET);
    void Clear();                                                       //Empty the file and then set file pointer to the beginning
    bool Eof();                                                         //Return true if end of file
    bool Valid();                                                       //Tells wether the file is valid or not

/********************************** FILE PATH *********************************/
    const string& GetFilename() const
    {
        return this->filename;
    }
    const string& GetDirectory() const
    {
        return this->directoryname;
    }
    const string& GetSubDirectory() const
    {
        return this->subdirectoryname;
    }
    const string& GetRoot() const
    {
        return this->rootname;
    }

    void SetFilename( const string &filename )
    {
        this->filename = filename;
    }
    void SetDirectory( const string &directory )
    {
        this->directoryname = directory;
    }
    void SetSubDirectory( const string &subdirectory )
    {
        this->subdirectoryname = subdirectory;
    }
    void SetRoot( const string &root )
    {
        this->rootname = root;
    }

    string GetFullPath();
    string GetAbsPath();

    void SetType( VSFileType type );
    void SetAltType( VSFileType type );
    VSFileType GetType() const
    {
        return file_type;
    }
    VSFileType GetAltType() const
    {
        return alt_type;
    }

    void SetIndex( int index );
    void SetVolume( VSVolumeType big );
    bool UseVolume();

    friend class VSImage;

private:
    void private_init();
};
};

std::ostream&operator<<( std::ostream &ostr, VSFileSystem::VSError err );
std::ostream&operator<<( std::ostream &ostr, VSFileSystem::VSFileType type );
std::string nameof( VSFileSystem::VSFileType type );

#if defined (_WIN32) && !defined (__CYGWIN__)
//Emulation of posix scandir() call
int scandir( const char *dirname, struct dirent ***namelist,
            int (*select)( const struct dirent* ),
            int (*compar)( const struct dirent**, const struct dirent** ) );
#endif

#endif
