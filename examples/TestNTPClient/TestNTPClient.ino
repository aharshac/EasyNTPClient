/*
  EasyNTPClient test suite — ESP8266, ESP32, Uno+ESP-01
  Open Serial Monitor at 115200 baud after flashing.
  Fill in WIFI_SSID and WIFI_PASSWORD before flashing.
*/

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include "esp_task_wdt.h"
#else
  #include <SoftwareSerial.h>
  #include <WiFiEsp.h>
  #include <WiFiEspUdp.h>
#endif
#include <EasyNTPClient.h>

#if defined(ESP8266) || defined(ESP32)
  using NTPUdp = WiFiUDP;
#else
  static SoftwareSerial esp_serial(3, 2);  // D3→ESP-01 TX, D2→ESP-01 RX
  using NTPUdp = WiFiEspUDP;
#endif

const char* WIFI_SSID     = "<SSID>";
const char* WIFI_PASSWORD = "<PASSWORD>";

// Minimum plausible Unix timestamp (2024-01-01 00:00:00 UTC).
static const unsigned long MIN_UNIX_2024 = 1704067200UL;
// Maximum plausible Unix timestamp (2030-01-01 00:00:00 UTC).
static const unsigned long MAX_UNIX_2030 = 1893456000UL;

// ── test harness ─────────────────────────────────────────────────────────────

int g_passed = 0;
int g_failed = 0;

void check(bool condition, const __FlashStringHelper* label) {
  if (condition) { Serial.print(F("[PASS] ")); g_passed++; }
  else           { Serial.print(F("[FAIL] ")); g_failed++; }
  Serial.println(label);
}

// ── WiFi ─────────────────────────────────────────────────────────────────────

void wifi_connect() {
  Serial.print(F("Connecting to WiFi"));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print(F(".")); }
  Serial.println(F(" connected."));
}

// ── NTP constants ────────────────────────────────────────────────────────────

void test_constants() {
  Serial.println(F("\n-- NTP constants --"));
  check(NTP_PACKET_SIZE == 48,           F("NTP_PACKET_SIZE == 48"));
  check(NTP_TX_TIMESTAMP_OFFSET == 40,   F("NTP_TX_TIMESTAMP_OFFSET == 40"));
  check(NTP_SERVER_PORT == 123,          F("NTP_SERVER_PORT == 123"));
  check(NTP_REQUEST_PORT == 1123,        F("NTP_REQUEST_PORT == 1123"));
  check(NTP_HEADER_LI   == 0b11000000,  F("NTP_HEADER_LI == 0xC0"));
  check(NTP_HEADER_VN   == 0b00100000,  F("NTP_HEADER_VN == 0x20"));
  check(NTP_HEADER_MODE == 0b00000011,  F("NTP_HEADER_MODE == 0x03"));
  // Combined byte 0 must equal the first byte of the old magic constant 0xEC0600E3.
  check((NTP_HEADER_LI | NTP_HEADER_VN | NTP_HEADER_MODE) == 0xE3,
        F("header byte 0 matches original magic constant 0xE3"));
}

// ── basic sync ───────────────────────────────────────────────────────────────

void test_basic_sync() {
  Serial.println(F("\n-- basic sync --"));
  NTPUdp udp;
  EasyNTPClient client(udp, "pool.ntp.org");

  unsigned long t = client.getUnixTime();
  check(t > MIN_UNIX_2024, F("time is after 2024-01-01"));
  check(t < MAX_UNIX_2030, F("time is before 2030-01-01"));
}

// ── socket reuse ─────────────────────────────────────────────────────────────

void test_client_reuse() {
  Serial.println(F("\n-- socket reuse across client instances --"));

  // Both clients share the same UDP object. Without Fix B+C the second
  // begin() is skipped (static flag) and the second sync fails.
  NTPUdp udp;
  unsigned long t1 = 0, t2 = 0;

  {
    EasyNTPClient c1(udp, "pool.ntp.org");
    t1 = c1.getUnixTime(); // opens socket, syncs, closes socket (Fix B)
  }
  delay(500);
  {
    EasyNTPClient c2(udp, "pool.ntp.org");
    t2 = c2.getUnixTime(); // Fix C re-runs begin(); Fix B already closed it cleanly
  }

  check(t1 > MIN_UNIX_2024, F("first client syncs successfully"));
  check(t2 > MIN_UNIX_2024, F("second client syncs on same UDP object"));
  check(t2 >= t1 && (t2 - t1) < 5, F("timestamps consistent between clients"));
}

// ── offset decoupling ────────────────────────────────────────────────────────

void test_offset_immediate() {
  Serial.println(F("\n-- offset change takes effect without resync --"));
  NTPUdp udp;
  EasyNTPClient client(udp, "pool.ntp.org");

  // Sync once with offset = 0; capture base time.
  client.setTimeOffset(0);
  unsigned long base = client.getUnixTime();
  check(base > MIN_UNIX_2024, F("initial sync with offset=0 succeeds"));

  // Change offset without waiting for a resync (mUpdateInterval not elapsed).
  client.setTimeOffset(3600);
  unsigned long adjusted = client.getUnixTime();
  long delta = (long)adjusted - (long)base;
  check(delta >= 3598 && delta <= 3602, F("+3600 s offset reflected on next call"));

  // Reverse to zero.
  client.setTimeOffset(0);
  unsigned long restored = client.getUnixTime();
  delta = (long)restored - (long)base;
  check(delta >= 0 && delta <= 2, F("offset=0 reflected immediately after reversal"));
}

// ── wasUpdated flag ──────────────────────────────────────────────────────────

void test_was_updated() {
  Serial.println(F("\n-- wasUpdated() flag --"));
  NTPUdp udp;
  EasyNTPClient client(udp, "pool.ntp.org");

  check(!client.wasUpdated(), F("wasUpdated() is false before first sync"));

  client.getUnixTime();
  check(client.wasUpdated(), F("wasUpdated() is true after successful sync"));
}

// ── NTP server get/set ───────────────────────────────────────────────────────

void test_set_ntp_server() {
  Serial.println(F("\n-- setNTPServer() / getNTPServer() --"));
  NTPUdp udp;
  EasyNTPClient client(udp, "pool.ntp.org");

  check(strcmp(client.getNTPServer(), "pool.ntp.org") == 0,
        F("getNTPServer() returns initial pool"));

  client.setNTPServer("time.cloudflare.com");
  check(strcmp(client.getNTPServer(), "time.cloudflare.com") == 0,
        F("getNTPServer() reflects setNTPServer()"));

  unsigned long t = client.getUnixTime();
  check(t > MIN_UNIX_2024, F("syncs from server set via setNTPServer()"));
}

// ── update interval get/set ───────────────────────────────────────────────────

void test_set_update_interval() {
  Serial.println(F("\n-- setUpdateInterval() / getUpdateInterval() / 4-arg constructor --"));
  NTPUdp udp;

  EasyNTPClient client(udp, "pool.ntp.org");
  client.setUpdateInterval(30);
  check(client.getUpdateInterval() == 30,
        F("getUpdateInterval() reflects setUpdateInterval(30 s)"));

  EasyNTPClient client2(udp, "pool.ntp.org", 0, 120);
  check(client2.getUpdateInterval() == 120,
        F("4-arg constructor sets update interval to 120 s"));

  unsigned long t = client2.getUnixTime();
  check(t > MIN_UNIX_2024, F("syncs with interval set via 4-arg constructor"));
}

// ── stale time preservation ──────────────────────────────────────────────────

void test_stale_time() {
#if defined(ESP8266) || defined(ESP32)
  Serial.println(F("\n-- stale time preserved when resync fails (wait ~70 s) --"));
  NTPUdp udp;
  EasyNTPClient client(udp, "pool.ntp.org");

  // Populate mServerTime with a good sync.
  unsigned long good = client.getUnixTime();
  check(good > MIN_UNIX_2024, F("initial sync succeeds before WiFi drop"));

  // Drop WiFi and wait for mUpdateInterval (60 s default) to expire so the
  // next getUnixTime() call attempts a resync, fails, and should return the
  // preserved mServerTime + millis() drift.
  WiFi.disconnect();
  Serial.println(F("   WiFi disconnected. Waiting 65 s..."));
#if defined(ESP32)
  // vTaskDelay() does not reset the TWDT; feed it once per second.
  for (int i = 0; i < 65; i++) { delay(1000); esp_task_wdt_reset(); }
#else
  delay(65000);
#endif

  unsigned long stale = client.getUnixTime();
  long drift = (long)stale - (long)good;
  // Allow 63-72 s: 65 s delay plus up to 7 s for sync timeout polling.
  check(drift >= 63 && drift <= 72,
        F("stale time preserved and advancing during no-WiFi"));

  // Reconnect before the next test group.
  wifi_connect();
#else
  Serial.println(F("\n-- stale time: skipped"
                   " (WiFiEsp does not support WiFi.disconnect() on AVR) --"));
#endif
}

// ── entry points ─────────────────────────────────────────────────────────────

#define RUN(n, fn) do { \
  Serial.println(F("\n>>> [" #n "/8] " #fn)); \
  int _p = g_passed, _f = g_failed; \
  fn(); \
  Serial.print(F("<<< [" #n "/8] ")); \
  Serial.print(g_passed - _p); Serial.print(F(" passed, ")); \
  Serial.print(g_failed - _f); Serial.println(F(" failed")); \
} while (0)

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("\n=== EasyNTPClient test suite ==="));

#if !defined(ESP8266) && !defined(ESP32)
  esp_serial.begin(9600);
  WiFi.init(&esp_serial);
#endif

  wifi_connect();

  RUN(1, test_constants);
  RUN(2, test_basic_sync);
  RUN(3, test_client_reuse);
  RUN(4, test_offset_immediate);
  RUN(5, test_was_updated);
  RUN(6, test_set_ntp_server);
  RUN(7, test_set_update_interval);
  RUN(8, test_stale_time);

  Serial.println(F("\n=== Results ==="));
  Serial.print(g_passed); Serial.println(F(" passed"));
  Serial.print(g_failed); Serial.println(F(" failed"));
}

#undef RUN

void loop() {}
