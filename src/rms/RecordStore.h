#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "RecordEnumerationImpl.h"

class RecordFilter;
class RecordComparator;

class RecordStore {
private:
    RecordStore(std::string filePath, RecordEnumerationImpl* records);
    void save();
    static RecordEnumerationImpl* load(const std::string& filePath);
    static std::unique_ptr<RecordStore> createRecordStore(std::string name, bool createIfNecessary);
    static void log(std::string s);

    inline static std::string recordStoreDir;
    inline static std::unordered_map<std::string, std::unique_ptr<RecordStore>> opened;
    std::string filePath;
    std::unique_ptr<RecordEnumerationImpl> records;

public:
    inline static std::string packPrefix = "";
    static void setPackPrefix(const std::string& prefix);
    static void setRecordStoreDir(const char* progName);
    static RecordStore* openRecordStore(std::string name, bool createIfNecessary);
    void closeRecordStore();
    static void deleteRecordStore(std::string name);
    static std::vector<std::string> listRecordStores();
    RecordEnumeration* enumerateRecords(RecordFilter* filter, RecordComparator* comparator, bool keepUpdated);
    int addRecord(std::vector<int8_t> arr, int offset, int numBytes);
    void setRecord(int recordId, std::vector<int8_t> arr, int offset, int numBytes);
};