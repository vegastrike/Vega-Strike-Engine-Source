/* 
 * Vega Strike
 * Copyright (C) 2003 Mike Byron.
 * Some code borrowed from David Ranger.
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

#include "vegastrike.h"

#include "guitexture.h"


#include <gnuhash.h>


#include <string>
#include <png.h>
#include "vsfilesystem.h"
#include "gfx/vsimage.h"
#include "gldrv/gl_globals.h"
#include "gfx/aux_texture.h"

using namespace VSFileSystem;

// Read a texture from a file and bind it.
bool GuiTexture::read(const std::string& fileName) {
	Texture *oldTexture (m_texture);
	m_texture = new Texture(fileName.c_str(), 0, BILINEAR);
	if (m_texture && !m_texture->LoadSuccess() && oldTexture) {
		delete m_texture;
		m_texture = oldTexture;
	} else {
		delete oldTexture;
	}
	return m_texture->LoadSuccess();
}

// Draw this texture, stretching to fit the rect.
void GuiTexture::draw(const Rect& rect) const {
	// Don't draw unless there is something usable.
	if(m_texture == NULL || !m_texture->LoadSuccess()) {
		return;
	}

	m_texture->MakeActive();
	GFXBegin(GFXQUAD);
	GFXColor4f(1, 1, 1, 1);
	GFXTexCoord2f(0, 1);
	GFXVertexf(Vector(rect.left(), rect.top(), 0.00f));
	GFXTexCoord2f(0, 0);
	GFXVertexf(Vector(rect.left(), rect.bottom(), 0.00f));
	GFXTexCoord2f(1, 0);
	GFXVertexf(Vector(rect.right(), rect.bottom(), 0.00f));
	GFXTexCoord2f(1, 1);
	GFXVertexf(Vector(rect.right(), rect.top(), 0.00f));
	GFXEnd();
}

// CONSTRUCTION
GuiTexture::GuiTexture(void)
: m_texture(NULL)
{
}

GuiTexture::~GuiTexture(void)
{
	if (m_texture) delete m_texture;
}

