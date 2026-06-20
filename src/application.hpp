/**
 * @file application.hpp
 * @brief Top-level orchestrator — owns all modules and runs the event loop.
 *
 * The Application class ties together file I/O, the in-memory buffer,
 * the editor, display, search, and command handling.  Its run() method
 * contains the main loop: read key → translate to command → execute →
 * re-render.
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "command_handler.hpp"
#include "display_view.hpp"
#include "editor.hpp"
#include "file_buffer.hpp"
#include "search.hpp"
#include "undo_history.hpp"

#include <string>
#include <vector>

class Application {
public:
    /**
     * @brief Construct the application.
     * @param filePath  Optional initial file to open (may be empty).
     */
    explicit Application(const std::string& filePath = "");

    /**
     * @brief Run the main event loop until the user quits.
     * @return Exit code (0 = success).
     */
    int run();

private:
    // Owned modules.
    FileBuffer   m_buffer;
    UndoHistory  m_undoHistory;
    Editor       m_editor;
    DisplayView  m_display;
    AppState     m_state;

    // Search state.
    std::vector<uint8_t> m_lastSearchPattern;

    // ---- Command dispatch ----
    void dispatchCommand(Command cmd);

    // ---- Navigation ----
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void pageUp();
    void pageDown();
    void goToStart();
    void goToEnd();
    void goToOffset();

    // ---- Column switching ----
    void switchColumn();

    // ---- Editing ----
    void enterEditMode();
    void handleEditKey(int key);

    // ---- File operations ----
    bool openFile(const std::string& path);
    void saveFile();
    void openNewFile();

    // ---- Search ----
    void searchPrompt();
    void searchNext();
    void searchPrev();

    // ---- Quit ----
    void quit();

    // ---- Scroll management ----
    void ensureCursorVisible();
};

#endif // APPLICATION_HPP
