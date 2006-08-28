#include "vsnet_socket.h"
#include "vsnet_sockethttp.h"
#include "vsnet_oss.h"
#include "vsnet_err.h"

#include "packet.h"

#include "netui.h"


static int httpnum = -1;

VsnetHTTPSocket::VsnetHTTPSocket(
                 const AddressIP& remote_ip,
				 const std::string&     host,
                 const std::string&     path,
                 SocketSet&       set )
		: VsnetTCPSocket(httpnum--, remote_ip, set), _path(path), _hostheader(host),
		  _incompleteheadersection(0), _content_length(0), _send_more_data(false)

		{
	this->_fd = -1;
}

/*
  NETFIXME: We don't allow https: URLs.
  Doing so would involve including all the OpenSSL libraries and
  figuring out how to use them.
*/

static void hostFromURI(const std::string& uri, std::string &host, std::string &fullhost, std::string &path, unsigned short &port) {
	std::string protocol = uri.substr(0, 7);
	std::string::size_type pos = 0;
	if (protocol == "http://") {
		pos = 7;
	}
	std::string::size_type endhost = uri.find(':', pos);
	std::string::size_type endhostport = uri.find('/', pos);
	if (endhost==std::string::npos || endhost>=endhostport) {
		endhost = endhostport;
	}
	host = uri.substr(pos, endhost-pos);
	fullhost = uri.substr(pos, endhostport-pos);
	port = atoi(uri.substr(endhost+1, endhostport-endhost-1).c_str());
	if (!port)
		port = 80;
	path = uri.substr(endhostport);
	if (path.empty())
		path = "/";
}

static AddressIP remoteIPFromURI(const std::string& uri) {
	std::string dummy1, dummy2;
	unsigned short port;
	
	std::string host;
	hostFromURI(uri, host, dummy1, dummy2, port);
	if (!port)
		port = 80;
	return NetUIBase::lookupHost ( host.c_str(), port );
}

VsnetHTTPSocket::VsnetHTTPSocket(
                 const std::string& uri,
                 SocketSet&       set )
		: VsnetTCPSocket(httpnum--, remoteIPFromURI(uri), set),
		  _incompleteheadersection(0), _content_length(0), _send_more_data(false)
		{
	std::string dummy;
    unsigned short port;
	hostFromURI(uri, dummy, this->_hostheader, this->_path, port);
	this->_fd = -1;

}

void VsnetHTTPSocket::reopenConnection() {
	this->_fd = NetUIBase::createClientSocket(_remote_ip, true);
}

void VsnetHTTPSocket::dump( std::ostream& ostr ) const
{
	VsnetTCPSocket::dump( ostr );
    ostr << "URI: http://" << _hostheader << _path;
}

int VsnetHTTPSocket::lower_sendbuf(  )
{
	if (!(this->_fd == -1 || _send_more_data || _content_length ||
		  _incomplete_header || _header.size())) {
		this->close_fd();
	}
	if ( this->_fd == -1 ) {
		reopenConnection();
	}
	
	_sq_mx.lock( );
	if( _sq_current.empty() && !_sq.empty() )
	{
		PacketPtr m = _sq.pop( );
		int len = m->getSendBufferLength();
		Header h( len );
		std::string httpHeader;
		char endHeaderLen[50];
		sprintf(endHeaderLen, "Content-Length: %d\r\n\r\n", (sizeof(Header)+len) );
		httpHeader = "POST " + this->_path + " HTTP/1.1\r\n"
			"Host: " + this->_hostheader + "\r\n"
			"User-Agent: Vsnet/1.0\r\n"
			"Connection: keep-alive\r\n"
			"Accept: message/x-vsnet-packet\r\n"
			"Keep-Alive: 300\r\n"
			"Content-Type: message/x-vsnet-packet\r\n" +
			endHeaderLen;
		char *data = new char[ httpHeader.length() + sizeof(Header) + len + 1 ];
		memcpy(data, httpHeader.c_str(), httpHeader.length());
		memcpy(data + httpHeader.length(), (&h), sizeof(Header));
		memcpy(data + httpHeader.length() + sizeof(Header), m->getSendBuffer(), len);
		// We want it in one sendbuf.
		_sq_current.push( PacketMem( data, httpHeader.length() + sizeof(Header) + len ) );
		delete[] data;
		_sq_off = 0;
	}
	_sq_mx.unlock( );
	int ret = VsnetTCPSocket::lower_sendbuf( );
	if (ret == 0) {
		// What!?! A HTTP server decided to close the connection? The horror...
		reopenConnection();
		VsnetTCPSocket::lower_sendbuf( );
	}
	return ret;
}

bool VsnetHTTPSocket::lower_selected( int datalen )
{
	if ( this->_fd == -1 || _connection_closed ) {
		return false;
	}

	if (_content_length == 0) {

		while (datalen!=0) {
			char rcvchr=0;
			int ret = VsnetOSS::recv( get_fd(), &rcvchr, 1, 0 );
			if (ret==0) {
				// closed;
				_fd = -1;
				return false;
			}
			if (ret<0) {
				if (vsnetEWouldBlock()) {
					datalen = 0;
					return false;
				}
				// errored;
				_fd = -1;
				return false;
			}
			if (datalen>0)
				datalen -= ret;
			if (rcvchr=='\r' && _incompleteheadersection==0) {
				_incompleteheadersection++;
				continue;
			}
			if (rcvchr=='\n' && _incompleteheadersection==1) {
				if (_incompleteheader.empty()) {
					_incompleteheadersection = 0;
					break;
				} else {
					_incompleteheadersection++;
					continue;
				}
			}
			if (_incompleteheadersection==2) {
				_incompleteheadersection=0;
				if (!isspace(rcvchr)) {
					if (_header.empty()) {
						std::string::size_type sp1, sp2;
						sp1 = _incompleteheader.find(' ');
						if (sp1==std::string::npos) break;
						sp2 = _incompleteheader.find(' ', sp1+1);
						if (sp2==std::string::npos) break;
						_header.insert(std::pair<std::string, std::string>(
										   "Status", _incompleteheader.substr(sp1+1, sp2-sp1-1)));
					} else {
						std::string::size_type colon, colonsp;
						colon = _incompleteheader.find(':');
						if (colon==std::string::npos) break;
						colonsp = colon;
						do {
							colonsp++;
						} while (colonsp<_incompleteheader.length() &&
								 isspace(_incompleteheader[colonsp]));
						_header.insert(std::pair<std::string, std::string>(
										   _incompleteheader.substr(0,colon), _incompleteheader.substr(colonsp)));
					}
					_incompleteheader = "";
				}
			}
			_incompleteheader.push_back(rcvchr);
			if (datalen<=0)
				return false;
		}
		std::map<std::string, std::string>::const_iterator iter;

		_content_length = -1;
		_send_more_data = true;
	
		iter = _header.find("Status");
		if (iter == _header.end()) return false;
	
		if ((*iter).second != "200") {
			COUT << "Received HTTP error: status is "+ (*iter).second << std::endl;
		}
	
		iter = _header.find("Content-Length");
		if (iter != _header.end()) {
			_content_length = atoi((*iter).second.c_str());
			if (_content_length==0)
				_content_length = -1;
		}

		iter = _header.find("Content-Type");
		if (iter != _header.end()) {
			if ((*iter).second != "message/x-vsnet-packet") {
				COUT << "Invalid content type " << (*iter).second << std::endl;
			}
		}
		iter = _header.find("Connection");
		if (iter != _header.end()) {
			if ((*iter).second == "close") {
				_send_more_data = false;
			}
		} else {
			// assume no more data allowed.
			_send_more_data = false;
		}
	}

	// Now, the socket *should* contain message/x-vsnet-packet data, if we checked.
	bool ret = VsnetTCPSocket::lower_selected( _content_length );
	if (ret) {
		_cpq_mx.lock();
		if (_cpq.size()) {
			PacketPtr pkt = _cpq.back();
			if (pkt) {
				if (_content_length >0) {
					_content_length -= (pkt->getDataLength() + pkt->getHeaderLength());
					if (_content_length<0) 
						_content_length = 0;
				}
			} else {
				_content_length = 0;
			}
		} else {
			_content_length = 0;
		}
		_cpq_mx.unlock();
	} else {
		/*
		  Incomplete packet... now _content_length is wrong...
		  but we will fix it when this packet finishes.
		*/
	}
	
	if (_content_length == 0 && !_send_more_data && _fd!=-1 && !_connection_closed) {
		this->close_fd();
	}
	_connection_closed = false;
	return ret;
}
