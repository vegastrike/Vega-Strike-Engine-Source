#include <config.h>

#ifndef NETCOMM_NOWEBCAM
#include "networking/webcam_support.h"
#endif /* NETCOMM_NOWEBCAM */
#ifdef NETCOMM_JVOIP
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#endif /* NETCOMM_JVOIP */

#define VOIP_PORT	5000

#include "networkcomm.h"

#ifdef NETCOMM_JVOIP
void	CheckVOIPError( int val)
{
	if( val>=0)
		return;
	string error = JVOIPGetErrorString( val);
	cerr<<"!!! JVOIP ERROR : "<<error<<endl;
	exit(1);
}
#endif /* NETCOMM_JVOIP */

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
	CheckError( this->session->AddDestination( ntohl( clt->cltadr.inaddr() ), VOIP_PORT));
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
#ifndef NETCOMM_NOWEBCAM
	if( this->Webcam)
	{
		this->Webcam->Shutdown();
		delete this->Webcam;
		this->Webcam = NULL;
	}
#endif /* NETCOMM_NOWEBCAM */
}

