#ifndef __FILEUTIL_H
#define __FILEUTIL_H

#include <string>
#include <vector>
#include "networking/const.h"

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
	void			WriteSaveFiles( string savestr, string xmlstr, string path, string name);

#ifdef CRYPTO
	extern HASHMETHOD		Hash;
#endif
	int				HashCompare( string filename, unsigned char * hashdigest);
	int				HashFileCompute( string filename, unsigned char * hashdigest);
	int				HashCompute( const char * filename, unsigned char * digest);
}

#endif
