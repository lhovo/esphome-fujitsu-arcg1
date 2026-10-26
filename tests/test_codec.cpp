#include "../components/fujitsu_general/arcg1_codec.h"
#include "../components/fujitsu_general/arcg1_forward.h"
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
  ShortCommandHistory short_history;
  assert(!short_history.is_repeat(OFF, 0));
  assert(short_history.is_repeat(OFF, 100));
  assert(short_history.is_repeat(OFF, 250));  // Held repeats extend the window.
  assert(!short_history.is_repeat(OFF, 450));  // Exactly 200 ms is a new press.
  assert(!short_history.is_repeat(TOGGLE_VERTICAL, 460));
  assert(!short_history.is_repeat(OFF, 470));
  // OFF -> intervening local/received state -> OFF must accept the second OFF.
  short_history.reset();
  assert(!short_history.is_repeat(OFF, 480));
  assert(short_history.is_repeat(OFF, 490));
  short_history.reset();
  short_history.reset();
  assert(!short_history.is_repeat(OFF, 500));
  short_history.reset();
  assert(!short_history.is_repeat(OFF, 0xFFFFFFF0U));
  assert(short_history.is_repeat(OFF, 20));
  assert(!short_history.is_repeat(OFF, 220));
  std::puts("PASS short-command repeats, intervening-state reset, expiry and clock wrap");

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
    assert(forwardable(expected.data(), expected.size()));
    assert(encode_state(capture.temperature,capture.mode,capture.fan,capture.turn_on) == expected);
    // Every single-bit corruption must fail validation, including header/body/checksum.
    for (size_t i = 0; i < expected.size(); ++i) {
      for (uint8_t bit = 1; bit; bit <<= 1) {
        auto corrupted = expected; corrupted[i] ^= bit;
        assert(!valid_state(corrupted.data(), corrupted.size()));
        assert(!forwardable(corrupted.data(), corrupted.size()));
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
    assert(forwardable(expected.data(), expected.size()) == (commands[i] == OFF));
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

  TransmissionHistory history;
  const auto state_a = encode_state(24,1,0,true);
  const auto state_b = encode_state(25,1,0,false);
  const auto vertical = encode_short(TOGGLE_VERTICAL);
  const auto horizontal = encode_short(TOGGLE_HORIZONTAL);
  assert(!history.matches(state_a.data(), state_a.size(), 0));
  history.remember(state_a.data(), state_a.size(), 100);
  history.remember(vertical.data(), vertical.size(), 250);
  history.remember(horizontal.data(), horizontal.size(), 300);
  // A later swing transmission must not cause the earlier state echo to repeat.
  assert(history.matches(state_a.data(), state_a.size(), 400));
  assert(history.matches(vertical.data(), vertical.size(), 400));
  assert(history.matches(horizontal.data(), horizontal.size(), 400));
  assert(!history.matches(state_b.data(), state_b.size(), 400));
  assert(history.matches(state_a.data(), state_a.size(), 1099));
  assert(!history.matches(state_a.data(), state_a.size(), 1100));
  const auto off = encode_short(OFF);
  history.remember(off.data(), off.size(), 1200);
  assert(history.matches(off.data(), off.size(), 1300));
  assert(!history.matches(off.data(), off.size(), 2200));
  // millis() wraps: unsigned elapsed time must still suppress recent echo.
  history.remember(state_b.data(), state_b.size(), 0xFFFFFFF0U);
  assert(history.matches(state_b.data(), state_b.size(), 20));
  assert(!history.matches(state_b.data(), state_b.size(), 1100));
  std::puts("PASS forwarding allow-list, recent multi-packet echo history, expiry and clock wrap");
}
