#ifndef __NETWORKCOMM_H
#define __NETWORKCOMM_H

#include <list>
#include "client.h"

class WebcamSupport;
class JVOIPSession;
class JVOIPSessionParams;
class JVOIPRTPTransmissionParams;

class NetworkCommunication
{
	private:
		// List of clients we are communicating with
		list<Client *>	commClients;
		// Webcam support
		WebcamSupport *		Webcam;
		JVOIPSession *				session;
		JVOIPSessionParams *		params;
		JVOIPRTPTransmissionParams* rtpparams;

	public:
		NetworkCommunication();
		~NetworkCommunication();

		int		InitWebcam();
		int		InitSession( float frequency);
		int		DestroySession();
};

#endif

