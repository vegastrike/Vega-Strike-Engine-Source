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

#ifndef VOICECALL_H

#define VOICECALL_H

#define ERR_VOICECALL_NOTINITIALIZED				-1
#define ERR_VOICECALL_MIXERNEEDED				-2
#define ERR_VOICECALL_TRANSMITTERNEEDED				-3
#define ERR_VOICECALL_SAMPLEOUTPUTNEEDED			-4
#define ERR_VOICECALL_SAMPLEINPUTNEEDED				-5
#define	ERR_VOICECALL_CANTPOLLDATA				-6
#define ERR_VOICECALL_CANTGETINPUTSAMPLES			-7
#define ERR_VOICECALL_CANTSTARTSAMPLING				-8
#define ERR_VOICECALL_CANTCOMPRESSBLOCK				-9
#define ERR_VOICECALL_CANTADD3DINFO				-10
#define ERR_VOICECALL_CANTTRANSMITBLOCK				-11
#define ERR_VOICECALL_CANTGETSAMPLEBLOCKFROMMIXER		-12
#define ERR_VOICECALL_CANTPLAYSAMPLEBLOCK			-13
#define ERR_VOICECALL_CANTDECOMPRESS				-14
#define ERR_VOICECALL_CANTCREATE3DEFFECT			-15
#define ERR_VOICECALL_CANTADDBLOCKTOMIXER			-16
#define ERR_VOICECALL_CANTGETTRANSMISSIONBLOCK			-17
#define ERR_VOICECALL_CANTGETSAMPLEOFFSETFROMMIXER		-18
#define ERR_VOICECALL_CANTSETSAMPLEOFFSET			-19
#define ERR_VOICECALL_SAMPLINGTIMERNEEDED			-20
#define ERR_VOICECALL_CANTRESTARTTIMER				-21
#define ERR_VOICECALL_CANTSTARTITERATION			-22
#define ERR_VOICECALL_OUTOFMEM					-23

// start namespace
namespace VoIPFramework
{

char *GetVoiceCallErrorString(int errcode);
	
class SampleInput;
class SampleOutput;
class VoiceCompressor;
class VoiceDecompressor;
class Location3D;
class Transform3D;
class VoiceMixer;
class VoiceTransmitter;
class SamplingTimer;

class VoiceCall
{
public:
	VoiceCall();
	virtual ~VoiceCall();
	void Cleanup();
		
	int SetIOComponents(SampleInput *si,SampleOutput *so);
	void SetCompressionComponents(VoiceCompressor *vc,VoiceDecompressor *vd);
	void Set3DComponents(Location3D *loc,Transform3D *trans);
	int SetMixer(VoiceMixer *vmix);
	int SetTransmitter(VoiceTransmitter *vt);
	int SetSamplingTimer(SamplingTimer *timer);

	int Step(int *componenterror,bool *intervalpassed);
private:
	SampleInput *sampin;
	SampleOutput *sampout;
	VoiceCompressor *vcompress;
	VoiceDecompressor *vdecompress;
	Location3D *loc3d;
	Transform3D *transf3d;
	VoiceMixer *vmixer;
	VoiceTransmitter *vtransmit;
	SamplingTimer *samptimer;
	bool initialized;
};

// end namespace
};

#endif // VOICECALL_H
