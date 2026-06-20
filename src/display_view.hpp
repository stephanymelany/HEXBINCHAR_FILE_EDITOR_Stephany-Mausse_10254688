/**
 * @file display_view.hpp
 * @brief NCurses-based terminal UI for the hex editor.
 *
 * Renders the tabular view with four columns: Offset, Hex, Binary, Char.
 * Handles colour initialisation, screen layout calculation, cursor
 * highlighting, status bar, and input prompts.
 */

#ifndef DISPLAY_VIEW_HPP
#define DISPLAY_VIEW_HPP

#include "file_buffer.hpp"
#include "editor.hpp"

#include <cstddef>
#include <string>

/**
 * @brief Application-wide state shared between modules.
 */
struct AppState {
    size_t     cursorOffset  = 0;      ///< Currently selected byte.
    size_t     scrollOffset  = 0;      ///< First byte visible on screen.
    int        bytesPerRow   = 8;      ///< Bytes displayed per row.
    EditColumn activeColumn  = EditColumn::HEX;
    bool       editMode      = false;
    bool       running       = true;
    std::string filePath;
    std::string statusMessage;         ///< Transient message for the status bar.
};

class DisplayView {
public:
    /** @brief Initialise ncurses, colours, and key settings. */
    void init();

    /** @brief Shut down ncurses and restore the terminal. */
    void shutdown();

    /**
     * @brief Render the full screen: header, data table, and status bar.
     * @param buffer  The file data to display.
     * @param state   Current application state (cursor, scroll, etc.).
     * @param editor  The editor, used to show partial input feedback.
     */
    void render(const FileBuffer& buffer, const AppState& state,
                const Editor& editor);

    /**
     * @brief Read a single key press (blocking).
     * @return The ncurses key code.
     */
    int getKey();

    /**
     * @brief Prompt the user for a text string on the status bar.
     * @param prompt  The prompt text to display.
     * @return The entered string (empty if cancelled with ESC).
     */
    std::string promptInput(const std::string& prompt);

    /**
     * @brief Show a confirmation dialog (Y/N) on the status bar.
     * @param message  The question to display.
     * @return true if the user pressed Y.
     */
    bool confirmDialog(const std::string& message);

    /** @brief Number of data rows visible on screen. */
    int visibleRows() const;

    /** @brief Current terminal width. */
    int termWidth() const;

    /** @brief Current terminal height. */
    int termHeight() const;

private:
    int m_width  = 0;
    int m_height = 0;

    // Colour pair IDs.
    static constexpr int COLOR_HEADER    = 1;
    static constexpr int COLOR_CURSOR    = 2;
    static constexpr int COLOR_MODIFIED  = 3;
    static constexpr int COLOR_NONPRINT  = 4;
    static constexpr int COLOR_STATUS    = 5;
    static constexpr int COLOR_BORDER    = 6;
    static constexpr int COLOR_EDITING   = 7;

    /** @brief Draw the title/header bar. */
    void drawHeader(const AppState& state);

    /** @brief Draw column headings (Offset | Hex | Binary | Char). */
    void drawColumnHeaders(int bytesPerRow);

    /** @brief Draw one row of data (offset + hex + binary + char). */
    void drawDataRow(int screenRow, size_t rowOffset, int bytesPerRow,
                     const FileBuffer& buffer, const AppState& state,
                     const Editor& editor);

    /** @brief Draw the status/help bar at the bottom. */
    void drawStatusBar(const AppState& state, const FileBuffer& buffer);

    /** @brief Format a byte as a two-character hex string. */
    static std::string toHex(uint8_t byte);

    /** @brief Format a byte as an eight-character binary string. */
    static std::string toBin(uint8_t byte);

    /** @brief Format a byte as a printable character (or '.'). */
    static char toChar(uint8_t byte);
};

#endif // DISPLAY_VIEW_HPP
