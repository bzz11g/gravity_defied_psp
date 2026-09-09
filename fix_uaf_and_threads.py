import sys

cpp_path = 'src/rms/RecordStore.cpp'
with open(cpp_path, 'r') as f:
    content = f.read()

# Fix 1: proxyMap should be cleared when deleting or proxyMap should hold the instance
content = content.replace("""void RecordStore::deleteRecordStore(std::string name)
{
    log("deleteRecordStore(" + name + ")");
    recordsMap.erase(name);
    flushToDisk();
}""", """void RecordStore::deleteRecordStore(std::string name)
{
    log("deleteRecordStore(" + name + ")");
    recordsMap.erase(name);

    // Also remove the proxy to avoid Use-After-Free
    extern std::unordered_map<std::string, std::unique_ptr<RecordStore>> g_proxyMap;
    g_proxyMap.erase(name);

    flushToDisk();
}""")

# We need to expose g_proxyMap
content = content.replace("static std::unordered_map<std::string, std::unique_ptr<RecordStore>> proxyMap;", "std::unordered_map<std::string, std::unique_ptr<RecordStore>> g_proxyMap;")
content = content.replace("proxyMap[prefixedName]", "g_proxyMap[prefixedName]")

# Fix 2: Platform-dependent serialization (size_t)
content = content.replace("size_t count = recordsMap.size();", "uint32_t count = static_cast<uint32_t>(recordsMap.size());")
content = content.replace("size_t len = pair.first.size();", "uint32_t len = static_cast<uint32_t>(pair.first.size());")
content = content.replace("size_t count = 0;\n    try {\n        inStream.readVariable(&count);", "uint32_t count = 0;\n    try {\n        inStream.readVariable(&count);")
content = content.replace("size_t len = 0;\n            inStream.readVariable(&len);", "uint32_t len = 0;\n            inStream.readVariable(&len);")


with open(cpp_path, 'w') as f:
    f.write(content)


main_path = 'src/main.cpp'
with open(main_path, 'r') as f:
    content = f.read()

content = content.replace("bool g_shouldExit = false;", "#include <atomic>\nstd::atomic<bool> g_shouldExit{false};")
with open(main_path, 'w') as f:
    f.write(content)

micro_path = 'src/Micro.cpp'
with open(micro_path, 'r') as f:
    content = f.read()

content = content.replace("extern bool g_shouldExit;", "#include <atomic>\nextern std::atomic<bool> g_shouldExit;")
with open(micro_path, 'w') as f:
    f.write(content)

menu_path = 'src/MenuManager.cpp'
with open(menu_path, 'r') as f:
    content = f.read()

content = content.replace("extern bool g_shouldExit;", "#include <atomic>\n                extern std::atomic<bool> g_shouldExit;")
with open(menu_path, 'w') as f:
    f.write(content)
