#include <string>
#include <stdio.h>
#include <errno.h>
#include "endianness.h"
#include "cmd/unit_generic.h"
#include "packet.h"
#include "netbuffer.h"
#include "client.h"
#include "md5.h"
#include "vs_path.h"

int	md5Compute( string filename, unsigned char * md5digest)
{
	// Add the galaxy md5sum in the netbuffer (as we should be at the end of it) in order to control on client side
	string fulluniv = datadir+filename;
	int ret;
	if( (ret=md5SumFile( fulluniv.c_str(),
							md5digest))<0 || ret)
	{
		cout<<"!!! ERROR = couldn't get universe file md5sum (not found or error) !!!"<<endl;
	}
	else
		cout<<"-- MD5 FILE : "<<fulluniv<<" = "<<md5digest<<" --"<<endl;

	return ret;
}

int md5CheckFile( string filename, unsigned char * md5digest)
{
	string full_univ_path = datadir+filename;
	unsigned char * local_digest = new unsigned char[MD5_DIGEST_SIZE];
	int ret;
	if( (ret=md5SumFile( full_univ_path.c_str(), local_digest))<0)
		cout<<"!!! ERROR = couldn't compute md5 digest on universe file !!!"<<endl;
	delete local_digest;
	// If the file does not exist or if md5sum are !=
	if( ret) return 0;
	if( memcmp( md5digest, local_digest, MD5_DIGEST_SIZE))
	{
		cout<<"MD5 does not match : "<<md5digest<<" != "<<local_digest<<endl;
		return 0;
	}
	return 1;
}
int	md5SumFile( const char * filename, unsigned char * digest)
{
	FILE * fp = fopen( filename, "r");
	if( !fp)
	{
		if( errno==ENOENT)
			// Return 1 if file does not exists
			return 1;
		else
		{
			cout<<"!!! ERROR = couldn't compute md5 digest on universe file !!!"<<endl;
			exit(1);
			//return -1;
		}
	}

	unsigned char buffer[1024];
	MD5_CTX	ctx;
	int nb=0;

	MD5Init( &ctx);

	while( (nb = fread( buffer, sizeof( unsigned char), 1024, fp)) > 0)
		MD5Update( &ctx, buffer, nb);

	MD5Final( digest, &ctx);
	fclose( fp);

	return 0;
}

using std::string;

ClientState::ClientState()
{
	this->delay=50;
	this->client_serial = 0;
	this->pos.position.Set( 20400,-35400,84700000.0);
	this->veloc.Set( 0,0,0);
	this->accel.Set( 0,0,0);
}

ClientState::ClientState( Unit * un)
{
	this->client_serial = un->GetSerial();
	this->pos.position = un->curr_physical_state.position;
	this->pos.orientation = un->curr_physical_state.orientation;
	this->veloc = un->Velocity;
	this->accel = un->GetAcceleration();
}
ClientState::ClientState( ObjSerial serial)
{
	this->client_serial = serial;
	this->pos.position.Set( 20400,-35400,84700000.0);
	//pos.orientation.Set( 100,0,0);
	this->veloc.Set( 0,0,0);
	this->accel.Set( 0,0,0);
	this->delay = 50;
}

ClientState::ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc)
{
	this->client_serial = serial;
	this->pos.position = posit;
	this->pos.orientation = orientat;
	this->veloc = velocity;
	this->accel = acc;
}

ClientState::ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc, unsigned int del)
{
	this->delay = del;
	this->client_serial = serial;
	this->pos.position = posit;
	this->pos.orientation = orientat;
	this->veloc = velocity;
	this->accel = acc;
}

ClientState::ClientState( ObjSerial serial, Transformation trans, Vector velocity, Vector acc, unsigned int del)
{
	this->delay = del;
	this->client_serial = serial;
	this->pos = trans;
	this->veloc = velocity;
	this->accel = acc;
}

void		ClientState::display() const
{
	cout<<"SERIAL="<<this->client_serial;
	cout<<" - Position="<<pos.position.i<<","<<pos.position.j<<","<<pos.position.k;
	cout<<" - Orientation="<<pos.orientation.v.i<<","<<pos.orientation.v.j<<","<<pos.orientation.v.k;
	cout<<" - Velocity="<<veloc.i<<","<<veloc.j<<","<<veloc.k;
	cout<<" - Acceleration="<<accel.i<<","<<accel.j<<","<<accel.k<<endl;
}

int		ClientState::operator==( const ClientState & ctmp)
{
	// Need == operator for Transformation class
	return (this->client_serial==ctmp.client_serial && this->pos.position==ctmp.pos.position && this->pos.orientation==ctmp.pos.orientation);
	//return 0;
}

void	ClientState::netswap()
{
	// Switch everything to host byte order
	this->delay = VSSwapHostIntToLittle( this->delay);
	this->client_serial = OBJSERIAL_TONET( this->client_serial);

	this->pos.netswap();
	this->veloc.netswap();
	this->accel.netswap();
}

std::ostream& operator<<( std::ostream& ostr, const Client& c )
{
    ostr << "(clnt addr " << c.cltadr
         << " sock=" << c.sock;
    return ostr;
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

void	FileUtil::WriteSaveFiles( string savestr, string xmlstr, string path, string name)
{
	string savefile;
	FILE * fp;

	// Write the save file
	savefile = path+name+".save";
	fp = fopen( savefile.c_str(), "w");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		exit(1);
	}
	fwrite( savestr.c_str(), sizeof( char), savestr.length(), fp);
	fclose( fp);
	// Write the XML file
	savefile = path+name+".xml";
	fp = fopen( savefile.c_str(), "w");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		exit(1);
	}
	fwrite( xmlstr.c_str(), sizeof( char), xmlstr.length(), fp);
	fclose( fp);
}
