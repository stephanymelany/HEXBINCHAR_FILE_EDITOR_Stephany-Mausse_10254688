/**
 * @file display_view.cpp
 * @brief NCurses rendering implementation for the hex editor.
 *
 * Layout (8 bytes per row):
 *   Offset   | Hexadecimal              | Binary                                        | Char
 *   00000000 | 48 65 6C 6C 6F 20 57 6F | 01001000 01100101 01101100 01101100 01101111.. | Hello Wo
 */

#include "display_view.hpp"

#include <ncurses.h>

#include <algorithm>
#include <cstring>
#include <sstream>

// ============================================================================
// Initialisation / Shutdown
// ============================================================================

void DisplayView::init() {
    initscr();              // Start ncurses mode.
    cbreak();               // Disable line buffering.
    noecho();               // Don't echo typed characters.
    keypad(stdscr, TRUE);   // Enable function/arrow keys.
    curs_set(0);            // Hide the hardware cursor.

    // Initialise colours if the terminal supports them.
    if (has_colors()) {
        start_color();
        use_default_colors();

        // Define colour pairs.
        init_pair(COLOR_HEADER,   COLOR_BLACK,   COLOR_CYAN);
        init_pair(COLOR_CURSOR,   COLOR_BLACK,   COLOR_YELLOW);
        init_pair(COLOR_MODIFIED, COLOR_RED,     -1);
        init_pair(COLOR_NONPRINT, COLOR_BLUE,    -1);
        init_pair(COLOR_STATUS,   COLOR_BLACK,   COLOR_GREEN);
        init_pair(COLOR_BORDER,   COLOR_CYAN,    -1);
        init_pair(COLOR_EDITING,  COLOR_BLACK,   COLOR_MAGENTA);
    }

    getmaxyx(stdscr, m_height, m_width);
}

void DisplayView::shutdown() {
    endwin();
}

// ============================================================================
// Full screen render
// ============================================================================

void DisplayView::render(const FileBuffer& buffer, const AppState& state,
                         const Editor& editor) {
    // Refresh terminal dimensions (in case of resize).
    getmaxyx(stdscr, m_height, m_width);

    erase();  // Clear the virtual screen buffer.

    drawHeader(state);
    drawColumnHeaders(state.bytesPerRow);

    // Data rows start at line 2 (after header + column headers).
    int dataStartRow = 2;
    int dataRows     = visibleRows();

    for (int row = 0; row < dataRows; ++row) {
        size_t rowOffset = state.scrollOffset +
                           static_cast<size_t>(row) * state.bytesPerRow;
        if (rowOffset >= buffer.size()) break;

        drawDataRow(dataStartRow + row, rowOffset, state.bytesPerRow,
                    buffer, state, editor);
    }

    drawStatusBar(state, buffer);

    refresh();  // Push changes to the physical screen.
}

// ============================================================================
// Input
// ============================================================================

int DisplayView::getKey() {
    return wgetch(stdscr);
}

std::string DisplayView::promptInput(const std::string& prompt) {
    int row = m_height - 1;
    move(row, 0);
    clrtoeol();
    attron(COLOR_PAIR(COLOR_STATUS));
    mvprintw(row, 0, "%s", prompt.c_str());
    attroff(COLOR_PAIR(COLOR_STATUS));

    echo();
    curs_set(1);
    nocbreak();  // Use line-buffered input for the prompt.

    char inputBuf[256] = {};
    // Position cursor right after the prompt text.
    move(row, static_cast<int>(prompt.size()));
    getnstr(inputBuf, sizeof(inputBuf) - 1);

    noecho();
    curs_set(0);
    cbreak();

    return std::string(inputBuf);
}

bool DisplayView::confirmDialog(const std::string& message) {
    int row = m_height - 1;
    move(row, 0);
    clrtoeol();
    attron(COLOR_PAIR(COLOR_STATUS) | A_BOLD);
    mvprintw(row, 0, "%s (y/n) ", message.c_str());
    attroff(COLOR_PAIR(COLOR_STATUS) | A_BOLD);
    refresh();

    int ch = wgetch(stdscr);
    return (ch == 'y' || ch == 'Y');
}

// ============================================================================
// Geometry queries
// ============================================================================

int DisplayView::visibleRows() const {
    // header(1) + column header(1) + status bar(2) = 4 reserved rows.
    return std::max(1, m_height - 4);
}

int DisplayView::termWidth() const  { return m_width; }
int DisplayView::termHeight() const { return m_height; }

// ============================================================================
// Drawing helpers — private
// ============================================================================

void DisplayView::drawHeader(const AppState& state) {
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);

    // Build header line, padded to terminal width.
    std::ostringstream hdr;
    hdr << " HEX/BIN/CHAR Editor v1.0";

    // File name (right-aligned portion).
    std::string fname = state.filePath.empty() ? "[No File]" : state.filePath;
    std::string modified = (state.editMode ? " [EDITING]" : "");

    // Center section: filename.
    int leftLen  = static_cast<int>(hdr.str().size());
    int rightLen = static_cast<int>(fname.size() + modified.size() + 2);
    int gap      = std::max(2, m_width - leftLen - rightLen);

    hdr << std::string(gap, ' ') << fname << modified;

    // Pad to full width.
    std::string headerStr = hdr.str();
    if (static_cast<int>(headerStr.size()) < m_width) {
        headerStr.append(m_width - headerStr.size(), ' ');
    }

    mvprintw(0, 0, "%s", headerStr.c_str());
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
}

void DisplayView::drawColumnHeaders(int bytesPerRow) {
    attron(COLOR_PAIR(COLOR_BORDER) | A_BOLD);

    std::ostringstream cols;
    cols << " Offset   | ";

    // Hex column header.
    cols << "Hex";
    int hexColWidth = bytesPerRow * 3;  // "XX " per byte.
    int hexPad = std::max(0, hexColWidth - 3);
    cols << std::string(hexPad, ' ') << "| ";

    // Binary column header.
    cols << "Binary";
    int binColWidth = bytesPerRow * 9;  // "XXXXXXXX " per byte.
    int binPad = std::max(0, binColWidth - 6);
    cols << std::string(binPad, ' ') << "| ";

    // Char column header.
    cols << "Char";

    std::string colStr = cols.str();
    if (static_cast<int>(colStr.size()) < m_width) {
        colStr.append(m_width - colStr.size(), ' ');
    }

    mvprintw(1, 0, "%s", colStr.c_str());
    attroff(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
}

void DisplayView::drawDataRow(int screenRow, size_t rowOffset, int bytesPerRow,
                               const FileBuffer& buffer, const AppState& state,
                               const Editor& editor) {
    // -- Offset column --
    attron(COLOR_PAIR(COLOR_BORDER));
    mvprintw(screenRow, 0, " %08lX | ",
             static_cast<unsigned long>(rowOffset));
    attroff(COLOR_PAIR(COLOR_BORDER));

    // Calculate column start positions.
    int hexStart  = 12;  // After " XXXXXXXX | "
    int binStart  = hexStart + bytesPerRow * 3 + 2;  // After hex + "| "
    int charStart = binStart + bytesPerRow * 9 + 2;  // After bin + "| "

    // -- Hex column --
    for (int i = 0; i < bytesPerRow; ++i) {
        size_t offset = rowOffset + i;
        if (offset >= buffer.size()) {
            mvprintw(screenRow, hexStart + i * 3, "   ");
            continue;
        }

        uint8_t byte = *buffer.getByte(offset);
        bool isCursor = (offset == state.cursorOffset);
        bool isEditing = editor.isEditing() && (offset == editor.editOffset());

        // Choose colour.
        if (isEditing && state.activeColumn == EditColumn::HEX) {
            attron(COLOR_PAIR(COLOR_EDITING) | A_BOLD);
        } else if (isCursor && state.activeColumn == EditColumn::HEX) {
            attron(COLOR_PAIR(COLOR_CURSOR) | A_BOLD);
        }

        // If editing this byte in HEX, show partial input.
        if (isEditing && editor.activeColumn() == EditColumn::HEX
            && !editor.partialInput().empty()) {
            std::string display = editor.partialInput() + "_";
            if (display.size() < 2) display += " ";
            mvprintw(screenRow, hexStart + i * 3, "%s ",
                     display.substr(0, 2).c_str());
        } else {
            mvprintw(screenRow, hexStart + i * 3, "%s ",
                     toHex(byte).c_str());
        }

        // Reset colour.
        if (isEditing && state.activeColumn == EditColumn::HEX) {
            attroff(COLOR_PAIR(COLOR_EDITING) | A_BOLD);
        } else if (isCursor && state.activeColumn == EditColumn::HEX) {
            attroff(COLOR_PAIR(COLOR_CURSOR) | A_BOLD);
        }
    }

    // -- Separator --
    attron(COLOR_PAIR(COLOR_BORDER));
    mvprintw(screenRow, binStart - 2, "| ");
    attroff(COLOR_PAIR(COLOR_BORDER));

    // -- Binary column --
    for (int i = 0; i < bytesPerRow; ++i) {
        size_t offset = rowOffset + i;
        if (offset >= buffer.size()) {
            mvprintw(screenRow, binStart + i * 9, "         ");
            continue;
        }

        uint8_t byte = *buffer.getByte(offset);
        bool isCursor = (offset == state.cursorOffset);
        bool isEditing = editor.isEditing() && (offset == editor.editOffset());

        if (isEditing && state.activeColumn == EditColumn::BIN) {
            attron(COLOR_PAIR(COLOR_EDITING) | A_BOLD);
        } else if (isCursor && state.activeColumn == EditColumn::BIN) {
            attron(COLOR_PAIR(COLOR_CURSOR) | A_BOLD);
        }

        // If editing this byte in BIN, show partial input.
        if (isEditing && editor.activeColumn() == EditColumn::BIN
            && !editor.partialInput().empty()) {
            std::string display = editor.partialInput();
            while (display.size() < 8) display += "_";
            mvprintw(screenRow, binStart + i * 9, "%s ",
                     display.substr(0, 8).c_str());
        } else {
            mvprintw(screenRow, binStart + i * 9, "%s ",
                     toBin(byte).c_str());
        }

        if (isEditing && state.activeColumn == EditColumn::BIN) {
            attroff(COLOR_PAIR(COLOR_EDITING) | A_BOLD);
        } else if (isCursor && state.activeColumn == EditColumn::BIN) {
            attroff(COLOR_PAIR(COLOR_CURSOR) | A_BOLD);
        }
    }

    // -- Separator --
    attron(COLOR_PAIR(COLOR_BORDER));
    mvprintw(screenRow, charStart - 2, "| ");
    attroff(COLOR_PAIR(COLOR_BORDER));

    // -- Char column --
    for (int i = 0; i < bytesPerRow; ++i) {
        size_t offset = rowOffset + i;
        if (offset >= buffer.size()) break;

        uint8_t byte = *buffer.getByte(offset);
        bool isCursor = (offset == state.cursorOffset);
        bool isEditing = editor.isEditing() && (offset == editor.editOffset());
        char ch = toChar(byte);

        if (isEditing && state.activeColumn == EditColumn::CHAR) {
            attron(COLOR_PAIR(COLOR_EDITING) | A_BOLD);
        } else if (isCursor && state.activeColumn == EditColumn::CHAR) {
            attron(COLOR_PAIR(COLOR_CURSOR) | A_BOLD);
        } else if (ch == '.') {
            attron(COLOR_PAIR(COLOR_NONPRINT));
        }

        mvaddch(screenRow, charStart + i, ch);

        if (isEditing && state.activeColumn == EditColumn::CHAR) {
            attroff(COLOR_PAIR(COLOR_EDITING) | A_BOLD);
        } else if (isCursor && state.activeColumn == EditColumn::CHAR) {
            attroff(COLOR_PAIR(COLOR_CURSOR) | A_BOLD);
        } else if (ch == '.') {
            attroff(COLOR_PAIR(COLOR_NONPRINT));
        }
    }
}

void DisplayView::drawStatusBar(const AppState& state,
                                 const FileBuffer& buffer) {
    // Row for help keys.
    int helpRow = m_height - 2;
    attron(COLOR_PAIR(COLOR_STATUS));

    std::string helpLine =
        " [Arrows] Navigate  [TAB] Column  [E/Enter] Edit  "
        "[S] Save  [Q] Quit  [U] Undo  [R] Redo  [/] Search  "
        "[G] GoTo  [O] Open";

    if (static_cast<int>(helpLine.size()) < m_width) {
        helpLine.append(m_width - helpLine.size(), ' ');
    }
    mvprintw(helpRow, 0, "%s", helpLine.substr(0, m_width).c_str());
    attroff(COLOR_PAIR(COLOR_STATUS));

    // Status row (bottom).
    int statusRow = m_height - 1;
    attron(COLOR_PAIR(COLOR_STATUS));

    std::ostringstream status;
    status << " Offset: 0x" << std::uppercase << std::hex << state.cursorOffset
           << " (" << std::dec << state.cursorOffset << ")"
           << "  Size: " << buffer.size() << " bytes";

    if (buffer.isDirty()) {
        status << "  [MODIFIED]";
    }

    // Active column indicator.
    status << "  Column: ";
    switch (state.activeColumn) {
        case EditColumn::HEX:  status << "HEX";  break;
        case EditColumn::BIN:  status << "BIN";  break;
        case EditColumn::CHAR: status << "CHAR"; break;
    }

    if (!state.statusMessage.empty()) {
        status << "  | " << state.statusMessage;
    }

    std::string statusStr = status.str();
    if (static_cast<int>(statusStr.size()) < m_width) {
        statusStr.append(m_width - statusStr.size(), ' ');
    }
    mvprintw(statusRow, 0, "%s", statusStr.substr(0, m_width).c_str());
    attroff(COLOR_PAIR(COLOR_STATUS));
}

// ============================================================================
// Format helpers — static
// ============================================================================

std::string DisplayView::toHex(uint8_t byte) {
    static const char hexDigits[] = "0123456789ABCDEF";
    std::string result(2, '0');
    result[0] = hexDigits[(byte >> 4) & 0x0F];
    result[1] = hexDigits[byte & 0x0F];
    return result;
}

std::string DisplayView::toBin(uint8_t byte) {
    std::string result(8, '0');
    for (int i = 7; i >= 0; --i) {
        result[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    return result;
}

char DisplayView::toChar(uint8_t byte) {
    // Printable ASCII range: 32 (space) to 126 (~).
    if (byte >= 32 && byte <= 126) {
        return static_cast<char>(byte);
    }
    return '.';  // Non-printable placeholder.
}
