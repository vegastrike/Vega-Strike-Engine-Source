/*

    This file is a part of JVOIPLIB, a library designed to facilitate
    the use of Voice over IP (VoIP).

    Copyright (C) 2000-2002  Jori Liesenborgs (jori@lumumba.luc.ac.be)

    This library (JVOIPLIB) is based upon work done for my thesis at
    the School for Knowledge Technology (Belgium/The Netherlands)

    The full GNU Library General Public License can be found in the
    file LICENSE.LGPL which is included in the source code archive.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA

*/

#include "jvoiperrors.h"
#include <stdio.h>

struct JVOIPErrorDescription
{
	int errcode;
	const char *description;
};

static JVOIPErrorDescription errordescriptions[]={
	{ERR_JVOIPLIB_GENERAL_OUTOFMEM,"Out of memory"},
	{ERR_JVOIPLIB_GENERAL_CANTINITMUTEX,"Can't initialize a mutex"},
	{ERR_JVOIPLIB_GENERAL_CANTINITSIGWAIT,"Can't initialize a signal waiter"},
	{ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT,"Tried to initialize a component which was already initialized"},
	{ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT,"Tried to use a component without it being initialized"},
	{ERR_JVOIPLIB_GENERAL_ILLEGALSTATEPARAMETER,"An illegal component state parameter was passed"},
	{ERR_JVOIPLIB_GENERAL_VOICEBLOCKINFODOESNTMATCHSETTINGS,"The information in a voice block didn't match the current settings"},
	{ERR_JVOIPLIB_VOIPTHREAD_VOICECALLERROR,"An error occured in the main VoIP thread"},
	{ERR_JVOIPLIB_VOIPTHREAD_INTERVALACTIONERROR,"An error occurred in the action after a sample interval"},
	{ERR_JVOIPLIB_SIGWAIT_CANTCREATEPIPE,"Signal waiter initialization failed: unable to create a pipe"},
	{ERR_JVOIPLIB_SIGWAIT_NOTINIT,"Tried to use a signal waiter without it being initialized"},
	{ERR_JVOIPLIB_SIGWAIT_ALREADYINIT,"Tried to initialize a signal waiter but it is already initialized"},
	{ERR_JVOIPLIB_SESSION_ALREADYCREATED,"Tried to create a VoIP session but it was already active"},
	{ERR_JVOIPLIB_SESSION_NOTCREATED,"Tried to use a VoIP session which was not created"},
	{ERR_JVOIPLIB_SESSION_CANTREGUSERDEFINPUT,"Unable to register the user defined input module"},
	{ERR_JVOIPLIB_SESSION_CANTREGUSERDEFOUTPUT,"Unable to register the user defined output module"},
	{ERR_JVOIPLIB_SESSION_CANTREGUSERDEFLOCALISATION,"Unable to register the user defined localisation module"},
	{ERR_JVOIPLIB_SESSION_CANTREGUSERDEFCOMPRESSION,"Unable to register the user defined compression module"},
	{ERR_JVOIPLIB_SESSION_CANTREGUSERDEFTRANSMISSION,"Unable to register the user defined transmission module"},
	{ERR_JVOIPLIB_SESSION_CANTREGUSERDEFMIXER,"Unable to register the user defined mixer"},
	{ERR_JVOIPLIB_SESSION_CANTADDDESTINATION,"Couldn't add the specified destination"},
	{ERR_JVOIPLIB_SESSION_CANTDELETEDESTINATION,"Couldn't delete the specified destination"},
	{ERR_JVOIPLIB_SESSION_CANTJOINMULTICASTGROUP,"Unable to join the requested multicast group"},
	{ERR_JVOIPLIB_SESSION_CANTLEAVEMULTICASTGROUP,"Unable to leave the requested multicast group"},
	{ERR_JVOIPLIB_SESSION_UNSUPPORTEDRECEIVETYPE,"The receive type you specified is not supported by the transmission module"},
	{ERR_JVOIPLIB_SESSION_CANTSETRECEIVETYPE,"Unable to set the specified receive type"},
	{ERR_JVOIPLIB_SESSION_MULTICASTINGNOTSUPPORTED,"The current transmission module doesn't support multicasting"},
	{ERR_JVOIPLIB_SESSION_CANTADDTOACCEPTLIST,"Can't add the specified IP-port pair to the accept list"},
	{ERR_JVOIPLIB_SESSION_CANTADDTOIGNORELIST,"Can't add the specified IP-port pair to the ignore list"},
	{ERR_JVOIPLIB_SESSION_CANTDELETEFROMACCEPTLIST,"Can't delete the specified IP-port pair from the accept list"},
	{ERR_JVOIPLIB_SESSION_CANTDELETEFROMIGNORELIST,"Can't delete the specified IP-port pair from the ignore list"},
	{ERR_JVOIPLIB_SESSION_ILLEGALINPUTTYPE,"An illegal input type was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALOUTPUTTYPE,"An illegal output type was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALLOCALISATIONTYPE,"An illegal localisation type was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALCOMPRESSIONTYPE,"An illegal compression type was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALTRANSMISSIONTYPE,"An illegal transmission type was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALINPUTSAMPLINGRATE,"An illegal input sampling rate was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALOUTPUTSAMPLINGRATE,"An illegal output sampling rate was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALSAMPLEINTERVAL,"An illegal sample interval was specified"},
	{ERR_JVOIPLIB_SESSION_ILLEGALMIXERTYPE,"An mixer type was specified"},
	{ERR_JVOIPLIB_SESSION_CANTINITUSERDEFVOICEINPUT,"Couldn't initialize the user defined voice input module"},
	{ERR_JVOIPLIB_SESSION_CANTINITUSERDEFVOICEOUTPUT,"Couldn't initialize the user defined voice output module"},
	{ERR_JVOIPLIB_SESSION_CANTINITUSERDEFCOMPRESSIONMODULE,"Couldn't initialize the user defined compression module"},
	{ERR_JVOIPLIB_SESSION_CANTINITUSERDEFLOCALISATION,"Couldn't initialize the user defined localisation module"},
	{ERR_JVOIPLIB_SESSION_CANTINITUSERDEFMIXER,"Couldn't initialize the user defined mixer"},
	{ERR_JVOIPLIB_SESSION_CANTINITUSERDEFTRANSMISSION,"Couldn't initialize the user defined transmission module"},
	{ERR_JVOIPLIB_SESSION_USERDEFINPUTISNULL,"User defined input is NULL"},
	{ERR_JVOIPLIB_SESSION_USERDEFOUTPUTISNULL,"User defined output is NULL"},
	{ERR_JVOIPLIB_SESSION_USERDEFLOCALISATIONISNULL,"User defined localisation is NULL"},
	{ERR_JVOIPLIB_SESSION_USERDEFCOMPRESSIONISNULL,"User defined compression is NULL"},
	{ERR_JVOIPLIB_SESSION_USERDEFMIXERISNULL,"User defined mixer is NULL"},
	{ERR_JVOIPLIB_SESSION_USERDEFTRANSMISSIONISNULL,"User defined transmission is NULL"},
	{ERR_JVOIPLIB_SESSION_CANTSTARTVOICECALLTHREAD,"Can't start the main VoIP thread"},
	{ERR_JVOIPLIB_SESSION_UNUSEDCOMPONENT,"You requested information about a component type which isn't used"},
	{ERR_JVOIPLIB_SESSION_ILLEGALCOMPONENTTYPE,"You specified an illegal component type"},
	{ERR_JVOIPLIB_SESSION_ACTIONFAILEDANDRESTOREFAILED,"The action you requested failed and the previous state couldn't be restored. The VoIP session was destroyed."},
	{ERR_JVOIPLIB_SESSION_VOICEINPUTDOESNTSUPPORTSAMPINT,"The voice input module doesn't support the requested sample interval"},
	{ERR_JVOIPLIB_SESSION_VOICEOUTPUTDOESNTSUPPORTSAMPINT,"The voice output module doesn't support the requested sample interval"},
	{ERR_JVOIPLIB_SESSION_COMPRESSIONDOESNTSUPPORTSAMPINT,"The compression module doesn't support the requested sample interval"},
	{ERR_JVOIPLIB_SESSION_MIXERDOESNTSUPPORTSAMPINT,"The mixer doesn't support the requested sample interval"},
	{ERR_JVOIPLIB_SESSION_TRANSMISSIONDOESNTSUPPORTSAMPINT,"The transmission component doesn't support the requested sample interval"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEINPUTSAMPINT,"Can't set the sample interval for the user defined voice input module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEOUTPUTSAMPINT,"Can't set the sample interval for the user defined voice output module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFCOMPRESSIONSAMPINT,"Can't set the sample interval for the user defined compression module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFMIXERSAMPINT,"Can't set the sample interval for the user defined mixer"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFTRANSSAMPINT,"Can't set the sample interval for the user defined transmission module"},
	{ERR_JVOIPLIB_SESSION_VOICEINPUTDOESTSUPPORTINPUTSAMPRATE,"The input module doesn't support the specified input sampling rate"},
	{ERR_JVOIPLIB_SESSION_COMPRESSIONDOESNTSUPPORTINPUTSAMPRATE,"The compression module doesn't support the specified input sampling rate"},
	{ERR_JVOIPLIB_SESSION_TRANSMISSIONDOESNTSUPPORTINPUTSAMPRATE,"The transmission module doesn't support the specified input sampling rate"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEININPUTSAMPRATE,"Can't set the input sampling rate for the user defined input module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFCOMPRESSIONINPUTSAMPRATE,"Can't set the input sampling rate for the user defined compression module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFTRANSMISSIONINPUTSAMPRATE,"Can't set the input sampling rate for the user defined transmission module"},
	{ERR_JVOIPLIB_SESSION_VOICEOUTPUTDOESNTSUPPORTOUTPUTSAMPRATE,"The output module doesn't support the specified output sampling rate"},
	{ERR_JVOIPLIB_SESSION_LOCALISATIONDOESNTSUPPORTOUTPUTSAMPRATE,"The localisation module doesn't support the specified output sampling rate"},
	{ERR_JVOIPLIB_SESSION_MIXERDOESNTSUPPORTOUTPUTSAMPRATE,"The mixer doesn't support the specified output sampling rate"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEOUTOUTPUTSAMPRATE,"Can't set the output sampling rate for the user defined output module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFLOCALISATIONOUTPUTSAMPRATE,"Can't set the output sampling rate for the user defined localisation module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFMIXEROUTPUTSAMPRATE,"Can't set the output sampling rate for the user defined mixer"},
	{ERR_JVOIPLIB_SESSION_VOICEINPUTDOESNTSUPPORTINPUTENCODING,"The input module doesn't support the specified input encoding type"},
	{ERR_JVOIPLIB_SESSION_COMPRESSIONDOESNTSUPPORTINPUTENCODING,"The compression module doesn't support the specified input encoding type"},
	{ERR_JVOIPLIB_SESSION_TRANSMISSIONDOESNTSUPPORTINPUTENCODING,"The transmission module doesn't support the specified input encoding type"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEININPUTENCODING,"Can't set the input encoding type for the user defined voice input module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFCOMPRESSIONINPUTENCODING,"Can't set the input encoding type for the user defined compression module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFTRANSMISSIONINPUTENCODING,"Can't set the input encoding type for the user defined transmission module"},
	{ERR_JVOIPLIB_SESSION_VOICEOUTPUTDOESNTSUPPORTOUTPUTENCODING,"The output module doesn't support the specified output encoding type"},
	{ERR_JVOIPLIB_SESSION_LOCALISATIONDOESNTSUPPORTOUTPUTENCODING,"The localisation module doesn't support the specified output encoding type"},
	{ERR_JVOIPLIB_SESSION_MIXERDOESNTSUPPORTOUTPUTENCODING,"The mixer doesn't support the specified output encoding type"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEOUTOUTPUTENCODING,"Can't set the output encoding type for the user defined voice output module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFLOCALISATIONOUTPUTENCODING,"Can't set the output encoding type for the user defined localisation module"},
	{ERR_JVOIPLIB_SESSION_CANTSETUSERDEFMIXEROUTPUTENCODING,"Can't set the output encoding type for the user defined mixer"},	
	{ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFINPUTSTATE,"Unable to save the state of the user defined input module"},
	{ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFOUTPUTSTATE,"Unable to save the state of the user defined output module"},
	{ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFLOCALISATIONSTATE,"Unable to save the state of the user defined localisation module"},
	{ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFMIXERSTATE,"Unable to save the state of the user defined mixer"},
	{ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFTRANSMISSIONSTATE,"Unable to save the state of the user defined transmission module"},
	{ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFCOMPRESSIONSTATE,"Unable to save the state of the user defined compression module"},
	{ERR_JVOIPLIB_SESSION_USERDEFINEDCREATEFAILED,"An error occurred in the user defined create routine"},
	{ERR_JVOIPLIB_SOUNDCARDIO_ALREADYOPENEDFORREADING,"The sound device is already opened for reading"},
	{ERR_JVOIPLIB_SOUNDCARDIO_ALREADYOPENEDFORWRITING,"The sound device is already opened for writing"},
	{ERR_JVOIPLIB_SOUNDCARDIO_CANTOPENDEVICE,"Couldn't open the sound device"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DEVICECLOSED,"Tried to use the sound device but it was already closed"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DEVICENOTOPENEDFORINPUT,"The soundcard device is not opened for reading"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DRIVERALREADYINIT,"The sound device driver is already initialized"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DRIVERNOTINIT,"The sound device driver was not initialized"},
	{ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM,"Unable to set a soundcard driver parameter"},
	{ERR_JVOIPLIB_SOUNDCARDIO_CANTSTARTTHREAD,"Couldn't start the sound device driver thread"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DRIVERALREADYREGISTERED,"The sound device driver was already registered"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DRIVERHASNOINPUTTHREAD,"The sound driver has no input thread"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DRIVERHASNOOUTPUTTHREAD,"The sound driver has no output thread"},
	{ERR_JVOIPLIB_SAMPCONV_BADCONVERSIONPARAM,"An illegal sample conversion parameter was specified"},
	{ERR_JVOIPLIB_SIMPLETIMER_CANTSTARTTHREAD,"Couldn't start the thread for the simple timer"},
	{ERR_JVOIPLIB_RTPTRANS_UNSUPPORTEDSAMPLERATE,"The RTP transmission module doesn't support the specified sampling rate"},
	{ERR_JVOIPLIB_RTPTRANS_CANTINITRTPSESSION,"Couldn't initialized the RTP session"},
	{ERR_JVOIPLIB_RTPTRANS_CANTSTARTTHREAD,"The RTP transmission module couldn't start it's background thread"},
	{ERR_JVOIPLIB_RTPTRANS_CANTPOLLFORDATA,"The RTP transmission module was unable to poll for incoming data"},
	{ERR_JVOIPLIB_RTPTRANS_CANTREJOINMULTICASTGROUP,"The RTP transmission was unable to rejoin its multicast groups"},
	{ERR_JVOIPLIB_RTPTRANS_CANTREINSTALLDESTINATIONS,"The RTP transmission module couldn't reinstall its destinations"},
	{ERR_JVOIPLIB_RTPTRANS_CANTREINSTALLACCEPTLIST,"The RTP transmission module couldn't reinstall its accept list"},
	{ERR_JVOIPLIB_RTPTRANS_CANTREINSTALLIGNORELIST,"The RTP transmission module couldn't reinstall its ignore list"},
	{ERR_JVOIPLIB_RTPTRANS_CANTINCREMENTTIMESTAMP,"The RTP transmission module couldn't increment the timestamp"},
	{ERR_JVOIPLIB_RTPTRANS_CANTSENDPACKET,"The RTP transmission module couldn't send a packet"},
	{ERR_JVOIPLIB_RTPTRANS_PACKETTOOLARGE,"The RTP transmission module couldn't process data because the packet was too large"},
	{ERR_JVOIPLIB_DPCMCOMP_BADFORMAT,"The DPCM compressor couldn't process data: bad data format"},
	{ERR_JVOIPLIB_DPCMCOMP_TOOMUCHDATA,"The DPCM compressor couldn't process data: block too large"}, 
	{ERR_JVOIPLIB_HRTFLOC_UNSUPPORTEDSAMPLERATE,"The HRTF localisation module needs an output sample rate of 44100Hz"},
	{ERR_JVOIPLIB_GSMCOMP_UNSUPPORTEDSAMPLERATE,"The GSM compression module needs an input sample rate of 8000Hz"},
	{ERR_JVOIPLIB_GSMCOMP_UNSUPPORTEDSAMPLEINTERVAL,"The GSM compression module needs a sample interval which is a multiple of 20ms"},
	{ERR_JVOIPLIB_GSMCOMP_CANTSTARTTHREAD,"Can't start GSM compression thread"},
	{ERR_JVOIPLIB_LPCCOMP_UNSUPPORTEDSAMPLERATE,"The LPC compression module needs an input sample rate of 8000Hz"},
	{ERR_JVOIPLIB_LPCCOMP_UNSUPPORTEDSAMPLEINTERVAL,"The LPC compression module needs a sample interval which is a multiple of 20ms"},
	{ERR_JVOIPLIB_LPCCOMP_CANTSTARTTHREAD,"Can't start LPC compression thread"},
	{ERR_JVOIPLIB_SOUNDCARDIO_COULDNTREOPENDEVICE,"Could not reopen soundcard device"},
	{ERR_JVOIPLIB_SOUNDCARDIO_DRIVERSHOULDNTBEUSEDAFTERBADREOPEN,"Tried to use the driver after a failure in reopening the soundcard device"},
	{ERR_JVOIPLIB_SESSION_DESTRUCTORCALLEDDURINGACTIVESESSION,"The session's destructor was called while the session was still active. Use the 'Destroy' method to terminate it first"},
	{0,NULL} };

std::string JVOIPGetErrorString(int errcode)
{
	int i;

	if (errcode >= 0)
		return std::string("No error");

	i = 0;
	while (errordescriptions[i].errcode < 0)
	{
		if (errordescriptions[i].errcode == errcode)
			return std::string(errordescriptions[i].description);
		i++;
	}

	char errnumstr[256];
	std::string errstr; 

	errstr = "Unknown error code ";
	sprintf(errnumstr,"%d",errcode);
	errstr += errnumstr;
	return errstr;
}

