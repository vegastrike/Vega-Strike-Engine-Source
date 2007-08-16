#include <assert.h>
#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/vsnet_oss.h"
#include "networking/const.h"
#include "posh.h"
#include "gfxlib_struct.h"
std::string getSimpleString(std::string &input){
  int len=getSimpleInt(input);
  if (len>=0&&(unsigned int)len<=input.length()){
    std::string retval=input.substr(0,len);
    input=input.substr(len);
    return retval;
  }
  return "";
}
char getSimpleChar(std::string &input){
  char retval=input[0];
  input=input.substr(1);
  return retval;
}
int getSimpleInt(std::string &input){
  std::string::size_type where=input.find(" ");
  if (where!=string::npos) {
    std::string num=input.substr(0,where);
    int len=XMLSupport::parse_int(num);
    input=input.substr(where+1);
    return len;
  }
  return 0;  
}
void addSimpleString(std::string &input, const std::string adder){
  addSimpleInt(input,adder.length());
  input+=adder;
}
void addSimpleChar(std::string &input, const char adder){
  char add[2]={adder,'\0'};
  input+=std::string(add,1);
}
void addSimpleInt(std::string &input, const int adder){
  input+=XMLSupport::tostring(adder)+" ";
}

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

/** If there is a platform where b in the call VsnetOSS::memcpy(a,b,c) must be char*
 *  instead of const char* or const void*, that systems header files are
 *  badly broken. That requirement must be a lie.
 *  If you have that, implement an ifdef in VsnetOSS::VsnetOSS::memcpy
 */
NetBuffer::NetBuffer( const char * buf, int bufsize)
		{
			offset = 0;
			size=bufsize+1;
			this->buffer = new char[size];
			memset( buffer, 0x20, size);
			VsnetOSS::memcpy( buffer, buf, bufsize);
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
void	NetBuffer::resizeBuffer( unsigned int newsize)
		{
			if( size-1 < newsize)
			{
				char * tmp = new char [newsize+1];
				memset( tmp, 0, newsize+1);
				VsnetOSS::memcpy( tmp, buffer, size);
				delete []buffer;
				buffer = tmp;
				size = newsize+1;
			}
		}
// Check the buffer to see if we can still get info from it
bool	NetBuffer::checkBuffer( int len, const char * fun)
{
	if( offset+len >= size)
	{
		std::cerr<<"!!! ERROR : trying to read more data than buffer size (offset="<<offset<<" - size="<<size<<" - to read="<<len<<") in "<<fun<<" !!!"<<std::endl;

		// NETFIXME: Be more graceful... but this is nice for Debugging in GDB.
		int *x=NULL;
		(const_cast<char*>(fun))[0]+=*x;
		
		return false;
	}
	return true;
}

enum NBType {
	NB_CHAR=187,
	NB_SHORT,
	NB_SERIAL,
	NB_INT32,
	NB_UINT32,
	NB_FLOAT=123,
	NB_DOUBLE,
	NB_STRING=33,
	NB_BUFFER=44,
	NB_CLIENTSTATE=211,
	NB_TRANSFORMATION,
	NB_VECTOR,
	NB_QVECTOR,
	NB_COLOR,
	NB_MATRIX,
	NB_QUATERNION,
	NB_SHIELD,
	NB_ARMOR,
	NB_GFXMAT,
	NB_GFXLIGHT,
	NB_GFXLIGHTLOCAL,
};
#define NB_CASE(a)  case a: return #a; break;
char typeerrbuf[32];
const char *getTypeStr( unsigned char c ) {
	switch (c) {
		NB_CASE(NB_CHAR);
		NB_CASE(NB_SHORT);
		NB_CASE(NB_SERIAL);
		NB_CASE(NB_INT32);
		NB_CASE(NB_UINT32);
		NB_CASE(NB_FLOAT);
		NB_CASE(NB_DOUBLE);
		NB_CASE(NB_STRING);
		NB_CASE(NB_BUFFER);
		NB_CASE(NB_CLIENTSTATE);
		NB_CASE(NB_TRANSFORMATION);
		NB_CASE(NB_VECTOR);
		NB_CASE(NB_QVECTOR);
		NB_CASE(NB_COLOR);
		NB_CASE(NB_MATRIX);
		NB_CASE(NB_QUATERNION);
		NB_CASE(NB_SHIELD);
		NB_CASE(NB_ARMOR);
		NB_CASE(NB_GFXMAT);
		NB_CASE(NB_GFXLIGHT);
		NB_CASE(NB_GFXLIGHTLOCAL);
	default:
		sprintf(typeerrbuf, "<<Byte %d>>", (int)c);
		return typeerrbuf;
	}
}

void	NetBuffer::addType( unsigned char c)
		{
			int tmpsize = sizeof( c);
			resizeBuffer( offset+tmpsize);
			VsnetOSS::memcpy( buffer+offset, &c, sizeof( c));
			offset += tmpsize;
		}
unsigned char	NetBuffer::getType()
		{
			unsigned char c;
			if (!checkBuffer( sizeof( c), "getChar"))
				return 0;
			VsnetOSS::memcpy( &c, buffer+offset, sizeof( c));
			offset+=sizeof(c);
			return c;
		}

bool NetBuffer::checkType(unsigned char c) {
	const char *typ = getTypeStr(c);
	checkBuffer(1, typ); // Does this type exist?
	unsigned char got = getType();

	if (got != c) {
		const char *typgot = getTypeStr(got);
		std::cerr<<"!!! ERROR : attempt to read invalid data at offset="<<offset<<": Actual type is a "<<typgot<<" but I wanted a "<<typ<<" !!!"<<std::endl;

		// NETFIXME: Be more graceful... but this is nice for Debugging in GDB.
		int *x=NULL;
		(const_cast<char*>(typ))[0]+=*x;
		
		return false;
	}
	return true;
}

#define NETBUF_DEBUG_CLISERV

#ifdef NETBUF_DEBUG_CLISERV
// Must be enabled or disabled on ALL CLIENTS and ALL SERVERS
// Since packets will be incompatible.
#define ADD_NB(type) addType(type)
#define CHECK_NB(type) checkType(type)
#else
#define ADD_NB(type)
#define CHECK_NB(type)
#endif
	

// NOTE : IMPORTANT - I ONLY INCREMENT OFFSET IN PRIMARY DATATYPES SINCE ALL OTHER ARE COMPOSED WITH THEM
void	NetBuffer::addClientState( ClientState cs)
{
	ADD_NB(NB_CLIENTSTATE);
    //this->addFloat( cs.delay);
    this->addSerial( cs.client_serial);
    this->addTransformation( cs.pos);
    this->addVector( cs.veloc);
    this->addVector( cs.angveloc);
}

ClientState NetBuffer::getClientState()
{
	CHECK_NB(NB_CLIENTSTATE);
    ClientState cs;
    //cs.delay = this->getFloat();
    cs.client_serial = this->getSerial();
    cs.pos = this->getTransformation();
    cs.veloc = this->getVector();
    cs.angveloc = this->getVector();

    return cs;
}

void	NetBuffer::addVector( Vector v)
		{
			ADD_NB(NB_VECTOR);
			this->addFloat( v.i);
			this->addFloat( v.j);
			this->addFloat( v.k);
		}
Vector	NetBuffer::getVector()
		{
			CHECK_NB(NB_VECTOR);
			Vector v;
			v.i = this->getFloat();
			v.j = this->getFloat();
			v.k = this->getFloat();

			return v;
		}
void	NetBuffer::addQVector( QVector v)
		{
			ADD_NB(NB_QVECTOR);
			this->addDouble( v.i);
			this->addDouble( v.j);
			this->addDouble( v.k);
		}
QVector	NetBuffer::getQVector()
		{
			CHECK_NB(NB_QVECTOR);
			QVector v;
			v.i = this->getDouble();
			v.j = this->getDouble();
			v.k = this->getDouble();

			return v;
		}
void	NetBuffer::addColor( GFXColor col)
		{
			ADD_NB(NB_COLOR);
			this->addFloat( col.r);
			this->addFloat( col.g);
			this->addFloat( col.b);
			this->addFloat( col.a);
		}
GFXColor NetBuffer::getColor()
		{
			CHECK_NB(NB_COLOR);
			GFXColor col;
			col.r = this->getFloat();
			col.g = this->getFloat();
			col.b = this->getFloat();
			col.a = this->getFloat();

			return col;
		}
void	NetBuffer::addMatrix( Matrix m)
		{
			ADD_NB(NB_MATRIX);
			for( int i=0; i<9; i++)
				this->addFloat( m.r[i]);
			this->addQVector( m.p);
		}
Matrix	NetBuffer::getMatrix()
		{
			CHECK_NB(NB_MATRIX);
			Matrix m;
			for( int i=0; i<9; i++)
				m.r[i] = this->getFloat();
			m.p = this->getQVector();

			return m;
		}
void	NetBuffer::addQuaternion( Quaternion quat)
		{
			ADD_NB(NB_QUATERNION);
			this->addFloat( quat.s);
			this->addVector( quat.v);
		}
Quaternion	NetBuffer::getQuaternion()
		{
			CHECK_NB(NB_QUATERNION);
			Quaternion q;

			q.s = this->getFloat();
			q.v = this->getVector();

			return q;
		}
void	NetBuffer::addTransformation( Transformation trans)
		{
			ADD_NB(NB_TRANSFORMATION);
			this->addQuaternion( trans.orientation);
			this->addQVector( trans.position);
		}
Transformation	NetBuffer::getTransformation()
		{
			CHECK_NB(NB_TRANSFORMATION);
			Transformation t;
			t.orientation = this->getQuaternion();
			t.position = this->getQVector();

			return t;
		}

void	NetBuffer::addShield( const Shield &shield)
{
	ADD_NB(NB_SHIELD);
	this->addChar( shield.number);
	this->addChar( shield.leak);
	this->addFloat( shield.recharge);
	this->addFloat( shield.efficiency );
	switch( shield.number)
	{
		case 2 :
			this->addFloat( shield.shield2fb.front);	
			this->addFloat( shield.shield2fb.back);	
			this->addFloat( shield.shield2fb.frontmax);	
			this->addFloat( shield.shield2fb.backmax);	
		break;
		case 4 :
			this->addFloat( shield.shield4fbrl.front);
			this->addFloat( shield.shield4fbrl.back);
			this->addFloat( shield.shield4fbrl.right);
			this->addFloat( shield.shield4fbrl.left);
			this->addFloat( shield.shield4fbrl.frontmax);
			this->addFloat( shield.shield4fbrl.backmax);
			this->addFloat( shield.shield4fbrl.rightmax);
			this->addFloat( shield.shield4fbrl.leftmax);
		break;
		case 8 :
			this->addFloat( shield.shield8.frontrighttop);
			this->addFloat( shield.shield8.backrighttop);
			this->addFloat( shield.shield8.frontlefttop);
			this->addFloat( shield.shield8.backlefttop);
			this->addFloat( shield.shield8.frontrightbottom);
			this->addFloat( shield.shield8.backrightbottom);
			this->addFloat( shield.shield8.frontleftbottom);
			this->addFloat( shield.shield8.backleftbottom);
			this->addFloat( shield.shield8.frontrighttopmax);
			this->addFloat( shield.shield8.backrighttopmax);
			this->addFloat( shield.shield8.frontlefttopmax);
			this->addFloat( shield.shield8.backlefttopmax);
			this->addFloat( shield.shield8.frontrightbottommax);
			this->addFloat( shield.shield8.backrightbottommax);
			this->addFloat( shield.shield8.frontleftbottommax);
			this->addFloat( shield.shield8.backleftbottommax);
		break;
	}
}
Shield	NetBuffer::getShield()
{
	CHECK_NB(NB_SHIELD);
	Shield shield;
	shield.number = this->getChar();
	shield.leak = this->getChar();
	shield.recharge = this->getFloat();
	shield.efficiency = this->getFloat();
	switch( shield.number)
	{
		case 2 :
			shield.shield2fb.front=this->getFloat();
			shield.shield2fb.back=this->getFloat();
			shield.shield2fb.frontmax=this->getFloat();
			shield.shield2fb.backmax=this->getFloat();
		break;
		case 4 :
			shield.shield4fbrl.front = this->getFloat();
			shield.shield4fbrl.back = this->getFloat();
			shield.shield4fbrl.right = this->getFloat();
			shield.shield4fbrl.left = this->getFloat();
			shield.shield4fbrl.frontmax = this->getFloat();
			shield.shield4fbrl.backmax = this->getFloat();
			shield.shield4fbrl.rightmax = this->getFloat();
			shield.shield4fbrl.leftmax = this->getFloat();
		break;
		case 8 :
			shield.shield8.frontrighttop=this->getFloat();
			shield.shield8.backrighttop=this->getFloat();
			shield.shield8.frontlefttop=this->getFloat();
			shield.shield8.backlefttop=this->getFloat();
			shield.shield8.frontrightbottom=this->getFloat();
			shield.shield8.backrightbottom=this->getFloat();
			shield.shield8.frontleftbottom=this->getFloat();
			shield.shield8.backleftbottom=this->getFloat();
			shield.shield8.frontrighttopmax=this->getFloat();
			shield.shield8.backrighttopmax=this->getFloat();
			shield.shield8.frontlefttopmax=this->getFloat();
			shield.shield8.backlefttopmax=this->getFloat();
			shield.shield8.frontrightbottommax=this->getFloat();
			shield.shield8.backrightbottommax=this->getFloat();
			shield.shield8.frontleftbottommax=this->getFloat();
			shield.shield8.backleftbottommax=this->getFloat();
		break;
	}

	return shield;
}
void		NetBuffer::addArmor( const Armor &armor)
{
	ADD_NB(NB_ARMOR);
	this->addFloat ( armor.frontrighttop);
	this->addFloat( armor.backrighttop);
	this->addFloat( armor.frontlefttop);
	this->addFloat( armor.backlefttop);
	this->addFloat ( armor.frontrightbottom);
	this->addFloat( armor.backrightbottom);
	this->addFloat( armor.frontleftbottom);
	this->addFloat( armor.backleftbottom);
}
Armor	NetBuffer::getArmor()
{
	CHECK_NB(NB_ARMOR);
	Armor armor;
	armor.frontrighttop = this->getFloat();
	armor.backrighttop = this->getFloat();
	armor.frontlefttop = this->getFloat();
	armor.backlefttop = this->getFloat();
	armor.frontrightbottom = this->getFloat();
	armor.backrightbottom = this->getFloat();
	armor.frontleftbottom = this->getFloat();
	armor.backleftbottom = this->getFloat();

	return armor;
}

void	NetBuffer::addSerial( ObjSerial serial)
		{
			ADD_NB(NB_SERIAL);
			this->addShort( serial);
		}
ObjSerial	NetBuffer::getSerial()
		{
			CHECK_NB(NB_SERIAL);
			return this->getShort();
		}
void	NetBuffer::addFloat( float f)
		{
			ADD_NB(NB_FLOAT);
			int tmpsize = sizeof( f);
			resizeBuffer( offset+tmpsize);
			posh_u32_t bits = POSH_BigFloatBits( f );
			*((posh_u32_t*)(this->buffer+offset)) = bits;
			offset += tmpsize;
		}
float	NetBuffer::getFloat()
		{
			CHECK_NB(NB_FLOAT);
			float s;
			if (!checkBuffer( sizeof( s), "getFloat"))
				return 0;
			posh_u32_t bits = *((posh_u32_t*)(this->buffer+offset));
			s = POSH_FloatFromBigBits( bits );
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addDouble( double d)
		{
			ADD_NB(NB_DOUBLE);
			int tmpsize = sizeof( d);
			resizeBuffer( offset+tmpsize);
			POSH_DoubleBits( d, (posh_byte_t *) this->buffer+offset);
			offset += tmpsize;
		}
double	NetBuffer::getDouble()
		{
			CHECK_NB(NB_DOUBLE);
			double s;
			if (!checkBuffer( sizeof( s), "getDouble"))
				return 0;
			s = POSH_DoubleFromBits( (posh_byte_t *) this->buffer+offset);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addShort( unsigned short s)
		{
			ADD_NB(NB_SHORT);
			int tmpsize = sizeof( s);
			resizeBuffer( offset+tmpsize);
			POSH_WriteU16ToBig( this->buffer+offset, s);
			offset += tmpsize;
		}
unsigned short	NetBuffer::getShort()
		{
			CHECK_NB(NB_SHORT);
			unsigned short s;
			if (!checkBuffer( sizeof( s), "getShort"))
				return 0;
			s = POSH_ReadU16FromBig( this->buffer+offset);
			//cerr<<"getShort :: offset="<<offset<<" - length="<<sizeof( s)<<" - value="<<s<<endl;
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addInt32( int i)
		{
			ADD_NB(NB_INT32);
			int tmpsize = sizeof( i);
			resizeBuffer( offset+tmpsize);
			POSH_WriteS32ToBig( this->buffer+offset, i);
			offset += tmpsize;
		}
int		NetBuffer::getInt32()
		{
			CHECK_NB(NB_INT32);
			int s;
			if (!checkBuffer( sizeof( s), "getInt32"))
				return 0;
			s = POSH_ReadS32FromBig( this->buffer+offset);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addUInt32( unsigned int i)
		{
			ADD_NB(NB_UINT32);
			int tmpsize = sizeof( i);
			resizeBuffer( offset+tmpsize);
			POSH_WriteU32ToBig( this->buffer+offset, i);
			offset += tmpsize;
		}
unsigned int	NetBuffer::getUInt32()
		{
			CHECK_NB(NB_UINT32);
			unsigned int s;
			if (!checkBuffer( sizeof( s), "getUInt32"))
				return 0;
			s = POSH_ReadU32FromBig( this->buffer+offset);
			offset+=sizeof(s);
			return s;
		}
void	NetBuffer::addChar( char c)
		{
			ADD_NB(NB_CHAR);
			int tmpsize = sizeof( c);
			resizeBuffer( offset+tmpsize);
			VsnetOSS::memcpy( buffer+offset, &c, sizeof( c));
			offset += tmpsize;
		}
char	NetBuffer::getChar()
		{
			CHECK_NB(NB_CHAR);
			char c;
			if (!checkBuffer( sizeof( c), "getChar"))
				return 0;
			VsnetOSS::memcpy( &c, buffer+offset, sizeof( c));
			offset+=sizeof(c);
			return c;
		}
void	NetBuffer::addBuffer( const unsigned char * buf, int bufsize)
		{
			ADD_NB(NB_BUFFER);
			resizeBuffer( offset+bufsize);
			VsnetOSS::memcpy( buffer+offset, buf, bufsize);
			offset+=bufsize;
		}
unsigned char* NetBuffer::extAddBuffer( int bufsize )
		{
			ADD_NB(NB_BUFFER);
			resizeBuffer( offset+bufsize);
            unsigned char* retval = (unsigned char*)buffer+offset;
			offset+=bufsize;
            return retval;
		}

static unsigned char null = '\0';

unsigned char *	NetBuffer::getBuffer( int offt)
		{
			CHECK_NB(NB_BUFFER);
			if (!checkBuffer( offt, "getBuffer"))
				return &null;
			unsigned char * tmp = (unsigned char *)buffer + offset;
			offset += offt;
			return tmp;
		}
		// Add and get a string with its length before the char * buffer part
void	NetBuffer::addString( const string& str)
{
	ADD_NB(NB_STRING);
	unsigned int len = str.length();
    if( len < 0xffff )
	{
        unsigned short length = len;
        this->addShort( length );
        resizeBuffer( offset+length );
        VsnetOSS::memcpy( buffer+offset, str.c_str(), length );
        offset += length;
	}
	else
	{
		assert( len < 0xffffffff );
        this->addShort( 0xffff );
		this->addInt32( len );
        resizeBuffer( offset+len );
        VsnetOSS::memcpy( buffer+offset, str.c_str(), len );
        offset += len;
	}
}

string	NetBuffer::getString()
{
	CHECK_NB(NB_STRING);
	unsigned short s;
	s = this->getShort();
	if( s != 0xffff )
	{
		if (!checkBuffer( s, "getString"))
			return std::string();
		char c = buffer[offset+s];
		buffer[offset+s]=0;
	    string str( buffer+offset);
	    buffer[offset+s]=c;
	    offset += s;

	    return str;
	}
	else
	{
		// NETFIXME: Possible DOS attack here...
		unsigned int len = this->getInt32();
		if (!checkBuffer( len, "getString"))
			return std::string();
		char c = buffer[offset+len];
		buffer[offset+len]=0;
	    string str( buffer+offset);
	    buffer[offset+len]=c;
	    offset += len;

	    return str;
	}
}

GFXMaterial		NetBuffer::getGFXMaterial()
{
	CHECK_NB(NB_GFXMAT);
	GFXMaterial mat;

	mat.ar = this->getFloat();
	mat.ag = this->getFloat();
	mat.ab = this->getFloat();
	mat.aa = this->getFloat();

	mat.dr = this->getFloat();
	mat.dg = this->getFloat();
	mat.db = this->getFloat();
	mat.da = this->getFloat();

	mat.sr = this->getFloat();
	mat.sg = this->getFloat();
	mat.sb = this->getFloat();
	mat.sa = this->getFloat();

	mat.er = this->getFloat();
	mat.eg = this->getFloat();
	mat.eb = this->getFloat();
	mat.ea = this->getFloat();

	mat.power = this->getFloat();

	return mat;
}

void	NetBuffer::addGFXMaterial( const GFXMaterial & mat)
{
	ADD_NB(NB_GFXMAT);
	this->addFloat( mat.ar);
	this->addFloat( mat.ag);
	this->addFloat( mat.ab);
	this->addFloat( mat.aa);

	this->addFloat( mat.dr);
	this->addFloat( mat.dg);
	this->addFloat( mat.db);
	this->addFloat( mat.da);

	this->addFloat( mat.sr);
	this->addFloat( mat.sg);
	this->addFloat( mat.sb);
	this->addFloat( mat.sa);

	this->addFloat( mat.er);
	this->addFloat( mat.eg);
	this->addFloat( mat.eb);
	this->addFloat( mat.ea);

	this->addFloat( mat.power);
}

GFXLight		NetBuffer::getGFXLight()
{
	CHECK_NB(NB_GFXLIGHT);
	GFXLight	light;
	int i=0;
	light.target = this->getInt32();
	for( i=0; i<3; i++)
		light.vect[i] = this->getFloat();
	light.options = this->getInt32();
	for( i=0; i<4; i++)
		light.diffuse[i] = this->getFloat();
	for( i=0; i<4; i++)
		light.specular[i] = this->getFloat();
	for( i=0; i<4; i++)
		light.ambient[i] = this->getFloat();
	for( i=0; i<3; i++)
		light.attenuate[i] = this->getFloat();
	for( i=0; i<3; i++)
		light.direction[i] = this->getFloat();
	light.exp = this->getFloat();
	light.cutoff = this->getFloat();

	return light;
}

void	NetBuffer::addGFXLight( const GFXLight & light)
{
	ADD_NB(NB_GFXLIGHT);
	int i=0;
	this->addInt32( light.target);
	for( i=0; i<3; i++)
		this->addFloat( light.vect[i]);
	this->addInt32( light.options);
	for( i=0; i<4; i++)
		this->addFloat( light.diffuse[i]);
	for( i=0; i<4; i++)
		this->addFloat( light.specular[i]);
	for( i=0; i<4; i++)
		this->addFloat( light.ambient[i]);
	for( i=0; i<3; i++)
		this->addFloat( light.attenuate[i]);
	for( i=0; i<3; i++)
		this->addFloat( light.direction[i]);
	this->addFloat( light.exp);
	this->addFloat( light.cutoff);
}

GFXLightLocal	NetBuffer::getGFXLightLocal()
{
	CHECK_NB(NB_GFXLIGHTLOCAL);
	GFXLightLocal light;

	light.ligh = this->getGFXLight();
	light.islocal = this->getChar();

	return light;
}

void	NetBuffer::addGFXLightLocal( const GFXLightLocal & light)
{
	ADD_NB(NB_GFXLIGHTLOCAL);
	this->addGFXLight( light.ligh);
	this->addChar( light.islocal);
}

unsigned int		NetBuffer::getDataLength() { return offset;}
unsigned int		NetBuffer::getSize() { return size;}
