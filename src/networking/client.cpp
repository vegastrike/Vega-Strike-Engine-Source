#include "client.h"
#include "lowlevel/vsnet_debug.h"

void Client::Init()
{
	_latest_timestamp = 0;
	_old_timestamp    = 0;
	_deltatime        = 0;
	latest_timeout=0;
	old_timeout=0;
	ingame = false;
	webcam = 0;
	portaudio = 0;
	secured = 0;
	jumpfile="";
    _disconnectReason = "none";
	comm_freq = 0;
}

Client::Client()
{
	this->Init();
}

Client::Client( SOCKETALT& s, bool tcp )
	    : is_tcp(tcp)
	    , sock(s)
{
	this->Init();
}

Client::~Client()
{
}

void Client::setLatestTimestamp( unsigned int ts )
{
    COUT << "set latest client timestamp " << ts << " (old=" << _old_timestamp << ")" << endl;
    _old_timestamp    = _latest_timestamp;
    _latest_timestamp = ts;

    // Compute the deltatime in seconds that is time between packet_timestamp
    // in ms and the old_timestamp in ms
    _deltatime = ((double)(ts - _old_timestamp))/1000;
    cerr<<"DELTATIME = "<<(_deltatime*1000)<<" ms --------------------"<<endl;
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

