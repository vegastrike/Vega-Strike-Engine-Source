#include "client.h"
#include "prediction.h"

Client::Client()
{
	latest_timestamp=0;
	old_timestamp=0;
	latest_timeout=0;
	old_timeout=0;
	deltatime=0;
	zone = 0;
	ingame = false;
	webcam = 0;
	portaudio = 0;
	secured = 0;
	jumpfile="";
    _disconnectReason = "none";
	comm_freq = MIN_COMMFREQ;
	prediction = new CubicSplinePrediction();
}

Client::Client( Prediction * pred)
{
	Client::Client(); this->prediction = pred;
}

Client::Client( SOCKETALT& s, bool tcp )
	    : is_tcp(tcp)
	    , sock(s)
{
	Client::Client();
}

inline bool Client::isTcp( ) const {
    return is_tcp;
}

inline bool Client::isUdp( ) const {
    return !is_tcp;
}

