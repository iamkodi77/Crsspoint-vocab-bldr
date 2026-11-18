#include "VocabBuilderActivity.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "CrossPointSettings.h"
#include "PluginListDraw.h"
#include "activities/RenderLock.h"
#include "activities/reader/DictionaryDefinitionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "util/Dictionary.h"
#include "vocabulary/VocabBuilderStore.h"

namespace {
constexpr unsigned long NOTICE_DURATION_MS = 1500;

Rect listRectFor(const GfxRenderer& renderer) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = renderer.getScreenHeight() - contentTop - metrics.buttonHintsHeight -
                            metrics.verticalSpacing * 2;
  return Rect{0, contentTop, renderer.getScreenWidth(), contentHeight};
}
}  // namespace

VocabBuilderActivity::VocabBuilderActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("VocabBuilder", renderer, mappedInput) {}

void VocabBuilderActivity::onEnter() {
  Activity::onEnter();
  loadWords();
  requestUpdate();
}

void VocabBuilderActivity::onExit() {
  Activity::onExit();
}

void VocabBuilderActivity::loadWords() {
  words = VocabBuilderStore::loadWords(SETTINGS.vocabFilePath);
  if (words.empty()) {
    screen = Screen::EMPTY;
    selectedIndex = 0;
  } else {
    screen = Screen::LIST;
    if (selectedIndex >= static_cast<int>(words.size())) {
      selectedIndex = static_cast<int>(words.size()) - 1;
    }
    if (selectedIndex < 0) {
      selectedIndex = 0;
    }
  }
}

void VocabBuilderActivity::showNotice(const std::string& msg) {
  noticeMessage = msg;
  showNoticePopup = true;
  noticeTime = millis();
  requestUpdate();
}

void VocabBuilderActivity::moveSelection(int delta) {
  if (screen == Screen::FLASHCARD) {
    if (words.empty()) return;
    const int count = static_cast<int>(words.size());
    flashcardIndex = (flashcardIndex + delta + count) % count;
    requestUpdate();
    return;
  }

  if (screen == Screen::LIST) {
    if (words.empty()) return;
    const int count = static_cast<int>(words.size());
    selectedIndex = (selectedIndex + delta + count) % count;
    requestUpdate();
  }
}

void VocabBuilderActivity::openWordOptions(int index) {
  if (index < 0 || index >= static_cast<int>(words.size())) return;
  const std::string word = words[static_cast<size_t>(index)];

  static const char* const OPTIONS[] = {
      "Look Up in Dictionary",
      "Review Flashcard",
      "Delete Word",
      "Clear All Words",
  };

  optionPopup.show(word.c_str(), OPTIONS, 4, 0, [this, index, word](int choice) {
    if (choice == 0) {
      lookupCurrentWord(word);
    } else if (choice == 1) {
      flashcardIndex = index;
      screen = Screen::FLASHCARD;
      requestUpdate();
    } else if (choice == 2) {
      deleteSelectedWord(index);
    } else if (choice == 3) {
      confirmClearAll();
    }
  });
}

void VocabBuilderActivity::deleteSelectedWord(int index) {
  if (index < 0 || index >= static_cast<int>(words.size())) return;
  const std::string word = words[static_cast<size_t>(index)];

  static const char* const CONFIRM_OPTIONS[] = {"Delete", "Cancel"};
  optionPopup.showMessage("Delete Word?", ("Remove \"" + word + "\" from Vocabulary Builder?").c_str(),
                          CONFIRM_OPTIONS, 2, 0, [this, index, word](int choice) {
                            if (choice == 0) {
                              VocabBuilderStore::deleteWordAtIndex(static_cast<size_t>(index),
                                                                   SETTINGS.vocabFilePath);
                              loadWords();
                              showNotice("Deleted: " + word);
                            }
                          });
}

void VocabBuilderActivity::confirmClearAll() {
  static const char* const CLEAR_OPTIONS[] = {"Clear All", "Cancel"};
  optionPopup.showMessage("Clear All Words?", "Delete all saved words from Vocabulary Builder?",
                          CLEAR_OPTIONS, 2, 1, [this](int choice) {
                            if (choice == 0) {
                              VocabBuilderStore::clearAll(SETTINGS.vocabFilePath);
                              loadWords();
                              showNotice("All words cleared");
                            }
                          });
}

void VocabBuilderActivity::lookupCurrentWord(const std::string& word) {
  if (SETTINGS.dictionaryName[0] == '\0') {
    showNotice("No dictionary selected in Settings");
    return;
  }

  Dictionary dict;
  if (!dict.open(SETTINGS.dictionaryName)) {
    showNotice("Failed to open dictionary");
    return;
  }

  std::string definition;
  std::string headword;
  Dictionary::LookupResult result = Dictionary::LookupResult::NotFound;
  const bool found = dict.lookup(word.c_str(), definition, headword, &result);

  if (found) {
    auto activity = makeUniqueNoThrow<DictionaryDefinitionActivity>(
        renderer, mappedInput, std::move(headword), std::move(definition), dict.definitionsAreHtml());
    if (activity && activity->enableLookup(dict, std::string(word))) {
      startActivityForResult(std::move(activity), [this](const ActivityResult&) {
        requestUpdate();
      });
      return;
    }
  }

  showNotice("Word not found in dictionary");
}

void VocabBuilderActivity::handleConfirm() {
  if (screen == Screen::EMPTY) {
    finish();
    return;
  }

  if (screen == Screen::FLASHCARD) {
    if (!words.empty() && flashcardIndex >= 0 && flashcardIndex < static_cast<int>(words.size())) {
      lookupCurrentWord(words[static_cast<size_t>(flashcardIndex)]);
    }
    return;
  }

  if (screen == Screen::LIST) {
    openWordOptions(selectedIndex);
  }
}

void VocabBuilderActivity::handleBack() {
  if (screen == Screen::FLASHCARD) {
    screen = Screen::LIST;
    requestUpdate();
    return;
  }
  finish();
}

void VocabBuilderActivity::loop() {
  if (showNoticePopup && (millis() - noticeTime) >= NOTICE_DURATION_MS) {
    showNoticePopup = false;
    noticeMessage.clear();
    requestUpdate();
  }

  if (optionPopup.handleInput(mappedInput, [this] { requestUpdate(); })) {
    return;
  }

  const bool back = mappedInput.wasPressed(MappedInputManager::Button::Back) || mappedInput.wasBackGesture();
  if (back) {
    handleBack();
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm) ||
      mappedInput.wasPressed(MappedInputManager::Button::Power)) {
    handleConfirm();
    return;
  }

  if (screen == Screen::EMPTY) return;

  // Swipe gesture handling
  const auto swipe = mappedInput.wasSwipe();
  if (swipe == MappedInputManager::SwipeDir::Up || swipe == MappedInputManager::SwipeDir::Down) {
    if (screen == Screen::FLASHCARD) {
      moveSelection(swipe == MappedInputManager::SwipeDir::Up ? 1 : -1);
      return;
    }
    const int rowHeight = renderer.getLineHeight(UI_10_FONT_ID) + 14;
    int step = listRectFor(renderer).height / rowHeight - 1;
    if (step < 1) step = 1;
    const int dir = (swipe == MappedInputManager::SwipeDir::Up) ? 1 : -1;
    const int target = std::max(0, std::min(static_cast<int>(words.size()) - 1, selectedIndex + dir * step));
    if (target != selectedIndex) {
      selectedIndex = target;
      requestUpdate();
    }
    return;
  }

  // Touch screen handling
  if (screen == Screen::FLASHCARD) {
    int touchX = 0, touchY = 0;
    if (mappedInput.wasScreenTapped(touchX, touchY)) {
      const int w = renderer.getScreenWidth();
      if (touchX < w / 3) {
        moveSelection(-1);  // Left side: Previous
      } else if (touchX > (w * 2) / 3) {
        moveSelection(1);   // Right side: Next
      } else {
        handleConfirm();    // Center: Lookup
      }
      return;
    }
  } else if (screen == Screen::LIST) {
    int touched = 0;
    const auto touch = pluginui::listTouch(mappedInput, renderer, listRectFor(renderer),
                                          static_cast<int>(words.size()), selectedIndex, touched);
    if (touch != MappedInputManager::RowTouch::None) {
      selectedIndex = touched;
      if (touch == MappedInputManager::RowTouch::Tap) {
        openWordOptions(selectedIndex);
      } else {
        requestUpdate();
      }
      return;
    }
  }

  buttonNavigator.onNext([this] {
    moveSelection(1);
    requestUpdate();
  });
  buttonNavigator.onPrevious([this] {
    moveSelection(-1);
    requestUpdate();
  });
}

void VocabBuilderActivity::renderList() {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int screenW = renderer.getScreenWidth();

  std::string headerSub = std::to_string(words.size()) + (words.size() == 1 ? " word" : " words");
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, screenW, metrics.headerHeight},
                 "Vocabulary Builder", headerSub.c_str());

  pluginui::drawList(renderer, listRectFor(renderer), static_cast<int>(words.size()), selectedIndex,
                     [this](int i) { return words[static_cast<size_t>(i)]; },
                     [this](int i) { return std::to_string(i + 1); },
                     nullptr);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "Options", tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void VocabBuilderActivity::renderFlashcard() {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int screenW = renderer.getScreenWidth();
  const int screenH = renderer.getScreenHeight();

  std::string headerSub = std::to_string(flashcardIndex + 1) + " of " + std::to_string(words.size());
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, screenW, metrics.headerHeight},
                 "Flashcard Review", headerSub.c_str());

  if (flashcardIndex >= 0 && flashcardIndex < static_cast<int>(words.size())) {
    const std::string& currentWord = words[static_cast<size_t>(flashcardIndex)];
    const int fontId = UI_14_FONT_ID;
    const int lineHeight = renderer.getLineHeight(fontId);
    renderer.drawCenteredText(fontId, screenH / 2 - lineHeight, currentWord.c_str(), EpdFontFamily::BOLD);

    const int hintFont = UI_10_FONT_ID;
    renderer.drawCenteredText(hintFont, screenH / 2 + lineHeight,
                              "Press Confirm or Tap Center to Look Up", EpdFontFamily::ITALIC);
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "Lookup", "◄ Prev", "Next ►");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void VocabBuilderActivity::renderEmpty() {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int screenW = renderer.getScreenWidth();
  const int screenH = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, screenW, metrics.headerHeight},
                 "Vocabulary Builder", "0 words");

  const int font = UI_10_FONT_ID;
  const int lineH = renderer.getLineHeight(font);
  int y = screenH / 2 - lineH * 3;

  renderer.drawCenteredText(font, y, "No vocabulary words saved yet.");
  y += lineH + 10;
  renderer.drawCenteredText(font, y, "Highlight a word while reading a book,");
  y += lineH + 4;
  renderer.drawCenteredText(font, y, "then tap 'Vocab' to save it here!");
  y += lineH + 16;
  renderer.drawCenteredText(font, y, "Saved as plain words in:", EpdFontFamily::ITALIC);
  y += lineH + 4;
  renderer.drawCenteredText(font, y, SETTINGS.vocabFilePath, EpdFontFamily::BOLD);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void VocabBuilderActivity::render(RenderLock&&) {
  renderer.clearScreen();

  if (screen == Screen::EMPTY) {
    renderEmpty();
  } else if (screen == Screen::FLASHCARD) {
    renderFlashcard();
  } else {
    renderList();
  }

  if (showNoticePopup && !noticeMessage.empty()) {
    GUI.drawPopup(renderer, noticeMessage.c_str());
  }

  if (optionPopup.processRender(renderer, mappedInput)) {
    return;
  }

  renderer.displayBuffer(HalDisplay::FAST_REFRESH);
}
