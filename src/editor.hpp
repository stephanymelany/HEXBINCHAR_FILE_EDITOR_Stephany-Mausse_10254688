/**
 * @file editor.hpp
 * @brief Edit logic for the hex editor — validates and applies user input.
 *
 * The Editor manages the editing state: which column is active
 * (HEX, BIN, or CHAR), accumulates partial input (e.g. two hex nibbles),
 * validates characters per format, and commits completed byte values
 * to the FileBuffer while recording undo history.
 */

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "file_buffer.hpp"
#include "undo_history.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @brief Identifies which display column the user is editing in.
 */
enum class EditColumn {
    HEX,   ///< Hexadecimal — expects two hex digits per byte.
    BIN,   ///< Binary      — expects eight binary digits per byte.
    CHAR   ///< Character    — expects one printable character per byte.
};

class Editor {
public:
    /**
     * @param buffer   Reference to the shared file buffer.
     * @param history  Reference to the shared undo history.
     */
    Editor(FileBuffer& buffer, UndoHistory& history);

    /** @brief Enter edit mode for the given column at the current offset. */
    void enterEditMode(EditColumn column, size_t offset);

    /** @brief Exit edit mode, discarding any partial input. */
    void exitEditMode();

    /**
     * @brief Process a single character of user input while in edit mode.
     * @param ch  The character typed by the user.
     * @return true  if the byte was fully committed (edit cycle complete).
     * @return false if more input is needed (partial nibble/bit) or invalid.
     *
     * In HEX mode, two valid hex digits complete one byte.
     * In BIN mode, eight valid binary digits complete one byte.
     * In CHAR mode, a single printable character completes one byte.
     */
    bool handleInput(int ch);

    /** @brief True if the editor is currently in edit mode. */
    bool isEditing() const;

    /** @brief The column currently being edited. */
    EditColumn activeColumn() const;

    /** @brief The byte offset currently being edited. */
    size_t editOffset() const;

    /** @brief Partial input accumulated so far (for display feedback). */
    const std::string& partialInput() const;

    /** @brief Undo the last edit, returning the affected offset or -1. */
    int performUndo();

    /** @brief Redo the last undone edit, returning the affected offset or -1. */
    int performRedo();

private:
    FileBuffer&  m_buffer;
    UndoHistory& m_history;

    bool       m_editing    = false;
    EditColumn m_column     = EditColumn::HEX;
    size_t     m_offset     = 0;
    std::string m_partial;  ///< Accumulated digits for the current byte.

    /** @brief Commit the accumulated partial input as a complete byte. */
    void commitByte(uint8_t value);

    /** @brief Validate a character for the current column format. */
    bool isValidChar(int ch) const;
};

#endif // EDITOR_HPP
