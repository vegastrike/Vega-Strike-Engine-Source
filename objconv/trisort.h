class Vector {
public:
    double x,y,z;
    Vector (double xx, double yy, double zz):x(xx),y(yy),z(zz) {}

};
#ifdef __APPLE__
int iswhitespace(int c) {
    return isspace (c)||isblank(c)||c=='\n'||c=='\r';
}
#endif
class Index {
public:
    Vector V;
    int p,n,t,c;
    Index (Vector xyz,int p, int n, int t, int c):V(xyz),p(p),n(n),t(t),c(c){}

    void write (FILE * fp) const;
};

class Plane {public:
    double a,b,c,d;
    Plane (int a,int b, int c, int d): a(a),b(b),c(c),d(d){}
    bool inFront (const Vector &) const;
    float frontBack (const Vector &v) const {return a*v.x+b*v.y+c*v.z+d;}
};
class Face {
    bool Cross (Plane &)const;
    bool inFront (const Plane &) const;
public:
std::vector <Index> p;
    int id;
    Plane planeEqu()const;
    bool operator < (const class Face &) const;
    Face () {static int temp=0; id = temp++;}
};
class Mesh {
    void processline (char * line);
    Index Mesh::processfacevertex(char * chunk) const;
    Face Mesh::processface( char * line)const;
std::vector <Vector> p;
std::vector <Vector> n;
std::vector <Vector> t;//tex coords i=s j=t
std::vector <Vector> c;
std::vector <Face> f;
public:
        Mesh (const char * filename);
    void sort();
    void write(const char * filename) const;
    void writeIndex (FILE * fp,const Index&)const;
};

