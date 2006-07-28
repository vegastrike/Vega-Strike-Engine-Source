#include <ext/hash_map>
#include <sys/time.h>
#include <sys/types.h>
#include<map>
#define stdext __gnu_cxx
size_t globmax=1024;
class size{
public:
  size_t num;
  size() {num=(size_t)rand()%globmax;}
  bool operator== (const size &a) const{
    return this->num==a.num;
  }
  bool operator< (const size &a) const{
    return this->num<a.num;
  }
};
namespace stdext{
  template<> class hash<size>{
    hash<size_t> a;
  public:
    size_t operator()(const size &s) const{
      return a(s.num);
    }
  };
}
double firsttime=0;
double queryTime() {
#ifdef WIN32
  LONGLONG tmpnewtime;
  QueryPerformanceCounter((LARGE_INTEGER*)&tmpnewtime);
  return ((double)tmpnewtime)/(double)freq-firsttime;
#elif 1
  struct timeval tv;
  (void) gettimeofday(&tv, NULL);
  double tmpnewtime = (double)tv.tv_sec + (double)tv.tv_usec * 1.e-6;
  return tmpnewtime-firsttime;
#elif defined(HAVE_SDL)

  double tmpnewtime = SDL_GetTicks() * 1.e-3;
  return tmpnewtime-firsttime;
#else
# error "We have no way to determine the time on this system."
  return 0.;
#endif
}
double getTime(){
  return queryTime();
}
std::map<size_t,size_t> pod;
stdext::hash_map<size,size> clas;
int main (int argc, char ** argv) {
  size_t lima = (size_t)atof(argv[1]);
  size_t limb = (size_t)atof(argv[2]);
  size_t limc = (size_t)atof(argv[3]);
  size_t limd = (size_t)atof(argv[4]);
  size_t lime = (size_t)atof(argv[5]);
  globmax=lime;
  std::vector <size> sdata(limb);
  double first=getTime();
  for (size_t i=0;i<lima;++i) {
    size ins,dat;
    pod[ins.num]=dat.num;
    clas[ins]=dat;
  }
  double second=getTime();
  int sum=0;
  size tmp;
  for (size_t j=0;j<limc;++j){
    for (size_t i=0;i<limb;++i) {
      size_t num=sdata[i].num;
      sum+=(pod.find(num)!=pod.end()?pod.find(num)->second:0);
      
    }
  }
  double third=getTime();
  for (size_t j=0;j<limc;++j){
    for (size_t i=0;i<limb;++i) {
      size_t num=sdata[i].num;
      tmp.num=num;
      sum+=(clas.find(tmp)!=clas.end()?clas.find(tmp)->second.num:0);      
    }
  }
  double fourth=getTime();
  for (size_t j=0;j<limc;++j){
    for (size_t i=0;i<limb;++i) {
      size_t num=sdata[i].num;
      pod.insert(std::pair<size_t,size_t>(num,num));      
    }
    if (j!=limc-1&&j<limd) {
      for (size_t i=0;i<limb;++i) {
        size_t num=sdata[i].num;        
        if (pod.find(num)==pod.end()) {
          //printf ("error deleting item not in class\n");
        }else
          pod.erase(pod.find(num));      
      }
    }
  }

  double fifth=getTime();
  for (size_t j=0;j<limc;++j){
    for (size_t i=0;i<limb;++i) {
      tmp.num=sdata[i].num;
      clas.insert(std::pair<size,size>(tmp,tmp));      
    }    
    if (j!=limc-1) {
      for (size_t i=0;i<limb;++i) {
        tmp.num=sdata[i].num;    
        if (clas.find(tmp)==clas.end()) {
          //printf ("error deleting item not in class\n");
        }else
          clas.erase(clas.find(tmp));      
      }
    }
  }
  double sixth=getTime();
  for (size_t j=0;j<limc;++j){
    for (size_t i=0;i<limb;++i) {
      size_t num=sdata[i].num;
      sum+=pod.find(num)->second;
      
    }
  }
  double seventh=getTime();
  for (size_t j=0;j<limc;++j){
    for (size_t i=0;i<limb;++i) {
      size_t num=sdata[i].num;
      tmp.num=num;
      sum+=clas.find(tmp)->second.num;      
    }
  }
  double eighth=getTime();  
  printf ("Outer Array Size %d. Num Important Elem %d. Repeats %d. Num Erases %d\nInitial Hit Percent %f\n",
          pod.size(),sdata.size(),limc,limd,sdata.size()/(double)lime);
  printf ("Find Percent Times %f %f\n",third-second,fourth-third);
  printf ("Insert/Erase Times %f %f\n",fifth-fourth,sixth-fifth);
  printf ("Find all itm Times %f %f\n",seventh-sixth,eighth-seventh);

  return sum;
}

