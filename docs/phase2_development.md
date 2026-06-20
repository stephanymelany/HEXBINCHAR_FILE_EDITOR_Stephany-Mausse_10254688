# Portfolio Part 2 — Development / Reflection Phase
## Task 1: A HEX/BIN/CHAR File Editor — Programming with C/C++ (DLBROEPRS01_D)

---

## GIT Repository

Complete source code, build configuration, and tests:
https://github.com/stephanymelany/HEXBINCHAR_FILE_EDITOR_Stephany-Mausse_10254688

---

## 1. Software Structure

The first version of the software is complete and operational. It is implemented in C++17 as a terminal application using the NCurses library, structured into nine modules that separate file I/O, in-memory data, editing logic, undo/redo history, search, display, and command translation. The architecture follows a single orchestrator pattern:

```
main.cpp -> Application -> FileManager -> FileBuffer <- Editor <- DisplayView
                        -> CommandHandler -------------+-- UndoHistory
                        -> Search ----------------------------------+
```

Each module has one clearly bounded responsibility, and `Application` owns all module instances, running the event loop: read a key, translate it to a command (or forward it to the editor while in edit mode), execute the command, and re-render.

### Module Annotations

**`FileManager`** — static `readFile()`/`writeFile()` methods perform binary disk I/O and return errors as strings rather than throwing, keeping error handling at the call site straightforward (`file_manager.cpp`/`.hpp`).

**`FileBuffer`** — owns the in-memory `std::vector<uint8_t>`, exposes bounds-checked `getByte()`/`setByte()` returning `std::optional<uint8_t>`, and tracks a dirty flag for unsaved-change protection (`file_buffer.cpp`/`.hpp`).

**`UndoHistory`** — two `std::stack<EditAction>` stacks (undo/redo); `push()` clears the redo stack; `undo()`/`redo()` pop from one stack and push to the other, returning `std::optional<EditAction>` (`undo_history.cpp`/`.hpp`).

**`Editor`** — mediates between keystrokes and the buffer; accumulates partial input (two hex nibbles, eight binary digits, or one character) and commits a full byte once the format's input is complete, pushing an `EditAction` to `UndoHistory` on commit (`editor.cpp`/`.hpp`).

**`Search`** — static `parseHexPattern()`/`parseAsciiPattern()` convert user input into a byte pattern; `findNext()`/`findPrev()` perform linear forward/backward scans over the buffer (`search.cpp`/`.hpp`).

**`DisplayView`** — wraps NCurses in a RAII interface (`init()`/`shutdown()`); `render()` draws the header, column headings, 8-bytes-per-row data table, and status bar, applying colour pairs for the cursor, edit state, and non-printable bytes; `promptInput()`/`confirmDialog()` provide modal text entry (`display_view.cpp`/`.hpp`).

**`CommandHandler`** — a single static `processKey()` translates raw NCurses key codes into a `Command` enum, isolating all key-binding knowledge from the rest of the codebase (`command_handler.cpp`/`.hpp`).

**`Application`** — owns every module instance and the `AppState` struct (cursor offset, scroll offset, active column, file path, status message); contains the main event loop and all command handlers (navigation, column switching, editing, file operations, search, quit) (`application.cpp`/`.hpp`).

**`main.cpp`** — parses command-line arguments (a file path, or `--help`) and constructs the `Application` instance.

---

## 2. Build, Run, and Test Instructions

### Requirements

- macOS or Linux
- C++17-compatible compiler (Apple Clang 12+, GCC 9+, or Clang 10+)
- CMake ≥ 3.16
- NCurses library

### Install Dependencies

**macOS (Homebrew):**
```bash
brew install cmake ncurses
```

**Ubuntu / Debian:**
```bash
sudo apt install cmake libncurses-dev
```

### Build

```bash
git clone https://github.com/stephanymelany/HEXBINCHAR_FILE_EDITOR_Stephany-Mausse_10254688.git
cd HEXBINCHAR_FILE_EDITOR_Stephany-Mausse_10254688
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The binary is produced at `build/hexedit`.

### Run

```bash
./build/hexedit path/to/file.bin    # open a file directly
./build/hexedit                     # prompt for a file path interactively
./build/hexedit --help              # show usage and key bindings
```

### Test

The CMake configuration builds a second executable, `hexedit_tests`, linking against every module except `main.cpp` (no NCurses dependency required for testing), and registers it with CTest:

```bash
cmake --build build
cd build && ctest --output-on-failure
```

The suite covers **FileBuffer** (4 tests), **UndoHistory** (5 tests), **Editor** (5 tests), **Search** (6 tests), and **FileManager** (3 tests) — **23 tests** in total, using a minimal assert-based framework with no external dependencies (`tests/test_main.cpp`).

---

## 3. Status

All nine planned modules are implemented and tested individually, their interactions are implemented and exercised by the integration paths in `Application`, and the overall software structure and user interaction described in the Phase 1 concept document are functional: navigation, column switching, editing in all three formats, undo/redo, search, and save/open all operate correctly against the current build. No outstanding defects were identified during this phase; further refinement is addressed in the Finalisation Phase.
