/**
 * @file undo_history.cpp
 * @brief Implementation of undo/redo history tracking.
 */

#include "undo_history.hpp"

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void UndoHistory::push(const EditAction& action) {
    m_undoStack.push(action);
    // A new edit invalidates the redo chain.
    while (!m_redoStack.empty()) {
        m_redoStack.pop();
    }
}

std::optional<EditAction> UndoHistory::undo() {
    if (m_undoStack.empty()) {
        return std::nullopt;
    }
    EditAction action = m_undoStack.top();
    m_undoStack.pop();
    m_redoStack.push(action);
    return action;
}

std::optional<EditAction> UndoHistory::redo() {
    if (m_redoStack.empty()) {
        return std::nullopt;
    }
    EditAction action = m_redoStack.top();
    m_redoStack.pop();
    m_undoStack.push(action);
    return action;
}

bool UndoHistory::canUndo() const {
    return !m_undoStack.empty();
}

bool UndoHistory::canRedo() const {
    return !m_redoStack.empty();
}

void UndoHistory::clear() {
    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();
}
