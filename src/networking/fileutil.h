#ifndef __FILEUTIL_H
#define __FILEUTIL_H

#include <string>
#include <vector>
#include "networking/const.h"
#include "vsfilesystem.h"

using namespace VSFileSystem;
using std::string;
using std::vector;

#ifdef CRYPTO
#include <crypto++/sha.h>
using namespace CryptoPP;
#endif

namespace FileUtil
{
	// Returns 2 strings -> vector[0] = xml unit, vector[1] = player save
	vector<string>	GetSaveFromBuffer( const char * buffer);
	void			WriteSaveFiles( string savestr, string xmlstr, string name);

	extern bool				use_crypto;
#ifdef CRYPTO
	extern HASHMETHOD		Hash;
#endif
	void			displayHash( unsigned char * hash, unsigned int length);
	int				HashCompare( string filename, unsigned char * hashdigest, VSFileType type);
	int				HashFileCompute( string filename, unsigned char * hashdigest, VSFileType type);
	int				HashCompute( const char * filename, unsigned char * digest, VSFileType type);
	int				HashStringCompute( std::string buffer, unsigned char * digest);
}

#endif
