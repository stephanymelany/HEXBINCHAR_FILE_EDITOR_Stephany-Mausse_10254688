/**
 * @file file_buffer.cpp
 * @brief Implementation of the in-memory byte buffer.
 */

#include "file_buffer.hpp"

#include <utility>

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void FileBuffer::loadFromData(std::vector<uint8_t> rawData) {
    m_data  = std::move(rawData);
    m_dirty = false;
}

std::optional<uint8_t> FileBuffer::getByte(size_t offset) const {
    if (offset >= m_data.size()) {
        return std::nullopt;
    }
    return m_data[offset];
}

bool FileBuffer::setByte(size_t offset, uint8_t value) {
    if (offset >= m_data.size()) {
        return false;
    }
    m_data[offset] = value;
    m_dirty = true;
    return true;
}

size_t FileBuffer::size() const {
    return m_data.size();
}

bool FileBuffer::empty() const {
    return m_data.empty();
}

bool FileBuffer::isDirty() const {
    return m_dirty;
}

void FileBuffer::clearDirty() {
    m_dirty = false;
}

const std::vector<uint8_t>& FileBuffer::data() const {
    return m_data;
}
