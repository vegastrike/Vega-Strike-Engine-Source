#ifndef __NETWORKCOMM_H
#define __NETWORKCOMM_H

#ifdef CRYPTO
#include <crypto++/randpool.h>
using namespace CryptoPP;
#endif

#include "const.h"
#include "client.h"
#include "clientptr.h"
#include <string>
#include <list>
#include <deque>
#include "boost/shared_ptr.hpp"
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
namespace VsnetDownload {
  namespace Server {
    class Manager;
  };
};
namespace VsnetDownload {
  namespace Client {
    class Manager;
  };
};

class WebcamSupport;

class NetworkCommunication
{
	private:
		// Text message
		std::deque<std::string>	message_history;		// Text message history
		unsigned short			max_messages;			// Maximum number of text messages recorded
		list<ClientPtr>			commClients;			// List of client communicating on the same frequency
		CltPtrIterator			webcamClient;			// The client we are watching the webcam

		char	crypt_key[DESKEY_SIZE];		// Key used for encryption on secured channels
		float				min_freq, max_freq;
		float				freq;			// Current communication frequency
		bool				active;			// Tell wether the communication system is active
		char				secured;		// Tell wether we are on a secured channel or not
		unsigned char		method;			// Method used to spread comms
#ifndef NETCOMM_NOWEBCAM
		// Webcam support
		WebcamSupport *		Webcam;
        boost::shared_ptr<VsnetDownload::Client::Manager> _downloader;
        boost::shared_ptr<VsnetDownload::Server::Manager> _downloadServer;
        SocketSet           _sock_set;      // Encapsulates select()
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
#ifdef CRYPTO
		//Algorithm *			cryptalgo;
		RandomPool			randPool;
		string				crypto_method;
		string				seed;
		unsigned int		key_length;
		string				pubKeyFilename;
		string				privKeyFilename;
		string				pubkey;
		string				privkey;

#endif
		void				GenerateKey();
		string				EncryptBuffer( const char * buffer, unsigned int length = 0);
		string				DecryptBuffer( const char * buffer, unsigned int length = 0);

	public:
		enum	CommunicationMethod { ClientBroadcast, ServerUnicast };

		NetworkCommunication();
		NetworkCommunication( float minfreq, float maxfreq, bool video, bool secured, string method);
		NetworkCommunication( int nb);
		~NetworkCommunication();

		int		InitSession( float frequency);
		//void	SendImage( SOCKETALT & send_sock);
		void	SendSound( SOCKETALT & send_sock, ObjSerial serial);
		void	SendMessage( SOCKETALT & send_sock, ObjSerial serial, string message);
		void	RecvSound( char * sndbuffer, int length, bool encrypted=false);
		void	RecvMessage( string message, bool encrypted=false);
		int		DestroySession();

		void	AddToSession( ClientPtr clt);
		void	RemoveFromSession( ClientPtr clt);

		bool	IsActive()	{ return active;}
		char *	GetWebcamCapture();

		char	HasWebcam();
		char	HasPortaudio();

		char	IsSecured() { return secured;}
		void	SwitchSecured();
		void	SwitchWebcam();

		float	MinFreq() { return this->min_freq; }
		float	MaxFreq() { return this->max_freq; }

    private:
        void private_init( );
};

#endif

