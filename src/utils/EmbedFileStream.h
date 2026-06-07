#pragma once

#include "FileStream.h"

#include <cmrc/cmrc.hpp>

class EmbedFileStream : public FileStream {
public:
    EmbedFileStream(const std::string& embedFilePath);
    virtual void setPos(long pos) override;
    virtual bool isOpen() override;

private:
    virtual void read_impl(char* s, std::size_t n) override;
    virtual void write_impl(char* s, std::size_t n) override;

    long buffPos = 0;
    cmrc::file fileData;
};
