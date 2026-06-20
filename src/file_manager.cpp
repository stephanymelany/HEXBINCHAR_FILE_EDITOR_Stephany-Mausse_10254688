/**
 * @file file_manager.cpp
 * @brief Implementation of file I/O operations.
 */

#include "file_manager.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------------------------
// readFile
// ---------------------------------------------------------------------------
std::pair<std::vector<uint8_t>, std::string>
FileManager::readFile(const std::string& path) {
    // Open in binary mode and seek to the end to determine file size.
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
        std::ostringstream err;
        err << "Cannot open file '" << path << "': " << std::strerror(errno);
        return {{}, err.str()};
    }

    auto fileSize = ifs.tellg();
    if (fileSize < 0) {
        return {{}, "Failed to determine file size."};
    }

    ifs.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    if (fileSize > 0) {
        ifs.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        if (!ifs) {
            std::ostringstream err;
            err << "Read error after " << ifs.gcount() << " of "
                << fileSize << " bytes.";
            return {{}, err.str()};
        }
    }

    return {buffer, {}};
}

// ---------------------------------------------------------------------------
// writeFile
// ---------------------------------------------------------------------------
std::string FileManager::writeFile(const std::string& path,
                                   const std::vector<uint8_t>& data) {
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) {
        std::ostringstream err;
        err << "Cannot open file '" << path << "' for writing: "
            << std::strerror(errno);
        return err.str();
    }

    if (!data.empty()) {
        ofs.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
        if (!ofs) {
            return "Write error — not all bytes were written.";
        }
    }

    return {};  // Success — empty error string.
}

// ---------------------------------------------------------------------------
// fileExists
// ---------------------------------------------------------------------------
bool FileManager::fileExists(const std::string& path) {
    std::ifstream ifs(path);
    return ifs.good();
}
