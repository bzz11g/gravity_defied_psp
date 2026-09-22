#include "Time.h"

#include <chrono>

#if defined(PSP) || defined(__PSP__)
#include <pspthreadman.h>
#else
#include <thread>
#endif

namespace Time {
int64_t currentTimeMillis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
}

void sleep(int64_t ms)
{
#if defined(PSP) || defined(__PSP__)
    sceKernelDelayThread(ms * 1000);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}
}
