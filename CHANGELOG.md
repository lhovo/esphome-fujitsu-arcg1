# 0.3.0

* Reset received short-command repeat tracking after a local climate command or
  valid received state frame, so an intervening state change does not suppress a
  subsequent OFF command within 200 ms.
* Add regression tests for repeat-tracking resets, held-button repeats, expiry,
  and clock wraparound; verified in Docker with AddressSanitizer and
  UndefinedBehaviorSanitizer.

# 0.2.0

* Add opt-in `forward_remote_commands: true` for AR-CG1 with a configured receiver.
* Forward validated received state and OFF frames after publishing climate state.
* Exclude all swing-toggle and vane-step commands from forwarding.
* Preserve exact received state fields, including the explicit turn-on bit.
* Defer sending 40 ms to leave the receiver callback; latest pending state/off wins.
* Cancel older pending forwards when Home Assistant issues a climate command.
* Replace last-frame-only echo matching with an eight-packet, one-second history.
* Add forwarding policy/history tests and configuration validation guards.

# 0.1.0

* Initial AR-CG1 state generation, reception and relative vane controls.
