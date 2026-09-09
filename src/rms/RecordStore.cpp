#include "RecordStore.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <cstring>

#ifdef WIN32
#include <libgen.h>
#include <direct.h>
#define mkdir _mkdir
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#endif

#ifdef PSP
#include <pspkernel.h>
#endif

#include "RecordStoreException.h"
#include "../utils/FileStream.h"
#include "../utils/String.h"
#include "../utils/BufferStream.h"

#ifdef PSP
#include "PSPSavedata.h"
#endif

std::unordered_map<std::string, std::unique_ptr<RecordStore>> g_proxyMap;

RecordStore::RecordStore(std::string name, RecordEnumerationImpl* records)
{
    this->name = name;
    this->records = records;
}

RecordEnumeration* RecordStore::enumerateRecords(RecordFilter* filter, RecordComparator* comparator, bool keepUpdated)
{
    assert(filter == nullptr);
    assert(comparator == nullptr);
    assert(!keepUpdated);
    log("enumerateRecords()");
    return records;
}

void RecordStore::closeRecordStore()
{
    // Do not free anything, it lives in recordsMap
}

int RecordStore::addRecord(std::vector<int8_t> arr, int offset, int numBytes)
{
    log("addRecord()");
    assert(static_cast<int>(arr.size()) == numBytes);
    assert(offset == 0);
    int id = records->addRecord(arr);
    flushToDisk();
    return id;
}

void RecordStore::setRecord(int recordId, std::vector<int8_t> arr, int offset, int numBytes)
{
    (void)offset;
    (void)numBytes;
    records->setRecord(recordId, arr);
    flushToDisk();
}

void RecordStore::flushToDisk()
{
    BufferStream outStream(std::ios::out | std::ios::binary);

    uint32_t count = static_cast<uint32_t>(recordsMap.size());
    outStream.writeVariable(&count);

    for (auto& pair : recordsMap) {
        uint32_t len = static_cast<uint32_t>(pair.first.size());
        outStream.writeVariable(&len);
        for (char c : pair.first) {
            int8_t val = (int8_t)c;
            outStream.writeVariable(&val);
        }
        pair.second->serialize(&outStream);
    }

    std::vector<int8_t> buf = outStream.getBuffer();

#ifdef PSP
    pspSilentSave(buf);
#else
    std::string filePath = recordStoreDir + "/DATA.BIN";
    // For non-PSP fallback, mkdir if needed
    // #include <sys/stat.h> should be included
    #ifdef WIN32
    mkdir(recordStoreDir.c_str());
    #else
    mkdir(recordStoreDir.c_str(), 0777);
    #endif
    if (!buf.empty()) {
        FILE* fp = fopen(filePath.c_str(), "wb");
        if (fp) {
            fwrite(buf.data(), 1, buf.size(), fp);
            fclose(fp);
        }
    }
#endif
}

void RecordStore::loadFromDisk()
{
    recordsMap.clear();

    std::vector<int8_t> buf;

    std::string filePath = recordStoreDir + "/DATA.BIN";
#ifdef PSP
    SceIoStat stat;
    if (sceIoGetstat(filePath.c_str(), &stat) < 0) return;
#else
    if (access(filePath.c_str(), F_OK) != 0) return;
#endif

#ifdef PSP
    SceUID fd = sceIoOpen(filePath.c_str(), PSP_O_RDONLY, 0777);
    if (fd < 0) return;

    SceOff size = sceIoLseek(fd, 0, PSP_SEEK_END);
    if (size <= 0) {
        sceIoClose(fd);
        return;
    }

    sceIoLseek(fd, 0, PSP_SEEK_SET);
    buf.resize(size);
    sceIoRead(fd, buf.data(), size);
    sceIoClose(fd);
#else
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return;
    }

    fseek(fp, 0, SEEK_SET);
    buf.resize(size);
    fread(buf.data(), 1, size, fp);
    fclose(fp);
#endif

    if (buf.empty()) return;

    BufferStream inStream(buf, std::ios::in | std::ios::binary);
    uint32_t count = 0;
    try {
        inStream.readVariable(&count);
        for (size_t i = 0; i < count; i++) {
            uint32_t len = 0;
            inStream.readVariable(&len);
            std::string key;
            for (size_t j = 0; j < len; j++) {
                int8_t val;
                inStream.readVariable(&val);
                key.push_back((char)val);
            }
            auto records = std::make_unique<RecordEnumerationImpl>();
            records->deserialize(&inStream);
            recordsMap[key] = std::move(records);
        }
    } catch (...) {
        // format error
    }
}

void RecordStore::setPackPrefix(const std::string& prefix)
{
    packPrefix = prefix;
}

RecordStore* RecordStore::openRecordStore(std::string name, bool createIfNecessary)
{
    std::string prefixedName = packPrefix + name;

    if (recordsMap.empty()) {
        static bool loaded = false;
        if (!loaded) {
            loaded = true;
            loadFromDisk();
        }
    }

    if (recordsMap.find(prefixedName) == recordsMap.end()) {
        if (createIfNecessary) {
            recordsMap[prefixedName] = std::make_unique<RecordEnumerationImpl>();
        } else {
            throw RecordStoreException();
        }
    }

    // Return a proxy object. Memory leak?
    // The original code: opened[prefixedName] = std::unique_ptr<RecordStore>(new RecordStore...);
    // So caller doesn't delete it? Caller of `openRecordStore` just gets a pointer.
    // Yes, RecordManager keeps it as a raw pointer and occasionally calls `closeRecordStore`.
    // We will just return a dynamically allocated proxy that the caller can safely discard or keep.
    // Wait, the original code cached `opened[prefixedName]` as a singleton `RecordStore`.
    // Let's implement a static map for proxy objects.
    if (g_proxyMap.find(prefixedName) == g_proxyMap.end()) {
        g_proxyMap[prefixedName] = std::unique_ptr<RecordStore>(new RecordStore(prefixedName, recordsMap[prefixedName].get()));
    }
    return g_proxyMap[prefixedName].get();
}

std::vector<std::string> RecordStore::listRecordStores()
{
    std::vector<std::string> result;
    for (const auto& pair : recordsMap) {
        result.push_back(pair.first);
    }
    log("listRecordStores() = {" + String::join(result, ", ") + "}");
    return result;
}

void RecordStore::deleteRecordStore(std::string name)
{
    log("deleteRecordStore(" + name + ")");
    recordsMap.erase(name);

    // Also remove the proxy to avoid Use-After-Free

    g_proxyMap.erase(name);

    flushToDisk();
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
    recordStoreDir = "savedata";
#endif
}
