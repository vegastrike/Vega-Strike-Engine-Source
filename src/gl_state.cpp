/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Alan Shieh
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "gfxlib.h"
#include "vegastrike.h"
#include "gl_globals.h"
#include <stack>
BOOL bTex0 = TRUE;
BOOL bTex1 = TRUE;

BOOL /*GFXDRVAPI*/ GFXEnable (STATE state)
{

	switch(state)
	{
	case LIGHTING:
		glEnable(GL_LIGHTING);
		break;
	case DEPTHTEST:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);
		break;
	case DEPTHWRITE:
		glDepthMask(1);
		break;
	case TEXTURE0:
		bTex0 = TRUE;
		break;
	case TEXTURE1:
		bTex1 = TRUE;
		break;
	case CULLFACE:
	  glEnable(GL_CULL_FACE);
	  break;
	}
	return TRUE;
	
}

BOOL /*GFXDRVAPI*/ GFXDisable (STATE state)
{
	
	switch(state)
	{
	case LIGHTING:
		glDisable(GL_LIGHTING);
		break;
	case DEPTHTEST:
		glDisable(GL_DEPTH_TEST);
		break;
	case DEPTHWRITE:
		glDepthMask(0);
		break;
	case TEXTURE0:
		bTex0 = FALSE;
		bTex1 = FALSE;
		break;
	case TEXTURE1:
		bTex1 = FALSE;
		break;
	case CULLFACE:
	  glDisable(GL_CULL_FACE);
	  break;
	}
	return TRUE;
}

BOOL GFXTextureAddressMode(ADDRESSMODE mode)
{
	float BColor [4] = {0,0,0,0};//set border color to clear... dunno if we wanna change?
	switch(mode)
	{
	case WRAP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);		
		break;
	case CLAMP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);		

		break;

	case BORDER:
		
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR, BColor);
		break;
	case MIRROR:
		//nope not goin here I hope
	default:
		return FALSE; // won't work
	}
	return TRUE;
}

struct BlendMode {
	BLENDFUNC sfactor, dfactor;
	BlendMode() {sfactor = dfactor = ONE;};
} currBlendMode;

stack<BlendMode> blendstack;

BOOL GFXBlendMode(enum BLENDFUNC src, enum BLENDFUNC dst)
{
	GLenum sfactor,dfactor;
	switch (src)
	{
	case ZERO:			sfactor = GL_ZERO;            
	break;
	case ONE:			sfactor = GL_ONE;
	break;
	case SRCALPHA:		sfactor = GL_SRC_ALPHA;
	break;
	case INVSRCALPHA:	sfactor = GL_ONE_MINUS_SRC_ALPHA;
	break;
	case DESTALPHA:		sfactor = GL_DST_ALPHA;
	break;
	case INVDESTALPHA:	sfactor = GL_ONE_MINUS_DST_ALPHA;
	break;
	case DESTCOLOR:		sfactor = GL_DST_COLOR;
	break;
	case INVDESTCOLOR:	sfactor = GL_ONE_MINUS_DST_COLOR;
	break;
	case SRCALPHASAT:	sfactor = GL_SRC_ALPHA_SATURATE;
	break;
	case SRCCOLOR:		
	case INVSRCCOLOR:	
	case BOTHSRCALPHA:   
	case BOTHINVSRCALPHA:
	case FORCE_DWORD:
	default:		
	return FALSE;
	}
	
	switch (dst)
	{
		case ZERO:			dfactor = GL_ZERO;            
		break;
		case ONE:			dfactor = GL_ONE;
		break;
		case SRCCOLOR:		dfactor = GL_SRC_COLOR;
		break;
		case INVSRCCOLOR:	dfactor = GL_ONE_MINUS_SRC_COLOR;
		break;
		case SRCALPHA:		dfactor = GL_SRC_ALPHA;
		break;
		case INVSRCALPHA:	dfactor = GL_ONE_MINUS_SRC_ALPHA;
		break;
		case DESTALPHA:		dfactor = GL_DST_ALPHA;
		break;
		case INVDESTALPHA:	dfactor = GL_ONE_MINUS_DST_ALPHA;
		break;
		case DESTCOLOR:		
		case INVDESTCOLOR:	
		case SRCALPHASAT:	
		case BOTHSRCALPHA:   
		case BOTHINVSRCALPHA:
		case FORCE_DWORD:
		default:		
		return FALSE;
	}
	glBlendFunc (sfactor, dfactor);
	currBlendMode.sfactor = src;
	currBlendMode.dfactor = dst;
	return TRUE;
}

BOOL GFXPushBlendMode()
{
	blendstack.push(currBlendMode);
	return TRUE;
}

BOOL GFXPopBlendMode()
{
	if(!blendstack.empty())
	{
		currBlendMode = blendstack.top();
		GFXBlendMode(currBlendMode.sfactor, currBlendMode.dfactor);
		blendstack.pop();
		return TRUE;
	}
	else return FALSE;
}

BOOL GFXDepthFunc(enum DEPTHFUNC dfunc)
{
	switch (dfunc)
	{
	case NEVER:		glDepthFunc (GL_NEVER);
		break;
	case LESS:		glDepthFunc (GL_LESS);
		break;
	case EQUAL:		glDepthFunc (GL_EQUAL);
		break;
	case LEQUAL:	glDepthFunc (GL_LEQUAL);
		break;

	case GREATER:	glDepthFunc (GL_GREATER);
		break;
	case NEQUAL:	glDepthFunc (GL_NOTEQUAL);
		break;
	case GEQUAL:	glDepthFunc (GL_GEQUAL);
		break;
	case ALWAYS:	glDepthFunc (GL_ALWAYS); 
		break;
	};
	return TRUE;

}

BOOL /*GFXDRVAPI*/ GFXSelectTexcoordSet(int stage, int texset)
{
	if (stage)
	{
		GFXStage1 = texset;
	}
	else
	{
		GFXStage0 = texset;
	}
	return TRUE;
}

BOOL GFXSetTexFunc(int stage, int texset)
{

	if (stage)
	{
		GFXStage1 = texset;
	}
	else
	{
		GFXStage0 = texset;
	}	
	if (g_game.Multitexture)
	{
		if (!stage)
		{
			glActiveTextureARB(GL_TEXTURE0_ARB);	
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
		if (stage==1)
		{
			glActiveTextureARB(GL_TEXTURE1_ARB);	
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		glActiveTextureARB(GL_TEXTURE0_ARB);	
	}	
	else return FALSE;
	
	return TRUE;
}

