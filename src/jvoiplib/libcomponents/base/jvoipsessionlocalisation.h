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

#ifndef JVOIPSESSIONLOCALISATION_H

#define JVOIPSESSIONLOCALISATION_H

#include "jvoiplocalisation.h"
#include "jvoipsession.h"
#include "voiceblock.h"

class JVOIPSessionLocalisation : public JVOIPLocalisation
{
public:
	JVOIPSessionLocalisation(JVOIPSession *sess) : JVOIPLocalisation(sess) { }
	~JVOIPSessionLocalisation() { }
	int Add3DInfo(VoIPFramework::VoiceBlock *vb);
	int Create3DEffect(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid);
	
	virtual int Create3DEffect(double local_x,double local_y,double local_z,
	                           double righteardir_x,double righteardir_y,double righteardir_z,
	                           double frontdir_x,double frontdir_y,double frontdir_z,
				   double updir_x,double updir_y,double updir_z,
				   double remote_x,double remote_y,double remote_z,
	                           VoIPFramework::VoiceBlock *vb,
	                           VoIPFramework::VOIPuint64 sourceid) = 0;
private:
	static void DeleteHandler(unsigned char **data,void *param);
	
	unsigned char buffer[JVOIP_3DINFOBUFFERLEN];
};

#endif // JVOIPSESSIONLOCALISATION_H
