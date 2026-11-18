#define NATIVE_TEST 1
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>

// Include store implementation directly for host test
#include "../VocabBuilderStore.h"
#include "../VocabBuilderStore.cpp"

void testCleanWord() {
    std::string out;

    // Basic word
    assert(VocabBuilderStore::cleanWord("hello", out, false) && out == "hello");
    assert(VocabBuilderStore::cleanWord("Hello", out, true) && out == "hello");
    assert(VocabBuilderStore::cleanWord("Hello", out, false) && out == "Hello");

    // Quotes and punctuation
    assert(VocabBuilderStore::cleanWord("\"ephemeral\"", out, false) && out == "ephemeral");
    assert(VocabBuilderStore::cleanWord("'serendipity'", out, false) && out == "serendipity");
    assert(VocabBuilderStore::cleanWord("ubiquitous.", out, false) && out == "ubiquitous");
    assert(VocabBuilderStore::cleanWord("mellifluous,", out, false) && out == "mellifluous");
    assert(VocabBuilderStore::cleanWord("(eloquent)", out, false) && out == "eloquent");
    assert(VocabBuilderStore::cleanWord("[ineffable!]", out, false) && out == "ineffable");

    // Multi-byte UTF-8 curly quotes and dashes
    // "“magnificent”"
    std::string curly = std::string("\xE2\x80\x9C") + "magnificent" + "\xE2\x80\x9D";
    assert(VocabBuilderStore::cleanWord(curly, out, false) && out == "magnificent");

    // Em-dash: "—conundrum—"
    std::string emdash = std::string("\xE2\x80\x94") + "conundrum" + "\xE2\x80\x94";
    assert(VocabBuilderStore::cleanWord(emdash, out, false) && out == "conundrum");

    // Preserving internal punctuation
    assert(VocabBuilderStore::cleanWord("don't", out, false) && out == "don't");
    assert(VocabBuilderStore::cleanWord("twenty-one", out, false) && out == "twenty-one");
    assert(VocabBuilderStore::cleanWord("\"well-known,\"", out, false) && out == "well-known");

    // Preserving foreign UTF-8 letters
    std::string cafe = std::string("\xE2\x80\x9C") + "caf\xC3\xA9" + "!" + "\xE2\x80\x9D";
    assert(VocabBuilderStore::cleanWord(cafe, out, false) && out == "caf\xC3\xA9");

    // Invalid / empty words
    assert(!VocabBuilderStore::cleanWord("", out, false));
    assert(!VocabBuilderStore::cleanWord("   \t\n  ", out, false));
    assert(!VocabBuilderStore::cleanWord("...", out, false));
    assert(!VocabBuilderStore::cleanWord("\"'--'\"", out, false));

    std::cout << "testCleanWord passed!" << std::endl;
}

void testFileStorage() {
    const char* testPath = "/tmp/test_vocab.txt";
    std::remove(testPath);

    // Initial empty
    assert(VocabBuilderStore::countWords(testPath) == 0);
    assert(VocabBuilderStore::loadWords(testPath).empty());

    // Save words
    auto res1 = VocabBuilderStore::saveWord("Serendipity", true, testPath);
    assert(res1 == VocabBuilderStore::SaveResult::Added);
    assert(VocabBuilderStore::countWords(testPath) == 1);

    // Duplicate check (case-insensitive)
    auto res2 = VocabBuilderStore::saveWord("\"serendipity,\"", true, testPath);
    assert(res2 == VocabBuilderStore::SaveResult::AlreadyExists);
    assert(VocabBuilderStore::countWords(testPath) == 1);

    // Another word
    auto res3 = VocabBuilderStore::saveWord("Ephemeral", true, testPath);
    assert(res3 == VocabBuilderStore::SaveResult::Added);
    assert(VocabBuilderStore::countWords(testPath) == 2);

    auto words = VocabBuilderStore::loadWords(testPath);
    assert(words.size() == 2);
    assert(words[0] == "serendipity");
    assert(words[1] == "ephemeral");

    // Check TXT file format: only plain words, one per line!
    std::ifstream in(testPath);
    std::string l1, l2;
    std::getline(in, l1);
    std::getline(in, l2);
    assert(l1 == "serendipity");
    assert(l2 == "ephemeral");
    in.close();

    // Delete word
    assert(VocabBuilderStore::deleteWordAtIndex(0, testPath));
    assert(VocabBuilderStore::countWords(testPath) == 1);
    assert(VocabBuilderStore::loadWords(testPath)[0] == "ephemeral");

    // Clear all
    assert(VocabBuilderStore::clearAll(testPath));
    assert(VocabBuilderStore::countWords(testPath) == 0);

    std::remove(testPath);
    std::cout << "testFileStorage passed!" << std::endl;
}

int main() {
    testCleanWord();
    testFileStorage();
    std::cout << "ALL TESTS PASSED!" << std::endl;
    return 0;
}
