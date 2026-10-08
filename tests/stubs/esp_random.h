#pragma once
#include <stdint.h>
inline uint32_t esp_random() {
  static uint32_t state=21;state=state*1664525u+1013904223u;return state;
}
