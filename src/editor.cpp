/**
 * @file editor.cpp
 * @brief Implementation of the edit logic — input validation, nibble
 *        accumulation, and byte commitment with undo tracking.
 */

#include "editor.hpp"

#include <cctype>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Editor::Editor(FileBuffer& buffer, UndoHistory& history)
    : m_buffer(buffer), m_history(history) {}

// ---------------------------------------------------------------------------
// Mode control
// ---------------------------------------------------------------------------

void Editor::enterEditMode(EditColumn column, size_t offset) {
    m_editing = true;
    m_column  = column;
    m_offset  = offset;
    m_partial.clear();
}

void Editor::exitEditMode() {
    m_editing = false;
    m_partial.clear();
}

// ---------------------------------------------------------------------------
// Input handling
// ---------------------------------------------------------------------------

bool Editor::handleInput(int ch) {
    if (!m_editing) return false;

    // Validate character for the active column.
    if (!isValidChar(ch)) return false;

    switch (m_column) {
        case EditColumn::HEX: {
            // Accumulate two hex nibbles.
            m_partial += static_cast<char>(std::toupper(ch));
            if (m_partial.size() == 2) {
                uint8_t value = static_cast<uint8_t>(
                    std::strtoul(m_partial.c_str(), nullptr, 16));
                commitByte(value);
                return true;
            }
            return false;
        }

        case EditColumn::BIN: {
            // Accumulate eight binary digits.
            m_partial += static_cast<char>(ch);
            if (m_partial.size() == 8) {
                uint8_t value = static_cast<uint8_t>(
                    std::strtoul(m_partial.c_str(), nullptr, 2));
                commitByte(value);
                return true;
            }
            return false;
        }

        case EditColumn::CHAR: {
            // Single character maps directly to a byte.
            uint8_t value = static_cast<uint8_t>(ch);
            commitByte(value);
            return true;
        }
    }

    return false;  // Unreachable, but satisfies the compiler.
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool Editor::isEditing() const               { return m_editing; }
EditColumn Editor::activeColumn() const       { return m_column; }
size_t Editor::editOffset() const             { return m_offset; }
const std::string& Editor::partialInput() const { return m_partial; }

// ---------------------------------------------------------------------------
// Undo / Redo
// ---------------------------------------------------------------------------

int Editor::performUndo() {
    auto action = m_history.undo();
    if (!action) return -1;
    m_buffer.setByte(action->offset, action->oldValue);
    return static_cast<int>(action->offset);
}

int Editor::performRedo() {
    auto action = m_history.redo();
    if (!action) return -1;
    m_buffer.setByte(action->offset, action->newValue);
    return static_cast<int>(action->offset);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void Editor::commitByte(uint8_t value) {
    auto oldVal = m_buffer.getByte(m_offset);
    if (oldVal.has_value()) {
        EditAction action{m_offset, *oldVal, value};
        m_buffer.setByte(m_offset, value);
        m_history.push(action);
    }
    m_partial.clear();
    m_editing = false;
}

bool Editor::isValidChar(int ch) const {
    switch (m_column) {
        case EditColumn::HEX:
            return std::isxdigit(ch) != 0;
        case EditColumn::BIN:
            return ch == '0' || ch == '1';
        case EditColumn::CHAR:
            // Accept any printable ASCII character (32–126).
            return ch >= 32 && ch <= 126;
    }
    return false;
}
