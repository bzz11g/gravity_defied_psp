#include <memory>
#include <string>
#include <iostream>

#include "Micro.h"
#include "psp/pspSetup.h"

#if defined(PSP) || defined(__PSP__)
#include <pspkernel.h>
PSP_MODULE_INFO("GravityDefied", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-1024);
#endif

int main(int argc, char** argv)
{
    InitGame();

    std::unique_ptr<Micro> micro = std::make_unique<Micro>();
    micro->startApp(argc, argv);

    ExitGame();

    return 0;
}
