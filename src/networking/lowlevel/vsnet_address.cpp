#include <config.h>

#include "vsnet_address.h"

std::ostream& operator<<( std::ostream& ostr, const AddressIP& adr )
{
    ostr << inet_ntoa(adr.sin_addr) << ":" << ntohs(adr.sin_port);
    return ostr;
}

void AddressIP::dump( )
{
    std::cout << *this << std::endl;
}

const char* AddressIP::ipadr( ) const
{
    return inet_ntoa( sin_addr);
}

unsigned long AddressIP::inaddr( ) const
{
    return sin_addr.s_addr;
}

bool operator==( const AddressIP& l, const AddressIP& r )
{
    return !memcmp( (sockaddr_in*)&l, (sockaddr_in*)&r, sizeof(sockaddr_in) );
}

bool operator!=( const AddressIP& l, const AddressIP& r )
{
    return memcmp( (sockaddr_in*)&l, (sockaddr_in*)&r, sizeof(sockaddr_in) );
}

bool operator<( const AddressIP& l, const AddressIP& r )
{
    return ( l.inaddr() < r.inaddr() );
}

