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

#include <stdlib.h>
#include "jvoipmixerstatus.h"

#include "debugnew.h"

JVOIPMixerStatus::JVOIPMixerStatus()
{
	opened = false;
	loaded = false;
}

JVOIPMixerStatus::~JVOIPMixerStatus()
{
	CloseMixer();	
}

int JVOIPMixerStatus::OpenMixer()
{
	if (opened)
		return -1;

	res = mixerOpen(&hmix,0,0,0,MIXER_OBJECTF_MIXER);
	if (res != MMSYSERR_NOERROR)
		return -1;
	res = mixerGetID((HMIXEROBJ)hmix,&mixid,MIXER_OBJECTF_HMIXER);
	if (res != MMSYSERR_NOERROR)
		return -1;
	
	opened = true;
	loaded = false;
	numControlIDs = 0;
	
	return 0;
}

int JVOIPMixerStatus::LoadStatus()
{
	unsigned int i,j,k,num;
	MIXERLINE mixline;
	MIXERCAPS mixercaps;
	MIXERLINECONTROLS mixlinectrl;
	unsigned int LineIDs[MIXSTAT_MAXNUM];
	unsigned int numControls[MIXSTAT_MAXNUM];
	int numlineids;

	if (!opened)
		return -1;
	
	Clear();
	numlineids = 0;

	res = mixerGetDevCaps(mixid,&mixercaps,sizeof(MIXERCAPS));
	if (res != MMSYSERR_NOERROR)
		return -1;
	
	for (i = 0 ; i < mixercaps.cDestinations ; i++)
	{
		mixline.cbStruct = sizeof(MIXERLINE);
		mixline.dwDestination = i;
		res = mixerGetLineInfo((HMIXEROBJ)hmix,&mixline,MIXER_GETLINEINFOF_DESTINATION);
		if (res == MMSYSERR_NOERROR)
		{
			LineIDs[numlineids] = mixline.dwLineID;
			numControls[numlineids] = mixline.cControls;
			numlineids++;

			num = mixline.cConnections;
			for (k = 0 ; k < num ; k++)
			{
				mixline.cbStruct = sizeof(MIXERLINE);
				mixline.dwDestination = i;
				mixline.dwSource = k;
				
				res = mixerGetLineInfo((HMIXEROBJ)hmix,&mixline,MIXER_GETLINEINFOF_SOURCE);
				if (res == MMSYSERR_NOERROR)
				{
					LineIDs[numlineids] = mixline.dwLineID;
					numControls[numlineids] = mixline.cControls;
					numlineids++;
				}
			}
		}
	}

	for (i = 0 ; i < (unsigned int)numlineids ; i++)
	{
		int end;

		mixlinectrl.cbStruct = sizeof(MIXERLINECONTROLS);
		mixlinectrl.cControls = numControls[i];
		mixlinectrl.dwLineID = LineIDs[i];
		mixlinectrl.pamxctrl = mixctrl+numControlIDs;
		mixlinectrl.cbmxctrl = sizeof(MIXERCONTROL);

		res = mixerGetLineControls((HMIXEROBJ)hmix,&mixlinectrl,MIXER_GETLINECONTROLSF_ALL);
		if (res == MMSYSERR_NOERROR)
		{
			end = numControlIDs + numControls[i];
			for (j = numControlIDs ; j < (unsigned int)end ; j++)
				GetControlDetails(j);
			numControlIDs = end;
		}
	}

	loaded = true;
	return 0;
}

int JVOIPMixerStatus::RestoreStatus()
{
	int pos;
	unsigned int i;

	if (!opened)
		return -1;
	if (!loaded)
		return -1;

	for (pos = 0 ; pos < numControlIDs ; pos++)
	{
		MIXERCONTROLDETAILS mixdet;
		MIXERCONTROLDETAILS_BOOLEAN mcdb[32];
		MIXERCONTROLDETAILS_SIGNED mcds[32];
		MIXERCONTROLDETAILS_UNSIGNED mcdu[32];
		unsigned int num;
		int size;
		int type;
		
		switch(mixctrl[pos].dwControlType)
		{
		case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
		case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
		case MIXERCONTROL_CONTROLTYPE_BUTTON:
		case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
		case MIXERCONTROL_CONTROLTYPE_MONO:
		case MIXERCONTROL_CONTROLTYPE_MUTE:
		case MIXERCONTROL_CONTROLTYPE_ONOFF:
		case MIXERCONTROL_CONTROLTYPE_STEREOENH:
		case MIXERCONTROL_CONTROLTYPE_MIXER:
		case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
		case MIXERCONTROL_CONTROLTYPE_MUX:
		case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
			size = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
			type = 1;
			break;
		case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
		case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
		case MIXERCONTROL_CONTROLTYPE_SIGNED:
		case MIXERCONTROL_CONTROLTYPE_DECIBELS:
		case MIXERCONTROL_CONTROLTYPE_PAN:
		case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
		case MIXERCONTROL_CONTROLTYPE_SLIDER:
			size = sizeof(MIXERCONTROLDETAILS_SIGNED);
			type = 2;
			break;
		case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
		case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
		case MIXERCONTROL_CONTROLTYPE_BASS:
		case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
		case MIXERCONTROL_CONTROLTYPE_FADER:
		case MIXERCONTROL_CONTROLTYPE_TREBLE:
		case MIXERCONTROL_CONTROLTYPE_VOLUME:
		case MIXERCONTROL_CONTROLTYPE_MICROTIME:
		case MIXERCONTROL_CONTROLTYPE_MILLITIME:
		case MIXERCONTROL_CONTROLTYPE_PERCENT:
			size = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
			type = 3;
			break;
		default:
			return -1;
		}

		num =  mixctrl[pos].cMultipleItems;
		if (num == 0)
			num = 1;

		if (ctrldetails[pos] != NULL)
		{
			if (type == 1)
			{
				for (i = 0 ; i < num ; i++)
				{
					memcpy(&mcdb[i],ctrldetails[pos]+i*size,size);
					if (mcdb[i].fValue == 0)
						mcdb[i].fValue = 1;
					else
						mcdb[i].fValue = 0;
				}

				mixdet.cbStruct = sizeof(MIXERCONTROLDETAILS);
				mixdet.dwControlID = mixctrl[pos].dwControlID;
				mixdet.cChannels = 1;
				mixdet.cMultipleItems = mixctrl[pos].cMultipleItems;
				mixdet.cbDetails = size;
				mixdet.paDetails = mcdb;
				
				res = mixerSetControlDetails((HMIXEROBJ)hmix,&mixdet,MIXER_GETCONTROLDETAILSF_VALUE);
			}
			else if (type == 2)
			{
				for (i = 0 ; i < num ; i++)
				{
					memcpy(&mcds[i],ctrldetails[pos]+i*size,size);
					if (mcds[i].lValue > 1000)
						mcds[i].lValue -= 1000;
					else
						mcds[i].lValue += 1000;
				}

				mixdet.cbStruct = sizeof(MIXERCONTROLDETAILS);
				mixdet.dwControlID = mixctrl[pos].dwControlID;
				mixdet.cChannels = 1;
				mixdet.cMultipleItems = mixctrl[pos].cMultipleItems;
				mixdet.cbDetails = size;
				mixdet.paDetails = mcds;
				res = mixerSetControlDetails((HMIXEROBJ)hmix,&mixdet,MIXER_GETCONTROLDETAILSF_VALUE);
			}
			else
			{
				for (i = 0 ; i < num ; i++)
				{
					memcpy(&mcdu[i],ctrldetails[pos]+i*size,size);
					if (mcdu[i].dwValue > 1000)
						mcdu[i].dwValue -= 1000;
					else
						mcdu[i].dwValue += 1000;
				}

				mixdet.cbStruct = sizeof(MIXERCONTROLDETAILS);
				mixdet.dwControlID = mixctrl[pos].dwControlID;
				mixdet.cChannels = 1;
				mixdet.cMultipleItems = mixctrl[pos].cMultipleItems;
				mixdet.cbDetails = size;
				mixdet.paDetails = mcdu;
				res = mixerSetControlDetails((HMIXEROBJ)hmix,&mixdet,MIXER_GETCONTROLDETAILSF_VALUE);
			}

			// Now set it really OK

			mixdet.cbStruct = sizeof(MIXERCONTROLDETAILS);
			mixdet.dwControlID = mixctrl[pos].dwControlID;
			mixdet.cChannels = 1;
			mixdet.cMultipleItems = mixctrl[pos].cMultipleItems;
			mixdet.cbDetails = size;
			mixdet.paDetails = ctrldetails[pos];
			res = mixerSetControlDetails((HMIXEROBJ)hmix,&mixdet,MIXER_GETCONTROLDETAILSF_VALUE);
		}
	}
	return 0;
}

int JVOIPMixerStatus::CloseMixer()
{
	if (!opened)
		return -1;
	Clear();
	mixerClose(hmix);
	opened = false;
	return 0;
}

void JVOIPMixerStatus::Clear()
{
	int i;

	for (i = 0 ; i < numControlIDs ; i++)
	{
		if (ctrldetails[i] != NULL)
			delete [] ctrldetails[i];
	}
	numControlIDs = 0;
}


int JVOIPMixerStatus::GetControlDetails(int pos)
{
	MIXERCONTROLDETAILS mixdet;
	int num,size;

	switch(mixctrl[pos].dwControlType)
	{
	case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
	case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
	case MIXERCONTROL_CONTROLTYPE_BUTTON:
	case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
	case MIXERCONTROL_CONTROLTYPE_MONO:
	case MIXERCONTROL_CONTROLTYPE_MUTE:
	case MIXERCONTROL_CONTROLTYPE_ONOFF:
	case MIXERCONTROL_CONTROLTYPE_STEREOENH:
	case MIXERCONTROL_CONTROLTYPE_MIXER:
	case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
	case MIXERCONTROL_CONTROLTYPE_MUX:
	case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
		size = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		break;
	case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
	case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
	case MIXERCONTROL_CONTROLTYPE_SIGNED:
	case MIXERCONTROL_CONTROLTYPE_DECIBELS:
	case MIXERCONTROL_CONTROLTYPE_PAN:
	case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
	case MIXERCONTROL_CONTROLTYPE_SLIDER:
		size = sizeof(MIXERCONTROLDETAILS_SIGNED);
		break;
	case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
	case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
	case MIXERCONTROL_CONTROLTYPE_BASS:
	case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
	case MIXERCONTROL_CONTROLTYPE_FADER:
	case MIXERCONTROL_CONTROLTYPE_TREBLE:
	case MIXERCONTROL_CONTROLTYPE_VOLUME:
	case MIXERCONTROL_CONTROLTYPE_MICROTIME:
	case MIXERCONTROL_CONTROLTYPE_MILLITIME:
	case MIXERCONTROL_CONTROLTYPE_PERCENT:
		size = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
		break;
	default:
		return -1;
	}

	num = mixctrl[pos].cMultipleItems;
	if (num == 0)
		num = 1;

	ctrldetails[pos] = new unsigned char [size*num];
	if (ctrldetails[pos] == NULL)
		return -1;

	mixdet.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixdet.dwControlID = mixctrl[pos].dwControlID;
	mixdet.cChannels = 1;
	mixdet.cMultipleItems = mixctrl[pos].cMultipleItems;
	mixdet.cbDetails = size;
	mixdet.paDetails = ctrldetails[pos];
	res = mixerGetControlDetails((HMIXEROBJ)hmix,&mixdet,MIXER_GETCONTROLDETAILSF_VALUE);
	if (res != MMSYSERR_NOERROR)
	{
		delete [] ctrldetails[pos];
		ctrldetails[pos] = NULL;
		return -1;
	}
	return 0;
}	
