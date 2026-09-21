#ifdef SWITCH_PLATFORM
#include "java/Runtime.h"
#include <malloc.h>
Runtime Runtime::instance;
Runtime& Runtime::getRuntime() { return instance; }
long_t Runtime::maxMemory() { return 512LL * 1024LL * 1024LL; }
long_t Runtime::totalMemory() { return maxMemory(); }
long_t Runtime::freeMemory() { const struct mallinfo info = mallinfo(); return static_cast<long_t>(info.fordblks); }
#endif
