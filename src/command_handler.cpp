/**
 * @file command_handler.cpp
 * @brief Implementation of key-to-command mapping.
 */

#include "command_handler.hpp"

#include <ncurses.h>

// ---------------------------------------------------------------------------
// Key bindings
// ---------------------------------------------------------------------------

Command CommandHandler::processKey(int key) {
    switch (key) {
        // Navigation — arrow keys
        case KEY_UP:     return Command::MOVE_UP;
        case KEY_DOWN:   return Command::MOVE_DOWN;
        case KEY_LEFT:   return Command::MOVE_LEFT;
        case KEY_RIGHT:  return Command::MOVE_RIGHT;

        // Navigation — page keys
        case KEY_PPAGE:  return Command::PAGE_UP;
        case KEY_NPAGE:  return Command::PAGE_DOWN;
        case KEY_HOME:   return Command::GO_TO_START;
        case KEY_END:    return Command::GO_TO_END;

        // 'g' or 'G' — go to offset
        case 'g':
        case 'G':        return Command::GO_TO_OFFSET;

        // TAB — switch column
        case '\t':       return Command::SWITCH_COLUMN;

        // 'e' or ENTER — enter edit mode
        case 'e':
        case 'E':
        case '\n':
        case KEY_ENTER:  return Command::ENTER_EDIT;

        // ESC — cancel / exit edit
        case 27:         return Command::CANCEL_EDIT;

        // Undo / Redo
        case 'u':
        case 'U':        return Command::UNDO;
        case 'r':
        case 'R':        return Command::REDO;

        // Search
        case '/':        return Command::SEARCH;
        case 'n':        return Command::SEARCH_NEXT;
        case 'N':        return Command::SEARCH_PREV;

        // File operations
        case 's':
        case 'S':        return Command::SAVE;
        case 'o':
        case 'O':        return Command::OPEN_FILE;

        // Quit
        case 'q':
        case 'Q':        return Command::QUIT;

        default:         return Command::NONE;
    }
}
