#include "VocabBuilderPlugin.h"

namespace VocabBuilderPlugin {

const char* defaultActionName(VocabDefaultAction action) {
  switch (action) {
    case VocabDefaultAction::SAVE_TO_VOCAB:
      return "Save to Vocab";
    case VocabDefaultAction::CLIP:
    default:
      return "Clip";
  }
}

const char* toggleName(uint8_t enabled) {
  return enabled ? "Enabled" : "Disabled";
}

}  // namespace VocabBuilderPlugin
