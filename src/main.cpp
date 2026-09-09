#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>

#include "Micro.h"

#ifdef PSP
#include <pspkernel.h>
PSP_MODULE_INFO("GravityDefied", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

volatile bool g_shouldExit = false;

int exit_callback(int arg1, int arg2, void *common) {
    (void)arg1; (void)arg2; (void)common;
    g_shouldExit = true;
    return 0;
}

int CallbackThread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void) {
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}
#endif

int main(int argc, char** argv)
{
#ifdef PSP
    SetupCallbacks();
#endif

    std::unique_ptr<Micro> micro = std::make_unique<Micro>();
    micro->startApp(argc, argv);

#ifdef PSP
    sceKernelExitGame();
    // Do not return from main on PSP when exiting via sceKernelExitGame.
    // Returning triggers the C library's exit(0) which implicitly calls sceKernelExitDeleteThread().
    // If sceKernelExitGame() is already tearing down the kernel, deleting the thread simultaneously
    // causes a fatal NOT_DORMANT kernel panic on real hardware.
    while (1) {
        sceKernelSleepThread();
    }
#endif
    return EXIT_SUCCESS;
};
