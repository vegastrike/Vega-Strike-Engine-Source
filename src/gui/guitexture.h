/* 
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
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

#ifndef __GUITEXTURE_H__
#define __GUITEXTURE_H__
#include <string>
#include "guidefs.h"

// The GuiTexture class encapsulates an OpenGL 2D texture.
// There is a cache so that a texture is only read and bound once.
class GuiTexture
{
public:
	// Read a texture from a file and bind it.
	bool read(const std::string& fileName);

	// Bind image to 2D texture in OpenGL, and make this object represent it.
	bool bindRawData(unsigned char* data, int width, int height, bool hasAlpha, const std::string& fileName);

	// Draw this texture, stretching to fit the rect.
	void draw(const Rect& rect) const;

    // CONSTRUCTION
	GuiTexture(void);
	virtual ~GuiTexture(void) {};

protected:
    // INTERNAL IMPLEMENTATION

protected:
    // VARIABLES
	GLuint m_glName;		// Unique ID for this texture used in OpenGL.
	std::string m_fileName;	// Name of the file this came from.
	int m_width, m_height;	// Original size of image.
};

#endif   // __GUITEXTURE_H__
