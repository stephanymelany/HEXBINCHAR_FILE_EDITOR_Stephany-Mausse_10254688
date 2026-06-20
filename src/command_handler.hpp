/**
 * @file command_handler.hpp
 * @brief Maps keyboard input to application commands.
 *
 * Translates ncurses key codes into a high-level Command enum that the
 * Application module can dispatch without knowing about raw keycodes.
 */

#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

/**
 * @brief High-level commands that the application can execute.
 */
enum class Command {
    NONE,            ///< No action (unknown or ignored key).

    // Navigation
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    GO_TO_START,     ///< Jump to offset 0.
    GO_TO_END,       ///< Jump to last byte.
    GO_TO_OFFSET,    ///< Prompt user for an offset.

    // Column switching
    SWITCH_COLUMN,   ///< TAB — cycle HEX → BIN → CHAR → HEX.

    // Editing
    ENTER_EDIT,      ///< Start editing the current byte.
    CANCEL_EDIT,     ///< ESC — cancel current edit.

    // Undo / Redo
    UNDO,
    REDO,

    // Search
    SEARCH,          ///< Open search prompt.
    SEARCH_NEXT,     ///< Find next occurrence.
    SEARCH_PREV,     ///< Find previous occurrence.

    // File operations
    SAVE,
    OPEN_FILE,       ///< Open a different file.
    QUIT
};

class CommandHandler {
public:
    /**
     * @brief Translate a raw ncurses key code into a Command.
     * @param key  The key code returned by wgetch().
     * @return The corresponding Command.
     */
    static Command processKey(int key);
};

#endif // COMMAND_HANDLER_HPP
