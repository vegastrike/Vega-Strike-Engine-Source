/*
 * env_map_gent.cpp
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


#include "src/endianness.h"
#include <float.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "gfx/vsimage.h"
#include "gfx/aux_texture.h"
#include "gldrv/sdds.h"
#ifndef WIN32
#else
#ifndef NOMINMAX
#define NOMINMAX 1
#endif //tells VCC not to generate min/max macros
#include <windows.h>
#include <wingdi.h>
#endif

#if _MSC_VER >= 1300
#define snprintf _snprintf
#endif

static char *InputName = NULL;
static char *OutputName = NULL;
static bool pushdown = false;
static float affine = 0;
static float multiplicitive = 1;
static float power = 1;

using namespace VSFileSystem;   // FIXME -- Shouldn't include an entire namespace, according to Google Style Guide -- stephengtuggy 2021-09-07

/*
struct Vector {
    float i;
    float j;
    float k;
};

#define NumLights 1

struct RGBColor
{
    float r, b, g;
};

struct Light
{
    Vector   Dir;
    RGBColor Ambient;
    RGBColor Intensity;
};

struct Material
{
    RGBColor Ka;
    RGBColor Kd;
    RGBColor Ks;
    int exp;
};

static Light    L[NumLights];
static Material M;

static float Power( float A, int B )
{
    float res = 1.;
    for (int i = 0; i < B; i++)
        res *= A;
    return res;
}

static void Lighting( RGBColor &Col, const Vector &Norm )
{
    static float OONL = 1./NumLights;
    Col.r = Col.g = Col.b = 0;
    for (int i = 0; i < NumLights; i++) {
        float dot = L[i].Dir.i*Norm.i+L[i].Dir.j*Norm.j+L[i].Dir.k*Norm.k;
        if (dot < 0) dot = 0;
        Col.r += OONL*L[i].Ambient.r*M.Ka.r+L[i].Intensity.r*( M.Kd.r*dot+M.Ks.r*Power( dot, M.exp ) );
        Col.g += OONL*L[i].Ambient.g*M.Ka.g+L[i].Intensity.g*( M.Kd.g*dot+M.Ks.g*Power( dot, M.exp ) );
        Col.b += OONL*L[i].Ambient.b*M.Ka.b+L[i].Intensity.b*( M.Kd.b*dot+M.Ks.b*Power( dot, M.exp ) );
        if (Col.r > 1) Col.r = 1;
        if (Col.b > 1) Col.b = 1;
        if (Col.g > 1) Col.g = 1;
        if (Col.r < 0) Col.r = 0;
        if (Col.b < 0) Col.b = 0;
        if (Col.g < 0) Col.g = 0;
    }
}
*/

const int lmwid = 512;
const int lmwido2 = lmwid / 2;
const float ooWIDTHo2 = 2. / lmwid;
const float PIoWIDTHo2 = 2 * 3.1415926535 / lmwid;
const char bytepp = 3;

struct CubeCoord {
    float s;
    float t;
    unsigned int TexMap;    //0 = front, 1=back,2=right,3=left,4=up,5=down
    unsigned int padding;  //added by chuck_starchaser
};

static void gluSphereMap(CubeCoord &Tex, Vector Normal, float Theta) {
    Tex.TexMap = 0;
    float vert = Normal.j;
    if (!pushdown) {
        Tex.t = vert * lmwido2 + lmwido2;
        Tex.s = Theta;
    } else {
        Tex.t = ((int) (vert * lmwido2)) % lmwid;
        Tex.s = Theta;
    }
}

static void TexMap(CubeCoord &Tex, Vector Normal) {
    float r[6] = {0, 0, 0, 0, 0, 0};
    Normal.i = Normal.i;
    Normal.j = -Normal.j;
    Normal.k = -Normal.k;
    const float CubeSize = lmwido2;     //half of the length of any of the cube's sides
    if (Normal.k) {
        r[0] = CubeSize / Normal.k;
    }                   //find what you need to multiply to get to the cube
    if (Normal.i) {
        r[2] = CubeSize / Normal.i;
    }                   //find what you need to multiply to get to the cube
    if (Normal.j) {
        r[4] = CubeSize / Normal.j;
    }                   //find what you need to multiply to get to the cube
    if (!Normal.k) {
        r[0] = r[1] = CubeSize * 1000;
    }
    if (!Normal.i) {
        r[2] = r[3] = CubeSize * 1000;
    }
    if (!Normal.j) {
        r[4] = r[5] = CubeSize * 1000;
    }
    r[1] = -r[0];
    r[3] = -r[2];
    r[5] = -r[4];

    float rf;
    rf = CubeSize * 1000;
    for (int i = 0; i < 6; i++) {
        if (r[i] >= CubeSize) {
            if (rf > r[i]) {
                rf = r[i];
                Tex.TexMap = i;
            }
        }
    }
    ///find coordinates on this tex map 'box in space'
    switch (Tex.TexMap) {
        case 0:
            Tex.s = rf * Normal.i + lmwido2;         //btw 0 and 256
            Tex.t = lmwido2 - rf * Normal.j;          //top left is 0,0
            break;
        case 1:
            Tex.s = lmwido2 - rf * Normal.i;         //btw 0 and 256
            Tex.t = lmwido2 - rf * Normal.j;          //top left is 0,0
            break;
        case 2:
            Tex.s = lmwido2 - rf * Normal.k;
            Tex.t = lmwido2 - rf * Normal.j;
            break;
        case 3:
            Tex.s = lmwido2 + rf * Normal.k;
            Tex.t = lmwido2 - rf * Normal.j;
            break;
        case 4:
            Tex.t = lmwido2 - rf * Normal.i;
            Tex.s = lmwido2 + rf * Normal.k;
            break;
        case 5:
            Tex.s = lmwido2 - rf * Normal.i;
            Tex.t = lmwido2 + rf * Normal.k;
            break;
    }
}

static bool LoadTex(char *FileName, unsigned char scdata[lmwid][lmwid][3]) {
    using namespace VSFileSystem;

    VSFile f;
    VSError err = f.OpenReadOnly(FileName, TextureFile);
    if (err > Ok) {
        return false;
    }
    Texture tex;
    unsigned char *data = tex.ReadImage(&f, texTransform, true);
    int bpp = tex.Depth();
    int format = tex.Format();

    unsigned char *buffer = NULL;
    bpp /= 8;
    //999 is the code for DDS file, we must decompress them.
    if (format == 999) {
        unsigned char *tmpbuffer = data + 2;
        TEXTUREFORMAT internformat;
        bpp = 1;
        //Make sure we are reading a DXT1 file. All backgrounds
        //should be DXT1
        switch (tex.mode) {
            case ::VSImage::_DXT1:
                internformat = DXT1;
                break;
            default:
                return false;
        }
        //we could hardware decompress, but that involves more
        //pollution of gl in gfx.
        ddsDecompress(tmpbuffer, buffer, internformat, tex.sizeY, tex.sizeX);
        //We are done with the DDS file data.  Remove it.
        free(data);
        data = buffer;

        //stride and row_pointers are used for the texTransform
        unsigned long stride = 4 * sizeof(unsigned char);
        unsigned char **row_pointers = (unsigned char **) malloc(sizeof(unsigned char *) * tex.sizeY);
        for (unsigned int i = 0; i < tex.sizeY; ++i) {
            row_pointers[i] = &data[i * stride * tex.sizeX];
        }
        //texTransform demands that the first argument (bpp) be 8. So we abide
        int tmp = 8;
        int tmp2 = PNG_HAS_COLOR + PNG_HAS_ALPHA;
        buffer = texTransform(tmp, tmp2, tex.sizeX, tex.sizeY, row_pointers);
        //We're done with row_pointers, free it
        free(row_pointers);
        row_pointers = NULL;
        //We're done with the decompressed dds data, free it
        free(data);
        //We set data to the transformed image data
        data = buffer;
        buffer = NULL;
        //it's 3 because 24/8
        bpp = 4;
    } else if (format & PNG_HAS_ALPHA) {
        bpp *= 4;
    } else {
        bpp *= 3;
    }
    if (data) {
        int ii;
        int jj;
        for (int i = 0; i < lmwid; i++) {
            ii = (i * tex.sizeY) / lmwid;
            for (int j = 0; j < lmwid; j++) {
                jj = (j * tex.sizeX) / lmwid;
                scdata[i][j][0] = data[(ii * tex.sizeX + jj) * bpp];
                scdata[i][j][1] = data[(ii * tex.sizeX + jj) * bpp + 1];
                scdata[i][j][2] = data[(ii * tex.sizeX + jj) * bpp + 2];
            }
        }
        free(data);
    } else {
        return false;
    }
    f.Close();
    return true;
}

struct Texmp {
    unsigned char D[lmwid][lmwid][3];
};

static char *makebgname(char *tmp, size_t size, const char *InputName, const char *add, const char *suffix) {
    size_t len = strlen(InputName) + strlen(add) + strlen(suffix) + 1;
    if (size > len) {
        strcpy(tmp, InputName);
        strcat(tmp, add);
        strcat(tmp, suffix);
    } else {
        std::string temp = InputName;
        temp += add;
        temp += suffix;
        strncpy(tmp, temp.c_str(), size);
        tmp[size - 1] = '\0';
    }
    return tmp;
}

static void Spherize(CubeCoord Tex[lmwid][lmwid], CubeCoord gluSph[lmwid][lmwid], unsigned char Col[]) {
    Texmp *Data = NULL;
    Data = new Texmp[6];
    if (!Data) {
        return;
    }          //borken down and down Data[5], right Data[3]

    size_t tmpsize = strlen(InputName) + 60;
    char *tmp = (char *) malloc(tmpsize);
    const char *suffix = ".image";
    {
        std::string temp(InputName);
        if (VSFileSystem::LookForFile(temp + "_up.image", TextureFile) > VSFileSystem::Ok) {
            //greater than Ok means failed to load.
            if (VSFileSystem::LookForFile(temp + "_sphere.image", TextureFile) > VSFileSystem::Ok) {
                if (VSFileSystem::LookForFile(temp + ".image", TextureFile) > VSFileSystem::Ok) {
                    suffix = ".bmp";
                }
            }
        }
        //backwards compatibility
    }
    if (!(LoadTex(makebgname(tmp, tmpsize, InputName, "_front",
                    suffix),
            Data[0].D)
            && LoadTex(makebgname(tmp, tmpsize, InputName, "_back",
                            suffix),
                    Data[1].D)
            && LoadTex(makebgname(tmp, tmpsize, InputName, "_left",
                            suffix),
                    Data[2].D)
            && LoadTex(makebgname(tmp, tmpsize, InputName, "_right",
                            suffix),
                    Data[3].D)
            && LoadTex(makebgname(tmp, tmpsize, InputName, "_up",
                            suffix),
                    Data[4].D)
            && LoadTex(makebgname(tmp, tmpsize, InputName, "_down",
                    suffix), Data[5].D))) {
        if (!LoadTex(makebgname(tmp, tmpsize, InputName, "_sphere", suffix), Data[0].D)) {
            LoadTex(makebgname(tmp, tmpsize, InputName, "", suffix), Data[0].D);
        }
        Tex = gluSph;
    }
    free(tmp);
    tmp = NULL;
    for (int t = 0; t < lmwid; t++) {
        for (int s = 0; s < lmwid; s++) {
            float r = 0;
            float g = 0;
            float b = 0;
            {
                float avg = 1;
                if ((int) floor(Tex[t][s].s) > lmwid - 1) {
                    Tex[t][s].s = lmwid - 1;
                }
                if ((int) floor(Tex[t][s].t) > lmwid - 1) {
                    Tex[t][s].t = lmwid - 1;
                }
                if ((int) floor(Tex[t][s].t) < 0) {
                    Tex[t][s].t = 0;
                }
                if ((int) floor(Tex[t][s].s) < 0) {
                    Tex[t][s].s = 0;
                }
                r = Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s)][0];
                g = Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s)][1];
                b = Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s)][2];
                if ((int) floor(Tex[t][s].s) < lmwid - 1) {
                    avg++;
                    r += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s + 1)][0];
                    g += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s + 1)][1];
                    b += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s + 1)][2];
                }
                if ((int) floor(Tex[t][s].t) < lmwid - 1) {
                    avg++;
                    r += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t + 1)][(int) floor(Tex[t][s].s)][0];
                    g += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t + 1)][(int) floor(Tex[t][s].s)][1];
                    b += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t + 1)][(int) floor(Tex[t][s].s)][2];
                }
                if ((int) floor(Tex[t][s].t) > 0) {
                    avg++;
                    r += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t - 1)][(int) floor(Tex[t][s].s)][0];
                    g += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t - 1)][(int) floor(Tex[t][s].s)][1];
                    b += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t - 1)][(int) floor(Tex[t][s].s)][2];
                }
                if ((int) floor(Tex[t][s].s) > 0) {
                    avg++;
                    r += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s - 1)][0];
                    g += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s - 1)][1];
                    b += Data[Tex[t][s].TexMap].D[(int) floor(Tex[t][s].t)][(int) floor(Tex[t][s].s - 1)][2];
                }
                r /= avg;
                g /= avg;
                b /= avg;
            }
            unsigned int rr = (unsigned int) r;
            unsigned int gg = (unsigned int) g;
            unsigned int bb = (unsigned int) b;
            if (affine != 0 || multiplicitive != 1 || power != 1) {
                rr = (unsigned int) (affine + ((std::pow((float) r, power)) * multiplicitive));
                gg = (unsigned int) (affine + ((std::pow((float) g, power)) * multiplicitive));
                bb = (unsigned int) (affine + ((std::pow((float) b, power)) * multiplicitive));
            }
            if (rr > 255) {
                rr = 255;
            }
            if (gg > 255) {
                gg = 255;
            }
            if (bb > 255) {
                bb = 255;
            }
            Col[bytepp * (lmwid * (lmwid - 1 - t) + (lmwid - 1 - s))] = rr;
            Col[bytepp * (lmwid * (lmwid - 1 - t) + (lmwid - 1 - s)) + 1] = gg;
            Col[bytepp * (lmwid * (lmwid - 1 - t) + (lmwid - 1 - s)) + 2] = bb;
            //Col[4*(256*(255-t)+(255-s))+3] = 255;
        }
    }
    delete[] Data;
}

static void GenerateSphereMap() {
    //float SinPhi;
    //float CosPhi;
    //float Theta;
    Vector Normal;
    //RGBColor Col;
    static CubeCoord TexCoord[lmwid][lmwid];
    static CubeCoord gluSphereCoord[lmwid][lmwid];
    unsigned char *LightMap = (unsigned char *) malloc(lmwid * lmwid * 4);
    int t;
    for (t = 0; t < lmwid; t++) {
        //keep in mind that t = lmwido2 (sin phi) +lmwido2
        float to256 = t / (104. * lmwid / 256) - 1.23;
        for (int s = 0; s < lmwid; s++) {
            //is is none other than Theta * lmwido2/PI
            float so256 = s / (104. * lmwid / 256) - 1.23;
            Normal.k = 2 * (1 - so256 * so256 - to256 * to256);
            float double_one_more_normal = 2 * (Normal.k + 1);
            if (double_one_more_normal >= 0) {
                Normal.i = so256 * sqrt(2 * (Normal.k + 1));
                Normal.j = to256 * sqrt(2 * (Normal.k + 1));
            } else {
                Normal.i = Normal.j = 0.0;
            }
            float sz = sqrt(Normal.k * Normal.k + Normal.j * Normal.j + Normal.i * Normal.i);
            Normal.k /= -sz;
            Normal.i /= sz;
            Normal.j /= sz;
            TexMap(TexCoord[t][s], Normal);             //find what the lighting is
            gluSphereMap(gluSphereCoord[t][s], Normal, s);
        }
    }
    Spherize(TexCoord, gluSphereCoord, LightMap);
    ::VSImage image;
    image.WriteImage((char *) OutputName, LightMap, PngImage, lmwid, lmwid, false, 8, TextureFile);
}

void EnvironmentMapGeneratorMain(const char *inpt, const char *outpt, float a, float m, float p, bool w) {
    affine = a;
    multiplicitive = m;
    power = p;
    pushdown = w;
    int size = sizeof(char) * strlen(inpt) + 40;
    char *tmp = (char *) malloc(size);
    strcpy(tmp, inpt);
    VSFile f;
    VSError err = f.OpenReadOnly(strcat(tmp, "_sphere.image"), TextureFile);
    if (err > Ok) {
        err = f.OpenReadOnly(strcat(tmp, "_sphere.bmp"), TextureFile);
        if (err > Ok) {
            memset(tmp, 0, size);
            strcpy(tmp, inpt);
            err = f.OpenReadOnly(strcat(tmp, "_up.image"), TextureFile);
            if (err > Ok) {
                err = f.OpenReadOnly(strcat(tmp, "_up.bmp"), TextureFile);
            }
        }
    }
    //bool share = false;
    std::string s;
    if (err > Ok) {
        //s = VSFileSystem::GetSharedTexturePath (std::string (inpt));
        s = VSFileSystem::sharedtextures + "/" + std::string(inpt);
        InputName = (char *) malloc(sizeof(char) * (s.length() + 2));
        strcpy(InputName, s.c_str());
    } else {
        f.Close();
        InputName = (char *) malloc(sizeof(char) * (strlen(inpt) + 2));
        strcpy(InputName, inpt);
    }
    OutputName = strdup(outpt);
    free(tmp);
    tmp = NULL;
    VS_LOG(info, (boost::format("input name %1%, output name %2%") % InputName % OutputName));
    VS_LOG(info, (boost::format("Affine %1% Mult %2% Pow %3%") % affine % multiplicitive % power));
    GenerateSphereMap();
    free(InputName);
    free(OutputName);
}



