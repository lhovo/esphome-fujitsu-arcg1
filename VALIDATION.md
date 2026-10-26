# Validation — version 0.1.0

Performed with ESPHome **2026.10.0**, ESP-IDF **5.5.6**, target **ESP32 / esp32dev**.

* Complete example YAML schema validation: PASS, including external override,
  model selection, transmitter/receiver references, and optional button lambdas.
* Full ESP32 firmware compilation and linking: PASS. Factory and OTA images were
  produced for validation only; they contain dummy credentials and are deliberately
  not distributed. Build your own using your existing secrets.
* Packet tests with g++ C++17, `-Wall -Wextra -Werror -pedantic`, AddressSanitizer
  and UndefinedBehaviorSanitizer: PASS. Leak detection was disabled because this
  execution environment does not expose process information through `/proc`.
* Nine labelled captured state frames: exact generated-byte matches and valid
  checksums, including explicit turn-on and stay-on differences.
* Five captured short commands: exact byte matches (off, two toggles, two steps).
* Every single-bit corruption of each captured state fixture: rejected.
* Truncated state frames, wrong short lengths/header and unsupported short command:
  rejected. Temperature clamping checked at protocol endpoints.
* The default upstream implementation remains in the same compilation unit;
  AR-CG1 uses a separate opt-in path. Default-model schema validation also passes.

The IDF component manager's process lookup required a build-environment-only
workaround to use its documented parent-PID fallback. This changed no packaged
source or generated device configuration.

Not verified here: physical IR transmission, air-conditioner acceptance, receiver
pulse timing under real conditions, echo/repeat handling, true swing position,
or vane-step direction. Follow the hardware checks in README.md. Packet tests are
not a substitute for these checks, and the runtime receive/toggle integration has
not been executed on hardware.
