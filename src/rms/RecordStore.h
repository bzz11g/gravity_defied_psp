#pragma once

#include <string>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "RecordEnumerationImpl.h"

class RecordFilter;
class RecordComparator;

class RecordStore {
private:
    RecordStore(std::string name, RecordEnumerationImpl* records);
    static void log(std::string s);

    inline static std::filesystem::path recordStoreDir;
    inline static std::unordered_map<std::string, std::unique_ptr<RecordEnumerationImpl>> recordsMap;

    std::string name;
    RecordEnumerationImpl* records;

public:
    inline static std::string packPrefix = "";
    static void setPackPrefix(const std::string& prefix);
    static void setRecordStoreDir(const char* progName);

    static void loadFromDisk();
    static void flushToDisk();

    static RecordStore* openRecordStore(std::string name, bool createIfNecessary);
    void closeRecordStore();
    static void deleteRecordStore(std::string name);
    static std::vector<std::string> listRecordStores();

    RecordEnumeration* enumerateRecords(RecordFilter* filter, RecordComparator* comparator, bool keepUpdated);
    int addRecord(std::vector<int8_t> arr, int offset, int numBytes);
    void setRecord(int recordId, std::vector<int8_t> arr, int offset, int numBytes);
};
