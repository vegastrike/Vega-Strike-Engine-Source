#ifdef NETCOMM

#include <config.h>

#include "vsnet_dloadmgr.h"
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
#include "packet.h"
#include <assert.h>

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
		netcomm->audio_inlength = sizeof( unsigned short)*framesPerBuffer;
		cerr<<"Input BUFFER SIZE = "<<netcomm->audio_inlength<<endl;
		memset( netcomm->audio_inbuffer, 0, MAXBUFFER);
		// Maybe put ushort by ushort in the buffer and prepare them for network (htons)
		memcpy( netcomm->audio_inbuffer, in, netcomm->audio_inlength);
	}

	return 0;
}

int	Pa_PlayCallback( void * inputBuffer, void * outputBuffer, unsigned long framesPerBuffer, PaTimestamp outTime, void * userdata)
{
	NetworkCommunication * netcomm = (NetworkCommunication *) userdata;
	unsigned short * out = (unsigned short *) outputBuffer;

	// Copy the last received sound buffer if not yet played (would be NULL) and if PA CPU load is low enough
	if( Pa_GetCPULoad(netcomm->outstream)<MAX_PA_CPU_LOAD && netcomm->audio_outbuffer)
		memcpy( out, netcomm->audio_outbuffer, sizeof( unsigned short)*framesPerBuffer);

	return 0;
}

#endif /* NETCOMM_PORTAUDIO */

NetworkCommunication::NetworkCommunication()
{
	this->active = false;
	this->max_messages = 25;
	this->method = ClientBroadcast;

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
	this->audio_inlength = 0;

#endif /* NETCOMM_PORTAUDIO */
#ifndef NETCOMM_NOWEBCAM

	this->Webcam = NULL;
	string webcam_enable = vs_config->getVariable ("network","use_webcam","false");
	// Init the webcam part
	if( webcam_enable == "true")
	{
		Webcam = new WebcamSupport();
		if( Webcam->Init() == -1)
		// Maybe put ushort by ushort in the buffer and prepare them for network (htons)
		{
			delete Webcam;
			this->Webcam = NULL;
		}
	}
    _downloader.reset( new VsnetDownload::Client::Manager( _sock_set ) );
    _sock_set.addDownloadManager( _downloader );
    _downloadServer.reset( new VsnetDownload::Server::Manager( _sock_set ) );
    _sock_set.addDownloadManager( _downloadServer );

#endif /* NETCOMM_NOWEBCAM */
}

char	NetworkCommunication::HasWebcam()
{
#ifndef NETCOMM_NOWEBCAM
	if( this->Webcam)
		return 1;
	return 0;
#else
	return 0;
#endif
}

char	NetworkCommunication::HasPortaudio()
{
#ifdef NETCOMM_PORTAUDIO
	return 1;
#else
	return 0;
#endif
}

NetworkCommunication::NetworkCommunication( int nb)
{
	NetworkCommunication::NetworkCommunication();
	assert( nb<65536);
	this->max_messages = nb;
}

void	NetworkCommunication::AddToSession( ClientPtr clt)
{
	this->commClients.push_back( clt);
	// If the client has a webcam enabled and we don't have one selected yet
	if( !this->webcamClient && clt->webcam)
		this->webcamClient = clt;
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
void	NetworkCommunication::SendMessage( SOCKETALT & send_sock, ObjSerial serial, string message)
{
	Packet p;
	// If max log size is reached we remove the oldest message
	if( this->message_history.size()==this->max_messages)
		this->message_history.pop_front();
	this->message_history.push_back( message);

	// Send the text message according to the chosen method
	char * msg = new char[message.length()+1];
	memcpy( msg, message.c_str(), message.length());
	msg[message.length()] = 0;
	if( method==ServerUnicast)
	{
		// SEND STRNIG PARAMETER TO SERVER SO THAT HE BROADCASTS IT TO CONCERNED PLAYERS
		p.send( CMD_TXTMESSAGE, serial, msg, message.length(), SENDRELIABLE, NULL, send_sock,
    	        __FILE__, PSEUDO__LINE__(229) );
	}
	else if( method==ClientBroadcast)
	{
		// SEND STRING PARAMETER TO EACH PLAYER COMMUNICATING ON THAT FREQUENCY
		CltPtrIterator it;
		for( it = commClients.begin(); it!=commClients.end(); it++)
		{
			p.send( CMD_TXTMESSAGE, serial, msg, message.length(), SENDRELIABLE, NULL, (*it)->sock,
						__FILE__, PSEUDO__LINE__(244) );
		}
	}
	delete msg;
}

/***************************************************************************************/
/**** Send sound sample in PortAudio mode                                           ****/
/**** Send audio_inbuffer over the network according to the chosen method           ****/
/***************************************************************************************/
void	NetworkCommunication::SendSound( SOCKETALT & sock, ObjSerial serial)
{
#ifdef USE_PORTAUDIO
	Packet p;
	if( method==ServerUnicast)
	{
		// SEND INPUT AUDIO BUFFER TO SERVER SO THAT HE BROADCASTS IT TO CONCERNED PLAYERS

		Packet p;
		// We don't need that to be reliable in UDP mode
		p.send( CMD_SOUNDSAMPLE, serial, this->audio_inbuffer, this->audio_inlength, SENDANDFORGET, NULL, sock,
    	        __FILE__, PSEUDO__LINE__(229) );
	}
	else if( method==ClientBroadcast)
	{
		// SEND INPUT AUDIO BUFFER TO EACH PLAYER COMMUNICATING ON THAT FREQUENCY
		CltPtrIterator it;
		for( it = commClients.begin(); it!=commClients.end(); it++)
		{
			if( (*it)->portaudio)
			{
				p.send( CMD_SOUNDSAMPLE, serial, this->audio_inbuffer, this->audio_inlength, SENDANDFORGET, NULL, (*it)->sock,
						__FILE__, PSEUDO__LINE__(244) );
			}
		}
	}
	memset( audio_inbuffer, 0, MAXBUFFER);
#endif /* USE_PORTAUDIO */
// Do not do anything when using JVoIP lib
}

/***************************************************************************************/
/**** Send a webcam capture                                                         ****/
/**** Send jpeg_buffer over the network according to the chosen method              ****/
/***************************************************************************************/
/*
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
	if( method==ServerUnicast)
	{
		// SEND GRABBED IMAGE TO SERVER SO THAT HE BROADCASTS IT TO CONCERNED PLAYERS
	}
	else if( method==ClientBroadcast)
	{
		// SEND GRABBED IMAGE TO EACH PLAYER COMMUNICATING ON THAT FREQUENCY
	}
#endif
}
*/

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

char *	NetworkCommunication::GetWebcamCapture()
{
	return Webcam->jpeg_buffer;
}

