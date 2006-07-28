#ifdef WIN32
#include <time.h>
#else

#include <unistd.h>
#include <sys/time.h>


#define RESETTIME() startTime()
#define REPORTTIME(comment) endTime(comment,__FILE__,__LINE__)

static timeval start;
static inline void startTime() {
  gettimeofday(&start, NULL);
}

static inline void endTime(const char* comment, const char* file, int lineno) {
  timeval end;
  gettimeofday(&end, NULL);
  double time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
  std::clog << file << "(" << comment << "):" << lineno << ": " << time << std::endl;
}
#endif
