#ifndef COREVECTOR_H
#define COREVECTOR_H

struct CoreVector {
    float i = 0, j = 0 ,k = 0;

    CoreVector() {}
    CoreVector(float i, float j, float k) : i(i), j(j), k(k) {}
};

#endif // COREVECTOR_H
