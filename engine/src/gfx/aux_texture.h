/*
 * aux_texture.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_TEXTURE_H
#define VEGA_STRIKE_ENGINE_GFX_TEXTURE_H

#include "gfx/vsimage.h"
#include "src/gfxlib.h"
#include "src/gfxlib_struct.h"
#include "src/SharedPool.h"

#include <string>
//#include "gfx/vsimage.h"
//#include "root_generic/vsfilesystem.h" this is included by gfxlib.h

/**
 *  The texture class defines a method of loading bitmap textures
 *  It also defines a hash table in which textures are kept and looked up
 *  to prevent the loading of duplicate textures
 */
;
class Texture : public ::VSImage {
    typedef unsigned int uint;
public:
    void FileNotFound(const std::string &); //undoes what it did to hash table when file is not located

    ///The file name used to load this texture
    StringPool::Reference texfilename;

    ///the filter mode of this texture
    enum FILTER ismipmapped;

    ///The data of this texture (used in between functions, deleted)
    unsigned char *data;

    ///The GFXname of this texture
    int name;

    ///The multitexture stage of this texture
    int stage;

    ///The minimum/maximum texturing coordinates for this texture (have in mind that it is not always enforce, ie. for meshes) (i=s, j=t, k=u)
    Vector mintcoord, maxtcoord;

    ///The original data that would represent this texture
    Texture *original;

    ///For re-biding
    bool bound;
    uint boundSizeX, boundSizeY;
    VSImageMode boundMode;

    ///The number of references on the original data
    int refcount;

    ///The target this will go to (cubemap or otherwise)
    enum TEXTURE_TARGET texture_target;
    enum TEXTURE_IMAGE_TARGET image_target;

    ///The address mode being used with this texture
    enum ADDRESSMODE address_mode;

    ///Returns if this texture is actually already loaded
    GFXBOOL checkold(const std::string &s, bool shared, std::string &hashname);
    void modold(const std::string &s, bool shared, std::string &hashname);

    ///Loads the old texture
    void setold();
    bool checkbad(const std::string &s);
    void setbad(const std::string &s);

    ///Inits the class with default values
    void InitTexture();

protected:

    ///Binds this texture to GFX library
    int Bind(int maxdimension, GFXBOOL detailtexture);

    ///UnBinds from GFX library
    void UnBind();

    ///Transfers this texture to GFX library
    void Transfer(int maxdimension, GFXBOOL detailtexture);

public:

    ///Binds this texture to the same name as the given texture - for multipart textures
    int Bind(Texture *other, int maxdimension = 65536, GFXBOOL detailtexture = GFXFALSE);

public:

    ///Creates an unbounded texture. Set data and dimensions before binding. Or explicitly load a file.
    Texture(int stage = 0,
            enum FILTER mipmap = MIPMAP,
            enum TEXTURE_TARGET target = TEXTURE2D,
            enum TEXTURE_IMAGE_TARGET imagetarget = TEXTURE_2D,
            enum ADDRESSMODE address_mode = DEFAULT_ADDRESS_MODE);

    ///Creates a texture with a single bitmap as color data and another grayscale .bmp as alpha data
    Texture(const char *,
            const char *,
            int stage = 0,
            enum FILTER mipmap = MIPMAP,
            enum TEXTURE_TARGET target = TEXTURE2D,
            enum TEXTURE_IMAGE_TARGET imagetarget = TEXTURE_2D,
            float alpha = 1,
            int zeroval = 0,
            GFXBOOL force = GFXFALSE,
            int max_dimension_size = 65536,
            GFXBOOL detail_texture = GFXFALSE,
            GFXBOOL nocache = false,
            enum ADDRESSMODE address_mode = DEFAULT_ADDRESS_MODE,
            Texture *main = 0);

    ///Creates a texture with only color data as a single bitmap
    Texture(const char *FileName,
            int stage = 0,
            enum FILTER mipmap = MIPMAP,
            enum TEXTURE_TARGET target = TEXTURE2D,
            enum TEXTURE_IMAGE_TARGET imagetarget = TEXTURE_2D,
            GFXBOOL force = GFXFALSE,
            int max_dimension_size = 65536,
            GFXBOOL detail_texture = GFXFALSE,
            GFXBOOL nocache = false,
            enum ADDRESSMODE address_mode = DEFAULT_ADDRESS_MODE,
            Texture *main = 0);
    Texture(VSFileSystem::VSFile *f,
            int stage = 0,
            enum FILTER mipmap = MIPMAP,
            enum TEXTURE_TARGET target = TEXTURE2D,
            enum TEXTURE_IMAGE_TARGET imagetarget = TEXTURE_2D,
            GFXBOOL force = GFXFALSE,
            int max_dimension_size = 65536,
            GFXBOOL detail_texture = GFXFALSE,
            GFXBOOL nocache = false,
            enum ADDRESSMODE address_mode = DEFAULT_ADDRESS_MODE,
            Texture *main = 0);
    void Load(const char *,
            const char *,
            int stage = 0,
            enum FILTER mipmap = MIPMAP,
            enum TEXTURE_TARGET target = TEXTURE2D,
            enum TEXTURE_IMAGE_TARGET imagetarget = TEXTURE_2D,
            float alpha = 1,
            int zeroval = 0,
            GFXBOOL force = GFXFALSE,
            int max_dimension_size = 65536,
            GFXBOOL detail_texture = GFXFALSE,
            GFXBOOL nocache = false,
            enum ADDRESSMODE address_mode = DEFAULT_ADDRESS_MODE,
            Texture *main = 0);
    void Load(const char *FileName,
            int stage = 0,
            enum FILTER mipmap = MIPMAP,
            enum TEXTURE_TARGET target = TEXTURE2D,
            enum TEXTURE_IMAGE_TARGET imagetarget = TEXTURE_2D,
            GFXBOOL force = GFXFALSE,
            int max_dimension_size = 65536,
            GFXBOOL detail_texture = GFXFALSE,
            GFXBOOL nocache = false,
            enum ADDRESSMODE address_mode = DEFAULT_ADDRESS_MODE,
            Texture *main = 0);
    virtual const Texture *Original() const;
    virtual Texture *Original();
    virtual Texture *Clone();

    ///Texture copy constructor that increases appropriate refcounts
    //Texture (Texture *t);

    ///Destructor for texture
    ~Texture() override;

    virtual unsigned int numFrames() const {
        return 1;
    }                                               //Number of frames in an animated texture (or, rather, keyframes)

    virtual double curTime() const {
        return 0;
    }                                       //Current time of an animated texture

    virtual void setTime(double tim) {
    }

    virtual float framesPerSecond() const {
        return 0;
    }                                               //Frames per second, if applicable.

    virtual unsigned int numLayers() const {
        return 1;
    }                                               //Number of layers in the texture. If more than 1, makeActive() sets up multitexturing units, and other use of multitexture units, other than [stage..stage+numLayers), should be avoided. If not enough multitexturing units are available, special fall-back mechanisms should be implmemented by MakeActive() so that it still renders as correctly as possible. In that case, canMultiPass(), numPasses() and MakeActive(stage,pass) could be used to process in multipass mode (one stage at a time, or a stage group at a time, depending on fallback implementation), if possible.

    virtual unsigned int numPasses() const {
        return 1;
    }                                               //Number of passes when on multipass rendering.

    virtual bool canMultiPass() const {
        return true;
    }                                             //True, if full functionality can be achieved after multipass rendering. If false, fallback rendering must be possible by multipass (or singlepass) rendering. True, also, if texturing units are sufficient for singlepass rendering.

    virtual bool constFrameRate() const {
        return true;
    }                                               //If frames advance at a regular rate - keyframe based animations would return false.

    int getStage() const {
        return stage;
    }

    int getFilter() const {
        return ismipmapped;
    }

    int getTarget() const {
        return texture_target;
    }

    int getImageTarget() const {
        return image_target;
    }

    ///Whether or not the string exists as a texture
    static Texture *Exists(std::string s);

    ///Whether or not the color and alpha data already exist
    static Texture *Exists(std::string s, std::string a);

    ///A way to sort the texture by the original address (to make sure like textures stick togehter
    bool operator<(const Texture &b) const;

    ///A way to test if the texture is equal to another based on original values
    bool operator==(const Texture &b) const;

    ///Make this instance a reference of "other"
    void setReference(Texture *other);

    ///Binds the texture in the GFX library
    virtual void MakeActive() {
        MakeActive(this->stage, 0);
    }                                                      //Assumes stage=this->stage, pass=0

    virtual void MakeActive(int stage) {
        MakeActive(stage, 0);
    }                                                         //Assumes pass=0

    virtual void MakeActive(int stage, int pass);

    virtual bool SetupPass(int pass, const enum BLENDFUNC src, const enum BLENDFUNC dst) {
        return SetupPass(pass, stage, src, dst);
    }                                                                                                                            //If one is going to perform multipass rendering of this texture, the Texture() must handle blending - SetupPass() sets up blending. If it returns false, then blending is not compatible with the requested blend mode emulation. One may assume that if numPasses()==1, no SetupPass() is needed. pass==-1 means restore setup. You should call it after multipass rendering.

    virtual bool SetupPass(int pass, int stage, const enum BLENDFUNC src, const enum BLENDFUNC dst) {
        return true;
    }                                                                                                              //If one is going to perform multipass rendering of this texture, the Texture() must handle blending - SetupPass() sets up blending. If it returns false, then blending is not compatible with the requested blend mode emulation. One may assume that if numPasses()==1, no SetupPass() is needed. pass==-1 means restore setup. You should call it after multipass rendering.

    ///If the texture has loaded properly returns true
    virtual bool LoadSuccess() {
        return name >= 0;
    }

    ///Changes priority of texture
    virtual void Prioritize(float);
};

#endif //VEGA_STRIKE_ENGINE_GFX_TEXTURE_H
