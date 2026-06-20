# HEX/BIN/CHAR File Editor

A terminal-based binary file editor written in C++17 that displays file contents simultaneously in **hexadecimal**, **binary**, and **character** format. Values can be edited in any of the three representations, and changes can be saved back to disk.

## Features

- **Tri-column display**: offset | hex | binary | char, all synchronised
- **Edit in any format**: type hex digits, binary bits, or ASCII characters
- **Undo / Redo**: full history of byte-level edits
- **Search**: find byte patterns by hex string or ASCII text
- **Go-to offset**: jump directly to any byte by hex address
- **Unsaved-changes protection**: confirmation prompts before destructive operations
- **Colour-coded UI**: cursor, modified bytes, non-printable characters highlighted

## Screenshot

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ HEX/BIN/CHAR Editor v1.0                         tests/test_data/ascii.txt  │
├──────────┬─────────────────────────────┬────────────────────────────────────┤
│  Offset  │ Hex                         │ Binary                   │ Char     │
│ 00000000 │ 48 65 6C 6C 6F 20 57 6F    │ 01001000 01100101 ...    │ Hello Wo │
│ 00000008 │ 72 6C 64 21 0A 54 68 69    │ 01110010 01101100 ...    │ rld!.Thi │
│ ...                                                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│ [Arrows] Navigate  [TAB] Column  [E] Edit  [S] Save  [Q] Quit  [/] Search  │
│ Offset: 0x00 (0)   Size: 33 bytes   Column: HEX                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Requirements

- **macOS** or **Linux**
- **C++17** compatible compiler (GCC 9+ or Clang 10+)
- **ncurses** library

### macOS (Homebrew)
```bash
brew install ncurses
```

### Linux (Debian/Ubuntu)
```bash
sudo apt install libncurses-dev
```

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

```bash
./hexedit [filename]    # Open a file directly
./hexedit               # Prompt for a file path interactively
./hexedit --help        # Show usage information
```

## Key Bindings

| Key              | Action                          |
|------------------|---------------------------------|
| Arrow keys       | Navigate through bytes          |
| TAB              | Switch column (HEX / BIN / CHAR)|
| E / Enter        | Enter edit mode                 |
| ESC              | Cancel current edit             |
| S                | Save file                       |
| O                | Open a different file           |
| U                | Undo last edit                  |
| R                | Redo                            |
| /                | Search for a byte pattern       |
| N                | Find next match                 |
| Shift+N          | Find previous match             |
| G                | Go to a specific offset         |
| PgUp / PgDn      | Scroll by page                  |
| Home / End       | Jump to start / end of file     |
| Q                | Quit                            |

## Edit Formats

- **HEX column**: Enter two hex digits (0–9, A–F)
- **BIN column**: Enter eight binary digits (0 or 1)
- **CHAR column**: Enter one printable ASCII character

## Running Tests

```bash
cd build
ctest --verbose
```

## Project Structure

```
.
├── CMakeLists.txt
├── README.md
├── docs/
│   └── conception.md
├── src/
│   ├── main.cpp
│   ├── application.hpp / .cpp
│   ├── file_buffer.hpp / .cpp
│   ├── file_manager.hpp / .cpp
│   ├── display_view.hpp / .cpp
│   ├── editor.hpp / .cpp
│   ├── command_handler.hpp / .cpp
│   ├── undo_history.hpp / .cpp
│   └── search.hpp / .cpp
└── tests/
    └── test_main.cpp
```