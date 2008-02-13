#include<cstdlib>
template <class ITEM> class Resizable {
  unsigned int num;
  unsigned int alloc;
  ITEM * q;
 public:
  Resizable() {num=0;alloc=16;q=(ITEM*)malloc (sizeof (ITEM)*16);}
  Resizable (const Resizable<ITEM> &c) {num = c.num; alloc = c.alloc;q = (ITEM*)malloc (sizeof (ITEM)*alloc);memcpy (q,c.q,sizeof(ITEM)*num);}
  ~Resizable () {free (q); q=NULL;}
  void assert_free (unsigned int n) {while(n+num>alloc) {alloc*=2;q=(ITEM *)realloc (q,sizeof (ITEM)*alloc);}}
  void push_back_nocheck (const ITEM &a) {q[num]=a;num++;}
  void inc_num (unsigned int n) {num+=n;}
  void push_back (const ITEM &a) {
    if (alloc<num+1) {
      alloc*=2;
      q = (ITEM *) realloc (q,sizeof (ITEM)*alloc);
    }
    q[num]=a;
    num++;
  }
  void push_back3 (const ITEM aa[3]) {
    if (alloc<num+3) {
      alloc*=2;
      q=(ITEM *)realloc (q,sizeof (ITEM)*alloc);
    }
    q[num]=aa[0];
    q[num+1]=aa[1];
    q[num+2]=aa[2];
    num+=3;
  }
  void push_back (const ITEM& aa,const ITEM& bb, const ITEM& cc) {
    if (alloc<num+3) {
      alloc*=2;
      q=(ITEM *)realloc (q,sizeof (ITEM)*alloc);
    }
    q[num]=aa;
    q[num+1]=bb;
    q[num+2]=cc;
    num+=3;
  }
  void push_backN (const ITEM *aa, const unsigned int N) {
    while (alloc<num+N) {
      alloc*=2;
      q=(ITEM *)realloc (q,sizeof (ITEM)*alloc);
    }
    for (unsigned int i=0;i<N;i++) {
      q[num+i]=aa[i];
    }
    num+=N;
  }
  ITEM * begin() {return q;}
  ITEM * end () {return q+num;}
  void clear () {num=0;}
  ITEM &back () {return q[num-1];}
  ITEM & pop_back () {num--;return q[num];}
  unsigned int size () {return num;}
};
