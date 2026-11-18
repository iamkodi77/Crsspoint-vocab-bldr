"""
Patch application logic for integrating the Vocabulary Builder plugin into CrossPoint Reader.
"""

import os
import re

class PatchError(Exception):
    pass

def read_file(path):
    with open(path, "r", encoding="utf-8") as f:
        return f.read()

def write_file(path, content):
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)

def replace_once(content, anchor, replacement, file_label):
    if replacement in content:
        # Already applied
        return content
    count = content.count(anchor)
    if count == 0:
        raise PatchError(f"{file_label}: expected anchor not found:\n{anchor!r}")
    if count > 1:
        raise PatchError(f"{file_label}: anchor matched {count} times, expected exactly 1:\n{anchor!r}")
    return content.replace(anchor, replacement, 1)

def patch_crosspoint_reader(repo_dir):
    """
    Patches the reader source files to add highlight-to-vocab action and notifications.
    """
    # 1. ActivityResult.h: add Vocab action to ClippingResult::Action
    act_res_path = os.path.join(repo_dir, "src", "activities", "ActivityResult.h")
    if os.path.exists(act_res_path):
        c = read_file(act_res_path)
        if "ClippingResult::Action::Vocab" not in c and "Action : uint8_t { Clip, Lookup, Bookmark, Vocab }" not in c:
            anchor = "enum class Action : uint8_t { Clip, Lookup, Bookmark };"
            replacement = "enum class Action : uint8_t { Clip, Lookup, Bookmark, Vocab };"
            c = replace_once(c, anchor, replacement, "ActivityResult.h")
            write_file(act_res_path, c)
            print("  ActivityResult.h patched (ClippingResult::Action::Vocab added).")

    # 2. ClipSelectionActivity.h: update actionCount from 3 to 4
    clip_h_path = os.path.join(repo_dir, "src", "activities", "reader", "ClipSelectionActivity.h")
    if os.path.exists(clip_h_path):
        c = read_file(clip_h_path)
        if "4 - firstAction()" not in c:
            anchor = "int actionCount() const { return mode == Mode::DefinitionLookup ? 1 : 3 - firstAction(); }"
            replacement = "int actionCount() const { return mode == Mode::DefinitionLookup ? 1 : 4 - firstAction(); }"
            c = replace_once(c, anchor, replacement, "ClipSelectionActivity.h")
            write_file(clip_h_path, c)
            print("  ClipSelectionActivity.h patched (actionCount updated).")

    # 3. ClipSelectionActivity.cpp: add Vocab to SELECTION_ACTIONS, actionRect, handleButtons
    clip_cpp_path = os.path.join(repo_dir, "src", "activities", "reader", "ClipSelectionActivity.cpp")
    if os.path.exists(clip_cpp_path):
        c = read_file(clip_cpp_path)
        modified = False
        # 3a. SELECTION_ACTIONS
        if "ClippingResult::Action::Vocab" not in c:
            anchor = (
                "constexpr ClippingResult::Action SELECTION_ACTIONS[] = {ClippingResult::Action::Lookup, ClippingResult::Action::Clip,\n"
                "                                                        ClippingResult::Action::Bookmark};"
            )
            replacement = (
                "constexpr ClippingResult::Action SELECTION_ACTIONS[] = {ClippingResult::Action::Lookup, ClippingResult::Action::Vocab,\n"
                "                                                        ClippingResult::Action::Clip, ClippingResult::Action::Bookmark};"
            )
            c = replace_once(c, anchor, replacement, "ClipSelectionActivity.cpp (SELECTION_ACTIONS)")
            modified = True

        # 3b. actionRect label width
        if 'renderer.getTextWidth(font, "Vocab")' not in c:
            anchor = (
                "  const int labelWidth =\n"
                "      std::max({renderer.getTextWidth(font, tr(STR_LOOKUP)), renderer.getTextWidth(font, tr(STR_CLIP)),\n"
                "                renderer.getTextWidth(font, tr(STR_BOOKMARK_OPTION))});"
            )
            replacement = (
                "  const int labelWidth =\n"
                "      std::max({renderer.getTextWidth(font, tr(STR_LOOKUP)), renderer.getTextWidth(font, \"Vocab\"),\n"
                "                renderer.getTextWidth(font, tr(STR_CLIP)), renderer.getTextWidth(font, tr(STR_BOOKMARK_OPTION))});"
            )
            c = replace_once(c, anchor, replacement, "ClipSelectionActivity.cpp (actionRect)")
            modified = True

        # 3c. handleButtons confirm action
        if "SETTINGS.vocabDefaultAction" not in c:
            anchor = (
                "  if (buttons & Input::INPUT_CONFIRM) {\n"
                "    confirmSelection(isLookup() ? ClippingResult::Action::Lookup : ClippingResult::Action::Clip);\n"
                "    return true;\n"
                "  }"
            )
            replacement = (
                "  if (buttons & Input::INPUT_CONFIRM) {\n"
                "    ClippingResult::Action confirmAct = isLookup() ? ClippingResult::Action::Lookup : ClippingResult::Action::Clip;\n"
                "    if (!isLookup() && SETTINGS.vocabDefaultAction == 1) {\n"
                "      confirmAct = ClippingResult::Action::Vocab;\n"
                "    }\n"
                "    confirmSelection(confirmAct);\n"
                "    return true;\n"
                "  }"
            )
            c = replace_once(c, anchor, replacement, "ClipSelectionActivity.cpp (handleButtons)")
            modified = True

        if modified:
            write_file(clip_cpp_path, c)
            print("  ClipSelectionActivity.cpp patched (Vocab action added).")

    # 4. BaseTheme.cpp: add "Vocab" label
    theme_path = os.path.join(repo_dir, "src", "components", "themes", "BaseTheme.cpp")
    if os.path.exists(theme_path):
        c = read_file(theme_path)
        if '"Vocab"' not in c:
            anchor = '  const char* labels[] = {tr(STR_LOOKUP), tr(STR_CLIP), tr(STR_BOOKMARK_OPTION)};'
            replacement = '  const char* labels[] = {tr(STR_LOOKUP), "Vocab", tr(STR_CLIP), tr(STR_BOOKMARK_OPTION)};'
            c = replace_once(c, anchor, replacement, "BaseTheme.cpp")
            write_file(theme_path, c)
            print("  BaseTheme.cpp patched (Vocab button label added).")

    # 5. EpubReaderActivity.h: add customClippingMessage
    epub_h_path = os.path.join(repo_dir, "src", "activities", "reader", "EpubReaderActivity.h")
    if os.path.exists(epub_h_path):
        c = read_file(epub_h_path)
        if "customClippingMessage" not in c:
            anchor = (
                "  bool showClippingMessage = false;\n"
                "  StrId clippingMessage = StrId::STR_CLIPPING_SAVED;\n"
                "  unsigned long clippingMessageTime = 0UL;"
            )
            replacement = (
                "  bool showClippingMessage = false;\n"
                "  StrId clippingMessage = StrId::STR_CLIPPING_SAVED;\n"
                "  std::string customClippingMessage;\n"
                "  unsigned long clippingMessageTime = 0UL;"
            )
            c = replace_once(c, anchor, replacement, "EpubReaderActivity.h")
            write_file(epub_h_path, c)
            print("  EpubReaderActivity.h patched (customClippingMessage added).")

    # 6. EpubReaderActivity.cpp: handle Vocab action, include VocabBuilderStore, popup
    epub_cpp_path = os.path.join(repo_dir, "src", "activities", "reader", "EpubReaderActivity.cpp")
    if os.path.exists(epub_cpp_path):
        c = read_file(epub_cpp_path)
        modified = False
        # 6a. Include
        if "vocabulary/VocabBuilderStore.h" not in c:
            inc_anchor = '#include "clippings/ClippingText.h"'
            c = replace_once(c, inc_anchor, inc_anchor + '\n#include "vocabulary/VocabBuilderStore.h"', "EpubReaderActivity.cpp (include)")
            modified = True

        # 6b. Action handling
        if "ClippingResult::Action::Vocab" not in c:
            act_anchor = "    if (clipping.action == ClippingResult::Action::Bookmark) {"
            vocab_handler = (
                "    if (clipping.action == ClippingResult::Action::Vocab) {\n"
                "      std::string clean;\n"
                "      if (!VocabBuilderStore::cleanWord(clipping.text, clean, SETTINGS.vocabLowercase != 0)) {\n"
                "        customClippingMessage = \"Invalid word\";\n"
                "      } else {\n"
                "        const auto saveRes = VocabBuilderStore::saveWord(clean, SETTINGS.vocabLowercase != 0, SETTINGS.vocabFilePath);\n"
                "        if (saveRes == VocabBuilderStore::SaveResult::Added) {\n"
                "          customClippingMessage = \"Saved: \" + clean;\n"
                "        } else if (saveRes == VocabBuilderStore::SaveResult::AlreadyExists) {\n"
                "          customClippingMessage = \"Already saved: \" + clean;\n"
                "        } else {\n"
                "          customClippingMessage = \"Failed to save word\";\n"
                "        }\n"
                "      }\n"
                "      showClippingMessage = true;\n"
                "      clippingMessageTime = millis();\n"
                "      requestUpdate();\n"
                "      return;\n"
                "    }\n"
                "    if (clipping.action == ClippingResult::Action::Bookmark) {"
            )
            c = replace_once(c, act_anchor, vocab_handler, "EpubReaderActivity.cpp (action handler)")
            modified = True

        # 6c. Auto-save on lookup
        if "SETTINGS.vocabAutoSaveLookup" not in c:
            lookup_anchor = "    if (clipping.action == ClippingResult::Action::Lookup) {"
            lookup_replacement = (
                "    if (clipping.action == ClippingResult::Action::Lookup) {\n"
                "      if (SETTINGS.vocabAutoSaveLookup) {\n"
                "        std::string cleanLookup;\n"
                "        if (VocabBuilderStore::cleanWord(clipping.text, cleanLookup, SETTINGS.vocabLowercase != 0)) {\n"
                "          VocabBuilderStore::saveWord(cleanLookup, SETTINGS.vocabLowercase != 0, SETTINGS.vocabFilePath);\n"
                "        }\n"
                "      }"
            )
            c = replace_once(c, lookup_anchor, lookup_replacement, "EpubReaderActivity.cpp (auto-save lookup)")
            modified = True

        # 6d. Timeout clear custom message
        if "customClippingMessage.clear();" not in c:
            timeout_anchor = (
                "  if (showClippingMessage && (millis() - clippingMessageTime) >= ReaderUtils::BOOKMARK_MESSAGE_DURATION_MS) {\n"
                "    showClippingMessage = false;\n"
                "    requestUpdate();\n"
                "  }"
            )
            timeout_replacement = (
                "  if (showClippingMessage && (millis() - clippingMessageTime) >= ReaderUtils::BOOKMARK_MESSAGE_DURATION_MS) {\n"
                "    showClippingMessage = false;\n"
                "    customClippingMessage.clear();\n"
                "    requestUpdate();\n"
                "  }"
            )
            c = replace_once(c, timeout_anchor, timeout_replacement, "EpubReaderActivity.cpp (timeout)")
            modified = True

        # 6e. Render popup with custom message
        if "!customClippingMessage.empty()" not in c:
            popup_anchor = (
                "  if (showClippingMessage) {\n"
                "    GUI.drawPopup(renderer, I18N.get(clippingMessage));\n"
                "  }"
            )
            popup_replacement = (
                "  if (showClippingMessage) {\n"
                "    if (!customClippingMessage.empty()) {\n"
                "      GUI.drawPopup(renderer, customClippingMessage.c_str());\n"
                "    } else {\n"
                "      GUI.drawPopup(renderer, I18N.get(clippingMessage));\n"
                "    }\n"
                "  }"
            )
            c = replace_once(c, popup_anchor, popup_replacement, "EpubReaderActivity.cpp (render popup)")
            modified = True

        if modified:
            write_file(epub_cpp_path, c)
            print("  EpubReaderActivity.cpp patched (Vocab handler and popup).")
