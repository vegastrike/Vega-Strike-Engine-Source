#ifdef NETCOMM

#include <config.h>

#ifndef NETCOMM_NOWEBCAM
#include "networking/webcam_support.h"
#endif /* NETCOMM_NOWEBCAM */
#ifdef NETCOMM_JVOIP
#include <jvoiplib/jvoipsession.h>
#include <jvoiplib/jvoiprtptransmission.h>
#endif /* NETCOMM_JVOIP */

#define VOIP_PORT	5000
extern bool cleanexit;

#include "gldrv/winsys.h"
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
	cerr<<"!!! PORTAUDIO ERROR : "<<Pa_GetErrorText( err)<<endl;
	if( err==paHostError)
		cerr<<"!!! PA HOST ERROR : "<<Pa_GetHostError()<<" - "<<Pa_GetErrorText( Pa_GetHostError())<<endl;
	cleanexit = true;
	winsys_exit(1);
}

void	Pa_DisplayInfo( PaDeviceID id)
{
	const PaDeviceInfo * info = Pa_GetDeviceInfo( id);

	cout<<"PORTAUDIO Device "<<id<<"---------------"<<endl;
	cout<<"\tName : "<<info->name<<endl;
	cout<<"\tStructure version : "<<info->structVersion<<endl;
	cout<<"\tmaxInputChannels : "<<info->maxInputChannels<<endl;
	cout<<"\tmaxOutputChannels : "<<info->maxOutputChannels<<endl;
	cout<<"\tnativeSampleFormats : ";
	switch( info->nativeSampleFormats)
	{
		case (paFloat32) :
			cout<<"paFloat32"<<endl; break;
		case (paInt16) :
			cout<<"paInt16"<<endl; break;
		case (paInt32) :
			cout<<"paInt32"<<endl; break;
		case (paInt24) :
			cout<<"paInt24"<<endl; break;
		case (paPackedInt24) :
			cout<<"paPackedInt24"<<endl; break;
		case (paInt8) :
			cout<<"paInt8"<<endl; break;
		case (paUInt8) :
			cout<<"paUInt8"<<endl; break;
		case (paCustomFormat) :
			cout<<"paCustomFormat"<<endl; break;
		
	}
	cout<<"\tnumSampleRates : "<<info->numSampleRates<<endl;
	for( int i=0; i<info->numSampleRates; i++)
		cout<<"\t\tRate "<<i<<" = "<<info->sampleRates[i];
}

int	Pa_RecordCallback( void * inputBuffer, void * outputBuffer, unsigned long framesPerBuffer, PaTimestamp outTime, void * userdata)
{
	NetworkCommunication * netcomm = (NetworkCommunication *) userdata;
	unsigned short * in = (unsigned short *) inputBuffer;

	// Only act if PA CPU Load is low enough and if we have data in the input buffer
	if( Pa_GetCPULoad(netcomm->instream)<MAX_PA_CPU_LOAD && in)
	{
		memset( netcomm->audio_inbuffer, 0, MAXBUFFER);
		memcpy( netcomm->audio_inbuffer, in, framesPerBuffer*sizeof(unsigned short));
	}

	return 0;
}

int	Pa_PlayCallback( void * inputBuffer, void * outputBuffer, unsigned long framesPerBuffer, PaTimestamp outTime, void * userdata)
{
	NetworkCommunication * netcomm = (NetworkCommunication *) userdata;
	unsigned short * out = (unsigned short *) outputBuffer;

	// Copy the last received sound buffer if not yet played (would be NULL) and if PA CPU load is low enough
	if( Pa_GetCPULoad(netcomm->outstream)<MAX_PA_CPU_LOAD && netcomm->audio_outbuffer)
		memcpy( outputBuffer, netcomm->audio_outbuffer, sizeof( unsigned short)*framesPerBuffer);

	return 0;
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

	this->indev = paNoDevice;
	this->outdev = paNoDevice;
	this->devinfo = NULL;
	// Initialize PortAudio lib
	CheckPAError( Pa_Initialize());

	// Testing purpose : list devices and display info about them
	int nbdev = Pa_CountDevices();
	for( int i=0; i<nbdev; i++)
		Pa_DisplayInfo( i);

	// Get the default devices for input and output streams
	this->indev = Pa_GetDefaultInputDeviceID();
	this->outdev = Pa_GetDefaultOutputDeviceID();
	this->sample_rate = 11025;

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

void	NetworkCommunication::AddToSession( ClientPtr clt, bool webcam, bool pa)
{
	this->commClients.push_back( clt);
	// If the client has a webcam enabled
	if( webcam)
		this->webcamClients.push_back( clt);
	// Affect 1st client to webcam view
	if( this->webcamClients.size()==1)
		this->webcamClient = clt;
	// If the client has a PortAudio support
	if( pa)
		this->paClients.push_back( clt);
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

/***************************************************************************************/
/**** Send text message                                                             ****/
/**** Broadcast string function param as a text message to the current frequency    ****/
/***************************************************************************************/
void	NetworkCommunication::SendMessage( SOCKETALT & send_sock, string message)
{
	// If max log size is reached we remove the oldest message
	if( this->message_history.size()==this->max_messages)
		this->message_history.pop_front();
	this->message_history.push_back( message);

	// Send the text message according to the chosen method
	if( method==1)
	{
		// SEND STRNIG PARAMETER TO SERVER SO THAT HE BROADCASTS IT TO CONCERNED PLAYERS
	}
	else if( method==2)
	{
		// SEND STRING PARAMETER TO EACH PLAYER COMMUNICATING ON THAT FREQUENCY
	}
}

/***************************************************************************************/
/**** Send sound sample in PortAudio mode                                           ****/
/**** Send audio_inbuffer over the network according to the chosen method           ****/
/***************************************************************************************/
void	NetworkCommunication::SendSound( SOCKETALT & send_sock)
{
#ifdef USE_PORTAUDIO
	if( method==1)
	{
		// SEND INPUT AUDIO BUFFER TO SERVER SO THAT HE BROADCASTS IT TO CONCERNED PLAYERS
	}
	else if( method==2)
	{
		// SEND INPUT AUDIO BUFFER TO EACH PLAYER COMMUNICATING ON THAT FREQUENCY
	}
	memset( audio_inbuffer, 0, MAXBUFFER);
#endif /* USE_PORTAUDIO */
// Do not do anything when using JVoIP lib
}

/***************************************************************************************/
/**** Send a webcam capture                                                         ****/
/**** Send jpeg_buffer over the network according to the chosen method              ****/
/***************************************************************************************/
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
	if( method==1)
	{
		// SEND GRABBED IMAGE TO SERVER SO THAT HE BROADCASTS IT TO CONCERNED PLAYERS
	}
	else if( method==2)
	{
		// SEND GRABBED IMAGE TO EACH PLAYER COMMUNICATING ON THAT FREQUENCY
	}

	/* MAYBE USE A VSDOWNLOADSERVER AND CLIENT
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

	// Create the input stream with indev and 1 channel with paInt16 samples
	CheckPAError( Pa_OpenStream( &this->instream,
								 this->indev, 1, paInt16, NULL,
								 paNoDevice, 0, paInt16, NULL,
								 this->sample_rate, 256, 0, paNoFlag,
								 Pa_RecordCallback, (void *)this));

	// Create the output stream with outdev and 1 channel with paInt16 samples
	CheckPAError( Pa_OpenStream( &this->outstream,
								 paNoDevice, 0, paInt16, NULL,
								 this->outdev, 1, paInt16, NULL,
								 this->sample_rate, 256, 0, paNoFlag,
								 Pa_PlayCallback, (void *)this));

	if( this->instream)
		CheckPAError( Pa_StartStream( this->instream));
	if( this->outstream)
		CheckPAError( Pa_StartStream( this->outstream));

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
#ifdef NETCOMM_PORTAUDIO
	CheckPAError( Pa_StopStream( this->instream));
	CheckPAError( Pa_StopStream( this->outstream));
#endif
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

#endif /* NETCOMM */

