#include "client.h"

void Client::Init()
{
	latest_timestamp=0;
	old_timestamp=0;
	latest_timeout=0;
	old_timeout=0;
	deltatime=0;
	ingame = false;
	webcam = 0;
	portaudio = 0;
	secured = 0;
	jumpfile="";
    _disconnectReason = "none";
	comm_freq = MIN_COMMFREQ;
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

