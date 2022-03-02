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


#ifndef _GFXLIB_STRUCT
#define _GFXLIB_STRUCT
#include "gfx/vec.h"
#include "endianness.h"

#include <vector>
#include <iterator>
#include <functional>

#ifndef GFXBOOL
#define GFXBOOL unsigned char
#endif
#define GFXTRUE 1
#define GFXFALSE 0

const int INDEX_BYTE = sizeof(unsigned char);
const int INDEX_SHORT = sizeof(unsigned short);
const int INDEX_INT = sizeof(unsigned int);
const int CHANGE_MUTABLE = (sizeof(unsigned int) * 2);
const int CHANGE_CHANGE = (sizeof(unsigned int) * 4);
const int HAS_COLOR = (sizeof(unsigned int) * 8);
#define USE_DISPLAY_LISTS
const int HAS_INDEX = sizeof(unsigned char) | sizeof(unsigned short) | sizeof(unsigned int);

///Creates a Display list. 0 is returned if no memory is avail for a display list
extern int /*GFXDRVAPI*/ GFXCreateList();
///Ends the display list call.  Returns false if unsuccessful
extern GFXBOOL /*GFXDRVAPI*/ GFXEndList();
///Removes a display list from application memory
extern void /*GFXDRVAPI*/ GFXDeleteList(int list);

/// Vertex, Normal, Texture, and (deprecated) Environment Mapping T2F_N3F_V3F format
struct GFXVertex {
    //Texcoord
    float s;
    float t;

    //Normal
    float i;
    float j;
    float k;

    //Position
    float x;
    float y;
    float z;

    //Tangent
    float tx;
    float ty;
    float tz;
    float tw;

    GFXVertex() {
    }

    GFXVertex(const QVector &vert, const Vector &norm, float s, float t) {
        SetVertex(vert.Cast());
        SetNormal(norm);
        SetTexCoord(s, t);
    }

    GFXVertex(const Vector &vert, const Vector &norm, float s, float t) {
        SetVertex(vert);
        SetNormal(norm);
        SetTexCoord(s, t);
    }

    GFXVertex(float x, float y, float z, float i, float j, float k, float s, float t) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->i = i;
        this->j = j;
        this->k = k;
        this->s = s;
        this->t = t;
    }

    GFXVertex &SetTexCoord(float s, float t) {
        this->s = s;
        this->t = t;
        return *this;
    }

    GFXVertex &SetNormal(const Vector &norm) {
        i = norm.i;
        j = norm.j;
        k = norm.k;
        return *this;
    }

    GFXVertex &SetVertex(const Vector &vert) {
        x = vert.i;
        y = vert.j;
        z = vert.k;
        return *this;
    }

    GFXVertex &SetTangent(const Vector &tgt, float parity = 1.f) {
        tx = tgt.x;
        ty = tgt.y;
        tz = tgt.z;
        tw = parity;
        return *this;
    }

    Vector GetVertex() const {
        return Vector(x, y, z);
    }

    const Vector &GetConstVertex() const {
        return *((Vector *) &x);
    }

    Vector GetNormal() const {
        return Vector(i, j, k);
    }

    Vector GetPosition() const {
        return Vector(x, y, z);
    }

    Vector GetTangent() const {
        return Vector(tx, ty, tz);
    }

    float GetTangentParity() const {
        return tw;
    }
};

//Stores a color (or any 4 valued vector)
struct GFXColor {
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;

    GFXColor() {
    }

    GFXColor(const Vector &v, float a = 1.0) {
        this->r = v.i;
        this->g = v.j;
        this->b = v.k;
        this->a = a;
    }

    GFXColor(float r, float g, float b) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = 1.0;
    }

    GFXColor(float r, float g, float b, float a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    void netswap() {
        this->r = VSSwapHostFloatToLittle(this->r);
        this->g = VSSwapHostFloatToLittle(this->g);
        this->b = VSSwapHostFloatToLittle(this->b);
        this->a = VSSwapHostFloatToLittle(this->a);
    }

    GFXColor clamp() const {
        return GFXColor(((r < 0) ? 0 : ((r > 1) ? 1 : r)), ((g < 0) ? 0 : ((g > 1) ? 1 : g)),
                ((b < 0) ? 0 : ((b > 1) ? 1 : b)), ((a < 0) ? 0 : ((a > 1) ? 1 : a)));
    }
};

inline GFXColor operator*(float s, const GFXColor &c) {
    return GFXColor(s * c.r, s * c.g, s * c.b, s * c.a);
}

inline GFXColor operator*(const GFXColor &c, float s) {
    return GFXColor(s * c.r, s * c.g, s * c.b, s * c.a);
}

inline GFXColor operator+(const GFXColor &c0, const GFXColor &c1) {
    return GFXColor(c0.r + c1.r, c0.g + c1.g, c0.b + c1.b, c0.a + c1.a);
}

inline GFXColor operator-(const GFXColor &c0, const GFXColor &c1) {
    return GFXColor(c0.r - c1.r, c0.g - c1.g, c0.b - c1.b, c0.a - c1.a);
}

inline GFXColor operator-(GFXColor &c0, const GFXColor &c1) {
    return GFXColor(c0.r - c1.r, c0.g - c1.g, c0.b - c1.b, c0.a - c1.a);
}

inline bool operator==(const GFXColor &c0, const GFXColor &c1) {
    return (c0.r == c1.r) && (c0.g == c1.g) && (c0.b == c1.b) && (c0.a == c1.a);
}

inline GFXColor colLerp(GFXColor a, GFXColor b, float bweight) {
    float aweight = 1.0f - bweight;
    return GFXColor(a.r * aweight + b.r * bweight,
            a.g * aweight + b.g * bweight,
            a.b * aweight + b.b * bweight,
            a.a * aweight + b.a * bweight);
}

///This vertex is used for the interleaved array argument for color based arrays T2F_C4F_N3F_V3F
struct GFXColorVertex {
    //Texcoord
    float s;
    float t;

    //Color
    float r;
    float g;
    float b;
    float a;

    //Normal
    float i;
    float j;
    float k;

    //Position
    float x;
    float y;
    float z;

    //Tangent
    float tx;
    float ty;
    float tz;
    float tw;

    GFXColorVertex() {
        this->x = 0.f;
        this->y = 0.f;
        this->z = 0.f;
        this->i = 0.f;
        this->j = 0.f;
        this->k = 0.f;
        this->r = 0.f;
        this->g = 0.f;
        this->b = 0.f;
        this->a = 0.f;
        this->s = 0.f;
        this->t = 0.f;
        this->tx = 0.f;
        this->ty = 0.f;
        this->tz = 0.f;
        this->tw = 1.f;
    }

    GFXColorVertex(const Vector &vert, const Vector &norm, const GFXColor &rgba, float s, float t) {
        SetVertex(vert);
        SetNormal(norm);
        SetColor(rgba);
        SetTexCoord(s, t);
    }

    GFXColorVertex(const Vector &vert, const GFXColor &rgba, float s, float t) {
        SetVertex(vert);
        SetColor(rgba);
        SetTexCoord(s, t);
    }

    GFXColorVertex(const Vector &vert, const GFXColor &rgba) {
        SetVertex(vert);
        SetColor(rgba);
    }

    GFXColorVertex(float x,
            float y,
            float z,
            float i,
            float j,
            float k,
            float r,
            float g,
            float b,
            float a,
            float s,
            float t) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->i = i;
        this->j = j;
        this->k = k;
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
        this->s = s;
        this->t = t;
    }

    GFXColorVertex &SetTexCoord(float s, float t) {
        this->s = s;
        this->t = t;
        return *this;
    }

    GFXColorVertex &SetNormal(const Vector &norm) {
        i = norm.i;
        j = norm.j;
        k = norm.k;
        return *this;
    }

    GFXColorVertex &SetVertex(const Vector &vert) {
        x = vert.i;
        y = vert.j;
        z = vert.k;
        return *this;
    }

    GFXColorVertex &SetColor(const GFXColor &col) {
        r = col.r;
        g = col.g;
        b = col.b;
        a = col.a;
        return *this;
    }

    GFXColorVertex &SetTangent(const Vector &tgt, float parity = 1.f) {
        tx = tgt.x;
        ty = tgt.y;
        tz = tgt.z;
        tw = parity;
        return *this;
    }

    Vector GetNormal() const {
        return Vector(i, j, k);
    }

    Vector GetPosition() const {
        return Vector(x, y, z);
    }

    void SetVtx(const GFXVertex &vv) {
        s = vv.s;
        t = vv.t;
        i = vv.i;
        j = vv.j;
        k = vv.k;
        x = vv.x;
        y = vv.y;
        z = vv.z;
    }

    Vector GetTangent() const {
        return Vector(tx, ty, tz);
    }

    float GetTangentParity() const {
        return tw;
    }
};

template<typename ELEM=float, int VSIZE = 3, int NSIZE = 0, int CSIZE = 0, int TSIZE0 = 0, int TSIZE1 = 0>
class VertexBuilder {

public:
    typedef ELEM ElementType;
    typedef std::vector<ElementType> BufferType;
    typedef typename BufferType::iterator iterator;
    typedef typename BufferType::const_iterator const_iterator;
    typedef typename BufferType::size_type size_type;

    static const int ESIZE = VSIZE + NSIZE + CSIZE + TSIZE0 + TSIZE1;

    class back_insert_iterator {
        friend class VertexBuilder<ELEM, VSIZE, NSIZE, CSIZE, TSIZE0, TSIZE1>;

        BufferType &buffer;
        typename BufferType::size_type where;

        back_insert_iterator(BufferType &_buffer, typename BufferType::size_type _where)
                : buffer(_buffer), where(_where) {
        }

        back_insert_iterator(const back_insert_iterator &other) : buffer(other.buffer), where(other.where) {
        }

        template<typename IT>
        void _set_at(const GFXVertex &vtx, IT where) {
            if (VSIZE >= 1) {
                *where = vtx.x;
                ++where;
            }
            if (VSIZE >= 2) {
                *where = vtx.y;
                ++where;
            }
            if (VSIZE >= 3) {
                *where = vtx.z;
                ++where;
            }
            if (NSIZE >= 1) {
                *where = vtx.i;
                ++where;
            }
            if (NSIZE >= 2) {
                *where = vtx.j;
                ++where;
            }
            if (NSIZE >= 3) {
                *where = vtx.k;
                ++where;
            }
            if (CSIZE >= 1) {
                *where = 1.f;
                ++where;
            }
            if (CSIZE >= 2) {
                *where = 1.f;
                ++where;
            }
            if (CSIZE >= 3) {
                *where = 1.f;
                ++where;
            }
            if (TSIZE0 >= 1) {
                *where = vtx.s;
                ++where;
            }
            if (TSIZE0 >= 2) {
                *where = vtx.t;
                ++where;
            }
            if (TSIZE0 >= 3) {
                *where = vtx.s;
                ++where;
            }
            if (TSIZE0 >= 4) {
                *where = vtx.t;
                ++where;
            }
            if (TSIZE1 >= 1) {
                *where = vtx.tx;
                ++where;
            }
            if (TSIZE1 >= 2) {
                *where = vtx.ty;
                ++where;
            }
            if (TSIZE1 >= 3) {
                *where = vtx.tz;
                ++where;
            }
            if (TSIZE1 >= 4) {
                *where = vtx.tw;
                ++where;
            }
        }

        template<typename IT>
        void _set_at(const GFXColorVertex &vtx, IT where) {
            if (VSIZE >= 1) {
                *where = vtx.x;
                ++where;
            }
            if (VSIZE >= 2) {
                *where = vtx.y;
                ++where;
            }
            if (VSIZE >= 3) {
                *where = vtx.z;
                ++where;
            }
            if (NSIZE >= 1) {
                *where = vtx.i;
                ++where;
            }
            if (NSIZE >= 2) {
                *where = vtx.j;
                ++where;
            }
            if (NSIZE >= 3) {
                *where = vtx.k;
                ++where;
            }
            if (CSIZE >= 1) {
                *where = vtx.r;
                ++where;
            }
            if (CSIZE >= 2) {
                *where = vtx.g;
                ++where;
            }
            if (CSIZE >= 3) {
                *where = vtx.b;
                ++where;
            }
            if (CSIZE >= 4) {
                *where = vtx.a;
                ++where;
            }
            if (TSIZE0 >= 1) {
                *where = vtx.s;
                ++where;
            }
            if (TSIZE0 >= 2) {
                *where = vtx.t;
                ++where;
            }
            if (TSIZE0 >= 3) {
                *where = vtx.s;
                ++where;
            }
            if (TSIZE0 >= 4) {
                *where = vtx.t;
                ++where;
            }
            if (TSIZE1 >= 1) {
                *where = vtx.tx;
                ++where;
            }
            if (TSIZE1 >= 2) {
                *where = vtx.ty;
                ++where;
            }
            if (TSIZE1 >= 3) {
                *where = vtx.tz;
                ++where;
            }
            if (TSIZE1 >= 4) {
                *where = vtx.tw;
                ++where;
            }
        }

    public:
        back_insert_iterator &operator++(int) {
            where += ESIZE;
            return this;
        }

        back_insert_iterator operator++() {
            back_insert_iterator rv(this);
            ++*this;
            return rv;
        }

        back_insert_iterator &operator*() {
            return *this;
        }

        const GFXVertex &operator=(const GFXVertex &vtx) {
            if ((where + ESIZE) > buffer.size()) {
                buffer.resize(where + ESIZE);
            }
            _set_at(vtx, buffer.begin() + where);
            return vtx;
        }

        const GFXColorVertex &operator=(const GFXColorVertex &vtx) {
            if ((where + ESIZE) > buffer.size()) {
                buffer.resize(where + ESIZE);
            }
            _set_at(vtx, buffer.begin() + where);
            return vtx;
        }
    };

private:
    BufferType buffer;

public:
    VertexBuilder() {
    };

    explicit VertexBuilder(size_type size) {
        reserve(size);
    }

    const ElementType *buffer_pointer() const {
        return &buffer[0];
    }

    const_iterator begin() const {
        return buffer.begin();
    }

    const_iterator end() const {
        return buffer.end();
    }

    iterator begin() {
        return buffer.begin();
    }

    iterator end() {
        return buffer.end();
    }

    back_insert_iterator insertor() {
        return back_insert_iterator(buffer, buffer.size());
    }

    size_type size() const {
        return buffer.size() / ESIZE;
    }

    void clear() {
        buffer.clear();
    }

    void reserve(size_type size) {
        buffer.reserve(size * ESIZE);
    }

    void insert(const GFXVertex &vtx) {
        *(insertor()) = vtx;
    }

    void insert(const GFXColorVertex &vtx) {
        *(insertor()) = vtx;
    }

    void insert(const QVector &v) {
        insert(v.x, v.y, v.z);
    }

    void insert(const Vector &v) {
        insert(v.x, v.y, v.z);
    }

    void insert(float x, float y, float z) {
        GFXVertex vtx;
        vtx.x = x;
        vtx.y = y;
        vtx.z = z;
        insert(vtx);
    }

    void insert(float x, float y, float z, const GFXColor &color) {
        GFXColorVertex vtx;
        vtx.x = x;
        vtx.y = y;
        vtx.z = z;
        vtx.r = color.r;
        vtx.g = color.g;
        vtx.b = color.b;
        vtx.a = color.a;
        insert(vtx);
    }

    void insert(float x, float y, float z, float s, float t) {
        GFXVertex vtx;
        vtx.x = x;
        vtx.y = y;
        vtx.z = z;
        vtx.s = s;
        vtx.t = t;
        insert(vtx);
    }
};

///important ATTENUATE const STAYS AT ONE...for w compat.
enum LIGHT_TARGET {
    ATTENUATE = 1,
    DIFFUSE = 2,
    SPECULAR = 4,
    AMBIENT = 8,
    POSITION = 16,
    EMISSION = 32
};

///Holds all information for a single light object
class GFXLight {
public:
///physical GL light its saved in
    int target;
///last is w for positional, otherwise 3 for spec
    int options;
    float vect[3];
    float diffuse[4];
    float specular[4];
    float ambient[4];
    float attenuate[3];
    float direction[3];
    float exp;
    float cutoff;
    float size;
    float occlusion;

public:
    GFXLight() {
        //vect[0]=vect[1]=vect[2]=vect[3]=0;
        vect[0] = vect[1] = vect[2] = 0;
        attenuate[0] = 1;
        attenuate[1] = attenuate[2] = 0;
        diffuse[0] = diffuse[1] = diffuse[2] = 0; //openGL defaults
        specular[0] = specular[1] = specular[2] = 0; //openGL defaults
        ambient[0] = ambient[1] = ambient[2] = 0;
        diffuse[3] = specular[3] = ambient[3] = 1;
        target = -1; //physical GL light its saved in

        direction[0] = direction[1] = direction[2] = 0.0;
        exp = 0.0f;
        cutoff = 180.0f;
        size = 0.0f;
        occlusion = 1.f;
        options = 0;
        apply_attenuate(attenuated());
    }

    GFXLight(const bool enabled, const GFXColor &vect,
            const GFXColor &diffuse = GFXColor(0, 0, 0, 1),
            const GFXColor &specular = GFXColor(0, 0, 0, 1),
            const GFXColor &ambient = GFXColor(0, 0, 0, 1),
            const GFXColor &attenuate = GFXColor(1, 0, 0),
            const GFXColor &direction = GFXColor(0, 0, 0),
            float exp = 0.0f,
            float cutoff = 180.0f,
            float size = 0.0f);

    GFXLight(const GFXLight &other) {
        vect[0] = other.vect[0];
        vect[1] = other.vect[1];
        vect[2] = other.vect[2];
        attenuate[0] = other.attenuate[0];
        attenuate[1] = other.attenuate[1];
        attenuate[2] = other.attenuate[2];
        diffuse[0] = other.diffuse[0];
        diffuse[1] = other.diffuse[1];
        diffuse[2] = other.diffuse[2];
        diffuse[3] = other.diffuse[3];
        specular[0] = other.specular[0];
        specular[1] = other.specular[1];
        specular[2] = other.specular[2];
        specular[3] = other.specular[3];
        ambient[0] = other.ambient[0];
        ambient[1] = other.ambient[1];
        ambient[2] = other.ambient[2];
        ambient[3] = other.ambient[3];
        target = other.target;
        direction[0] = other.direction[0];
        direction[1] = other.direction[1];
        direction[2] = other.direction[2];
        exp = other.exp;
        cutoff = other.cutoff;
        size = other.size;
        occlusion = other.occlusion;
        options = other.options;
    };

    void SetProperties(enum LIGHT_TARGET, const GFXColor &color);
    GFXColor GetProperties(enum LIGHT_TARGET) const;

    void setSize(float size_) {
        size = size_;
    }

    float getSize() const {
        return size;
    }

    Vector getPosition() const {
        return Vector(vect[0], vect[1], vect[2]);
    }

    void disable();
    void enable();
    bool attenuated() const;
    void apply_attenuate(bool attenuated);
};
///Contains 4 texture coordinates (deprecated)
struct GFXTVertex //transformed vertex
{
    float x;
    float y;
    float z;
    /// reciprocal homogenous w
    float rhw;
    int diffuse;
    int specular;
    float s, t;
    float u, v;
};

struct GFXLightLocal {
    GFXLight ligh;
    bool islocal;
};

enum POLYTYPE {
    GFXTRI,
    GFXQUAD,
    GFXLINE,
    GFXTRISTRIP,
    GFXQUADSTRIP,
    GFXTRIFAN,
    GFXLINESTRIP,
    GFXPOLY,
    GFXPOINT
};

enum POLYMODE {
    GFXPOINTMODE,
    GFXLINEMODE,
    GFXFILLMODE
};

enum POLYFACE {
    GFXFRONT,
    GFXBACK,
    GFXFRONTANDBACK
};

/**
 *   Meant to be a huge list of individual quads (like light maps)
 *   that need to be resizable, etc all to be drawn at once.... nice on GL :-)
 */
class /*GFXDRVAPI*/ GFXQuadList {
///Num vertices currently _allocated_ on quad list
    int numVertices;
///Number of quads to be drawn packed first numQuads*4 vertices
    int numQuads;
///Assignments to packed data for quad modification
    int *quadassignments;
///all numVertices allocated vertices and color
    union VCDAT {
        GFXVertex *vertices;
        GFXColorVertex *colors;
    }
            data;
///Is color in this quad list
    GFXBOOL isColor;
///number of "dirty" quads, hence gaps in quadassignments that must be assigned before more are allocated
    int Dirty;
public:
///Creates an initial Quad List
    GFXQuadList(GFXBOOL color = GFXFALSE);
///Trashes given quad list
    ~GFXQuadList();
///Draws all quads contained in quad list
    void Draw();
///Adds quad to quad list, an integer indicating number that should be used to henceforth Mod it or delete it
    int AddQuad(const GFXVertex *vertices, const GFXColorVertex *colors = 0);
///Removes quad from Quad list
    void DelQuad(int which);
///modifies quad in quad list to contain new vertices and color information
    void ModQuad(int which, const GFXVertex *vertices, float alpha = -1);
    void ModQuad(int which, const GFXColorVertex *vertices);
};

/**
 * A vertex list is a list of any conglomeration of POLY TYPES.
 * It is held for storage in an array of GFXVertex but attempts
 * to use a display list to hold information if possible
 */
class /*GFXDRVAPI*/ GFXVertexList {
    friend class GFXSphereVertexList;
protected:
///Num vertices allocated
    const GFXVertex *GetVertex(int index) const;
    const GFXColorVertex *GetColorVertex(int index) const;
    int numVertices{};
///Vertices and colors stored
    union VDAT {
        ///The data either does not have color data
        GFXVertex *vertices;
        ///Or has color data
        GFXColorVertex *colors;

        VDAT() : vertices(0) {
        };
    }
            data;
    union INDEX {
        unsigned char *b; //stride 1
        unsigned short *s; //stride 2
        unsigned int *i; //stride 4
        INDEX() : i(0) {
        };
    }
            index;
///Array of modes that vertices will be drawn with
    enum POLYTYPE *mode{};
    bool unique_mode{}; //See Draw()
///Display list number if list is indeed active. 0 otherwise
    int display_list{};
    unsigned int *vbo_elements{};
    int vbo_data{};
///number of different mode, drawn lists
    int numlists{};
/**
 * number of vertices in each individual mode.
 * 2 triangles 3 quads and 2 lines would be {6,12,4} as the offsets
 */
    int *offsets{};
///If vertex list has been mutated since last draw.  Low 3 bits store the stride of the index list (if avail). another bit for if color is presnet
    char changed{};
///copies nonindexed vertices to dst vertex array
    static void VtxCopy(GFXVertexList *thus, GFXVertex *dst, int offset, int howmany);
///Copies nonindex colored vertices to dst vertex array
    static void ColVtxCopy(GFXVertexList *thus, GFXVertex *dst, int offset, int howmany);
///Copies indexed colored vertices to dst vertex array
    static void ColIndVtxCopy(GFXVertexList *thus, GFXVertex *dst, int offset, int howmany);
///Copies indexed vertices to dst vertex array
    static void IndVtxCopy(GFXVertexList *thus, GFXVertex *dst, int offset, int howmany);
///Init function (call from construtor)
    void Init(enum POLYTYPE *poly,
            int numVertices,
            const GFXVertex *vert,
            const GFXColorVertex *colors,
            int numlists,
            int *offsets,
            bool Mutable,
            unsigned int *indices);
///Propagates modifications to the display list
public:
    void RefreshDisplayList();
protected:
    virtual void Draw(enum POLYTYPE *poly, const INDEX index, const int numLists, const int *offsets);
    void RenormalizeNormals();
    GFXVertexList();
public:
///creates a vertex list with 1 polytype and a given number of vertices
    inline GFXVertexList(enum POLYTYPE poly,
            int numVertices,
            const GFXVertex *vertices,
            int numindices,
            bool Mutable = false,
            unsigned int *index = 0) {
        Init(&poly, numVertices, vertices, 0, 1, &numindices, Mutable, index);
    }

///Creates a vertex list with an arbitrary number of poly types and given vertices, num list and offsets (see above)
    inline GFXVertexList(enum POLYTYPE *poly,
            int numVertices,
            const GFXVertex *vertices,
            int numlists,
            int *offsets,
            bool Mutable = false,
            unsigned int *index = 0) {
        Init(poly, numVertices, vertices, 0, numlists, offsets, Mutable, index);
    }

///Creates a vertex list with 1 poly type and color information to boot
    inline GFXVertexList(enum POLYTYPE poly,
            int numVertices,
            const GFXColorVertex *colors,
            int numindices,
            bool Mutable = false,
            unsigned int *index = 0) {
        Init(&poly, numVertices, 0, colors, 1, &numindices, Mutable, index);
    }

///Creates a vertex list with an arbitrary number of poly types and color
    inline GFXVertexList(enum POLYTYPE *poly,
            int numVertices,
            const GFXColorVertex *colors,
            int numlists,
            int *offsets,
            bool Mutable = false,
            unsigned int *index = 0) {
        Init(poly, numVertices, 0, colors, numlists, offsets, Mutable, index);
    }

    virtual ~GFXVertexList();
///Returns number of Triangles in vertex list (counts tri strips)
    virtual int numTris() const;
///Returns number of Quads in vertex list (counts quad strips)
    virtual int numQuads() const;
///Looks up the index in the appropriate short, char or int array
    unsigned int GetIndex(int offset) const;
    POLYTYPE *GetPolyType() const;
    int *GetOffsets() const;
    int GetNumLists() const;
    bool hasColor() const;

    int GetNumVertices() const {
        return numVertices;
    }

    virtual VDAT *Map(bool read, bool write);
    void UnMap();
///Returns the array of vertices to be mutated
    virtual VDAT *BeginMutate(int offset);
///Ends mutation and refreshes display list
    virtual void EndMutate(int newsize = 0);
///Loads the draw state (what is active) of a given vlist for mass drawing
    void LoadDrawState();
///Specifies array pointers and loads the draw state of a given vlist for mass drawing
    virtual void BeginDrawState(GFXBOOL lock = GFXTRUE);
///Draws a single copy of the mass-loaded vlist
    virtual void Draw();
    void Draw(enum POLYTYPE poly, int numV);
    void Draw(enum POLYTYPE poly, int numV, unsigned char *index);
    void Draw(enum POLYTYPE poly, int numV, unsigned short *index);
    void Draw(enum POLYTYPE poly, int numV, unsigned int *index);
///Loads draw state and prepares to draw only once
    void DrawOnce();
    virtual void EndDrawState(GFXBOOL lock = GFXTRUE);
///returns a packed vertex list with number of polys and number of tries to passed in arguments. Useful for getting vertex info from a mesh
    virtual void GetPolys(GFXVertex **vert, int *numPolys, int *numTris);
};

class /*GFXDRVAPI*/ GFXSphereVertexList : public GFXVertexList {
///Num vertices allocated
protected:
    float radius;
    GFXVertexList *sphere;
    virtual void Draw(enum POLYTYPE *poly, const INDEX index, const int numLists, const int *offsets);
public:
///creates a vertex list with 1 polytype and a given number of vertices
    GFXSphereVertexList(float radius, int detail, bool insideout, bool reverse_normals);
    ~GFXSphereVertexList();

///Returns the array of vertices to be mutated
    virtual VDAT *BeginMutate(int offset);
///Ends mutation and refreshes display list
    virtual void EndMutate(int newsize = 0);
///Loads the draw state (what is active) of a given vlist for mass drawing
///Specifies array pointers and loads the draw state of a given vlist for mass drawing
    virtual void BeginDrawState(GFXBOOL lock = GFXTRUE);
///Draws a single copy of the mass-loaded vlist
    virtual void Draw();
    virtual void EndDrawState(GFXBOOL lock = GFXTRUE);
///returns a packed vertex list with number of polys and number of tries to passed in arguments. Useful for getting vertex info from a mesh
    virtual void GetPolys(GFXVertex **vert, int *numPolys, int *numTris);
///generates procedural planetdata to the actual detaillevel with the "plasma method"
    virtual void ProceduralModification();
};

/**
 * Stores the Draw Context that a vertex list might be drawn with.
 * Especially useful for mass queued drawing (load matrix, draw... )
 */

/**
 * holds all parameters for materials
 */
struct GFXMaterial {
    /// ambient rgba, if you don't like these things, ask me to rename them
    float ar;
    float ag;
    float ab;
    float aa;
    /// diffuse rgba
    float dr;
    float dg;
    float db;
    float da;
    /// specular rgba
    float sr;
    float sg;
    float sb;
    float sa;
    /// emissive rgba
    float er;
    float eg;
    float eb;
    float ea;
    /// specular power
    float power;
};

inline void setMaterialAmbient(GFXMaterial &mat, float r, float g, float b, float a) {
    mat.ar = r;
    mat.ag = g;
    mat.ab = b;
    mat.aa = a;
}

inline void setMaterialDiffuse(GFXMaterial &mat, float r, float g, float b, float a) {
    mat.dr = r;
    mat.dg = g;
    mat.db = b;
    mat.da = a;
}

inline void setMaterialSpecular(GFXMaterial &mat, float r, float g, float b, float a) {
    mat.sr = r;
    mat.sg = g;
    mat.sb = b;
    mat.sa = a;
}

inline void setMaterialEmissive(GFXMaterial &mat, float r, float g, float b, float a) {
    mat.er = r;
    mat.eg = g;
    mat.eb = b;
    mat.ea = a;
}

inline void setMaterialAmbient(GFXMaterial &mat, float rgba) {
    mat.ar = mat.ag = mat.ab = mat.aa = rgba;
}

inline void setMaterialDiffuse(GFXMaterial &mat, float rgba) {
    mat.dr = mat.dg = mat.db = mat.da = rgba;
}

inline void setMaterialSpecular(GFXMaterial &mat, float rgba) {
    mat.sr = mat.sg = mat.sb = mat.sa = rgba;
}

inline void setMaterialEmissive(GFXMaterial &mat, float rgba) {
    mat.er = mat.eg = mat.eb = mat.ea = rgba;
}

inline void setMaterialAmbient(GFXMaterial &mat, const GFXColor &col) {
    mat.ar = col.r;
    mat.ag = col.g;
    mat.ab = col.b;
    mat.aa = col.a;
}

inline void setMaterialDiffuse(GFXMaterial &mat, const GFXColor &col) {
    mat.dr = col.r;
    mat.dg = col.g;
    mat.db = col.b;
    mat.da = col.a;
}

inline void setMaterialSpecular(GFXMaterial &mat, const GFXColor &col) {
    mat.sr = col.r;
    mat.sg = col.g;
    mat.sb = col.b;
    mat.sa = col.a;
}

inline void setMaterialEmissive(GFXMaterial &mat, const GFXColor &col) {
    mat.er = col.r;
    mat.eg = col.g;
    mat.eb = col.b;
    mat.ea = col.a;
}

inline GFXColor getMaterialAmbient(const GFXMaterial &mat) {
    return GFXColor(mat.ar, mat.ag, mat.ab, mat.aa);
}

inline GFXColor getMaterialDiffuse(const GFXMaterial &mat) {
    return GFXColor(mat.dr, mat.dg, mat.db, mat.da);
}

inline GFXColor getMaterialSpecular(const GFXMaterial &mat) {
    return GFXColor(mat.sr, mat.sg, mat.sb, mat.sa);
}

inline GFXColor getMaterialEmissive(const GFXMaterial &mat) {
    return GFXColor(mat.er, mat.eg, mat.eb, mat.ea);
}

//Textures may only be cube maps, Texture1d, Texture2d or Texture3d
enum TEXTURE_TARGET {
    TEXTURE1D,
    TEXTURE2D,
    TEXTURE3D,
    CUBEMAP,
    TEXTURERECT
};
///Textures may only be cube maps, Texture1d, Texture2d or Texture3d
enum TEXTURE_IMAGE_TARGET {
    TEXTURE_1D,
    TEXTURE_2D,
    TEXTURE_3D,
    CUBEMAP_POSITIVE_X,
    CUBEMAP_NEGATIVE_X,
    CUBEMAP_POSITIVE_Y,
    CUBEMAP_NEGATIVE_Y,
    CUBEMAP_POSITIVE_Z,
    CUBEMAP_NEGATIVE_Z,
    TEXTURE_RECTANGLE
};

/**
 * Unlike OpenGL, 3 matrices are saved, the Model, the View and the Projection
 * Consistency is maintained through pushing the rotation part of view
 * onto projection matrix
 */
enum MATRIXMODE {
    MODEL,
    PROJECTION,
    VIEW
};

enum TEXTUREFORMAT {
    DUMMY = 0,
    PALETTE8 = 1,
    RGB16 = 2,
    RGBA16 = 3,
    RGB24 = 4,
    RGBA32 = 5,
    RGB32 = 6,
    DXT1 = 7,
    DXT1RGBA = 8,
    DXT3 = 9,
    DXT5 = 10,
    PNGPALETTE8 = 11,
    PNGRGB24 = 12,
    PNGRGBA32 = 13
};
/**
 * The following state may be activated/deactivated
 * LIGHTING, DEPTHTEST,DEPTHWRITE, TEXTURE0, and TEXTURE1.  Future support
 * For arbitrary number of texture units should be added FIXME
 */
enum STATE {
    LIGHTING,
    DEPTHTEST,
    DEPTHWRITE,
    COLORWRITE,
    TEXTURE0,
    TEXTURE1,
    CULLFACE,
    SMOOTH,
    STENCIL
};

enum CLIPSTATE {
    GFX_NOT_VISIBLE,
    GFX_PARTIALLY_VISIBLE,
    GFX_TOTALLY_VISIBLE
};

/**
 * Address modes various textures may have
 */
enum ADDRESSMODE {
    WRAP,
    MIRROR,
    CLAMP,
    BORDER,
    DEFAULT_ADDRESS_MODE
};

enum FOGMODE {
    FOG_OFF,
    FOG_EXP,
    FOG_EXP2,
    FOG_LINEAR
};
/**
 * Blend functions that a blendmode may have Not all work for all systems
 */
enum BLENDFUNC {
    ZERO = 1,
    ONE = 2,
    SRCCOLOR = 3,
    INVSRCCOLOR = 4,
    SRCALPHA = 5,
    INVSRCALPHA = 6,
    DESTALPHA = 7,
    INVDESTALPHA = 8,
    DESTCOLOR = 9,
    INVDESTCOLOR = 10,
    SRCALPHASAT = 11,
    CONSTALPHA = 12,
    INVCONSTALPHA = 13,
    CONSTCOLOR = 14,
    INVCONSTCOLOR = 15
};

/**
 * Different types of filtering.
 * Bilinear and mipmap are independent params (hence powers of two)
 */
enum FILTER {
    NEAREST = 0x0,
    BILINEAR = 0x1,
    MIPMAP = 0x2,
    TRILINEAR = 0x4
};

/**
 * Used for depth and alpha comparisons
 */
enum DEPTHFUNC {
    NEVER, LESS, EQUAL, LEQUAL, GREATER, NEQUAL, GEQUAL, ALWAYS
};

/**
 * Used for stencil writes
 */
enum STENCILOP {
    KEEP, CLEAR, REPLACE, INCR, DECR, INVERT
};

///Pick data structures
struct PickData {
    int name;
    int zmin;
    int zmax;

    PickData(int name, int zmin, int zmax) : name(name), zmin(zmin), zmax(zmax) {
    }
};

#endif

