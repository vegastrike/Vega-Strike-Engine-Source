#ifndef __NETWORKCOMM_H
#define __NETWORKCOMM_H

#include "client.h"
#include "clientptr.h"
#include <string>
#include <list>
#include <deque>
#include <config.h> // for NETCOMM_NOSOUND

#ifndef NETCOMM_NOWEBCAM
class WebcamSupport;
#endif
#ifdef NETCOMM_JVOIP
class JVOIPSession;
class JVOIPSessionParams;
class JVOIPRTPTransmissionParams;
#endif
#ifdef USE_PORTAUDIO
#endif

class NetworkCommunication
{
	private:
		// Text message
		std::deque<std::string>	message_history;
		int		max_messages;
		// List of clients we are communicating with
		list<ClientPtr>	commClients;
		// The current communication frequency
		float	freq;
		bool	active;
		unsigned char	method;
#ifndef NETCOMM_NOWEBCAM
		// Webcam support
		WebcamSupport *		Webcam;
#endif
#ifdef NETCOMM_JVOIP
		JVOIPSession *				session;
		JVOIPSessionParams *		params;
		JVOIPRTPTransmissionParams* rtpparams;
#endif

	public:
		NetworkCommunication();
		NetworkCommunication( int nb);
		~NetworkCommunication();

		int		InitSession( float frequency);
		void	SendImage( SOCKETALT & send_sock);
		void	SendSound( SOCKETALT & send_sock);
		void	SendMessage( SOCKETALT & send_sock, string message);
		int		DestroySession();

		void	AddToSession( ClientPtr clt);
		void	RemoveFromSession( ClientPtr clt);

		bool	IsActive()	{ return active;}
};

#endif

