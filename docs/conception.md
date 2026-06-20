# Conception Document — HEX/BIN/CHAR File Editor

## Software Vision and User Interaction

The goal of this project is to design a terminal-based binary file editor that allows a user to inspect and modify the raw bytes of any file. The software presents file contents in a synchronised, tabular view with three parallel representations per byte: hexadecimal, binary, and character. The user navigates through the file byte-by-byte and can enter edit mode at any position, typing a new value in whichever column (HEX, BIN, or CHAR) they prefer. After editing, the representation automatically updates across all three columns. The file can be opened via a command-line argument or entered interactively at startup, and the modified contents can be saved back to disk at any point. A confirmation dialog protects against accidental loss of unsaved changes.

## Main Functions

**File I/O** — Opening a binary file reads all bytes into memory. Saving writes the in-memory buffer back to disk, replacing the file's original content. Errors (file not found, permission denied) are reported to the user via the status bar without crashing the program.

**Tabular Display** — The screen is divided into four columns: byte offset (hexadecimal address), hexadecimal values, binary bit patterns, and printable characters. Each row shows 8 bytes. Non-printable bytes are shown as `.` in the character column. The currently selected byte is highlighted in all three columns simultaneously.

**Editing** — The user activates edit mode with `E` or `Enter`. In HEX mode, two hex digits (e.g. `4F`) are accumulated before the byte is committed. In BIN mode, eight binary digits are required. In CHAR mode, a single printable ASCII character is applied immediately. Invalid characters for the current mode are silently ignored.

**Navigation** — Arrow keys move the cursor byte-by-byte or row-by-row. Page Up/Down scroll the view by a full screen. `G` prompts for a hex offset to jump to. `Home`/`End` jump to the first and last byte.

**Undo / Redo** — Every committed byte edit is recorded in a dual-stack undo history. `U` reverses the last edit; `R` re-applies it. Starting a new edit clears the redo stack.

**Search** — Pressing `/` opens a prompt. The user enters a byte pattern as a hex string (e.g. `48 65 6C 6C 6F`) or as ASCII text (e.g. `Hello`). The editor scans the buffer linearly and positions the cursor at the first match. `N` and `Shift+N` cycle through further matches forward and backward.

## Module Structure and Interaction

The software is decomposed into nine modules to separate concerns and facilitate independent testing.

**`FileManager`** performs all disk operations and returns errors as strings rather than throwing exceptions, keeping error handling straightforward. It feeds raw byte vectors to `FileBuffer`.

**`FileBuffer`** holds the in-memory byte array, provides bounds-checked `getByte`/`setByte` accessors, and maintains a dirty flag to track unsaved changes.

**`UndoHistory`** maintains two stacks of `EditAction` records (offset, old value, new value). Pushing a new action clears the redo stack; `undo()` and `redo()` pop from one stack and push to the other.

**`Editor`** mediates between the user's keystrokes and the buffer. It accumulates partial input (nibbles for HEX, bits for BIN) and commits a complete byte only when the required number of characters have been entered. On commit it pushes an `EditAction` to `UndoHistory`.

**`Search`** provides static methods for parsing patterns and performing linear forward/backward scans over the buffer data.

**`DisplayView`** wraps the NCurses library in a RAII interface. Its `render()` method draws the full screen — header, column headings, data rows, and a status bar — and applies colour pairs to differentiate the cursor, editing state, and non-printable characters. It also provides `promptInput()` and `confirmDialog()` for modal text entry.

**`CommandHandler`** translates raw NCurses key codes into a `Command` enum, isolating all key-binding knowledge in one place and keeping the rest of the code free of key-code literals.

**`Application`** owns all module instances and runs the main event loop: read a key → translate to command (or pass to editor if in edit mode) → execute the command → re-render. It manages the `AppState` struct (cursor offset, scroll offset, active column, file path, status message).

**`main.cpp`** parses command-line arguments and creates the `Application` instance.

## Language Choice: C++

C++ was chosen over C for three reasons. First, the Standard Library containers (`std::vector<uint8_t>`, `std::stack`, `std::optional`) eliminate manual memory management for the buffer and history stacks. Second, classes with constructors and destructors provide RAII for NCurses initialisation and file handles, preventing resource leaks. Third, `std::optional` as a return type from `getByte`, `undo`, and search functions makes the absence of a value explicit and avoids sentinel integer return codes, leading to safer, more readable code.
