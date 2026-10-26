# Validation — version 0.2.0

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
* Forwarding enabled with AR_CG1 and receiver: schema validation passes. Enabling
  forwarding for DEFAULT or without receiver_id is rejected.
* Forwarding allow-list: every captured valid state and OFF accepted; every captured
  vane command and each single-bit-corrupted state rejected.
* Eight-entry transmission history: suppression of earlier state and two subsequent
  swing packets, expiry, distinct packets, OFF and 32-bit clock wraparound pass.

The IDF component manager's process lookup required a build-environment-only
workaround to use its documented parent-PID fallback. This changed no packaged
source or generated device configuration.

Not verified here: physical IR transmission, air-conditioner acceptance, receiver
pulse timing under real conditions, echo/repeat handling, true swing position,
or vane-step direction. Follow the hardware checks in README.md. Packet tests are
not a substitute for these checks, and the runtime receive/toggle integration has
not been executed on hardware.
Forwarding scheduling/cancellation and optical echo suppression require hardware
verification as well; automated history tests do not simulate the ESPHome scheduler.
