#include <config.h>

#ifndef NETCOMM_NOWEBCAM
#include "networking/webcam_support.h"
#endif
#ifndef NETCOMM_NOSOUND
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#endif

#include "networkcomm.h"

NetworkCommunication::NetworkCommunication()
{
#ifndef NETCOMM_NOSOUND
	this->session = NULL;
	this->params = NULL;
	this->rtpparams = NULL;
#endif
#ifndef NETCOMM_NOWEBCAM
	this->Webcam = NULL;
#endif
}

int		NetworkCommunication::GrabImage()
{
#ifndef NETCOMM_NOWEBCAM
	if( Webcam)
		Webcam->CaptureImage();
#endif
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

#ifndef NETCOMM_NOWEBCAM
	// Init the webcam part
	int	ret = 0;
	// GET VALUES FROM CONFIG SOON !
	Webcam = new WebcamSupport();
	if( (ret=Webcam->Init()) == -1)
		delete Webcam;
	else
		this->Webcam->StartCapture();
#endif

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
#ifndef NETCOMM_NOWEBCAM
	if( this->Webcam)
	{
		this->Webcam->EndCapture();
		this->Webcam->Shutdown();
		delete this->Webcam;
	}
#endif

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
#ifndef NETCOMM_NOWEBCAM
	if( this->Webcam)
		delete this->Webcam;
#endif
}

