import sys
cpp_path = 'src/rms/RecordStore.cpp'
with open(cpp_path, 'r') as f:
    content = f.read()

# We need to make g_proxyMap a static class member or a local static correctly.
# If we declare it extern, it fails to link because we didn't define it.
# It was originally `std::unordered_map<std::string, std::unique_ptr<RecordStore>> g_proxyMap;`
# in RecordStore.cpp inside openRecordStore. But we moved it? Let's just define it at the top of the file.

content = content.replace("extern std::unordered_map<std::string, std::unique_ptr<RecordStore>> g_proxyMap;", "")

top_define = """std::unordered_map<std::string, std::unique_ptr<RecordStore>> g_proxyMap;

RecordStore::RecordStore(std::string name, RecordEnumerationImpl* records)"""

content = content.replace("RecordStore::RecordStore(std::string name, RecordEnumerationImpl* records)", top_define)
content = content.replace("std::unordered_map<std::string, std::unique_ptr<RecordStore>> g_proxyMap;\n    if (g_proxyMap.find", "if (g_proxyMap.find")

with open(cpp_path, 'w') as f:
    f.write(content)
