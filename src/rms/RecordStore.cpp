#include "RecordStore.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <cstring>
#include <dirent.h>

#ifdef WIN32
#include <libgen.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#endif

#ifdef PSP
#include <pspkernel.h>
#include <pspuser.h>
#include "../sfo_data.h"
#include "../icon0_data.h"
#endif

#include "RecordStoreException.h"
#include "../utils/FileStream.h"
#include "../utils/String.h"

bool g_saveSystemInitialized = false;

RecordStore::RecordStore(std::string name, RecordEnumerationImpl* records)
{
    this->name = name;
    this->records.reset(records);
}

RecordEnumeration* RecordStore::enumerateRecords(RecordFilter* filter, RecordComparator* comparator, bool keepUpdated)
{
    assert(filter == nullptr);
    assert(comparator == nullptr);
    assert(!keepUpdated);
    log("enumerateRecords()");
    return records.get();
}

void RecordStore::closeRecordStore()
{
    // nothing
}

int RecordStore::addRecord(std::vector<int8_t> arr, int offset, int numBytes)
{
    log("addRecord()");
    assert(static_cast<int>(arr.size()) == numBytes);
    assert(offset == 0);
    int id = records->addRecord(arr);
    saveAll();
    return id;
}

void RecordStore::setRecord(int recordId, std::vector<int8_t> arr, int offset, int numBytes)
{
    (void)offset;
    (void)numBytes;
    records->setRecord(recordId, arr);
    saveAll();
}

void RecordStore::saveAll()
{
    flushToDisk();
}

void RecordStore::init()
{
    if (g_saveSystemInitialized) return;

    std::string dataPath = recordStoreDir + "/DATA.BIN";

    FILE* f = fopen(dataPath.c_str(), "rb");
    if (f) {
        uint32_t mapSize;
        if (fread(&mapSize, sizeof(uint32_t), 1, f) == 1) {
            for (size_t i = 0; i < mapSize; ++i) {
                uint32_t keyLen;
                if (fread(&keyLen, sizeof(uint32_t), 1, f) != 1) break;
                char* keyBuf = new char[keyLen + 1];
                if (fread(keyBuf, 1, keyLen, f) != keyLen) { delete[] keyBuf; break; }
                keyBuf[keyLen] = '\0';
                std::string key(keyBuf);
                delete[] keyBuf;

                uint32_t numRecs;
                if (fread(&numRecs, sizeof(uint32_t), 1, f) != 1) break;

                RecordEnumerationImpl* recs = new RecordEnumerationImpl();
                for (size_t j = 0; j < numRecs; ++j) {
                    uint32_t recLen;
                    if (fread(&recLen, sizeof(uint32_t), 1, f) != 1) break;
                    std::vector<int8_t> recData(recLen);
                    if (recLen > 0) {
                        if (fread(recData.data(), 1, recLen, f) != recLen) break;
                    }
                    recs->addRecord(recData);
                }

                opened[key] = std::unique_ptr<RecordStore>(new RecordStore(key, recs));
            }
        }
        fclose(f);
    }

    g_saveSystemInitialized = true;
}

void RecordStore::flushToDisk()
{
    if (!g_saveSystemInitialized) return;

#ifdef PSP
    sceIoMkdir(recordStoreDir.c_str(), 0777);

    std::string sfoPath = recordStoreDir + "/PARAM.SFO";
    int fd = sceIoOpen(sfoPath.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, param_sfo_data, sizeof(param_sfo_data));
        sceIoClose(fd);
    }

    std::string icon0Path = recordStoreDir + "/ICON0.PNG";
    fd = sceIoOpen(icon0Path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, icon0_data, sizeof(icon0_data));
        sceIoClose(fd);
    }

    std::string dataPath = recordStoreDir + "/DATA.BIN";
    std::string tmpPath = dataPath + ".tmp";

    fd = sceIoOpen(tmpPath.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0) {
        uint32_t mapSize = opened.size();
        sceIoWrite(fd, &mapSize, sizeof(uint32_t));

        for (const auto& pair : opened) {
            if (!pair.second) continue;
            uint32_t keyLen = pair.first.length();
            sceIoWrite(fd, &keyLen, sizeof(uint32_t));
            sceIoWrite(fd, pair.first.c_str(), keyLen);

            RecordEnumerationImpl* recs = pair.second->records.get();
            uint32_t numRecs = recs->data.size();
            sceIoWrite(fd, &numRecs, sizeof(uint32_t));

            for (size_t i = 0; i < numRecs; ++i) {
                uint32_t recLen = recs->data[i].size();
                sceIoWrite(fd, &recLen, sizeof(uint32_t));
                if (recLen > 0) {
                    sceIoWrite(fd, recs->data[i].data(), recLen);
                }
            }
        }
        sceIoClose(fd);

        sceIoRemove(dataPath.c_str());
        sceIoRename(tmpPath.c_str(), dataPath.c_str());
    }
#else
    std::string dataPath = recordStoreDir + "/DATA.BIN";
    std::string tmpPath = dataPath + ".tmp";

    FILE* f = fopen(tmpPath.c_str(), "wb");
    if (f) {
        uint32_t mapSize = opened.size();
        fwrite(&mapSize, sizeof(uint32_t), 1, f);

        for (const auto& pair : opened) {
            if (!pair.second) continue;
            uint32_t keyLen = pair.first.length();
            fwrite(&keyLen, sizeof(uint32_t), 1, f);
            fwrite(pair.first.c_str(), 1, keyLen, f);

            RecordEnumerationImpl* recs = pair.second->records.get();
            uint32_t numRecs = recs->data.size();
            fwrite(&numRecs, sizeof(uint32_t), 1, f);

            for (size_t i = 0; i < numRecs; ++i) {
                uint32_t recLen = recs->data[i].size();
                fwrite(&recLen, sizeof(uint32_t), 1, f);
                if (recLen > 0) {
                    fwrite(recs->data[i].data(), 1, recLen, f);
                }
            }
        }
        fclose(f);

        remove(dataPath.c_str());
        rename(tmpPath.c_str(), dataPath.c_str());
    }
#endif
}

RecordEnumerationImpl* RecordStore::load()
{
    return new RecordEnumerationImpl();
}

void RecordStore::setPackPrefix(const std::string& prefix)
{
    packPrefix = prefix;
}

RecordStore* RecordStore::openRecordStore(std::string name, bool createIfNecessary)
{
    init();

    std::string prefixedName = packPrefix + name;
    if (opened.find(prefixedName) == opened.end()) {
        opened[prefixedName] = createRecordStore(prefixedName, createIfNecessary);
    }

    return opened[prefixedName].get();
}

std::unique_ptr<RecordStore> RecordStore::createRecordStore(std::string name, bool createIfNecessary)
{
    log("createRecordStore(" + name + ", " + std::to_string(createIfNecessary) + ")");

    if (createIfNecessary) {
        return std::unique_ptr<RecordStore>(new RecordStore(name, new RecordEnumerationImpl()));
    } else {
        return nullptr;
    }
}

std::vector<std::string> RecordStore::listRecordStores()
{
    std::vector<std::string> result;
    for (const auto& pair : opened) {
        result.push_back(pair.first);
    }
    return result;
}

void RecordStore::deleteRecordStore(std::string name)
{
    log("deleteRecordStore(" + name + ")");
    opened.erase(name);
}

void RecordStore::log(std::string s)
{
    std::cout << s << std::endl;
}

void RecordStore::setRecordStoreDir([[maybe_unused]] const char* progName)
{
#ifdef PSP
    recordStoreDir = "ms0:/PSP/SAVEDATA/GDEF01224";
#else
    #ifdef WIN32
        const char* base = dirname(strdup(progName));
        recordStoreDir = std::string(base) + "/savedata";
    #else
        recordStoreDir = "savedata";
    #endif
    mkdir(recordStoreDir.c_str(), 0777);
#endif
}
