#include "../components/fujitsu_general/arcg1_codec.h"
#include <cassert>
#include <cstdio>
#include <vector>

using namespace esphome::fujitsu_general::arcg1;

static uint8_t reverse(uint8_t x) {
  uint8_t out = 0;
  for (int i = 0; i < 8; ++i) { out = (out << 1) | (x & 1); x >>= 1; }
  return out;
}

struct Capture {
  const char *label;
  std::vector<uint8_t> aeha;
  uint8_t temperature, mode, fan;
  bool turn_on;
};

int main() {
  // Independent, unchanged AEHA payloads supplied by the hardware owner.
  const Capture captures[] = {
    {"24 C auto turn on", {0,8,8,0x3F,0x10,0x0C,0x81,0x80,0,0,0,0,0x72},24,1,0,true},
    {"25 C auto", {0,8,8,0x3F,0x10,0x0C,9,0x80,0,0,0,0,0xFC},25,1,0,false},
    {"25 C high", {0,8,8,0x3F,0x10,0x0C,9,0x80,0x80,0,0,0,0x7C},25,1,1,false},
    {"25 C medium", {0,8,8,0x3F,0x10,0x0C,9,0x80,0x40,0,0,0,0xBC},25,1,2,false},
    {"25 C low", {0,8,8,0x3F,0x10,0x0C,9,0x80,0xC0,0,0,0,0x3C},25,1,3,false},
    {"25 C dry", {0,8,8,0x3F,0x10,0x0C,9,0x40,0,0,0,0,0x7C},25,2,0,false},
    {"25 C fan", {0,8,8,0x3F,0x10,0x0C,9,0xC0,0,0,0,0,0xBC},25,3,0,false},
    {"25 C heat", {0,8,8,0x3F,0x10,0x0C,9,0x20,0,0,0,0,0x3C},25,4,0,false},
    {"25 C auto mode", {0,8,8,0x3F,0x10,0x0C,9,0,0,0,0,0,2},25,0,0,false},
  };
  for (const auto &capture : captures) {
    StateFrame expected{0x14,0x63};
    for (size_t i = 0; i < capture.aeha.size(); ++i) expected[i+2] = reverse(capture.aeha[i]);
    assert(valid_state(expected.data(), expected.size()));
    assert(encode_state(capture.temperature,capture.mode,capture.fan,capture.turn_on) == expected);
    // Every single-bit corruption must fail validation, including header/body/checksum.
    for (size_t i = 0; i < expected.size(); ++i) {
      for (uint8_t bit = 1; bit; bit <<= 1) {
        auto corrupted = expected; corrupted[i] ^= bit;
        assert(!valid_state(corrupted.data(), corrupted.size()));
      }
    }
    for (size_t length = 0; length < STATE_LENGTH; ++length)
      assert(!valid_state(expected.data(), length));
    std::printf("PASS %s\n", capture.label);
  }
  const uint8_t aeha_commands[] = {0x40,0x5E,0xB6,0x36,0x9E};
  const uint8_t commands[] = {OFF,TOGGLE_HORIZONTAL,TOGGLE_VERTICAL,STEP_VERTICAL,STEP_HORIZONTAL};
  for (size_t i = 0; i < 5; ++i) {
    const ShortFrame expected{0x14,0x63,0,0x10,0x10,reverse(aeha_commands[i])};
    assert(expected == encode_short(commands[i]));
    assert(valid_short(expected.data(), expected.size()));
    assert(!valid_short(expected.data(), 5));
    assert(!valid_short(expected.data(), 7));
    auto corrupted = expected; corrupted[0] ^= 1;
    assert(!valid_short(corrupted.data(), corrupted.size()));
  }
  auto unsupported = encode_short(0x09);
  assert(!valid_short(unsupported.data(), unsupported.size()));
  assert(encode_state(0,1,0,false)[8] == 0);
  assert(encode_state(255,1,0,false)[8] == 0xE0);
  std::puts("PASS all 5 captured short commands, corruption/truncation rejection and temperature clamping");
}
