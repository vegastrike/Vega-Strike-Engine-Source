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

#include "jvoipcompression.h"
#include "jvoipcompressionmodule.h"
#include "jvoiperrors.h"
#include "jvoipsession.h"
#include "voiceblock.h"
#include "jvoipdpcmcompression.h"
#include "jvoipsilencesuppression.h"
#include "jvoipgsmcompression.h"
#include "jvoiplpccompression.h"
#include "jvoipulawcompression.h"

#include "debugnew.h"

JVOIPCompression::JVOIPCompression(JVOIPSession *sess,JVOIPCompressionModule *userdefcompmod):JVOIPComponent(sess)
{
	udef_compmod = userdefcompmod;
	cur_compmod = NULL;
	init = false;
	curmajorcomptype = 0;
}

JVOIPCompression::~JVOIPCompression()
{
	Cleanup();
}

int JVOIPCompression::Init(int sampinterval,int inputsamprate,int inputbytespersample,JVOIPSessionParams::CompressionType comptype,const JVOIPComponentParams *componentparams)
{
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	if ((status = InitModules()) < 0)
		return status;
		
	if ((status = ProcessCompressionType(comptype,&cur_compmod,&curmajorcomptype)) < 0)
	{
		DeleteModules();
		return status;
	}
	if (cur_compmod != NULL) // not 'No compression'
	{
		if ((status = cur_compmod->InitCompressor(sampinterval,inputsamprate,inputbytespersample,componentparams)) < 0)
		{
			if (cur_compmod == udef_compmod)
			{
				DeleteModules();
				return ERR_JVOIPLIB_SESSION_CANTINITUSERDEFCOMPRESSIONMODULE;
			}
			DeleteModules();
			return status;
		}
	}

	init = true;
	return 0;
}

int JVOIPCompression::Cleanup()
{
	int status;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (cur_compmod)
		cur_compmod->CleanupCompressor();
	if ((status = DeleteModules()) < 0)
		return status;
	init = false;
	return 0;
}

int JVOIPCompression::Compress(VoIPFramework::VoiceBlock *vb)
{
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	if (cur_compmod != NULL)
	{
		status = cur_compmod->Compress(vb);
		if (status < 0)
			return status;
	}
	else
		vb->SetMinorCompressionID(0);

	vb->SetMajorCompressionID(curmajorcomptype);
	return 0;
}

int JVOIPCompression::Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	int numsamples;
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	switch(vb->GetMajorCompressionID())
	{
	case JVOIPCOMPRESSIONTYPE_MAJOR_NOCOMPRESSION:
		// No compression
		break;
	case JVOIPCOMPRESSIONTYPE_MAJOR_USERDEFINEDCOMPRESSION:
		if (udef_compmod == NULL)
		{
			vb->Clear();
			return 0;
		}

		status = udef_compmod->Decompress(vb,sourceid);
		if (status < 0)
			return status;
		break;
	case JVOIPCOMPRESSIONTYPE_MAJOR_DPCMCOMPRESSION:
		status = dpcm_compmod->Decompress(vb,sourceid);
		if (status < 0)
			return status;
		break;
	case JVOIPCOMPRESSIONTYPE_MAJOR_GSMCOMPRESSION:
		status = gsm_compmod->Decompress(vb,sourceid);
		if (status < 0)
			return status;
		break;
	case JVOIPCOMPRESSIONTYPE_MAJOR_LPCCOMPRESSION:
		status = lpc_compmod->Decompress(vb,sourceid);
		if (status < 0)
			return status;
		break;
	case JVOIPCOMPRESSIONTYPE_MAJOR_ULAWCOMPRESSION:
		status = ulaw_compmod->Decompress(vb,sourceid);
		if (status < 0)
			return status;
		break;
	default:
		// Unknown compression type
		vb->Clear();
		return 0;
	}

	// The block has been decompressed, we can now calculate the number
	// of samples
		
	numsamples = vb->GetNumBytes();
	if (vb->IsStereo())
		numsamples /= 2;
	if (vb->GetBytesPerSample() != 1)
		numsamples /= 2;
	vb->SetNumSamples(numsamples);
	return 0;
}

int JVOIPCompression::GetComponentState(JVOIPComponentState **compstate)
{
	// NOTE: This function is NOT used!
	*compstate = NULL;
	return 0;
}

int JVOIPCompression::SetComponentState(JVOIPComponentState *compstate)
{
	// NOTE: This function is NOT used!
	return 0;
}

std::string JVOIPCompression::GetComponentName()
{
	if (!init)
		return std::string("");
	if (cur_compmod)
		return cur_compmod->GetComponentName();
	return std::string("No compression selected");
}

std::string JVOIPCompression::GetComponentDescription()
{
	if (!init)
		return std::string("");
	if (cur_compmod)
		return cur_compmod->GetComponentDescription();
	return std::string("No compression selected");
}

std::vector<JVOIPCompParamInfo> *JVOIPCompression::GetComponentParameters() throw (JVOIPException)
{
	if (!init)
		return NULL;
	if (cur_compmod)
		return cur_compmod->GetComponentParameters();
	return NULL;
}

bool JVOIPCompression::SupportsSampleInterval(int ival)
{
	if (!init)
		return false;
	if (cur_compmod)
		return cur_compmod->SupportsSampleInterval(ival);
	return true;
}

int JVOIPCompression::SetSampleInterval(int ival)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (cur_compmod)
	{
		int status;

		status = cur_compmod->SetSampleInterval(ival);
		if ((status < 0) && (cur_compmod == udef_compmod))
			return ERR_JVOIPLIB_SESSION_CANTSETUSERDEFCOMPRESSIONSAMPINT;
		return status;
	}
	return 0;
}

bool JVOIPCompression::SupportsInputSamplingRate(int irate)
{
	if (!init)
		return false;
	if (cur_compmod)
		return cur_compmod->SupportsInputSamplingRate(irate);
	return true;
}

int JVOIPCompression::SetInputSamplingRate(int irate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (cur_compmod)
	{
		int status;

		status = cur_compmod->SetInputSamplingRate(irate);
		if ((status < 0) && (cur_compmod == udef_compmod))
			return ERR_JVOIPLIB_SESSION_CANTSETUSERDEFCOMPRESSIONINPUTSAMPRATE;
		return status;
	}
	return 0;
}

bool JVOIPCompression::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return false;
	if (cur_compmod)
		return cur_compmod->SupportsInputBytesPerSample(inputbytespersample);
	return true;
}

int JVOIPCompression::SetInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (cur_compmod)
	{
		int status;
		
		status = cur_compmod->SetInputBytesPerSample(inputbytespersample);
		if ((status < 0) && (cur_compmod == udef_compmod))
			return ERR_JVOIPLIB_SESSION_CANTSETUSERDEFCOMPRESSIONINPUTENCODING;
		return status;
	}
	return 0;
}

int JVOIPCompression::ProcessCompressionType(JVOIPSessionParams::CompressionType comptype,JVOIPCompressionModule **compmod,int *majorcomptype)
{
	switch(comptype)
	{
	case JVOIPSessionParams::NoCompression:
		*compmod = NULL;
		*majorcomptype = JVOIPCOMPRESSIONTYPE_MAJOR_NOCOMPRESSION;
		break;
	case JVOIPSessionParams::SilenceSuppression:
		*compmod = silsup_compmod;
		*majorcomptype = JVOIPCOMPRESSIONTYPE_MAJOR_NOCOMPRESSION;
		break;
	case JVOIPSessionParams::UserDefinedCompression:
		if (udef_compmod == NULL)
			return ERR_JVOIPLIB_SESSION_USERDEFCOMPRESSIONISNULL;
		*compmod = udef_compmod;
		*majorcomptype = JVOIPCOMPRESSIONTYPE_MAJOR_USERDEFINEDCOMPRESSION;
		break;
	case JVOIPSessionParams::DPCM:
		*compmod = dpcm_compmod;
		*majorcomptype = JVOIPCOMPRESSIONTYPE_MAJOR_DPCMCOMPRESSION;
		break;
	case JVOIPSessionParams::GSM:
		*compmod = gsm_compmod;
		*majorcomptype = JVOIPCOMPRESSIONTYPE_MAJOR_GSMCOMPRESSION; 
		break;
	case JVOIPSessionParams::LPC:
		*compmod = lpc_compmod;
		*majorcomptype = JVOIPCOMPRESSIONTYPE_MAJOR_LPCCOMPRESSION; 
		break;
	case JVOIPSessionParams::ULawEncoding:
		*compmod = ulaw_compmod;
		*majorcomptype = JVOIPCOMPRESSIONTYPE_MAJOR_ULAWCOMPRESSION; 
		break;
	default:
		return ERR_JVOIPLIB_SESSION_ILLEGALCOMPRESSIONTYPE;
	}
	if (*compmod == NULL && comptype != JVOIPSessionParams::NoCompression)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	return 0;
}

#define CREATE_AND_INIT(compmod,ClassName) \
	{ \
		compmod = new ClassName(voipsession); \
		if (compmod == NULL) \
		{ \
			DeleteModules(); \
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM; \
		} \
		if ((status = compmod->InitDecompressor()) < 0) \
		{ \
			DeleteModules(); \
			return status; \
		} \
	}

int JVOIPCompression::InitModules()
{
	int status;

	// Set the compression components to NULL
	silsup_compmod = NULL;
	dpcm_compmod = NULL;
	gsm_compmod = NULL;
	lpc_compmod = NULL;
	ulaw_compmod = NULL;

	// Here, all modules are created and their decompressors are initialized
	
	CREATE_AND_INIT(silsup_compmod,JVOIPSilenceSuppression);
	CREATE_AND_INIT(dpcm_compmod,JVOIPDPCMCompression);
	CREATE_AND_INIT(gsm_compmod,JVOIPGSMCompression);
	CREATE_AND_INIT(lpc_compmod,JVOIPLPCCompression);
	CREATE_AND_INIT(ulaw_compmod,JVOIPULawCompression);

	// init the user defined module
	if (udef_compmod)
	{
		status = udef_compmod->InitDecompressor();
		if (status < 0)
		{
			DeleteModules();
			return ERR_JVOIPLIB_SESSION_CANTINITUSERDEFCOMPRESSIONMODULE;
		}
	}
	return 0;
}

int JVOIPCompression::DeleteModules()
{
	if (silsup_compmod)
		delete silsup_compmod;
	if (dpcm_compmod)
		delete dpcm_compmod;
	if (gsm_compmod)
		delete gsm_compmod;
	if (lpc_compmod)
		delete lpc_compmod;
	if (ulaw_compmod)
		delete ulaw_compmod;
	return 0;
}

int JVOIPCompression::SetCompressionType(JVOIPSessionParams::CompressionType comptype,const JVOIPComponentParams *componentparams,bool *fatalerror)
{
	JVOIPCompressionModule *newmod;
	JVOIPComponentState *compstate = NULL;
	int samprate,bytespersample,sampinterval;
	int newmajorcomptype;
	int status;

	*fatalerror = true;
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	*fatalerror = false;
	if ((status = ProcessCompressionType(comptype,&newmod,&newmajorcomptype)) < 0)
		return status;
		
	if (cur_compmod)
	{
		status = cur_compmod->GetComponentState(&compstate);
		if (status < 0)
		{
			if (cur_compmod == udef_compmod)
				return ERR_JVOIPLIB_SESSION_CANTSAVEUSERDEFCOMPRESSIONSTATE;
			return status;
		}
		
		cur_compmod->CleanupCompressor();
	}
	
	if (newmod != NULL) // no compression selected
	{
		sampinterval = voipsession->SessionParameters().GetSampleInterval();
		bytespersample = (voipsession->SessionParameters().GetInputSampleEncodingType() == JVOIPSessionParams::EightBit)?1:2;
		samprate = voipsession->SessionParameters().GetInputSamplingRate();
	
		status = newmod->InitCompressor(sampinterval,samprate,bytespersample,componentparams);
		if (status < 0)
		{
			// try to reinstall the previous compression module
			
			if (cur_compmod == NULL)
			{
				if (newmod == udef_compmod)
					return ERR_JVOIPLIB_SESSION_CANTINITUSERDEFCOMPRESSIONMODULE;
				return status;
			}
			else // try to reset the component state
			{
				JVOIPComponentParams *oldparams;
			
				// reinitialize the compressor
				oldparams = voipsession->SessionParameters().GetCompressionParams();
				if (cur_compmod->InitCompressor(sampinterval,samprate,bytespersample,oldparams) < 0)
				{
					if (compstate)
						delete compstate;
					*fatalerror = true;
					return ERR_JVOIPLIB_SESSION_ACTIONFAILEDANDRESTOREFAILED;
				}
				
				if (cur_compmod->SetComponentState(compstate) < 0)
				{
					if (compstate)
						delete compstate;
					*fatalerror = true;
					return ERR_JVOIPLIB_SESSION_ACTIONFAILEDANDRESTOREFAILED;
				}
				
				if (compstate)
					delete compstate;
				if (newmod == udef_compmod)
					return ERR_JVOIPLIB_SESSION_CANTINITUSERDEFCOMPRESSIONMODULE;
				return status;
			}
		}
	}
	
	cur_compmod = newmod;
	curmajorcomptype = newmajorcomptype;
	
	if (compstate)
		delete compstate;
	
	return 0;
}

