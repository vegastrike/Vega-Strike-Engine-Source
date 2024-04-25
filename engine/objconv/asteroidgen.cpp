/*
 * asteroidgen.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338328
#endif
#include <float.h>
#if defined(__APPLE__) && defined(__MACH__)
#define sqrtf sqrt
#define cosf cos
#define sinf sin
#endif

float safety_zone = 0;
using std::vector;

class Vector {
public:
    float i;
    float j;
    float k;
    float s;
    float t;

    Vector(float x = 0, float y = 0, float z = 0) {
        i = x;
        j = y;
        k = z;
    }

    Vector(float x, float y, float z, float s, float t) {
        i = x;
        j = y;
        k = z;
        this->s = s;
        this->t = t;
    }

    Vector operator+(Vector b) {
        return Vector(i + b.i, j + b.j, k + b.k);
    }

    float Mag() {
        return sqrtf(i * i + j * j + k * k);
    }

    void Yaw(float rad) //only works with unit vector
    {
        float theta = 0.0f;
        float m = Mag();
        if (i > 0) {
            theta = (float) atan(k / i);
        } else if (i < 0) {
            theta = M_PI + (float) atan(k / i);
        } else if (k <= 0 && i == 0) {
            theta = -M_PI / 2;
        } else if (k > 0 && i == 0) {
            theta = M_PI / 2;
        }
        theta += rad;
        i = m * cosf(theta);
        k = m * sinf(theta);
    }

    void Roll(float rad) {
        float theta = 0.0f;
        float m = Mag();
        if (i > 0) {
            theta = (float) atan(j / i);
        } else if (i < 0) {
            theta = M_PI + (float) atan(j / i);
        } else if (j <= 0 && i == 0) {
            theta = -M_PI / 2;
        } else if (j > 0 && i == 0) {
            theta = M_PI / 2;
        }
        theta += rad;
        i = m * cosf(theta);
        j = m * sinf(theta);
    }

    void Pitch(float rad) {
        float theta = 0.0f;
        float m = Mag();
        if (k > 0) {
            theta = (float) atan(j / k);
        } else if (k < 0) {
            theta = M_PI + (float) atan(j / k);
        } else if (j <= 0 && k == 0) {
            theta = -M_PI / 2;
        } else if (j > 0 && k == 0) {
            theta = M_PI / 2;
        }
        theta += rad;
        k = m * cosf(theta);
        j = m * sinf(theta);
    }
};
class Tri {
public:
    bool quad;
    int a;
    float sa, ta;
    int b;
    float sb, tb;
    int c;
    float sc, tc;
    int d;
    float sd, td;

    Tri(int x, int y, int z) {
        c = x;
        b = y;
        a = z;
        quad = false;
    }

    Tri(int x, int y, int z, int w) {
        d = x;
        c = y;
        b = z;
        a = w;
        quad = true;
    }

    void Write(FILE *fp) {
        if (!quad) {
            fprintf(fp, "<Tri>\n");
        } else {
            fprintf(fp, "<Quad>\n");
        }
        fprintf(fp, "<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", a, sa, ta);
        fprintf(fp, "<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", b, sb, tb);
        fprintf(fp, "<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", c, sc, tc);
        if (!quad) {
            fprintf(fp, "</Tri>\n");
        } else {
            fprintf(fp, "<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", d, sd, td);
            fprintf(fp, "</Quad>\n");
        }
    }
};
struct asteroid {
    Vector center;
    float radius = 0.0f; //Initializing to a value so the compiler shuts up about it
    Vector YawPitchRoll;
    int num_polys = 0.0f; //Initializing to a value so the compiler shuts up about it
    vector<Vector> points;
    vector<Tri> polygon;

    asteroid() : center(0, 0, 0), YawPitchRoll(0, 0, 0) {
    }

    void CenterAndRotate() {
        for (unsigned int i = 0; i < points.size(); i++) {
            //points[i].Pitch (YawPitchRoll.i);
            //points[i].Yaw (YawPitchRoll.j);
            //points[i].Roll (YawPitchRoll.k);
            points[i].i += center.i;
            points[i].j += center.j;
            points[i].k += center.k;
        }
    }
};

char texture[100] = "Asteroid.bmp";
float scale = 1;

void determine_centers_and_radii(vector<asteroid> &field,
        const Vector &cube_sides,
        const float radiusmin,
        const float radiusmax,
        const int poly_min,
        const int poly_max) {
    for (unsigned int i = 0; i < field.size(); i++) {
        field[i].center.i = cube_sides.i * ((float) rand()) / RAND_MAX - cube_sides.i / 2;
        field[i].center.j = cube_sides.j * ((float) rand()) / RAND_MAX - cube_sides.j / 2;
        field[i].center.k = cube_sides.k * ((float) rand()) / RAND_MAX - cube_sides.k / 2;
        float radiusratio = ((float) rand()) / RAND_MAX;
        field[i].radius = radiusmin + (radiusmax - radiusmin) * radiusratio;
        radiusratio *= radiusratio * (poly_max + 1 - poly_min);
        field[i].num_polys = (int) radiusratio + poly_min;
        if (field[i].num_polys < 4) {
            field[i].num_polys = 4;
        }
        field[i].YawPitchRoll.i = 2 * M_PI * ((float) rand()) / RAND_MAX;
        field[i].YawPitchRoll.j = 2 * M_PI * ((float) rand()) / RAND_MAX;
        field[i].YawPitchRoll.k = 2 * M_PI * ((float) rand()) / RAND_MAX;
        bool insideanother = false;
        if (field[i].center.Mag() < safety_zone) {
            insideanother = true;
        } else {
            for (unsigned int j = 0; j < i; j++) {
                if ((fabs(field[j].center.i - field[i].center.i) < 1.2
                        * (field[j].radius + field[i].radius)
                        && fabs(field[j].center.j - field[i].center.j) < 1.2 * (field[j].radius + field[i].radius)
                        && fabs(field[j].center.j - field[i].center.j) < 1.2 * (field[j].radius + field[i].radius))) {
                    insideanother = true;
                    break;
                }
            }
        }
        if (insideanother) {
            i--;
        }
    }
}

float getR(float minr, float maxr) {
    return ((maxr - minr) * ((float) rand()) / RAND_MAX) + minr;
}

void generateTet(vector<Vector> &v, vector<Tri> &p, const float minr, const float maxr) {
    double h = 1 / sqrt((double) 2.0);
    double r = getR(minr, maxr);
    double rA = r;
    v.push_back(Vector(0, -r, h * r, 0, 0));
    r = getR(minr, maxr);
    v.push_back(Vector(r, 0, -h * r, .5, 0));
    r = rA;
    v.push_back(Vector(0, -r, h * r, 1, 0));
    r = getR(minr, maxr);
    double rD = r;
    v.push_back(Vector(0, r, h * r, 1, 1));
    r = getR(minr, maxr);
    v.push_back(Vector(-r, 0, -h * r, .5, 1));
    r = rD;
    v.push_back(Vector(0, r, h * r, 0, 1));
    p.push_back(Tri(0, 5, 1));     //AFB
    p.push_back(Tri(3, 2, 4));     //DCE
    p.push_back(Tri(4, 2, 1));     //ECB
    p.push_back(Tri(1, 5, 4));     //BFE
}

void generateNTet(vector<Vector> &v,
        vector<Tri> &p,
        const float minr,
        const float maxr,
        unsigned int stacks,
        unsigned int slices) {
    for (unsigned int i = 0; i < stacks + 2; i++) {
        float tempR = getR(minr, maxr);
        for (unsigned int j = 0; j < slices; j++) {
            if (i != 0 && i != stacks + 1) {
                tempR = getR(minr, maxr);
            }                  ///don't want the tip ot have different points
            float projR = tempR * sin(M_PI * i / (stacks + 1));
            if ((i != 0 && i != stacks + 1) || j == 0) {
                v.push_back(Vector(projR * cos(2 * M_PI * j / (slices)),                   //i
                        tempR * cos(M_PI * i / (stacks + 1)),                    //j
                        projR * sin(2 * M_PI * j / (slices)),                    //k
                        ((float) j) / (slices - 1)
                                + ((i == 0 || i == stacks + 1) ? .5 : 0),                      //s
                        ((float) i) / (stacks + 1)));                    //t
            }
            if (i != 0 && i != 1 && i != stacks + 1) {
                p.push_back(Tri(1 + (i - 2) * slices + j,
                        1 + (i - 1) * slices + j,
                        1 + (i - 1) * slices + ((j + 1) % slices),
                        1 + (i - 2) * slices + ((j + 1) % slices)));
            } else if (i == 1) {
                //do top pyr

                p.push_back(Tri(0,
                        1 + j,
                        1 + ((j + 1) % slices)));
            } else if (i == stacks + 1) {
                p.push_back(Tri(1 + (i - 2) * slices + j,
                        1 + (i - 1) * slices,
                        1 + (i - 2) * slices + ((j + 1) % slices)));
            }
            //do bottom pyr
        }
    }
}

void generateDoubleTet(vector<Vector> &v, vector<Tri> &p, const float minr, const float maxr, int num) {
    generateNTet(v, p, minr, maxr, 1, num);
}

void createShapes(asteroid &a, float dev) {
    if (a.num_polys < 6) {
        generateTet(a.points, a.polygon, a.radius * (1 - dev), a.radius * (1 + dev));
    } else if (a.num_polys < 8) {
        generateDoubleTet(a.points, a.polygon, a.radius * (1 - dev), a.radius * (1 + dev), 3);
    } else if (a.num_polys < 10) {
        generateDoubleTet(a.points, a.polygon, a.radius * (1 - dev), a.radius * (1 + dev), 4);
    } else if (a.num_polys < 12) {
        generateDoubleTet(a.points, a.polygon, a.radius * (1 - dev), a.radius * (1 + dev), 6);
    } else if (a.num_polys < 36) {
        generateNTet(a.points,
                a.polygon,
                a.radius * (1 - dev),
                a.radius * (1 + dev),
                a.num_polys / 6,
                a.num_polys % 6 + 3);
    } else {
        generateNTet(a.points,
                a.polygon,
                a.radius * (1 - dev),
                a.radius * (1 + dev),
                a.num_polys / 12,
                a.num_polys % 12 + 6);
    }
}

void write_mesh(FILE *fp, vector<asteroid> &field) {
    fprintf(fp, "<Mesh texture=\"%s\" sharevertex=\"1\" scale=\"%f\">\n<Points>\n", texture, scale);
    unsigned int i;
    unsigned int counter = 0;
    for (i = 0; i < field.size(); i++) {
        unsigned int j;
        for (j = 0; j < field[i].points.size(); j++) {
            fprintf(fp,
                    "<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\" t=\"%f\"/>\n</Point>\n",
                    field[i].points[j].i,
                    field[i].points[j].j,
                    field[i].points[j].k,
                    field[i].points[j].s,
                    field[i].points[j].t);
        }
        for (j = 0; j < field[i].polygon.size(); j++) {
            field[i].polygon[j].sa = field[i].points[field[i].polygon[j].a].s;
            field[i].polygon[j].ta = field[i].points[field[i].polygon[j].a].t;
            field[i].polygon[j].a += counter;
            field[i].polygon[j].sb = field[i].points[field[i].polygon[j].b].s;
            field[i].polygon[j].tb = field[i].points[field[i].polygon[j].b].t;
            field[i].polygon[j].b += counter;
            field[i].polygon[j].sc = field[i].points[field[i].polygon[j].c].s;
            field[i].polygon[j].tc = field[i].points[field[i].polygon[j].c].t;
            field[i].polygon[j].c += counter;
            if (field[i].polygon[j].quad) {
                field[i].polygon[j].sd = field[i].points[field[i].polygon[j].d].s;
                field[i].polygon[j].td = field[i].points[field[i].polygon[j].d].t;
                field[i].polygon[j].d += counter;
            }
        }
        counter += field[i].points.size();
    }
    fprintf(fp, "</Points>\n<Polygons>\n");
    for (i = 0; i < field.size(); i++) {
        unsigned int j;
        for (j = 0; j < field[i].polygon.size(); j++) {
            field[i].polygon[j].Write(fp);
        }
    }
    //ED: Not sure what the following printf is supposed to mean.
    //fprintf (fp,"</Polygons>\n<Material reflect=\"%d\">\n<Specular red=\"%f\" green=\"%f\" blue=\"%f\" alpha=\"%f\"/>\n</Material>\n</Mesh>\n",0,0,0,0,1);
}

void createShapes(vector<asteroid> &field, float deviation) {
    for (unsigned int i = 0; i < field.size(); i++) {
        createShapes(field[i], deviation);
        field[i].CenterAndRotate();
    }
}

bool isBoxInsideOuterRadius(Vector center, float sizeofBox, float radius) {
    if (center.Mag() > radius) {
        return false;
    }
    return true;
}

bool isBoxOutsideInnerRadius(Vector center, float sizeofBox, float radius) {
    if (radius <= 0) {
        return true;
    }
    Vector pnts[8];
    sizeofBox /= 2;
    float mini = -radius;
    float maxi = radius;
    float minj = -radius;
    float maxj = radius;
    float mink = -radius;
    float maxk = radius;

    pnts[0] = center + Vector(-sizeofBox, sizeofBox, -sizeofBox);
    pnts[1] = center + Vector(-sizeofBox, sizeofBox, sizeofBox);
    pnts[2] = center + Vector(-sizeofBox, -sizeofBox, -sizeofBox);
    pnts[3] = center + Vector(-sizeofBox, -sizeofBox, sizeofBox);
    pnts[4] = center + Vector(sizeofBox, sizeofBox, -sizeofBox);
    pnts[5] = center + Vector(sizeofBox, sizeofBox, sizeofBox);
    pnts[6] = center + Vector(sizeofBox, -sizeofBox, -sizeofBox);
    pnts[7] = center + Vector(sizeofBox, -sizeofBox, sizeofBox);
    int i;
    for (i = 0; i < 8; i++) {
        if (pnts[i].i < maxi) {
            break;
        }
    }
    if (i == 8) {
        return true;
    }
    for (i = 0; i < 8; i++) {
        if (pnts[i].i > mini) {
            break;
        }
    }
    if (i == 8) {
        return true;
    }
    for (i = 0; i < 8; i++) {
        if (pnts[i].j < maxj) {
            break;
        }
    }
    if (i == 8) {
        return true;
    }
    for (i = 0; i < 8; i++) {
        if (pnts[i].j > minj) {
            break;
        }
    }
    if (i == 8) {
        return true;
    }
    for (i = 0; i < 8; i++) {
        if (pnts[i].k < maxk) {
            break;
        }
    }
    if (i == 8) {
        return true;
    }
    for (i = 0; i < 8; i++) {
        if (pnts[i].k > mink) {
            break;
        }
    }
    if (i == 8) {
        return true;
    }
    return false;
}

#ifdef RAND
float randRadius( float radius )
{
    return ( (rand()*2*radius)/RAND_MAX )-radius;
}
Vector randVecInCube( float radius )
{
    return Vector( randRadius( radius ), randRadius( radius ), randRadius( radius ) );
}
#else

float randRadius(float BoxSize) {
    return ((rand() * BoxSize) / RAND_MAX) - (BoxSize / 2);
}

Vector randVecInCube(float BoxSize, float x, float y, float z) {
    return Vector(randRadius(BoxSize) + x, randRadius(BoxSize) + y, randRadius(BoxSize) + z);
}

#endif

#ifdef RAND
void write_unit( FILE *fp, const char *astFile, int num_cubes, float innerRadius, float outerRadius, float BoxSize )
{
    fprintf( fp, "<Unit>" );
    if (!innerRadius) {
        num_cubes--;
        fprintf( fp, "\n\t<SubUnit file=\"%s\" />", astFile );
    }
    for (int i = 0; i < num_cubes; i++) {
        printf( "\nNumber %d out of %d", i, (int) num_cubes );
        Vector vec;
        do
            vec = randVecInCube( outerRadius );
        while ( !( isBoxInsideOuterRadius( vec, BoxSize, outerRadius ) && isBoxOutsideInnerRadius( vec, BoxSize, innerRadius ) ) );
        fprintf( fp, "\n\t<SubUnit file=\"%s\" x=\"%f\" y=\"%f\" z=\"%f\" />", astFile, vec.i, vec.j, vec.k );
    }
#else

void write_unit(FILE *fp, const char *astFile, float offset, float innerRadius, float outerRadius, float BoxSize) {
    fprintf(fp, "<Unit>");
    for (float x = -outerRadius; x < outerRadius; x += offset) {
        for (float y = -outerRadius; y < outerRadius; y += offset) {
            for (float z = -outerRadius; z < outerRadius; z += offset) {
                Vector vec;
                vec = randVecInCube(BoxSize, x, y, z);
                if ((isBoxInsideOuterRadius(vec, BoxSize,
                        outerRadius) && isBoxOutsideInnerRadius(vec, BoxSize, innerRadius))) {
                    fprintf(fp, "\n\t<SubUnit file=\"%s\" x=\"%f\" y=\"%f\" z=\"%f\" />", astFile, vec.i, vec.j, vec.k);
                }
                //printf("\nIteration with Vector <%f,%f,%f>",vec.i,vec.j,vec.k);
            }
        }
    }

#endif

    fprintf(
            fp,
            "\n\t<Thrust>\n\t\t<Engine Afterburner=\"00.000000\" Forward=\"0\" Retro=\"0\" Left=\"0\" Right=\"0\" Top=\"0\" Bottom=\"0\" />\n\t\t<Maneuver yaw=\"0\" pitch=\"0\" roll=\"0\" />\n\t</Thrust>\n\t<Defense HudImage=\"af-hud.spr\">\n\t\t<Hull strength=\"400000000\" />\n\t</Defense>\n\t<Stats mass=\"60\" momentofinertia=\"60\" fuel=\"20000\">\n\t</Stats>\n</Unit>");
}

int main(int argc, char **argv) {
    vector<asteroid> field;
    Vector cube_sides(1024, 1024, 1024);
    float radiusmin, radiusmax;
    int poly_min, poly_max;
    float deviation;
    char unitfilename[16384];
    int num_cubes = 0;
    float offset = 0;
    float innerRadius;
    float outerRadius;
    char filename[16384];
    int numroids;
    int randomseed = 0;
    if (argc >= 9) {
        sscanf(argv[1], "%s", filename);
        sscanf(argv[2], "%f", &cube_sides.i);
        cube_sides.j = cube_sides.k = cube_sides.i;
        sscanf(argv[3], "%d", &numroids);
        sscanf(argv[4], "%f", &radiusmin);
        sscanf(argv[5], "%f", &radiusmax);
        sscanf(argv[6], "%f", &deviation);
        sscanf(argv[7], "%d", &poly_min);
        sscanf(argv[8], "%d", &poly_max);
        if (argc >= 10) {
            sscanf(argv[9], "%f", &safety_zone);
            if (argc >= 11) {
                sscanf(argv[10], "%d", &randomseed);
                if (argc >= 15) {
                    sscanf(argv[11], "%s", unitfilename);
#ifdef RAND
                    sscanf( argv[12], "%d", &num_cubes );
#else
                    sscanf(argv[12], "%f", &offset);
#endif
                    sscanf(argv[13], "%f", &innerRadius);
                    sscanf(argv[14], "%f", &outerRadius);
                }
            }
        }
    } else {
        printf("Enter Output File:\n");
        if (scanf("%s", filename) != 1) {
            return -1;
        }
        printf("Enter Size of field?\n");
        if (scanf("%f", &cube_sides.i) != 1) {
            return -1;
        }
        cube_sides.j = cube_sides.k = cube_sides.i;
        printf("Enter number of asteroids\n");
        if (scanf("%d", &numroids) != 1) {
            return -1;
        }
        printf("Enter asteroid minimum and max radius\n");
        if (scanf("%f %f", &radiusmin, &radiusmax) != 2) {
            return -1;
        }
        printf("Enter cragginess from 0 to 1\n");
        if (scanf("%f", &deviation) != 1) {
            return -1;
        }
        printf("Enter minimum number of polys and max num polys\n");
        if (scanf("%d %d", &poly_min, &poly_max) != 2) {
            return -1;
        }
        printf("Enter size of safety zone (0 to disable)\n");
        if (scanf("%f", &safety_zone) != 1) {
            return -1;
        }
        printf("Enter random seed (0 to use clock\n");
        if (scanf("%d", &randomseed) != 1) {
            return -1;
        }
        printf("Do you want a unit file? (y/n)\n");
        if (scanf("%c", (char *) &num_cubes) != 1) {
            return -1;
        }
        if ((char) num_cubes == 'y') {
            printf("Enter Output Unit File:\n");
            if (scanf("%s", unitfilename) != 1) {
                return -1;
            }
#ifdef RAND
            printf( "Enter number of cubes?\n" );
            if(scanf( "%d", &num_cubes ) != 1) return -1;
#else
            printf("Enter offset of cubes?\n");
            if (scanf("%f", &offset) != 1) {
                return -1;
            }
#endif
            printf("Enter inner radius?\n");
            if (scanf("%f", &innerRadius) != 1) {
                return -1;
            }
            printf("Enter outer radius?\n");
            if (scanf("%f", &outerRadius) != 1) {
                return -1;
            }
        } else {
            num_cubes = 0;
            offset = 0;
        }
    }
    if (randomseed != 0) {
        srand(randomseed);
    } else {
#ifndef _WIN32
        srand(time(NULL));
#endif
    }
    for (int i = 0; i < numroids; i++) {
        field.push_back(asteroid());
    }
    determine_centers_and_radii(field, cube_sides, radiusmin, radiusmax, poly_min, poly_max);
    createShapes(field, deviation);
    FILE *fp = fopen(filename, "w");
    write_mesh(fp, field);
    fclose(fp);
    if (num_cubes || offset) {
        const char *newfilename = "asteroids";
        fp = fopen(unitfilename, "w");
#ifdef RAND
        write_unit( fp, newfilename, num_cubes, innerRadius, outerRadius, cube_sides.i );
#else
        write_unit(fp, newfilename, offset, innerRadius, outerRadius, cube_sides.i);
#endif
        fclose(fp);
    }
    return 0;
}

