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

#include "jvoipsessionparams.h"

#include "debugnew.h"

JVOIPSessionParams::JVOIPSessionParams()
{
	inputsamprate = 8000;
	outputsamprate = 8000;
	sampinterval = 20;
	
	inputtype = SoundcardInput;
	outputtype = SoundcardOutput;
	loctype = NoLocalisation;
	comptype = NoCompression;
	mixertype = NormalMixer;
	transtype = RTP;
	receivetype = AcceptAll;
	inputsampenc = EightBit;
	outputsampenc = EightBit;

	params_input = NULL;
	params_output = NULL;
	params_loc = NULL;
	params_comp = NULL;
	params_trans = NULL;
	params_mixer = NULL;
}

JVOIPSessionParams::JVOIPSessionParams(const JVOIPSessionParams &src) throw (JVOIPException)
{
	params_input = NULL;
	params_output = NULL;
	params_loc = NULL;
	params_comp = NULL;
	params_trans = NULL;
	params_mixer = NULL;

	if (!CopyMembers(src))
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
}

JVOIPSessionParams::~JVOIPSessionParams()
{
	if (params_input)
		delete params_input;
	if (params_output)
		delete params_output;
	if (params_loc)
		delete params_loc;
	if (params_comp)
		delete params_comp;
	if (params_trans)
		delete params_trans;
	if (params_mixer)
		delete params_mixer;
}

JVOIPSessionParams &JVOIPSessionParams::operator=(const JVOIPSessionParams &src) throw (JVOIPException)
{
	if (!CopyMembers(src))
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
	return *this;
}

bool JVOIPSessionParams::CopyMembers(const JVOIPSessionParams &src)
{
	inputsamprate = src.inputsamprate;
	outputsamprate = src.outputsamprate;
	sampinterval = src.sampinterval;
	inputsampenc = src.inputsampenc;
	outputsampenc = src.outputsampenc;
	inputtype = src.inputtype;
	outputtype = src.outputtype;
	loctype = src.loctype;
	comptype = src.comptype;
	transtype = src.transtype;
	mixertype = src.mixertype;
	receivetype = src.receivetype;
	
	if (SetVoiceInputParams(src.params_input) < 0)
		return false;
	if (SetVoiceOutputParams(src.params_output) < 0)
		return false;
	if (SetLocalisationParams(src.params_loc) < 0)
		return false;
	if (SetCompressionParams(src.params_comp) < 0)
		return false;
	if (SetMixerParams(src.params_mixer) < 0)
		return false;
	if (SetTransmissionParams(src.params_trans) < 0)
		return false;
	return true;
}
