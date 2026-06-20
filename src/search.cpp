/**
 * @file search.cpp
 * @brief Implementation of byte-pattern search.
 */

#include "search.hpp"

#include <cctype>
#include <cstdlib>
#include <sstream>

// ---------------------------------------------------------------------------
// Pattern parsing
// ---------------------------------------------------------------------------

std::vector<uint8_t> Search::parseHexPattern(const std::string& hexStr) {
    std::vector<uint8_t> pattern;
    std::istringstream iss(hexStr);
    std::string token;

    while (iss >> token) {
        // Validate: each token must be 1-2 hex digits.
        if (token.size() > 2) return {};
        for (char c : token) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) return {};
        }
        auto val = std::strtoul(token.c_str(), nullptr, 16);
        pattern.push_back(static_cast<uint8_t>(val));
    }

    return pattern;
}

std::vector<uint8_t> Search::parseAsciiPattern(const std::string& text) {
    return std::vector<uint8_t>(text.begin(), text.end());
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------

std::optional<size_t> Search::findNext(const FileBuffer& buffer,
                                       const std::vector<uint8_t>& pattern,
                                       size_t startOffset) {
    if (pattern.empty() || buffer.empty()) return std::nullopt;

    size_t bufSize     = buffer.size();
    size_t patternSize = pattern.size();

    if (patternSize > bufSize) return std::nullopt;

    for (size_t i = startOffset; i <= bufSize - patternSize; ++i) {
        bool match = true;
        for (size_t j = 0; j < patternSize; ++j) {
            auto byte = buffer.getByte(i + j);
            if (!byte || *byte != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }

    return std::nullopt;
}

std::optional<size_t> Search::findPrev(const FileBuffer& buffer,
                                       const std::vector<uint8_t>& pattern,
                                       size_t startOffset) {
    if (pattern.empty() || buffer.empty()) return std::nullopt;

    size_t bufSize     = buffer.size();
    size_t patternSize = pattern.size();

    if (patternSize > bufSize) return std::nullopt;

    // Cap startOffset so pattern can fit.
    size_t maxStart = bufSize - patternSize;
    size_t i = (startOffset < maxStart) ? startOffset : maxStart;

    // Search backwards (using size_t wraparound detection).
    while (true) {
        bool match = true;
        for (size_t j = 0; j < patternSize; ++j) {
            auto byte = buffer.getByte(i + j);
            if (!byte || *byte != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;

        if (i == 0) break;
        --i;
    }

    return std::nullopt;
}
