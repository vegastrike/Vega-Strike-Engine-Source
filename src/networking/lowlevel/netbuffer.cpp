#include <assert.h>
#include "networking/lowlevel/netbuffer.h"
#include "networking/const.h"
#include "posh.h"
#include "gfxlib_struct.h"

NetBuffer::NetBuffer()
		{
			buffer = new char[MAXBUFFER];
			offset = 0;
			size = MAXBUFFER;
			memset( buffer, 0x20, size);
			this->buffer[size-1] = 0;
		}
NetBuffer::NetBuffer( int bufsize)
		{
			buffer = new char[bufsize];
			offset = 0;
			size = bufsize;
			memset( buffer, 0x20, size);
			this->buffer[size-1] = 0;
		}
NetBuffer::NetBuffer( char * buf, int bufsize)
		{
			offset = 0;
			size=bufsize+1;
			this->buffer = new char[size];
			memset( buffer, 0x20, size);
			memcpy( buffer, buf, bufsize);
			this->buffer[size-1] = 0;
		}
NetBuffer::NetBuffer( const char * buf, int bufsize)
		{
			offset = 0;
			size=bufsize+1;
			this->buffer = new char[size];
			memset( buffer, 0x20, size);
			memcpy( buffer, buf, bufsize);
			this->buffer[size-1] = 0;
		}
NetBuffer::~NetBuffer()
		{
			if( buffer != NULL)
				delete []buffer;
		}
void	NetBuffer::Reset()
		{
			memset( buffer, 0x20, size);
			offset=0;
		}

char *	NetBuffer::getData() { return buffer;}

		// Extends the buffer if we exceed its size
void	NetBuffer::resizeBuffer( int newsize)
		{
			if( size-1 < newsize)
			{
				char * tmp = new char [newsize+1];
				memset( tmp, 0, newsize+1);
				memcpy( tmp, buffer, size);
				delete buffer;
				buffer = tmp;
				size = newsize+1;
			}
		}
// Check the buffer to see if we can still get info from it
void	NetBuffer::checkBuffer( int len, const char * fun)
{
#ifndef NDEBUG
	if( offset+len > size-1)
	{
		cerr<<"!!! ERROR : trying to read more data than buffer size (offset="<<offset<<" - size="<<size<<" - to read="<<len<<") in "<<fun<<" !!!"<<endl;
		exit(1);
	}
#endif
}

// NOTE : IMPORTANT - I ONLY INCREMENT OFFSET IN PRIMARY DATATYPES SINCE ALL OTHER ARE COMPOSED WITH THEM

void	NetBuffer::addClientState( ClientState cs)
		{
			this->addUInt32( cs.delay);
			this->addSerial( cs.client_serial);
			this->addTransformation( cs.pos);
			this->addVector( cs.veloc);
			this->addVector( cs.accel);
		}
ClientState NetBuffer::getClientState()
		{
			ClientState cs;
			cs.delay = this->getUInt32();
			cs.client_serial = this->getSerial();
			cs.pos = this->getTransformation();
			cs.veloc = this->getVector();
			cs.accel = this->getVector();

			return cs;
		}

void	NetBuffer::addVector( Vector v)
		{
			this->addFloat( v.i);
			this->addFloat( v.j);
			this->addFloat( v.k);
		}
Vector	NetBuffer::getVector()
		{
			Vector v;
			v.i = this->getFloat();
			v.j = this->getFloat();
			v.k = this->getFloat();

			return v;
		}
void	NetBuffer::addQVector( QVector v)
		{
			this->addDouble( v.i);
			this->addDouble( v.j);
			this->addDouble( v.k);
		}
QVector	NetBuffer::getQVector()
		{
			Vector v;
			v.i = this->getDouble();
			v.j = this->getDouble();
			v.k = this->getDouble();

			return v;
		}
void	NetBuffer::addColor( GFXColor col)
		{
			this->addFloat( col.r);
			this->addFloat( col.g);
			this->addFloat( col.b);
			this->addFloat( col.a);
		}
GFXColor NetBuffer::getColor()
		{
			GFXColor col;
			col.r = this->getFloat();
			col.g = this->getFloat();
			col.b = this->getFloat();
			col.a = this->getFloat();

			return col;
		}
void	NetBuffer::addMatrix( Matrix m)
		{
			for( int i=0; i<9; i++)
				this->addFloat( m.r[i]);
			this->addQVector( m.p);
		}
Matrix	NetBuffer::getMatrix()
		{
			Matrix m;
			for( int i=0; i<9; i++)
				m.r[i] = this->getFloat();
			m.p = this->getQVector();

			return m;
		}
void	NetBuffer::addQuaternion( Quaternion quat)
		{
			this->addFloat( quat.s);
			this->addVector( quat.v);
		}
Quaternion	NetBuffer::getQuaternion()
		{
			Quaternion q;

			q.s = this->getFloat();
			q.v = this->getVector();

			return q;
		}
void	NetBuffer::addTransformation( Transformation trans)
		{
			this->addQuaternion( trans.orientation);
			this->addQVector( trans.position);
		}
Transformation	NetBuffer::getTransformation()
		{
			Transformation t;
			t.orientation = this->getQuaternion();
			t.position = this->getQVector();

			return t;
		}

void	NetBuffer::addShield( Shield shield)
{
	this->addChar( shield.number);
	this->addChar( shield.leak);
	this->addFloat( shield.recharge);
	switch( shield.number)
	{
		case 2 :
			this->addFloat( shield.fb[0]);	
			this->addFloat( shield.fb[1]);	
			this->addFloat( shield.fb[2]);	
			this->addFloat( shield.fb[3]);	
		break;
		case 4 :
			this->addShort( shield.fbrl.front);
			this->addShort( shield.fbrl.back);
			this->addShort( shield.fbrl.right);
			this->addShort( shield.fbrl.left);
			this->addShort( shield.fbrl.frontmax);
			this->addShort( shield.fbrl.backmax);
			this->addShort( shield.fbrl.rightmax);
			this->addShort( shield.fbrl.leftmax);
		break;
		case 6 :
			this->addShort( shield.fbrltb.v[0]);
			this->addShort( shield.fbrltb.v[1]);
			this->addShort( shield.fbrltb.v[2]);
			this->addShort( shield.fbrltb.v[3]);
			this->addShort( shield.fbrltb.v[4]);
			this->addShort( shield.fbrltb.v[5]);
			this->addShort( shield.fbrltb.fbmax);
			this->addShort( shield.fbrltb.rltbmax);
		break;
	}
}
Shield	NetBuffer::getShield()
{
	Shield shield;
	shield.number = this->getChar();
	shield.leak = this->getChar();
	shield.recharge = this->getFloat();
	switch( shield.number)
	{
		case 2 :
			shield.fb[0]=this->getFloat();
			shield.fb[1]=this->getFloat();
			shield.fb[2]=this->getFloat();
			shield.fb[3]=this->getFloat();
		break;
		case 4 :
			shield.fbrl.front = this->getShort();
			shield.fbrl.back = this->getShort();
			shield.fbrl.right = this->getShort();
			shield.fbrl.left = this->getShort();
			shield.fbrl.frontmax = this->getShort();
			shield.fbrl.backmax = this->getShort();
			shield.fbrl.rightmax = this->getShort();
			shield.fbrl.leftmax = this->getShort();
		break;
		case 6 :
			shield.fbrltb.v[0] = this->getShort();
			shield.fbrltb.v[1] = this->getShort();
			shield.fbrltb.v[2] = this->getShort();
			shield.fbrltb.v[3] = this->getShort();
			shield.fbrltb.v[4] = this->getShort();
			shield.fbrltb.v[5] = this->getShort();
			shield.fbrltb.fbmax = this->getShort();
			shield.fbrltb.rltbmax = this->getShort();
		break;
	}

	return shield;
}
void		NetBuffer::addArmor( Armor armor)
{
	this->addShort( armor.front);
	this->addShort( armor.back);
	this->addShort( armor.right);
	this->addShort( armor.left);
}
Armor	NetBuffer::getArmor()
{
	Armor armor;
	armor.front = this->getShort();
	armor.back = this->getShort();
	armor.right = this->getShort();
	armor.left = this->getShort();

	return armor;
}

void	NetBuffer::addSerial( ObjSerial serial)
		{
			this->addShort( serial);
		}
ObjSerial	NetBuffer::getSerial()
		{
			return this->getShort();
		}
void	NetBuffer::addFloat( float f)
		{
			int tmpsize = sizeof( f);
			resizeBuffer( offset+tmpsize);
			POSH_WriteU32ToBig( this->buffer+offset, POSH_BigFloatBits( f));
			offset += tmpsize;
		}
float	NetBuffer::getFloat()
		{
			float s;
			checkBuffer( sizeof( s), "getFloat");
			s = POSH_FloatFromBigBits( POSH_ReadU32FromBig( this->buffer+offset));
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addDouble( double d)
		{
			int tmpsize = sizeof( d);
			resizeBuffer( offset+tmpsize);
			POSH_DoubleBits( d, (posh_byte_t *) this->buffer+offset);
			offset += tmpsize;
		}
double	NetBuffer::getDouble()
		{
			double s;
			checkBuffer( sizeof( s), "getDouble");
			s = POSH_DoubleFromBits( (posh_byte_t *) this->buffer+offset);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addShort( unsigned short s)
		{
			int tmpsize = sizeof( s);
			resizeBuffer( offset+tmpsize);
			POSH_WriteU16ToBig( this->buffer+offset, s);
			offset += tmpsize;
		}
unsigned short	NetBuffer::getShort()
		{
			unsigned short s;
			checkBuffer( sizeof( s), "getShort");
			s = POSH_ReadU16FromBig( this->buffer+offset);
			//cerr<<"getShort :: offset="<<offset<<" - length="<<sizeof( s)<<" - value="<<s<<endl;
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addInt32( int i)
		{
			int tmpsize = sizeof( i);
			resizeBuffer( offset+tmpsize);
			POSH_WriteS32ToBig( this->buffer+offset, i);
			offset += tmpsize;
		}
int		NetBuffer::getInt32()
		{
			int s;
			checkBuffer( sizeof( s), "getInt32");
			s = POSH_ReadS32FromBig( this->buffer+offset);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addUInt32( unsigned int i)
		{
			int tmpsize = sizeof( i);
			resizeBuffer( offset+tmpsize);
			POSH_WriteU32ToBig( this->buffer+offset, i);
			offset += tmpsize;
		}
unsigned int	NetBuffer::getUInt32()
		{
			unsigned int s;
			checkBuffer( sizeof( s), "getUInt32");
			s = POSH_ReadU32FromBig( this->buffer+offset);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addChar( char c)
		{
			int tmpsize = sizeof( c);
			resizeBuffer( offset+tmpsize);
			memcpy( buffer+offset, &c, sizeof( c));
			offset += tmpsize;
		}
char	NetBuffer::getChar()
		{
			char c;
			checkBuffer( sizeof( c), "getChar");
			memcpy( &c, buffer+offset, sizeof( c));
			offset+=sizeof(c);
			return c;
		}
void	NetBuffer::addBuffer( unsigned char * buf, int bufsize)
		{
			resizeBuffer( offset+bufsize);
			memcpy( buffer+offset, buf, bufsize);
			offset+=bufsize;
		}
unsigned char* NetBuffer::extAddBuffer( int bufsize)
		{
			resizeBuffer( offset+bufsize);
            unsigned char* retval = (unsigned char*)buffer+offset;
			offset+=bufsize;
            return retval;
		}
unsigned char *	NetBuffer::getBuffer( int offt)
		{
			checkBuffer( offt, "getBuffer");
			unsigned char * tmp = (unsigned char *)buffer + offset;
			offset += offt;
			return tmp;
		}
void	NetBuffer::addBuffer( const unsigned char * buf, int bufsize)
		{
			resizeBuffer( offset+bufsize);
			memcpy( buffer+offset, buf, bufsize);
			offset+=bufsize;
		}
		// Add and get a string with its length before the char * buffer part
void	NetBuffer::addString( string str)
		{
			assert( str.length()<0xFFFF);
			unsigned short length = str.length();
			this->addShort( length);
			resizeBuffer( offset+length);
			memcpy( buffer+offset, str.c_str(), length);
			offset += length;
		}
string	NetBuffer::getString()
		{
			unsigned short s;
			s = this->getShort();
			checkBuffer( s, "getString");
			//cerr<<"getString :: offset="<<offset<<" - length="<<s<<endl;
			char c = buffer[offset+s];
			buffer[offset+s]=0;
			string str( buffer+offset);
			buffer[offset+s]=c;
			offset += s;

			return str;
		}

int		NetBuffer::getDataLength() { return offset;}
int		NetBuffer::getSize() { return size;}
