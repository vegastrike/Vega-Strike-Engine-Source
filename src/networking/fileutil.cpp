#include "fileutil.h"
#include "networking/lowlevel/vsnet_headers.h"
#include "vs_path.h"

#ifdef _WIN32
#include <winsock2.h> // for ntohl
#endif

#ifdef CRYPTO
#include <crypto++/sha.h>
HASHMETHOD	FileUtil::Hash;
#endif


void	FileUtil::WriteSaveFiles( string savestr, string xmlstr, string path, string name)
{
	string savefile;
	FILE * fp;

	// Write the save file
	savefile = path+name+".save";
	fp = fopen( savefile.c_str(), "wb");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		exit(1);
	}
	fwrite( savestr.c_str(), sizeof( char), savestr.length(), fp);
	fclose( fp);
	// Write the XML file
	savefile = path+name+".xml";
	fp = fopen( savefile.c_str(), "wb");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		exit(1);
	}
	fwrite( xmlstr.c_str(), sizeof( char), xmlstr.length(), fp);
	fclose( fp);
}

vector<string>	FileUtil::GetSaveFromBuffer( const char * buffer)
{
	vector<string> saves;
	// Extract the length of save file
	unsigned int save_size = ntohl( *( (unsigned int *)(buffer)));
	cout<<"\tSave size = "<<save_size<<endl;
	// Extract the length of xml file
	unsigned int xml_size = ntohl( *( (unsigned int *)(buffer + sizeof( unsigned int) + save_size)));
	cout<<"\tXML size = "<<xml_size<<endl;

	int buflen = 2*sizeof( unsigned int)+save_size+xml_size;
	char * savebuf = new char[buflen+1];
	memcpy( savebuf, buffer, buflen);
	savebuf[buflen] = 0;
	savebuf[sizeof( unsigned int)+save_size]=0;
	savebuf[2*sizeof( unsigned int)+xml_size+save_size]=0;
	// First element is XML Unit and second element is player save
	saves.push_back( string( savebuf+2*sizeof( unsigned int)+save_size));
	saves.push_back( string( savebuf+sizeof( unsigned int)));
	delete savebuf;

	return saves;
}

int		FileUtil::HashStringCompute( std::string buffer, unsigned char * digest)
{
#ifdef CRYPTO
	HASHMETHOD		Hash;

	const int block_size = Hash.OptimalBlockSize();
	unsigned char * hashbuf = new unsigned char[block_size];
	char * buf = buffer.c_str();
	int nb=0, offset=0;

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
#else
	return 0;
#endif
}

int		FileUtil::HashCompute( const char * filename, unsigned char * digest)
{
#ifdef CRYPTO
	HASHMETHOD		Hash;
	FILE * fp = fopen( filename, "rb");
	if( !fp)
	{
		if( errno==ENOENT)
			// Return 1 if file does not exists
			return 1;
		else
		{
			cout<<"!!! ERROR = couldn't compute hash digest on "<<filename<<" file !!!"<<endl;
			exit(1);
			//return -1;
		}
	}

	const int block_size = Hash.OptimalBlockSize();
	unsigned char * buffer = new unsigned char[block_size];
	int nb=0;

	while( (nb = fread( buffer, sizeof( unsigned char), block_size, fp)) > 0)
		Hash.Update( buffer, nb);

	Hash.Final( digest);
	delete buffer;
	fclose( fp);

	return 0;
#else
	return 0;
#endif
}

int		FileUtil::HashCompare( string filename, unsigned char * hashdigest)
{
#ifdef CRYPTO
	HASHMETHOD		Hash;
	string full_univ_path = datadir+filename;
	unsigned char * local_digest = new unsigned char[Hash.DigestSize()];
	int ret;
	ret=HashCompute( full_univ_path.c_str(), local_digest);
	// If the file does not exist or if hashes are !=
	if( ret)
	{
		delete local_digest;
		return 0;
	}
	if( memcmp( hashdigest, local_digest, Hash.DigestSize()))
	{
		cout<<"HashDigest does not match : "<<hashdigest<<" != "<<local_digest<<endl;
		delete local_digest;
		return 0;
	}
	delete local_digest;
	return 1;
#else
	return 0;
#endif
}

int		FileUtil::HashFileCompute( string filename, unsigned char * hashdigest)
{
#ifdef CRYPTO
	HASHMETHOD		Hash;
	string fulluniv = datadir+filename;
	int ret;
	if( (ret=HashCompute( fulluniv.c_str(), hashdigest))<0 || ret)
	{
		cerr<<"!!! ERROR = couldn't get "<<filename<<" HashDigest (not found or error) !!!"<<endl;
		cerr<<"-- FILE HASH : "<<fulluniv<<" = "<<hashdigest<<" --"<<endl;
	}
	else
		cerr<<"-- FILE HASH : "<<fulluniv<<" = "<<hashdigest<<" --"<<endl;

	return ret;
#else
	return 0;
#endif
}

