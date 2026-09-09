#ifdef PSP
#include "PSPSavedata.h"
#include <pspkernel.h>

#include <cstring>
#include <string>


#include "icon0_data.h"



#include "sfo_data.h"

void pspSilentSave(const std::vector<int8_t>& buf)
{
    std::string saveDir = "ms0:/PSP/SAVEDATA/GDEF01224";
    sceIoMkdir(saveDir.c_str(), 0777);

    // Write PARAM.SFO
    {
        SceUID fd = sceIoOpen((saveDir + "/PARAM.SFO").c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fd >= 0) {
            sceIoWrite(fd, default_sfo_data, default_sfo_data_len);
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
