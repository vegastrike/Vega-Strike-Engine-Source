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
 
class VsnetHTTPSocket : public VsnetSocketBase //VsnetTCPSocket
{
    // I would like an entire VsnetTCPSocket, but that requires that data is VS packet format, which HTTP is not.
  std::string _path;
  std::string _hostheader;
  std::map<std::string, std::string> _header;
  std::string _incompleteheader;
  int _incompleteheadersection;
  int _content_length;
  bool _send_more_data; // Has the server allowed keep-alive?
  
  std::list<std::string> dataToSend;
  int sendDataPos;
  std::string dataToReceive;
  AddressIP _remote_ip;
  std::string waitingToReceive;
  int numRetries;
  bool readHeader;
  bool parseHeaderByte( char rcvchr );
  bool ischunked;
  int timeToNextRequest; // Wait a second between requests.
  bool readingchunked;
  int chunkedlen;
  char chunkedchar;
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
	virtual void lower_clean_sendbuf();
    friend std::ostream& operator<<( std::ostream& ostr, const VsnetHTTPSocket& s ); 

	bool sendstr(const std::string &data);
	bool recvstr(std::string &data);
    virtual bool need_test_writable( );
    virtual bool write_on_negative();
    virtual int  optPayloadSize( ) const { return 500; }

	virtual bool isActive();
    void resendData();
    void reopenConnection();
    virtual bool isReadyToSend(fd_set*);
    virtual void dump( std::ostream& ostr ) const;
};

std::ostream& operator<<( std::ostream& ostr, const VsnetHTTPSocket& s );
#endif
