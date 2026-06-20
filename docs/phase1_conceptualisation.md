# Portfolio Part 1 — Conceptualisation Phase
## Task 1: A HEX/BIN/CHAR File Editor — Programming with C/C++ (DLBROEPRS01_D)

---

## Software Vision and User Interaction

The goal is a terminal-based binary file editor that lets a user inspect and modify the raw bytes of any file. The software presents file contents in a synchronised, tabular view with three parallel representations per byte: hexadecimal, binary, and character. The user navigates the file byte-by-byte and can enter edit mode at any position, typing a new value in whichever column (HEX, BIN, or CHAR) is preferred; the other two columns update automatically on commit. The file is opened via a command-line argument or entered interactively at startup, and the modified contents are written back to disk on demand. A confirmation prompt protects against accidental loss of unsaved changes.

---

## Main Functions

**File I/O** — reads all bytes of a file into memory; writes the in-memory buffer back to disk on save; reports errors (file not found, permission denied) via the status bar without crashing.

**Tabular display** — four synchronised columns (offset, hex, binary, char), 8 bytes per row; non-printable bytes shown as `.`; the selected byte is highlighted across all three value columns.

**Editing** — HEX mode accumulates two hex digits before committing a byte; BIN mode requires eight binary digits; CHAR mode commits a single printable ASCII character immediately; invalid input for the active mode is rejected.

**Navigation** — arrow keys move byte-by-byte/row-by-row; Page Up/Down scroll a full screen; `G` jumps to a hex offset; Home/End jump to the file boundaries.

**Undo/Redo** — every committed edit is recorded in a dual-stack history; `U` reverses, `R` re-applies; a new edit clears the redo stack.

**Search** — byte patterns entered as hex (e.g. `48 65 6C`) or ASCII text are located by linear scan; `N` / Shift+N cycle forward/backward through matches.

---

## Module Structure and Interaction

The software is decomposed into nine cooperating modules.

**`FileManager`** performs disk I/O and returns errors as strings, feeding raw byte vectors to `FileBuffer`, which holds the in-memory array with bounds-checked accessors and a dirty flag.

**`UndoHistory`** maintains two stacks of `EditAction` records (offset, old value, new value); pushing a new action clears the redo stack.

**`Editor`** mediates between keystrokes and the buffer: it accumulates partial input (nibbles for HEX, bits for BIN), commits a complete byte once enough characters are entered, and pushes the resulting `EditAction` to `UndoHistory`.

**`Search`** provides static pattern-parsing and linear forward/backward scan methods over the buffer.

**`DisplayView`** wraps NCurses in a RAII interface; its `render()` method draws the header, column headings, data rows, and status bar, and applies colour pairs to distinguish the cursor, edit state, and non-printable bytes; it also exposes `promptInput()` and `confirmDialog()` for modal text entry.

**`CommandHandler`** isolates all key-binding knowledge by translating raw NCurses key codes into a `Command` enum.

**`Application`** owns every module instance and runs the event loop: read a key, translate it to a command (or forward it to the editor in edit mode), execute it, and re-render; it also owns the `AppState` struct (cursor offset, scroll offset, active column, file path, status message).

**`main.cpp`** parses command-line arguments and constructs the `Application`.

---

## Programming Language: C++

C++ was chosen over C for three reasons.

1. Standard Library containers (`std::vector<uint8_t>`, `std::stack`, `std::optional`) remove the need for manual memory management of the buffer and history stacks.
2. Classes with constructors/destructors give RAII for NCurses initialisation and file handles, preventing resource leaks on error paths.
3. Returning `std::optional` from `getByte`, `undo`/`redo`, and the search functions makes the absence of a value explicit, avoiding sentinel integer codes and producing safer, more readable code.
