/**
 * @file search.hpp
 * @brief Byte-pattern search within the file buffer.
 *
 * Supports searching for patterns specified as hex strings
 * (e.g. "48 65 6C 6C 6F") or plain ASCII text.
 */

#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "file_buffer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class Search {
public:
    /**
     * @brief Parse a hex string into a byte pattern.
     * @param hexStr  Space-separated hex values, e.g. "4A 0D 0A".
     * @return The parsed byte sequence, or an empty vector on parse error.
     */
    static std::vector<uint8_t> parseHexPattern(const std::string& hexStr);

    /**
     * @brief Convert a plain ASCII string directly into a byte pattern.
     * @param text  The ASCII text to search for.
     * @return The byte sequence corresponding to the text.
     */
    static std::vector<uint8_t> parseAsciiPattern(const std::string& text);

    /**
     * @brief Search forward from a given offset for a byte pattern.
     * @param buffer   The file buffer to search in.
     * @param pattern  The byte sequence to find.
     * @param startOffset  Start position for the search (inclusive).
     * @return The offset of the first match, or std::nullopt if not found.
     */
    static std::optional<size_t> findNext(const FileBuffer& buffer,
                                          const std::vector<uint8_t>& pattern,
                                          size_t startOffset);

    /**
     * @brief Search backward from a given offset for a byte pattern.
     * @param buffer   The file buffer to search in.
     * @param pattern  The byte sequence to find.
     * @param startOffset  Start position for the search (inclusive).
     * @return The offset of the last match at or before startOffset,
     *         or std::nullopt if not found.
     */
    static std::optional<size_t> findPrev(const FileBuffer& buffer,
                                          const std::vector<uint8_t>& pattern,
                                          size_t startOffset);
};

#endif // SEARCH_HPP
