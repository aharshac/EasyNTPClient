# EasyNTPClient — Agent Guide

Arduino library that queries an NTP server over UDP and returns Unix time.
Single class, ~100 lines of implementation. Target platforms: ESP8266, ESP32, Arduino AVR (via WiFiEsp shield).

---

## Repository layout

```txt
src/
  EasyNTPClient.h       # Class declaration, member types, defaults
  EasyNTPClient.cpp     # NTP query logic, caching, Unix time conversion
examples/
  NodeMCU/              # ESP8266 + ESP8266WiFi + WiFiUdp
  ArduinoEspWifiShield/ # Arduino UNO + ESP-01 via SoftwareSerial + WiFiEsp
library.properties      # Arduino Library Manager metadata
library.json            # PlatformIO metadata
keywords.txt            # Arduino IDE syntax highlighting
```

No build system is runnable locally. Validation is done through CI (compile-only; no hardware tests).

---

## How to validate changes

The CI compiles examples against real Arduino toolchains. To check a change:

1. Confirm it compiles for both target platforms:
   - `esp8266:esp8266:nodemcuv2` — NodeMCU example
   - `arduino:avr:uno` — ArduinoEspWifiShield example

2. Verify `.esp8266.test.skip` / `.uno.test.skip` marker files in example
   directories are still correct after any platform-scoped change.

3. For any type or API change, check both examples still compile without warnings.

There are no unit tests. Functional correctness is verified on hardware only.

---

## Code conventions

- Arduino C++ style: no STL, no exceptions, no heap allocation after init.
- `unsigned long` for timestamps and `millis()` values.
- `int32_t` (or `long`) for signed offsets — never bare `int`, which is 16-bit on AVR.
- The NTP→Unix epoch offset is the constant `2208988800UL` (do not change).
- `static` local variables inside methods are used for one-time init; be aware they persist for the program lifetime.
- No serial debug output in the library itself; examples may use `Serial`.

---

## Platform constraints

| Platform | `int` width | `long` width | Notes |
|----------|-------------|--------------|-------|
| AVR (UNO, Mega) | 16-bit | 32-bit | `int` overflows for UTC offsets > ±9h (32767s) |
| ESP8266 | 32-bit | 32-bit | No overflow risk |
| ESP32 | 32-bit | 32-bit | No overflow risk |

Always use `int32_t` or `long` for the time offset parameter, not `int`.

---

## Key invariants to preserve

- `getUnixTime()` must never make a network call more than once per `mUpdateInterval` ms.
- `getServerTime()` returns `0` on any failure (no exception, no assert).
- The caller is responsible for establishing WiFi before calling `getUnixTime()`.
- UDP socket is opened lazily inside `getServerTime()` on the first call.
- `millis()` wraparound (~49 days) is handled correctly by unsigned subtraction; do not change the delta logic.

---

## What NOT to do

- Do not add heap allocation (`new`, `malloc`) or STL containers.
- Do not add serial logging to the library source.
- Do not break the 3-constructor API surface — it is the public interface.
- Do not change the `2208988800UL` constant.
- Do not make `getServerTime()` blocking for longer than `maxPoll * pollIntv` ms (currently 15 × 150 = 2250ms).

---

## Known issues (do not re-introduce workarounds for these)

- `mServerTime` was historically typed as `long` (signed); it must be `unsigned long`.
- The `pktLen != 48` check rejects valid NTP packets with extension fields; correct condition is `pktLen < 48`.
- The UDP socket opened via `static` init cannot recover without a device reset.
- `mUpdateInterval` has no public setter; adding `setUpdateInterval()`/`getUpdateInterval()` is a planned improvement.

---

## Metadata files

Both `library.properties` and `library.json` must stay in sync on:

- `version` — bump on every release (semver)
- `name`, `author`/`authors`, `description`

`library.json` `platforms` field should reflect all supported PlatformIO platforms (not just `espressif`).
