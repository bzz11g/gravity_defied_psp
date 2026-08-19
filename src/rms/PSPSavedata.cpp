#include "PSPSavedata.h"

#ifdef PSP

#include <pspkernel.h>
#include <psputility.h>
#include <cstring>
#include <cstdio>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(assets);

#define GAME_NAME  "GRAVITYDE01"
#define SAVE_NAME  ""
#define FILE_NAME  "GDTR.DAT"
#define SAVE_TITLE "Gravity Defied"
#define SAVE_SUBTITLE "Game Progress"
#define SAVE_DETAIL "Settings, highscores and progression"

static bool saveDirInitialized = false;

void pspRunSaveDialog(int mode)
{
    if (mode == PSP_UTILITY_SAVEDATA_AUTOLOAD && saveDirInitialized) {
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

    auto fs = cmrc::assets::get_filesystem();
    auto icon0 = fs.open("assets/psp_logo.png");
    params.icon0FileData.buf = (void*)icon0.begin();
    params.icon0FileData.bufSize = icon0.size();
    params.icon0FileData.size = icon0.size();
    params.icon1FileData.buf = nullptr;
    params.icon1FileData.bufSize = 0;
    params.icon1FileData.size = 0;

    auto pic1 = fs.open("assets/psp_bg.png");
    params.pic1FileData.buf = (void*)pic1.begin();
    params.pic1FileData.bufSize = pic1.size();
    params.pic1FileData.size = pic1.size();
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

    if (mode == PSP_UTILITY_SAVEDATA_AUTOLOAD) {
        saveDirInitialized = true;
    }
}

void pspEnsureSaveDirectory()
{
    pspRunSaveDialog(PSP_UTILITY_SAVEDATA_AUTOLOAD);
}

#endif
