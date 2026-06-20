/**
 * @file file_buffer.hpp
 * @brief In-memory byte buffer for file contents.
 *
 * FileBuffer stores the loaded file data as a contiguous vector of bytes,
 * provides random access with bounds checking, and tracks whether
 * modifications have been made since the last save.
 */

#ifndef FILE_BUFFER_HPP
#define FILE_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

class FileBuffer {
public:
    FileBuffer() = default;

    /**
     * @brief Initialise the buffer with raw byte data.
     * @param rawData  Vector of bytes (typically read from a file).
     *
     * Replaces any previously held data and resets the dirty flag.
     */
    void loadFromData(std::vector<uint8_t> rawData);

    /**
     * @brief Read a single byte at the given offset.
     * @param offset  Zero-based byte position.
     * @return The byte value, or std::nullopt if offset is out of range.
     */
    std::optional<uint8_t> getByte(size_t offset) const;

    /**
     * @brief Write a single byte at the given offset.
     * @param offset  Zero-based byte position (must be < size()).
     * @param value   The new byte value.
     * @return true if the write succeeded, false if offset was out of range.
     *
     * Sets the dirty flag on success.
     */
    bool setByte(size_t offset, uint8_t value);

    /** @brief Number of bytes currently held in the buffer. */
    size_t size() const;

    /** @brief True if the buffer is empty (size == 0). */
    bool empty() const;

    /** @brief True if any byte has been modified since the last load/clearDirty. */
    bool isDirty() const;

    /** @brief Reset the dirty flag (e.g. after saving). */
    void clearDirty();

    /** @brief Const reference to the underlying byte vector (for serialisation). */
    const std::vector<uint8_t>& data() const;

private:
    std::vector<uint8_t> m_data;  ///< Raw byte storage.
    bool m_dirty = false;         ///< Modification flag.
};

#endif // FILE_BUFFER_HPP
