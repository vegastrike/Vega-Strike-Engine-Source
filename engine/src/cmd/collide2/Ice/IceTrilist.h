///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a triangle container.
 *	\file		IceTrilist.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICETRILIST_H__
#define __ICETRILIST_H__

class ICEMATHS_API TriList : public Container {
public:
    // Constructor / Destructor
    TriList() {
    }

    ~TriList() {
    }

    inline_    uint32_t GetNbTriangles() const {
        return GetNbEntries() / 9;
    }

    inline_    Triangle *GetTriangles() const {
        return (Triangle *) GetEntries();
    }

    void AddTri(const Triangle &tri) {
        Add(tri.mVerts[0].x).Add(tri.mVerts[0].y).Add(tri.mVerts[0].z);
        Add(tri.mVerts[1].x).Add(tri.mVerts[1].y).Add(tri.mVerts[1].z);
        Add(tri.mVerts[2].x).Add(tri.mVerts[2].y).Add(tri.mVerts[2].z);
    }

    void AddTri(const Point &p0, const Point &p1, const Point &p2) {
        Add(p0.x).Add(p0.y).Add(p0.z);
        Add(p1.x).Add(p1.y).Add(p1.z);
        Add(p2.x).Add(p2.y).Add(p2.z);
    }
};

class ICEMATHS_API TriangleList : public Container {
public:
    // Constructor / Destructor
    TriangleList() {
    }

    ~TriangleList() {
    }

    inline_    uint32_t GetNbTriangles() const {
        return GetNbEntries() / 3;
    }

    inline_    IndexedTriangle *GetTriangles() const {
        return (IndexedTriangle *) GetEntries();
    }

    void AddTriangle(const IndexedTriangle &tri) {
        Add(tri.mVRef[0]).Add(tri.mVRef[1]).Add(tri.mVRef[2]);
    }

    void AddTriangle(uint32_t vref0, uint32_t vref1, uint32_t vref2) {
        Add(vref0).Add(vref1).Add(vref2);
    }
};

#endif //__ICETRILIST_H__
