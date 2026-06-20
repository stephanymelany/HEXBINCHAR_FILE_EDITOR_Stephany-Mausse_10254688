/**
 * @file test_main.cpp
 * @brief Unit tests for the HEX/BIN/CHAR File Editor core modules.
 *
 * Uses a minimal assert-based test framework (no external dependencies).
 * Each test function returns void and uses TEST_ASSERT macros.
 */

#include "file_buffer.hpp"
#include "file_manager.hpp"
#include "editor.hpp"
#include "undo_history.hpp"
#include "search.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// ===========================================================================
// Minimal test framework
// ===========================================================================

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST_ASSERT(expr)                                                    \
    do {                                                                     \
        if (!(expr)) {                                                       \
            std::cerr << "  FAIL: " << #expr                                \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";      \
            throw std::runtime_error("Assertion failed");                    \
        }                                                                    \
    } while (0)

static void runTest(const std::string& name, std::function<void()> fn) {
    std::cout << "  Running: " << name << " ... ";
    try {
        fn();
        std::cout << "PASS\n";
        ++g_testsPassed;
    } catch (...) {
        std::cout << "FAIL\n";
        ++g_testsFailed;
    }
}

// ===========================================================================
// FileBuffer tests
// ===========================================================================

static void test_buffer_empty() {
    FileBuffer buf;
    TEST_ASSERT(buf.size() == 0);
    TEST_ASSERT(buf.empty());
    TEST_ASSERT(!buf.isDirty());
    TEST_ASSERT(buf.getByte(0) == std::nullopt);
}

static void test_buffer_load_and_access() {
    FileBuffer buf;
    buf.loadFromData({0x48, 0x65, 0x6C, 0x6C, 0x6F});  // "Hello"

    TEST_ASSERT(buf.size() == 5);
    TEST_ASSERT(!buf.empty());
    TEST_ASSERT(!buf.isDirty());
    TEST_ASSERT(buf.getByte(0) == 0x48);
    TEST_ASSERT(buf.getByte(4) == 0x6F);
    TEST_ASSERT(buf.getByte(5) == std::nullopt);  // Out of range.
}

static void test_buffer_set_byte() {
    FileBuffer buf;
    buf.loadFromData({0x00, 0x01, 0x02});

    TEST_ASSERT(buf.setByte(1, 0xFF));
    TEST_ASSERT(buf.getByte(1) == 0xFF);
    TEST_ASSERT(buf.isDirty());

    // Out-of-range write should fail.
    TEST_ASSERT(!buf.setByte(10, 0xAA));
}

static void test_buffer_clear_dirty() {
    FileBuffer buf;
    buf.loadFromData({0x00});
    buf.setByte(0, 0xFF);
    TEST_ASSERT(buf.isDirty());
    buf.clearDirty();
    TEST_ASSERT(!buf.isDirty());
}

// ===========================================================================
// UndoHistory tests
// ===========================================================================

static void test_undo_empty() {
    UndoHistory hist;
    TEST_ASSERT(!hist.canUndo());
    TEST_ASSERT(!hist.canRedo());
    TEST_ASSERT(hist.undo() == std::nullopt);
    TEST_ASSERT(hist.redo() == std::nullopt);
}

static void test_undo_push_and_undo() {
    UndoHistory hist;
    EditAction act{5, 0xAA, 0xBB};
    hist.push(act);

    TEST_ASSERT(hist.canUndo());
    TEST_ASSERT(!hist.canRedo());

    auto result = hist.undo();
    TEST_ASSERT(result.has_value());
    TEST_ASSERT(result->offset   == 5);
    TEST_ASSERT(result->oldValue == 0xAA);
    TEST_ASSERT(result->newValue == 0xBB);

    TEST_ASSERT(!hist.canUndo());
    TEST_ASSERT(hist.canRedo());
}

static void test_undo_redo() {
    UndoHistory hist;
    hist.push({0, 0x00, 0x01});
    hist.undo();

    TEST_ASSERT(hist.canRedo());
    auto redone = hist.redo();
    TEST_ASSERT(redone.has_value());
    TEST_ASSERT(redone->newValue == 0x01);
    TEST_ASSERT(hist.canUndo());
    TEST_ASSERT(!hist.canRedo());
}

static void test_undo_new_edit_clears_redo() {
    UndoHistory hist;
    hist.push({0, 0x00, 0x01});
    hist.undo();
    TEST_ASSERT(hist.canRedo());

    // New edit should clear redo stack.
    hist.push({1, 0x00, 0x02});
    TEST_ASSERT(!hist.canRedo());
}

static void test_undo_clear() {
    UndoHistory hist;
    hist.push({0, 0x00, 0x01});
    hist.push({1, 0x00, 0x02});
    hist.clear();
    TEST_ASSERT(!hist.canUndo());
    TEST_ASSERT(!hist.canRedo());
}

// ===========================================================================
// Editor tests
// ===========================================================================

static void test_editor_hex_input() {
    FileBuffer buf;
    UndoHistory hist;
    buf.loadFromData({0x00});
    Editor ed(buf, hist);

    ed.enterEditMode(EditColumn::HEX, 0);
    TEST_ASSERT(ed.isEditing());

    // First hex digit — not yet complete.
    TEST_ASSERT(!ed.handleInput('4'));
    TEST_ASSERT(ed.isEditing());

    // Second hex digit — complete.
    TEST_ASSERT(ed.handleInput('F'));
    TEST_ASSERT(!ed.isEditing());
    TEST_ASSERT(buf.getByte(0) == 0x4F);
    TEST_ASSERT(buf.isDirty());
}

static void test_editor_bin_input() {
    FileBuffer buf;
    UndoHistory hist;
    buf.loadFromData({0x00});
    Editor ed(buf, hist);

    ed.enterEditMode(EditColumn::BIN, 0);
    // Feed 7 bits — not complete yet.
    for (char c : std::string("0000000")) {
        TEST_ASSERT(!ed.handleInput(c));
    }
    // 8th bit — complete, value = 0b00000001 = 1.
    TEST_ASSERT(ed.handleInput('1'));
    TEST_ASSERT(!ed.isEditing());
    TEST_ASSERT(buf.getByte(0) == 0x01);
}

static void test_editor_char_input() {
    FileBuffer buf;
    UndoHistory hist;
    buf.loadFromData({0x00});
    Editor ed(buf, hist);

    ed.enterEditMode(EditColumn::CHAR, 0);
    TEST_ASSERT(ed.handleInput('A'));
    TEST_ASSERT(!ed.isEditing());
    TEST_ASSERT(buf.getByte(0) == 0x41);  // 'A' = 0x41
}

static void test_editor_invalid_char_ignored() {
    FileBuffer buf;
    UndoHistory hist;
    buf.loadFromData({0x00});
    Editor ed(buf, hist);

    ed.enterEditMode(EditColumn::HEX, 0);
    // 'Z' is not a valid hex digit — should be ignored.
    TEST_ASSERT(!ed.handleInput('Z'));
    TEST_ASSERT(ed.isEditing());
    TEST_ASSERT(ed.partialInput().empty());
}

static void test_editor_undo_redo() {
    FileBuffer buf;
    UndoHistory hist;
    buf.loadFromData({0x00, 0x01});
    Editor ed(buf, hist);

    // Edit byte 0: 0x00 → 0xFF
    ed.enterEditMode(EditColumn::HEX, 0);
    ed.handleInput('F');
    ed.handleInput('F');
    TEST_ASSERT(buf.getByte(0) == 0xFF);

    // Undo.
    int off = ed.performUndo();
    TEST_ASSERT(off == 0);
    TEST_ASSERT(buf.getByte(0) == 0x00);

    // Redo.
    off = ed.performRedo();
    TEST_ASSERT(off == 0);
    TEST_ASSERT(buf.getByte(0) == 0xFF);
}

// ===========================================================================
// Search tests
// ===========================================================================

static void test_search_parse_hex() {
    auto pat = Search::parseHexPattern("48 65 6C");
    TEST_ASSERT(pat.size() == 3);
    TEST_ASSERT(pat[0] == 0x48);
    TEST_ASSERT(pat[1] == 0x65);
    TEST_ASSERT(pat[2] == 0x6C);
}

static void test_search_parse_hex_invalid() {
    // "ZZ" is not valid hex — should return empty.
    auto pat = Search::parseHexPattern("ZZ");
    TEST_ASSERT(pat.empty());
}

static void test_search_parse_ascii() {
    auto pat = Search::parseAsciiPattern("Hi");
    TEST_ASSERT(pat.size() == 2);
    TEST_ASSERT(pat[0] == 'H');
    TEST_ASSERT(pat[1] == 'i');
}

static void test_search_find_next() {
    FileBuffer buf;
    buf.loadFromData({0x00, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00});  // .Hello.

    std::vector<uint8_t> pat = {0x48, 0x65, 0x6C};  // "Hel"
    auto result = Search::findNext(buf, pat, 0);
    TEST_ASSERT(result.has_value());
    TEST_ASSERT(*result == 1);

    // No match from offset 2 onwards for full pattern.
    result = Search::findNext(buf, {0x48}, 2);
    TEST_ASSERT(!result.has_value());
}

static void test_search_find_prev() {
    FileBuffer buf;
    buf.loadFromData({0x41, 0x42, 0x41, 0x42});  // ABAB

    std::vector<uint8_t> pat = {0x41};  // 'A'
    auto result = Search::findPrev(buf, pat, 3);
    TEST_ASSERT(result.has_value());
    TEST_ASSERT(*result == 2);
}

static void test_search_not_found() {
    FileBuffer buf;
    buf.loadFromData({0x00, 0x01, 0x02});

    auto result = Search::findNext(buf, {0xFF}, 0);
    TEST_ASSERT(!result.has_value());
}

// ===========================================================================
// FileManager tests (write and read a temp file)
// ===========================================================================

static void test_file_manager_write_read() {
    const std::string tmpPath = "/tmp/hexedit_test_tmp.bin";

    std::vector<uint8_t> original = {0xDE, 0xAD, 0xBE, 0xEF};
    std::string writeErr = FileManager::writeFile(tmpPath, original);
    TEST_ASSERT(writeErr.empty());

    auto [readData, readErr] = FileManager::readFile(tmpPath);
    TEST_ASSERT(readErr.empty());
    TEST_ASSERT(readData == original);

    std::remove(tmpPath.c_str());
}

static void test_file_manager_read_nonexistent() {
    auto [data, err] = FileManager::readFile("/tmp/this_file_does_not_exist_hexedit.bin");
    TEST_ASSERT(!err.empty());
    TEST_ASSERT(data.empty());
}

static void test_file_manager_exists() {
    const std::string tmpPath = "/tmp/hexedit_exists_test.bin";
    std::ofstream f(tmpPath);
    f << "x";
    f.close();

    TEST_ASSERT(FileManager::fileExists(tmpPath));
    TEST_ASSERT(!FileManager::fileExists("/tmp/hexedit_no_such_file_xyz.bin"));

    std::remove(tmpPath.c_str());
}

// ===========================================================================
// main
// ===========================================================================

int main() {
    std::cout << "\n=== HEX/BIN/CHAR File Editor — Unit Tests ===\n\n";

    std::cout << "FileBuffer:\n";
    runTest("empty buffer",          test_buffer_empty);
    runTest("load and access",       test_buffer_load_and_access);
    runTest("set byte",              test_buffer_set_byte);
    runTest("clear dirty",           test_buffer_clear_dirty);

    std::cout << "\nUndoHistory:\n";
    runTest("empty history",         test_undo_empty);
    runTest("push and undo",         test_undo_push_and_undo);
    runTest("undo then redo",        test_undo_redo);
    runTest("new edit clears redo",  test_undo_new_edit_clears_redo);
    runTest("clear",                 test_undo_clear);

    std::cout << "\nEditor:\n";
    runTest("hex input",             test_editor_hex_input);
    runTest("binary input",          test_editor_bin_input);
    runTest("char input",            test_editor_char_input);
    runTest("invalid char ignored",  test_editor_invalid_char_ignored);
    runTest("undo / redo",           test_editor_undo_redo);

    std::cout << "\nSearch:\n";
    runTest("parse hex pattern",     test_search_parse_hex);
    runTest("parse hex invalid",     test_search_parse_hex_invalid);
    runTest("parse ascii pattern",   test_search_parse_ascii);
    runTest("find next",             test_search_find_next);
    runTest("find prev",             test_search_find_prev);
    runTest("not found",             test_search_not_found);

    std::cout << "\nFileManager:\n";
    runTest("write and read",        test_file_manager_write_read);
    runTest("read nonexistent",      test_file_manager_read_nonexistent);
    runTest("file exists",           test_file_manager_exists);

    std::cout << "\n=== Results: "
              << g_testsPassed << " passed, "
              << g_testsFailed << " failed ===\n\n";

    return (g_testsFailed == 0) ? 0 : 1;
}
