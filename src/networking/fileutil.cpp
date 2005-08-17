#include "cmd/unit_generic.h"
#include "networking/lowlevel/vsnet_headers.h"
#include "fileutil.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "networking/lowlevel/netbuffer.h"

//#ifdef _WIN32
//#include <winsock.h> // for ntohl
//#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifdef FAR
#define FAR
#endif


#ifdef CRYPTO
#include <crypto++/sha.h>
HASHMETHOD	FileUtil::Hash;
#endif

using namespace VSFileSystem;

bool	FileUtil::use_crypto = false;

void	FileUtil::WriteSaveFiles( string savestr, string xmlstr, string name)
{
	string savefile;

	// Write the save file
	savefile = name+".save";
	VSFile f;
	VSError err = f.OpenCreateWrite( savefile, AccountFile);
	if( err>Ok)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		VSExit(1);
	}
	f.Write( savestr);
	f.Close();
	// Write the XML file
	savefile = name+".xml";
	err = f.OpenCreateWrite( savefile, AccountFile);
	if( err>Ok)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		VSExit(1);
	}
	f.Write( xmlstr);
	f.Close();
}

vector<string>	FileUtil::GetSaveFromBuffer( NetBuffer &buffer)
{
	vector<string> saves;
	// Extract the length of save file
	saves.push_back( buffer.getString() );
	saves.push_back( buffer.getString() );

	return saves;
}

int		FileUtil::HashStringCompute( std::string buffer, unsigned char * digest)
{
#ifdef CRYPTO
if( use_crypto)
{
	HASHMETHOD		Hash;

	const int block_size = Hash.OptimalBlockSize();
	unsigned char * hashbuf = new unsigned char[block_size];
	const char * buf = buffer.c_str();
	int nb=0;
	unsigned int offset=0;

	while( offset<buffer.length())
	{
		memcpy( hashbuf, buf+offset, block_size);
		offset += block_size;
		Hash.Update( hashbuf, block_size);
	}
	// Here offset is bigger than buffer length so we go back offset times
	offset -= block_size;
	if( ( nb=( buffer.length()-offset)) )
	{
		memcpy( hashbuf, buf+offset, nb);
		Hash.Update( hashbuf, nb);
	}

	Hash.Final( digest);
	delete hashbuf;

	return 0;
}
else
	return 0;
#else
	return 0;
#endif
}

void	FileUtil::displayHash( unsigned char * hash, unsigned int length)
{
	for( unsigned int i=0; i<length; i++)
		cerr<<hash[i];
}

int		FileUtil::HashCompute( const char * filename, unsigned char * digest, VSFileType type)
{
#ifdef CRYPTO
if( use_crypto)
{
	HASHMETHOD		Hash;
	VSFile f;
	VSError err = f.OpenReadOnly( filename, type);
	if( err>Ok)
	{
		if( errno==ENOENT)
			// Return 1 if file does not exists
			return 1;
		else
		{
			cerr<<"!!! ERROR = couldn't compute hash digest on "<<filename<<" file !!!"<<endl;
			VSExit(1);
			//return -1;
		}
	}

	const int block_size = Hash.OptimalBlockSize();
	unsigned char * buffer = new unsigned char[block_size];
	int nb=0;

	while( (nb = f.Read( buffer, block_size)) > 0)
		Hash.Update( buffer, nb);

	Hash.Final( digest);
	delete []buffer;
	f.Close();

	return 0;
}
else
	return 0;
#else
	return 0;
#endif
}

// Warning : now takes an absolute path
int		FileUtil::HashCompare( string filename, unsigned char * hashdigest, VSFileType type)
{
#ifdef CRYPTO
if( use_crypto)
{
	HASHMETHOD		Hash;
	//string full_univ_path = VSFileSystem::datadir+filename;
	unsigned char * local_digest = new unsigned char[Hash.DigestSize()];
	int ret;
	//ret=HashCompute( full_univ_path.c_str(), local_digest);
	ret=HashCompute( filename.c_str(), local_digest, type);
	// If the file does not exist or if hashes are !=
	if( ret)
	{
		delete []local_digest;
		return 0;
	}
	if( memcmp( hashdigest, local_digest, Hash.DigestSize()))
	{
		cerr<<"HashDigest does not match : '";
		displayHash(hashdigest, Hash.DigestSize());
		cerr<<"' != '";
		displayHash(local_digest, Hash.DigestSize());
		cerr<<"' for file "<<filename<<endl;
		delete []local_digest;
		return 0;
	}
	cerr<<"HashDigest MATCH : '";
	displayHash(hashdigest, Hash.DigestSize());
	cerr<<"' == '";
	displayHash(local_digest, Hash.DigestSize());
	cerr<<"' for file "<<filename<<endl;

	delete []local_digest;
	return 1;
}
else
	return 1;
#else
	return 1;
#endif
}

int		FileUtil::HashFileCompute( string filename, unsigned char * hashdigest, VSFileType type)
{
#ifdef CRYPTO
if( use_crypto)
{
	HASHMETHOD		Hash;
	//string fulluniv = VSFileSystem::datadir+filename;
	int ret;
	if( (ret=HashCompute( filename.c_str(), hashdigest, type))<0 || ret)
	{
		cerr<<"!!! ERROR = couldn't get "<<filename<<" HashDigest (not found or error) !!!"<<endl;
		if( ret)
			VSExit(1);
		cerr<<"-- FILE HASH : "<<filename<<" = "<<hashdigest<<" --"<<endl;
	}
	else
		cerr<<"-- FILE HASH : "<<filename<<" = "<<hashdigest<<" --"<<endl;

	return ret;
}
else
	return 0;
#else
	return 0;
#endif
}

