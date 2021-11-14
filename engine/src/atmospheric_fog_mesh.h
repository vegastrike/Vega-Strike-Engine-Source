#ifndef ATMOSPHERICFOGMESH_H
#define ATMOSPHERICFOGMESH_H

#include <string>

struct AtmosphericFogMesh
{
    std::string meshname;
    double scale;
    float  er;
    float  eg;
    float  eb;
    float  ea;
    float  dr;
    float  dg;
    float  db;
    float  da;
    double focus;
    double concavity;
    int    tail_mode_start;
    int    tail_mode_end;
    int    min_alpha;
    int    max_alpha;
    AtmosphericFogMesh();
};

#endif // ATMOSPHERICFOGMESH_H
