#include <config.h>

#ifndef NETCOMM_NOWEBCAM
#include "networking/webcam_support.h"
#endif /* NETCOMM_NOWEBCAM */
#ifdef NETCOMM_JVOIP
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#endif /* NETCOMM_JVOIP */

#define VOIP_PORT	5000
extern bool cleanexit;

#include "networkcomm.h"

#ifdef NETCOMM_JVOIP
void	CheckVOIPError( int val)
{
	if( val>=0)
		return;
	string error = JVOIPGetErrorString( val);
	cerr<<"!!! JVOIP ERROR : "<<error<<endl;
	cleanexit = true;
	winsys_exit(1);
}
#endif /* NETCOMM_JVOIP */
#ifdef NETCOMM_PORTAUDIO
void	CheckPAError( PaError err)
{
	if( err==paNoError)
		return;
	cerr<<"!!! PORTAUDIO ERROR : "<<Pa_GetErrorText( err)<<end;
	if( err==paHostError)
		cerr<<"!!! PA HOST ERROR : "<<Pa_GetHostError()<<" - "<<Pa_GetErrorText( Pa_GetHostError())<<endl;
	cleanexit = true;
	winsys_exit(1);
}

void	Pa_DisplayInfo( PaDeviceId id)
{
	PaDeviceInfo * info = Pa_GetDeviceInfo( id);

	cout<<"PORTAUDIO Device "<<id<<"---------------"<<endl;
	cout<<"\tName : "<<info->name<<endl;
	cout<<"\tStructure version : "<<info->structVersion<<endl;
	cout<<"\tmaxInputChannels : "<<info->maxInputChannels<<endl;
	cout<<"\tmaxOutputChannels : "<<info->maxOutputChannels<<endl;
	cout<<"\tnativeSampleFormats : ";
	switch( info->nativeSampleFormats)
	{
		case (1<<0) :
			cout<<"paFloat32"<<endl; break;
		case (1<<1) :
			cout<<"paInt16"<<endl; break;
		case (1<<2) :
			cout<<"paInt32"<<endl; break;
		case (1<<3) :
			cout<<"paInt24"<<endl; break;
		case (1<<4) :
			cout<<"paPackedInt24"<<endl; break;
		case (1<<5) :
			cout<<"paInt8"<<endl; break;
		case (1<<6) :
			cout<<"paUInt8"<<endl; break;
		case (1<<16) :
			cout<<"paCustomFormat"<<endl; break;
		
	}
	cout<<"\tnumSampleRates : "<<info->numSampleRates<<endl;
	for( int i=0; i<info->numSampleRates; i++)
		cout<<"\t\tRate "<<i<<" = "<<info->sampleRates[i];
}
#endif /* NETCOMM_PORTAUDIO */

NetworkCommunication::NetworkCommunication()
{
	this->active = false;
	this->max_messages = 25;
	this->method = 1;

#ifdef NETCOMM_JVOIP

	this->session = NULL;
	this->params = NULL;
	this->rtpparams = NULL;

#endif /* NETCOMM_JVOIP */
#ifdef NETCOMM_PORTAUDIO

	this->dev = paNoDevice;
	this->devinfo = NULL;
	// Initialize PortAudio lib
	CheckPAError( Pa_Initialize());

	// Testing purpose : list devices and display info about them
	PaDeviceInfo * info;
	int nbdev = Pa_CountDevices();
	for( int i=0; i<nbdev; i++)
		Pa_DisplayInfo( i);

	// Get the default devices for input and output streams
	this->indev = Pa_GetDefaultInputDeviceID();
	this->outdev = Pa_GetDefaultOutputDeviceID();
	this->samplerate = 11025;

#endif /* NETCOMM_PORTAUDIO */
#ifndef NETCOMM_NOWEBCAM

	this->Webcam = NULL;
	string webcam_enable = vs_config->getVariable ("network","use_webcam","false");
	// Init the webcam part
	if( webcam_enable == "true")
	{
		Webcam = new WebcamSupport();
		if( Webcam->Init() == -1)
		{
			delete Webcam;
			this->Webcam = NULL;
		}
	}

#endif /* NETCOMM_NOWEBCAM */
}

NetworkCommunication::NetworkCommunication( int nb)
{
	NetworkCommunication::NetworkCommunication();
	this->max_messages = nb;
}

void	NetworkCommunication::AddToSession( ClientPtr clt)
{
	this->commClients.push_back( clt);
#ifdef NETCOMM_JVOIP
	CheckVOIPError( this->session->AddDestination( ntohl( clt->cltadr.inaddr() ), VOIP_PORT));
#endif /* NETCOMM_JVOIP */
}

void	NetworkCommunication::RemoveFromSession( ClientPtr clt)
{
	this->commClients.remove( clt);
#ifdef NETCOMM_JVOIP
	CheckVOIPError( this->session->AddDestination( ntohl( clt->cltadr.inaddr() ), VOIP_PORT));
#endif /* NETCOMM_JVOIP */
}

void	NetworkCommunication::SendMessage( SOCKETALT & send_sock, string message)
{
	// If max log size is reached we remove the oldest message
	if( this->message_history.size()==this->max_messages)
		this->message_history.pop_front();
	this->message_history.push_back( message);

	// Send the text message according to method
}

// Send sound sample
void	NetworkCommunication::SendSound( SOCKETALT & send_sock)
{
#ifdef USE_PORTAUDIO
#endif /* USE_PORTAUDIO */
// Do not do anything when using JVoIP lib
}

// Send a grabbed image
void	NetworkCommunication::SendImage( SOCKETALT & send_sock)
{
	string jpeg_str( "");
#ifndef NETCOMM_NOWEBCAM
	if( Webcam && Webcam->isReady())
	{
		//cerr<<"--- Trying to grab an image..."<<endl;
		//cerr<<"\t";
		jpeg_str = Webcam->CaptureImage();
		//cerr<<"--- grabbing finished"<<endl;
	}
	switch( method)
	{
		case 1 :
		{
			// SEND GRABBED IMAGE TO SERVER SO THAT HE BROADCASTS IT TO CONCERNED PLAYERS
		}
		break;
		case 2 :
		{
			// SEND GRABBED IMAGE TO EACH PLAYER COMMUNICATING ON THAT FREQUENCY
		}
		break;
		default :
			cerr<<"!!! ERROR : communication method do not exist !!!"<<endl;
			exit(1);
	}

		/*
		Packet p;
		// We don't need that to be reliable in UDP mode
		p.send( CMD_CAMSHOT, clt->serial, netbuf.getData(), netbuf.getDataLength(), SENDANDFORGET, NULL, clt->clt_sock,
                      __FILE__, PSEUDO__LINE__(49) );
		*/
#endif /* NETCOMM_NOWEBCAM */
}

int		NetworkCommunication::InitSession( float frequency)
{
	// Init the VoIP session
#ifdef NETCOMM_JVOIP

	this->session = new JVOIPSession;
	this->session->SetSampleInterval(100);

	this->params = new JVOIPSessionParams;
	this->rtpparams = new JVOIPRTPTransmissionParams;

	CheckVOIPError( this->session->Create( *(this->params)));

#endif /* NETCOMM_JVOIP */
#ifdef NETCOMM_PORTAUDIO

	CheckPAError( Pa_OpenDefaultStream( &this->stream,
								 this->indev, 1, paInt16, NULL,
								 this->outdev, 2, paInt16, NULL,
								 this->samplerate, 256, 0, paNoFlag,
								 Pa_Callback, (void *)this));

#endif /* NETCOMM_PORTAUDIO */

#ifndef NETCOMM_NOWEBCAM
	if( Webcam)
		this->Webcam->StartCapture();
#endif /* NETCOMM_NOWEBCAM */

	this->active = true;
	this->freq = frequency;
	return 0;
}

int		NetworkCommunication::DestroySession()
{
	this->active = false;
#ifdef NETCOMM_JVOIP
	CheckVOIPError( this->session->Destroy());

	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;
#endif /* NETCOMM_JVOIP */
#ifndef NETCOMM_NOWEBCAM
	if( this->Webcam)
		this->Webcam->EndCapture();
#endif /* NETCOMM_NOWEBCAM */

	return 0;
}

NetworkCommunication::~NetworkCommunication()
{
#ifdef NETCOMM_JVOIP

	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;

#endif /* NETCOMM_JVOIP */
#ifdef NETCOMM_PORTAUDIO

	CheckPAError( Pa_Terminate());

#endif /* NETCOMM_PORTAUDIO */
#ifndef NETCOMM_NOWEBCAM

	if( this->Webcam)
	{
		this->Webcam->Shutdown();
		delete this->Webcam;
		this->Webcam = NULL;
	}

#endif /* NETCOMM_NOWEBCAM */
}

