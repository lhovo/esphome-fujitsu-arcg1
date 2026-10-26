# Fujitsu AR-CG1 climate extension for ESPHome

External override of ESPHome's `fujitsu_general` component, based on **ESPHome
2026.10.0**. Adds an opt-in `model: AR_CG1` using the supplied AR-CG1 captures.
No captured-command replay: temperature, mode, fan, turn-on and checksums are
generated from climate settings. Handheld remote packets update the climate entity.

## Install

1. Unzip the package. Copy `components/fujitsu_general/` into a `components/`
   directory alongside your existing ESPHome device YAML. With the usual Device
   Builder layout this is `/config/esphome/components/fujitsu_general/`; use your
   actual YAML directory if different.
2. Keep your existing working Wi-Fi, API, OTA, logger, ESP32 and device-name
   settings. Do not replace these with settings from the earlier Tesla project.
3. Add the following at the top level:

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [fujitsu_general]
```

4. Replace the existing climate block (do not create a duplicate) with:

```yaml
climate:
  - platform: fujitsu_general
    id: lounge_ac
    name: "Lounge Aircon"
    model: AR_CG1
    transmitter_id: ir_tx
    receiver_id: ir_rx
```

5. Retain the known-working board connections: power enable GPIO21 ALWAYS_ON,
   transmitter GPIO4 at 50% duty, receiver GPIO14 inverted. Set receiver
   `idle: 10ms`. The complete `lounge-aircon.yaml` example includes these plus
   optional step buttons and an assumption-correction button.
6. Validate and build using ESPHome 2026.10.0. Install when you can observe the
   unit and recover via serial if necessary. Check the logs for
   `Protocol model: AR-CG1 (15/6 bytes)`.

The example is complete, but keep your existing working API encryption key and
OTA method when merging. Its encrypted OTA section is for a device already able
to receive encrypted updates or for a serial install. Do not use it to migrate
older password-only OTA firmware without the documented ESPHome migration step.
Example secret names are `wifi_ssid`, `wifi_password`, `api_encryption_key`.
No real secrets or hardware-targeted firmware binaries are included.

## Supported controls

* Off, Cool, Heat, Dry, Fan only, Auto heat/cool.
* Integer target temperatures, 16–30 C (protocol range; verify your unit's limits).
* Auto, High, Medium, Low fan; Quiet is not advertised for AR-CG1.
* Off / Vertical / Horizontal / Both swing, implemented by per-axis toggles.
* Optional vane-step buttons through `step_vertical()` / `step_horizontal()`.
* Receive state frames, off, swing toggle and vane-step commands.
* Original 16-byte/7-byte behaviour remains the default when `model` is omitted
  or set to `DEFAULT`.

Clock, timers and sleep are out of scope. No room temperature sensor is included;
the remote's setpoint is not the measured room temperature. You can use the
standard `sensor:` climate option with a separate temperature sensor.

## Swing correction

At boot both axes are **assumed stopped**. The device cannot query the air
conditioner or infer absolute swing from the captured state packet. Stop the
vanes physically, then use the example's **Assume Swing Stopped** button to align
the assumption without transmitting. Do this again after missed remote commands
or a restart if needed. The next swing selection sends the necessary toggles.

For another known physical state, the same method accepts
`esphome::climate::CLIMATE_SWING_VERTICAL`, `CLIMATE_SWING_HORIZONTAL`, or
`CLIMATE_SWING_BOTH` as well as `CLIMATE_SWING_OFF`.

The two vane-step directions use the reference protocol names; the captures'
SET-button labels were reversed. Verify which vanes move on hardware.
Step commands are actions, not absolute angle settings.

## Hardware verification checklist

1. Ensure reliable USB-C power (this board previously showed Wi-Fi brownouts).
2. Place the transmitter in sight of the air conditioner. Start with mode OFF.
3. Select Cool, 24 C, Auto fan: confirm the unit starts and accepts the settings.
4. Change to 25 C, then High/Medium/Low/Auto fan; confirm each on the unit.
5. Test Heat/Dry/Fan/Auto where the unit supports them, then Off.
6. Use the original remote: confirm setpoint/mode/fan/off update Home Assistant.
7. Establish the swing assumption, test each axis, both axes and Off.
8. Verify step directions and watch for doubled swing toggles due to local echo.
9. Reboot: normal climate settings may restore, but swing resets to assumed OFF.

These checks need real hardware. Passing automated tests does not establish that
the air conditioner accepts transmitted commands.

## Tests

`bash tests/run_tests.sh` uses g++ and AddressSanitizer/UBSan. It compares generated
frames against nine independent captured state payloads and five short commands,
checks all single-bit corruptions of the state fixtures, rejects truncated frames,
and verifies temperature clamping. See `VALIDATION.md` for checks run for this release.

## Roll back / upgrade

Remove the `external_components` entry, `model: AR_CG1`, and optional lambdas that
call custom methods to return to the built-in component. That built-in version
does not support this captured AR-CG1 format. Back up your working firmware before
testing. Revalidate this override before upgrading ESPHome; it includes upstream
runtime code and is not guaranteed compatible with later releases.

## License

See `LICENSE` for upstream ESPHome terms (C++ GPLv3, Python MIT). Added code uses
the corresponding upstream license. Source provenance and protocol notes are in
`PROTOCOL.md`.
