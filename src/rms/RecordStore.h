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
    RecordStore(std::string name, RecordEnumerationImpl* records);
    static RecordEnumerationImpl* load();
    static std::unique_ptr<RecordStore> createRecordStore(std::string name, bool createIfNecessary);
    static void log(std::string s);

    inline static std::string recordStoreDir;
    inline static std::unordered_map<std::string, std::unique_ptr<RecordStore>> opened;
    std::string name;
    std::unique_ptr<RecordEnumerationImpl> records;

public:
    static void saveAll();
    static void init();
    static void flushToDisk();


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