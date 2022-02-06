/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

///A single OpenGL texture
#include "gldrv/gl_globals.h"
#include <string>
#include <stdio.h>
class Texture {
public:
    enum ColorMask {
        COLOR_MASK_PALETTE = 1,
        COLOR_MASK_COLOR = 2,
        COLOR_MASK_ALPHA = 4
    };
    enum ColorType {
        COLOR_TYPE_UNKNOWN = -1,
        COLOR_TYPE_GRAY = 0,
        COLOR_TYPE_PALETTE = (COLOR_MASK_COLOR | COLOR_MASK_PALETTE),
        COLOR_TYPE_RGB = (COLOR_MASK_COLOR),
        COLOR_TYPE_RGB_ALPHA = (COLOR_MASK_COLOR | COLOR_MASK_ALPHA),
        COLOR_TYPE_GRAY_ALPHA = (COLOR_MASK_ALPHA)
    };
    struct FileData {
        int bpp;
        ColorType color_type;
        unsigned char *data;
        unsigned char *palette;
        unsigned int width;
        unsigned int height;

        FileData() : bpp(8), color_type(COLOR_TYPE_UNKNOWN), data(NULL), palette(NULL), width(0), height(0)
        {
        }
    };
    typedef void (*TextureTransform)(FileData *data, unsigned char **row_pointers);

    static void defaultTransform(FileData *data, unsigned char **row_pointers)
    {
        //do nothing
    }

/// Gets the data and information of the open file fp and returns the information into the pointer to the data struct.  Returns true if successful or else false.
    static bool getTextureData(FILE *fp, FileData *data, TextureTransform tt = defaultTransform);
/// Gets the data and information of the filename and returns the information into the pointer to the data struct.  Returns true if successful or else prints an error and returns false.
    static bool getTextureData(const std::string &file, FileData *data, TextureTransform tt = defaultTransform);

private:
/// Private copy constructor to generate errors.
    Texture(const Texture &)
    {
    }

/// Private = operator to generate errors.
    Texture &operator=(const Texture &)
    {
        return *this;
    }

    unsigned int handle;       ///< An OpenGL handle to the texture.
/// Used to make sure that Model s that call Draw two times do not have to Bind the texture more than one time.
    static unsigned int last_handle;
public:
/// This constructor opens a file and generates an OpenGl texture using that data.  handle is set to that value.
    Texture(const std::string &file, TextureTransform tt = defaultTransform);
/// Destructor only called if no more Texture s with this file are being used.
    Texture(FILE *file, TextureTransform tt = defaultTransform);
/// Destructor only called if no more Texture s with this file are being used.
    ~Texture();
/// Opens a file and generates an OpenGl texture using that data.  handle is set to that value..
    void loadTexture(const std::string &file, TextureTransform tt = defaultTransform);
/// Opens a file and generates an OpenGl texture using that data.  handle is set to that value..
    void loadTexture(FILE *file, TextureTransform tt = defaultTransform);
/// This calls the opengl bind function.  This does not do anything if handle == last_handle to save time and then sets last_handle to this handle
    void bind();

/// Returns true if the texture is loaded correctly.
    bool LoadSuccess()
    {
        return ((unsigned int) handle) != ((unsigned int) -1);
    }

/// This is used only for sorting... even though there is no real < for textures.
    bool operator<(const Texture &oth) const
    {
        return handle < oth.handle;
    }

/// Used for comparison between two textures.
    bool operator==(const Texture &oth) const
    {
        return handle == oth.handle;
    }
};

