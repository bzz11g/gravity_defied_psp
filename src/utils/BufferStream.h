#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include "FileStream.h"

class BufferStream : public FileStream {
public:
    BufferStream(std::ios::openmode mode = std::ios::in | std::ios::out)
        : m_mode(mode), m_pos(0)
    {
    }

    BufferStream(const std::vector<int8_t>& buf, std::ios::openmode mode = std::ios::in | std::ios::out)
        : m_mode(mode), m_pos(0), m_buffer(buf)
    {
    }

    std::vector<int8_t>& getBuffer() { return m_buffer; }

    bool isOpen() override {
        return true;
    }

    void setPos(std::streampos pos) override {
        m_pos = static_cast<size_t>(pos);
    }

private:
    std::ios::openmode m_mode;
    size_t m_pos;
    std::vector<int8_t> m_buffer;

    void read_impl(char* s, std::streamsize n) override
    {
        if (m_pos + n > m_buffer.size()) {
            return;
        }
        std::memcpy(s, m_buffer.data() + m_pos, n);
        m_pos += n;
    }

    void write_impl(char* s, std::streamsize n) override
    {
        if (m_pos + n > m_buffer.size()) {
            m_buffer.resize(m_pos + n);
        }
        std::memcpy(m_buffer.data() + m_pos, s, n);
        m_pos += n;
    }
};
