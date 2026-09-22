#include "pspSetup.h"
#include "glib2d.h"
#include "intra/intraFont.h"
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include "../Micro.h"

int exit_callback(int arg1, int arg2, void *common) {
    (void)arg1; (void)arg2; (void)common;
    Micro::field_249 = false;
    return 0;
}

int CallbackThread(SceSize args, void *argp) {
    (void)args; (void)argp;
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

void InitGame(void) {
    SetupCallbacks();
    scePowerSetClockFrequency(333, 333, 166);
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    g2dInit();
    intraFontInit();
}

void ExitGame(void) {
    intraFontShutdown();
    g2dTerm();
    sceKernelExitGame();
}
