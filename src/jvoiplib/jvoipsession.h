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

#ifndef JVOIPSESSION_H

#define JVOIPSESSION_H

#include "jvoiptypes.h"
#include "jvoipexception.h"
#include "jvoipsessionparams.h"
#include "jvoipthread.h"
#include "jvoipsamplingtimer.h"
#include "jvoipcomponent.h"
#include "voicecall.h"
#include "jmutex.h"
#include <string>
#include <vector>

#ifdef WIN32
	#include <windows.h>
#else // unix-like systems
	#include <sys/time.h>
	#include <unistd.h>
#endif // WIN32

#define JVOIP_ALLPORTS								0
#define JVOIP_3DINFOBUFFERLEN							4096

class JVOIPVoiceInput;
class JVOIPVoiceOutput;
class JVOIPLocalisation;
class JVOIPCompression;
class JVOIPCompressionModule;
class JVOIPTransmission;
class JVOIPSignalWait;
class JVOIPMixer;
class JVOIPSamplingTimer;

class JVOIPSession
{
public:
	enum ComponentType { VoiceInput,VoiceOutput,Localisation,Compression,Transmission,Mixer };

	JVOIPSession() throw (JVOIPException);
	virtual ~JVOIPSession() throw (JVOIPException);
	int Create(const JVOIPSessionParams &cp);
	bool IsActive();
	int Destroy();
	
	// instruct the transmitter about destinations and allowed incoming packets
	int AddDestination(JVOIPuint32 destip,JVOIPuint16 destportbase);
	int DeleteDestination(JVOIPuint32 destip,JVOIPuint16 destportbase);
	void ClearDestinations();
	
	int JoinMulticastGroup(JVOIPuint32 mcastip);
	int LeaveMulticastGroup(JVOIPuint32 mcastip);
	void LeaveAllMulticastGroups();
	
	int SetReceiveType(JVOIPSessionParams::ReceiveType rt);
	int AddToAcceptList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	int DeleteFromAcceptList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	void ClearAcceptList();
	int AddToIgnoreList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	int DeleteFromIgnoreList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	void ClearIgnoreList();

	// Retrieve current settings
	int GetSampleInterval(JVOIPuint16 *iv);
	int GetInputSamplingRate(JVOIPuint16 *ir);
	int GetOutputSamplingRate(JVOIPuint16 *outr);
	int GetInputSampleEncodingType(JVOIPSessionParams::SampleEncodingType *ie);
	int GetOutputSampleEncodingType(JVOIPSessionParams::SampleEncodingType *oe);
	int GetVoiceInputType(JVOIPSessionParams::VoiceInputType *i);
	int GetVoiceOutputType(JVOIPSessionParams::VoiceOutputType *o);
	int GetLocalisationType(JVOIPSessionParams::LocalisationType *l);
	int GetCompressionType(JVOIPSessionParams::CompressionType *c);
	int GetMixerType(JVOIPSessionParams::MixerType *m);
	int GetTransmissionType(JVOIPSessionParams::TransmissionType *t);
	int GetReceiveType(JVOIPSessionParams::ReceiveType *r);

	// info about components
	std::string GetComponentName(ComponentType comptype);
	std::string GetComponentDescription(ComponentType comptype);
	int GetComponentParameters(ComponentType comptype,std::vector<JVOIPCompParamInfo> **params);
	
	// change certain settings while in a session
	int SetSampleInterval(JVOIPuint16 ival);
	int SetInputSamplingRate(JVOIPuint16 irate);
	int SetOutputSamplingRate(JVOIPuint16 orate);
	int SetInputSampleEncodingType(JVOIPSessionParams::SampleEncodingType ie);
	int SetOutputSampleEncodingType(JVOIPSessionParams::SampleEncodingType oe);
	int SetVoiceInputType(JVOIPSessionParams::VoiceInputType i,JVOIPComponentParams *params);
	int SetVoiceOutputType(JVOIPSessionParams::VoiceOutputType o,JVOIPComponentParams *params);
	int SetLocalisationType(JVOIPSessionParams::LocalisationType l,JVOIPComponentParams *params);
	int SetCompressionType(JVOIPSessionParams::CompressionType c,JVOIPComponentParams *params);
	int SetMixerType(JVOIPSessionParams::MixerType m,JVOIPComponentParams *params);
	int SetTransmissionType(JVOIPSessionParams::TransmissionType t,JVOIPComponentParams *params);
protected:	
	// these functions can perform user defined initialisation and destruction
	virtual bool UserDefinedCreate() { return true; }
	virtual void UserDefinedDestroy() { }
	
	// Functions called for user defined components
	virtual bool RegisterUserDefinedInput(JVOIPVoiceInput **i)				{ *i = NULL; return true; }
	virtual bool RegisterUserDefinedOutput(JVOIPVoiceOutput **o)				{ *o = NULL; return true; }
	virtual bool RegisterUserDefinedLocalisation(JVOIPLocalisation **l)			{ *l = NULL; return true; }
	virtual bool RegisterUserDefinedCompressionModule(JVOIPCompressionModule **c)		{ *c = NULL; return true; }
	virtual bool RegisterUserDefinedTransmission(JVOIPTransmission **t)			{ *t = NULL; return true; }
	virtual bool RegisterUserDefinedMixer(JVOIPMixer **m)					{ *m = NULL; return true; }
	virtual void UnregisterUserDefinedInput(JVOIPVoiceInput *i)				{ }
	virtual void UnregisterUserDefinedOutput(JVOIPVoiceOutput *o)				{ }
	virtual void UnregisterUserDefinedLocalisation(JVOIPLocalisation *l)			{ }
	virtual void UnregisterUserDefinedCompressionModule(JVOIPCompressionModule *c)		{ }
	virtual void UnregisterUserDefinedTransmission(JVOIPTransmission *t)			{ }
	virtual void UnregisterUserDefinedMixer(JVOIPMixer *m)					{ }
	
	// make it possible to change the action after a sampling interval
	virtual bool IntervalAction(bool *waitforsignal);
	
	// 3D localisation stuff (by default, only for the standard localisation routines, not for user defined)
	virtual bool RetrieveOwnPosition(double *xpos,double *ypos,double *zpos,
	                                 double *righteardir_x,double *righteardir_y,double *righteardir_z,
					 double *frontdir_x,double *frontdir_y,double *frontdir_z,
					 double *updir_x,double *updir_y,double *updir_z) 				{ return false; }
	virtual bool EncodeOwnPosition(unsigned char encodebuffer[JVOIP_3DINFOBUFFERLEN], int *len)			{ return false; }
	virtual bool DecodePositionalInfo(unsigned char buffer[], int len, double *xpos,double *ypos,double *zpos)	{ return false; }
	
	// Error handling routine
	virtual void ThreadFinishedHandler(int threaderr,int voicecallerr = 0,int componenterr = 0)			{ }
private:
	// functions to let thread access some members in a clean way
	VoIPFramework::VoiceCall &GetVoiceCall()					{ return voicecall; }
	JMutex &GetComponentMutex()							{ return componentmutex; }
	JVOIPSignalWait *GetSignalWaiter()						{ return sigwait; }
	const JVOIPSessionParams &SessionParameters() const				{ return sessparams; }
	
	void ClearComponents();
	void DeleteComponents();
	int RegisterUserDefinedComponents();
	void UnregisterUserDefinedComponents();
	void ThreadFinished(int threaderr,int voicecallerr = 0,int componenterr = 0);
	void GeneralDestroyActions();
	void GetTheCurrentTime(struct timeval *tv);
	inline void ResetActions();
	
	int ProcessInputType(JVOIPSessionParams::VoiceInputType it,JVOIPVoiceInput **vi);
	int ProcessOutputType(JVOIPSessionParams::VoiceOutputType ot,JVOIPVoiceOutput **vo);
	int ProcessLocalisationType(JVOIPSessionParams::LocalisationType lt,JVOIPLocalisation **loc);
	int ProcessCompressionModuleType(JVOIPSessionParams::CompressionType ct,JVOIPCompressionModule **compmod);
	int ProcessMixerType(JVOIPSessionParams::MixerType mt,JVOIPMixer **mix);
	int ProcessTransmissionType(JVOIPSessionParams::TransmissionType tt,JVOIPTransmission **trans);

	JVOIPSessionParams sessparams;
	JVOIPThread voipthread;
	JVOIPSignalWait *sigwait;
	JMutex componentmutex;
	bool created,destroying;
	VoIPFramework::VoiceCall voicecall;
	
	// User defined components
	JVOIPVoiceInput *udef_voicein;
	JVOIPVoiceOutput *udef_voiceout;
	JVOIPLocalisation *udef_loc;
	JVOIPCompressionModule *udef_compmod;
	JVOIPTransmission *udef_trans;
	JVOIPMixer *udef_mixer;
	
	// some interval action related variables
	bool firstpass;
	struct timeval starttime;
	double loopcount;
	
	friend class JVOIPThread;
	friend class JVOIPCompression;
	friend class JVOIPSessionLocalisation;
protected:
	int LockComponents() 								{ return componentmutex.Lock(); }
	int UnlockComponents() 								{ return componentmutex.Unlock(); }
	
	// The components	
	JVOIPVoiceInput *call_voicein;
	JVOIPVoiceOutput *call_voiceout;
	JVOIPLocalisation *call_loc;
	JVOIPCompression *call_comp;
	JVOIPTransmission *call_trans;
	JVOIPMixer *call_mixer;
	JVOIPSamplingTimer *call_timer;
};

#endif // JVOIPSESSION_H
