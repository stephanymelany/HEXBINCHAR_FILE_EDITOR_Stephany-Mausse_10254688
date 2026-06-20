# ===========================================================================
# CMakeLists.txt — HEX/BIN/CHAR File Editor
# ===========================================================================
cmake_minimum_required(VERSION 3.16)
project(HexEditor
    VERSION   1.0.0
    LANGUAGES CXX
    DESCRIPTION "A terminal-based HEX/BIN/CHAR file editor"
)

# ---------------------------------------------------------------------------
# Compiler settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warnings (treat as errors in Debug for code quality).
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# NCurses dependency
# ---------------------------------------------------------------------------
# On macOS, Homebrew's ncurses is keg-only, so we hint CMake.
if(APPLE)
    set(CURSES_NEED_NCURSES TRUE)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/ncurses")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/ncurses")
endif()

find_package(Curses REQUIRED)

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
set(HEXEDIT_SOURCES
    src/main.cpp
    src/application.cpp
    src/file_buffer.cpp
    src/file_manager.cpp
    src/display_view.cpp
    src/editor.cpp
    src/command_handler.cpp
    src/undo_history.cpp
    src/search.cpp
)

add_executable(hexedit ${HEXEDIT_SOURCES})

target_include_directories(hexedit PRIVATE
    ${CURSES_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(hexedit PRIVATE ${CURSES_LIBRARIES})

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()

# Test executable — links against all modules except main.cpp.
set(TEST_SOURCES
    tests/test_main.cpp
    src/file_buffer.cpp
    src/file_manager.cpp
    src/editor.cpp
    src/undo_history.cpp
    src/search.cpp
)

add_executable(hexedit_tests ${TEST_SOURCES})

target_include_directories(hexedit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Tests don't need ncurses (no display_view or command_handler).
add_test(NAME HexEditorTests COMMAND hexedit_tests)

# ---------------------------------------------------------------------------
# Install (optional)
# ---------------------------------------------------------------------------
install(TARGETS hexedit DESTINATION bin)
