#pragma once

#include "arcg1_codec.h"
#include <cstring>

namespace esphome::fujitsu_general::arcg1 {

// Only settings/turn-on and OFF. Never repeat relative vane commands.
inline bool forwardable(const uint8_t *frame, size_t length) {
  return valid_state(frame, length) || (valid_short(frame, length) && frame[5] == OFF);
}

// Small history rather than only the last packet: a climate operation may send
// a state frame followed by two swing frames before the receiver processes echo.
class TransmissionHistory {
 public:
  static constexpr uint32_t ECHO_WINDOW_MS = 1000;

  void remember(const uint8_t *frame, size_t length, uint32_t now) {
    if (length != STATE_LENGTH && length != SHORT_LENGTH)
      return;
    auto &entry = this->entries_[this->next_];
    std::memcpy(entry.frame.data(), frame, length);
    entry.length = length;
    entry.time = now;
    this->next_ = (this->next_ + 1) % this->entries_.size();
  }

  bool matches(const uint8_t *frame, size_t length, uint32_t now) const {
    if (length != STATE_LENGTH && length != SHORT_LENGTH)
      return false;
    for (const auto &entry : this->entries_) {
      if (entry.length == length && static_cast<uint32_t>(now - entry.time) < ECHO_WINDOW_MS &&
          std::memcmp(entry.frame.data(), frame, length) == 0)
        return true;
    }
    return false;
  }

 protected:
  struct Entry {
    StateFrame frame{};
    uint32_t time{0};
    uint8_t length{0};
  };
  std::array<Entry, 8> entries_{};
  size_t next_{0};
};
}  // namespace esphome::fujitsu_general::arcg1
