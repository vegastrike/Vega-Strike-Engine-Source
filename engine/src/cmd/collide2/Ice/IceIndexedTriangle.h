///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a handy indexed triangle class.
 *	\file		IceIndexedTriangle.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
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
#ifndef __ICEINDEXEDTRIANGLE_H__
#define __ICEINDEXEDTRIANGLE_H__

// An indexed triangle class.
class IndexedTriangle {
public:
    //! Constructor
    inline_ IndexedTriangle() {
    }
    //! Constructor
    inline_ IndexedTriangle(uint32_t r0, uint32_t r1, uint32_t r2) {
        mVRef[0] = r0;
        mVRef[1] = r1;
        mVRef[2] = r2;
    }
    //! Copy constructor
    inline_ IndexedTriangle(const IndexedTriangle &triangle) {
        mVRef[0] = triangle.mVRef[0];
        mVRef[1] = triangle.mVRef[1];
        mVRef[2] = triangle.mVRef[2];
    }
    //! Destructor
    inline_                    ~IndexedTriangle() {
    }

    //! Vertex-references
    uint32_t mVRef[3];

    // Methods
    void Flip();
    float Area(const Point *verts) const;
    float Perimeter(const Point *verts) const;
    float Compacity(const Point *verts) const;
    void Normal(const Point *verts, Point &normal) const;
    void DenormalizedNormal(const Point *verts, Point &normal) const;
    void Center(const Point *verts, Point &center) const;
    void CenteredNormal(const Point *verts, Point &normal) const;
    void RandomPoint(const Point *verts, Point &random) const;
    bool IsVisible(const Point *verts, const Point &source) const;
    bool BackfaceCulling(const Point *verts, const Point &source) const;
    float ComputeOcclusionPotential(const Point *verts, const Point &view) const;
    bool ReplaceVertex(uint32_t oldref, uint32_t newref);
    bool IsDegenerate() const;
    bool HasVertex(uint32_t ref) const;
    bool HasVertex(uint32_t ref, uint32_t *index) const;
    uint8_t FindEdge(uint32_t vref0, uint32_t vref1) const;
    uint32_t OppositeVertex(uint32_t vref0, uint32_t vref1) const;

    inline_    uint32_t OppositeVertex(uint8_t edgenb) const {
        return mVRef[2 - edgenb];
    }

    void GetVRefs(uint8_t edgenb, uint32_t &vref0, uint32_t &vref1, uint32_t &vref2) const;
    float MinEdgeLength(const Point *verts) const;
    float MaxEdgeLength(const Point *verts) const;
    void ComputePoint(const Point *verts, float u, float v, Point &pt, uint32_t *nearvtx = nullptr) const;
    float Angle(const IndexedTriangle &tri, const Point *verts) const;

    inline_    Plane PlaneEquation(const Point *verts) const {
        return Plane(verts[mVRef[0]], verts[mVRef[1]], verts[mVRef[2]]);
    }

    bool Equal(const IndexedTriangle &tri) const;
};

#endif // __ICEINDEXEDTRIANGLE_H__
