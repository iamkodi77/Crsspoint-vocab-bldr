#pragma once

#include <cstdint>

enum class VocabDefaultAction : uint8_t {
  CLIP = 0,
  SAVE_TO_VOCAB = 1,
};

namespace VocabBuilderPlugin {
  const char* defaultActionName(VocabDefaultAction action);
  const char* toggleName(uint8_t enabled);
}
