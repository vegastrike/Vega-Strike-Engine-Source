#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <ctype.h>
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
    Vector x;
    int p,n,t,c;
    Index (Vector xyz,int p, int n, int t, int c):x(xyz),p(p),n(n),t(t),c(c){}

    void write (FILE * fp) const;
};
class Face {
public:
std::vector <Index> p;
    bool operator < (const class Face &) const{return true;}
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

void Mesh::sort () {
std::sort (f.begin(),f.end());
}
void Index::write (FILE * fp) const{
    fprintf (fp,"%d/",p);
    if (n!=-1)
        fprintf (fp,"%d",n);
    fprintf (fp,"/");
    if (t!=-1)
        fprintf (fp,"%d",t);
    if (c!=-1) {
        fprintf (fp,"/%d",c);
    }
}
void Mesh::write(const char * filename)const {
    FILE * fp = fopen (filename,"w");
    if (fp) {
    unsigned int i;
    for (i=0;i<p.size();i++) {
        fprintf (fp,"p %lf %lf %lf\n",p[i].x,p[i].y,p[i].z);
    }
    for (i=0;i<n.size();i++) {
        fprintf (fp,"n %lf %lf %lf\n",n[i].x,n[i].y,n[i].z);
    }
    for (i=0;i<t.size();i++) {
        fprintf (fp,"p %lf %lf\n",t[i].x,t[i].y);        
    }
    for (i=0;i<c.size();i++) {
        fprintf (fp,"c %lf %lf %lf\n",c[i].x,c[i].y,c[i].z);        
    }
    for (i=0;i<f.size();i++) {
        fprintf (fp,"f");
        for (unsigned j=0;j<f[i].p.size();j++) {
            fprintf (fp," ");
            f[i].p[j].write (fp);
        }
        fprintf (fp,"\n");
    }
    fclose (fp);
    }
}
char * findspace (char *line) {
    while (iswhitespace(line[0])&&line[0])
        line++;
    if (!line[0])
        return NULL;
    while (!iswhitespace(line[0])&&line[0])
        line++;
    return line;
}
Index Mesh::processfacevertex(char * vertex)const {
    int a,b,c,d;a=0;b=c=d=-1;
    sscanf (vertex,"%d",&a);
    while (vertex[0] && (*vertex)!='/')
        vertex++;
    if (vertex[0]=='/') {
        vertex++;
        if (vertex[0]!='/')
            sscanf (vertex,"%d",&b);
        while (vertex[0] && (*vertex)!='/')
            vertex++;
        if (vertex[0]=='/') {
            vertex++;
            if (vertex[0]!='/')
                sscanf (vertex,"%d",&c);
            while (vertex[0] && (*vertex)!='/')
                vertex++;
            if (vertex[0]=='/') {
                vertex++;
                sscanf (vertex,"%d",&d);
            }
        }
    }
    Vector v(0,0,0);
    if (a<=p.size()&&a>0)
        v=p[a-1];
    return Index (v,a,b,c,d);
}
Face Mesh::processface (char * line) const{
    Face f;
    while (line[0]) {
        char * lastspace = findspace (line);
        if (lastspace) {
            bool done= (lastspace[0]==0);
            (*lastspace)=0;
            f.p.push_back (processfacevertex (line));
            line =lastspace+1;
            if (done)
                break;
        }else {
            break;
        }
    }
    return f;
}
void Mesh::processline (char * line) {
    double a=0,b=0,c=0,d=0;
    switch (line[0]) {
        case 'p':
            sscanf (line,"p %lf %lf %lf",&a,&b,&c);
            p.push_back (Vector (a,b,c));
            break;
        case 'n':
            sscanf (line,"n %lf %lf %lf",&a,&b,&c);
            n.push_back (Vector (a,b,c));            
            break;
        case 't':
            sscanf (line,"t %lf %lf",&a,&b);
            t.push_back (Vector (a,b,0));
            break;
        case 'c':
            sscanf (line,"c %lf %lf %lf %lf",&a,&b,&c,&d);
            this->c.push_back (Vector (a,b,c));
            break;
        case 'f':
            f.push_back (processface(line+1));
            break;
        default:
            break;
    }
}

Mesh::Mesh (const char * filename) {
    FILE * fp = fopen ( filename,"r");
    char line [65536];
    line[65535]=0;
    while (fgets (line,65535,fp)) {
        processline(line);
    }
}

int main (int argc, char ** argv) {
    for (int i=1;i<argc;i++) {
        Mesh m(argv[i]);
        m.sort();
std::string rez (argv[i]);
        rez+=".out";
        m.write (rez.c_str());
    }
};