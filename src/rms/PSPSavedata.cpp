#include "PSPSavedata.h"

#ifdef PSP

#include <pspkernel.h>
#include <psputility.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(assets);

#define GAME_NAME  "GRAVITYDE01"
#define SAVE_NAME  ""
#define FILE_NAME  "GDTR.DAT"
#define SAVE_TITLE "Gravity Defied"
#define SAVE_SUBTITLE "Game Progress"
#define SAVE_DETAIL "Settings, highscores and progression"

void pspRunSaveDialog(int mode)
{
    // If we're loading, ensure there's actually a save file to prevent hanging
    if (mode == PSP_UTILITY_SAVEDATA_LOAD || mode == PSP_UTILITY_SAVEDATA_AUTOLOAD) {
        SceIoStat stat;
        if (sceIoGetstat("ms0:/PSP/SAVEDATA/GRAVITYDE01/GDTR.DAT", &stat) < 0) {
            return; // File doesn't exist, skip loading
        }
    } else {
        // If saving, ensure directory exists to prevent crash
        SceIoStat stat;
        if (sceIoGetstat("ms0:/PSP/SAVEDATA/GRAVITYDE01", &stat) < 0) {
            // Try to create directory
            if (sceIoMkdir("ms0:/PSP/SAVEDATA/GRAVITYDE01", 0777) < 0) {
                // If mkdir fails but we need intermediate dirs?
                sceIoMkdir("ms0:/PSP/SAVEDATA", 0777);
                sceIoMkdir("ms0:/PSP/SAVEDATA/GRAVITYDE01", 0777);
            }
        }
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

    if (mode == PSP_UTILITY_SAVEDATA_SAVE || mode == PSP_UTILITY_SAVEDATA_AUTOSAVE) {
        auto fs = cmrc::assets::get_filesystem();
        auto icon0 = fs.open("assets/psp_logo.png");
        params.icon0FileData.buf = std::malloc(icon0.size());
        std::memcpy(params.icon0FileData.buf, icon0.begin(), icon0.size());
        params.icon0FileData.bufSize = icon0.size();
        params.icon0FileData.size = icon0.size();

        auto pic1 = fs.open("assets/psp_bg.png");
        params.pic1FileData.buf = std::malloc(pic1.size());
        std::memcpy(params.pic1FileData.buf, pic1.begin(), pic1.size());
        params.pic1FileData.bufSize = pic1.size();
        params.pic1FileData.size = pic1.size();
    } else {
        params.icon0FileData.buf = nullptr;
        params.icon0FileData.bufSize = 0;
        params.icon0FileData.size = 0;

        params.pic1FileData.buf = nullptr;
        params.pic1FileData.bufSize = 0;
        params.pic1FileData.size = 0;
    }

    params.icon1FileData.buf = nullptr;
    params.icon1FileData.bufSize = 0;
    params.icon1FileData.size = 0;

    params.snd0FileData.buf = nullptr;
    params.snd0FileData.bufSize = 0;
    params.snd0FileData.size = 0;

    alignas(64) char dummyBuf[256] = {0};
    params.dataBuf = dummyBuf;
    params.dataBufSize = sizeof(dummyBuf);
    params.dataSize = sizeof(dummyBuf);

    sceUtilitySavedataInitStart(&params);

    while (true) {
        int status = sceUtilitySavedataGetStatus();
        if (status == PSP_UTILITY_DIALOG_VISIBLE) {
            sceUtilitySavedataUpdate(2); // Fix possible crash by changing to Update(2)
        } else if (status == PSP_UTILITY_DIALOG_QUIT) {
            sceUtilitySavedataShutdownStart();
        } else if (status == PSP_UTILITY_DIALOG_FINISHED ||
                   status == PSP_UTILITY_DIALOG_NONE) {
            break;
        }
        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
    }

    if (params.icon0FileData.buf != nullptr) {
        std::free(params.icon0FileData.buf);
    }
    if (params.pic1FileData.buf != nullptr) {
        std::free(params.pic1FileData.buf);
    }
}

#endif
