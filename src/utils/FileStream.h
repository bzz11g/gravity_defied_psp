#pragma once

#include <cstdio>
#include <cstddef>
#include <string>
#include <algorithm>

class FileStream {
public:
    FileStream()
        : file(nullptr)
    {
    }

    FileStream(const std::string& filePath, const char* mode)
        : file(std::fopen(filePath.c_str(), mode))
    {
    }

    virtual ~FileStream()
    {
        if (file) {
            std::fclose(file);
        }
    }

    FileStream(const FileStream&) = delete;
    FileStream& operator=(const FileStream&) = delete;

    template <class T>
    void readVariable(T* p, bool swapEndianness = false, std::size_t size = 0)
    {
        char* pChar = reinterpret_cast<char*>(p);
        if (!size) {
            size = sizeof(T);
        }
        read_impl(pChar, size);
        if (swapEndianness) {
            std::reverse(pChar, pChar + size);
        }
    }

    template <class T>
    void writeVariable(T* p, std::size_t size = 0)
    {
        char* pChar = reinterpret_cast<char*>(p);
        if (!size) {
            size = sizeof(T);
        }
        write_impl(pChar, size);
    }

    virtual bool isOpen()
    {
        return file != nullptr;
    }

    virtual void setPos(long pos)
    {
        if (file) {
            std::fseek(file, pos, SEEK_SET);
        }
    }

private:
    virtual void read_impl(char* s, std::size_t n)
    {
        if (file) {
            std::fread(s, 1, n, file);
        }
    }

    virtual void write_impl(char* s, std::size_t n)
    {
        if (file) {
            std::fwrite(s, 1, n, file);
        }
    }

    FILE* file;
};
