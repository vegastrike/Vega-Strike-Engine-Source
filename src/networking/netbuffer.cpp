#include <assert.h>
#include "networking/netbuffer.h"
#include "networking/const.h"
#include "endianness.h"
#include "gfxlib_struct.h"

NetBuffer::NetBuffer()
		{
			buffer = new char[MAXBUFFER];
			offset = 0;
			size = MAXBUFFER;
		}
NetBuffer::NetBuffer( int bufsize)
		{
			buffer = new char[bufsize];
			offset = 0;
			size = bufsize;
		}
NetBuffer::NetBuffer( char * buf, int bufsize)
		{
			offset = 0;
			size=bufsize;
			this->buffer = new char[size];
			memcpy( buffer, buf, bufsize);
		}
NetBuffer::NetBuffer( const char * buf, int bufsize)
		{
			offset = 0;
			size=bufsize;
			this->buffer = new char[size];
			memcpy( buffer, buf, bufsize);
		}
NetBuffer::~NetBuffer()
		{
			if( buffer != NULL)
				delete buffer;
		}
void	NetBuffer::Reset()
		{
			memset( buffer, 0, size);
			offset=0;
		}

char *	NetBuffer::getData() { return buffer;}

		// Extends the buffer if we exceed its size
void	NetBuffer::resizeBuffer( int newsize)
		{
			if( size < newsize)
			{
				char * tmp = new char [newsize];
				memcpy( tmp, buffer, newsize);
				delete buffer;
				buffer = tmp;
			}
		}

void	NetBuffer::addClientState( ClientState cs)
		{
			int tmpsize = sizeof( cs);
			resizeBuffer( offset+tmpsize);
			cs.netswap();
			memcpy( buffer+offset, &cs, tmpsize);
			offset += tmpsize;
		}
ClientState NetBuffer::getClientState()
		{
			ClientState cs;
			memcpy( &cs, buffer+offset, sizeof( cs));
			offset += sizeof( cs);
			cs.netswap();
			return cs;
		}

void	NetBuffer::addVector( Vector v)
		{
			int tmpsize = sizeof( v);
			resizeBuffer( offset+tmpsize);
			v.netswap();
			memcpy( buffer+offset, &v, sizeof( v));
			offset += tmpsize;
		}
Vector	NetBuffer::getVector()
		{
			Vector v;
			memcpy( &v, buffer+offset, sizeof( v));
			offset += sizeof( v);
			v.netswap();
			return v;
		}
void	NetBuffer::addQVector( QVector v)
		{
			int tmpsize = sizeof( v);
			resizeBuffer( offset+tmpsize);
			v.netswap();
			memcpy( buffer+offset, &v, sizeof( v));
			offset += tmpsize;
		}
QVector	NetBuffer::getQVector()
		{
			Vector v;
			memcpy( &v, buffer+offset, sizeof( v));
			offset += sizeof( v);
			v.netswap();
			return v;
		}
void	NetBuffer::addColor( GFXColor col)
		{
			int tmpsize = sizeof( col);
			resizeBuffer( offset+tmpsize);
			col.netswap();
			memcpy( buffer+offset, &col, sizeof( col));
			offset += tmpsize;
		}
GFXColor NetBuffer::getColor()
		{
			GFXColor col;
			memcpy( &col, buffer+offset, sizeof( col));
			offset += sizeof( col);
			col.netswap();
			return col;
		}
void	NetBuffer::addMatrix( Matrix m)
		{
			int tmpsize = sizeof( m);
			resizeBuffer( offset+tmpsize);
			m.netswap();
			memcpy( buffer+offset, &m, sizeof( m));
			offset += tmpsize;
		}
Matrix	NetBuffer::getMatrix()
		{
			Matrix m;
			memcpy( &m, buffer+offset, sizeof(m));
			offset += sizeof( m);
			m.netswap();
			return m;
		}
void	NetBuffer::addQuaternion( Quaternion quat)
		{
			int tmpsize = sizeof( quat);
			resizeBuffer( offset+tmpsize);
			quat.netswap();
			memcpy( buffer+offset, &quat, sizeof( quat));
			offset += tmpsize;
		}
Quaternion	NetBuffer::getQuaternion()
		{
			Quaternion q;
			memcpy( &q, buffer+offset, sizeof(q));
			offset += sizeof( q);
			q.netswap();
			return q;
		}
void	NetBuffer::addTransformation( Transformation trans)
		{
			int tmpsize = sizeof( trans);
			resizeBuffer( offset+tmpsize);
			trans.netswap();
			memcpy( buffer+offset, &trans, sizeof( trans));
			offset += tmpsize;
		}
Transformation	NetBuffer::getTransformation()
		{
			Transformation t;
			memcpy( &t, buffer+offset, sizeof(t));
			offset += sizeof( t);
			t.netswap();
			return t;
		}

void	NetBuffer::addShield( SHIELD shield)
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
SHIELD	NetBuffer::getShield()
{
	Unit::Shield shield;
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
void		NetBuffer::addArmor( ARMOR armor)
{
	this->addShort( armor.front);
	this->addShort( armor.back);
	this->addShort( armor.right);
	this->addShort( armor.left);
}
ARMOR	NetBuffer::getArmor()
{
	Unit::Armor armor;
	armor.front = this->getShort();
	armor.back = this->getShort();
	armor.right = this->getShort();
	armor.left = this->getShort();

	return armor;
}

void	NetBuffer::addSerial( ObjSerial serial)
		{
			int tmpsize = sizeof( serial);
			resizeBuffer( offset+tmpsize);
			serial = OBJSERIAL_TONET( serial);
			memcpy( buffer+offset, &serial, sizeof( serial));
			offset += tmpsize;
		}
ObjSerial	NetBuffer::getSerial()
		{
			ObjSerial s;
			memcpy( &s, buffer+offset, sizeof( s));
			offset+=sizeof(s);
			s = OBJSERIAL_TONET( s);
			return s;
		}
void	NetBuffer::addFloat( float f)
		{
			int tmpsize = sizeof( f);
			resizeBuffer( offset+tmpsize);
			f = VSSwapHostFloatToLittle( f);
			memcpy( buffer+offset, &f, sizeof( f));
			offset += tmpsize;
		}
float	NetBuffer::getFloat()
		{
			float s;
			memcpy( &s, buffer+offset, sizeof( s));
			s = VSSwapHostFloatToLittle( s);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addDouble( double d)
		{
			int tmpsize = sizeof( d);
			resizeBuffer( offset+tmpsize);
			d = VSSwapHostDoubleToLittle( d);
			memcpy( buffer+offset, &d, sizeof( d));
			offset += tmpsize;
		}
double	NetBuffer::getDouble()
		{
			double s;
			memcpy( &s, buffer+offset, sizeof( s));
			s = VSSwapHostDoubleToLittle( s);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addShort( unsigned short s)
		{
			int tmpsize = sizeof( s);
			resizeBuffer( offset+tmpsize);
			s = VSSwapHostShortToLittle( s);
			memcpy( buffer+offset, &s, sizeof( s));
			offset += tmpsize;
		}
unsigned short	NetBuffer::getShort()
		{
			unsigned short s;
			memcpy( &s, buffer+offset, sizeof( s));
			offset+=sizeof(s);
			s = VSSwapHostShortToLittle( s);
			return s;
		}
void	NetBuffer::addInt32( int i)
		{
			int tmpsize = sizeof( i);
			resizeBuffer( offset+tmpsize);
			i = VSSwapHostIntToLittle( i);
			memcpy( buffer+offset, &i, sizeof( i));
			offset += tmpsize;
		}
int		NetBuffer::getInt32()
		{
			int s;
			memcpy( &s, buffer+offset, sizeof( s));
			offset+=sizeof(s);
			s = VSSwapHostIntToLittle( s);
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
unsigned char *	NetBuffer::getBuffer( int offt)
		{
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
			unsigned short netlength = 0;
			resizeBuffer( offset+length+sizeof( length));
			netlength = VSSwapHostShortToLittle( length);
			memcpy( buffer+offset, &netlength, sizeof( length));
			offset += sizeof(length);
			memcpy( buffer+offset, str.c_str(), length);
			offset += length;
		}
string	NetBuffer::getString()
		{
			unsigned short s;
			memcpy( &s, buffer+offset, sizeof( s));
			s = VSSwapHostShortToLittle( s);
			offset+=sizeof(s);
			char c = buffer[offset+s];
			buffer[offset+s]=0;
			string str( buffer+offset);
			buffer[offset+s]=c;
			offset += s;

			return str;
		}

int		NetBuffer::getDataLength() { return offset;}
int		NetBuffer::getSize() { return size;}
