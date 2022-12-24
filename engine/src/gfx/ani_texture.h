/*
 * ani_texture.h
 *
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


#ifndef __ANI_TEXTURE_H__
#define __ANI_TEXTURE_H__

#include "aux_texture.h"
#include "vsfilesystem.h"
#include "vid_file.h"

#include <cstdio>
#include "../SharedPool.h"

#include "audio/Types.h"
#include "preferred_types.h"
#include "audio/Source.h"

class AnimatedTexture : public Texture {
    vega_types::SharedPtr<vega_types::SequenceContainer<vega_types::SharedPtr<Texture>>> Decal;
    unsigned int activebound; //For video mode
    double physicsactive;
    bool loadSuccess;

    void AniInit();

    //For video mode
    bool vidMode;
    bool detailTex;
    enum FILTER ismipmapped;
    int texstage;
    vega_types::SharedPtr<Audio::Source> timeSource;

    vector<StringPool::Reference> frames; //Filenames for each frame
    vector<Vector> frames_maxtc; //Maximum tcoords for each frame
    vector<Vector> frames_mintc; //Minimum tcoords for each frame

    VidFile *vidSource;

    StringPool::Reference wrapper_file_path;
    VSFileSystem::VSFileType wrapper_file_type;

    //Options
    enum optionenum {
        optInterpolateFrames = 0x01,
        optInterpolateTCoord = 0x02,
        optLoopInterp = 0x04,
        optLoop = 0x08,
        optSoundTiming = 0x10
    };
    unsigned char options;

    //Implementation
    GFXColor multipass_interp_basecolor;
    enum ADDRESSMODE defaultAddressMode; //default texture address mode, read from .ani

protected:
    unsigned int numframes;
    float timeperframe;
    unsigned int active;
    unsigned int nextactive; //It is computable, but it's much more convenient this way
    float active_fraction; //For interpolated animations
    double curtime;

    // for video de-jittering
    double lastcurtime;
    double lastrealtime;

    bool constframerate;
    bool done;

public:
    void setTime(double tim) override;

    double curTime() const override {
        return curtime;
    }

    unsigned int numFrames() const override {
        return numframes;
    }

    float framesPerSecond() const override {
        return 1 / timeperframe;
    }

    unsigned int numLayers() const override;

    unsigned int numPasses() const override;

    bool canMultiPass() const override {
        return true;
    }

    bool constFrameRate() const override {
        return constframerate;
    }

    AnimatedTexture();

    AnimatedTexture(int stage, enum FILTER imm, bool detailtexture = false);

    AnimatedTexture(const char *file, int stage, enum FILTER imm, bool detailtexture = false);

    AnimatedTexture(VSFileSystem::VSFile &openedfile, int stage, enum FILTER imm, bool detailtexture = false);

    void Load(VSFileSystem::VSFile &f, int stage, enum FILTER ismipmapped, bool detailtex = false);

    void LoadAni(VSFileSystem::VSFile &f, int stage, enum FILTER ismipmapped, bool detailtex = false);

    void LoadVideoSource(VSFileSystem::VSFile &f);

    virtual void LoadFrame(int num); //For video mode

    void Destroy();

    const vega_types::SharedPtr<const Texture> OriginalConst() const override;

    virtual vega_types::SharedPtr<Texture> Original();

    ~AnimatedTexture() override;

    virtual vega_types::SharedPtr<Texture> Clone();

    virtual void MakeActive() {
        MakeActive(texstage, 0);
    }                                                   //MSVC bug seems to hide MakeActive() if we define MakeActive(int,int) - the suckers!

    virtual void MakeActive(int stage) {
        MakeActive(stage, 0);
    }                                                         //MSVC bug seems to hide MakeActive(int) if we define MakeActive(int,int) - the suckers!

    virtual void MakeActive(int stage, int pass);

    bool SetupPass(int pass, int stage, const enum BLENDFUNC src, const enum BLENDFUNC dst);

    bool SetupPass(int pass, const enum BLENDFUNC src, const enum BLENDFUNC dst) {
        return SetupPass(pass, texstage, src, dst);
    }

    void SetInterpolateFrames(bool set) {
        options = (options & ~optInterpolateFrames) | (set ? optInterpolateFrames : 0);
    }

    void SetInterpolateTCoord(bool set) {
        options = (options & ~optInterpolateTCoord) | (set ? optInterpolateTCoord : 0);
    }

    void SetLoopInterp(bool set) {
        options = (options & ~optLoopInterp) | (set ? optLoopInterp : 0);
    }

    void SetLoop(bool set) {
        options = (options & ~optLoop) | (set ? optLoop : 0);
    }

    bool GetInterpolateFrames() const {
        return (options & optInterpolateFrames) != 0;
    }

    bool GetInterpolateTCoord() const {
        return (options & optInterpolateTCoord) != 0;
    }

    bool GetLoopInterp() const {
        return (options & optLoopInterp) != 0;
    }

    bool GetLoop() const {
        return (options & optLoop) != 0;
    }

    vega_types::SharedPtr<Audio::Source> GetTimeSource() const {
        return (options & optSoundTiming) ? timeSource : vega_types::SharedPtr<Audio::Source>();
    }

    void SetTimeSource(vega_types::SharedPtr<Audio::Source> source);

    void ClearTimeSource();

    static void UpdateAllPhysics();

    static void UpdateAllFrame();

    //resets the animation to beginning
    void Reset();

    bool Done() const;

    virtual bool LoadSuccess();

    //Some useful factory methods -- also defined in ani_texture.cpp
    static AnimatedTexture *CreateVideoTexture(const std::string &fname,
            int stage = 0,
            enum FILTER ismipmapped = BILINEAR,
            bool detailtex = false);
};

#endif

