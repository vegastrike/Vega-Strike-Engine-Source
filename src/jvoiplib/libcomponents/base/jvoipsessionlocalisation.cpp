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

#include "jvoipsessionlocalisation.h"

#include "debugnew.h"

int JVOIPSessionLocalisation::Add3DInfo(VoIPFramework::VoiceBlock *vb)
{
	int len;
	
	if (!voipsession->EncodeOwnPosition(buffer,&len))
	{
		vb->SetInfo3D(NULL,0);
		return 0;		
	}
	
	vb->SetInfo3D(buffer,len);
	vb->SetInfo3DDeleteHandler(DeleteHandler,this);
	return 0;
}

int JVOIPSessionLocalisation::Create3DEffect(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	double lx,ly,lz,rx,ry,rz,eardirx,eardiry,eardirz;
	double frontx,fronty,frontz,upx,upy,upz;

	if (!vb->Has3DInfo())
		return 0;
		
	if (!voipsession->RetrieveOwnPosition(&lx,&ly,&lz,&eardirx,&eardiry,&eardirz,&frontx,&fronty,&frontz,&upx,&upy,&upz))
		return 0;
	if (!voipsession->DecodePositionalInfo(vb->GetInfo3D(false),vb->GetInfo3DLength(),&rx,&ry,&rz))
		return 0;
	vb->SetInfo3D(NULL,0);
	return Create3DEffect(lx,ly,lz,eardirx,eardiry,eardirz,frontx,fronty,frontz,upx,upy,upz,rx,ry,rz,vb,sourceid);
}

void JVOIPSessionLocalisation::DeleteHandler(unsigned char **data,void *param)
{
	*data = NULL; // show that the data does not need to be deleted
}

