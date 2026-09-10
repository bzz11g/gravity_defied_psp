#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>

#include "Micro.h"

#ifdef PSP
#include <pspkernel.h>
PSP_MODULE_INFO("GravityDefied", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int exit_callback(int arg1, int arg2, void *common) {
    Micro::field_249 = false;
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

    try {
        std::unique_ptr<Micro> micro = std::make_unique<Micro>();
        micro->startApp(argc, argv);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

#ifdef PSP
    sceKernelExitGame();
#endif

    return EXIT_SUCCESS;
};
