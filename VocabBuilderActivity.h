#pragma once

#include <GfxRenderer.h>
#include <I18n.h>
#include <MappedInputManager.h>
#include <activities/Activity.h>
#include <components/OptionPopup.h>
#include <util/ButtonNavigator.h>

#include <string>
#include <vector>

class VocabBuilderActivity final : public Activity {
 public:
  explicit VocabBuilderActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  enum class Screen : uint8_t { LIST, FLASHCARD, EMPTY };

  void loadWords();
  void handleConfirm();
  void handleBack();
  void moveSelection(int delta);
  void openWordOptions(int index);
  void lookupCurrentWord(const std::string& word);
  void deleteSelectedWord(int index);
  void confirmClearAll();
  void showNotice(const std::string& msg);

  void renderList();
  void renderFlashcard();
  void renderEmpty();

  Screen screen = Screen::LIST;
  std::vector<std::string> words;
  int selectedIndex = 0;
  int flashcardIndex = 0;
  OptionPopup optionPopup;
  ButtonNavigator buttonNavigator;
  std::string noticeMessage;
  unsigned long noticeTime = 0;
  bool showNoticePopup = false;
};
