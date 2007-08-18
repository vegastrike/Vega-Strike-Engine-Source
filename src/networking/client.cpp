#include "client.h"
#include "lowlevel/vsnet_debug.h"
#include "networking/prediction.h"

void Client::Init()
{
	_latest_timestamp = 0;
	_old_timestamp    = 0;
	_deltatime        = 0;
	_next_deltatime   = 0.5; // Some (sane) default here!
	elapsed_since_packet=0;
	latest_timeout=0;

// NETFIXME: Cubic Splines appear to be broken...
	
//	prediction = new MixedPrediction();
	prediction = new LinearPrediction();
	old_timeout=0;
	ingame = false;
	loginstate = CONNECTED;
	webcam = 0;
	portaudio = 0;
	secured = 0;
        jumpok=0;
	jumpfile="";
    _disconnectReason = "none";
	comm_freq = 0;
	last_packet=ClientState();
}

// NetClient initialization
Client::Client()
{
	lossy_socket=NULL;
	this->Init();
}

// NetServer initialization
Client::Client( SOCKETALT& s )
	    : tcp_sock(s)
{
	lossy_socket=&tcp_sock;
	cltadr = s.getRemoteAddress();
	this->Init();
}

Client::~Client()
{
	if( prediction)
    {
		delete prediction;
        prediction = NULL;
    }
}

void Client::setUDP(SOCKETALT *udpSock, AddressIP &udpadr) {
	this->lossy_socket=udpSock;
	this->cltudpadr=udpadr;
}

void Client::setTCP() {
	this->lossy_socket=&this->tcp_sock;
	this->cltudpadr=this->cltadr;
}

void Client::setLatestTimestamp( unsigned int ts )
{
//    COUT << "set latest client timestamp " << ts << " (old=" << _old_timestamp << ")" << endl;
    _old_timestamp    = _latest_timestamp;
    _latest_timestamp = ts;

    // Compute the deltatime in seconds that is time between packet_timestamp
    // in ms and the old_timestamp in ms
    _deltatime = ((double)(ts - _old_timestamp))/1000;
	_next_deltatime = .33*_deltatime+.67*_next_deltatime;
//    cerr<<"DELTATIME = "<<(_deltatime*1000)<<" ms --------------------"<<endl;
//    cerr<<"NEXTDELTATIME = "<<(_deltatime*1000)<<" ms --------------------"<<endl;
}

void Client::clearLatestTimestamp( )
{
	_latest_timestamp = 0;
	_old_timestamp    = 0;
    _deltatime        = 0;
}

unsigned int Client::getLatestTimestamp( ) const
{
    return _latest_timestamp;
}

double Client::getDeltatime( ) const
{
    return _deltatime;
}

double Client::getNextDeltatime( ) const
{
    return _next_deltatime;
}

