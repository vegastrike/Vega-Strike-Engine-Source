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

#include <map>
#include <string>
#include <png.h>
#include "vsfilesystem.h"
#include "gfx/vsimage.h"
using namespace VSFileSystem;


// TEXTURE CACHE

// Typedefs to make things easier.
typedef std::map<std::string, GuiTexture> TextureCacheType;		// The cache.
typedef std::pair<std::string, GuiTexture> TextureCacheEntry;	// Entry in the cache.

// The cache.
static TextureCacheType textureCache;

// Helper function for readImage.
static void png_cexcept_error(png_structp png_ptr, png_const_charp msg) {
#ifndef PNG_NO_CONSOLE_IO
		VSFileSystem::vs_fprintf(stderr, "libpng error: %s\n", msg);
#endif
}

// Read an image file.  Borrowed from code by David Ranger.
static unsigned char* readImageFile(FILE* fp, int& bpp, int& color_type, int& width, int& height) {
		bool strip_16 = 0;
		unsigned char sig[8];
		png_structp png_ptr;
		png_bytepp row_pointers;
		png_infop info_ptr;
		int interlace_type;
		VSFileSystem::vs_read(sig, 1, 8, fp);
		if (!png_check_sig(sig, 8)) {
				return NULL;    /* bad signature */
		}
		png_ptr = png_create_read_struct(
						PNG_LIBPNG_VER_STRING, NULL,
						(png_error_ptr)png_cexcept_error,
						(png_error_ptr)NULL
						);
		if (png_ptr == NULL) {
				return NULL;
		}
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
				png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
				return NULL;
		}
		if (setjmp(png_jmpbuf(png_ptr))) {
				/* Free all of the memory associated with the png_ptr and info_ptr */
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				/* If we get here, we had a problem reading the file */
				return NULL;
		}
		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);

		png_read_info(png_ptr, info_ptr);       /* read all PNG info up to image data */
		png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&width, (png_uint_32 *)&height, &bpp, &color_type, &interlace_type, NULL, NULL);
# if __BYTE_ORDER != __BIG_ENDIAN
		if (bpp==16)
				png_set_swap (png_ptr);
#endif
		if (bpp==16&&strip_16) {
				png_set_strip_16(png_ptr);
		}
		if (strip_16&&color_type == PNG_COLOR_TYPE_PALETTE) {
				png_set_palette_to_rgb(png_ptr);
		}
		if (color_type == PNG_COLOR_TYPE_GRAY && bpp < 8) {
				png_set_gray_1_2_4_to_8(png_ptr);
		}
		png_set_expand (png_ptr);
		png_read_update_info (png_ptr,info_ptr);

		png_uint_32 localWidth, localHeight;
		png_get_IHDR(png_ptr, info_ptr, &localWidth, &localHeight, &bpp, &color_type, &interlace_type, NULL, NULL);
		row_pointers = (unsigned char **)malloc (sizeof(unsigned char*) * localHeight);
		int numchan=1;
		if (color_type & PNG_COLOR_MASK_COLOR) {
				numchan = 3;
		}
		if (color_type & PNG_COLOR_MASK_PALETTE)
				numchan =1;
		if (color_type & PNG_COLOR_MASK_ALPHA) {
				numchan++;
		}
		unsigned long stride = numchan*sizeof(unsigned char) * bpp/8;
		unsigned char* image = (unsigned char*)malloc(stride*localWidth*localHeight);
		for (unsigned int i=0;i<localHeight;i++) {
				row_pointers[i] = &image[i*stride*localWidth];
		}
		png_read_image (png_ptr,row_pointers);
		free(row_pointers);
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

		width = localWidth;
		height = localHeight;

		return image;
}

// Add a new entry to the cache.
static const GuiTexture* addTextureToCache(const std::string& fileName) {
	assert(textureCache.find(fileName) == textureCache.end());		// Make sure there are no duplicates.

	// Open the file.
	VSFile file;
	VSError err;
	if(fileName.size() > 0) {
			err = file.OpenReadOnly(fileName, TextureFile);
	}
	if(err>Ok) {
		printf("GuiTexture: Cannot open file '%s'.\n", fileName.c_str());
		return NULL;
	}

	// Read the image file.
	int bppDummy;			// Not used.
	int colorType;			// Info about color in the image.
	int width, height;		// Size of the image.
	VSImage img;
	unsigned char* image = img.ReadImage(&file);
	colorType = img.Format();
	width = img.sizeX;
	height = img.sizeY;
	// Make sure file gets closed in case of errors.
	file.Close();
	if(!image) {
		printf("GuiTexture: problem reading texture file '%s'.\n", fileName.c_str());
		return NULL;
	}

	// Bind the texture in OpenGL.
	static const int PNG_HAS_ALPHA = 4;
	const bool hasAlpha = ( colorType & PNG_HAS_ALPHA );
	GuiTexture texture;
	const bool success = texture.bindRawData(image, width, height, hasAlpha, fileName);
	free(image);			// Make sure this gets freed before we return.
	if(!success) {
		printf("GuiTexture: Problem binding texture from file '%s'.\n", fileName.c_str());
		return NULL;
	}

	// Store the new texture in our cache.
	textureCache.insert(TextureCacheEntry(fileName,texture));

	return &textureCache.find(fileName)->second;
}

// Read a texture from a file and bind it.
bool GuiTexture::read(const std::string& fileName) {
	const GuiTexture* texture = NULL;

	TextureCacheType::const_iterator i = textureCache.find(fileName);
	if(i == textureCache.end()) {
		// Don't have it yet.  Try to read it.
		texture = addTextureToCache(fileName);
		if(!texture) {
			// Assume addTextureToCache put out an error message.
			return false;
		}
	} else {
		// In the cache already.
		texture = &i->second;
	}

	// Copy the state from the cached object to this one.
	*this = *texture;

	return true;
}

// Bind image to 2D texture in OpenGL, and make this object represent it.
bool GuiTexture::bindRawData(unsigned char* data, int width, int height, bool hasAlpha, const std::string& fileName) {
	// Get a unique id for the texture.  OpenGL refers to these as texture "names".
    glGenTextures (1,&m_glName);

	// Set up the properties of this texture.
    glBindTexture(GL_TEXTURE_2D, m_glName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(
		GL_TEXTURE_2D,
		0,
		(hasAlpha?GL_RGBA8:GL_RGB8),
		width,
		height,
		0, 
		(hasAlpha?GL_RGBA:GL_RGB),
		GL_UNSIGNED_BYTE,
		data
		);

	// Keep the width and height.
	m_width = width;
	m_height = height;

	m_fileName = fileName;

	return true;
}

// Draw this texture, stretching to fit the rect.
void GuiTexture::draw(const Rect& rect) const {
	// Don't draw unless there is something usable.
	if(m_width <= 0 || m_height <= 0) {
		return;
	}

	glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, m_glName);
        glBegin(GL_QUADS);
			glColor4f(1, 1, 1, 1);

            glTexCoord2f(0, 1);
            glVertex2f(rect.left(), rect.top());

            glTexCoord2f(0, 0);
            glVertex2f(rect.left(), rect.bottom());

            glTexCoord2f(1, 0);
            glVertex2f(rect.right(), rect.bottom());

            glTexCoord2f(1, 1);
            glVertex2f(rect.right(), rect.top());
        glEnd();

	glDisable(GL_TEXTURE_2D);
}

// CONSTRUCTION
GuiTexture::GuiTexture(void)
:
m_glName(0),
m_fileName(),
m_width(0),
m_height(0)
{
}
