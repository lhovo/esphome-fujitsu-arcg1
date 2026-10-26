#pragma once

// AR-CG1: decoded bytes are LSB-first on the wire (AEHA logs reverse each byte).
// Implemented from labelled AR-CG1 captures; see PROTOCOL.md.
#include <array>
#include <cstddef>
#include <cstdint>

namespace esphome::fujitsu_general::arcg1 {
constexpr size_t STATE_LENGTH = 15;
constexpr size_t SHORT_LENGTH = 6;
constexpr uint8_t STATE = 0xFC;
constexpr uint8_t OFF = 0x02;
constexpr uint8_t STEP_VERTICAL = 0x6C;
constexpr uint8_t TOGGLE_VERTICAL = 0x6D;
constexpr uint8_t STEP_HORIZONTAL = 0x79;
constexpr uint8_t TOGGLE_HORIZONTAL = 0x7A;
using StateFrame = std::array<uint8_t, STATE_LENGTH>;
using ShortFrame = std::array<uint8_t, SHORT_LENGTH>;

inline bool valid_header(const uint8_t *data, size_t length) {
  return length >= SHORT_LENGTH && data[0] == 0x14 && data[1] == 0x63 && data[2] == 0x00 &&
         data[3] == 0x10 && data[4] == 0x10;
}

inline uint8_t checksum(const uint8_t *data) {
  uint8_t sum = 0;
  for (size_t i = 0; i < STATE_LENGTH - 1; ++i)
    sum += data[i];
  return static_cast<uint8_t>(0x9B - sum);
}

inline bool valid_state(const uint8_t *data, size_t length) {
  return length == STATE_LENGTH && valid_header(data, length) && data[5] == STATE && data[6] == 8 &&
         data[7] == 0x30 && checksum(data) == data[14] && (data[8] >> 4) <= 14 &&
         (data[9] & 7) <= 4 && (data[10] & 7) <= 3;
}

inline bool is_short_command(uint8_t command) {
  return command == OFF || command == STEP_VERTICAL || command == TOGGLE_VERTICAL ||
         command == STEP_HORIZONTAL || command == TOGGLE_HORIZONTAL;
}

inline bool valid_short(const uint8_t *data, size_t length) {
  return length == SHORT_LENGTH && valid_header(data, length) && is_short_command(data[5]);
}

inline ShortFrame encode_short(uint8_t command) { return {0x14, 0x63, 0x00, 0x10, 0x10, command}; }

inline StateFrame encode_state(uint8_t temperature, uint8_t mode, uint8_t fan, bool turn_on) {
  // Caller supplies validated mode (0..4) and fan (0..3).
  if (temperature < 16)
    temperature = 16;
  if (temperature > 30)
    temperature = 30;
  StateFrame data{0x14, 0x63, 0x00, 0x10, 0x10, STATE, 8, 0x30};
  data[8] = static_cast<uint8_t>(((temperature - 16) << 4) | (turn_on ? 1 : 0));
  data[9] = mode;
  data[10] = fan;
  data[14] = checksum(data.data());
  return data;
}
}  // namespace esphome::fujitsu_general::arcg1
