#ifndef __NETBUFFER_H
#define __NETBUFFER_H

#include <string.h>
#include "networking/const.h"
#include "cmd/weapon_xml.h"
#include "endianness.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"

class NetBuffer
{
		char *	buffer;
		int		offset;
		int		size;
	
	public :
		NetBuffer()
		{
			buffer = new char[MAXBUFFER];
			offset = 0;
			size = MAXBUFFER;
		}
		NetBuffer( int bufsize)
		{
			buffer = new char[bufsize];
			offset = 0;
			size = bufsize;
		}
		NetBuffer( char * buf, int bufsize)
		{
			offset = bufsize;
			resizeBuffer( bufsize);
			memcpy( buffer, buf, bufsize);
		}
		~NetBuffer()
		{
			if( buffer != NULL)
				delete buffer;
		}

		char *	getBuffer() { return buffer;}

		// Extends the buffer if we exceed MAXBUFFER
		void	resizeBuffer( int newsize)
		{
			if( size < newsize)
			{
				char * tmp = new char [newsize];
				memcpy( tmp, buffer, newsize);
				delete buffer;
				buffer = tmp;
			}
		}

		void	addClientState( ClientState cs)
		{
			int tmpsize = sizeof( cs);
			resizeBuffer( offset+tmpsize);
			cs.netswap();
			memcpy( buffer+offset, &cs, tmpsize);
			offset += tmpsize;
		}
		ClientState getClientState()
		{
			ClientState cs;
			memcpy( &cs, buffer+offset, sizeof( cs));
			offset += sizeof( cs);
			cs.netswap();
			return cs;
		}

		void	addVector( Vector v)
		{
			int tmpsize = sizeof( v);
			resizeBuffer( offset+tmpsize);
			v.netswap();
			memcpy( buffer+offset, &v, sizeof( v));
			offset += tmpsize;
		}
		Vector	getVector()
		{
			Vector v;
			memcpy( &v, buffer+offset, sizeof( v));
			offset += sizeof( v);
			v.netswap();
			return v;
		}
		void	addMatrix( Matrix m)
		{
			int tmpsize = sizeof( m);
			resizeBuffer( offset+tmpsize);
			m.netswap();
			memcpy( buffer+offset, &m, sizeof( m));
			offset += tmpsize;
		}
		Matrix	getMatrix()
		{
			Matrix m;
			memcpy( &m, buffer+offset, sizeof(m));
			offset += sizeof( m);
			m.netswap();
			return m;
		}
		void	addWeaponInfo( weapon_info wi)
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
		void	addQuaternion( Quaternion quat)
		{
			int tmpsize = sizeof( quat);
			resizeBuffer( offset+tmpsize);
			quat.netswap();
			memcpy( buffer+offset, &quat, sizeof( quat));
			offset += tmpsize;
		}
		Quaternion	getQuaternion()
		{
			Quaternion q;
			memcpy( &q, buffer+offset, sizeof(q));
			offset += sizeof( q);
			q.netswap();
			return q;
		}
		void	addTransformation( Transformation trans)
		{
			int tmpsize = sizeof( trans);
			resizeBuffer( offset+tmpsize);
			trans.netswap();
			memcpy( buffer+offset, &trans, sizeof( trans));
			offset += tmpsize;
		}
		Transformation	getTransformation()
		{
			Transformation t;
			memcpy( &t, buffer+offset, sizeof(t));
			offset += sizeof( t);
			t.netswap();
			return t;
		}

		void	addSerial( ObjSerial serial)
		{
			int tmpsize = sizeof( serial);
			resizeBuffer( offset+tmpsize);
			serial = OBJSERIAL_TONET( serial);
			memcpy( buffer+offset, &serial, sizeof( serial));
			offset += tmpsize;
		}
		ObjSerial	getSerial()
		{
			ObjSerial s;
			memcpy( &s, buffer+offset, sizeof( s));
			offset+=sizeof(s);
			s = OBJSERIAL_TONET( s);
			return s;
		}
		void	addFloat( float f)
		{
			int tmpsize = sizeof( f);
			resizeBuffer( offset+tmpsize);
			f = VSSwapHostFloatToLittle( f);
			memcpy( buffer+offset, &f, sizeof( f));
			offset += tmpsize;
		}
		float	getFloat()
		{
			float s;
			memcpy( &s, buffer+offset, sizeof( s));
			s = VSSwapHostFloatToLittle( s);
			offset+=sizeof(s);
			return s;
		}
		void	addDouble( double d)
		{
			int tmpsize = sizeof( d);
			resizeBuffer( offset+tmpsize);
			d = VSSwapHostDoubleToLittle( d);
			memcpy( buffer+offset, &d, sizeof( d));
			offset += tmpsize;
		}
		double	getDouble()
		{
			double s;
			memcpy( &s, buffer+offset, sizeof( s));
			s = VSSwapHostDoubleToLittle( s);
			offset+=sizeof(s);
			return s;
		}
		void	addShort( unsigned short s)
		{
			int tmpsize = sizeof( s);
			resizeBuffer( offset+tmpsize);
			s = VSSwapHostShortToLittle( s);
			memcpy( buffer+offset, &s, sizeof( s));
			offset += tmpsize;
		}
		unsigned short	getShort()
		{
			unsigned short s;
			memcpy( &s, buffer+offset, sizeof( s));
			offset+=sizeof(s);
			s = VSSwapHostShortToLittle( s);
			return s;
		}
		void	addInt32( int i)
		{
			int tmpsize = sizeof( i);
			resizeBuffer( offset+tmpsize);
			i = VSSwapHostIntToLittle( i);
			memcpy( buffer+offset, &i, sizeof( i));
			offset += tmpsize;
		}
		int		getInt32()
		{
			int s;
			memcpy( &s, buffer+offset, sizeof( s));
			offset+=sizeof(s);
			s = VSSwapHostIntToLittle( s);
			return s;
		}

		int		getDataSize() { return offset;}
		int		getSize() { return size;}
};

#endif
