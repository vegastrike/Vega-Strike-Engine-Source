#ifndef __NETWORKCOMM_H
#define __NETWORKCOMM_H

#include "client.h"
#include "clientptr.h"
#include <string>
#include <list>
#include <deque>
#include <config.h>

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
#define MAX_PA_CPU_LOAD		0.5
// (MAX_PA_CPU_LOAD*100)% CPU LOAD
#endif

typedef list<ClientPtr>::iterator CltPtrIterator;

class NetworkCommunication
{
	private:
		// Text message
		std::deque<std::string>	message_history;		// Text message history
		int				max_messages;					// Maximum number of text messages recorded
		list<ClientPtr>	commClients;					// List of client communicating on the same frequency
		ClientPtr		webcamClient;					// The client we are watching the webcam

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
		PortAudioStream *	instream;
		PortAudioStream *	outstream;

		double				sample_rate;
		int					audio_inlength;
		unsigned short		audio_inbuffer[MAXBUFFER];
		unsigned short		audio_outbuffer[MAXBUFFER];

		friend int	Pa_RecordCallback( void * inputBuffer, void * outputBuffer, unsigned long framesPerBuffer, PaTimestamp outTime, void * userdata);
		friend int	Pa_PlayCallback( void * inputBuffer, void * outputBuffer, unsigned long framesPerBuffer, PaTimestamp outTime, void * userdata);
#endif

	public:
		enum	ComminucationMethod { ClientBroadcast, ServerUnicast };

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

