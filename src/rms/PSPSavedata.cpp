#include "PSPSavedata.h"

#ifdef PSP

#include <pspkernel.h>
#include <psputility.h>
#include <cstring>
#include <cstdio>

#define GAME_NAME  "GRAVITYDE01"
#define SAVE_NAME  ""
#define FILE_NAME  "GDTR.DAT"
#define SAVE_TITLE "Gravity Defied"
#define SAVE_SUBTITLE "Game Progress"
#define SAVE_DETAIL "Settings, highscores and progression"

static bool saveDirInitialized = false;

void pspRunSaveDialog(int mode)
{
    if (mode == SCE_UTILITY_SAVEDATA_MAKEDATA && saveDirInitialized) {
        return;
    }

    SceUtilitySavedataParam params;
    std::memset(&params, 0, sizeof(params));
    params.base.size = sizeof(params);
    params.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
    params.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
    params.base.graphicsThread = 0x11;
    params.base.accessThread = 0x13;
    params.base.fontThread = 0x12;
    params.base.soundThread = 0x10;

    params.mode = static_cast<PspUtilitySavedataMode>(mode);
    params.overwrite = 1;
    params.focus = PSP_UTILITY_SAVEDATA_FOCUS_LATEST;

    std::strncpy(params.gameName, GAME_NAME, sizeof(params.gameName) - 1);
    std::strncpy(params.saveName, SAVE_NAME, sizeof(params.saveName) - 1);
    std::strncpy(params.fileName, FILE_NAME, sizeof(params.fileName) - 1);

    std::strncpy(params.sfoParam.title, SAVE_TITLE, sizeof(params.sfoParam.title) - 1);
    std::strncpy(params.sfoParam.savedataTitle, SAVE_SUBTITLE, sizeof(params.sfoParam.savedataTitle) - 1);
    std::strncpy(params.sfoParam.detail, SAVE_DETAIL, sizeof(params.sfoParam.detail) - 1);
    params.sfoParam.parentalLevel = 1;

    params.icon0FileData.buf = nullptr;
    params.icon0FileData.bufSize = 0;
    params.icon0FileData.size = 0;
    params.icon1FileData.buf = nullptr;
    params.icon1FileData.bufSize = 0;
    params.icon1FileData.size = 0;
    params.pic1FileData.buf = nullptr;
    params.pic1FileData.bufSize = 0;
    params.pic1FileData.size = 0;
    params.snd0FileData.buf = nullptr;
    params.snd0FileData.bufSize = 0;
    params.snd0FileData.size = 0;

    char dummyBuf[256] = {0};
    params.dataBuf = dummyBuf;
    params.dataBufSize = sizeof(dummyBuf);
    params.dataSize = sizeof(dummyBuf);

    sceUtilitySavedataInitStart(&params);

    while (true) {
        int status = sceUtilitySavedataGetStatus();
        if (status == PSP_UTILITY_DIALOG_VISIBLE) {
            sceUtilitySavedataUpdate(1);
        } else if (status == PSP_UTILITY_DIALOG_QUIT) {
            sceUtilitySavedataShutdownStart();
        } else if (status == PSP_UTILITY_DIALOG_FINISHED ||
                   status == PSP_UTILITY_DIALOG_NONE) {
            break;
        }
        sceKernelDelayThread(0);
    }

    if (mode == SCE_UTILITY_SAVEDATA_MAKEDATA) {
        saveDirInitialized = true;
    }
}

void pspEnsureSaveDirectory()
{
    pspRunSaveDialog(SCE_UTILITY_SAVEDATA_MAKEDATA);
}

void pspAutosave()
{
    pspRunSaveDialog(SCE_UTILITY_SAVEDATA_AUTOSAVE);
}

#endif
