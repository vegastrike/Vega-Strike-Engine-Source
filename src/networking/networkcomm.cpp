#include "networking/webcam_support.h"
#include "networking/networkcomm.h"

NetworkCommunication::NetworkCommunication()
{
}

int		NetworkCommunication::InitWebcam()
{
	int	ret = 0;
	// GET VALUES FROM CONFIG SOON !
	Webcam = new WebcamSupport();
	if( (ret=Webcam->Init()) == -1)
		delete Webcam;

	return ret;
}

NetworkCommunication::~NetworkCommunication()
{
	if( Webcam != NULL)
		delete Webcam;
}

