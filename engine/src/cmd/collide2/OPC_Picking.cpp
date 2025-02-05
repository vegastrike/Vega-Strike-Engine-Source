///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to perform "picking".
 *	\file		OPC_Picking.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2025-01-14
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmd/collide2/Opcode.h"

namespace Opcode {

#ifdef OPC_RAYHIT_CALLBACK

/*
	Possible RayCollider usages:
	- boolean query (shadow feeler)
	- closest hit
	- all hits
	- number of intersection (boolean)

*/

bool SetupAllHits(RayCollider &collider, CollisionFaces &contacts) {
    struct Local {
        static void AllContacts(const CollisionFace &hit, void *user_data) {
            CollisionFaces *CF = (CollisionFaces *) user_data;
            CF->AddFace(hit);
        }
    };

    collider.SetFirstContact(false);
    collider.SetHitCallback(Local::AllContacts);
    collider.SetUserData(&contacts);
    return true;
}

bool SetupClosestHit(RayCollider &collider, CollisionFace &closest_contact) {
    struct Local {
        static void ClosestContact(const CollisionFace &hit, void *user_data) {
            CollisionFace *CF = (CollisionFace *) user_data;
            if (hit.mDistance < CF->mDistance) {
                *CF = hit;
            }
        }
    };

    collider.SetFirstContact(false);
    collider.SetHitCallback(Local::ClosestContact);
    collider.SetUserData(&closest_contact);
    closest_contact.mDistance = MAX_FLOAT;
    return true;
}

bool SetupShadowFeeler(RayCollider &collider) {
    collider.SetFirstContact(true);
    collider.SetHitCallback(nullptr);
    return true;
}

bool SetupInOutTest(RayCollider &collider) {
    collider.SetFirstContact(false);
    collider.SetHitCallback(nullptr);
    // Results with collider.GetNbIntersections()
    return true;
}

bool Picking(
        CollisionFace &picked_face,
        const Ray &world_ray, const Model &model, const Matrix4x4 *world,
        float min_dist, float max_dist, const Point &view_point, CullModeCallback callback, void *user_data) {
    struct Local {
        struct CullData {
            CollisionFace *Closest;
            float MinLimit;
            CullModeCallback Callback;
            void *UserData;
            Point ViewPoint;
            const MeshInterface *IMesh;
        };

        // Called for each stabbed face
        static void RenderCullingCallback(const CollisionFace &hit, void *user_data) {
            CullData *Data = (CullData *) user_data;

            // Discard face if we already have a closer hit
            if (hit.mDistance >= Data->Closest->mDistance) {
                return;
            }

            // Discard face if hit point is smaller than min limit. This mainly happens when the face is in front
            // of the near clip plane (or straddles it). If we keep the face nonetheless, the user can select an
            // object that he may not even be able to see, which is very annoying.
            if (hit.mDistance <= Data->MinLimit) {
                return;
            }

            // This is the index of currently stabbed triangle.
            uint32_t StabbedFaceIndex = hit.mFaceID;

            // We may keep it or not, depending on backface culling
            bool KeepIt = true;

            // Catch *render* cull mode for this face
            CullMode CM = (Data->Callback)(StabbedFaceIndex, Data->UserData);

            if (CM != CULLMODE_NONE)    // Don't even compute culling for double-sided triangles
            {
                // Compute backface culling for current face

                VertexPointers VP;
                Data->IMesh->GetTriangle(VP, StabbedFaceIndex);
                if (VP.BackfaceCulling(Data->ViewPoint)) {
                    if (CM == CULLMODE_CW) {
                        KeepIt = false;
                    }
                } else {
                    if (CM == CULLMODE_CCW) {
                        KeepIt = false;
                    }
                }
            }

            if (KeepIt) {
                *Data->Closest = hit;
            }
        }
    };

    RayCollider RC;
    RC.SetMaxDist(max_dist);
    RC.SetTemporalCoherence(false);
    RC.SetCulling(false);        // We need all faces since some of them can be double-sided
    RC.SetFirstContact(false);
    RC.SetHitCallback(Local::RenderCullingCallback);

    picked_face.mFaceID = INVALID_ID;
    picked_face.mDistance = MAX_FLOAT;
    picked_face.mU = 0.0f;
    picked_face.mV = 0.0f;

    Local::CullData Data;
    Data.Closest = &picked_face;
    Data.MinLimit = min_dist;
    Data.Callback = callback;
    Data.UserData = user_data;
    Data.ViewPoint = view_point;
    Data.IMesh = model.GetMeshInterface();

    if (world) {
        // Get matrices
        Matrix4x4 InvWorld;
        InvertPRMatrix(InvWorld, *world);

        // Compute camera position in mesh space
        Data.ViewPoint *= InvWorld;
    }

    RC.SetUserData(&Data);
    if (RC.Collide(world_ray, model, world)) {
        return picked_face.mFaceID != INVALID_ID;
    }
    return false;
}

} // namespace Opcode

#endif

