#include "VocabBuilderStore.h"

#include <algorithm>
#include <cctype>
#include <cstring>

#ifdef NATIVE_TEST
#include <fstream>
#include <iostream>
#define LOG_ERR(tag, ...)
#define LOG_INF(tag, ...)
#else
#include <HalStorage.h>
#include <Logging.h>
#include <common/FsApiConstants.h>
#endif

const char* VocabBuilderStore::resolvePath(const char* customPath) {
  if (customPath && customPath[0] != '\0') {
    return customPath;
  }
  return DEFAULT_FILE_PATH;
}

bool VocabBuilderStore::cleanWord(const std::string& input, std::string& output, bool normalizeLowercase) {
  if (input.empty()) return false;

  auto matchTrimSeq = [](const char* p, size_t remaining) -> size_t {
    if (remaining == 0) return 0;
    unsigned char c0 = static_cast<unsigned char>(p[0]);

    // 1-byte ASCII punctuation, whitespace, brackets, quotes
    if (c0 < 0x80) {
      if (c0 <= ' ' || c0 == '"' || c0 == '\'' || c0 == '`' ||
          c0 == '.' || c0 == ',' || c0 == '!' || c0 == '?' ||
          c0 == ';' || c0 == ':' || c0 == '(' || c0 == ')' ||
          c0 == '[' || c0 == ']' || c0 == '{' || c0 == '}' ||
          c0 == '<' || c0 == '>' || c0 == '/' || c0 == '\\' ||
          c0 == '|' || c0 == '*' || c0 == '+' || c0 == '=' ||
          c0 == '^' || c0 == '%' || c0 == '$' || c0 == '#' ||
          c0 == '@' || c0 == '~' || c0 == '_' || c0 == '-') {
        return 1;
      }
      return 0;
    }

    // 2-byte UTF-8 sequences to trim (NBSP: 0xC2 0xA0, Guillemets: 0xC2 0xAB, 0xC2 0xBB)
    if (remaining >= 2 && c0 == 0xC2) {
      unsigned char c1 = static_cast<unsigned char>(p[1]);
      if (c1 == 0xA0 || c1 == 0xAB || c1 == 0xBB) {
        return 2;
      }
    }

    // 3-byte UTF-8 sequences to trim
    // Quotes: ‘ (0xE2 0x80 0x98), ’ (0xE2 0x80 0x99), ‚ (0xE2 0x80 0x9A),
    //         “ (0xE2 0x80 0x9C), ” (0xE2 0x80 0x9D), „ (0xE2 0x80 0x9E)
    // Dashes: – (0xE2 0x80 0x93, en dash), — (0xE2 0x80 0x94, em dash)
    // Other:  … (0xE2 0x80 0xA6, ellipsis), em-space (0xE2 0x80 0x83)
    if (remaining >= 3 && c0 == 0xE2 && static_cast<unsigned char>(p[1]) == 0x80) {
      unsigned char c2 = static_cast<unsigned char>(p[2]);
      if ((c2 >= 0x98 && c2 <= 0x9E) ||
          c2 == 0x93 || c2 == 0x94 ||
          c2 == 0xA6 || c2 == 0x83) {
        return 3;
      }
    }

    return 0;
  };

  // Find start: skip leading trim characters
  size_t start = 0;
  while (start < input.size()) {
    size_t match = matchTrimSeq(input.data() + start, input.size() - start);
    if (match > 0) {
      start += match;
    } else {
      break;
    }
  }

  if (start >= input.size()) return false;

  // Find end: skip trailing trim characters
  size_t end = input.size();
  while (end > start) {
    size_t charStart = end - 1;
    while (charStart > start && (static_cast<unsigned char>(input[charStart]) & 0xC0) == 0x80) {
      charStart--;
    }
    size_t charLen = end - charStart;
    size_t match = matchTrimSeq(input.data() + charStart, charLen);
    if (match == charLen) {
      end = charStart;
    } else {
      break;
    }
  }

  if (end <= start) return false;

  size_t wordLen = end - start;
  if (wordLen > MAX_WORD_BYTES) {
    size_t safeLen = MAX_WORD_BYTES;
    while (safeLen > 0 && (static_cast<unsigned char>(input[start + safeLen]) & 0xC0) == 0x80) {
      safeLen--;
    }
    wordLen = safeLen;
  }

  output.clear();
  output.reserve(wordLen);

  for (size_t i = 0; i < wordLen; ++i) {
    char c = input[start + i];
    if (normalizeLowercase && c >= 'A' && c <= 'Z') {
      output.push_back(static_cast<char>(c + 32));
    } else {
      output.push_back(c);
    }
  }

  return !output.empty();
}

std::vector<std::string> VocabBuilderStore::loadWords(const char* customPath) {
  std::vector<std::string> words;
  const char* path = resolvePath(customPath);

#ifndef NATIVE_TEST
  HalFile file = Storage.open(path, O_RDONLY);
  if (!file) return words;

  std::string line;
  char buf[256];
  int bytesRead = 0;
  while ((bytesRead = file.read(buf, sizeof(buf))) > 0) {
    for (int i = 0; i < bytesRead; ++i) {
      char c = buf[i];
      if (c == '\n') {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(0, 1);
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) line.pop_back();
        if (!line.empty()) words.push_back(line);
        line.clear();
      } else {
        line.push_back(c);
      }
    }
  }
  if (!line.empty()) {
    if (line.back() == '\r') line.pop_back();
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(0, 1);
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) line.pop_back();
    if (!line.empty()) words.push_back(line);
  }
  file.close();
#else
  std::ifstream f(path);
  if (!f.is_open()) return words;
  std::string line;
  while (std::getline(f, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(0, 1);
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) line.pop_back();
    if (!line.empty()) words.push_back(line);
  }
  f.close();
#endif
  return words;
}

bool VocabBuilderStore::containsWord(const std::string& word, const char* customPath) {
  if (word.empty()) return false;
  auto words = loadWords(customPath);
  for (const auto& w : words) {
    if (w.size() == word.size()) {
#if defined(_WIN32)
      if (_stricmp(w.c_str(), word.c_str()) == 0) return true;
#else
      if (strcasecmp(w.c_str(), word.c_str()) == 0) return true;
#endif
    }
  }
  return false;
}

VocabBuilderStore::SaveResult VocabBuilderStore::saveWord(const std::string& rawText, bool normalizeLowercase,
                                                         const char* customPath) {
  std::string clean;
  if (!cleanWord(rawText, clean, normalizeLowercase)) {
    return SaveResult::EmptyWord;
  }
  const char* path = resolvePath(customPath);

  if (containsWord(clean, path)) {
    return SaveResult::AlreadyExists;
  }

#ifndef NATIVE_TEST
  HalFile file = Storage.open(path, O_RDWR | O_CREAT | O_AT_END);
  if (!file) {
    LOG_ERR("VOCAB", "Failed to open %s for append", path);
    return SaveResult::Failed;
  }
  const size_t size = file.fileSize();
  if (size > 0) {
    if (file.seek(size - 1)) {
      int lastByte = file.read();
      if (lastByte >= 0 && lastByte != '\n') {
        file.seek(size);
        file.write(static_cast<uint8_t>('\n'));
      } else {
        file.seek(size);
      }
    }
  }
  file.write(reinterpret_cast<const uint8_t*>(clean.data()), clean.size());
  file.write(static_cast<uint8_t>('\n'));
  file.flush();
  file.close();
#else
  std::ofstream f(path, std::ios::out | std::ios::app);
  if (!f.is_open()) return SaveResult::Failed;
  f << clean << "\n";
  f.close();
#endif
  return SaveResult::Added;
}

bool VocabBuilderStore::rewriteFile(const std::vector<std::string>& words, const char* customPath) {
  const char* path = resolvePath(customPath);
#ifndef NATIVE_TEST
  HalFile file = Storage.open(path, O_RDWR | O_CREAT | O_TRUNC);
  if (!file) return false;
  for (const auto& w : words) {
    file.write(reinterpret_cast<const uint8_t*>(w.data()), w.size());
    file.write(static_cast<uint8_t>('\n'));
  }
  file.flush();
  file.close();
  return true;
#else
  std::ofstream f(path, std::ios::out | std::ios::trunc);
  if (!f.is_open()) return false;
  for (const auto& w : words) {
    f << w << "\n";
  }
  f.close();
  return true;
#endif
}

bool VocabBuilderStore::deleteWordAtIndex(size_t index, const char* customPath) {
  auto words = loadWords(customPath);
  if (index >= words.size()) return false;
  words.erase(words.begin() + index);
  return rewriteFile(words, customPath);
}

bool VocabBuilderStore::deleteWord(const std::string& word, const char* customPath) {
  auto words = loadWords(customPath);
  bool found = false;
  for (auto it = words.begin(); it != words.end(); ) {
#if defined(_WIN32)
    if (_stricmp(it->c_str(), word.c_str()) == 0) {
#else
    if (strcasecmp(it->c_str(), word.c_str()) == 0) {
#endif
      it = words.erase(it);
      found = true;
    } else {
      ++it;
    }
  }
  if (!found) return false;
  return rewriteFile(words, customPath);
}

bool VocabBuilderStore::clearAll(const char* customPath) {
  std::vector<std::string> emptyList;
  return rewriteFile(emptyList, customPath);
}

size_t VocabBuilderStore::countWords(const char* customPath) {
  return loadWords(customPath).size();
}
