#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class VocabBuilderStore {
 public:
  static constexpr const char* DEFAULT_FILE_PATH = "/vocabulary_builder.txt";
  static constexpr size_t MAX_WORD_BYTES = 128;

  enum class SaveResult : uint8_t {
    Added,
    AlreadyExists,
    EmptyWord,
    Failed,
  };

  // Cleans leading/trailing punctuation, quotes, and whitespace from raw text.
  // Preserves internal apostrophes/hyphens and UTF-8 characters.
  // Optionally normalizes ASCII characters to lowercase.
  static bool cleanWord(const std::string& input, std::string& output, bool normalizeLowercase = false);

  // Saves a plain word to the vocabulary builder TXT file.
  // Checks for duplicates first. Appends one plain word per line.
  // Only plain word is stored - no origin or metadata.
  static SaveResult saveWord(const std::string& rawText, bool normalizeLowercase = false,
                             const char* customPath = nullptr);

  // Loads all plain words from the TXT file in order.
  static std::vector<std::string> loadWords(const char* customPath = nullptr);

  // Checks if a word is already in the vocabulary list (case-insensitive).
  static bool containsWord(const std::string& word, const char* customPath = nullptr);

  // Deletes a word by index (0-based) from the TXT file.
  static bool deleteWordAtIndex(size_t index, const char* customPath = nullptr);

  // Deletes a word by matching text.
  static bool deleteWord(const std::string& word, const char* customPath = nullptr);

  // Clears all words from the TXT file.
  static bool clearAll(const char* customPath = nullptr);

  // Returns total number of words saved.
  static size_t countWords(const char* customPath = nullptr);

  // Rewrites the TXT file with the given list of plain words.
  static bool rewriteFile(const std::vector<std::string>& words, const char* customPath = nullptr);

  // Helper to resolve the active file path.
  static const char* resolvePath(const char* customPath);
};
