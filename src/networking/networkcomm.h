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
#ifdef NETCOMM_PORTAUDIO
#include <pa/portaudio.h>
#endif

class NetworkCommunication
{
	private:
		// Text message
		std::deque<std::string>	message_history;
		int		max_messages;
		// List of clients we are communicating with
		list<ClientPtr>	commClients;
		// List of clients using webcam
		list<ClientPtr>	webcamClients;
		// Selected webcam from webcamClients list
		ClientPtr		webcamClient;
#ifdef HYBRID_PROTO
		// Socket list -> one for each client we are communicating with
		list<SOCKETALT>	sockClients;
		SocketSet		_sock_set;
		SocketSet		_serversock_set;
#endif /* HYBRID_PROTO */
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
#ifdef NETCOMM_PORTAUDIO
		PaDeviceID			indev;
		PaDeviceID			outdev;
		PaDeviceInfo *		devinfo;
		PortAudioStream *	stream;

		double				sample_rate;
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

