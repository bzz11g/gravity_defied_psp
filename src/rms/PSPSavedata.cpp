#ifdef PSP
#include "PSPSavedata.h"
#include <pspkernel.h>

#include <cstring>
#include <string>


#include "icon0_data.h"



struct SfoHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t keyOffset;
    uint32_t valOffset;
    uint32_t count;
};

struct SfoEntry {
    uint16_t keyOffset;
    uint16_t format;
    uint32_t valLen;
    uint32_t valMaxLen;
    uint32_t valOffset;
};

std::vector<uint8_t> generateSfo() {
    std::string keys[] = {"CATEGORY", "SAVEDATA_DETAIL", "SAVEDATA_TITLE", "TITLE"};
    std::string vals[] = {"MS", "All Level Packs Progress", "Gravity Defied", "Gravity Defied"};
    uint32_t maxLens[] = {4, 1024, 128, 128};

    std::vector<uint8_t> keyBuf;
    for (const auto& k : keys) {
        for (char c : k) keyBuf.push_back(c);
        keyBuf.push_back(0);
    }
    while (keyBuf.size() % 4 != 0) keyBuf.push_back(0);

    std::vector<uint8_t> valBuf;
    std::vector<SfoEntry> entries;

    uint32_t currentKeyOff = 0;
    uint32_t currentValOff = 0;

    for (int i = 0; i < 4; ++i) {
        SfoEntry e;
        e.keyOffset = currentKeyOff;
        e.format = 0x0204;
        e.valLen = vals[i].length() + 1;
        e.valMaxLen = maxLens[i];
        e.valOffset = currentValOff;
        entries.push_back(e);

        for (char c : vals[i]) valBuf.push_back(c);
        valBuf.push_back(0);
        while (valBuf.size() < currentValOff + e.valMaxLen) valBuf.push_back(0);

        currentKeyOff += keys[i].length() + 1;
        currentValOff += e.valMaxLen;
    }

    SfoHeader h;
    h.magic = 0x46535000;
    h.version = 0x00000101;
    h.keyOffset = 20 + 16 * 4;
    h.valOffset = h.keyOffset + keyBuf.size();
    h.count = 4;

    std::vector<uint8_t> out;
    uint8_t* hp = (uint8_t*)&h;
    for (int i = 0; i < 20; ++i) out.push_back(hp[i]);

    for (const auto& e : entries) {
        uint8_t* ep = (uint8_t*)&e;
        for (int i = 0; i < 16; ++i) out.push_back(ep[i]);
    }

    out.insert(out.end(), keyBuf.begin(), keyBuf.end());
    out.insert(out.end(), valBuf.begin(), valBuf.end());

    return out;
}

void pspSilentSave(const std::vector<int8_t>& buf)
{
    std::string saveDir = "ms0:/PSP/SAVEDATA/GDEF01224";
    sceIoMkdir(saveDir.c_str(), 0777);

    // Write PARAM.SFO
    {
        SceUID fd = sceIoOpen((saveDir + "/PARAM.SFO").c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd >= 0) {
            std::vector<uint8_t> sfo = generateSfo();
            sceIoWrite(fd, sfo.data(), sfo.size());
            sceIoClose(fd);
        }
    }

    // Write ICON0.PNG
    {
        SceUID fd = sceIoOpen((saveDir + "/ICON0.PNG").c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd >= 0) {
            sceIoWrite(fd, default_icon0_png, default_icon0_png_len);
            sceIoClose(fd);
        }
    }

    // Write DATA.BIN atomically
    if(!buf.empty()) {
        std::string tmpFile = saveDir + "/DATA.BIN.tmp";
        std::string finalFile = saveDir + "/DATA.BIN";
        SceUID fd = sceIoOpen(tmpFile.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd >= 0) {
            sceIoWrite(fd, buf.data(), buf.size());
            sceIoClose(fd);
            sceIoRemove(finalFile.c_str());
            sceIoRename(tmpFile.c_str(), finalFile.c_str());
        }
    }
}
#endif
