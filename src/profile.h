#include <sys/time.h>
#include <unistd.h>

#define STARTTIMER() startTime()
#define ENDTIMER() endTime(__FILE__,__LINE__)

static timeval start;
static inline void startTime() {
  gettimeofday(&start, NULL);
}

static inline void endTime(const char* file, int lineno) {
  timeval end;
  gettimeofday(&end, NULL);
  double time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
  clog << "Tick at " << file << ":" << lineno << ": " << time << endl;
}
