/*
 * aux_texture.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Roy Falk, and other Vega Strike contributors
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


#include <stdio.h>
#include <assert.h>
#include "gfxlib.h"
#include <string>
#include <utility>
#include "endianness.h"
#include "hashtable.h"
#include "shared_ptr_hashtable.h"
#include "vsfilesystem.h"
#include "vsimage.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "main_loop.h"
#include "aux_texture.h"
#include "configxml.h"
#include "vega_cast_utils.h"
#include "preferred_types.h"

using std::string;
using namespace VSFileSystem;
using namespace XMLSupport;
///holds all the textures in a huge hash table
SharedPtrHashtable<string, Texture, 4007> texHashTable;
SharedPtrHashtable<string, bool, 4007> badtexHashTable;

vega_types::SharedPtr<Texture> Texture::Exists(string s, string a) {
    return Texture::Exists(s + a);
}

vega_types::SharedPtr<Texture> Texture::Exists(string s) {
    vega_types::SharedPtr<Texture> tmp = texHashTable.Get(VSFileSystem::GetHashName(s));
    if (tmp == NULL) {
        string tmpo;
        tmp = texHashTable.Get(VSFileSystem::GetSharedTextureHashName(s));
    }
    if (tmp) {
        return tmp->Original();
    }
    return tmp;
}

bool Texture::operator<(const Texture &b) const {
    return OriginalConst() < b.OriginalConst();
}

bool Texture::operator==(const Texture &b) const {
    return OriginalConst() == b.OriginalConst();
}

void Texture::setReference(vega_types::SharedPtr<Texture> other) {
    original = other;

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

GFXBOOL Texture::checkold(const string &s, bool shared, string &hashname) {
    hashname = shared ? VSFileSystem::GetSharedTextureHashName(s) : VSFileSystem::GetHashName(s);
    vega_types::SharedPtr<Texture> oldtex = texHashTable.Get(hashname);
    if (oldtex) {
        //*this = *oldtex;//will be obsoleted--unpredictable results with string()
        setReference(oldtex);
        return GFXTRUE;
    } else {
        return GFXFALSE;
    }
}

void Texture::modold(const string &s, bool shared, string &hashname) {
    hashname = shared ? VSFileSystem::GetSharedTextureHashName(s) : VSFileSystem::GetHashName(s);
    vega_types::SharedPtr<Texture> oldtex = vega_types::MakeShared<Texture>();
    //oldtex->InitTexture();new calls this
    oldtex->name = -1;
    oldtex->original = nullptr;
    oldtex->palette = nullptr;
    oldtex->data = nullptr;
    texHashTable.Put(hashname, oldtex);
    original = oldtex;
}

//void Texture::InitTexture() {
//    bound = false;
//    original = 0;
//    name = -1;
//    palette = nullptr;
//    data = nullptr;
//    mintcoord = Vector(0.0f, 0.0f, 0.0f);
//    maxtcoord = Vector(1.0f, 1.0f, 1.0f);
//    address_mode = DEFAULT_ADDRESS_MODE;
//}

void Texture::setold() {
    //*original = *this;//will be obsoleted in new C++ standard unpredictable results when using string() (and its strangeass copy constructor)
    *original = *this;
    //memcpy (original, this, sizeof (Texture));
    original->original = nullptr;
}

const vega_types::SharedPtr<const Texture> Texture::OriginalConst() const {
    if (original) {
        return original->OriginalConst();
    } else {
        return vega_dynamic_const_cast_shared_ptr<Texture>(shared_from_this());
    }
}

vega_types::SharedPtr<Texture> Texture::Original() {
    if (original) {
        return original->Original();
    } else {
        return vega_dynamic_cast_shared_ptr<Texture>(shared_from_this());
    }
}

vega_types::SharedPtr<Texture> Texture::Clone() {
    vega_types::SharedPtr<Texture> retval = vega_types::MakeShared<Texture>();
    vega_types::SharedPtr<Texture> target = Original();
    *retval = *target;
    //memcpy (this, target, sizeof (Texture));
    if (retval->name != -1) {
        retval->original = target;
    } else {
        retval->original = nullptr;
    }
    return retval;
    //assert (!original->original);
}

void Texture::FileNotFound(const string &tex_filename) {
    VS_LOG(warning, (boost::format("Texture::FileNotFound called with tex_filename: '%1%'") % tex_filename));
    //We may need to remove from texHashTable if we found the file but it is a bad one
    texHashTable.Delete(tex_filename);

    setbad(tex_filename);
    name = -1;
    if (data) {
        free(data);
        data = nullptr;
    }
    if (original) {
        original->name = -1;
        original.reset();
    }
    palette = nullptr;
}

bool Texture::checkbad(const string &s) {
    string hashname = VSFileSystem::GetSharedTextureHashName(s);
    vega_types::SharedPtr<bool> found = nullptr;
    found = badtexHashTable.Get(hashname);
    if (found) {
        return true;
    }
    hashname = VSFileSystem::GetHashName(s);
    found = badtexHashTable.Get(hashname);
    if (found) {
        return true;
    }
    return false;
}

void Texture::setbad(const string &s) {
    //Put both current path+texfile and shared texfile since they both have been looked for
    static vega_types::SharedPtr<bool> TRUEVAL = vega_types::MakeShared<bool>(true);
    if (!VSFileSystem::current_path.back().empty()) {
        badtexHashTable.Put(VSFileSystem::GetHashName(s), TRUEVAL);
    }
    badtexHashTable.Put(VSFileSystem::GetSharedTextureHashName(s), TRUEVAL);
}

Texture::Texture(int stage,
        enum FILTER mipmap,
        enum TEXTURE_TARGET target,
        enum TEXTURE_IMAGE_TARGET imagetarget,
        enum ADDRESSMODE address_mode) {
//    InitTexture();
    data = nullptr;
    ismipmapped = mipmap;
    palette = nullptr;
    texture_target = target;
    image_target = imagetarget;
    this->stage = stage;
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
                   vega_types::SharedPtr<Texture> main_texture) {
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
        if (main_texture) {
            Bind(main_texture, maxdimension, detailtexture);
        } else {
            Bind(maxdimension, detailtexture);
        }
        free(data);
        data = nullptr;
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
                   vega_types::SharedPtr<Texture> main_texture) {
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
        if (main_texture) {
            Bind(main_texture, maxdimension, detailtexture);
        } else {
            Bind(maxdimension, detailtexture);
        }
        free(data);
        data = nullptr;
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

Texture::~Texture() {
    if (original) {
        original.reset();
    } else {
        /**DEPRECATED
         *     if(data != NULL)
         *     {
         *             delete [] data;
         *
         *             data = NULL;
         *     }
         */
        UnBind();
        if (palette != nullptr) {
            free(palette);
            palette = nullptr;
        }
    }
}

void Texture::UnBind() {
    if (name != -1) {
        texHashTable.Delete(texfilename);
        GFXDeleteTexture(name);
        name = -1;
    }
    //glDeleteTextures(1, &name);
}

void Texture::Transfer(int maxdimension, GFXBOOL detailtexture) {
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

int Texture::Bind(int maxdimension, GFXBOOL detailtexture) {
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

int Texture::Bind(vega_types::SharedPtr<Texture> other, int maxdimension, GFXBOOL detailtexture) {
    UnBind();

    boundSizeX = other->boundSizeX;
    boundSizeY = other->boundSizeY;
    boundMode = other->boundMode;
    bound = other->bound;
    name = other->name;
    Transfer(maxdimension, detailtexture);
    return name;
}

void Texture::Prioritize(float priority) {
    GFXPrioritizeTexture(name, priority);
}

static void ActivateWhite(int stage) {
    static vega_types::SharedPtr<Texture> white = Texture::createTexture("white.bmp", 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1);
    if (white->LoadSuccess()) {
        white->MakeActive(stage);
    }
}

void Texture::MakeActive(int stag, int pass) {
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

 vega_types::SharedPtr<Texture> Texture::constructTexture(vega_types::SharedPtr<Texture> texture,
                                                       VSFileSystem::VSFile *f,
                                                       int stage,
                                                       enum FILTER mipmap,
                                                       enum TEXTURE_TARGET target,
                                                       enum TEXTURE_IMAGE_TARGET imagetarget,
                                                       GFXBOOL force,
                                                       int max_dimension_size,
                                                       GFXBOOL detail_texture,
                                                       GFXBOOL nocache,
                                                       enum ADDRESSMODE address_mode,
                                                       vega_types::SharedPtr<Texture> main_texture) {
    texture->data = nullptr;
    texture->ismipmapped = mipmap;
    texture->palette = nullptr;
    texture->texture_target = target;
    texture->image_target = imagetarget;
    texture->stage = stage;
    texture->data = texture->ReadImage(f, nullptr, true, nullptr);
    if (texture->data) {
        if (texture->mode >= texture->_DXT1 && texture->mode <= texture->_DXT5) {
            if ((int) texture->data[0] == 0) {
                detail_texture = GFXFALSE;      //NEAREST;
                texture->ismipmapped = NEAREST;
            }
        }
        if (main_texture) {
            texture->Bind(main_texture, max_dimension_size, detail_texture);
        } else {
            texture->Bind(max_dimension_size, detail_texture);
        }
        free(texture->data);
        texture->data = nullptr;
        if (!nocache) {
            texture->setold();
        }
    } else {
        texture->FileNotFound(texture->texfilename);
    }
    return texture;
}

vega_types::SharedPtr<Texture>
Texture::constructTexture(vega_types::SharedPtr<Texture> texture, const char *FileName, int stage, enum FILTER mipmap,
                          enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, unsigned char force_load,
                          int max_dimension_size, unsigned char detail_texture, unsigned char nocache,
                          enum ADDRESSMODE address_mode, vega_types::SharedPtr<Texture> main_texture) {
    texture->Load(FileName,
         stage,
         mipmap,
         target,
         imagetarget,
         force_load,
         max_dimension_size,
         detail_texture,
         nocache,
         address_mode,
         main_texture);
    return texture;
}

vega_types::SharedPtr<Texture>
Texture::createTexture(VSFileSystem::VSFile *f, int stage, enum FILTER mipmap, enum TEXTURE_TARGET target,
                       enum TEXTURE_IMAGE_TARGET imagetarget, unsigned char force, int max_dimension_size,
                       unsigned char detail_texture, unsigned char nocache, enum ADDRESSMODE address_mode,
                       vega_types::SharedPtr<Texture> main_texture) {
    vega_types::SharedPtr<Texture> return_value = vega_types::MakeShared<Texture>();
    return constructTexture(return_value, f, stage, mipmap, target, imagetarget, force, max_dimension_size, detail_texture, nocache, address_mode, std::move(main_texture));
}

vega_types::SharedPtr<Texture>
Texture::createTexture(const char *FileNameRGB, const char *FileNameA, int stage, enum FILTER mipmap,
                       enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, float alpha, int zeroval,
                       unsigned char force_load, int max_dimension_size, unsigned char detail_texture,
                       unsigned char nocache, enum ADDRESSMODE address_mode,
                       vega_types::SharedPtr<Texture> main_texture) {
    vega_types::SharedPtr<Texture> return_value = vega_types::MakeShared<Texture>();
    return constructTexture(return_value, FileNameRGB, FileNameA, stage, mipmap, target, imagetarget, alpha, zeroval, force_load, max_dimension_size, detail_texture, nocache, address_mode, std::move(main_texture));
}

vega_types::SharedPtr<Texture>
Texture::createTexture(const char *FileName, int stage, enum FILTER mipmap, enum TEXTURE_TARGET target,
                       enum TEXTURE_IMAGE_TARGET imagetarget, unsigned char force, int max_dimension_size,
                       unsigned char detail_texture, unsigned char nocache, enum ADDRESSMODE address_mode,
                       vega_types::SharedPtr<Texture> main_texture) {
    vega_types::SharedPtr<Texture> return_value = vega_types::MakeShared<Texture>();
    return constructTexture(return_value, FileName, stage, mipmap, target, imagetarget, force, max_dimension_size, detail_texture, nocache, address_mode, std::move(main_texture));
}

vega_types::SharedPtr<Texture>
Texture::constructTexture(vega_types::SharedPtr<Texture> texture, const char *FileNameRGB, const char *FileNameA,
                          int stage, enum FILTER mipmap, enum TEXTURE_TARGET target,
                          enum TEXTURE_IMAGE_TARGET imagetarget, float alpha, int zeroval, unsigned char force_load,
                          int max_dimension_size, unsigned char detail_texture, unsigned char nocache,
                          enum ADDRESSMODE address_mode, vega_types::SharedPtr<Texture> main_texture) {
    texture->Load(FileNameRGB,
         FileNameA,
         stage,
         mipmap,
         target,
         imagetarget,
         alpha,
         zeroval,
         force_load,
         max_dimension_size,
         detail_texture,
         nocache,
         address_mode,
         main_texture);
    return texture;
}
