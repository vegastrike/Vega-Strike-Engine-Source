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

#ifndef JVOIPCOMPONENT_H

#define JVOIPCOMPONENT_H

#include "jvoipcomponentparams.h"
#include "jvoipexception.h"
#include <stdlib.h>
#include <string>
#include <vector>

class JVOIPSession;

class JVOIPCompParamInfo
{
public:
	void SetParameterName(const std::string &name) { paramname = name; }
	void SetParameterValue(const std::string &val) { paramval = val; }
	std::string GetParameterName() const { return paramname; }
	std::string GetParameterValue() const { return paramval; }
private:
	std::string paramname;
	std::string paramval;
};

class JVOIPComponentState
{
public:
	JVOIPComponentState() { }
	virtual ~JVOIPComponentState() { }
};

class JVOIPComponent
{
public:
	JVOIPComponent(JVOIPSession *sess) { voipsession = sess; }
	virtual ~JVOIPComponent() { }
	
	virtual int GetComponentState(JVOIPComponentState **compstate) = 0;
	virtual int SetComponentState(JVOIPComponentState *compstate) = 0;
	
	virtual std::string GetComponentName() = 0;
	virtual std::string GetComponentDescription() = 0;
	virtual std::vector<JVOIPCompParamInfo> *GetComponentParameters() throw (JVOIPException) = 0;
protected:
	JVOIPSession *voipsession;
};

#endif // JVOIPCOMPONENT_H
