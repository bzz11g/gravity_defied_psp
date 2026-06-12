#include "RecordStore.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <cstring>
#include <string>

#ifdef WIN32
#include <libgen.h>
#include <direct.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#ifdef PSP
#include "PSPSavedata.h"
#endif

#include "RecordStoreException.h"
#include "../utils/FileStream.h"
#include "../utils/String.h"

static bool createDirectories(const std::string& path)
{
#ifdef WIN32
    std::string::size_type pos = 0;
    while (true) {
        pos = path.find_first_of("/\\", pos + 1);
        if (pos == std::string::npos) {
            break;
        }
        if (pos == 0) continue;
        std::string subdir = path.substr(0, pos);
        if (::mkdir(subdir.c_str()) != 0 && errno != EEXIST) {
            return false;
        }
    }
    if (::mkdir(path.c_str()) != 0 && errno != EEXIST) {
        return false;
    }
    return true;
#else
    std::string::size_type pos = 0;
    while (true) {
        pos = path.find('/', pos + 1);
        if (pos == std::string::npos) {
            break;
        }
        if (pos == 0) continue;
        std::string subdir = path.substr(0, pos);
        if (::mkdir(subdir.c_str(), 0755) != 0 && errno != EEXIST) {
            return false;
        }
    }
    if (::mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        return false;
    }
    return true;
#endif
}

static std::string parentPath(const std::string& path)
{
    std::string::size_type pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return ".";
    }
    if (pos == 0) {
        return "/";
    }
    return path.substr(0, pos);
}

static std::vector<std::string> listDirectory(const std::string& dirPath)
{
    std::vector<std::string> result;
#ifdef WIN32
    // Not implemented for WIN32
    (void)dirPath;
#else
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        return result;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name(entry->d_name);
        if (name != "." && name != "..") {
            result.push_back(name);
        }
    }
    closedir(dir);
#endif
    return result;
}

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
    // nothing
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
    FileStream outStream(filePath, "wb");
    records->serialize(&outStream);
}

RecordEnumerationImpl* RecordStore::load(std::string filePath)
{
    RecordEnumerationImpl* temp = new RecordEnumerationImpl();
    FileStream inStream(filePath, "rb");
    temp->deserialize(&inStream);
    return temp;
}

RecordStore* RecordStore::openRecordStore(std::string name, bool createIfNecessary)
{
    if (opened.find(name) == opened.end()) {
        opened[name] = createRecordStore(name, createIfNecessary);
    }

    return opened[name].get();
}

std::unique_ptr<RecordStore> RecordStore::createRecordStore(std::string name, bool createIfNecessary)
{
    log("createRecordStore(" + name + ", " + std::to_string(createIfNecessary) + ")");
    std::string filePath = recordStoreDir + "/" + name;

    if (access(filePath.c_str(), F_OK) == 0) {
        return std::unique_ptr<RecordStore>(new RecordStore(filePath, load(filePath)));
    }

    if (createIfNecessary) {
        createDirectories(parentPath(filePath));

        std::unique_ptr<RecordStore> rs(new RecordStore(filePath, new RecordEnumerationImpl()));
        rs->save();
        return rs;
    } else {
        throw RecordStoreException();
    }
}

std::vector<std::string> RecordStore::listRecordStores()
{
    std::vector<std::string> result = listDirectory(recordStoreDir);

    log("listRecordStores() = {" + String::join(result, ", ") + "}");

    return result;
}

void RecordStore::deleteRecordStore(std::string name)
{
    log("deleteRecordStore(" + name + ")");
    throw std::runtime_error("deleteRecordStore is not implemented");
}

void RecordStore::log(std::string s)
{
    std::cout << s << std::endl;
}

void RecordStore::setRecordStoreDir([[maybe_unused]] const char* progName)
{
#ifdef PSP
    recordStoreDir = "ms0:/PSP/SAVEDATA/GRAVITYDE01";
    pspEnsureSaveDirectory();
#elif WIN32
    const char* base = dirname(strdup(progName));
    recordStoreDir = std::string(base) + "/recordStore";
#else
    const char* homeDir = getenv("HOME");
    if (!homeDir)
        homeDir = getpwuid(getuid())->pw_dir;

    if (!homeDir)
        throw std::runtime_error("Error getting home directory");

    recordStoreDir = std::string(homeDir) + "/.GravityDefied";
#endif
}
