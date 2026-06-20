/**
 * @file undo_history.hpp
 * @brief Tracks edit actions for undo/redo support.
 *
 * Maintains two stacks: one for undoable actions and one for redoable
 * actions.  Pushing a new action clears the redo stack, matching the
 * behaviour users expect from standard undo/redo.
 */

#ifndef UNDO_HISTORY_HPP
#define UNDO_HISTORY_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stack>

/**
 * @brief A single byte-level edit that can be undone or redone.
 */
struct EditAction {
    size_t  offset;    ///< Byte position in the file buffer.
    uint8_t oldValue;  ///< Value before the edit.
    uint8_t newValue;  ///< Value after the edit.
};

class UndoHistory {
public:
    UndoHistory() = default;

    /**
     * @brief Record a new edit action.
     *
     * Clears the redo stack — once a new edit is made, the previous
     * redo chain is invalidated.
     */
    void push(const EditAction& action);

    /**
     * @brief Pop the most recent action from the undo stack.
     * @return The action to reverse, or std::nullopt if nothing to undo.
     *
     * The popped action is moved onto the redo stack.
     */
    std::optional<EditAction> undo();

    /**
     * @brief Pop the most recent action from the redo stack.
     * @return The action to re-apply, or std::nullopt if nothing to redo.
     *
     * The popped action is moved back onto the undo stack.
     */
    std::optional<EditAction> redo();

    /** @brief True if there are actions that can be undone. */
    bool canUndo() const;

    /** @brief True if there are actions that can be redone. */
    bool canRedo() const;

    /** @brief Discard all undo and redo history. */
    void clear();

private:
    std::stack<EditAction> m_undoStack;
    std::stack<EditAction> m_redoStack;
};

#endif // UNDO_HISTORY_HPP
