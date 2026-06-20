# Portfolio Part 3 — Finalisation Phase
## Abstract: Design and Implementation of a HEX/BIN/CHAR File Editor
### Programming with C/C++ (DLBROEPRS01_D)
**Mausse, Stephany | Matriculation No. 10254688**

---

## 1. Problem Definition and Objective

Inspecting and modifying the raw byte content of a file is a recurring need in systems programming, reverse engineering, and embedded development, yet general-purpose text editors are unsuitable for this task because they interpret bytes through a character encoding and cannot reliably display or edit arbitrary binary data. The objective of this project was to design and implement a terminal-based file editor that displays the full content of any file simultaneously in three synchronised representations — hexadecimal, binary, and printable character — and that allows the user to edit a byte through any of the three representations while keeping the other two consistent. The core challenges were: maintaining a single source of truth for the byte data while presenting and editing it through three different input grammars; providing safe, reversible editing through undo and redo; and doing so within a responsive terminal interface without an external GUI toolkit.

---

## 2. Methodology and Approach

The development strategy followed the three-phase structure required by the module: a conceptualisation phase in which the software's interaction model and module boundaries were defined before any code was written; a development and reflection phase in which each module was implemented and unit-tested in isolation before being integrated; and a finalisation phase in which the integrated system was reviewed, refined, and documented.

The chosen architecture decomposes the system into nine single-responsibility modules coordinated by one orchestrator class, `Application`, which owns all module instances and runs the central event loop of read key, translate to command, execute, and re-render. This separation allows the data layer (`FileBuffer`, `UndoHistory`), the logic layer (`Editor`, `Search`, `FileManager`), and the presentation layer (`DisplayView`, `CommandHandler`) to be tested independently of one another and independently of the terminal itself, which is essential since five of the nine modules have no dependency on NCurses and can therefore be exercised by an automated test suite.

C++17 was selected over C specifically for `std::optional`, used throughout as the return type of fallible operations such as `getByte`, `undo`, and `redo`, which removes the need for sentinel error codes, and for RAII, which guarantees that NCurses state and file handles are released correctly even on early-return error paths.

---

## 3. Quality of Implementation

The implementation comprises nine source/header module pairs plus a CMake-based build system targeting C++17 with warnings treated as a quality gate (`-Wall -Wextra -Wpedantic`). Each module exposes a narrow, documented interface (see the Doxygen-style header comments in the repository) and internal state is kept private, accessed only through bounds-checked accessors.

A dedicated test executable, `hexedit_tests`, links against every module except `main.cpp` and display/command-handling code, and is registered with CTest; it currently runs **23 unit tests** across `FileBuffer`, `UndoHistory`, `Editor`, `Search`, and `FileManager`, covering both expected behaviour (round-trip file I/O, correct nibble/bit accumulation, undo/redo correctness) and edge cases (out-of-range offsets, invalid input characters, empty files, patterns not found). All tests pass on the current build.

---

## 4. Creativity and Correctness

Beyond the literal requirements of the task, the implementation adds:

- A **dual-stack undo/redo system** with standard redo-invalidation semantics.
- A **unified search facility** that accepts either a hex byte pattern or plain ASCII text and disambiguates automatically.
- **Colour-coded highlighting** that distinguishes the cursor, an in-progress edit, and non-printable bytes simultaneously across all three columns.
- An **unsaved-changes guard** that prompts for confirmation before a destructive open or quit.

Correctness was prioritised over premature optimisation: all buffer access goes through bounds-checked accessors returning `std::optional`, and the search algorithm, while linear rather than using a more advanced string-matching algorithm such as Boyer–Moore, was deliberately chosen for its simplicity and verifiability given the typical file sizes this tool targets.

---

## 5. Conclusion

The completed HEX/BIN/CHAR file editor satisfies the task brief: it displays file content in tabular hexadecimal, binary, and character form; supports editing through any of the three representations with synchronised updates; accepts a file path as a command-line argument or interactively; and persists changes back to disk. The modular architecture and accompanying unit test suite were the two design decisions with the greatest impact on the project's quality, since they allowed the majority of the logic to be verified mechanically rather than through manual terminal testing alone.

The complete source code, build instructions, and test suite are available at:
https://github.com/stephanymelany/HEXBINCHAR_FILE_EDITOR_Stephany-Mausse_10254688

---

## Summary of Key Decisions and Solutions

### Architectural Decisions

**Single orchestrator pattern:** `Application` owns all module instances rather than modules referencing each other directly, which keeps coupling explicit and makes the data flow (`FileManager` → `FileBuffer` ← `Editor` ← `DisplayView`) traceable from one file.

**Separation of NCurses-dependent and NCurses-independent code:** `DisplayView` and `CommandHandler` are the only modules that touch the terminal library; this was a deliberate decision taken in the conceptualisation phase specifically so the remaining seven modules could be unit-tested in a normal CI-style environment without a terminal attached.

**`std::optional` over sentinel values:** every fallible accessor (`FileBuffer::getByte`, `UndoHistory::undo`/`redo`, `Search::findNext`/`findPrev`) returns `std::optional<T>` instead of a boolean flag plus an out-parameter, removing an entire class of forgotten-check bugs.

**Error reporting as strings rather than exceptions:** `FileManager` reports I/O failures as a `std::string` in a pair return rather than throwing, which keeps the call sites in `Application` simple if/else checks and avoids introducing exception-handling paths into a terminal UI loop where an uncaught exception would corrupt the screen state.

### Editing Model

**Partial-input accumulation:** the `Editor` module accumulates characters until a format-specific threshold is reached (two hex nibbles, eight binary digits, one character) before committing a byte; this was chosen over committing on every keystroke so that an in-progress, invalid intermediate value is never written to the buffer or pushed to undo history.

**Auto-advance after commit:** after a byte is successfully edited, the cursor automatically moves to the next offset, which was added during the development phase after observing that editing a run of consecutive bytes otherwise required an extra keypress per byte.

### Refinements Made During Finalisation

- Verified that all 23 unit tests pass cleanly and that the test executable builds without requiring NCurses, confirming the module boundary chosen in the conceptualisation phase holds in practice.
- Reviewed all public headers for consistent Doxygen-style documentation so the structure described in the Portfolio Part 2 documentation matches the delivered code exactly.
- Confirmed build reproducibility from a clean clone using the documented CMake sequence, and confirmed the binary is produced at the documented path (`build/hexedit`).
- Removed unused `#include` directives flagged by the IDE (`<tuple>`, `<iterator>`, `<iomanip>`) and added a `.clangd` configuration to enforce C++17 in the language server, eliminating all IDE diagnostics.

---

## Known Limitations

- **Search** uses a linear scan rather than a sub-linear string-matching algorithm; acceptable for the editor's intended use on moderately sized files, but would not scale to multi-gigabyte inputs.
- The editor **loads the entire file into memory** (`std::vector<uint8_t>`), which is simple and fast for typical files but is not suitable for files larger than available RAM; this was a conscious scope decision recorded already in the Phase 1 concept document.

---

## Final Product and Submission Bundle

This final portfolio part bundles the completed software together with the results delivered in Phase 1 and Phase 2, as required for the Finalisation Phase submission.

### GIT Repository (Final State)

Complete, current source code:
https://github.com/stephanymelany/HEXBINCHAR_FILE_EDITOR_Stephany-Mausse_10254688

### Result from Phase 1 — Conceptualisation

See [`phase1_conceptualisation.md`](phase1_conceptualisation.md) in this `docs/` directory.

### Result from Phase 2 — Development / Reflection

See [`phase2_development.md`](phase2_development.md) in this `docs/` directory.

---

## Affidavit — Reminder

Per the module requirements, the affidavit confirming independent authorship of this portfolio must be submitted electronically via myCampus before this examination performance can be submitted. Please ensure this has been completed in myCampus prior to uploading this Phase 3 document.
