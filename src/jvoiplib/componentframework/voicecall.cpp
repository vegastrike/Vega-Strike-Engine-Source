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

#include "voicecall.h"
#include "sampleinput.h"
#include "sampleoutput.h"
#include "voicecompressor.h"
#include "voicedecompressor.h"
#include "location3d.h"
#include "transform3d.h"
#include "voicemixer.h"
#include "voicetransmitter.h"
#include "voiceblock.h"
#include "voicetypes.h"
#include "samplingtimer.h"
#include <stdlib.h>

#include "debugnew.h"

// start namespace
namespace VoIPFramework
{

struct VoiceCallErrorDescription
{
	int errcode; 
	char *description;
};

static VoiceCallErrorDescription voicecallerrors[]={
	{ ERR_VOICECALL_NOTINITIALIZED,"VoiceCall not initialized"},
	{ ERR_VOICECALL_MIXERNEEDED,"You must specify a mixer object"},
	{ ERR_VOICECALL_TRANSMITTERNEEDED,"You must specify a transmission module"},
	{ ERR_VOICECALL_SAMPLEOUTPUTNEEDED,"You must specify a sample output module"},
	{ ERR_VOICECALL_SAMPLEINPUTNEEDED,"You must specify a sample input module"},
	{ ERR_VOICECALL_CANTPOLLDATA,"Error while trying to poll for new data"},
	{ ERR_VOICECALL_CANTGETINPUTSAMPLES,"Couldn't obtain input samples"},
	{ ERR_VOICECALL_CANTSTARTSAMPLING,"Couldn't start sampling"},
	{ ERR_VOICECALL_CANTCOMPRESSBLOCK,"Can't compress block"},
	{ ERR_VOICECALL_CANTADD3DINFO,"Can't add 3D info"},
	{ ERR_VOICECALL_CANTTRANSMITBLOCK,"Can't transmit VoiceBlock"},
	{ ERR_VOICECALL_CANTGETSAMPLEBLOCKFROMMIXER,"Can't get sample block from mixer"},
	{ ERR_VOICECALL_CANTPLAYSAMPLEBLOCK,"Can't play back sample block"},
	{ ERR_VOICECALL_CANTDECOMPRESS,"Can't decompress block"},
	{ ERR_VOICECALL_CANTCREATE3DEFFECT,"Can't create 3D effect"},
	{ ERR_VOICECALL_CANTADDBLOCKTOMIXER,"Can't add VoiceBlock data to mixer"},
	{ ERR_VOICECALL_CANTGETTRANSMISSIONBLOCK,"Can't get block from transmission component"},
	{ ERR_VOICECALL_CANTGETSAMPLEOFFSETFROMMIXER,"Can't get the current sample offset from the mixer"},
	{ ERR_VOICECALL_CANTSETSAMPLEOFFSET,"Can't set sample offset in the transmission component"},
	{ ERR_VOICECALL_SAMPLINGTIMERNEEDED,"A sampling timer component is required"},
	{ ERR_VOICECALL_CANTRESTARTTIMER,"Couldn't restart timer"}, 
	{ ERR_VOICECALL_CANTSTARTITERATION,"Couldn't start iteration over voice sources"},
	{ ERR_VOICECALL_OUTOFMEM,"Out of memory"},
	{ 0,NULL} };

char *GetVoiceCallErrorString(int errcode)
{
	int i;
	
	if (errcode >= 0)
		return "No error";
	
	i = 0;
	while (voicecallerrors[i].errcode < 0)
	{
		if (voicecallerrors[i].errcode == errcode)
			return voicecallerrors[i].description;
		i++;
	}
	return "Unknown error code";
}


VoiceCall::VoiceCall()
{
	Cleanup();
}

VoiceCall::~VoiceCall()
{
}

void VoiceCall::Cleanup()
{
	sampin = NULL;
	sampout = NULL;
	vcompress = NULL;
	vdecompress = NULL;
	loc3d = NULL;
	transf3d = NULL;
	vmixer = NULL;
	vtransmit = NULL;
	samptimer = NULL;
	initialized = false;
}

int VoiceCall::SetIOComponents(SampleInput *si,SampleOutput *so)
{
	if (si == NULL)
		return ERR_VOICECALL_SAMPLEINPUTNEEDED;
	if (so == NULL)
		return ERR_VOICECALL_SAMPLEOUTPUTNEEDED;
	sampin = si;
	sampout = so;
	if (vmixer != NULL && vtransmit != NULL && samptimer != NULL)
		initialized = true;
	return 0;
}

void VoiceCall::SetCompressionComponents(VoiceCompressor *vc,VoiceDecompressor *vd)
{
	vcompress = vc;
	vdecompress = vd;
}

void VoiceCall::Set3DComponents(Location3D *loc,Transform3D *trans)
{
	loc3d = loc;
	transf3d = trans;
}

int VoiceCall::SetMixer(VoiceMixer *vmix)
{
	if (vmix == NULL)
		return ERR_VOICECALL_MIXERNEEDED;
	vmixer = vmix;
	if (sampin != NULL && sampout != NULL && vtransmit != NULL && samptimer != NULL)
		initialized = true;
	return 0;
}

int VoiceCall::SetTransmitter(VoiceTransmitter *vt)
{
	if (vt == NULL)
		return ERR_VOICECALL_TRANSMITTERNEEDED;
	vtransmit = vt;
	if (sampin != NULL && sampout != NULL && vmixer != NULL && samptimer != NULL)
		initialized = true;
	return 0;
}

int VoiceCall::SetSamplingTimer(SamplingTimer *timer)
{
	if (timer == NULL)
		return ERR_VOICECALL_SAMPLINGTIMERNEEDED;
	samptimer = timer;
	if (sampin != NULL && sampout != NULL && vmixer != NULL && vtransmit != NULL)
		initialized = true;
	return 0;
}

int VoiceCall::Step(int *componenterror,bool *intervalpassed)
{
	if (!initialized)
	{
		*componenterror = 0;
		return ERR_VOICECALL_NOTINITIALIZED;
	}
	
	if (samptimer->HasTimeOut())
	{
		VoiceBlock block,block2;
		VOIPdouble sampleoffset;
		int status;

		/* Get sampled block and restart sampling */

		if ((status = sampin->GetSampleBlock(&block)) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTGETINPUTSAMPLES;
		}
		if ((status = sampin->StartSampling()) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTSTARTSAMPLING;
		}

		/* Get a block from the mixer and play it */

		if ((status = vmixer->GetSampleBlock(&block2)) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTGETSAMPLEBLOCKFROMMIXER;
		}
		if ((status = sampout->Play(&block2)) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTPLAYSAMPLEBLOCK;
		}

		/* Restart timer */

		if ((status = samptimer->RestartTimer()) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTRESTARTTIMER;
		}

		/* Prepare sampled block for transmission and transmit it */
		
		if (loc3d != NULL)
		{
			if ((status = loc3d->Add3DInfo(&block)) < 0)
			{
				*componenterror = status;
				return ERR_VOICECALL_CANTADD3DINFO;
			}
		}
		if (vcompress != NULL)
		{
			if ((status = vcompress->Compress(&block)) < 0)
			{
				*componenterror = status;
				return ERR_VOICECALL_CANTCOMPRESSBLOCK;
			}
		}
		if ((status = vtransmit->SendBlock(&block)) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTTRANSMITBLOCK;
		}

		/* Adjust the current sample offset */

		if ((status = vmixer->GetSampleOffset(&sampleoffset)) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTGETSAMPLEOFFSETFROMMIXER;
		}
		if ((status = vtransmit->SetSampleOffset(sampleoffset)) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTSETSAMPLEOFFSET;
		}

		/* Poll for data from connection */

		if ((status = vtransmit->Poll()) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTPOLLDATA;
		}
		
		/* Add input from the connection to the mixer */
		
		if ((status = vtransmit->StartVoiceSourceIteration()) < 0)
		{
			*componenterror = status;
			return ERR_VOICECALL_CANTSTARTITERATION;
		}

		if (vtransmit->GotoFirstVoiceSource())
		{
			VOIPuint64 sourceid;
			
			do
			{
				sourceid = vtransmit->GetVoiceSourceID();
				do
				{
					block.ClearAllVoiceBlockParams();
					
					if ((status = vtransmit->GetSampleBlock(&block)) < 0)
					{
						vtransmit->EndVoiceSourceIteration();
						*componenterror = status;
						return ERR_VOICECALL_CANTGETTRANSMISSIONBLOCK;
					}
				
					if (vdecompress != NULL)
					{
						if ((status = vdecompress->Decompress(&block,sourceid)) < 0)
						{
							vtransmit->EndVoiceSourceIteration();
							*componenterror = status;
							return ERR_VOICECALL_CANTDECOMPRESS;
						}
					}

					if (transf3d != NULL)
					{
						if ((status = transf3d->Create3DEffect(&block,sourceid)) < 0)
						{
							vtransmit->EndVoiceSourceIteration();
							*componenterror = status;
							return ERR_VOICECALL_CANTCREATE3DEFFECT;
						}
					}
					if ((status = vmixer->AddBlock(&block,sourceid)) < 0)
					{
						vtransmit->EndVoiceSourceIteration();
						*componenterror = status;
						return ERR_VOICECALL_CANTADDBLOCKTOMIXER;
					}
				} while (vtransmit->SourceHasMoreData());
			} while (vtransmit->GotoNextVoiceSource());
		}
		vtransmit->EndVoiceSourceIteration();
		*intervalpassed = true;
	}
	else
		*intervalpassed = false;

	return 0;
}

// end namespace
};
