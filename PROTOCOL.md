# AR-CG1 captured protocol

All byte indices below are zero-based. Bytes are sent least-significant bit first.
ESPHome's AEHA dumper prints their bit-reversed equivalents, after address 0x28C6.
The protocol bytes start with 14 63, corresponding to that dumped address.

## State frame: 15 bytes

`14 63 00 10 10 FC 08 30 TT MM FF 00 00 00 CC`

* `TT`: high nibble = integer Celsius minus 16. Low bit = explicit turn-on request.
  A state frame with that bit clear is still an ON/settings command, not OFF.
* `MM & 7`: 0 auto heat/cool, 1 cool, 2 dry, 3 fan, 4 heat.
* `FF & 7`: 0 auto, 1 high, 2 medium, 3 low.
* `CC = (0x9B - sum(bytes 0..13)) & 0xFF`.
* Timers/sleep are not implemented. Transmitted timer bytes are zero.
* No absolute swing value has been established in the captured state frames.
  Receiving a state frame leaves assumed swing unchanged.

## Short frames: 6 bytes, no appended checksum

`14 63 00 10 10 CMD`

| CMD | Action | Captured AEHA final byte |
| --- | --- | --- |
| 02 | Off | 40 |
| 6C | Step vertical (reference direction) | 36 |
| 6D | Toggle vertical swing | B6 |
| 79 | Step horizontal (reference direction) | 9E |
| 7A | Toggle horizontal swing | 5E |

The owner's labels for the two SET buttons were opposite to reference directions.
Verify the physical vane directions before relying on the step button labels.
Each command was captured identically three times. Absolute vane angle is unknown.

## Pulse encoding

Existing ESPHome Fujitsu timings: 38 kHz, header mark/space 3300/1600 us,
bit mark 420 us, one space 1200 us, zero space 420 us, footer mark 420 us.
The extension uses a 20 ms inter-frame gap for AR-CG1 to separate consecutive
state/toggle frames at the configured 10 ms receiver idle threshold.

## Swing assumptions

The integration starts by assuming swing OFF on both axes, without transmitting.
It sends toggle commands only for axes whose requested state changes relative to
its tracked assumption. Valid received toggle commands update that assumption.
Vane-step commands do not change assumed swing, because their effect on ongoing
swing has not been established on this air conditioner.

An identical received short command within 200 ms is treated as a held-button
repeat. Matching locally transmitted packets/toggles within 500 ms are treated as
IR echo. A genuine matching handheld press in that interval may be ignored.
Short frames have no checksum; validation can only check header, command and length.

This is one-way IR: commands can be missed and assumed state can diverge.
The correction method changes only the tracked assumption and sends no IR.

## Sources

* Hardware-owner AR-CG1 captures, 25–26 October 2026 (test fixtures retain payloads).
* ESPHome 2026.10.0 fujitsu_general runtime and configuration, retained under the
  upstream licenses: https://github.com/esphome/esphome/tree/2026.10.0/esphome/components/fujitsu_general
* Fujitsu variant/checksum/swing constants cross-checked against IRremoteESP8266:
  https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/ir_Fujitsu.cpp
  and https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/ir_Fujitsu.h

AR-CG1 is the tested capture source. This release does not claim complete support
for every ARDB1/ARJW2 remote or its optional features.
