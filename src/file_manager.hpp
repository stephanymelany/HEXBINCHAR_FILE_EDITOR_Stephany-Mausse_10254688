/**
 * @file file_manager.hpp
 * @brief Handles reading from and writing to files on disk.
 *
 * Provides static utility methods for binary file I/O with proper
 * error reporting via std::string messages.
 */

#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <cstdint>
#include <string>
#include <vector>

class FileManager {
public:
    /**
     * @brief Read the entire contents of a binary file into memory.
     * @param path  Filesystem path to the file.
     * @return A pair of {byte vector, error message}.
     *         On success the error string is empty; on failure the vector is empty.
     */
    static std::pair<std::vector<uint8_t>, std::string> readFile(const std::string& path);

    /**
     * @brief Write a byte vector to a file, replacing its contents.
     * @param path  Filesystem path to write to.
     * @param data  The bytes to write.
     * @return An empty string on success, or an error description on failure.
     */
    static std::string writeFile(const std::string& path,
                                 const std::vector<uint8_t>& data);

    /**
     * @brief Check whether a file exists and is readable.
     * @param path  Filesystem path to check.
     * @return true if the file can be opened for reading.
     */
    static bool fileExists(const std::string& path);
};

#endif // FILE_MANAGER_HPP
