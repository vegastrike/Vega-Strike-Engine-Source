#include "boltdrawmanager.h"

#include "options.h"

BoltDrawManager::BoltDrawManager()
{
    if (!boltmesh) {
        GFXVertex    vtx[12];
#define V( ii, xx, yy, zz, ss,                                                                                                \
           tt ) vtx[ii].x = xx; vtx[ii].y = yy; vtx[ii].z = zz+game_options.bolt_offset+.875; vtx[ii].i = 0; vtx[ii].j = 0; vtx[ii].k = 1; \
    vtx[ii].s = ss; vtx[ii].t = tt;
        V( 0, 0, 0, -.875, 0, .5 );
        V( 1, 0, -1, 0, .875, 1 );
        V( 2, 0, 0, .125, 1, .5 );
        V( 3, 0, 1, 0, .875, 0 );
        V( 4, 0, 0, -.875, 0, .5 );
        V( 5, -1, 0, 0, .875, 1 );
        V( 6, 0, 0, .125, 1, .5 );
        V( 7, 1, 0, 0, .875, 0 );
        V( 8, 1, 0, 0, .1875, 0 );
        V( 9, 0, 1, 0, .375, .1875 );
        V( 10, -1, 0, 0, .1875, .375 );
        V( 11, 0, -1, 0, 0, .1875 );
        boltmesh = new GFXVertexList( GFXQUAD, 12, vtx, 12, false );         //not mutable;
    }
}

GFXVertexList *BoltDrawManager::boltmesh = NULL;
BoltDrawManager::~BoltDrawManager()
{
    unsigned int i;
    for (i = 0; i < cachedecals.size(); i++)
        boltdecals.DelTexture( cachedecals[i] );
    cachedecals.clear();
    for (i = 0; i < animations.size(); i++)
        delete animations[i];
    for (i = 0; i < balls.size(); i++)
        for (int j = balls[i].size()-1; j >= 0; j--)
            balls[i][j].Destroy( j );
    for (i = 0; i < bolts.size(); i++)
        for (int j = bolts[i].size()-1; j >= 0; j--)
            bolts[i][j].Destroy( j );
}
