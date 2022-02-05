/**
 * aux_texture.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, Roy Falk,
 * and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#include <stdio.h>
#include <assert.h>
#include "gfxlib.h"
#include <string>
#include "endianness.h"
#include "hashtable.h"
#include "vsfilesystem.h"
#include "vsimage.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "main_loop.h"
#include "aux_texture.h"
#include "configxml.h"

using std::string;
using namespace VSFileSystem;
using namespace XMLSupport;
///holds all the textures in a huge hash table
Hashtable<string, Texture, 4007> texHashTable;
Hashtable<string, bool, 4007> badtexHashTable;

Texture *Texture::Exists(string s, string a)
{
    return Texture::Exists(s + a);
}

Texture *Texture::Exists(string s)
{
    Texture *tmp = texHashTable.Get(VSFileSystem::GetHashName(s));
    if (tmp == NULL) {
        string tmpo;
        tmp = texHashTable.Get(VSFileSystem::GetSharedTextureHashName(s));
    }
    if (tmp) {
        return tmp->Original();
    }
    return tmp;
}

bool Texture::operator<(const Texture &b) const
{
    return Original() < b.Original();
}

bool Texture::operator==(const Texture &b) const
{
    return Original() == b.Original();
}

void Texture::setReference(Texture *other)
{
    original = other;
    original->refcount++;

    //Copy shared attributes
    texfilename = other->texfilename;
    data = other->data;
    name = other->name;
    bound = other->bound;
    boundSizeX = other->boundSizeX;
    boundSizeY = other->boundSizeY;
    boundMode = other->boundMode;
    texture_target = other->texture_target;
    image_target = other->image_target;
}

GFXBOOL Texture::checkold(const string &s, bool shared, string &hashname)
{
    hashname = shared ? VSFileSystem::GetSharedTextureHashName(s) : VSFileSystem::GetHashName(s);
    Texture *oldtex = texHashTable.Get(hashname);
    if (oldtex != NULL) {
        //*this = *oldtex;//will be obsoleted--unpredictable results with string()
        setReference(oldtex);
        return GFXTRUE;
    } else {
        return GFXFALSE;
    }
}

void Texture::modold(const string &s, bool shared, string &hashname)
{
    hashname = shared ? VSFileSystem::GetSharedTextureHashName(s) : VSFileSystem::GetHashName(s);
    Texture *oldtex = new Texture;
    //oldtex->InitTexture();new calls this
    oldtex->name = -1;
    oldtex->refcount = 1;
    oldtex->original = NULL;
    oldtex->palette = NULL;
    oldtex->data = NULL;
    texHashTable.Put(hashname, oldtex);
    original = oldtex;
}

void Texture::InitTexture()
{
    bound = false;
    original = 0;
    refcount = 0;
    name = -1;
    palette = NULL;
    data = NULL;
    mintcoord = Vector(0.0f, 0.0f, 0.0f);
    maxtcoord = Vector(1.0f, 1.0f, 1.0f);
    address_mode = DEFAULT_ADDRESS_MODE;
}

void Texture::setold()
{
    //*original = *this;//will be obsoleted in new C++ standard unpredictable results when using string() (and its strangeass copy constructor)
    *original = *this;
    //memcpy (original, this, sizeof (Texture));
    original->original = NULL;
    original->refcount++;
}

const Texture *Texture::Original() const
{
    if (original) {
        return original->Original();
    } else {
        return this;
    }
}

Texture *Texture::Original()
{
    if (original) {
        return original->Original();
    } else {
        return this;
    }
}

Texture *Texture::Clone()
{
    Texture *retval = new Texture();
    Texture *target = Original();
    *retval = *target;
    //memcpy (this, target, sizeof (Texture));
    if (retval->name != -1) {
        retval->original = target;
        retval->original->refcount++;
    } else {
        retval->original = NULL;
    }
    retval->refcount = 0;
    return retval;
    //assert (!original->original);
}

void Texture::FileNotFound(const string &texfilename)
{
    //We may need to remove from texHashTable if we found the file but it is a bad one
    texHashTable.Delete(texfilename);

    setbad(texfilename);
    name = -1;
    data = NULL;
    if (original != NULL) {
        original->name = -1;
        delete original;
        original = NULL;
    }
    palette = NULL;
}

bool Texture::checkbad(const string &s)
{
    string hashname = VSFileSystem::GetSharedTextureHashName(s);
    bool *found = NULL;
    found = badtexHashTable.Get(hashname);
    if (found != NULL) {
        return true;
    }
    hashname = VSFileSystem::GetHashName(s);
    found = badtexHashTable.Get(hashname);
    if (found != NULL) {
        return true;
    }
    return false;
}

void Texture::setbad(const string &s)
{
    //Put both current path+texfile and shared texfile since they both have been looked for
    static bool _TRUEVAL = true;
    if (VSFileSystem::current_path.back() != "") {
        badtexHashTable.Put(VSFileSystem::GetHashName(s), &_TRUEVAL);
    }
    badtexHashTable.Put(VSFileSystem::GetSharedTextureHashName(s), &_TRUEVAL);
}

Texture::Texture(int stage,
                 enum FILTER mipmap,
                 enum TEXTURE_TARGET target,
                 enum TEXTURE_IMAGE_TARGET imagetarget,
                 enum ADDRESSMODE address_mode)
{
    InitTexture();
    data = NULL;
    ismipmapped = mipmap;
    palette = NULL;
    texture_target = target;
    image_target = imagetarget;
    this->stage = stage;
}

Texture::Texture(VSFile *f,
                 int stage,
                 enum FILTER mipmap,
                 enum TEXTURE_TARGET target,
                 enum TEXTURE_IMAGE_TARGET imagetarget,
                 GFXBOOL force_load,
                 int maxdimension,
                 GFXBOOL detailtexture,
                 GFXBOOL nocache,
                 enum ADDRESSMODE address_mode,
                 Texture *main)
{
    data = NULL;
    ismipmapped = mipmap;
    InitTexture();
    palette = NULL;
    texture_target = target;
    image_target = imagetarget;
    this->stage = stage;
    data = this->ReadImage(f, NULL, true, NULL);
    if (data) {
        if (mode >= _DXT1 && mode <= _DXT5) {
            if ((int) data[0] == 0) {
                detailtexture = NEAREST;
                ismipmapped = NEAREST;
            }
        }
        if (main) {
            Bind(main, maxdimension, detailtexture);
        } else {
            Bind(maxdimension, detailtexture);
        }
        free(data);
        data = NULL;
        if (!nocache) {
            setold();
        }
    } else {
        FileNotFound(texfilename);
    }
}

Texture::Texture(const char *FileName,
                 int stage,
                 enum FILTER mipmap,
                 enum TEXTURE_TARGET target,
                 enum TEXTURE_IMAGE_TARGET imagetarget,
                 GFXBOOL force_load,
                 int maxdimension,
                 GFXBOOL detailtexture,
                 GFXBOOL nocache,
                 enum ADDRESSMODE address_mode,
                 Texture *main)
{
    InitTexture();
    Load(FileName,
         stage,
         mipmap,
         target,
         imagetarget,
         force_load,
         maxdimension,
         detailtexture,
         nocache,
         address_mode,
         main);
}

void Texture::Load(const char *FileName,
                   int stage,
                   enum FILTER mipmap,
                   enum TEXTURE_TARGET target,
                   enum TEXTURE_IMAGE_TARGET imagetarget,
                   GFXBOOL force_load,
                   int maxdimension,
                   GFXBOOL detailtexture,
                   GFXBOOL nocache,
                   enum ADDRESSMODE address_mode,
                   Texture *main)
{
    if (data != nullptr) {
        free(data);
        data = nullptr;
    }
    if (palette != nullptr) {
        free(palette);
        palette = nullptr;
    }
    ismipmapped = mipmap;
    texture_target = target;
    image_target = imagetarget;
    this->stage = stage;
    this->address_mode = address_mode;
    string texfn = string(FileName);
    if (checkbad(texfn)) {
        return;
    }
    if (!nocache) {
        string tempstr;
        if (checkold(texfn, false, tempstr)
                || checkold(texfn, true, tempstr)) {
            texfilename = tempstr;
            return;
        }
    }
    char *t = strdup(FileName);
    int tmp = strlen(FileName);
    if (tmp > 3) {
        t[tmp - 3] = 'a';
        t[tmp - 2] = 'l';
        t[tmp - 1] = 'p';
    }
    VSFile f2;
    VSError err2 = VSFileSystem::FileNotFound;
    if (t) {
        if (t[0] != '\0') {
            static bool use_alphamap = parse_bool(vs_config->getVariable("graphics",
                                                                         "bitmap_alphamap",
                                                                         "true"));
            if (use_alphamap) {
                err2 = f2.OpenReadOnly(t, TextureFile);
            }
        }
    }
    if (err2 <= Ok) {
        //texfilename += string(t);
    }
    //this->texfilename = texfilename;
    //strcpy (filename,texfilename.c_str());
    VSFile f;
    VSError err; //FIXME err not always initialized before use
    err = Ok; //FIXME this line added temporarily by chuck_starchaser
    if (FileName) {
        if (FileName[0]) {
            err = f.OpenReadOnly(FileName, TextureFile);
        }
    }
    bool shared = (err == Shared);
    free(t);
    if (err <= Ok && g_game.use_textures == 0 && !force_load) {
        f.Close();
        err = Unspecified;
    }
    if (err > Ok) { //FIXME err not guaranteed to have been initialized!
        FileNotFound(texfn);
        if (err2 <= Ok) {
            f2.Close();
        }
        return;
    }
    if (!nocache) {
        string tempstr;
        modold(texfn, shared, tempstr);
        texfilename = tempstr;
    }
    if (texfn.find("white") == string::npos) {
        bootstrap_draw("Loading " + string(FileName));
    }
    //strcpy(filename, FileName);
    if (err2 > Ok) {
        data = this->ReadImage(&f, NULL, true, NULL);
    } else {
        data = this->ReadImage(&f, NULL, true, &f2);
    }
    if (data) {
        if (mode >= _DXT1 && mode <= _DXT5) {
            if ((int) data[0] == 0) {
                detailtexture = NEAREST;
                ismipmapped = NEAREST;
            }
        }
        if (main) {
            Bind(main, maxdimension, detailtexture);
        } else {
            Bind(maxdimension, detailtexture);
        }
        free(data);
        data = NULL;
        if (!nocache) {
            setold();
        }
    } else {
        FileNotFound(texfilename);
    }
    f.Close();
    if (f2.Valid()) {
        f2.Close();
    }
}

Texture::Texture(const char *FileNameRGB,
                 const char *FileNameA,
                 int stage,
                 enum FILTER mipmap,
                 enum TEXTURE_TARGET target,
                 enum TEXTURE_IMAGE_TARGET imagetarget,
                 float alpha,
                 int zeroval,
                 GFXBOOL force_load,
                 int maxdimension,
                 GFXBOOL detailtexture,
                 GFXBOOL nocache,
                 enum ADDRESSMODE address_mode,
                 Texture *main)
{
    InitTexture();
    Load(FileNameRGB,
         FileNameA,
         stage,
         mipmap,
         target,
         imagetarget,
         alpha,
         zeroval,
         force_load,
         maxdimension,
         detailtexture,
         nocache,
         address_mode,
         main);
}

void Texture::Load(const char *FileNameRGB,
                   const char *FileNameA,
                   int stage,
                   enum FILTER mipmap,
                   enum TEXTURE_TARGET target,
                   enum TEXTURE_IMAGE_TARGET imagetarget,
                   float alpha,
                   int zeroval,
                   GFXBOOL force_load,
                   int maxdimension,
                   GFXBOOL detailtexture,
                   GFXBOOL nocache,
                   enum ADDRESSMODE address_mode,
                   Texture *main)
{
    if (data != nullptr) {
        free(data);
        data = nullptr;
    }
    if (palette != nullptr) {
        free(palette);
        palette = nullptr;
    }
    ismipmapped = mipmap;

    this->stage = stage;
    this->address_mode = address_mode;
    texture_target = target;
    image_target = imagetarget;
    string texfn = string(FileNameRGB) + string("|") + string(FileNameA);
    if (!nocache) {
        string tempstr;
        if (checkold(texfn, false, tempstr)
                || checkold(texfn, true, tempstr)) {
            texfilename = tempstr;
            return;
        }
    }
    //this->texfilename = texfilename;
    //strcpy (filename,texfilename.c_str());
    VSFile f;
    VSError err = Unspecified;
    err = f.OpenReadOnly(FileNameRGB, TextureFile);
    if (!nocache) {
        bool shared = (err == Shared);
        string tempstr;
        modold(texfn, shared, tempstr);
        texfilename = tempstr;
    }
    if (err <= Ok && g_game.use_textures == 0 && !force_load) {
        f.Close();
        err = Unspecified;
    }
    if (err > Ok) {
        FileNotFound(texfilename);
        return;
    }
    VSFile f1;
    VSError err1 = Unspecified;
    if (FileNameA) {
        static bool use_alphamap =
                parse_bool(vs_config->getVariable("graphics",
                                                  "bitmap_alphamap",
                                                  "true"));
        if (use_alphamap) {
            std::string tmp;
            err1 = f1.OpenReadOnly(FileNameA, TextureFile);

            if (err1 > Ok) {
                data = NULL;
                FileNameA = NULL;
            }
        } else {
            FileNameA = 0;
        }
    }
    if (err1 > Ok) {
        data = this->ReadImage(&f, NULL, true, NULL);
    } else {
        data = this->ReadImage(&f, NULL, true, &f1);
    }
    if (data) {
        if (mode >= _DXT1 && mode <= _DXT5) {
            if ((int) data[0] == 0) {
                detailtexture = NEAREST;
                ismipmapped = NEAREST;
            }
        }
        if (main) {
            Bind(main, maxdimension, detailtexture);
        } else {
            Bind(maxdimension, detailtexture);
        }
        free(data);
        data = NULL;
        if (!nocache) {
            setold();
        }
    } else {
        FileNotFound(texfilename);
    }
    f.Close();
    if (err1 <= Ok) {
        f1.Close();
    }
}

Texture::~Texture()
{
    if (original == NULL) {
        /**DEPRECATED
         *     if(data != NULL)
         *     {
         *             delete [] data;
         *
         *             data = NULL;
         *     }
         */
        UnBind();
        if (palette != NULL) {
            free(palette);
            palette = NULL;
        }
    } else {
        original->refcount--;
        if (original->refcount == 0) {
            delete original;
        }
    }
}

void Texture::UnBind()
{
    if (name != -1) {
        texHashTable.Delete(texfilename);
        GFXDeleteTexture(name);
        name = -1;
    }
    //glDeleteTextures(1, &name);
}

void Texture::Transfer(int maxdimension, GFXBOOL detailtexture)
{
    //Implement this in D3D
    //if(mode == _8BIT)
    //glColorTable(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, palette);

    TEXTUREFORMAT internformat;
    switch (mode) {
        //send DXT1,DXT3,DXT5 ...need to have the function call other glcompressedTexImage2D function
        case _DXT1:
            internformat = DXT1;
            break;
        case _DXT1RGBA:
            internformat = DXT1RGBA;
            break;
        case _DXT3:
            internformat = DXT3;
            break;
        case _DXT5:
            internformat = DXT5;
            break;
        case _24BITRGBA:
            internformat = RGBA32;
            break;
        case _24BIT:
            internformat = RGB24;
            break;
        case _8BIT:
            internformat = PALETTE8;
            break;
        default:
            return;
    }
    if (img_sides == SIDE_SINGLE) {
        GFXTransferTexture(data, name, sizeX, sizeY, internformat, image_target, maxdimension, detailtexture);
    } else {
        GFXTransferTexture(data, name, sizeX, sizeY, internformat, CUBEMAP_POSITIVE_X, maxdimension, detailtexture, 0);
        GFXTransferTexture(data, name, sizeX, sizeY, internformat, CUBEMAP_NEGATIVE_X, maxdimension, detailtexture, 1);
        GFXTransferTexture(data, name, sizeX, sizeY, internformat, CUBEMAP_POSITIVE_Y, maxdimension, detailtexture, 2);
        GFXTransferTexture(data, name, sizeX, sizeY, internformat, CUBEMAP_NEGATIVE_Y, maxdimension, detailtexture, 3);
        GFXTransferTexture(data, name, sizeX, sizeY, internformat, CUBEMAP_POSITIVE_Z, maxdimension, detailtexture, 4);
        GFXTransferTexture(data, name, sizeX, sizeY, internformat, CUBEMAP_NEGATIVE_Z, maxdimension, detailtexture, 5);
    }
}

int Texture::Bind(int maxdimension, GFXBOOL detailtexture)
{
    if (!bound || (boundSizeX != sizeX) || (boundSizeY != sizeY) || (boundMode != mode)) {
        UnBind();
        switch (mode) {
            case _DXT1:
                GFXCreateTexture(sizeX, sizeY, DXT1, &name, NULL, stage, ismipmapped, texture_target, address_mode);
                break;
            case _DXT1RGBA:
                GFXCreateTexture(sizeX, sizeY, DXT1RGBA, &name, NULL, stage, ismipmapped, texture_target, address_mode);
                break;

            case _DXT3:
                GFXCreateTexture(sizeX, sizeY, DXT3, &name, NULL, stage, ismipmapped, texture_target, address_mode);
                break;
            case _DXT5:
                GFXCreateTexture(sizeX, sizeY, DXT5, &name, NULL, stage, ismipmapped, texture_target, address_mode);
                break;

            case _24BITRGBA:
                //GFXCreateTexture(sizeX, sizeY, RGBA32, &name, NULL, stage);
                GFXCreateTexture(sizeX, sizeY, RGBA32, &name, NULL, stage, ismipmapped, texture_target, address_mode);
                break;
            case _24BIT:
                //not supported by most cards, so i use rgba32
                //GFXCreateTexture(sizeX, sizeY, RGB24, &name);
                GFXCreateTexture(sizeX, sizeY, RGB24, &name, NULL, stage, ismipmapped, texture_target, address_mode);
                break;
            case _8BIT:
                GFXCreateTexture(sizeX,
                                 sizeY,
                                 PALETTE8,
                                 &name,
                                 (char *) palette,
                                 stage,
                                 ismipmapped,
                                 texture_target,
                                 address_mode);
                break;
        }
    }
    boundSizeX = sizeX;
    boundSizeY = sizeY;
    boundMode = mode;
    bound = true;
    Transfer(maxdimension, detailtexture);
    return name;
}

int Texture::Bind(Texture *other, int maxdimension, GFXBOOL detailtexture)
{
    UnBind();

    boundSizeX = other->boundSizeX;
    boundSizeY = other->boundSizeY;
    boundMode = other->boundMode;
    bound = other->bound;
    name = other->name;
    Transfer(maxdimension, detailtexture);
    return name;
}

void Texture::Prioritize(float priority)
{
    GFXPrioritizeTexture(name, priority);
}

static void ActivateWhite(int stage)
{
    static Texture *white = new Texture("white.bmp", 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1);
    if (white->LoadSuccess()) {
        white->MakeActive(stage);
    }
}

void Texture::MakeActive(int stag, int pass)
{
    if ((name == -1) || (pass != 0)) {
        ActivateWhite(stag);
    } else {
        // Lazy-init default address mode
        // So that texture_target is properly initiated
        // by the time we do
        if (address_mode == DEFAULT_ADDRESS_MODE) {
            switch (texture_target) {
                case TEXTURE1D:
                case TEXTURE2D:
                case TEXTURE3D:
                default:
                    address_mode = WRAP;
                    break;
                case TEXTURERECT:
                case CUBEMAP:
                    address_mode = CLAMP;
                    break;
            }
        }

        GFXActiveTexture(stag);
        GFXSelectTexture(name, stag);
        GFXTextureAddressMode(address_mode, texture_target);         //In case it changed - it's possible
    }
}

