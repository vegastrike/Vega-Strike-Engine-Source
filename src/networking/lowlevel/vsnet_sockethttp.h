#ifndef VSNET_SOCKET_HTTP_H
#define VSNET_SOCKET_HTTP_H

#include <config.h>

#include <sys/types.h>

#include "vsnet_socketbase.h"
#include "vsnet_sockettcp.h"

typedef unsigned char u_int8_t;
typedef unsigned int u_int32_t;

/***********************************************************************
 * VsnetTCPSocket - declaration
 ***********************************************************************/
 
class VsnetHTTPSocket : public VsnetTCPSocket
{
    // I would like an entire VsnetTCPSocket, but that requires that data is VS packet format, which HTTP is not.
    std::string _path;
	std::string _hostheader;
	std::map<std::string, std::string> _header;
	std::string _incompleteheader;
	int _incompleteheadersection;
	int _content_length;
	bool _send_more_data; // Has the server allowed keep-alive?
public:
    VsnetHTTPSocket(
                 const AddressIP&   remote_ip,
				 const std::string& host,
                 const std::string& path,
                 SocketSet&         set );
    VsnetHTTPSocket(
                 const std::string& url,
                 SocketSet&         set );
	
	virtual bool lower_selected( int datalen = -1 );
	virtual int lower_sendbuf();

    virtual int  optPayloadSize( ) const { return 500; }

	virtual bool isActive() { return true; }

    void reopenConnection();
	
    virtual void dump( std::ostream& ostr ) const;
};

#endif
