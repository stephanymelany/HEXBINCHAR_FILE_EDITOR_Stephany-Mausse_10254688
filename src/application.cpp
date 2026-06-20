/**
 * @file application.cpp
 * @brief Implementation of the top-level Application orchestrator.
 *
 * Contains the main event loop and all command handlers.
 */

#include "application.hpp"
#include "file_manager.hpp"

#include <sstream>

// ============================================================================
// Construction
// ============================================================================

Application::Application(const std::string& filePath)
    : m_editor(m_buffer, m_undoHistory)
{
    m_state.filePath = filePath;
}

// ============================================================================
// Main event loop
// ============================================================================

int Application::run() {
    m_display.init();

    // Open the initial file, or prompt if none was given.
    if (!m_state.filePath.empty()) {
        if (!openFile(m_state.filePath)) {
            // File couldn't be opened — prompt the user.
            m_state.filePath.clear();
        }
    }

    if (m_state.filePath.empty()) {
        // No file yet — prompt interactively.
        std::string path = m_display.promptInput("Enter file path: ");
        if (path.empty() || !openFile(path)) {
            m_display.shutdown();
            return 1;
        }
    }

    m_state.running = true;

    while (m_state.running) {
        m_display.render(m_buffer, m_state, m_editor);

        int key = m_display.getKey();

        if (m_editor.isEditing()) {
            // In edit mode, most keys go to the editor.
            // ESC cancels, everything else is edit input.
            if (key == 27) {  // ESC
                m_editor.exitEditMode();
                m_state.editMode = false;
                m_state.statusMessage = "Edit cancelled.";
            } else {
                handleEditKey(key);
            }
        } else {
            // Normal mode — translate key to command.
            Command cmd = CommandHandler::processKey(key);
            dispatchCommand(cmd);
        }
    }

    m_display.shutdown();
    return 0;
}

// ============================================================================
// Command dispatch
// ============================================================================

void Application::dispatchCommand(Command cmd) {
    m_state.statusMessage.clear();

    switch (cmd) {
        case Command::MOVE_UP:       moveUp();        break;
        case Command::MOVE_DOWN:     moveDown();      break;
        case Command::MOVE_LEFT:     moveLeft();      break;
        case Command::MOVE_RIGHT:    moveRight();     break;
        case Command::PAGE_UP:       pageUp();        break;
        case Command::PAGE_DOWN:     pageDown();      break;
        case Command::GO_TO_START:   goToStart();     break;
        case Command::GO_TO_END:     goToEnd();       break;
        case Command::GO_TO_OFFSET:  goToOffset();    break;
        case Command::SWITCH_COLUMN: switchColumn();  break;
        case Command::ENTER_EDIT:    enterEditMode(); break;
        case Command::CANCEL_EDIT:   /* nothing in normal mode */ break;
        case Command::UNDO: {
            int off = m_editor.performUndo();
            if (off >= 0) {
                m_state.cursorOffset = static_cast<size_t>(off);
                ensureCursorVisible();
                m_state.statusMessage = "Undo.";
            } else {
                m_state.statusMessage = "Nothing to undo.";
            }
            break;
        }
        case Command::REDO: {
            int off = m_editor.performRedo();
            if (off >= 0) {
                m_state.cursorOffset = static_cast<size_t>(off);
                ensureCursorVisible();
                m_state.statusMessage = "Redo.";
            } else {
                m_state.statusMessage = "Nothing to redo.";
            }
            break;
        }
        case Command::SEARCH:       searchPrompt();  break;
        case Command::SEARCH_NEXT:  searchNext();    break;
        case Command::SEARCH_PREV:  searchPrev();    break;
        case Command::SAVE:         saveFile();      break;
        case Command::OPEN_FILE:    openNewFile();   break;
        case Command::QUIT:         quit();          break;
        case Command::NONE:         /* ignored */    break;
    }
}

// ============================================================================
// Navigation
// ============================================================================

void Application::moveUp() {
    if (m_buffer.empty()) return;
    if (m_state.cursorOffset >= static_cast<size_t>(m_state.bytesPerRow)) {
        m_state.cursorOffset -= m_state.bytesPerRow;
    } else {
        m_state.cursorOffset = 0;
    }
    ensureCursorVisible();
}

void Application::moveDown() {
    if (m_buffer.empty()) return;
    size_t next = m_state.cursorOffset + m_state.bytesPerRow;
    if (next < m_buffer.size()) {
        m_state.cursorOffset = next;
    } else {
        m_state.cursorOffset = m_buffer.size() - 1;
    }
    ensureCursorVisible();
}

void Application::moveLeft() {
    if (m_buffer.empty()) return;
    if (m_state.cursorOffset > 0) {
        --m_state.cursorOffset;
    }
    ensureCursorVisible();
}

void Application::moveRight() {
    if (m_buffer.empty()) return;
    if (m_state.cursorOffset + 1 < m_buffer.size()) {
        ++m_state.cursorOffset;
    }
    ensureCursorVisible();
}

void Application::pageUp() {
    if (m_buffer.empty()) return;
    size_t jump = static_cast<size_t>(m_display.visibleRows()) * m_state.bytesPerRow;
    if (m_state.cursorOffset >= jump) {
        m_state.cursorOffset -= jump;
    } else {
        m_state.cursorOffset = 0;
    }
    ensureCursorVisible();
}

void Application::pageDown() {
    if (m_buffer.empty()) return;
    size_t jump = static_cast<size_t>(m_display.visibleRows()) * m_state.bytesPerRow;
    m_state.cursorOffset += jump;
    if (m_state.cursorOffset >= m_buffer.size()) {
        m_state.cursorOffset = m_buffer.size() - 1;
    }
    ensureCursorVisible();
}

void Application::goToStart() {
    m_state.cursorOffset = 0;
    ensureCursorVisible();
}

void Application::goToEnd() {
    if (!m_buffer.empty()) {
        m_state.cursorOffset = m_buffer.size() - 1;
    }
    ensureCursorVisible();
}

void Application::goToOffset() {
    std::string input = m_display.promptInput("Go to offset (hex): 0x");
    if (input.empty()) return;

    size_t offset = 0;
    try {
        offset = std::stoull(input, nullptr, 16);
    } catch (...) {
        m_state.statusMessage = "Invalid hex offset.";
        return;
    }

    if (offset >= m_buffer.size()) {
        m_state.statusMessage = "Offset out of range.";
        return;
    }

    m_state.cursorOffset = offset;
    ensureCursorVisible();
}

// ============================================================================
// Column switching
// ============================================================================

void Application::switchColumn() {
    switch (m_state.activeColumn) {
        case EditColumn::HEX:  m_state.activeColumn = EditColumn::BIN;  break;
        case EditColumn::BIN:  m_state.activeColumn = EditColumn::CHAR; break;
        case EditColumn::CHAR: m_state.activeColumn = EditColumn::HEX;  break;
    }
}

// ============================================================================
// Editing
// ============================================================================

void Application::enterEditMode() {
    if (m_buffer.empty()) {
        m_state.statusMessage = "No data to edit.";
        return;
    }
    m_editor.enterEditMode(m_state.activeColumn, m_state.cursorOffset);
    m_state.editMode = true;
    m_state.statusMessage = "Editing — type value, ESC to cancel.";
}

void Application::handleEditKey(int key) {
    bool completed = m_editor.handleInput(key);
    if (completed) {
        m_state.editMode = false;
        m_state.statusMessage = "Byte modified.";

        // Auto-advance to the next byte.
        if (m_state.cursorOffset + 1 < m_buffer.size()) {
            ++m_state.cursorOffset;
            ensureCursorVisible();
        }
    }
    // If not completed, the editor is still accumulating partial input.
}

// ============================================================================
// File operations
// ============================================================================

bool Application::openFile(const std::string& path) {
    auto [data, error] = FileManager::readFile(path);
    if (!error.empty()) {
        m_state.statusMessage = "Error: " + error;
        return false;
    }

    m_buffer.loadFromData(std::move(data));
    m_state.filePath      = path;
    m_state.cursorOffset  = 0;
    m_state.scrollOffset  = 0;
    m_undoHistory.clear();
    m_state.statusMessage = "Opened: " + path + " (" +
                            std::to_string(m_buffer.size()) + " bytes)";
    return true;
}

void Application::saveFile() {
    if (m_state.filePath.empty()) {
        m_state.statusMessage = "No file path — use 'O' to open a file first.";
        return;
    }

    std::string error = FileManager::writeFile(m_state.filePath, m_buffer.data());
    if (!error.empty()) {
        m_state.statusMessage = "Save error: " + error;
    } else {
        m_buffer.clearDirty();
        m_state.statusMessage = "Saved to " + m_state.filePath;
    }
}

void Application::openNewFile() {
    // Warn about unsaved changes.
    if (m_buffer.isDirty()) {
        bool proceed = m_display.confirmDialog(
            "Unsaved changes will be lost. Continue?");
        if (!proceed) {
            m_state.statusMessage = "Open cancelled.";
            return;
        }
    }

    std::string path = m_display.promptInput("Open file: ");
    if (path.empty()) {
        m_state.statusMessage = "Open cancelled.";
        return;
    }

    openFile(path);
}

// ============================================================================
// Search
// ============================================================================

void Application::searchPrompt() {
    std::string input = m_display.promptInput(
        "Search (hex e.g. '48 65' or text e.g. 'Hello'): ");
    if (input.empty()) return;

    // Try to parse as hex first; if that fails, treat as ASCII.
    auto pattern = Search::parseHexPattern(input);
    if (pattern.empty()) {
        pattern = Search::parseAsciiPattern(input);
    }

    if (pattern.empty()) {
        m_state.statusMessage = "Invalid search pattern.";
        return;
    }

    m_lastSearchPattern = pattern;

    // Search forward from the current cursor position.
    auto result = Search::findNext(m_buffer, pattern,
                                   m_state.cursorOffset);
    if (result) {
        m_state.cursorOffset = *result;
        ensureCursorVisible();
        m_state.statusMessage = "Found at offset 0x" +
            ([](size_t v) {
                std::ostringstream ss;
                ss << std::uppercase << std::hex << v;
                return ss.str();
            })(*result);
    } else {
        m_state.statusMessage = "Pattern not found.";
    }
}

void Application::searchNext() {
    if (m_lastSearchPattern.empty()) {
        m_state.statusMessage = "No previous search. Press '/' to search.";
        return;
    }

    size_t start = m_state.cursorOffset + 1;
    auto result = Search::findNext(m_buffer, m_lastSearchPattern, start);
    if (result) {
        m_state.cursorOffset = *result;
        ensureCursorVisible();
        std::ostringstream ss;
        ss << "Found at offset 0x" << std::uppercase << std::hex << *result;
        m_state.statusMessage = ss.str();
    } else {
        m_state.statusMessage = "No more matches.";
    }
}

void Application::searchPrev() {
    if (m_lastSearchPattern.empty()) {
        m_state.statusMessage = "No previous search. Press '/' to search.";
        return;
    }

    size_t start = (m_state.cursorOffset > 0) ? m_state.cursorOffset - 1 : 0;
    auto result = Search::findPrev(m_buffer, m_lastSearchPattern, start);
    if (result) {
        m_state.cursorOffset = *result;
        ensureCursorVisible();
        std::ostringstream ss;
        ss << "Found at offset 0x" << std::uppercase << std::hex << *result;
        m_state.statusMessage = ss.str();
    } else {
        m_state.statusMessage = "No earlier matches.";
    }
}

// ============================================================================
// Quit
// ============================================================================

void Application::quit() {
    if (m_buffer.isDirty()) {
        bool discard = m_display.confirmDialog(
            "Unsaved changes! Quit without saving?");
        if (!discard) {
            m_state.statusMessage = "Quit cancelled.";
            return;
        }
    }
    m_state.running = false;
}

// ============================================================================
// Scroll management
// ============================================================================

void Application::ensureCursorVisible() {
    int bpr  = m_state.bytesPerRow;
    int rows = m_display.visibleRows();

    // Calculate the row the cursor is on (relative to file start).
    size_t cursorRow  = m_state.cursorOffset / bpr;
    size_t scrollRow  = m_state.scrollOffset / bpr;

    // If cursor is above the visible window, scroll up.
    if (cursorRow < scrollRow) {
        m_state.scrollOffset = cursorRow * bpr;
    }

    // If cursor is below the visible window, scroll down.
    if (cursorRow >= scrollRow + static_cast<size_t>(rows)) {
        m_state.scrollOffset = (cursorRow - rows + 1) * bpr;
    }
}
