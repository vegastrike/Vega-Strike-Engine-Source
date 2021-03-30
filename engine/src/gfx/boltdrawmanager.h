#ifndef BOLTDRAWMANAGER_H
#define BOLTDRAWMANAGER_H

#include "gfx/decalqueue.h"
#include "bolt.h"
#include "vec.h"

#include <vector>

class Animation;

class BoltDrawManager
{
public:
    class DecalQueue boltdecals;
    static GFXVertexList * boltmesh;
    static QVector camera_position;
    static float pixel_angle;
    static float elapsed_time;

    vector <std::string> animationname;
    vector <Animation *> animations; // Balls are animated
    vector <vector <Bolt> > bolts; // The inner vector is all of the same type.
    vector <vector <Bolt> > balls;

    BoltDrawManager();
    ~BoltDrawManager();

    static BoltDrawManager& GetInstance();

    static void Draw();
};

#endif // BOLTDRAWMANAGER_H
