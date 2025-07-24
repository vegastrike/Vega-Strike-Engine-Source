/*
 * gl_texture.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#define GL_EXT_texture_env_combine 1
#include "gldrv/sdds.h"
#include "gl_globals.h"
#include "configuration/configuration.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "src/config_xml.h"
#include "src/gfxlib.h"

#include "root_generic/options.h"
#include "src/vs_logging.h"

#ifndef GL_TEXTURE_CUBE_MAP_EXT
#define GL_TEXTURE_CUBE_MAP_EXT 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 0x851A
#endif

//#define  MAX_TEXTURES 16384
static GLint MAX_TEXTURE_SIZE = 256;

extern GLenum GetGLTextureTarget(enum TEXTURE_TARGET texture_target);

GLenum GetUncompressedTextureFormat(TEXTUREFORMAT textureformat) {
    switch (textureformat) {
        case RGB24:
            return GL_RGB;

        case RGB32:
            return GL_RGB;

        case DXT1RGBA:
        case DXT3:
        case DXT5:
        case DXT1:
        case RGBA32:
            return GL_RGBA;

        case RGBA16:
            return GL_RGBA16;

        case RGB16:
            return GL_RGB16;

        default:
            return GL_RGBA;
    }
}

struct GLTexture {
    //unsigned char *texture;
    GLubyte *palette;
    int width;
    int height;
    int iwidth;                                  //Interface width
    int iheight;                         //Interface height
    int texturestage;
    GLuint name;
    GFXBOOL alive;
    GLenum textureformat;
    GLenum targets;
    enum FILTER mipmapped;
};
//static GLTexture *textures=NULL;
//static GLEnum * targets=NULL;

static vector<GLTexture> textures;
static int activetexture[32] = {
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
};

static void ConvertPalette(unsigned char *dest, unsigned char *src) {
    for (int a = 0; a < 256; a++, dest += 4, src += 4) {
        memcpy(dest, src, 3);
        dest[3] = 255;
    }
}

int tmp_abs(int num) {
    return num < 0 ? -num : num;
}

bool isPowerOfTwo(int num, int &which) {
    which = 0;
    while (tmp_abs(num) > 1) {
        if ((num / 2) * 2 != num) {
            return false;
        }
        which++;
        num /= 2;
    }
    return true;
}

#if defined(__APPLE__) && defined (__MACH__)

static GLint round2( GLint n )
{
    GLint m;
    for (m = 1; m < n; m *= 2)
        ;
    /* m>=n */
    if (m-n <= n-m/2)
        return m;
    else
        return m/2;
}

static GLint bytes_per_pixel( GLenum format, GLenum type )
{
    GLint n, m;
    switch (format)
    {
    case GL_COLOR_INDEX:
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE:
        n = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        n = 2;
        break;
    case GL_RGB:
    case GL_BGR:
        n = 3;
        break;
    case GL_RGBA:
    case GL_BGRA:
#ifdef GL_EXT_abgr
    case GL_ABGR_EXT:
#endif
        n = 4;
        break;
    default:
        n = 0;
    }
    switch (type)
    {
    case GL_UNSIGNED_BYTE:
        m = sizeof (GLubyte);
        break;
    case GL_BYTE:
        m = sizeof (GLbyte);
        break;
    case GL_BITMAP:
        m = 1;
        break;
    case GL_UNSIGNED_SHORT:
        m = sizeof (GLushort);
        break;
    case GL_SHORT:
        m = sizeof (GLshort);
        break;
    case GL_UNSIGNED_INT:
        m = sizeof (GLuint);
        break;
    case GL_INT:
        m = sizeof (GLint);
        break;
    case GL_FLOAT:
        m = sizeof (GLfloat);
        break;
    default:
        m = 0;
    }
    return n*m;
}

static GLint appleBuild2DMipmaps( GLenum target,
                                  GLint components,
                                  GLsizei width,
                                  GLsizei height,
                                  GLenum format,
                                  GLenum type,
                                  const void *data )
{
    GLint     w, h, maxsize;
    void     *image, *newimage;
    GLint     neww, newh, level, bpp;
    int       error;
    GLboolean done;
    GLint     retval = 0;
    GLint     unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
    GLint     packrowlength, packalignment, packskiprows, packskippixels;
    if (width < 1 || height < 1)
        return GLU_INVALID_VALUE;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxsize );

    w   = round2( width );
    if (w > maxsize)
        w = maxsize;
    h   = round2( height );
    if (h > maxsize)
        h = maxsize;
    bpp = bytes_per_pixel( format, type );
    if (bpp == 0)
        /* probably a bad format or type enum */
        return GLU_INVALID_ENUM;
    /* Get current glPixelStore values */
    glGetIntegerv( GL_UNPACK_ROW_LENGTH, &unpackrowlength );
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &unpackalignment );
    glGetIntegerv( GL_UNPACK_SKIP_ROWS, &unpackskiprows );
    glGetIntegerv( GL_UNPACK_SKIP_PIXELS, &unpackskippixels );
    glGetIntegerv( GL_PACK_ROW_LENGTH, &packrowlength );
    glGetIntegerv( GL_PACK_ALIGNMENT, &packalignment );
    glGetIntegerv( GL_PACK_SKIP_ROWS, &packskiprows );
    glGetIntegerv( GL_PACK_SKIP_PIXELS, &packskippixels );

    /* set pixel packing */
    glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glPixelStorei( GL_PACK_SKIP_ROWS, 0 );
    glPixelStorei( GL_PACK_SKIP_PIXELS, 0 );

    done = GL_FALSE;
    if (w != width || h != height) {
        /* must rescale image to get "top" mipmap texture image */
        image = malloc( (w+4)*h*bpp );
        if (!image)
            return GLU_OUT_OF_MEMORY;
        error = gluScaleImage( format, width, height, type, data,
                               w, h, type, image );
        if (error) {
            retval = error;
            done   = GL_TRUE;
        }
    } else {
        image = (void*) data;
    }
    level = 0;
    while (!done) {
        if (image != data) {
            /* set pixel unpacking */
            glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
            glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
        }
        glTexImage2D( target, level, components, w, h, 0, format, type, image );
        if (w == 1 && h == 1)
            break;
        neww     = (w < 2) ? 1 : w/2;
        newh     = (h < 2) ? 1 : h/2;
        newimage = malloc( (neww+4)*newh*bpp );
        if (!newimage)
            return GLU_OUT_OF_MEMORY;
        error    = gluScaleImage( format, w, h, type, image,
                                  neww, newh, type, newimage );
        if (error) {
            retval = error;
            done   = GL_TRUE;
        }
        if (image != data)
            free( image );
        image = newimage;

        w     = neww;
        h     = newh;
        level++;
    }
    if (image != data)
        free( image );
    /* Restore original glPixelStore state */
    glPixelStorei( GL_UNPACK_ROW_LENGTH, unpackrowlength );
    glPixelStorei( GL_UNPACK_ALIGNMENT, unpackalignment );
    glPixelStorei( GL_UNPACK_SKIP_ROWS, unpackskiprows );
    glPixelStorei( GL_UNPACK_SKIP_PIXELS, unpackskippixels );
    glPixelStorei( GL_PACK_ROW_LENGTH, packrowlength );
    glPixelStorei( GL_PACK_ALIGNMENT, packalignment );
    glPixelStorei( GL_PACK_SKIP_ROWS, packskiprows );
    glPixelStorei( GL_PACK_SKIP_PIXELS, packskippixels );

    return retval;
}

#define gluBuild2DMipmaps appleBuild2DMipmaps

#endif

GFXBOOL /*GFXDRVAPI*/ GFXCreateTexture(int width,
        int height,
        TEXTUREFORMAT textureformat,
        int *handle,
        char *palette,
        int texturestage,
        enum FILTER mipmap,
        enum TEXTURE_TARGET texture_target,
        enum ADDRESSMODE address_mode) {
    int dummy = 0;
    if ((mipmap & (MIPMAP | TRILINEAR)) && !isPowerOfTwo(width, dummy)) {
        VS_LOG(info, (boost::format("Width %1% not a power of two") % width));
    }
    if ((mipmap & (MIPMAP | TRILINEAR)) && !isPowerOfTwo(height, dummy)) {
        VS_LOG(info, (boost::format("Height %1% not a power of two") % height));
    }
    GFXActiveTexture(texturestage);
    *handle = 0;
    while (*handle < static_cast<int>(textures.size())) {
        if (!textures.at(*handle).alive) {
            break;
        } else {
            (*handle)++;
        }
    }
    if ((*handle) == static_cast<int>(textures.size())) {
        textures.push_back(GLTexture());
        textures.back().palette = NULL;
        textures.back().alive = GFXTRUE;
        textures.back().name = -1;
        textures.back().width = textures.back().height = textures.back().iwidth = textures.back().iheight = 1;
    }
    switch (texture_target) {
        case TEXTURE1D:
            textures.at(*handle).targets = GL_TEXTURE_1D;
            break;
        case TEXTURE2D:
            textures.at(*handle).targets = GL_TEXTURE_2D;
            break;
        case TEXTURE3D:
            textures.at(*handle).targets = GL_TEXTURE_3D;
            break;
        case CUBEMAP:
            textures.at(*handle).targets = GL_TEXTURE_CUBE_MAP_EXT;
            break;
        case TEXTURERECT:
            textures.at(*handle).targets = GL_TEXTURE_RECTANGLE_ARB;
            break;
    }
    //for those libs with stubbed out handle gen't
    textures.at(*handle).name = *handle + 1;
    textures.at(*handle).alive = GFXTRUE;
    textures.at(*handle).texturestage = texturestage;
    textures.at(*handle).mipmapped = mipmap;
    glGenTextures(1, &textures.at(*handle).name);
    glBindTexture(textures.at(*handle).targets, textures.at(*handle).name);
    activetexture[texturestage] = *handle;
    GFXTextureAddressMode(address_mode, texture_target);
    if (textures.at(*handle).mipmapped & (TRILINEAR | MIPMAP) && gl_options.mipmap >= 2) {
        glTexParameteri(textures.at(*handle).targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (textures.at(*handle).mipmapped & TRILINEAR && gl_options.mipmap >= 3) {
            glTexParameteri(textures.at(*handle).targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        } else {
            glTexParameteri(textures.at(*handle).targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }
    } else {
        if (textures.at(*handle).mipmapped == NEAREST || gl_options.mipmap == 0) {
            glTexParameteri(textures.at(*handle).targets, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(textures.at(*handle).targets, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(textures.at(*handle).targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(textures.at(*handle).targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
    }
    glTexParameterf(textures.at(*handle).targets, GL_TEXTURE_PRIORITY, .5);
    textures.at(*handle).width = width;
    textures.at(*handle).height = height;
    textures.at(*handle).iwidth = width;
    textures.at(*handle).iheight = height;
    textures.at(*handle).palette = NULL;
    if (palette && textureformat == PALETTE8) {
        VS_LOG(trace, " palette ");
        textures.at(*handle).palette = (GLubyte *) malloc(sizeof(GLubyte) * 1024);
        ConvertPalette(textures.at(*handle).palette, (unsigned char *) palette);
    }
    textures.at(*handle).textureformat = GetUncompressedTextureFormat(textureformat);
    return GFXTRUE;
}

void /*GFXDRVAPI*/ GFXPrioritizeTexture(unsigned int handle, float priority) {
    glPrioritizeTextures(1,
#if defined (__APPLE__)
            (GLuint*)
#endif
            &handle, &priority);
}

void /*GFXDRVAPI*/ GFXAttachPalette(unsigned char *palette, int handle) {
    ConvertPalette(textures.at(handle).palette, palette);
    //memcpy (textures.at(handle).palette,palette,768);
}

static void DownSampleTexture(unsigned char **newbuf,
        const unsigned char *oldbuf,
        int &height,
        int &width,
        int pixsize,
        int handle,
        int maxheight,
        int maxwidth,
        float newfade) {
    assert(pixsize <= 4);

    int i, j, k, l, m, n, o;
    if (MAX_TEXTURE_SIZE < maxwidth) {
        maxwidth = MAX_TEXTURE_SIZE;
    }
    if (MAX_TEXTURE_SIZE < maxheight) {
        maxheight = MAX_TEXTURE_SIZE;
    }
    int newwidth = width > maxwidth ? maxwidth : width;
    int scalewidth = width / newwidth;
    int newheight = height > maxheight ? maxheight : height;
    int scaleheight = height / newheight;
    int inewfade = (int) (newfade * 0x100);
    //Proposed downsampling code -- end
    if ((scalewidth != 2) || (scaleheight != 2) || (inewfade != 0x100)) {
        //Generic, area average downsampling (optimized)
        //Principle: The main optimizations/features
        //a) integer arithmetic, with propper scaling for propper saturation
        //b) unrolled loops (more parallelism, if the optimizer supports it)
        //c) improved locality due to 32-pixel chunking
        int wmask = scalewidth - 1;
        int hmask = scaleheight - 1;
        int tshift = 0;
        int ostride = newwidth * pixsize;
        int istride = width * pixsize;
        int rowstride = scaleheight * istride;
        int chunkstride = 32 * pixsize;
        int ichunkstride = scalewidth * chunkstride;
        int wshift = 0;
        int hshift = 0;
        int amask = wmask;
        while (amask) {
            amask >>= 1, tshift++, wshift++;
        }
        amask = hmask;
        while (amask) {
            amask >>= 1, tshift++, hshift++;
        }
        int tmask = (1 << tshift) - 1;
        *newbuf = static_cast<unsigned char *>(malloc(static_cast<size_t>(newheight) * static_cast<size_t>(newwidth)
                * static_cast<size_t>(pixsize) * sizeof(unsigned char)));
        unsigned int temp[32 * 4];
        unsigned char *orow = (*newbuf);
        const unsigned char *irow = oldbuf;
        for (i = 0; i < newheight; i++, orow += ostride, irow += rowstride) {
            const unsigned char *crow = irow;
            unsigned char *orow2 = orow;
            for (j = 0; j < newwidth; j += 32, crow += ichunkstride, orow2 += chunkstride) {
                const unsigned char *crow2 = crow;
                for (k = 0; k < chunkstride; k++) {
                    temp[k] = 0;
                }
                for (m = 0; m < scaleheight; m++, crow2 += istride) {
                    for (k = n = l = 0; (k < chunkstride) && (j + l < newwidth); k += pixsize, l++) {
                        for (o = 0; o < scalewidth; o++) {
                            (temp[k + 0] += crow2[n++]),
                                    (pixsize > 1) && (temp[k + 1] += crow2[n++]),
                                    (pixsize > 2) && (temp[k + 2] += crow2[n++]),
                                    //Unrolled loop
                                    (pixsize > 3) && (temp[k + 3] += crow2[n++]);
                        }
                    }
                }
                for (k = l = 0; (k < chunkstride) && (j + l < newwidth); k += pixsize, l++) {
                    (orow2[k + 0] =
                            (unsigned char) ((((temp[k + 0] + tmask) >> tshift) * inewfade + 0x80 * (0x100 - inewfade))
                                    >> 8)),
                            (pixsize > 1)
                                    && (orow2[k + 1] = (unsigned char) (
                                            (((temp[k + 1] + tmask) >> tshift) * inewfade + 0x80 * (0x100 - inewfade))
                                                    >> 8)),
                            (pixsize > 2)
                                    && (orow2[k + 2] = (unsigned char) (
                                            (((temp[k + 2] + tmask) >> tshift) * inewfade + 0x80 * (0x100 - inewfade))
                                                    >> 8)),
                            //Unrolled loop
                            (pixsize > 3)
                                    && (orow2[k + 3] = (unsigned char) (
                                            (((temp[k + 3] + tmask) >> tshift) * inewfade + 0x80 * (0x100 - inewfade))
                                                    >> 8));
                }
            }
        }
    } else {
        //Specific purpose downsampler: 2x2 averaging
        //a) Very little overhead
        //b) Very common case (mipmap generation)
        *newbuf = static_cast<unsigned char *>(malloc(static_cast<size_t>(newheight) * static_cast<size_t>(newwidth)
                * static_cast<size_t>(pixsize) * sizeof(unsigned char)));
        unsigned char *orow = (*newbuf);
        int ostride = newwidth * pixsize;
        int istride = width * pixsize;
        const unsigned char *irow[2] = {oldbuf, oldbuf + istride};
        unsigned int temp[4] = {0, 0, 0, 0};
        for (i = 0; i < newheight; i++, irow[0] += 2 * istride, irow[1] += 2 * istride, orow += ostride) {
            for (j = k = 0; j < newwidth; j++, k += pixsize) {
                (temp[0] = irow[0][(k << 1) + 0]),
                        (pixsize > 1) && (temp[1] = irow[0][(k << 1) + 1]),
                        (pixsize > 2) && (temp[2] = irow[0][(k << 1) + 2]),
                        //Unrolled loop
                        (pixsize > 3) && (temp[3] = irow[0][(k << 1) + 3]);

                (temp[0] += irow[0][(k << 1) + pixsize + 0]),
                        (pixsize > 1) && (temp[1] += irow[0][(k << 1) + pixsize + 1]),
                        (pixsize > 2) && (temp[2] += irow[0][(k << 1) + pixsize + 2]),
                        //Unrolled loop
                        (pixsize > 3) && (temp[3] += irow[0][(k << 1) + pixsize + 3]);

                (temp[0] += irow[1][(k << 1) + 0]),
                        (pixsize > 1) && (temp[1] += irow[1][(k << 1) + 1]),
                        (pixsize > 2) && (temp[2] += irow[1][(k << 1) + 2]),
                        //Unrolled loop
                        (pixsize > 3) && (temp[3] += irow[1][(k << 1) + 3]);

                (temp[0] += irow[1][(k << 1) + pixsize + 0]),
                        (pixsize > 1) && (temp[1] += irow[1][(k << 1) + pixsize + 1]),
                        (pixsize > 2) && (temp[2] += irow[1][(k << 1) + pixsize + 2]),
                        //Unrolled loop
                        (pixsize > 3) && (temp[3] += irow[1][(k << 1) + pixsize + 3]);

                (orow[k + 0] = (unsigned char) ((temp[0] + 3) >> 2)),
                        (pixsize > 1) && (orow[k + 1] = (unsigned char) ((temp[1] + 3) >> 2)),
                        (pixsize > 2) && (orow[k + 2] = (unsigned char) ((temp[2] + 3) >> 2)),
                        //Unrolled loop
                        (pixsize > 3) && (orow[k + 3] = (unsigned char) ((temp[3] + 3) >> 2));
            }
        }
    }
    width = newwidth;
    height = newheight;
}

static GLenum RGBCompressed(GLenum internalformat) {
    if (gl_options.compression) {
        internalformat = GL_COMPRESSED_RGB_ARB;
        if (configuration()->graphics.s3tc) {
            internalformat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        }
    }
    return internalformat;
}

static GLenum RGBACompressed(GLenum internalformat) {
    if (gl_options.compression) {
        internalformat = GL_COMPRESSED_RGBA_ARB;
        if (configuration()->graphics.s3tc) {
            switch (gl_options.compression) {
                case 3:
                    internalformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                    break;
                case 2:
                    internalformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                    break;
                case 1:
                    internalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                    break;
            }
        }
    }
    return internalformat;
}

GLenum GetTextureFormat(TEXTUREFORMAT textureformat) {
    switch (textureformat) {
        case PNGRGB24:
        case RGB32:
            return RGBCompressed(GL_RGB);

        case PNGRGBA32:
        case RGBA32:
            return RGBACompressed(GL_RGBA);

        case RGBA16:
            return RGBACompressed(GL_RGBA16);

        case RGB16:
            return RGBCompressed(GL_RGB16);

        case DXT1:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

        case DXT1RGBA:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

        case DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

        case DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

        default:
        case DUMMY:
        case RGB24:
            return RGBCompressed(GL_RGB);
    }
}

GLenum GetImageTarget(TEXTURE_IMAGE_TARGET imagetarget) {
    GLenum image2D = GL_TEXTURE_2D;
    switch (imagetarget) {
        case TEXTURE_2D:
            image2D = GL_TEXTURE_2D;
            break;
        case CUBEMAP_POSITIVE_X:
            image2D = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT;
            break;
        case CUBEMAP_NEGATIVE_X:
            image2D = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT;
            break;
        case CUBEMAP_POSITIVE_Y:
            image2D = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT;
            break;
        case CUBEMAP_NEGATIVE_Y:
            image2D = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT;
            break;
        case CUBEMAP_POSITIVE_Z:
            image2D = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT;
            break;
        case CUBEMAP_NEGATIVE_Z:
            image2D = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT;
            break;
        case TEXTURE_RECTANGLE:
            image2D = GL_TEXTURE_RECTANGLE_ARB;
            break;
        default:
            assert(0 || !"Bad case in file gl_texture.cpp, line 668 as of this writing.");
            break;
    }
    return image2D;
}

const char *GetImageTargetName(TEXTURE_IMAGE_TARGET imagetarget) {
    switch (imagetarget) {
        case TEXTURE_1D:
            return "TEXTURE_1D";

        case TEXTURE_2D:
            return "TEXTURE_2D";

        case TEXTURE_3D:
            return "TEXTURE_3D";

        case CUBEMAP_POSITIVE_X:
            return "CUBEMAP_POSITIVE_X";

        case CUBEMAP_NEGATIVE_X:
            return "CUBEMAP_NEGATIVE_X";

        case CUBEMAP_POSITIVE_Y:
            return "CUBEMAP_POSITIVE_Y";

        case CUBEMAP_NEGATIVE_Y:
            return "CUBEMAP_NEGATIVE_Y";

        case CUBEMAP_POSITIVE_Z:
            return "CUBEMAP_POSITIVE_Z";

        case CUBEMAP_NEGATIVE_Z:
            return "CUBEMAP_NEGATIVE_Z";

        case TEXTURE_RECTANGLE:
            return "TEXTURE_RECTANGLE";

        default:
            return "UNK";
    }
}

GFXBOOL /*GFXDRVAPI*/ GFXTransferSubTexture(unsigned char *buffer,
        int handle,
        int x,
        int y,
        unsigned int width,
        unsigned int height,
        enum TEXTURE_IMAGE_TARGET imagetarget) {
    GLenum image2D = GetImageTarget(imagetarget);
    glBindTexture(textures.at(handle).targets, textures.at(handle).name);

//internalformat = GetTextureFormat (handle);

    glTexSubImage2D(image2D, 0, x, y, width, height, textures.at(handle).textureformat, GL_UNSIGNED_BYTE, buffer);
    return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXTransferTexture(unsigned char *buffer,
        int handle,
        int inWidth,
        int inHeight,
        TEXTUREFORMAT internformat,
        enum TEXTURE_IMAGE_TARGET imagetarget,
        int maxdimension,
        GFXBOOL detail_texture,
        unsigned int pageIndex) {
    if (handle < 0) {
        return GFXFALSE;
    }
    int error = 0;

    int logsize = 1;
    int logwid = 1;
    unsigned char *data = NULL;
    unsigned char *tempbuf = NULL;
    GLenum internalformat;
    GLenum image2D = GetImageTarget(imagetarget);
    glBindTexture(textures.at(handle).targets, textures.at(handle).name);
    int blocksize = 16;
    bool comptemp = gl_options.compression;

    //Read in the number of mipmaps from buffer
    int offset1 = 2;
    int offset2;
    int mips = 0;
    if (internformat >= DXT1 && internformat <= DXT5) {
        mips = 0;
        if (buffer[0]) {
            mips = mips * 10 + (buffer[0] - '0');
        }
        if (buffer[1]) {
            mips = mips * 10 + (buffer[1] - '0');
        }
    }
    if (inWidth > 0) {
        textures.at(handle).iwidth = textures.at(handle).width = inWidth;
    }
    if (inHeight > 0) {
        textures.at(handle).iheight = textures.at(handle).height = inHeight;
    }
    //This code i believe is executed if our texture isn't power of two
    if ((textures.at(handle).mipmapped & (TRILINEAR | MIPMAP))
            && (!isPowerOfTwo(textures.at(handle).width, logwid) || !isPowerOfTwo(textures.at(handle).height, logsize))) {
        static unsigned char NONPOWEROFTWO[1024] = {
                255, 127, 127, 255,
                255, 255, 0, 255,
                255, 255, 0, 255,
                255, 127, 127, 255
        };
        buffer = NONPOWEROFTWO;
        textures.at(handle).width = 2;
        textures.at(handle).height = 2;
        //assert (false);
    }
    logsize = logsize > logwid ? logsize : logwid;
    //By default, if we have no limit set, aux_texture sends us a high number
    //for the max dimension, so that we know to grep the GL max number.
    //Otherwise maxdimension is set by some user argument based on quality settings.
    if (maxdimension == 65536) {
        maxdimension = configuration()->graphics.max_texture_dimension;
    }
    VS_LOG(debug,
            (boost::format(
                    "Transferring %1%x%2% texture, page %3% (eff: %4%x%5% - limited at %6% - %7% mips), onto name %8% (%9%)")
                    % textures.at(handle).iwidth
                    % textures.at(handle).iheight
                    % pageIndex
                    % textures.at(handle).width
                    % textures.at(handle).height
                    % maxdimension
                    % mips
                    % textures.at(handle).name
                    % GetImageTargetName(imagetarget)));
    if (maxdimension == 44) {
        detail_texture = 0;
        maxdimension = 256;
        if (internformat == DXT1 || internformat == DXT1RGBA) {
            blocksize = 8;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (internformat >= DXT1 && internformat <= DXT5) {
            if (textures.at(handle).width > 8 && textures.at(handle).height > 8 && mips > 0) {
                offset1 += ((textures.at(handle).width + 3) / 4) * ((textures.at(handle).height + 3) / 4) * blocksize;
                if (textures.at(handle).width > 1) {
                    textures.at(handle).width >>= 1;
                }
                if (textures.at(handle).height > 1) {
                    textures.at(handle).height >>= 1;
                }
                if (textures.at(handle).iwidth > 1) {
                    textures.at(handle).iwidth >>= 1;
                }
                if (textures.at(handle).iheight > 1) {
                    textures.at(handle).iheight >>= 1;
                }
                --mips;
            }
        }
    }
    //If we are DDS, we can scale to max dimension by choosing a pre-made mipmap.
    if (internformat == DXT1 || internformat == DXT1RGBA) {
        blocksize = 8;
    }
    if (internformat >= DXT1 && internformat <= DXT5) {
        while ((textures.at(handle).width > maxdimension || textures.at(handle).height > maxdimension) && mips > 0) {
            offset1 += ((textures.at(handle).width + 3) / 4) * ((textures.at(handle).height + 3) / 4) * blocksize;
            if (textures.at(handle).width > 1) {
                textures.at(handle).width >>= 1;
            }
            if (textures.at(handle).height > 1) {
                textures.at(handle).height >>= 1;
            }
            if (textures.at(handle).iwidth > 1) {
                textures.at(handle).iwidth >>= 1;
            }
            if (textures.at(handle).iheight > 1) {
                textures.at(handle).iheight >>= 1;
            }
            --mips;
        }
        offset2 = offset1;
        int w = textures.at(handle).width;
        int h = textures.at(handle).height;
        for (int i = 0; i < mips; ++i) {
            offset2 += ((w + 3) / 4) * ((h + 3) / 4) * blocksize;
            if (w > 1) {
                w >>= 1;
            }
            if (h > 1) {
                h >>= 1;
            }
        }
    } else {
        //If we're not DDS, we have to generate a scaled version of the image
        if (textures.at(handle).iwidth > maxdimension || textures.at(handle).iheight > maxdimension || textures.at(handle).iwidth
                > MAX_TEXTURE_SIZE || textures.at(handle).iheight > MAX_TEXTURE_SIZE) {
#if !defined (GL_COLOR_INDEX8_EXT)
            if (internformat != PALETTE8) {
#else
            if (internformat != PALETTE8 || gl_options.PaletteExt) {
#endif
                textures.at(handle).height = textures.at(handle).iheight;
                textures.at(handle).width = textures.at(handle).iwidth;
                DownSampleTexture(&tempbuf,
                        buffer,
                        textures.at(handle).height,
                        textures.at(handle).width,
                        (internformat == PALETTE8 ? 1 : (internformat == RGBA32 ? 4 : 3))
                                * sizeof(unsigned char),
                        handle,
                        maxdimension,
                        maxdimension,
                        1);
                buffer = tempbuf;
                VS_LOG(debug,
                        (boost::format("Downsampled %1%x%2% texture (target: %3%x%4% - limited at %5%)")
                                % textures.at(handle).iwidth
                                % textures.at(handle).iheight
                                % textures.at(handle).width
                                % textures.at(handle).height
                                % maxdimension));
            }
            offset2 = 2;
        } else {
            offset2 = offset1;
            int w = textures.at(handle).width;
            int h = textures.at(handle).height;
            switch (internformat) {
                case PALETTE8:
                    offset2 += (w * h);
                    break;
                case RGB16:
                case RGBA16:
                    offset2 += (w * h) * 2;
                    break;
                case RGB24:
                    offset2 += (w * h) * 3;
                    break;
                case RGBA32:
                case RGB32:
                    offset2 += (w * h) * 4;
                    break;
                case DXT1:
                case DXT1RGBA:
                    offset2 += (((w + 3) / 4) * ((h + 3) / 4)) * 8;
                    break;
                case DXT3:
                case DXT5:
                    offset2 += (((w + 3) / 4) * ((h + 3) / 4)) * 16;
                    break;
                case PNGPALETTE8:
                    offset2 += (w * h);
                    break;
                case PNGRGB24:
                    offset2 += (w * h) * 3;
                    break;
                case PNGRGBA32:
                    offset2 += (w * h) * 4;
                    break;
                default:
                    offset2 = 2;
            }
        }
    }
    //skip to desired page
    offset1 += pageIndex * (offset2 - 2);

    int height = textures.at(handle).height;
    int width = textures.at(handle).width;
    //If s3tc compression is disabled, our DDS files must be software decompressed
    if (internformat >= DXT1 && internformat <= DXT5 && !configuration()->graphics.s3tc) {
        unsigned char *tmpbuffer = buffer + offset1;
        ddsDecompress(tmpbuffer, data, internformat, textures.at(handle).height, textures.at(handle).width);
        buffer = data;
        internformat = RGBA32;
        textures.at(handle).textureformat = GL_RGBA;
    }
    if (internformat != PALETTE8 && internformat != PNGPALETTE8) {
        internalformat = GetTextureFormat(internformat);
        if (((textures.at(handle).mipmapped & (TRILINEAR | MIPMAP)) && gl_options.mipmap >= 2) || detail_texture) {
            if (detail_texture) {
                textures.at(handle).mipmapped = configuration()->graphics.detail_texture_filter;
                glTexParameteri(textures.at(handle).targets, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                if (textures.at(handle).mipmapped & TRILINEAR) {
                    glTexParameteri(textures.at(handle).targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                } else {
                    glTexParameteri(textures.at(handle).targets, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                }
            }
            //If we are DDS and we need to generate mipmaps (almost everything gets sent here, even non-3d visuals)
            if (internformat >= DXT1 && internformat <= DXT5) {
                int size = 0;
                int i = 0;
                unsigned int offset = 0;
                //The following takes into account C/C++'s catenation of floats to int
                //by adding 3, we ensure that when width or height is 1, we get a 1 rather than 0
                //from the division by 4. Because of catenation, all other numbers will result with
                //the expected number as if the +3 wasn't there. same as max(1,width/4)
                size = ((width + 3) / 4) * ((height + 3) / 4) * blocksize;
                for (i = 0; i < mips; ++i) {
                    glCompressedTexImage2D_p(image2D,
                            i,
                            internalformat,
                            width,
                            height,
                            0,
                            size,
                            buffer + offset1 + offset);
                    //We halve width and height until they reach 1, or i == mips
                    if (width != 1) {
                        width >>= 1;
                    }
                    if (height != 1) {
                        height >>= 1;
                    }
                    if (i < mips - 1) {
                        offset += size;
                    }
                    size = ((width + 3) / 4) * ((height + 3) / 4) * blocksize;
                }
                /* HACK */
                //This is a workaround for ani_texture which hates not having
                //mipmaps.
                if (mips == 0) {
                    size = ((width + 3) / 4) * ((height + 3) / 4) * blocksize;
                    //We need to reverse some parameters that are set cuz
                    //we're supposed to have mipmaps here.  But ani_texture hates us.
                    glTexParameteri(textures.at(handle).targets, GL_TEXTURE_BASE_LEVEL, 0);
                    glTexParameteri(textures.at(handle).targets, GL_TEXTURE_MAX_LEVEL, 0);
                    glCompressedTexImage2D_p(image2D, 0, internalformat, width, height, 0, size, buffer + offset1);
                }
                /* END HACK */
            } else {
                //We want mipmaps but we have uncompressed data
                gluBuild2DMipmaps(image2D,
                        internalformat,
                        textures.at(handle).width,
                        textures.at(handle).height,
                        textures.at(handle).textureformat,
                        GL_UNSIGNED_BYTE,
                        buffer);
            }
            if (tempbuf != nullptr) {
                free(tempbuf);
                tempbuf = nullptr;
            }
        } else {
            //WE HAVE NO MIPMAPS HERE
            if (internformat >= DXT1 && internformat <= DXT5) {
                int size = 0;
                size = ((width + 3) / 4) * ((height + 3) / 4) * blocksize;
                //force GL to only display our one texture (just in case)
                glTexParameteri(textures.at(handle).targets, GL_TEXTURE_BASE_LEVEL, 0);
                glTexParameteri(textures.at(handle).targets, GL_TEXTURE_MAX_LEVEL, 0);
                glCompressedTexImage2D_p(image2D, 0, internalformat, width, height, 0, size, buffer + offset1);
            } else {
                glTexImage2D(image2D,
                        0,
                        internalformat,
                        textures.at(handle).width,
                        textures.at(handle).height,
                        0,
                        textures.at(handle).textureformat,
                        GL_UNSIGNED_BYTE,
                        buffer);
            }
        }
    } else {
        //THIS IS 8bpp LAND
        internalformat = GetTextureFormat(internformat);
        //IRIX has no GL_COLOR_INDEX8 extension
#if defined (GL_COLOR_INDEX8_EXT)
        if (gl_options.PaletteExt) {
            error = glGetError();
            glColorTable_p(textures.at(handle).targets, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_BYTE, textures.at(handle).palette);
            error = glGetError();
            if (error) {
                if (tempbuf != nullptr) {
                    free(tempbuf);
                    tempbuf = nullptr;
                }
                gl_options.compression = comptemp;
                if (data != nullptr) {
                    free(data);
                    data = nullptr;
                }
                return GFXFALSE;
            }
            if ((textures.at(handle).mipmapped & (MIPMAP | TRILINEAR)) && gl_options.mipmap >= 2) {
                gluBuild2DMipmaps(image2D,
                        GL_COLOR_INDEX8_EXT,
                        textures.at(handle).width,
                        textures.at(handle).height,
                        GL_COLOR_INDEX,
                        GL_UNSIGNED_BYTE,
                        buffer);
            } else {
                glTexImage2D(image2D,
                        0,
                        GL_COLOR_INDEX8_EXT,
                        textures.at(handle).width,
                        textures.at(handle).height,
                        0,
                        GL_COLOR_INDEX,
                        GL_UNSIGNED_BYTE,
                        buffer);
            }
        } else
#endif
        {
            int nsize = 4 * textures.at(handle).iheight * textures.at(handle).iwidth;
            unsigned char *tbuf = (unsigned char *) malloc(sizeof(unsigned char) * nsize);
            //textures.at(handle).texture = tbuf;
            int j = 0;
            for (int i = 0; i < nsize; i += 4) {
                tbuf[i] = textures.at(handle).palette[4 * buffer[j]];
                tbuf[i + 1] = textures.at(handle).palette[4 * buffer[j] + 1];
                tbuf[i + 2] = textures.at(handle).palette[4 * buffer[j] + 2];
                //used to be 255
                tbuf[i + 3] = textures.at(handle).palette[4 * buffer[j] + 3];
                j++;
            }
            GFXTransferTexture(
                    tbuf, handle,
                    textures.at(handle).iwidth, textures.at(handle).iheight,
                    RGBA32, imagetarget, maxdimension, detail_texture);
            free(tbuf);
        }
    }
    if (tempbuf != nullptr) {
        free(tempbuf);
        tempbuf = nullptr;
    }
    gl_options.compression = comptemp;
    if (data != nullptr) {
        free(data);
        data = nullptr;
    }
    return GFXTRUE;
}

void /*GFXDRVAPI*/ GFXDeleteTexture(const size_t handle) {
    if (handle >= textures.size()) {
        VS_LOG(error, (boost::format("GFXDeleteTexture(const size_t handle) called with invalid handle value %1%, which is greater than textures.size(): %2%")
                % handle
                % textures.size()));
        return;
    }
    if (textures.at(handle).alive) {
        glDeleteTextures(1, &textures.at(handle).name);
        for (int & texture : activetexture) {
            if (texture == handle) {
                texture = -1;
            }
        }
    }
    if (textures.at(handle).palette != nullptr) {
        free(textures.at(handle).palette);
        textures.at(handle).palette = nullptr;
    }
    textures.at(handle).alive = GFXFALSE;
}

void GFXInitTextureManager() {
    for (auto & texture : textures) {
        texture.palette = nullptr;
        texture.width = texture.height = texture.iwidth = texture.iheight = 0;
        texture.texturestage = 0;
        texture.name = 0;
        texture.alive = 0;
        texture.textureformat = DUMMY;
        texture.targets = 0;
        texture.mipmapped = NEAREST;
    }
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MAX_TEXTURE_SIZE);
}

void GFXDestroyAllTextures() {
    // TODO: There's got to be a more efficient way to do this -- SGT 2024-04-18
    for (size_t handle = 0; handle < textures.size(); handle++) {
        GFXDeleteTexture(handle);
    }
}

void GFXTextureCoordGenMode(int stage, GFXTEXTURECOORDMODE tex, const float params[4], const float paramt[4]) {
    if (stage && stage >= static_cast<int>(gl_options.Multitexture)) {
        return;
    }
    GFXActiveTexture(stage);
    switch (tex) {
        case NO_GEN:
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
            break;
        case EYE_LINEAR_GEN:
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGenfv(GL_S, GL_EYE_PLANE, params);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGenfv(GL_T, GL_EYE_PLANE, paramt);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
            break;
        case OBJECT_LINEAR_GEN:
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGenfv(GL_S, GL_OBJECT_PLANE, params);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGenfv(GL_T, GL_OBJECT_PLANE, paramt);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
            break;
        case SPHERE_MAP_GEN:
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
            break;
        case CUBE_MAP_GEN:
#ifdef NV_CUBE_MAP
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glEnable(GL_TEXTURE_GEN_R);
#else
            assert( 0 );
#endif
            break;
    }
}

void /*GFXDRVAPI*/ GFXSelectTexture(int handle, int stage) {
    if (stage && stage >= static_cast<int>(gl_options.Multitexture)) {
        return;
    }
    if (activetexture[stage] != handle) {
        GFXActiveTexture(stage);
        activetexture[stage] = handle;
        if (gl_options.Multitexture || (stage == 0)) {
            glBindTexture(textures.at(handle).targets, textures.at(handle).name);
        }
    }
}

void GFXTextureEnv(int stage, GFXTEXTUREENVMODES mode, float arg2) {
    if (stage && stage >= static_cast<int>(gl_options.Multitexture)) {
        return;
    }
    GLenum type;
    GFXActiveTexture(stage);
    switch (mode) {
        case GFXREPLACETEXTURE:
            type = GL_REPLACE;
            goto ENVMODE;
        case GFXADDTEXTURE:
            type = GL_ADD;
            goto ENVMODE;
        case GFXMODULATETEXTURE:
            type = GL_MODULATE;
        ENVMODE:
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, type);
            break;
        case GFXINTERPOLATETEXTURE:
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PREVIOUS_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, GL_CONSTANT);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, GL_SRC_ALPHA);
            {
                GLfloat arg2v[4] = {
                        0, 0, 0, 1.0f - arg2
                };
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, arg2v);
            }
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_INTERPOLATE_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
            glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
            break;
        case GFXCOMPOSITETEXTURE:
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_CONSTANT);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_ONE_MINUS_SRC_ALPHA);
            {
                GLfloat arg2v[4] = {
                        0, 0, 0, arg2
                };
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, arg2v);
            }
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
            glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
            glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
            break;
        case GFXADDSIGNEDTEXTURE:
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PREVIOUS_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD_SIGNED_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
            glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
            glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
            break;
        case GFXDETAILTEXTURE:
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PREVIOUS_ARB);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);
            glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 2);
            break;
    }
}

#ifndef GL_CLAMP_TO_EDGE_EXT
#define GL_CLAMP_TO_EDGE_EXT 0x812F
#endif
#ifndef GL_CLAMP_TO_BORDER_ARB
#define GL_CLAMP_TO_BORDER_ARB 0x812D
#endif

void GFXTextureWrap(int stage, GFXTEXTUREWRAPMODES mode, enum TEXTURE_TARGET target) {
    if (stage && stage >= static_cast<int>(gl_options.Multitexture)) {
        return;
    }
    GFXActiveTexture(stage);
    GLenum tt = GetGLTextureTarget(target);
    GLenum e1 = GL_REPEAT;
    GLenum e2 = 0;
    switch (mode) {
        case GFXCLAMPTEXTURE:
            e1 = GL_CLAMP;
            e2 = GL_CLAMP_TO_EDGE_EXT;
            break;
        case GFXREPEATTEXTURE:
            e1 = GL_REPEAT;
            e2 = 0;
            break;
        case GFXBORDERTEXTURE:
            e1 = GL_CLAMP;
            e2 = GL_CLAMP_TO_BORDER_ARB;
            break;
    }
    glTexParameteri(tt, GL_TEXTURE_WRAP_S, e1);
    if (target != TEXTURE1D) {
        glTexParameteri(tt, GL_TEXTURE_WRAP_T, e1);
    }
    if (target == TEXTURE3D) {
        glTexParameteri(tt, GL_TEXTURE_WRAP_R, e1);
    }
    if (e2) {
        glTexParameteri(tt, GL_TEXTURE_WRAP_S, e2);
        if (target != TEXTURE1D) {
            glTexParameteri(tt, GL_TEXTURE_WRAP_T, e2);
        }
        if (target == TEXTURE3D) {
            glTexParameteri(tt, GL_TEXTURE_WRAP_R, e2);
        }
    }
}
