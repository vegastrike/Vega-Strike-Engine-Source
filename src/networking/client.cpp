#include "client.h"

Client::Client()
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

Client::Client( SOCKETALT& s, bool tcp )
	    : is_tcp(tcp)
	    , sock(s)
{
	Client::Client();
}

Client::~Client()
{
}

