/*

    This file is a part of JVOIPLIB, a library designed to facilitate
    the use of Voice over IP (VoIP).

    Copyright (C) 2000-2002  Jori Liesenborgs (jori@lumumba.luc.ac.be)

    This library (JVOIPLIB) is based upon work done for my thesis at
    the School for Knowledge Technology (Belgium/The Netherlands)

    This file was developed at the 'Expertise Centre for Digital
    Media' (EDM) in Diepenbeek, Belgium (http://www.edm.luc.ac.be).
    The EDM is a research institute of the 'Limburgs Universitair
    Centrum' (LUC) (http://www.luc.ac.be).

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

#ifndef JVOIPRTPTRANSMITTER_H

#define JVOIPRTPTRANSMITTER_H

#include "jvoiptransmission.h"
#include "jvoipexception.h"
#include "jvoipcomponentparams.h"
#include "jthread.h"
#include "jmutex.h"
#include <list>

#define JVOIPRTPTRANS_DEFAULTPORTBASE						5000
#define JVOIPRTPTRANS_DEFAULTBUFFERING						0.050 // 50 milliseconds
#define JVOIPRTPTRANS_MINBUFFERING						0.020 // 20 milliseconds
#define JVOIPRTPTRANS_MAXPACKSIZE						60000

class RTPSession;
class RTPPacket;
class JVOIPRTPTransmission;

class JVOIPRTPTransmissionParams : public JVOIPComponentParams
{
public:
	JVOIPRTPTransmissionParams(int pbase = JVOIPRTPTRANS_DEFAULTPORTBASE,bool autobuffer = true,
	                           double defbuffering = JVOIPRTPTRANS_DEFAULTBUFFERING,
				   double minbuffering = JVOIPRTPTRANS_MINBUFFERING,
	                           bool acceptownpack = false,unsigned long locIP = 0) 			{ portbase = pbase; autoadjustbuffering = autobuffer; localip = locIP; defaultbuffer = defbuffering; minimbuffer = minbuffering; acceptownpackets = acceptownpack; }
	~JVOIPRTPTransmissionParams() { }
	JVOIPComponentParams *CreateCopy() const;
	int GetPortBase() const 								{ return portbase; }
	void SetPortBase(int pb) 								{ portbase = pb; }
	unsigned long GetLocalIP() const 							{ return localip; }
	void SetLocalIP(unsigned long locIP) 							{ localip = locIP; }
	bool GetAutoAdjustBuffer() const 							{ return autoadjustbuffering; }
	void SetAutoAdjustBuffer(bool autobuffer) 						{ autoadjustbuffering = autobuffer; }
	double GetDefaultBuffering() const 							{ return defaultbuffer; }
	void SetDefaultBuffering(double defbuffer)						{ defaultbuffer = defbuffer; }
	double GetMinimumBuffering() const 							{ return minimbuffer; }
	void SetMinimumBuffering(double minbuffer)						{ minimbuffer = minbuffer; }
	bool GetAcceptOwnPackets() const							{ return acceptownpackets; }
	void SetAcceptOwnPackets(bool acceptownpack)						{ acceptownpackets = acceptownpack; }
private:
	int portbase;
	unsigned long localip;
	bool autoadjustbuffering;
	double defaultbuffer;
	double minimbuffer;
	bool acceptownpackets;
};

class JVOIPRTPTransmissionThread : public JThread
{
public:
	JVOIPRTPTransmissionThread(JVOIPRTPTransmission *rtptrans) throw (JVOIPException);
	~JVOIPRTPTransmissionThread();
	void *Thread();
	void StopThread();
private:
	bool stopthread;
	JMutex stopmutex;
	JVOIPRTPTransmission *rtptransmission;
};

class JVOIPRTPTransmission : public JVOIPTransmission
{
public:
	JVOIPRTPTransmission(JVOIPSession *sess) throw (JVOIPException);
	~JVOIPRTPTransmission();
	
	int Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams);
	int Cleanup();
	void Reset();
	
	int SendBlock(VoIPFramework::VoiceBlock *vb);
	int StartVoiceSourceIteration();
	void EndVoiceSourceIteration();
	bool GotoFirstVoiceSource();
	bool GotoNextVoiceSource();
	VoIPFramework::VOIPuint64 GetVoiceSourceID();
	int GetSampleBlock(VoIPFramework::VoiceBlock *vb);
	int Poll();
	bool SourceHasMoreData();
	int SetSampleOffset(VoIPFramework::VOIPdouble offset);
	
	bool AddDestination(JVOIPuint32 destip,JVOIPuint16 destportbase);
	bool DeleteDestination(JVOIPuint32 destip,JVOIPuint16 destportbase);
	void ClearDestinations();

	bool SupportsMulticasting();	
	bool JoinMulticastGroup(JVOIPuint32 mcastip);
	bool LeaveMulticastGroup(JVOIPuint32 mcastip);
	void LeaveAllMulticastGroups();

	bool SupportsReceiveType(JVOIPSessionParams::ReceiveType rt);
	bool SetReceiveType(JVOIPSessionParams::ReceiveType rt);
	bool AddToAcceptList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	bool DeleteFromAcceptList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	void ClearAcceptList();
	bool AddToIgnoreList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	bool DeleteFromIgnoreList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS);
	void ClearIgnoreList();

	bool SupportsSampleInterval(int ival);
	int SetSampleInterval(int ival);
	bool SupportsInputSamplingRate(int irate);
	int SetInputSamplingRate(int irate);
	bool SupportsInputBytesPerSample(int inputbytespersample);
	int SetInputBytesPerSample(int inputbytespersample);

	int GetComponentState(JVOIPComponentState **compstate);
	int SetComponentState(JVOIPComponentState *compstate);
	
	std::string GetComponentName();
	std::string GetComponentDescription();
	std::vector<JVOIPCompParamInfo> *GetComponentParameters() throw (JVOIPException);
private:
	RTPSession *GetRTPSession() 							{ return rtpsess; }
	int LockRTPSession()								{ return rtpsessmutex.Lock(); }
	int UnlockRTPSession()								{ return rtpsessmutex.Unlock(); }
	int LockRTPSync()								{ return rtpsyncmutex.Lock(); }
	int UnlockRTPSync()								{ return rtpsyncmutex.Unlock(); }
	int CreateNewRTPSession();
	bool SupportedSampleRate(int rate);
	int EncodeData(VoIPFramework::VoiceBlock *vb,unsigned char *payloadtype,bool *marker);
	void ClearVoiceBlock(VoIPFramework::VoiceBlock *vb);
	
	RTPSession *rtpsess;
	JMutex rtpsessmutex,rtpsyncmutex;
	
	bool init;
	JVOIPRTPTransmissionThread transthread;
	int bytespersample,samplingrate;
	double interval;
	VoIPFramework::VOIPuint64 curid;
	VoIPFramework::VOIPdouble cursampleoffset;
	
	// info about participants
	
	class PacknumOffsetPair
	{
	public:
		PacknumOffsetPair(VoIPFramework::VOIPuint64 num,VoIPFramework::VOIPdouble off) { packnum = num; offset = off; }
		
		VoIPFramework::VOIPuint64 packnum;
		VoIPFramework::VOIPdouble offset;
	};
	
	class ParticipantInfo
	{
	public:
		ParticipantInfo(int id,unsigned long ssrc) { participantid = id; syncsrc = ssrc; ClearAllPublicFields(); }
		ParticipantInfo(const ParticipantInfo &src)
		{
			std::list<PacknumOffsetPair>::const_iterator it;

			participantid = src.participantid;
			syncsrc = src.syncsrc;

			isnewsource = src.isnewsource;
			for (it = src.receivedpacknums.begin() ; it != src.receivedpacknums.end() ; ++it)
				receivedpacknums.push_back(*it);
			samplingrate = src.samplingrate;
			lastknownjitter = src.lastknownjitter;
			buffering = src.buffering;
			basepacknum = src.basepacknum;
			prevpacknum = src.prevpacknum;
			packnumcycles = src.packnumcycles;
			packnumoutrangecount = src.packnumoutrangecount;
			basetimestamp = src.basetimestamp;
			prevtimestamp = src.prevtimestamp;
			timestampcycles = src.timestampcycles;
			timestampoutrangecount = src.timestampoutrangecount;
			basetimeoffset = src.basetimeoffset;
			prevjittertime = src.prevjittertime;
		}
		
		int ID() { return participantid; }
		unsigned long SSRC() { return syncsrc; }
		
		void ClearAllPublicFields() { isnewsource = true; receivedpacknums.clear(); samplingrate = 0; lastknownjitter = 0;
					      buffering = 0; basepacknum = 0; prevpacknum = 0; packnumcycles = 0; packnumoutrangecount = 0;
				   	      basetimestamp = 0; prevtimestamp = 0; timestampcycles = 0; timestampoutrangecount = 0;
				   	      basetimeoffset = 0; prevjittertime = 0; }
		
		bool isnewsource;
		std::list<PacknumOffsetPair> receivedpacknums;
		
		int samplingrate;
		unsigned long lastknownjitter;
		time_t prevjittertime;
		VoIPFramework::VOIPdouble buffering;
		
		// packet number info
		unsigned long basepacknum;
		unsigned long prevpacknum;
		unsigned long packnumcycles;
		int packnumoutrangecount;
		
		// timestamp info
		unsigned long basetimestamp;
		unsigned long prevtimestamp;
		unsigned long timestampcycles;
		int timestampoutrangecount;
		VoIPFramework::VOIPdouble basetimeoffset;
	private:
		int participantid;
		unsigned long syncsrc;
	};
	
	std::list<ParticipantInfo*> participants;
	void ClearParticipantInfo();
	inline ParticipantInfo *GetParticipantInfo(unsigned long ssrc);
	bool AlreadyProcessedPacket(RTPPacket *pack,ParticipantInfo *partinf,VoIPFramework::VOIPuint64 *packetnum);
	bool FillInVoiceBlock(VoIPFramework::VoiceBlock *vb,RTPPacket *pack,ParticipantInfo *partinf);
	bool CalculateTimeOffset(VoIPFramework::VoiceBlock *vb,RTPPacket *pack,ParticipantInfo *partinf,VoIPFramework::VOIPdouble *offset);
	
	// backup info in case the rtp session needs to be changed

	class IPPortPair
	{
	public:
		IPPortPair(JVOIPuint32 i,JVOIPuint16 p) { ip = i; port = p; }
		
		JVOIPuint32 ip;
		JVOIPuint16 port;
	};
		
	std::list<JVOIPuint32> mcastIPs;
	std::list<IPPortPair> destinations, acceptlist, ignorelist;
	JVOIPSessionParams::ReceiveType receivetype;
	
	// saved info from the component parameters
	
	int portbase;
	unsigned long localip;
	bool autoadjustbuffering;
	double defaultbuffer;
	bool acceptownpackets;
	double minimbuffer;
	
	// statistical info
	
	double numbytessent;
	double numbytesreceived;

	unsigned char sendbuf[JVOIPRTPTRANS_MAXPACKSIZE];
	
	// rtp event handlers
		
	static void StaticRTPJoinHandler(int exceptiontype,void *exceptiondata,void *usrdata);
	static void StaticRTPLeaveHandler(int exceptiontype,void *exceptiondata,void *usrdata);
	void RTPJoinHandler(unsigned long ssrc);
	void RTPLeaveHandler(unsigned long ssrc);
	
	friend class JVOIPRTPTransmissionThread;
	friend class JVOIPRTPTransmissionState;
};

#endif // JVOIPRTPTRANSMITTER_H
