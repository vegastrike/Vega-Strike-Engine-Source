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

#include "jvoipsession.h"
#include "jvoiperrors.h"
#include "jvoiptransmission.h"
#include "jvoipvoiceinput.h"
#include "jvoipvoiceoutput.h"
#include "jvoipcompression.h"
#include "jvoipcompressionmodule.h"
#include "jvoiplocalisation.h"
#include "jvoipmixer.h"
#include "jvoipsigwait.h"
#include "jvoipsoundcardinput.h"
#include "jvoipsoundcardoutput.h"
#include "jvoipnormalmixer.h"
#include "jvoiprtptransmission.h"
#include "jvoipnoinput.h"
#include "jvoipnooutput.h"
#include "jvoipsimplelocalisation.h"
#include "jvoiphrtflocalisation.h"
#include <iostream>

#ifdef WIN32
	#include <sys/timeb.h>

	inline void JVOIPSession::GetTheCurrentTime(struct timeval *tv)
	{
		struct _timeb tb;
		
	        _ftime(&tb);
        	tv->tv_sec = tb.time;
	        tv->tv_usec = tb.millitm*1000;
	}

#else // unix-like systems
	inline void JVOIPSession::GetTheCurrentTime(struct timeval *tv)
	{
		gettimeofday(tv,NULL);
	}
#endif // WIN32

#include "debugnew.h"

JVOIPSession::JVOIPSession() throw (JVOIPException) : voipthread(*this)
{
	destroying = false;
	created = false;
	if (componentmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	ClearComponents();
}

JVOIPSession::~JVOIPSession() throw (JVOIPException)
{
	// NOTE: Here, we'll only check if the session is already destroyed or not. If not,
	// an error is generated. It is impossible to simply call the 'Destroy' function, since
	// this might give problems in case of inheritance: a child class of JVOIPSession will
	// be destroyed BEFORE JVOIPSession itself. So, any virtual functions which are called
	// at this point, can cause an error.
	componentmutex.Lock();
	if (created)
	{
		std::cerr << "JVOIPLIB: Error! " << std::endl
		          << "  The destructor of JVOIPSession has been called without the session being destroyed." << std::endl
	                  << "  To solve this problem, make sure you explicitly call JVOIPSession::Destroy before the" << std::endl
		          << "  the JVOIPSession object's destructor is called." << std::endl;
	
		// signal that we're in the destructor
		destroying = true;
		
		voicecall.Cleanup();
		voipthread.SignalStop();
	
		// we MUST unlock the component mutex: otherwise the VoiceCall thread will block
		componentmutex.Unlock();
		voipthread.WaitForStop();
	
		componentmutex.Lock();
		GeneralDestroyActions();
		
		componentmutex.Unlock();
		throw JVOIPException(ERR_JVOIPLIB_SESSION_DESTRUCTORCALLEDDURINGACTIVESESSION);
	}
	else
		componentmutex.Unlock();
}

#define CLEANRETURN(errcode) 	{\
					DeleteComponents();\
					UnregisterUserDefinedComponents();\
					ClearComponents();\
					componentmutex.Unlock();\
					return errcode;\
				}
				
#define CHECKERROR(errcode,isuserdefined,udeferror)	{\
								if (status < 0)\
								{\
									if (isuserdefined)\
									{\
										CLEANRETURN(udeferror);\
									}\
									CLEANRETURN(errcode);\
								}\
							}

int JVOIPSession::Create(const JVOIPSessionParams &cp)
{
	int status;
	int sampinrate,sampoutrate,sampinterval;
	int inputbytespersample,outputbytespersample;
	JVOIPSessionParams::SampleEncodingType inputenc,outputenc;
	bool needstereo;

	// first check some parameters
	
	if ((sampinrate = cp.GetInputSamplingRate()) < 1)
		return ERR_JVOIPLIB_SESSION_ILLEGALINPUTSAMPLINGRATE;
	if ((sampoutrate = cp.GetOutputSamplingRate()) < 1)
		return ERR_JVOIPLIB_SESSION_ILLEGALOUTPUTSAMPLINGRATE;
	if ((sampinterval = cp.GetSampleInterval()) < 1 || sampinterval > 5000)
		return ERR_JVOIPLIB_SESSION_ILLEGALSAMPLEINTERVAL;
	
	componentmutex.Lock();
	if (created)
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_ALREADYCREATED;
	}

	// make sure all the components are NULLed
	ClearComponents();
	
	// register the user defined components
	status = RegisterUserDefinedComponents();
	if (status < 0)
	{
		componentmutex.Unlock();
		return status;
	}
	
	///////////////////////////////////////////////////////////////////////////////
	//
	// process component types
	//
	///////////////////////////////////////////////////////////////////////////////

	if ((status = ProcessInputType(cp.GetVoiceInputType(),&call_voicein)) < 0)
		CLEANRETURN(status);
	if ((status = ProcessOutputType(cp.GetVoiceOutputType(),&call_voiceout)) < 0)
		CLEANRETURN(status);
			
	if (cp.GetLocalisationType() != JVOIPSessionParams::NoLocalisation)
		needstereo = true;
	else
		needstereo = false;
	if ((status = ProcessLocalisationType(cp.GetLocalisationType(),&call_loc)) < 0)
		CLEANRETURN(status);
	
	call_comp = new JVOIPCompression(this,udef_compmod);
	if (call_comp == NULL)
		CLEANRETURN(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
	
	if ((status = ProcessMixerType(cp.GetMixerType(),&call_mixer)) < 0)
		CLEANRETURN(status);
	if ((status = ProcessTransmissionType(cp.GetTransmissionType(),&call_trans)) < 0)
		CLEANRETURN(status);
	
	///////////////////////////////////////////////////////////////////////////////
	//
	// Do initialisation
	//
	///////////////////////////////////////////////////////////////////////////////
	
	inputenc = cp.GetInputSampleEncodingType();
	outputenc = cp.GetOutputSampleEncodingType();
	
	inputbytespersample = (inputenc == JVOIPSessionParams::EightBit)?1:2;
	outputbytespersample = (outputenc == JVOIPSessionParams::EightBit)?1:2;
	
	status = call_voicein->Init(sampinterval,sampinrate,inputbytespersample,cp.GetVoiceInputParams());
	CHECKERROR(status,call_voicein == udef_voicein,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFVOICEINPUT);
	
	status = call_voiceout->Init(sampinterval,sampoutrate,outputbytespersample,needstereo,cp.GetVoiceOutputParams());
	CHECKERROR(status,call_voiceout == udef_voiceout,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFVOICEOUTPUT);
	
	status = call_comp->Init(sampinterval,sampinrate,inputbytespersample,cp.GetCompressionType(),cp.GetCompressionParams());
	if (status < 0)
		CLEANRETURN(status);

	if (call_loc)
	{	
		status = call_loc->Init(sampoutrate,outputbytespersample,cp.GetLocalisationParams());
		CHECKERROR(status,call_loc == udef_loc,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFLOCALISATION);
	}
	
	status = call_mixer->Init(sampinterval,sampoutrate,outputbytespersample,needstereo,cp.GetMixerParams());
	CHECKERROR(status,call_mixer == udef_mixer,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFMIXER);
	
	status = call_trans->Init(sampinterval,sampinrate,inputbytespersample,cp.GetTransmissionParams());
	CHECKERROR(status,call_trans == udef_trans,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFTRANSMISSION);
	
	// Get the samplingtimer
	call_timer = call_voicein->GetSamplingTimer();
	if (call_timer == NULL)
		CLEANRETURN(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
	sigwait = call_timer->GetSignalWaiter();

	// Set all the VoIP components
	voicecall.Cleanup();
	voicecall.SetIOComponents(call_voicein,call_voiceout);
	voicecall.SetCompressionComponents(call_comp,call_comp);
	voicecall.Set3DComponents(call_loc,call_loc);
	voicecall.SetMixer(call_mixer);
	voicecall.SetTransmitter(call_trans);
	voicecall.SetSamplingTimer(call_timer);
	
	// initialisation for the default interval action
	firstpass =  true;
	
	// allow the user to do some initialisation of its own
	if (!UserDefinedCreate())
		CLEANRETURN(ERR_JVOIPLIB_SESSION_USERDEFINEDCREATEFAILED);
	
	// start the VoiceCall thread
        if (voipthread.Start() < 0)
        {
        	UserDefinedDestroy();
        	CLEANRETURN(ERR_JVOIPLIB_SESSION_CANTSTARTVOICECALLTHREAD);
        }
		
        try // in case we've run out of memory
        {
		sessparams = cp;
	}
	catch (JVOIPException e)
	{
        	UserDefinedDestroy();
		CLEANRETURN(e.GetErrorNumber());
	}
	
	created = true;		
	componentmutex.Unlock();

	// REMARK:	
	// Note that only at this point the VoiceCall thread actually starts running,
	// since now we have unlocked the componentmutex.
	
	return 0;
}

#undef CLEANRETURN
#undef CHECKERROR

#define CHECKCREATE_ERROR \
	{\
		componentmutex.Lock();\
		if (!created)\
		{\
			componentmutex.Unlock();\
			return ERR_JVOIPLIB_SESSION_NOTCREATED;\
		}\
	}

#define CHECKCREATE_NOERROR \
	{\
		componentmutex.Lock();\
		if (!created)\
		{\
			componentmutex.Unlock();\
			return;\
		}\
	}
	
#define UNLOCK_RETURN_VAL(x) \
	{\
		componentmutex.Unlock();\
		return (x);\
	}

bool JVOIPSession::IsActive()
{
	bool a;

	componentmutex.Lock();
	a = created;
	componentmutex.Unlock();
	return a;
}

int JVOIPSession::Destroy()
{
	CHECKCREATE_ERROR;
	
	
	// clear
	voicecall.Cleanup();
	voipthread.SignalStop();
	
	// we MUST unlock the component mutex: otherwise the VoiceCall thread will block
	componentmutex.Unlock();
	voipthread.WaitForStop();
	
	componentmutex.Lock();
	GeneralDestroyActions();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::AddDestination(JVOIPuint32 destip,JVOIPuint16 destportbase)
{
	CHECKCREATE_ERROR;
	
	bool ok = call_trans->AddDestination(destip,destportbase);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTADDDESTINATION;
	return 0;
}

int JVOIPSession::DeleteDestination(JVOIPuint32 destip,JVOIPuint16 destportbase)
{
	CHECKCREATE_ERROR;
	
	bool ok = call_trans->DeleteDestination(destip,destportbase);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTDELETEDESTINATION;
	return 0;
}

void JVOIPSession::ClearDestinations()
{
	CHECKCREATE_NOERROR;
	
	call_trans->ClearDestinations();
	componentmutex.Unlock();
}

int JVOIPSession::JoinMulticastGroup(JVOIPuint32 mcastip)
{
	CHECKCREATE_ERROR;
	
	if (!call_trans->SupportsMulticasting())
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_MULTICASTINGNOTSUPPORTED;
	}
	bool ok = call_trans->JoinMulticastGroup(mcastip);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTJOINMULTICASTGROUP;
	return 0;
}

int JVOIPSession::LeaveMulticastGroup(JVOIPuint32 mcastip)
{
	CHECKCREATE_ERROR;
	
	if (!call_trans->SupportsMulticasting())
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_MULTICASTINGNOTSUPPORTED;
	}
	bool ok = call_trans->LeaveMulticastGroup(mcastip);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTLEAVEMULTICASTGROUP;
	return 0;
}

void JVOIPSession::LeaveAllMulticastGroups()
{
	CHECKCREATE_NOERROR;
	
	if (!call_trans->SupportsMulticasting())
	{
		componentmutex.Unlock();
		return;
	}
	call_trans->LeaveAllMulticastGroups();
	componentmutex.Unlock();
}

int JVOIPSession::SetReceiveType(JVOIPSessionParams::ReceiveType rt)
{
	CHECKCREATE_ERROR;

	if (!call_trans->SupportsReceiveType(rt))
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_UNSUPPORTEDRECEIVETYPE;
	}
	bool ok = call_trans->SetReceiveType(rt);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTSETRECEIVETYPE;
	return 0;
}

int JVOIPSession::AddToAcceptList(JVOIPuint32 ip,JVOIPuint16 port)
{
	CHECKCREATE_ERROR;
	
	if (!call_trans->SupportsReceiveType(JVOIPSessionParams::AcceptSome))
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_UNSUPPORTEDRECEIVETYPE;
	}
	bool ok = call_trans->AddToAcceptList(ip,port);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTADDTOACCEPTLIST;
	return 0;
}

int JVOIPSession::DeleteFromAcceptList(JVOIPuint32 ip,JVOIPuint16 port)
{
	CHECKCREATE_ERROR;
	
	if (!call_trans->SupportsReceiveType(JVOIPSessionParams::AcceptSome))
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_UNSUPPORTEDRECEIVETYPE;
	}
	bool ok = call_trans->DeleteFromAcceptList(ip,port);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTDELETEFROMACCEPTLIST;
	return 0;
}

void JVOIPSession::ClearAcceptList()
{
	CHECKCREATE_NOERROR;
	
	call_trans->ClearAcceptList();
	componentmutex.Unlock();
}

int JVOIPSession::AddToIgnoreList(JVOIPuint32 ip,JVOIPuint16 port)
{
	CHECKCREATE_ERROR;
	
	if (!call_trans->SupportsReceiveType(JVOIPSessionParams::IgnoreSome))
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_UNSUPPORTEDRECEIVETYPE;
	}
	bool ok = call_trans->AddToIgnoreList(ip,port);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTADDTOIGNORELIST;
	return 0;
}

int JVOIPSession::DeleteFromIgnoreList(JVOIPuint32 ip,JVOIPuint16 port)
{
	CHECKCREATE_ERROR;
	
	if (!call_trans->SupportsReceiveType(JVOIPSessionParams::IgnoreSome))
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_UNSUPPORTEDRECEIVETYPE;
	}
	bool ok = call_trans->DeleteFromIgnoreList(ip,port);
	componentmutex.Unlock();
	if (!ok)
		return ERR_JVOIPLIB_SESSION_CANTDELETEFROMIGNORELIST;
	return 0;
}

void JVOIPSession::ClearIgnoreList()
{
	CHECKCREATE_NOERROR;
	
	call_trans->ClearIgnoreList();
	componentmutex.Unlock();
}

bool JVOIPSession::IntervalAction(bool *waitforsignal)
{
	struct timeval curtv;
	bool reset;
	double msecdiff,diff;
	
	reset = false;
	if (firstpass)
	{
		GetTheCurrentTime(&starttime);
		loopcount = 1;
		firstpass = false;
	}
	else
	{
		GetTheCurrentTime(&curtv);

		if (curtv.tv_sec - starttime.tv_sec > 10) // wait a least 10 seconds (to ensure some accuracy)
		{
			msecdiff = ((double)(curtv.tv_sec - starttime.tv_sec))*1000.0;
			msecdiff += ((double)curtv.tv_usec - (double)starttime.tv_usec)/1000.0;
			diff = (msecdiff-((double)sessparams.GetSampleInterval())*loopcount);
			if (diff > 200.0)
				reset = true;
			else if (diff < -10.0)
				reset = true;

		}
		loopcount++;
	}

	if (reset)
	{
		ResetActions();
		
		// since we've reset the input device, there won't be a signal to wait
		// for...
		*waitforsignal = false;
	}
	return true;
}

#define DELNULL(x) 	{\
				if (x)\
				{\
					delete x;\
			 		x = NULL;\
			 	}\
			}

int JVOIPSession::RegisterUserDefinedComponents()
{
	if (!RegisterUserDefinedInput(&udef_voicein))
		return ERR_JVOIPLIB_SESSION_CANTREGUSERDEFINPUT;
	if (!RegisterUserDefinedOutput(&udef_voiceout))
	{
		DELNULL(udef_voicein);
		return ERR_JVOIPLIB_SESSION_CANTREGUSERDEFOUTPUT;
	}
	if (!RegisterUserDefinedLocalisation(&udef_loc))
	{
		DELNULL(udef_voicein);
		DELNULL(udef_voiceout);
		return ERR_JVOIPLIB_SESSION_CANTREGUSERDEFLOCALISATION;
	}
	if (!RegisterUserDefinedCompressionModule(&udef_compmod))
	{
		DELNULL(udef_voicein);
		DELNULL(udef_voiceout);
		DELNULL(udef_loc);
		return ERR_JVOIPLIB_SESSION_CANTREGUSERDEFCOMPRESSION;
	}
	if (!RegisterUserDefinedTransmission(&udef_trans))
	{
		DELNULL(udef_voicein);
		DELNULL(udef_voiceout);
		DELNULL(udef_loc);
		DELNULL(udef_compmod);
		return ERR_JVOIPLIB_SESSION_CANTREGUSERDEFTRANSMISSION;
	}
	if (!RegisterUserDefinedMixer(&udef_mixer))
	{
		DELNULL(udef_voicein);
		DELNULL(udef_voiceout);
		DELNULL(udef_loc);
		DELNULL(udef_compmod);
		DELNULL(udef_trans);
		return ERR_JVOIPLIB_SESSION_CANTREGUSERDEFMIXER;
	}
	
	return 0;
}

#undef DELNULL

void JVOIPSession::ClearComponents()
{
	call_voicein = NULL;
	call_voiceout = NULL;
	call_loc = NULL;
	call_comp = NULL;
	call_trans = NULL;
	call_timer = NULL;
	call_mixer = NULL;
	udef_voicein = NULL;
	udef_voiceout = NULL;
	udef_loc = NULL;
	udef_compmod = NULL;
	udef_trans = NULL;
	udef_mixer = NULL;
	sigwait = NULL;
}

void JVOIPSession::DeleteComponents()
{
	if (call_voicein && call_voicein != udef_voicein)
		delete call_voicein;
	if (call_voiceout && call_voiceout != udef_voiceout)
		delete call_voiceout;
	if (call_loc && call_loc != udef_loc)
		delete call_loc;
	if (call_comp)
		delete call_comp;
	if (call_trans && call_trans != udef_trans)
		delete call_trans;
	if (call_mixer && call_mixer != udef_mixer)
		delete call_mixer;
	call_voicein = NULL;
	call_voiceout = NULL;
	call_loc = NULL;
	call_comp = NULL;
	call_trans = NULL;
	call_mixer = NULL;
	call_timer = NULL;
}

void JVOIPSession::UnregisterUserDefinedComponents()
{
	UnregisterUserDefinedInput(udef_voicein);
	UnregisterUserDefinedOutput(udef_voiceout);
	UnregisterUserDefinedLocalisation(udef_loc);
	UnregisterUserDefinedCompressionModule(udef_compmod);
	UnregisterUserDefinedTransmission(udef_trans);
	UnregisterUserDefinedMixer(udef_mixer);
}

void JVOIPSession::ThreadFinished(int threaderr,int voicecallerr,int componenterr)
{
	if (!destroying)
		ThreadFinishedHandler(threaderr,voicecallerr,componenterr);
	GeneralDestroyActions();
}

void JVOIPSession::GeneralDestroyActions()
{
	created = false;
	if (!destroying)
		UserDefinedDestroy();
	DeleteComponents();
	if (!destroying)
		UnregisterUserDefinedComponents();
	ClearComponents();
	voicecall.Cleanup();
}

int JVOIPSession::ProcessInputType(JVOIPSessionParams::VoiceInputType it,JVOIPVoiceInput **vi)
{
	JVOIPVoiceInput *voiceinput = NULL;
	
	switch(it)
	{
	case JVOIPSessionParams::UserDefinedInput:
		if (!udef_voicein)
			return ERR_JVOIPLIB_SESSION_USERDEFINPUTISNULL;
		voiceinput = udef_voicein;
		break;
	case JVOIPSessionParams::SoundcardInput:
		voiceinput = new JVOIPSoundcardInput(this);
		break;
	case JVOIPSessionParams::NoInput:
		voiceinput = new JVOIPNoInput(this);
		break;
	default:
		return ERR_JVOIPLIB_SESSION_ILLEGALINPUTTYPE;
	}
	if (voiceinput == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	*vi = voiceinput;
	return 0;
}

int JVOIPSession::ProcessOutputType(JVOIPSessionParams::VoiceOutputType ot,JVOIPVoiceOutput **vo)
{
	JVOIPVoiceOutput *voiceoutput = NULL;
	
	switch(ot)
	{
	case JVOIPSessionParams::UserDefinedOutput:
		if (!udef_voiceout)
			return ERR_JVOIPLIB_SESSION_USERDEFOUTPUTISNULL;
		voiceoutput = udef_voiceout;
		break;
	case JVOIPSessionParams::SoundcardOutput:
		voiceoutput = new JVOIPSoundcardOutput(this);
		break;
	case JVOIPSessionParams::NoOutput:
		voiceoutput = new JVOIPNoOutput(this);
		break;
	default:
		return ERR_JVOIPLIB_SESSION_ILLEGALOUTPUTTYPE;
	}
	if (voiceoutput == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	*vo = voiceoutput;
	return 0;
}

int JVOIPSession::ProcessLocalisationType(JVOIPSessionParams::LocalisationType lt,JVOIPLocalisation **loc)
{
	JVOIPLocalisation *localisation = NULL;
	
	switch(lt)
	{
	case JVOIPSessionParams::NoLocalisation:
		break;
	case JVOIPSessionParams::UserDefinedLocalisation:
		if (!udef_loc)
			return ERR_JVOIPLIB_SESSION_USERDEFLOCALISATIONISNULL;
		localisation = udef_loc;
		break;
	case JVOIPSessionParams::SimpleLocalisation:
		localisation = new JVOIPSimpleLocalisation(this);
		break;
	case JVOIPSessionParams::HRTFLocalisation:
		localisation = new JVOIPHRTFLocalisation(this);
		break;
	default:
		return ERR_JVOIPLIB_SESSION_ILLEGALLOCALISATIONTYPE;
	}
	if (lt != JVOIPSessionParams::NoLocalisation && localisation == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	*loc = localisation;
	return 0;
}

int JVOIPSession::ProcessMixerType(JVOIPSessionParams::MixerType mt,JVOIPMixer **mix)
{
	JVOIPMixer *mixer = NULL;
	
	switch(mt)
	{
	case JVOIPSessionParams::UserDefinedMixer:
		if (!udef_mixer)
			return ERR_JVOIPLIB_SESSION_USERDEFMIXERISNULL;
		mixer = udef_mixer;
		break;
	case JVOIPSessionParams::NormalMixer:
		mixer = new JVOIPNormalMixer(this);
		break;
	default:
		return ERR_JVOIPLIB_SESSION_ILLEGALMIXERTYPE;
	}
	if (mixer == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	*mix = mixer;
	return 0;
}

int JVOIPSession::ProcessTransmissionType(JVOIPSessionParams::TransmissionType tt,JVOIPTransmission **trans)
{
	JVOIPTransmission *transmission = NULL;
	
	switch(tt)
	{
	case JVOIPSessionParams::UserDefinedTransmission:
		if (!udef_trans)
			return ERR_JVOIPLIB_SESSION_USERDEFTRANSMISSIONISNULL;
		transmission = udef_trans;
		break;
	case JVOIPSessionParams::RTP:
		transmission = new JVOIPRTPTransmission(this);
		break;
	default:
		return ERR_JVOIPLIB_SESSION_ILLEGALTRANSMISSIONTYPE;
	}
	if (transmission == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	*trans = transmission;
	return 0;
}

int JVOIPSession::GetSampleInterval(JVOIPuint16 *iv)
{
	CHECKCREATE_ERROR;
	*iv = sessparams.GetSampleInterval();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetInputSamplingRate(JVOIPuint16 *ir)
{
	CHECKCREATE_ERROR;
	*ir = sessparams.GetInputSamplingRate();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetOutputSamplingRate(JVOIPuint16 *outr)
{
	CHECKCREATE_ERROR;
	*outr = sessparams.GetOutputSamplingRate();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetInputSampleEncodingType(JVOIPSessionParams::SampleEncodingType *ie)
{
	CHECKCREATE_ERROR;
	*ie = sessparams.GetInputSampleEncodingType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetOutputSampleEncodingType(JVOIPSessionParams::SampleEncodingType *oe)
{
	CHECKCREATE_ERROR;
	*oe = sessparams.GetOutputSampleEncodingType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetVoiceInputType(JVOIPSessionParams::VoiceInputType *i)
{
	CHECKCREATE_ERROR;
	*i = sessparams.GetVoiceInputType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetVoiceOutputType(JVOIPSessionParams::VoiceOutputType *o)
{
	CHECKCREATE_ERROR;
	*o = sessparams.GetVoiceOutputType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetLocalisationType(JVOIPSessionParams::LocalisationType *l)
{
	CHECKCREATE_ERROR;
	*l = sessparams.GetLocalisationType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetCompressionType(JVOIPSessionParams::CompressionType *c)
{
	CHECKCREATE_ERROR;
	*c = sessparams.GetCompressionType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetMixerType(JVOIPSessionParams::MixerType *m)
{
	CHECKCREATE_ERROR;
	*m = sessparams.GetMixerType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetTransmissionType(JVOIPSessionParams::TransmissionType *t)
{
	CHECKCREATE_ERROR;
	*t = sessparams.GetTransmissionType();
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::GetReceiveType(JVOIPSessionParams::ReceiveType *r)
{
	CHECKCREATE_ERROR;
	*r = sessparams.GetReceiveType();
	UNLOCK_RETURN_VAL(0);
}

std::string JVOIPSession::GetComponentName(ComponentType comptype)
{
	componentmutex.Lock();
	if (!created)
	{
		componentmutex.Unlock();
		return std::string("Error: session not created");
	}
	
	std::string str;
	JVOIPComponent *component;
	
	switch(comptype)
	{
	case VoiceInput:
		component = call_voicein;
		break;
	case VoiceOutput:
		component = call_voiceout;
		break;
	case Localisation:
		component = call_loc;
		break;
	case Compression:
		component = call_comp;
		break;
	case Transmission:
		component = call_trans;
		break;
	case Mixer:
		component = call_mixer;
		break;
	default:
		componentmutex.Unlock();
		return std::string("Error: illegal component type");	
	}
	if (component == NULL)
		str = std::string("Component unused");
	else
		str = component->GetComponentName();
	componentmutex.Unlock();
	return str;
}

std::string JVOIPSession::GetComponentDescription(ComponentType comptype)
{
	componentmutex.Lock();
	if (!created)
	{
		componentmutex.Unlock();
		return std::string("Error: session not created");
	}
	
	std::string str;
	JVOIPComponent *component;
	
	switch(comptype)
	{
	case VoiceInput:
		component = call_voicein;
		break;
	case VoiceOutput:
		component = call_voiceout;
		break;
	case Localisation:
		component = call_loc;
		break;
	case Compression:
		component = call_comp;
		break;
	case Transmission:
		component = call_trans;
		break;
	case Mixer:
		component = call_mixer;
		break;
	default:
		componentmutex.Unlock();
		return std::string("Error: illegal component type");	
	}
	if (component == NULL)
		str = std::string("Component unused");
	else
		str = component->GetComponentDescription();
	componentmutex.Unlock();
	return str;
}

int JVOIPSession::GetComponentParameters(ComponentType comptype,std::vector<JVOIPCompParamInfo> **params)
{
	std::vector<JVOIPCompParamInfo> *componentparams;
	JVOIPComponent *component;
	
	CHECKCREATE_ERROR;
	
	switch(comptype)
	{
	case VoiceInput:
		component = call_voicein;
		break;
	case VoiceOutput:
		component = call_voiceout;
		break;
	case Localisation:
		component = call_loc;
		break;
	case Compression:
		component = call_comp;
		break;
	case Transmission:
		component = call_trans;
		break;
	case Mixer:
		component = call_mixer;
		break;
	default:
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_ILLEGALCOMPONENTTYPE;	
	}
	if (component == NULL)
	{
		componentmutex.Unlock();
		return ERR_JVOIPLIB_SESSION_UNUSEDCOMPONENT;
	}
	else
	{
		try
		{
			componentparams = component->GetComponentParameters();
		}
		catch(JVOIPException) // should only occur on out of memory
		{
			UNLOCK_RETURN_VAL(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
		}
	}
	*params = componentparams;
	UNLOCK_RETURN_VAL(0);
}

#define RESTORE_FAILED_ACTION \
	{\
		voipthread.SignalStop();\
		componentmutex.Unlock();\
		voipthread.WaitForStop();\
		componentmutex.Lock();\
		GeneralDestroyActions();\
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_ACTIONFAILEDANDRESTOREFAILED)\
	}
	
#define DESTROY_SESSION(status) \
	{\
		voipthread.SignalStop();\
		componentmutex.Unlock();\
		voipthread.WaitForStop();\
		componentmutex.Lock();\
		GeneralDestroyActions();\
		UNLOCK_RETURN_VAL(status)\
	}

#define UNLOCK_COMPONENTERROR(errcode,isuserdefined,userdefinederror) \
	{\
		if (isuserdefined)\
			UNLOCK_RETURN_VAL(userdefinederror);\
		UNLOCK_RETURN_VAL(errcode);\
	}

int JVOIPSession::SetSampleInterval(JVOIPuint16 ival)
{
	if (ival < 1 || ival > 5000)
		return ERR_JVOIPLIB_SESSION_ILLEGALSAMPLEINTERVAL;
	
	CHECKCREATE_ERROR;
	if (ival == sessparams.GetSampleInterval())
		UNLOCK_RETURN_VAL(0);
	
	// first we'll check if the components support the sample interval
	if (!call_voicein->SupportsSampleInterval(ival))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_VOICEINPUTDOESNTSUPPORTSAMPINT);
	if (!call_voiceout->SupportsSampleInterval(ival))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_VOICEOUTPUTDOESNTSUPPORTSAMPINT);
	if (!call_comp->SupportsSampleInterval(ival))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_COMPRESSIONDOESNTSUPPORTSAMPINT);
	if (!call_mixer->SupportsSampleInterval(ival))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_MIXERDOESNTSUPPORTSAMPINT);
	if (!call_trans->SupportsSampleInterval(ival))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_TRANSMISSIONDOESNTSUPPORTSAMPINT);

	// now, we'll try to set the sampling interval, if it fails, we'll try to reset
	// the old sampling interval
	
	int oldsampleinterval = sessparams.GetSampleInterval();
	int status;
	
	if ((status = call_voicein->SetSampleInterval(ival)) < 0)
	{
		if (call_voicein->SetSampleInterval(oldsampleinterval) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_voicein == udef_voicein,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEINPUTSAMPINT);
	}
	if ((status = call_voiceout->SetSampleInterval(ival)) < 0)
	{
		if (call_voicein->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_voiceout->SetSampleInterval(oldsampleinterval) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_voiceout == udef_voiceout,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEOUTPUTSAMPINT);
	}
	if ((status = call_comp->SetSampleInterval(ival)) < 0)
	{
		if (call_voicein->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_voiceout->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_comp->SetSampleInterval(oldsampleinterval) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_RETURN_VAL(status);
	}
	if ((status = call_mixer->SetSampleInterval(ival)) < 0)
	{
		if (call_voicein->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_voiceout->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_comp->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_mixer->SetSampleInterval(oldsampleinterval) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_mixer == udef_mixer,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFMIXERSAMPINT);
	}
	if ((status = call_trans->SetSampleInterval(ival)) < 0)
	{
		if (call_voicein->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_voiceout->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_comp->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_mixer->SetSampleInterval(oldsampleinterval) < 0 ||
		    call_trans->SetSampleInterval(oldsampleinterval) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_trans == udef_trans,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFTRANSSAMPINT);
	}
	sessparams.SetSampleInterval(ival);

	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	ResetActions(); // to make sure that everything stays synchronized
	
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetInputSamplingRate(JVOIPuint16 irate)
{
	if (irate < 1)
		return ERR_JVOIPLIB_SESSION_ILLEGALINPUTSAMPLINGRATE;
	
	CHECKCREATE_ERROR;
	if (irate == sessparams.GetInputSamplingRate())
		UNLOCK_RETURN_VAL(0);
	
	// first we'll check if the components support the sampling rate
	if (!call_voicein->SupportsInputSamplingRate(irate))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_VOICEINPUTDOESTSUPPORTINPUTSAMPRATE);
	if (!call_comp->SupportsInputSamplingRate(irate))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_COMPRESSIONDOESNTSUPPORTINPUTSAMPRATE);
	if (!call_trans->SupportsInputSamplingRate(irate))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_TRANSMISSIONDOESNTSUPPORTINPUTSAMPRATE);

	// now, we'll try to set the sampling rate, if it fails, we'll try to reset
	// the old sampling rate
	
	int oldinputsamprate = sessparams.GetInputSamplingRate();
	int status;

	if ((status = call_voicein->SetInputSamplingRate(irate)) < 0)
	{
		if (call_voicein->SetInputSamplingRate(oldinputsamprate) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_voicein == udef_voicein,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEININPUTSAMPRATE);
	}
	if ((status = call_comp->SetInputSamplingRate(irate)) < 0)
	{
		if (call_voicein->SetInputSamplingRate(oldinputsamprate) < 0 ||
		    call_comp->SetInputSamplingRate(oldinputsamprate) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_RETURN_VAL(status);
	}
	if ((status = call_trans->SetInputSamplingRate(irate)) < 0)
	{
		if (call_voicein->SetInputSamplingRate(oldinputsamprate) < 0 ||
		    call_comp->SetInputSamplingRate(oldinputsamprate) < 0 ||
		    call_trans->SetInputSamplingRate(oldinputsamprate) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_trans == udef_trans,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFTRANSMISSIONINPUTSAMPRATE);
	}
	sessparams.SetInputSamplingRate(irate);

	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}

	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetOutputSamplingRate(JVOIPuint16 orate)
{
	if (orate < 1)
		return ERR_JVOIPLIB_SESSION_ILLEGALOUTPUTSAMPLINGRATE;
	
	CHECKCREATE_ERROR;
	if (orate == sessparams.GetOutputSamplingRate())
		UNLOCK_RETURN_VAL(0);
	
	// first we'll check if the components support the sampling rate
	if (!call_voiceout->SupportsOutputSamplingRate(orate))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_VOICEOUTPUTDOESNTSUPPORTOUTPUTSAMPRATE);
	if (call_loc && !call_loc->SupportsOutputSamplingRate(orate))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_LOCALISATIONDOESNTSUPPORTOUTPUTSAMPRATE);
	if (!call_mixer->SupportsOutputSamplingRate(orate))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_MIXERDOESNTSUPPORTOUTPUTSAMPRATE);

	// now, we'll try to set the sampling rate, if it fails, we'll try to reset
	// the old sampling rate
	
	int oldoutputsamprate = sessparams.GetOutputSamplingRate();
	int status;

	if ((status = call_voiceout->SetOutputSamplingRate(orate)) < 0)
	{
		if (call_voiceout->SetOutputSamplingRate(oldoutputsamprate) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_voiceout == udef_voiceout,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEOUTOUTPUTSAMPRATE);
	}
	if (call_loc && (status = call_loc->SetOutputSamplingRate(orate)) < 0)
	{
		if (call_voiceout->SetOutputSamplingRate(oldoutputsamprate) < 0 ||
		    call_loc->SetOutputSamplingRate(oldoutputsamprate) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_loc == udef_loc,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFLOCALISATIONOUTPUTSAMPRATE);
	}
	if ((status = call_mixer->SetOutputSamplingRate(orate)) < 0)
	{
		if (call_voiceout->SetOutputSamplingRate(oldoutputsamprate) < 0 ||
		    (call_loc && call_loc->SetOutputSamplingRate(oldoutputsamprate) < 0) ||
		    call_mixer->SetOutputSamplingRate(oldoutputsamprate) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_mixer == udef_mixer,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFMIXEROUTPUTSAMPRATE);
	}
	sessparams.SetOutputSamplingRate(orate);

	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	
	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetInputSampleEncodingType(JVOIPSessionParams::SampleEncodingType ie)
{
	CHECKCREATE_ERROR;
	if (ie == sessparams.GetInputSampleEncodingType())
		UNLOCK_RETURN_VAL(0);
	
	int numencbytes;
	
	if (ie == JVOIPSessionParams::EightBit)
		numencbytes = 1;
	else
		numencbytes = 2;
	
	// first we'll check if the components support the encoding type
	if (!call_voicein->SupportsInputBytesPerSample(numencbytes))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_VOICEINPUTDOESNTSUPPORTINPUTENCODING);
	if (!call_comp->SupportsInputBytesPerSample(numencbytes))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_COMPRESSIONDOESNTSUPPORTINPUTENCODING);
	if (!call_trans->SupportsInputBytesPerSample(numencbytes))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_TRANSMISSIONDOESNTSUPPORTINPUTENCODING);

	// now, we'll try to set the encoding type, if it fails, we'll try to reset
	// the old encoding type
	
	int oldnumencbytes = ((sessparams.GetInputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2);
	int status;

	if ((status = call_voicein->SetInputBytesPerSample(numencbytes)) < 0)
	{
		if (call_voicein->SetInputBytesPerSample(oldnumencbytes) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_voicein == udef_voicein,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEININPUTENCODING);
	}
	if ((status = call_comp->SetInputBytesPerSample(numencbytes)) < 0)
	{
		if (call_voicein->SetInputBytesPerSample(oldnumencbytes) < 0 ||
		    call_comp->SetInputBytesPerSample(oldnumencbytes) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_RETURN_VAL(status);
	}
	if ((status = call_trans->SetInputBytesPerSample(numencbytes)) < 0)
	{
		if (call_voicein->SetInputBytesPerSample(oldnumencbytes) < 0 ||
		    call_comp->SetInputBytesPerSample(oldnumencbytes) < 0 ||
		    call_trans->SetInputBytesPerSample(oldnumencbytes) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_trans == udef_trans,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFTRANSMISSIONINPUTENCODING);
	}

	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	
	sessparams.SetInputSampleEncodingType(ie);		
	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetOutputSampleEncodingType(JVOIPSessionParams::SampleEncodingType oe)
{
	CHECKCREATE_ERROR;
	if (oe == sessparams.GetOutputSampleEncodingType())
		UNLOCK_RETURN_VAL(0);
	
	int numencbytes;
	
	if (oe == JVOIPSessionParams::EightBit)
		numencbytes = 1;
	else
		numencbytes = 2;
	
	// first we'll check if the components support the encoding type
	if (!call_voiceout->SupportsOutputBytesPerSample(numencbytes))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_VOICEOUTPUTDOESNTSUPPORTOUTPUTENCODING);
	if (call_loc && !call_loc->SupportsOutputBytesPerSample(numencbytes))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_LOCALISATIONDOESNTSUPPORTOUTPUTENCODING);
	if (!call_mixer->SupportsOutputBytesPerSample(numencbytes))
		UNLOCK_RETURN_VAL(ERR_JVOIPLIB_SESSION_MIXERDOESNTSUPPORTOUTPUTENCODING);

	// now, we'll try to set the encoding type, if it fails, we'll try to reset
	// the old encoding type
	
	int oldnumencbytes = ((sessparams.GetOutputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2);
	int status;

	if ((status = call_voiceout->SetOutputBytesPerSample(numencbytes)) < 0)
	{
		if (call_voiceout->SetOutputBytesPerSample(oldnumencbytes) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_voiceout == udef_voiceout,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFVOICEOUTOUTPUTENCODING);
	}
	if (call_loc && (status = call_loc->SetOutputBytesPerSample(numencbytes)) < 0)
	{
		if (call_voiceout->SetOutputBytesPerSample(oldnumencbytes) < 0 ||
		    call_loc->SetOutputBytesPerSample(oldnumencbytes) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_loc == udef_loc,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFLOCALISATIONOUTPUTENCODING);
	}
	if ((status = call_mixer->SetOutputBytesPerSample(numencbytes)) < 0)
	{
		if (call_voiceout->SetOutputBytesPerSample(oldnumencbytes) < 0 ||
		    (call_loc && (call_loc->SetOutputBytesPerSample(oldnumencbytes) < 0)) ||
		    call_mixer->SetOutputBytesPerSample(oldnumencbytes) < 0)
			RESTORE_FAILED_ACTION;
		UNLOCK_COMPONENTERROR(status,call_mixer == udef_mixer,ERR_JVOIPLIB_SESSION_CANTSETUSERDEFMIXEROUTPUTENCODING);
	}
	sessparams.SetOutputSampleEncodingType(oe);

	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	ResetActions(); // to make sure that everything stays synchronized
	
	UNLOCK_RETURN_VAL(0);
}

#define SAVECOMPONENTSTATE(comp,compstate,isuserdef,udeferr) \
	{\
		int status;\
		\
		if ((status = comp->GetComponentState(&compstate)) < 0)\
		{\
			UNLOCK_COMPONENTERROR(status,isuserdef,udeferr);\
		}\
	}

	
#define DELETECOMPONENTSTATE	\
	{\
		if (compstate)\
		{\
			delete compstate;\
			compstate = NULL;\
		}\
	}

int JVOIPSession::SetVoiceInputType(JVOIPSessionParams::VoiceInputType i,JVOIPComponentParams *params)
{
	JVOIPVoiceInput *newvoicein;
	JVOIPComponentState *compstate = NULL;
	int sampint,rate,bytespersamp;
	int status;

	CHECKCREATE_ERROR;
	status = ProcessInputType(i,&newvoicein);
	if (status < 0)
		UNLOCK_RETURN_VAL(status);
	
	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	
	// Ok, now we have FULL access to the JVOIPVoiceInput related stuff
	
	// first, we're gonna save the state of the component
	SAVECOMPONENTSTATE(call_voicein,compstate,call_voicein == udef_voicein,ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFINPUTSTATE)

	// clean up the current component	
	call_voicein->Cleanup();
	
	sampint = sessparams.GetSampleInterval();
	rate = sessparams.GetInputSamplingRate();
	bytespersamp = (sessparams.GetInputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2;

	status = newvoicein->Init(sampint,rate,bytespersamp,params);
	if (status < 0) // try to reinstall the previous component
	{
		JVOIPComponentParams *oldparams;

		delete newvoicein;
				
		oldparams = sessparams.GetVoiceInputParams();
		if (call_voicein->Init(sampint,rate,bytespersamp,oldparams) < 0)
			RESTORE_FAILED_ACTION;
		if (call_voicein->SetComponentState(compstate) < 0)
		{
			DELETECOMPONENTSTATE;
			RESTORE_FAILED_ACTION;
		}
		DELETECOMPONENTSTATE;
		UNLOCK_COMPONENTERROR(status,newvoicein == udef_voicein,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFVOICEINPUT);
	}

	DELETECOMPONENTSTATE;
	if (call_voicein != udef_voicein)
		delete call_voicein;
	
	// set the new stuff
	call_voicein = newvoicein;	
	call_timer = call_voicein->GetSamplingTimer();
	sigwait = call_timer->GetSignalWaiter();
	voicecall.SetIOComponents(call_voicein,call_voiceout);
	voicecall.SetSamplingTimer(call_timer);
	
	// adjust the component parameters in the session parameters
	sessparams.SetVoiceInputType(i);
	if ((status = sessparams.SetVoiceInputParams(params)) < 0) // WOW! probably out of memory
		DESTROY_SESSION(status);
	
	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetVoiceOutputType(JVOIPSessionParams::VoiceOutputType o,JVOIPComponentParams *params)
{
	JVOIPVoiceOutput *newvoiceout;
	JVOIPComponentState *compstate = NULL;
	int sampint,rate,bytespersamp;
	int status;
	bool needstereo;

	CHECKCREATE_ERROR;
	status = ProcessOutputType(o,&newvoiceout);
	if (status < 0)
		UNLOCK_RETURN_VAL(status);
	
	// first, we're gonna save the state of the component
	SAVECOMPONENTSTATE(call_voiceout,compstate,call_voiceout == udef_voiceout,ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFOUTPUTSTATE)

	// clean up the current component	
	call_voiceout->Cleanup();
	
	sampint = sessparams.GetSampleInterval();
	rate = sessparams.GetOutputSamplingRate();
	bytespersamp = (sessparams.GetOutputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2;
	needstereo = (call_loc == NULL)?false:true;
	
	status = newvoiceout->Init(sampint,rate,bytespersamp,needstereo,params);
	if (status < 0) // try to reinstall the previous component
	{
		JVOIPComponentParams *oldparams;
		
		delete newvoiceout;
		
		oldparams = sessparams.GetVoiceOutputParams();
		if (call_voiceout->Init(sampint,rate,bytespersamp,needstereo,oldparams) < 0)
			RESTORE_FAILED_ACTION;
		if (call_voiceout->SetComponentState(compstate) < 0)
		{
			DELETECOMPONENTSTATE;
			RESTORE_FAILED_ACTION;
		}
		DELETECOMPONENTSTATE;
		UNLOCK_COMPONENTERROR(status,newvoiceout == udef_voiceout,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFVOICEOUTPUT);
	}

	DELETECOMPONENTSTATE;
	if (call_voiceout != udef_voiceout)
		delete call_voiceout;
	
	// set the new stuff
	call_voiceout = newvoiceout;	
	voicecall.SetIOComponents(call_voicein,call_voiceout);
	
	// adjust the component parameters in the session parameters
	sessparams.SetVoiceOutputType(o);
	if ((status = sessparams.SetVoiceOutputParams(params)) < 0) // WOW! probably out of memory
		DESTROY_SESSION(status);
	
	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetLocalisationType(JVOIPSessionParams::LocalisationType l,JVOIPComponentParams *params)
{
	JVOIPLocalisation *newloc;
	JVOIPComponentState *compstate = NULL;
	int status;
	bool prevstereo,newstereo;
	
	CHECKCREATE_ERROR;
	status = ProcessLocalisationType(l,&newloc);
	if (status < 0)
		UNLOCK_RETURN_VAL(status);

	// check if we have to change the stereo settings
 	prevstereo = (call_loc == NULL)?false:true;
	newstereo = (newloc == NULL)?false:true;
	if (newstereo != prevstereo)
	{
		if ((status = call_voiceout->SetStereo(newstereo)) < 0)
		{
			if (newloc)
				delete newloc;
		
			// try to reset the old stereo
			if (call_voiceout->SetStereo(prevstereo) < 0)
				RESTORE_FAILED_ACTION;
			UNLOCK_RETURN_VAL(status);
		}
		if ((status = call_mixer->SetStereo(newstereo)) < 0)
		{
			if (newloc)
				delete newloc;
			
			// try to reset the old stereo, just to make sure
			if (call_mixer->SetStereo(prevstereo) < 0)
				RESTORE_FAILED_ACTION;
			if (call_voiceout->SetStereo(prevstereo) < 0)
				RESTORE_FAILED_ACTION;
			UNLOCK_RETURN_VAL(status);
		}
	}
	
	if (newloc == NULL) // no localisation
	{
		if (call_loc)
		{
			delete call_loc;
			call_loc = NULL;
		}
	}
	else
	{
		int bytespersamp,rate;
		
		rate = sessparams.GetOutputSamplingRate();
		bytespersamp = (sessparams.GetOutputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2;
		if (call_loc)
		{
			// first, we're gonna save the state of the component
			SAVECOMPONENTSTATE(call_loc,compstate,call_loc == udef_loc,ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFLOCALISATIONSTATE)

			// clean up the current component	
			call_loc->Cleanup();
		}
		if ((status = newloc->Init(rate,bytespersamp,params)) < 0) // try to restore
		{
			if (newloc)
				delete newloc;
			
			if (call_loc)
			{
				JVOIPComponentParams *oldparams;
		
				oldparams = sessparams.GetLocalisationParams();
				if (call_loc->Init(rate,bytespersamp,oldparams) < 0)
				{
					DELETECOMPONENTSTATE;
					RESTORE_FAILED_ACTION;
				}
				if (call_loc->SetComponentState(compstate) < 0)
					RESTORE_FAILED_ACTION;
			}
			DELETECOMPONENTSTATE;
			UNLOCK_COMPONENTERROR(status,newloc == udef_loc,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFLOCALISATION);
		}
		DELETECOMPONENTSTATE;
		
		if (call_loc && call_loc != udef_loc)
			delete call_loc;
			
		call_loc = newloc;
	}
	
	voicecall.Set3DComponents(call_loc,call_loc);
	sessparams.SetLocalisationType(l);
	if (call_loc == NULL)
		sessparams.SetLocalisationParams(NULL);
	else
	{
		if ((status = sessparams.SetLocalisationParams(params)) < 0) // WOW! probably out of memory
			DESTROY_SESSION(status);
	}
							
	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetCompressionType(JVOIPSessionParams::CompressionType c,JVOIPComponentParams *params)
{
	int status;
	bool fatalerror;
	
	CHECKCREATE_ERROR;
	
	fatalerror = false;
	status = call_comp->SetCompressionType(c,params,&fatalerror);
	if (status < 0)
	{
		if (fatalerror)
			DESTROY_SESSION(status);
		UNLOCK_RETURN_VAL(status);
	}
	
	sessparams.SetCompressionType(c);
	if ((status = sessparams.SetCompressionParams(params)) < 0)
		DESTROY_SESSION(status);
	
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetMixerType(JVOIPSessionParams::MixerType m,JVOIPComponentParams *params)
{
	JVOIPMixer *newmixer;
	JVOIPComponentState *compstate = NULL;
	int sampint,rate,bytespersamp;
	int status;
	bool needstereo;

	CHECKCREATE_ERROR;
	status = ProcessMixerType(m,&newmixer);
	if (status < 0)
		UNLOCK_RETURN_VAL(status);
	
	// first, we're gonna save the state of the component
	SAVECOMPONENTSTATE(call_mixer,compstate,call_mixer == udef_mixer,ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFMIXERSTATE)

	// clean up the current component	
	call_mixer->Cleanup();
	
	sampint = sessparams.GetSampleInterval();
	rate = sessparams.GetOutputSamplingRate();
	bytespersamp = (sessparams.GetOutputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2;
	needstereo = (call_loc == NULL)?false:true;
	
	status = newmixer->Init(sampint,rate,bytespersamp,needstereo,params);
	if (status < 0) // try to reinstall the previous component
	{
		JVOIPComponentParams *oldparams;
		
		delete newmixer;
		
		oldparams = sessparams.GetMixerParams();
		if (call_mixer->Init(sampint,rate,bytespersamp,needstereo,oldparams) < 0)
			RESTORE_FAILED_ACTION;
		if (call_mixer->SetComponentState(compstate) < 0)
		{
			DELETECOMPONENTSTATE;
			RESTORE_FAILED_ACTION;
		}
		DELETECOMPONENTSTATE;
		UNLOCK_COMPONENTERROR(status,newmixer == udef_mixer,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFMIXER);
	}

	DELETECOMPONENTSTATE;
	
	if (call_mixer != udef_mixer)
		delete call_mixer;
	
	// set the new stuff
	call_mixer = newmixer;	
	voicecall.SetMixer(call_mixer);
	
	// adjust the component parameters in the session parameters
	sessparams.SetMixerType(m);
	if ((status = sessparams.SetMixerParams(params)) < 0) // WOW! probably out of memory
		DESTROY_SESSION(status);
	
	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

int JVOIPSession::SetTransmissionType(JVOIPSessionParams::TransmissionType t,JVOIPComponentParams *params)
{
	JVOIPTransmission *newtrans;
	JVOIPComponentState *compstate = NULL;
	int ival,rate,bytespersamp;
	int status;

	CHECKCREATE_ERROR;
	status = ProcessTransmissionType(t,&newtrans);
	if (status < 0)
		UNLOCK_RETURN_VAL(status);
	
	// first, we're gonna save the state of the component
	SAVECOMPONENTSTATE(call_trans,compstate,call_trans == udef_trans,ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFTRANSMISSIONSTATE)

	// clean up the current component	
	call_trans->Cleanup();
	
	ival = sessparams.GetSampleInterval();
	rate = sessparams.GetInputSamplingRate();
	bytespersamp = (sessparams.GetInputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2;
	
	status = newtrans->Init(ival,rate,bytespersamp,params);
	if (status < 0) // try to reinstall the previous component
	{
		JVOIPComponentParams *oldparams;
	
		delete newtrans;
			
		oldparams = sessparams.GetTransmissionParams();
		if (call_trans->Init(ival,rate,bytespersamp,oldparams) < 0)
			RESTORE_FAILED_ACTION;
		if (call_trans->SetComponentState(compstate) < 0)
		{
			DELETECOMPONENTSTATE;
			RESTORE_FAILED_ACTION;
		}
		DELETECOMPONENTSTATE;
		UNLOCK_COMPONENTERROR(status,newtrans == udef_trans,ERR_JVOIPLIB_SESSION_CANTINITUSERDEFTRANSMISSION);
	}

	DELETECOMPONENTSTATE;
	
	if (call_trans != udef_trans)
		delete call_trans;
	
	// set the new stuff
	call_trans = newtrans;	
	voicecall.SetTransmitter(call_trans);
	
	// adjust the component parameters in the session parameters
	sessparams.SetTransmissionType(t);
	if ((status = sessparams.SetTransmissionParams(params)) < 0) // WOW! probably out of memory
		DESTROY_SESSION(status);
	
	// Make sure the thread doesn't get stuck waiting for a signal which will
	// never come...

	if (voipthread.IsWaitingForSignal())
	{
		sigwait->Signal(); // to make sure the component switch can happen as fast as possible
		while (voipthread.IsWaitingForSignal())
			; // wait till the sigwait is no longer needed
	}
	ResetActions(); // to make sure that everything stays synchronized
	UNLOCK_RETURN_VAL(0);
}

#undef CHECKCREATE_ERROR
#undef CHECKCREATE_NOERROR
#undef UNLOCK_RETURN_VAL

inline void JVOIPSession::ResetActions()
{
	firstpass = true;
	call_voicein->Reset();
	call_voiceout->Reset();
	call_trans->Reset();
}
