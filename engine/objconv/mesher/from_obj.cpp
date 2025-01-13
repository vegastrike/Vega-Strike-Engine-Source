/*
 * from_obj.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

#include "PrecompiledHeaders/Converter.h"
#include "from_obj.h"

#include <utility>

#include <cstring>  //We are using C style string functions here

using namespace std;

/* Renamed to get rid of namespace collisions */
class TObj_Vector {
public:
    float i, j, k;

    TObj_Vector() {
    }

    TObj_Vector(const GFXVertex &v) : i(v.x), j(v.y), k(v.z) {
    }

    TObj_Vector(float _i, float _j, float _k) : i(_i), j(_j), k(_k) {
    }

    TObj_Vector operator+(const TObj_Vector &o) const {
        return TObj_Vector(i + o.i, j + o.j, k + o.k);
    }

    TObj_Vector operator-(const TObj_Vector &o) const {
        return TObj_Vector(i - o.i, j - o.j, k - o.k);
    }

    TObj_Vector Cross(const TObj_Vector &v) const {
        return TObj_Vector(j * v.k - k * v.j, k * v.i - i * v.k, i * v.j - j * v.i);
    }

    float Dot(const TObj_Vector &b) const {
        return i * b.i + j * b.j + k * b.k;
    }

    void Normalize() {
        float f = Dot(*this);
        if (f > 0) {
            f = 1.0f / (float) sqrt(f);
        }
        i *= f;
        j *= f;
        k *= f;
    }

    bool operator==(const TObj_Vector &o) const {
        return i == o.i && j == o.j && k == o.k;
    }
};

struct MTL : public GFXMaterial {
    MTL() {
        blend_src = ONE;
        blend_dst = ZERO;
        reflect = false;
        usenormals = true;
    }

    bool usenormals;
    bool reflect;
    int blend_src;
    int blend_dst;
    vector<textureholder> textures;
    textureholder detail;

    vector<float> detailplanei;
    vector<float> detailplanej;
    vector<float> detailplanek;
};
struct IntRef {
    int v;
    int n;
    int t;

    IntRef() {
        v = -1;
        n = -1;
        t = -1;
    }

    IntRef(int vv, int nn, int tt) {
        v = vv;
        n = nn;
        t = tt;
    }

    bool operator==(const IntRef &o) const {
        return v == o.v && n == o.n && t == o.t;
    }

    bool operator!=(const IntRef &o) const {
        return v != o.v || n != o.n || t != o.t;
    }

    bool operator<(const IntRef &o) const {
        if (v < o.v) {
            return true;
        } else if (v == o.v) {
            if (n < o.n) {
                return true;
            } else if (n == o.n) {
                return t < o.t;
            }
        }
        return false;
    }
};
struct VTX {
    VTX(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {
    }

    VTX() : x(0), y(0), z(0) {
    }

    bool operator==(const VTX &o) const {
        return x == o.x && y == o.y && z == o.z;
    }

    bool operator!=(const VTX &o) const {
        return x != o.x || y != o.y || z != o.z;
    }

    bool operator<(const VTX &o) const {
        return (x < o.x) || ((x == o.x) && ((y < o.y) || ((y == o.y) && (z < o.z))));
    }

    float x, y, z;
};
struct TEX {
    TEX(float _s, float _t) : s(_s), t(_t) {
    }

    TEX() : s(0), t(0) {
    }

    bool operator==(const TEX &o) const {
        return s == o.s && t == o.t;
    }

    bool operator!=(const TEX &o) const {
        return s != o.s || t != o.t;
    }

    bool operator<(const TEX &o) const {
        return (s < o.s) || ((s == o.s) && (t < o.t));
    }

    float s, t;
};
struct NORMAL {
    NORMAL(float _i, float _j, float _k) : i(_i), j(_j), k(_k) {
    }

    NORMAL() : i(1), j(0), k(0) {
    }

    bool operator==(const NORMAL &o) const {
        return i == o.i && j == o.j && k == o.k;
    }

    bool operator!=(const NORMAL &o) const {
        return i != o.i || j != o.j || k != o.k;
    }

    bool operator<(const NORMAL &o) const {
        return (i < o.i) || ((i == o.i) && ((j < o.j) || ((j == o.j) && (k < o.k))));
    }

    float i, j, k;
};
struct FACE {
    FACE() : num(0) {
    }

    FACE(const IntRef &_r1, const IntRef &_r2) : num(2), r1(_r1), r2(_r2) {
    }

    FACE(const IntRef &_r1, const IntRef &_r2, const IntRef &_r3) : num(3), r1(_r1), r2(_r2), r3(_r3) {
    }

    FACE(const IntRef &_r1, const IntRef &_r2, const IntRef &_r3, const IntRef &_r4)
            : num(4), r1(_r1), r2(_r2), r3(_r3), r4(_r4) {
    }

    int num;
    IntRef r1, r2, r3, r4;
};

vector<string> splitWhiteSpace(string inp) {
    string::size_type where;
    vector<string> ret;
    while ((where = inp.find_first_of("\t ")) != string::npos) {
        if (where != 0) {
            ret.push_back(inp.substr(0, where));
        }
        inp = inp.substr(where + 1);
    }
    if (inp.length() && where == string::npos) {
        ret.push_back(inp);
    }
    return ret;
}

IntRef parsePoly(string inp) {
    IntRef ret;
    const char *st = inp.c_str();
    if (3 == sscanf(st, "%d/%d/%d", &ret.v, &ret.t, &ret.n)) {
        ret.v -= 1;
        ret.t -= 1;
        ret.n -= 1;
        return ret;
    }
    if (2 == sscanf(st, "%d/%d/", &ret.v, &ret.t)) {
        ret.v -= 1;
        ret.t -= 1;
        ret.n = -1;
        return ret;
    }
    if (2 == sscanf(st, "%d/%d", &ret.v, &ret.t)) {
        ret.v -= 1;
        ret.t -= 1;
        ret.n = -1;
        return ret;
    }
    if (2 == sscanf(st, "%d//%d", &ret.v, &ret.n)) {
        ret.v -= 1;
        ret.n -= 1;
        ret.t = -1;
        return ret;
    }
    sscanf(st, "%d", &ret.v);
    ret.v -= 1;
    ret.t = -1;
    ret.n = -1;
    return ret;
}

void charstoupper(char *buf) {
    while (*buf) {
        *buf = toupper(*buf);
        ++buf;
    }
}

void wordtoupper(char *buf) {
    while (*buf && !isspace(*buf)) {
        *buf = toupper(*buf);
        ++buf;
    }
}

textureholder makeTextureHolder(std::string str, int which) {
    textureholder ret;
    ret.type = TEXTURE;
    if (str.find(".ani") != string::npos) {
        ret.type = ANIMATION;
    }
    ret.index = which;
    for (std::string::iterator it = str.begin(); it != str.end(); it++) {
        ret.name.push_back((unsigned char) *it);
    }
    return ret;
}

textureholder makeTechniqueHolder(std::string str) {
    textureholder ret = makeTextureHolder(str, 0);
    ret.type = TECHNIQUE;
    return ret;
}

static int AddElement(XML &xml,
        const IntRef &r,
        map<IntRef, int> &elements,
        const vector<VTX> &vtxlist,
        const vector<TEX> &txclist,
        const vector<NORMAL> &normallist) {
    map<IntRef, int>::iterator e = elements.find(r);
    if (e == elements.end()) {
        pair<IntRef, int> elem = pair<IntRef, int>(r, int(xml.vertices.size()));
        elements.insert(elem);
        GFXVertex v;
        if (r.n >= 0) {
            v.i = normallist[r.n].i;
            v.j = normallist[r.n].j;
            v.k = normallist[r.n].k;
        } else {
            v.i = v.j = v.k = 0;
        }
        if (r.t >= 0) {
            v.s = txclist[r.t].s;
            v.t = txclist[r.t].t;
        } else {
            v.s = v.t = 0;
        }
        if (r.v >= 0) {
            v.x = vtxlist[r.v].x;
            v.y = vtxlist[r.v].y;
            v.z = vtxlist[r.v].z;
        } else {
            v.x = v.y = v.z = 0;
        }
        xml.vertices.push_back(v);

        return elem.second;
    } else {
        return (*e).second;
    }
}

static void ObjToXML(XML &xml,
        const vector<VTX> &vtxlist,
        const vector<TEX> &txclist,
        const vector<NORMAL> &normallist,
        const vector<FACE> &facelist) {
    map<IntRef, int> elements;
    for (vector<FACE>::const_iterator fiter = facelist.begin(); fiter != facelist.end(); fiter++) {
        int e1 = (((*fiter).num >= 1) ? AddElement(xml, (*fiter).r1, elements, vtxlist, txclist, normallist) : -1);
        int e2 = (((*fiter).num >= 2) ? AddElement(xml, (*fiter).r2, elements, vtxlist, txclist, normallist) : -1);
        int e3 = (((*fiter).num >= 3) ? AddElement(xml, (*fiter).r3, elements, vtxlist, txclist, normallist) : -1);
        int e4 = (((*fiter).num >= 4) ? AddElement(xml, (*fiter).r4, elements, vtxlist, txclist, normallist) : -1);
        switch ((*fiter).num) {
            case 2:
                //Line...
                xml.lines
                        .push_back(line(e1,
                                e2,
                                xml.vertices[e1].s,
                                xml.vertices[e1].t,
                                xml.vertices[e2].s,
                                xml.vertices[e2].t));
                break;
            case 3:
                //Triangle...
                xml.tris.push_back(triangle(e1, e2, e3, xml.vertices[e1].s, xml.vertices[e1].t, xml.vertices[e2].s,
                        xml.vertices[e2].t, xml.vertices[e3].s, xml.vertices[e3].t));
                break;
            case 4:
                //Quad...
                xml.quads.push_back(quad(e1, e2, e3, e4, xml.vertices[e1].s, xml.vertices[e1].t, xml.vertices[e2].s,
                        xml.vertices[e2].t, xml.vertices[e3].s, xml.vertices[e3].t, xml.vertices[e4].s,
                        xml.vertices[e4].t));
                break;
        }
    }
}

static FACE map_face(const FACE &f,
        const vector<int> &vtxmap,
        const vector<int> &txcmap,
        const vector<int> &normalmap) {
    FACE rf(f);
    switch (f.num) {
        case 4:
            rf.r4.v = ((rf.r4.v >= 0) ? vtxmap[rf.r4.v] : -1);
            rf.r4.t = ((rf.r4.t >= 0) ? txcmap[rf.r4.t] : -1);
            rf.r4.n = ((rf.r4.n >= 0) ? normalmap[rf.r4.n] : -1);
        case 3:
            rf.r3.v = ((rf.r3.v >= 0) ? vtxmap[rf.r3.v] : -1);
            rf.r3.t = ((rf.r3.t >= 0) ? txcmap[rf.r3.t] : -1);
            rf.r3.n = ((rf.r3.n >= 0) ? normalmap[rf.r3.n] : -1);
        case 2:
            rf.r2.v = ((rf.r2.v >= 0) ? vtxmap[rf.r2.v] : -1);
            rf.r2.t = ((rf.r2.t >= 0) ? txcmap[rf.r2.t] : -1);
            rf.r2.n = ((rf.r2.n >= 0) ? normalmap[rf.r2.n] : -1);
        case 1:
            rf.r1.v = ((rf.r1.v >= 0) ? vtxmap[rf.r1.v] : -1);
            rf.r1.t = ((rf.r1.t >= 0) ? txcmap[rf.r1.t] : -1);
            rf.r1.n = ((rf.r1.n >= 0) ? normalmap[rf.r1.n] : -1);
    }
    return rf;
}

string ObjGetMtl(FILE *obj, string objpath) {
    fseek(obj, 0, SEEK_END);
    int osize = ftell(obj);
    fseek(obj, 0, SEEK_SET);
    char *buf = (char *) malloc((osize + 1) * sizeof(char));
    char *str = (char *) malloc((osize + 1) * sizeof(char));

    string ret;
    while (ret.empty() && fgets(buf, osize, obj)) {
        if (buf[0] == '#' || buf[0] == 0) {
            continue;
        }
        if (1 == sscanf(buf, "mtllib %s", str)) {
            ret = str;
        }
    }
    free(buf);
    free(str);

    fseek(obj, 0, SEEK_SET);

    //Now, copy path part
    string::size_type p = objpath.rfind('/');
#if defined (_WIN32) && !defined (__CYGWIN__)
    if (p == string::npos) {
        p = objpath.rfind( '\\' );
    } else {
        string::size_type p2 = objpath.find( '\\', p+1 );
        if (p2 != string::npos) p = p2;
    }
#endif
    if (p != string::npos) {
        ret = objpath.substr(0, p + 1) + ret;
    }
    return ret;
}

void ObjToXMESH(FILE *obj, FILE *mtl, vector<XML> &xmllist, bool forcenormals) {
    bool flips = atoi(Converter::getNamedOption("flips").c_str()) != 0;
    bool flipt = atoi(Converter::getNamedOption("flipt").c_str()) != 0;
    bool flipn = atoi(Converter::getNamedOption("flipn").c_str()) != 0;
    bool flipx = atoi(Converter::getNamedOption("flipx").c_str()) != 0;
    bool flipy = atoi(Converter::getNamedOption("flipy").c_str()) != 0;
    bool flipz = atoi(Converter::getNamedOption("flipz").c_str()) != 0;
    bool noopt = atoi(Converter::getNamedOption("no-optimize").c_str()) != 0;

    fseek(obj, 0, SEEK_END);
    int osize = ftell(obj);
    fseek(obj, 0, SEEK_SET);
    fseek(mtl, 0, SEEK_END);
    {
        int msize = ftell(mtl);
        osize = osize > msize ? osize : msize;
    }
    fseek(mtl, 0, SEEK_SET);
    char *buf = (char *) malloc((osize + 1) * sizeof(char));
    char *str = (char *) malloc((osize + 1) * sizeof(char));
    char *str1 = (char *) malloc((osize + 1) * sizeof(char));
    char *str2 = (char *) malloc((osize + 1) * sizeof(char));
    char *str3 = (char *) malloc((osize + 1) * sizeof(char));
    char *str4 = (char *) malloc((osize + 1) * sizeof(char));
    map<string, MTL> mtls;
    mtls["default"] = MTL();
    MTL *cur = &mtls["default"];
    while (fgets(buf, osize, mtl)) {
        if (buf[0] == '#' || buf[0] == 0) {
            continue;
        }
        if (1 == sscanf(buf, "newmtl %s\n", str)) {
            mtls[str] = MTL();
            cur = &mtls[str];
            continue;
        }
        wordtoupper(buf);
        float tmpblend;
        int tmpint;
        if (1 == sscanf(buf, "BLEND %f\n", &tmpblend)) {
            if (tmpblend == 1) {
                cur->blend_src = ONE;
                cur->blend_dst = ONE;
            } else if (tmpblend == .5) {
                cur->blend_src = SRCALPHA;
                cur->blend_dst = INVSRCALPHA;
            } else {
                cur->blend_src = ONE;
                cur->blend_dst = ZERO;
            }
        }
        if (3 == sscanf(buf, "KA %f %f %f\n", &cur->ar, &cur->ag, &cur->ab)) {
            cur->aa = 1;
        }
        if (3 == sscanf(buf, "KS %f %f %f\n", &cur->sr, &cur->sg, &cur->sb)) {
            cur->sa = 1;
        }
        if (3 == sscanf(buf, "KD %f %f %f\n", &cur->dr, &cur->dg, &cur->db)) {
            cur->da = 1;
        }
        if (3 == sscanf(buf, "KE %f %f %f\n", &cur->er, &cur->eg, &cur->eb)) {
            cur->ea = 1;
        }
        if (1 == sscanf(buf, "NORMALS %f\n", &tmpblend)) {
            if (tmpblend != 0) {
                cur->usenormals = true;
            } else {
                cur->usenormals = false;
            }
        }
        if (1 == sscanf(buf, "MAP_REFLECTION %f\n", &tmpblend)) {
            if (tmpblend != 0) {
                cur->reflect = 1;
            } else {
                cur->reflect = 0;
            }
        }
        sscanf(buf, "NS %f\n", &cur->power);
        float floate, floatf, floatg;
        if (3 == sscanf(buf, "DETAIL_PLANE %f %f %f\n", &floate, &floatf, &floatg)) {
            cur->detailplanei.push_back(floate);
            cur->detailplanej.push_back(floatf);
            cur->detailplanek.push_back(floatg);
        }
        if (1 == sscanf(buf, "ILLUM %f\n", &floate)) {
            cur->er = floate;
            cur->eg = floate;
            cur->eb = floate;
        }
        if (1 == sscanf(buf, "MAP_KD %s\n", str)) {
            cur->textures.push_back(makeTextureHolder(str, 0));
        }
        if (1 == sscanf(buf, "MAP_KS %s\n", str)) {
            cur->textures.push_back(makeTextureHolder(str, 1));
        }
        if ((1 == sscanf(buf, "MAP_KA %s\n", str)) || (1 == sscanf(buf, "MAP_DAMAGE %s\n", str))) {
            cur->textures.push_back(makeTextureHolder(str, 2));
        }
        if (1 == sscanf(buf, "MAP_KE %s\n", str)) {
            cur->textures.push_back(makeTextureHolder(str, 3));
        }
        if (1 == sscanf(buf, "MAP_NORMAL %s\n", str)) {
            cur->textures.push_back(makeTextureHolder(str, 4));
        }
        if (1 == sscanf(buf, "MAP_DETAIL %s\n", str)) {
            cur->detail = makeTextureHolder(str, 0);
        }
        if (2 == sscanf(buf, "MAP_%d %s\n", &tmpint, str)) {
            cur->textures.push_back(makeTextureHolder(str, tmpint));
        }
        if (1 == sscanf(buf, "TECHNIQUE %s\n", str)) {
            cur->textures.push_back(makeTechniqueHolder(str));
        }
    }

    // Add phoney texture to textureless materials
    // (they often produce crashes)
    for (map<string, MTL>::iterator it = mtls.begin(); it != mtls.end(); ++it) {
        cur = &(it->second);
        if (cur->textures.empty()) {
            cur->textures.push_back(textureholder());
        }
    }
    bool changemat = false;

    vector<VTX> vtxlist;
    map<VTX, int> vtxmap;
    vector<int> vtxmap_ii;

    vector<TEX> txclist;
    map<TEX, int> txcmap;
    vector<int> txcmap_ii;

    map<NORMAL, int> normalmap;
    vector<int> normalmap_ii;
    vector<NORMAL> normallist;

    map<string, vector<FACE> > facelistlist;
    vector<FACE> facelist;

    map<string, string> mattexmap;
    string curmat;

    vector<pair<float, float> > tex;
    while (fgets(buf, osize, obj)) {
        if (buf[0] == '#' || buf[0] == 'g') {
            continue;
        }
        if (1 == sscanf(buf, "usemtl %s\n", str)) {
            curmat = str;
            if (mtls.find(curmat) == mtls.end()) {
                printf("WARNING: referenced material not found: %s\n", curmat.c_str());
            }
            if (changemat) {
                //append to facelistlist
                facelistlist[curmat].insert(facelistlist[curmat].end(), facelist.begin(), facelist.end());
                facelist.clear();
            }
            changemat = true;
            continue;
        }
        if (1 == sscanf(buf, "usemat %s\n", str)) {
            curmat = str;
            if (mtls.find(curmat) == mtls.end()) {
                printf("WARNING: referenced material not found: %s\n", curmat.c_str());
            }
            mtls[curmat].textures.clear();
            mtls[curmat].textures.push_back(makeTextureHolder(str, 0));
        }
        GFXVertex v;

        memset(&v, 0, sizeof(GFXVertex));
        if (3 == sscanf(buf, "v %f %f %f\n", &v.x, &v.y, &v.z)) {
            //xml.vertices.push_back(v);
            //xml.num_vertex_references.push_back(0);
            if (flipx) {
                v.x = -v.x;
            }
            if (flipy) {
                v.y = -v.y;
            }
            if (flipz) {
                v.z = -v.z;
            }
            VTX vv(v.x, v.y, v.z);
            map<VTX, int>::iterator vi = (noopt ? vtxmap.end() : vtxmap.find(vv));
            if (vi == vtxmap.end()) {
                vtxmap_ii.push_back(int(vtxlist.size()));
                vtxmap.insert(pair<VTX, int>(vv, int(vtxlist.size())));
                vtxlist.push_back(vv);
            } else {
                vtxmap_ii.push_back((*vi).second);
            }
            continue;
        }
        if (3 == sscanf(buf, "vn %f %f %f\n", &v.i, &v.j, &v.k)) {
            //Sharing is loussy in .obj files... so lets merge a little
            if (flipn) {
                v.i = -v.i, v.j = -v.j, v.k = -v.k;
            }
            if (flipx) {
                v.x = -v.i;
            }
            if (flipy) {
                v.y = -v.j;
            }
            if (flipz) {
                v.z = -v.k;
            }
            NORMAL n(v.i, v.j, v.k);
            map<NORMAL, int>::iterator mi = (noopt ? normalmap.end() : normalmap.find(n));
            if (mi == normalmap.end()) {
                normalmap_ii.push_back(int(normallist.size()));
                normalmap.insert(pair<NORMAL, int>(n, int(normallist.size())));
                normallist.push_back(n);
            } else {
                normalmap_ii.push_back((*mi).second);
            }
            continue;
        }
        if (2 == sscanf(buf, "vt %f %f\n", &v.s, &v.t)) {
            //tex.push_back(pair<float,float>(v.s,v.t));
            //Sharing is loussy in .obj files... so lets merge a little
            TEX t((flips ? (1.0f - v.s) : v.s), (flipt ? v.t : (1.0f - v.t)));
            map<TEX, int>::iterator mi = (noopt ? txcmap.end() : txcmap.find(t));
            if (mi == txcmap.end()) {
                txcmap_ii.push_back(int(txclist.size()));
                txcmap.insert(pair<TEX, int>(t, int(txclist.size())));
                txclist.push_back(t);
            } else {
                txcmap_ii.push_back((*mi).second);
            }
            continue;
        }
        int rv = sscanf(buf, "f %s %s %s %s %s\n", str, str1, str2, str3, str4);
        if (rv >= 2) {
            switch (rv) {
                case 5:
                    //fan - decomposed into tris. Sorry, but after fanification-stripification is done, everything will be allright again.
                {
                    vector<string> splitwhite = splitWhiteSpace(buf + 1);
                    for (vector<string>::size_type i = 2; i < splitwhite.size(); i++) {
                        facelist.push_back(map_face(FACE(parsePoly(splitwhite[0]), parsePoly(splitwhite[i - 1]),
                                        parsePoly(splitwhite[i - 2])),
                                vtxmap_ii,
                                txcmap_ii,
                                normalmap_ii));
                    }
                    break;
                }
                case 4:
                    facelist.push_back(map_face(FACE(parsePoly(str), parsePoly(str1), parsePoly(str2),
                            parsePoly(str3)), vtxmap_ii, txcmap_ii, normalmap_ii));
                    break;
                case 3:
                    facelist.push_back(map_face(FACE(parsePoly(str), parsePoly(str1),
                            parsePoly(str2)), vtxmap_ii, txcmap_ii, normalmap_ii));
                    break;
                case 2:
                    facelist.push_back(map_face(FACE(parsePoly(str), parsePoly(str1)),
                            vtxmap_ii,
                            txcmap_ii,
                            normalmap_ii));
                    break;
            }
        }
        continue;
    }
    //append to facelistlist
    facelistlist[curmat].insert(facelistlist[curmat].end(), facelist.begin(), facelist.end());
    facelist.clear();

    int textnum = 0;
    int totface = 0;
    int totindex = 0;
    for (map<string, vector<FACE> >::iterator it = facelistlist.begin(); it != facelistlist.end(); it++) {
        XML xml;

        string mat = (*it).first;

        xml.vertices.clear();
        xml.tris.clear();
        xml.quads.clear();
        xml.lines.clear();
        xml.linestrips.clear();
        xml.tristrips.clear();
        xml.quadstrips.clear();
        xml.trifans.clear();

        xml.material = mtls[mat];
        xml.textures = mtls[mat].textures;
        xml.detailtexture = mtls[mat].detail;
        for (vector<float>::size_type jjjj = 0; jjjj < mtls[mat].detailplanei.size(); ++jjjj) {
            Mesh_vec3f v;
            v.x = mtls[mat].detailplanei[jjjj];
            v.y = mtls[mat].detailplanej[jjjj];
            v.z = mtls[mat].detailplanek[jjjj];
            xml.detailplanes.push_back(Mesh_vec3f(v));
        }
        xml.blend_src = mtls[mat].blend_src;
        xml.blend_dst = mtls[mat].blend_dst;
        xml.reflect = mtls[mat].reflect;
        if (forcenormals) {
            mtls[mat].usenormals = true;
        }
        xml.usenormals = mtls[mat].usenormals;

        ObjToXML(xml, vtxlist, txclist, normallist, (*it).second);
        xmllist.push_back(xml);

        printf("%u_0: %u faces, %u vertices, %u lines, %u tris, %u quads, %u indices\n",
                textnum,
                (unsigned int) (*it).second.size(),
                (unsigned int) xml.vertices.size(),
                (unsigned int) xml.lines.size(),
                (unsigned int) xml.tris.size(),
                (unsigned int) xml.quads.size(),
                (unsigned int) (xml.tris.size() * 3 + xml.quads.size() * 4)
        );
        textnum++;
        totface += int((*it).second.size());
        totindex += int(xml.tris.size() * 3 + xml.quads.size() * 4);
    }
    printf("Total faces: %d\n", totface);
    printf("Total indices: %d\n", totindex);
}

void ObjToBFXM(FILE *obj, FILE *mtl, FILE *outputFile, bool forcenormals) {
    vector<XML> xmllist;

    ObjToXMESH(obj, mtl, xmllist, forcenormals);

    int textnum = 0;
    for (vector<XML>::iterator it = xmllist.begin(); it != xmllist.end(); ++it, ++textnum) {
        xmeshToBFXM(*it, outputFile, textnum == 0 ? 'c' : 'a', forcenormals);
    }
}

