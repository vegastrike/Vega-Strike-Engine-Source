#ifndef __NETBUFFER_H
#define __NETBUFFER_H

#include <string.h>
#include "cmd/weapon_xml.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include "networking/lowlevel/vsnet_clientstate.h"
#include "cmd/unit_armorshield.h"
#include "gfxlib_struct.h"

struct GFXColor;
std::string getSimpleString(std::string &input);
char getSimpleChar(std::string &input);
int getSimpleInt(std::string &input);
void addSimpleString(std::string &input, const std::string adder);
void addSimpleChar(std::string &input, const char adder);
void addSimpleInt(std::string &input, const int adder);
class NetBuffer
{
		char *	buffer;
		unsigned int	offset;
		unsigned int	size;
	
	public :
		NetBuffer();
		NetBuffer( int bufsize);
		NetBuffer( const char * buf, int bufsize);
		~NetBuffer();

		void			Reset();
		char *			getData();
		unsigned int getOffset()const {return offset;} //useful to see if it's at the end of data
		// Extends the buffer if we exceed its size
		void			resizeBuffer( unsigned int newsize);
		// Check if there is still enough data for 'len' to be read in buffer
		bool			checkBuffer( int len, const char * fun);

		void			addClientState( ClientState cs);
		ClientState 	getClientState();
		void			addVector( Vector v);
		Vector			getVector();
		void			addQVector( QVector v);
		QVector			getQVector();
		void			addColor( GFXColor col);
		GFXColor		getColor();
		void			addMatrix( Matrix m);
		Matrix			getMatrix();
		void			addQuaternion( Quaternion quat);
		Quaternion		getQuaternion();
		void			addTransformation( Transformation trans);
		Transformation	getTransformation();
		void			addWeaponInfo( weapon_info wi)
		{
			wi.netswap();
			char *	buf = NULL;
			int		wi_size = 0;
			setWeaponInfoToBuffer( wi, buf, wi_size);
			resizeBuffer( offset+wi_size);
			memcpy( buffer+offset, buf, wi_size);
			offset += wi_size;
			delete buf;
		}
		weapon_info	getWeaponInfo()
		{
			int wisize=0;
			weapon_info tmp = getWeaponInfoFromBuffer( buffer+offset, wisize);
			tmp.netswap();
			offset += wisize;
			return tmp;
		}
		void		addShield( const Shield &shield);
		Shield		getShield();
		void		addArmor( const Armor &armor);
		Armor		getArmor();

		void			addSerial( ObjSerial serial);
		ObjSerial		getSerial();
		void			addFloat( float f);
		float			getFloat();
		void			addDouble( double d);
		double			getDouble();
		void			addShort( unsigned short s);
		unsigned short	getShort();
		void			addInt32( int i);
		int				getInt32();
		void			addUInt32( unsigned int i);
		unsigned int	getUInt32();
		void			addChar( char c);
		char			getChar();
		void			addType( unsigned char t);
		bool			checkType( unsigned char t);
		unsigned char	getType();
		void			addBuffer( const unsigned char * buf, int bufsize);
        unsigned char*  extAddBuffer( int bufsize);
		unsigned char*  getBuffer( int offt);
		// Add and get a string with its length before the char * buffer part
		void			addString( const string& str);
		string			getString();

		GFXMaterial		getGFXMaterial();
		void			addGFXMaterial( const GFXMaterial & mat);
		GFXLight		getGFXLight();
		void			addGFXLight( const GFXLight & light);
		GFXLightLocal	getGFXLightLocal();
		void			addGFXLightLocal( const GFXLightLocal & light);

		unsigned int	getDataLength();
		unsigned int	getSize();
};

#endif
