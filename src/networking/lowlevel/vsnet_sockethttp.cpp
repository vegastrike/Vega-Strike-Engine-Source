#include "vsnet_socket.h"
#include "vsnet_sockethttp.h"
#include "vsnet_oss.h"
#include "vsnet_err.h"

#include "lin_time.h"

#include "packet.h"
#include "netui.h"


static int httpnum = -1;

VsnetHTTPSocket::VsnetHTTPSocket(
                 const AddressIP& remote_ip,
				 const std::string&     host,
                 const std::string&     path,
                 SocketSet&       set )
		: VsnetSocketBase(-1, "http", set), _path(path), _hostheader(host),
		  _incompleteheadersection(0), _content_length(0), _send_more_data(false)

		{
			numRetries = 0;
			timeToNextRequest = 0;
                  readHeader=false;
                  ischunked=false;
                  this->_remote_ip=remote_ip;
                  ischunked=false;
                  chunkedlen=0;
                  chunkedchar='\0';
                  readingchunked=false;

}
bool VsnetHTTPSocket::write_on_negative() {
  return need_test_writable();
}
bool VsnetHTTPSocket::need_test_writable( ){
  if (_fd<=0) {
    if ((int)(queryTime()-1) < timeToNextRequest) {
      return false;
    }
  }
//  std::cout << "retry: " << (int)(queryTime()-1) << " < " << timeToNextRequest << std::endl;
  return !dataToSend.empty();
}
bool ishex(char x) {
  if (x>='0'&&x<='9') return true;
  if (x>='a'&&x<='f') return true;
  if (x>='A'&&x<='F') return true;
  if (x=='x'||x=='X') return true;
  return false;
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
bool VsnetHTTPSocket::isActive() {
  return _content_length==0 && !dataToReceive.empty();
    //  return waitingToReceive!=false;
}
VsnetHTTPSocket::VsnetHTTPSocket(
                 const std::string& uri,
                 SocketSet&       set )
		: VsnetSocketBase(-1, "http", set),
		  _incompleteheadersection(0), _content_length(0), _send_more_data(false)
{
  numRetries = 0;
  timeToNextRequest = 0;
  readHeader=false;
  std::string dummy;
  unsigned short port;
  this->_remote_ip=remoteIPFromURI(uri);
  hostFromURI(uri, dummy, this->_hostheader, this->_path, port);
  ischunked=false;
  readingchunked=false;
  chunkedlen=0;
  chunkedchar='\0';
}

void VsnetHTTPSocket::reopenConnection() {

  if (dataToReceive.length()==0)
      waitingToReceive=std::string();
  _send_more_data=true;
  timeToNextRequest = (int)queryTime() + 2;
  if (this->_fd>=0) {
    this->close_fd();
    this->_fd=-1;
  }
  _incompleteheadersection=0;
  _incompleteheader=std::string();
  _header.clear();
  this->_fd = NetUIBase::createClientSocket(_remote_ip, true,true);
}

bool VsnetHTTPSocket::isReadyToSend(fd_set* write_set_select){
  if (get_write_fd()<0||FD_ISSET(get_write_fd(),write_set_select)) {
	  return need_test_writable();
  }
  return false;
}
void VsnetHTTPSocket::dump( std::ostream& ostr ) const
{
  //VsnetSocketBase::dump( ostr );
  ostr << "URI: http://" << _remote_ip << _path;
}

bool VsnetHTTPSocket::sendstr(const std::string &data) {
	dataToSend.push_back(data);
	return true;
}

bool VsnetHTTPSocket::recvstr(std::string &data) {
	if (!waitingToReceive.empty() ) {
		if (_content_length==0 && !dataToReceive.empty()) {
		  waitingToReceive = std::string();
                  data = dataToReceive;
                  dataToReceive = std::string();
                  _incompleteheader=std::string();
                  _header.clear();      
                  return true;
		}
	}
	return false;
}


std::ostream& operator<<( std::ostream& ostr, const VsnetHTTPSocket& s )
{
  s.dump( ostr );
    return ostr;
}
void VsnetHTTPSocket::lower_clean_sendbuf( ) { 
	if ( waitingToReceive.empty() && this->_fd == -1 ) {
		reopenConnection();
	}
}
extern int NONBLOCKING_CONNECT;
int VsnetHTTPSocket::lower_sendbuf(  )
{
        if (!waitingToReceive.empty())
		return 0;
	
	if ( this->_fd == -1 ) {
		printf("reopening from lower_sendbuf...\n");
		reopenConnection();
		if (NONBLOCKING_CONNECT) {
		   return 0;
		}
	}

	if (!(_send_more_data || _content_length ||
		  _incompleteheader.length() || _header.size())) {
          COUT << "Error: HTTP data being sent while incomplete header exists";
          this->close_fd();
          this->_fd=-1;
	}

	std::string dataSending = dataToSend.front();
	std::string httpData;
	char endHeaderLen[50];
	sprintf(endHeaderLen, "Content-Length: %d\r\n\r\n", dataSending.length() );
	httpData = "POST " + this->_path + " HTTP/1.1\r\n"
		"Host: " + this->_hostheader + "\r\n"
		"User-Agent: Vsnet/1.0\r\n"
		"Connection: keep-alive\r\n"
		"Accept: message/x-vsnet-packet\r\n"
		"Keep-Alive: 300\r\n"
		"Content-Type: message/x-vsnet-packet\r\n" +
		endHeaderLen + dataSending;

	const char *httpDataStr = httpData.data();
	int pos = 0;
	int retrycnt = 10;
	int blockcnt = 10;
	while (true) {
		int len = httpData.length() - pos;
		int netsent;
		netsent = ::send( _fd, &httpDataStr[pos], len, 0 );
		if (netsent <=0 ) {
			if (vsnetEWouldBlock()) {
				if (blockcnt>0) {
					blockcnt--;
					continue;
				} else {
					// Can't hold up anything trying to wait to send data.
					return 0;
				}
			}
			if (retrycnt) {
//				printf("Server closed in writing... reopening\n");
				// What!?! A HTTP server decided to close the connection? The horror...
				reopenConnection();
				if (NONBLOCKING_CONNECT) {
			
					return 0;
				}
			} else {
				return 0;
			}
			retrycnt --;
			continue;
		}
		pos += netsent;
		if (pos>=httpData.length())
			break;
	}
	waitingToReceive = dataToSend.front();
	dataToSend.pop_front();
	return 1;
}

bool VsnetHTTPSocket::parseHeaderByte( char rcvchr )
{
	if (rcvchr=='\r' && _incompleteheadersection!=1) {
		_incompleteheadersection++;
		if (_incompleteheadersection==1) // is it the first \r?
			return false;
	}
	if (rcvchr=='\n' && _incompleteheadersection>0) {
		if (_incompleteheadersection>2) {
			_incompleteheadersection = 0;
			return true;
		} else {
			_incompleteheadersection++;
			return false;
		}
	}
	while (_incompleteheadersection>=2) {
		if (_incompleteheadersection==2)
			_incompleteheadersection=0;
		if (_incompleteheadersection>2 || !isspace(rcvchr)) {
			if (_header.empty()) {
				std::string::size_type sp1, sp2;
				sp1 = _incompleteheader.find(' ');
				if (sp1==std::string::npos) break;
				//sp2 = _incompleteheader.find(' ', sp1+1);
				//if (sp2==std::string::npos) break;
				_header.insert(std::pair<std::string, std::string>(
								   "Status", _incompleteheader.substr(sp1+1/*, sp2-sp1-1*/)));
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
                break;//always break
	}
	if (rcvchr=='\r' && _incompleteheadersection!=1 && _incompleteheadersection>2)
		return false;
	
	_incompleteheader += (rcvchr);
	return false;
}
int errchance=9;
void VsnetHTTPSocket::resendData() {
    if (this->_fd>=0) {
	this->close_fd();
    }
    this->_fd=-1;
	if (waitingToReceive.empty())
		return;
	if (numRetries >= 10) {
		waitingToReceive = std::string();
		dataToReceive = "e";
		return;
	}
	numRetries ++;
	timeToNextRequest = (int)queryTime() + 2;
    dataToSend.push_front(waitingToReceive);
    waitingToReceive=std::string();
    _header.clear();
    readHeader=false;
    _content_length=0;
    readingchunked=false;
    ischunked=false;
    _send_more_data=false;
    chunkedlen=0;
    _incompleteheadersection=0;							
    chunkedchar='\0';
}
bool VsnetHTTPSocket::lower_selected( int datalen )
{
        if (waitingToReceive.empty()) {
			if (this->_fd>=0) { // dataToSend.size()==0&&
            this->close_fd();
            this->_fd=-1;//don't bother keepalive
          }
          return false;
	}
	if ( this->_fd == -1 ) {
	    resendData();
	    return false;
	}

	char rcvbuf[1440];
	int ret=0;
	int bufpos=0;

	while (datalen!=0) {
		int dataToRead;
		int dataIWantToRead = 1400;
		if (_content_length>0) {
			dataIWantToRead = _content_length;
		}
		dataToRead = dataIWantToRead;
		if (datalen>0) {
			dataToRead = datalen<dataIWantToRead?datalen:dataIWantToRead;
		}
		cout<<" Reading "<<dataToRead<<" characters...";
		int ret = VsnetOSS::recv( get_fd(), &rcvbuf, dataToRead, 0 );
		if (ret>0) cout<<"got "<<std::string(rcvbuf, ret)<<endl; // DELETEME!
		else cout << "recv returned " << ret << endl;
		if (ret==0) {
			//It is not an error if closed without a Content-Length header.
			if (_content_length>=0&&( (readHeader==false /*set to false on success return*/&&dataToReceive.length()==0)||readHeader==true)) {
				// incomplete transfer...
			  printf("Server closed in reading...resending\n");
 	          resendData();
			  return false;
			}
			datalen = 0;
			_content_length=0;
			break; // Done reading!
		}else
		if (ret<0) {
			if (vsnetEWouldBlock()) {
				datalen = 0;
				return false;
			}
			// errored
			perror("Error in reading HTTP socket");
			resendData();
			return false;
		}
		// Let ret==0 but transfer complete fall through.
		if (datalen>0||datalen<0) {
			if (datalen!=-1) datalen -= ret;
                        bufpos=0;
                        if (_content_length == 0) {
                          for (bufpos=0;bufpos<ret;bufpos++) {
					char rcvchr = rcvbuf[bufpos];
					if (parseHeaderByte(rcvchr)) {
						std::map<std::string, std::string>::const_iterator iter;

						_content_length = -1;
                                                ischunked=false;
						_send_more_data = true;
	
						iter = _header.find("Status");
						
						if (iter == _header.end()) {
                                                  COUT<<"Missing status, resending\n";
                                                  resendData();
                                                  return false;
                                                }
                                                if((*iter).second.find("100")!=std::string::npos) {                                                  
                                                  _content_length=0;
                                                  _header.clear();
												  printf ("100 found in header at marker %d... rest of string looks like:%s\n",bufpos,rcvbuf+bufpos);
                                                  continue;
                                                
                                                  
                                                }else if((*iter).second.find("200")==std::string::npos) {
                                                  COUT << "Received HTTP error: status is "+ (*iter).second << std::endl;
                                                  resendData();
                                                  return false;
						}
	
						iter = _header.find("Content-Length");
						if (iter != _header.end()) {
							_content_length = atoi((*iter).second.c_str());
							if (_content_length==0)
								_content_length = -1;
						}

						
						  // Don't need to check content-type any more.
						iter = _header.find("Content-Type");
						if (iter != _header.end()) {
							if ((*iter).second != "message/x-vsnet-packet"&&(*iter).second != "text/html") {
								COUT << "content type " << (*iter).second << std::endl;
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
						iter = _header.find("Transfer-Encoding");
						if (iter != _header.end()) {
							if ((*iter).second == "chunked") {
								ischunked = true;
                                                                
							}
						} else {
							// assume no more data allowed.
							ischunked = false;
						}
                                                readHeader=true;
                                                bufpos++;
                                                if (ischunked) {
                                                  readingchunked=true;
                                                }
                                                chunkedlen=0;
                                                chunkedchar='\0';
                                                break;
					}
				}
			}
                        while(bufpos<ret) {
                          for (;bufpos<ret&&readingchunked;bufpos++) {            
                            char rcvchr=rcvbuf[bufpos];
                            if (rcvchr=='\n'&&chunkedchar!='\0'/*make sure that it had read something of interest*/) {
                              readingchunked=false;
                              if (chunkedlen==0)
                                goto donewiththis; // break(3);
                            }
                            if (ishex(rcvchr)&&(ishex(chunkedchar)||chunkedchar=='\0')) {
                              if (rcvchr>='0'&&rcvchr<='9') {
                                chunkedlen*=16;
                                chunkedlen+=rcvchr-'0';
                                chunkedchar=rcvchr;
                              }
                              else if (rcvchr>='A'&&rcvchr<='F') {
                                chunkedlen*=16;
                                chunkedlen+=rcvchr-'A'+10;
                                chunkedchar=rcvchr;
                              }   
                              else if (rcvchr>='a'&&rcvchr<='f') {
                                chunkedlen*=16;
                                chunkedlen+=rcvchr-'a'+10;
                                chunkedchar=rcvchr;
                              }
                              chunkedchar=rcvchr;
                            }
                            if (rcvchr==';') chunkedchar=';';// this means extension follow and we shouldn't add numbers
                          }
                          if (readingchunked==false&&_content_length!=0&&bufpos<ret) {
                            
                            // Now, the socket *should* contain message/x-vsnet-packet data.
                            int delta=ret-bufpos;
                            if (delta>chunkedlen&&ischunked)
                              delta=chunkedlen;
                            dataToReceive += std::string(rcvbuf+bufpos, (std::string::size_type)delta);
                            if (ischunked) {
                              chunkedlen-=delta;
                              if (chunkedlen==0){
                                chunkedchar='\0';
                                readingchunked=true;
                              }
                            }
                            bufpos+=delta;
                            if (_content_length>0) {
                              _content_length -= ret;
                              if (_content_length<=0) {
                                _content_length = 0;
                                break;
                              }
                            }
                          }
                        }
		}
		if (datalen!=-1&&datalen<=0) {
			return false;
		}
	}
 donewiththis:
	/*
	  // What the heck?
	if (datalen == -1) {
		resendData();
		datalen = 0;
		return false;
	}
	*/
	if (_content_length == 0 && !_send_more_data && _fd!=-1) {
          this->close_fd();
          this->_fd=-1;
          //reopenConnection();
          if (dataToReceive.length()==0||(dataToReceive[0]=='!'&&dataToReceive.length()<4)) {//don't bother with the bangs
            dataToReceive=std::string();
            waitingToReceive=std::string();//can't receive a null info
          }
	}
	//_connection_closed = false;
        readHeader=false;
	return true;
}
