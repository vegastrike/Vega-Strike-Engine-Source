#ifndef __NETWORKCOMM_H
#define __NETWORKCOMM_H

#include <list>
#include "client.h"
#ifdef __VOIP
#include "jvoiplib/jvoipsession.h"
#include "jvoiplib/jvoiprtptransmission.h"
#endif

class WebcamSupport;

class NetworkCommunication
{
	private:
		// List of clients we are communicating with
		list<Client *>	commClients;
		// Webcam support
		WebcamSupport *		Webcam;
#ifdef __VOIP
		JVOIPSession				session;
		JVOIPSessionParams			params;
		JVOIPRTPTransmissionParams	rtpparams;
#endif

	public:
		NetworkCommunication();
		~NetworkCommunication();

		int		InitWebcam();
};

#endif
