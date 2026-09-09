#include "RecordStore.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <cstring>

#ifdef WIN32
#include <libgen.h>
#else
#include <unistd.h>
#include <pwd.h>
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
    std::filesystem::path filePath = recordStoreDir / "DATA.BIN";
    std::filesystem::create_directories(filePath.parent_path());
    std::ofstream os(filePath, std::ios::out | std::ios::binary);
    if(os.is_open() && !buf.empty()) {
        os.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    }
#endif
}

void RecordStore::loadFromDisk()
{
    recordsMap.clear();

    std::vector<int8_t> buf;

    std::filesystem::path filePath = recordStoreDir / "DATA.BIN";
    std::error_code ec;
    if (!std::filesystem::exists(filePath, ec) || ec) return;

    std::ifstream is(filePath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!is.is_open()) return;

    std::streamsize size = is.tellg();
    if (size <= 0) return;

    is.seekg(0, std::ios::beg);
    buf.resize(size);
    if (is.read(reinterpret_cast<char*>(buf.data()), size)) {
        // successfully read
    }

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
