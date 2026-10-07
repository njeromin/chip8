#include "platform/thread.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <time.h>
#endif

void platform_sleep(long ms) {
#ifdef _WIN32
  Sleep(ms);
#else
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
#endif
}
