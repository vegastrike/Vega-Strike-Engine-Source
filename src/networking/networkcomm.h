#ifndef __NETWORKCOMM_H
#define __NETWORKCOMM_H

#include <list>
#include "client.h"

class WebcamSupport;
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
		// Webcam support
		WebcamSupport *		Webcam;
#ifndef NETCOMM_NOSOUND
		JVOIPSession *				session;
		JVOIPSessionParams *		params;
		JVOIPRTPTransmissionParams* rtpparams;
#endif

	public:
		NetworkCommunication();
		~NetworkCommunication();

		int		InitSession( float frequency);
		int		GrabImage();
		int		DestroySession();
};

#endif

