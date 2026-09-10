#include "RecordStore.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <dirent.h>

#ifdef PSP
#include <pspkernel.h>
#elif defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "../utils/FileStream.h"
#include "../utils/String.h"

RecordStore::RecordStore(std::string filePath, RecordEnumerationImpl* records)
{
    this->filePath = filePath;
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
    save();
}

int RecordStore::addRecord(std::vector<int8_t> arr, int offset, int numBytes)
{
    log("addRecord()");
    assert(static_cast<int>(arr.size()) == numBytes);
    assert(offset == 0);
    int id = records->addRecord(arr);
    save();
    return id;
}

void RecordStore::setRecord(int recordId, std::vector<int8_t> arr, int offset, int numBytes)
{
    (void)offset;
    (void)numBytes;
    records->setRecord(recordId, arr);
    save();
}

void RecordStore::save()
{
    FileStream outStream(filePath, std::ios::out | std::ios::binary);
    records->serialize(&outStream);
}

RecordEnumerationImpl* RecordStore::load(const std::string& filePath)
{
    RecordEnumerationImpl* temp = new RecordEnumerationImpl();
    FileStream inStream(filePath, std::ios::in | std::ios::binary);
    temp->deserialize(&inStream);
    return temp;
}

void RecordStore::setPackPrefix(const std::string& prefix)
{
    packPrefix = prefix;
}

RecordStore* RecordStore::openRecordStore(std::string name, bool createIfNecessary)
{
    std::string prefixedName = packPrefix + name;
    if (opened.find(prefixedName) == opened.end()) {
        auto store = createRecordStore(prefixedName, createIfNecessary);
        if (!store) {
            return nullptr;
        }
        opened[prefixedName] = std::move(store);
    }

    return opened[prefixedName].get();
}

std::unique_ptr<RecordStore> RecordStore::createRecordStore(std::string name, bool createIfNecessary)
{
    log("createRecordStore(" + name + ", " + std::to_string(createIfNecessary) + ")");
    std::string filePath = recordStoreDir + name;

    FILE* f = fopen(filePath.c_str(), "rb");
    if (f != nullptr) {
        fclose(f);
        return std::unique_ptr<RecordStore>(new RecordStore(filePath, load(filePath)));
    }

    if (createIfNecessary) {
        std::unique_ptr<RecordStore> rs(new RecordStore(filePath, new RecordEnumerationImpl()));
        rs->save();
        return rs;
    } else {
        return nullptr;
    }
}

std::vector<std::string> RecordStore::listRecordStores()
{
    std::vector<std::string> result;
    DIR* dir = opendir(recordStoreDir.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (std::strcmp(entry->d_name, ".") != 0 && std::strcmp(entry->d_name, "..") != 0) {
                result.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }

    log("listRecordStores() = {" + String::join(result, ", ") + "}");

    return result;
}

void RecordStore::deleteRecordStore(std::string name)
{
    log("deleteRecordStore(" + name + ")");
    std::string filePath = recordStoreDir + name;
#ifdef PSP
    sceIoRemove(filePath.c_str());
#else
    std::remove(filePath.c_str());
#endif
}

void RecordStore::log(std::string s)
{
    std::cout << s << std::endl;
}

void RecordStore::setRecordStoreDir(const char* progName)
{
    std::string appDir = "./";
    if (progName != nullptr && progName[0] != '\0') {
        std::string strProg(progName);
        size_t lastSlash = strProg.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            appDir = strProg.substr(0, lastSlash + 1);
        }
    }

    recordStoreDir = appDir + "save/";

#ifdef PSP
    sceIoMkdir(recordStoreDir.c_str(), 0777);
#elif defined(_WIN32)
    _mkdir(recordStoreDir.c_str());
#else
    mkdir(recordStoreDir.c_str(), 0777);
#endif
}
