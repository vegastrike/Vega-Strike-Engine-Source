#include "networking/netbuffer.h"
#include "networking/const.h"
#include "endianness.h"


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
			size=bufsize;
			this->buffer = new char[size];
			memcpy( buffer, buf, bufsize);
		}
NetBuffer::NetBuffer( const char * buf, int bufsize)
		{
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
void	NetBuffer::addBuffer( char * buf, int bufsize)
		{
			resizeBuffer( offset+bufsize);
			memcpy( buffer+offset, buf, bufsize);
			offset+=bufsize;
		}
char *	NetBuffer::getBuffer( int offt)
		{
			char * tmp = buffer+offset;
			offset += offt;
			return tmp;
		}
void	NetBuffer::addBuffer( const char * buf, int bufsize)
		{
			resizeBuffer( offset+bufsize);
			memcpy( buffer+offset, buf, bufsize);
			offset+=bufsize;
		}
		// Add and get a string with its length before the char * buffer part
void	NetBuffer::addString( string str)
		{
			int length = str.length();
			int netlength = 0;
			resizeBuffer( offset+length+sizeof( int));
			netlength = VSSwapHostIntToLittle( length);
			memcpy( buffer+offset, &netlength, sizeof( int));
			offset += sizeof(int);
			memcpy( buffer+offset, str.c_str(), length);
			offset += length;
		}
string	NetBuffer::getString()
		{
			int s;
			memcpy( &s, buffer+offset, sizeof( s));
			s = VSSwapHostIntToLittle( s);
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
