#ifndef __NETWORKCOMM_H
#define __NETWORKCOMM_H

#include "client.h"
//struct Client;
#include <list>
#include <config.h> // for NETCOMM_NOSOUND

#ifndef NETCOMM_NOWEBCAM
class WebcamSupport;
#endif
#ifndef NETCOMM_NOSOUND
class JVOIPSession;
class JVOIPSessionParams;
class JVOIPRTPTransmissionParams;
#endif

class NetworkCommunication
{
	private:
		// List of clients we are communicating with
		list<Client *>	commClients;
		// The current communication frequency
		float	freq;
#ifndef NETCOMM_NOWEBCAM
		// Webcam support
		WebcamSupport *		Webcam;
#endif
#ifndef NETCOMM_NOSOUND
		JVOIPSession *				session;
		JVOIPSessionParams *		params;
		JVOIPRTPTransmissionParams* rtpparams;
#endif

	public:
		NetworkCommunication();
		~NetworkCommunication();

		int		InitSession( float frequency);
		int		GrabImage( ObjSerial serial);
		int		DestroySession();
		bool	WebcamEnabled();
		bool	WebcamTime();
};

#endif

