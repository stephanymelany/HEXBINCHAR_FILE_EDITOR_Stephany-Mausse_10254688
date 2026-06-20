/**
 * @file main.cpp
 * @brief Entry point for the HEX/BIN/CHAR File Editor.
 *
 * Usage:
 *   ./hexedit [filename]     Open a file directly.
 *   ./hexedit --help         Show usage information.
 *   ./hexedit                Prompt for a file path interactively.
 */

#include "application.hpp"

#include <cstring>
#include <iostream>
#include <string>

/**
 * @brief Print usage information to stdout.
 */
static void printUsage(const char* programName) {
    std::cout
        << "HEX/BIN/CHAR File Editor v1.0\n"
        << "==============================\n\n"
        << "Usage:\n"
        << "  " << programName << " [filename]    Open a file for viewing/editing.\n"
        << "  " << programName << " --help        Show this help message.\n"
        << "  " << programName << "               Start and enter file path interactively.\n\n"
        << "Key Bindings:\n"
        << "  Arrow keys     Navigate through bytes\n"
        << "  TAB            Switch column (HEX / BIN / CHAR)\n"
        << "  E / Enter      Enter edit mode for the current byte\n"
        << "  ESC            Cancel current edit\n"
        << "  S              Save file\n"
        << "  O              Open a different file\n"
        << "  U              Undo last edit\n"
        << "  R              Redo\n"
        << "  /              Search for a byte pattern\n"
        << "  N              Find next match\n"
        << "  Shift+N        Find previous match\n"
        << "  G              Go to a specific offset\n"
        << "  PgUp / PgDn    Scroll by page\n"
        << "  Home / End     Jump to start / end of file\n"
        << "  Q              Quit\n\n"
        << "Edit Formats:\n"
        << "  HEX column:  Enter two hex digits (0-9, A-F).\n"
        << "  BIN column:  Enter eight binary digits (0 or 1).\n"
        << "  CHAR column: Enter one printable ASCII character.\n"
        << std::endl;
}

int main(int argc, char* argv[]) {
    std::string filePath;

    // Parse command-line arguments.
    if (argc > 1) {
        if (std::strcmp(argv[1], "--help") == 0 ||
            std::strcmp(argv[1], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        filePath = argv[1];
    }

    Application app(filePath);
    return app.run();
}
