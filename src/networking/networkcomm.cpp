#include "networking/webcam_support.h"
#include "networkcomm.h"
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"

NetworkCommunication::NetworkCommunication()
{
}

int		NetworkCommunication::GrabImage()
{
	if( Webcam)
		Webcam->CaptureImage();
	return 0;
}

int		NetworkCommunication::InitSession( float frequency)
{
	// Init the VoIP session
#ifndef NETCOMM_NOSOUND
	this->session = new JVOIPSession;
	this->params = new JVOIPSessionParams;
	this->rtpparams = new JVOIPRTPTransmissionParams;
#endif

	// Init the webcam part
	int	ret = 0;
	// GET VALUES FROM CONFIG SOON !
	Webcam = new WebcamSupport();
	if( (ret=Webcam->Init()) == -1)
		delete Webcam;
	else
		this->Webcam->StartCapture();

	return ret;
}

int		NetworkCommunication::DestroySession()
{
#ifndef NETCOMM_NOSOUND
	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;
#endif
	if( this->Webcam)
	{
		this->Webcam->EndCapture();
		this->Webcam->Shutdown();
		delete this->Webcam;
	}

	return 0;
}

NetworkCommunication::~NetworkCommunication()
{
#ifndef NETCOMM_NOSOUND
	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;
#endif
	if( this->Webcam)
		delete this->Webcam;
}

