#include "RecordStore.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <dirent.h>

#include <unistd.h>
#ifdef PSP
#include <pspkernel.h>
#elif defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
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

std::string RecordStore::getCurrentPackDir()
{
    std::string pack = packPrefix;
    if (pack.empty()) {
        pack = "original";
    } else {
        if (pack.back() == '_') {
            pack.pop_back();
        }
    }
    std::string clean;
    for (char c : pack) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') {
            clean += c;
        } else if (c == ' ') {
            clean += '_';
        }
    }
    if (clean.empty()) {
        clean = "original";
    }
    return clean;
}

void RecordStore::makeDir(const std::string& path)
{
    if (path.empty()) return;
    std::string dirPath = path;
    while (dirPath.length() > 1 && (dirPath.back() == '/' || dirPath.back() == '\\')) {
        dirPath.pop_back();
    }
#ifdef PSP
    sceIoMkdir(dirPath.c_str(), 0777);
#elif defined(_WIN32)
    _mkdir(dirPath.c_str());
#else
    mkdir(dirPath.c_str(), 0777);
#endif
}

void RecordStore::save()
{
    if (!records) {
        return;
    }

    const auto& data = records->getData();
    if (data.empty()) {
        return;
    }

    makeDir(rootSaveDir);
    makeDir(rootSaveDir + getCurrentPackDir());

    FILE* f = fopen(filePath.c_str(), "wb");
    if (!f) {
        log("Failed to open file for writing: " + filePath);
        return;
    }

    int32_t currentPos = static_cast<int32_t>(records->getCurrentPos());
    uint32_t numRecords = static_cast<uint32_t>(data.size());

    fwrite(&currentPos, sizeof(int32_t), 1, f);
    fwrite(&numRecords, sizeof(uint32_t), 1, f);

    for (size_t i = 0; i < data.size(); ++i) {
        uint32_t recSize = static_cast<uint32_t>(data[i].size());
        fwrite(&recSize, sizeof(uint32_t), 1, f);
        if (recSize > 0) {
            fwrite(data[i].data(), 1, recSize, f);
        }
    }

    fflush(f);
    fclose(f);
    log("Saved " + std::to_string(numRecords) + " records to " + filePath);
}

void RecordStore::saveAllOpened()
{
    for (auto& pair : opened) {
        if (pair.second) {
            pair.second->save();
        }
    }
}

RecordEnumerationImpl* RecordStore::load(const std::string& filePath)
{
    RecordEnumerationImpl* temp = new RecordEnumerationImpl();
    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) {
        return temp;
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fileSize <= 0) {
        fclose(f);
        return temp;
    }

    int32_t currentPos = 0;
    uint32_t numRecords = 0;
    if (fread(&currentPos, sizeof(int32_t), 1, f) == 1 &&
        fread(&numRecords, sizeof(uint32_t), 1, f) == 1) {

        temp->setCurrentPos(currentPos);
        for (uint32_t i = 0; i < numRecords; ++i) {
            uint32_t recSize = 0;
            if (fread(&recSize, sizeof(uint32_t), 1, f) == 1) {
                if (recSize > 0) {
                    std::vector<int8_t> buf(recSize);
                    if (fread(buf.data(), 1, recSize, f) == recSize) {
                        temp->addRecord(buf);
                    }
                } else {
                    temp->addRecord(std::vector<int8_t>());
                }
            }
        }
    }

    fclose(f);
    return temp;
}

void RecordStore::setPackPrefix(const std::string& prefix)
{
    packPrefix = prefix;
}

RecordStore* RecordStore::openRecordStore(std::string name, bool createIfNecessary)
{
    std::string prefixedKey = packPrefix + name;
    if (opened.find(prefixedKey) == opened.end()) {
        auto store = createRecordStore(name, createIfNecessary);
        if (!store) {
            return nullptr;
        }
        opened[prefixedKey] = std::move(store);
    }

    return opened[prefixedKey].get();
}

std::unique_ptr<RecordStore> RecordStore::createRecordStore(std::string name, bool createIfNecessary)
{
    log("createRecordStore(" + name + ", " + std::to_string(createIfNecessary) + ")");
    std::string packDir = rootSaveDir + getCurrentPackDir() + "/";
    makeDir(rootSaveDir);
    makeDir(rootSaveDir + getCurrentPackDir());

    std::string filePath = packDir + name;

    FILE* f = fopen(filePath.c_str(), "rb");
    if (f != nullptr) {
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fclose(f);

        if (sz > 0) {
            return std::unique_ptr<RecordStore>(new RecordStore(filePath, load(filePath)));
        }
    }

    if (createIfNecessary) {
        std::unique_ptr<RecordStore> rs(new RecordStore(filePath, new RecordEnumerationImpl()));
        return rs;
    } else {
        return nullptr;
    }
}

std::vector<std::string> RecordStore::listRecordStores()
{
    std::vector<std::string> result;
    std::string packDir = rootSaveDir + getCurrentPackDir() + "/";
    DIR* dir = opendir(packDir.c_str());
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
    std::string packDir = rootSaveDir + getCurrentPackDir() + "/";
    std::string filePath = packDir + name;
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
    char basePath[256] = {0};
    if (progName != nullptr && progName[0] != '\0') {
        const char* lastSlash = std::strrchr(progName, '/');
        if (!lastSlash) lastSlash = std::strrchr(progName, '\\');
        if (lastSlash != nullptr) {
            size_t len = lastSlash - progName + 1;
            if (len < sizeof(basePath)) {
                std::strncpy(basePath, progName, len);
                basePath[len] = '\0';
            }
        }
    }

    if (basePath[0] == '\0') {
#ifdef PSP
        if (getcwd(basePath, sizeof(basePath) - 1) != nullptr && basePath[0] != '\0') {
            size_t len = std::strlen(basePath);
            if (basePath[len - 1] != '/' && basePath[len - 1] != '\\') {
                std::strcat(basePath, "/");
            }
        } else {
            std::strncpy(basePath, "ms0:/PSP/GAME/GravityDefied/", sizeof(basePath) - 1);
        }
#else
        if (getcwd(basePath, sizeof(basePath) - 1) != nullptr && basePath[0] != '\0') {
            size_t len = std::strlen(basePath);
            if (basePath[len - 1] != '/' && basePath[len - 1] != '\\') {
                std::strcat(basePath, "/");
            }
        } else {
            std::strncpy(basePath, "./", sizeof(basePath) - 1);
        }
#endif
    }

    rootSaveDir = std::string(basePath) + "save/";

    makeDir(rootSaveDir);
    makeDir(rootSaveDir + getCurrentPackDir());
}
