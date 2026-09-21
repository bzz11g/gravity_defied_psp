#include "Time.h"

#include <SDL2/SDL.h>

#if defined(PSP) || defined(__PSP__)
#include <pspkernel.h>
#endif

namespace Time {
int64_t currentTimeMillis()
{
    return static_cast<int64_t>(SDL_GetTicks64());
}

void sleep(int64_t ms)
{
    if (ms <= 0) {
        return;
    }
#if defined(PSP) || defined(__PSP__)
    sceKernelDelayThread(static_cast<SceUInt>(ms * 1000));
#else
    SDL_Delay(static_cast<Uint32>(ms));
#endif
}
}